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
#include "lock.h"               /* lock, unlock */
#include "join_params.h"         /* constant parameters */
#include "cpu_mapping.h"        /* get_cpu_id */
//#include "queue.h"
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

#ifndef HASH
#define HASH(X, MASK, SKIP) (((X) & MASK) >> SKIP)
#endif

struct arg_t {
    int32_t tid;
    Hashtable *ht;
    Relation *relR;
    Relation *relS;
    pthread_barrier_t * barrier;
//    int64_t             num_results;
//    int                 num_threads;
    ThreadResult *threadResults; /* results of the thread */

#ifdef MEASURE_LATENCY
    struct timeval start, buildPhaseEnd, probePhaseEnd, end;
#endif
};

/**
 * Allocates a hashtable of NUM_BUCKETS and inits everything to 0.
 * @param ht pointer to a Hashtable pointer
 */
void allocate_hashtable(Hashtable ** ppht, uint32_t nbuckets) {

    Hashtable * ht;
    ht              = (Hashtable*)malloc(sizeof(Hashtable));
    ht->num_buckets = nbuckets;
    NEXT_POW_2((ht->num_buckets));

    // Allocate hashtable buckets cache line aligned.
    // Allocate (ht->num_buckets * sizeof(Bucket)) bytes and place the address of the allocated memory
    // in (void**)&ht->buckets. The address of the allocated memory will be a multiple of CACHE_LINE_SIZE.
    if (posix_memalign((void**)&ht->buckets, CACHE_LINE_SIZE, ht->num_buckets * sizeof(Bucket))) {
        perror("Aligned allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    // ht->buckets: pointer to block of memory to fill
    // 0: value to set
    // ht->num_buckets * sizeof(Bucket): number of bytes to be set to the value.
    memset(ht->buckets, 0, ht->num_buckets * sizeof(Bucket));
    ht->skip_bits = 0; /* the default for modulo hash */
    ht->hash_mask = (ht->num_buckets - 1) << ht->skip_bits;
    *ppht = ht;
}

/**
 * Initializes a new BucketBuffer for later use in allocating
 * buckets when overflow occurs.
 *
 * @param ppbuf [in,out] bucket buffer to be initialized
 */
void init_bucket_buffer(BucketBuffer ** ppbuf) {
    BucketBuffer * overflowbuf;
    overflowbuf = (BucketBuffer*) malloc(sizeof(BucketBuffer));
    overflowbuf->count = 0;
    overflowbuf->next  = NULL;

    *ppbuf = overflowbuf;
}

/**
 * Returns a new Bucket from the given BucketBuffer.
 * If the BucketBuffer does not have enough space, then allocates
 * a new BucketBuffer and adds to the list.
 *
 * @param result [out] the new bucket
 * @param buf [in,out] the pointer to the BucketBuffer pointer
 */
static inline void get_new_bucket(Bucket ** result, BucketBuffer ** buf) {
    if((*buf)->count < OVERFLOW_BUF_SIZE) {
        *result = (*buf)->buf + (*buf)->count;
        (*buf)->count ++;
    }
    else {
        /* need to allocate new buffer */
        BucketBuffer * new_buf = (BucketBuffer*)malloc(sizeof(BucketBuffer));
        new_buf->count = 1;
        new_buf->next  = *buf;
        *buf    = new_buf;
        *result = new_buf->buf;
    }
}

/** De-allocates all the BucketBuffer */
void free_bucket_buffer(BucketBuffer * buf) {
    do {
        BucketBuffer * tmp = buf->next;
        free(buf);
        buf = tmp;
    } while(buf);
}

void build(ThreadArg &args) {

    std::cout << "building" << std::endl;

    QueueTask task = *args.task;
    Relation rel = *args.relR;
    Hashtable ht = *args.ht;
    BucketBuffer *overflowBuf = args.overflowBuf;
    int startTup = task.startTupleIndex;
    int endTup = task.endTupleIndex;

    std::cout << "start = " << startTup << " end = " << endTup << "\n" << std::endl;


    const uint32_t hashmask = args.ht->hash_mask;
    const uint32_t skipbits = args.ht->skip_bits;

    // Loop through all tuples assigned to this thread.
    for (int i = startTup; i < endTup; i++) {
        Tuple * dest;         // dest tuple.
        Bucket * curr, * nxt; // cur, next buckets.

        int32_t idx = HASH(rel.tuples[i].key, hashmask, skipbits);
        // Copy the tuple to appropriate hash bucket.
        // If full, follow nxt pointer to find correct place.
        curr = ht.buckets+idx;
        lock(&curr->latch);
        nxt = curr->next;

        if (curr->count == BUCKET_SIZE) {
            if (!nxt || nxt->count == BUCKET_SIZE) {
                Bucket * b;
                get_new_bucket(&b, &overflowBuf);
                curr->next = b;
                b->next    = nxt;
                b->count   = 1;
                dest       = b->tuples;
            } else {
                dest = nxt->tuples + nxt->count;
                nxt->count ++;
            }
        } else {
            dest = curr->tuples + curr->count;
            curr->count ++;
        }
        *dest = rel.tuples[i]; // Store the tuple in bucket.
        unlock(&curr->latch);
    }
    return;
}

void probe(ThreadArg &args) {

//    std::cout << "probing" << std::endl;

    QueueTask task = *args.task;
    Relation rel = *args.relS;
    Hashtable ht = *args.ht;
    BucketBuffer *overflowBuf = args.overflowBuf;
    int startTup = task.startTupleIndex;
    int endTup = startTup + args.taskSize;

    uint32_t i, j;
    int64_t matches = 0;
    const uint32_t hashmask = args.ht->hash_mask;
    const uint32_t skipbits = args.ht->skip_bits;

    // Loop through all tuples this thread was assigned.
    for (int i = startTup; i < endTup; i++) {

        // Calculate hash.
        intkey_t idx = HASH(rel.tuples[i].key, hashmask, skipbits);

        // Use hash as offset to find relevant bucket.
        Bucket * b = ht.buckets+idx;

        do {
            for (j = 0; j < b->count; j++) {
                if (rel.tuples[i].key == b->tuples[j].key) {
                    matches ++;
                    ChainTuple * chainTup = cb_next_writepos(args.threadJoinResults->chainedTupBuf);
                    chainTup->key        = rel.tuples[i].key;    /* key */
                    chainTup->sPayload  = rel.tuples[i].payload; /* S-rid */
                    chainTup->rPayload  = b->tuples[j].payload;  /* R-rid */
                }
            }
            b = b->next;    // Follow overflow pointer.
        } while (b);        // AKA while there is a valid next pointer.
    }
    args.threadJoinResults->numResults += matches;
    return;
}

