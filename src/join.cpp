//
// Created by Sofia Tijanic on 2023-06-19.
//

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include <pthread.h>            /* pthread_* */
#include <string.h>             /* memset */
#include <stdio.h>              /* printf */
#include <stdlib.h>             /* memalign */
#include <sys/time.h>           /* gettimeofday */
#include <chrono>

#include "join.h"
#include "join_params.h"         /* constant parameters */
#include "cpu_mapping.h"        /* get_cpu_id */
#include "thread_pool.h"
#include "queue.h"

//pthread_barrier_t barrier;
typedef struct arg_t arg_t;

#ifndef NEXT_POW_2
/**
 *  compute the next number, greater than or equal to 32-bit unsigned v.
 *  taken from "bit twiddling hacks":
 *  http://graphics.stanford.edu/~seander/bithacks.html
 */
#define NEXT_POW_2(V)                           \
    do {                                        \
        V--;                                    \
        V |= V >> 1;                            \
        V |= V >> 2;                            \
        V |= V >> 4;                            \
        V |= V >> 8;                            \
        V |= V >> 16;                           \
        V++;                                    \
    } while(0)
#endif

struct arg_t {
    int32_t tid;
    hashtable_t *ht;
    relation_t *relR;
    relation_t *relS;
    pthread_barrier_t * barrier;
//    int64_t             num_results;
//    int                 num_threads;
    threadresult_t *threadResults; /* results of the thread */

#ifdef MEASURE_LATENCY
    struct timeval start, buildPhaseEnd, probePhaseEnd, end;
#endif
};


/**
 * Allocates a hashtable of NUM_BUCKETS and inits everything to 0.
 * @param ht pointer to a hashtable_t pointer
 */
void allocate_hashtable(hashtable_t ** ppht, uint32_t nbuckets) {

    hashtable_t * ht;
    ht              = (hashtable_t*)malloc(sizeof(hashtable_t));
    ht->num_buckets = nbuckets;
    NEXT_POW_2((ht->num_buckets));

    // Allocate hashtable buckets cache line aligned.
    // posix_memalign(void **memptr, size_t alignment, size_t size); [Option End]
    // Allocate (ht->num_buckets * sizeof(bucket_t)) bytes and place the address of the allocated memory
    // in (void**)&ht->buckets. The address of the allocated memory will be a multiple of CACHE_LINE_SIZE.
    if (posix_memalign((void**)&ht->buckets, CACHE_LINE_SIZE, ht->num_buckets * sizeof(bucket_t))) {
        perror("Aligned allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    // ht->buckets: pointer to block of memory to fill
    // 0: value to set
    // ht->num_buckets * sizeof(bucket_t): number of bytes to be set to the value.
    memset(ht->buckets, 0, ht->num_buckets * sizeof(bucket_t));
    ht->skip_bits = 0; /* the default for modulo hash */
    ht->hash_mask = (ht->num_buckets - 1) << ht->skip_bits;
    *ppht = ht;
}

/**
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 * @return
 */
//void * run_thread(void * param) {
//        arg_t * args = (arg_t*) param;
//
//        std::cout << "hi from thread " << args->tid << std::endl;
//
//        pthread_barrier_wait(args->barrier);
//}

/**
 * @param param the parameters of the thread, i.e. tid, ht, reln, ...
 * @return
 */
//void * run_thread_OG(void * param) {
//
//        int rv;
////        std::string checkpoint;
//        arg_t * args = (arg_t*) param;
//        bucket_buffer_t * overflowbuf; // Allocate overflow buffer for each thread.
//        init_bucket_buffer(&overflowbuf);
//
//#ifdef PERF_COUNTERS
//    if (args->tid == 0) { // Note: only thread 0 does this.
//        initSaveResultsCSV();
//        PCM_initPerformanceMonitor();
//        PCM_start();
//    }
//#endif
//
//        // Wait at a barrier until each thread starts and start timer.
////        BARRIER_ARRIVE(args->barrier, rv);
//
//#ifdef MEASURE_LATENCY
//        gettimeofday(&args->start, NULL);
//#endif
//
//        // BUILD HASHTABLE: insert tuples from the assigned part of relR to the ht.
//        build_hashtable_mt(args->ht, &args->relR, &overflowbuf);
//
//#ifdef MEASURE_LATENCY
//        gettimeofday(&args->buildPhaseEnd, NULL);
//#endif
//
////        BARRIER_ARRIVE(args->barrier, rv); /// Wait at a barrier until each thread completes build phase.
//
//#ifdef PERF_COUNTERS \
//        if (args->tid == 0) {
//            PCM_stop();
////      checkpoint = "hashtable-built";
////      PCM_SaveResultsCSV(args->num_threads, checkpoint);
//            PCM_start();
//        }
//    /* Just to make sure we get consistent performance numbers */
////    BARRIER_ARRIVE(args->barrier, rv);
//#endif
//
//#ifdef SAVE_JOIN_RESULTS
//        chainedtuplebuffer_t * chainedbuf = chainedtuplebuffer_init();
//#else
//        void * chainedbuf = NULL;
//#endif
//
//        // PROBE HASHTABLE: probe for matching tuples from the assigned part of relS.
//        args->num_results = probe_hashtable(args->ht, &args->relS, chainedbuf);
//
//#ifdef MEASURE_LATENCY
//        // For reliable timing we have to wait until all finish.
////    BARRIER_ARRIVE(args->barrier, rv);
//        gettimeofday(&args->probePhaseEnd, NULL);
//        gettimeofday(&args->end, NULL);
//#endif
//
//#ifdef SAVE_JOIN_RESULTS
//        args->threadresult->nresults = args->num_results;
//        args->threadresult->threadid = args->tid;
//        args->threadresult->results  = (void *) chainedbuf;
//#endif
//
////        BARRIER_ARRIVE(args->barrier, rv);
//
//#ifdef PERF_COUNTERS
//        if (args->tid == 0) {
//            PCM_stop();
////        checkpoint = "hashtable-probed";
////        PCM_SaveResultsCSV(args->num_threads, checkpoint);
//            PCM_cleanup();
//    }
//    /* Just to make sure we get consistent performance numbers */
////    BARRIER_ARRIVE(args->barrier, rv);
//#endif
//
//        free_bucket_buffer(overflowbuf); // Clean-up the overflow buffers.
//        return 0;
//}


result_t * join(relation_t *relR, relation_t *relS, int skew) {
    int nthreads = 1;
    int taskSize = 50;

    Queue queue(relR->num_tuples, relS->num_tuples, taskSize, relR, relS);

    ThreadPool threadPool(nthreads, queue, relR, relS);
    threadPool.start();

    return NULL;

    int i, rv;
    int64_t result = 0;
    int32_t numR, numS, numRthr, numSthr, numRthrOutlier, numSthrOutlier; // total and per thread num.
    int32_t numTuplesR, numTuplesS, numPartitionsR, numPartitionsS;
    cpu_set_t set;                        // Linux struct representing set of CPUs.
    pthread_attr_t attr;
    arg_t args[nthreads];
    pthread_t tid[nthreads];
    pthread_barrier_t barrier;

    // Hashtable Initialization.
    hashtable_t * ht;
    uint32_t nbuckets = (relR->num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
    allocate_hashtable(&ht, nbuckets);

//    numTuplesR = relR->num_tuples;
//    numTuplesS = relS->num_tuples;
    numPartitionsR = 1000;
    numPartitionsS = 1000;

    // Initializes the thread attributes object pointed to by attr with default attribute values.
    pthread_attr_init(&attr);

    // Initialize thread barrier.
    if(pthread_barrier_init(&barrier, NULL, nthreads) != 0){
        printf("Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }

#ifdef SAVE_JOIN_RESULTS
    result_t * joinresult = 0;
    joinresult = (result_t *) malloc(sizeof(result_t));
    joinresult->resultlist = (threadresult_t *) malloc(sizeof(threadresult_t) * nthreads);
#endif

//    for (i = 0; i < nthreads; i++) { // 16 threads
//
//        int cpu_idx = get_cpu_id(i);
//        CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
//        CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.
//        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set); // Bind a thread to a specific core.
//
//        // Thread arg set-up.
//        args[i].tid = i;
//        args[i].ht = ht;
//        args[i].relR = relR;
//        args[i].relS = relS;
//        args[i].threadResults = &(joinresult->resultlist[i]);
//        args[i].barrier = &barrier;
//
//        rv = pthread_create(&tid[i], &attr, run_thread, (void*)&args[i]);
//        if (rv) {
//            printf("[ERROR] return code from pthread_create() is %d\n", rv);
//            exit(-1);
//        }
////        else { std::cout << "\nCreated thread with threadID: " << tid[i] << std::endl; }
//    }
//
//   for (i = 0; i < nthreads; i++) {
//        pthread_join(tid[i], NULL);
//        /* sum up results */
////        result += args[i].num_results;
//    }

    pthread_barrier_destroy(&barrier);

}

