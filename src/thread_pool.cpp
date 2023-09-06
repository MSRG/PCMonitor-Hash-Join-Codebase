
#include <thread>
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
#include <functional>
#include <sys/time.h>           /* gettimeofday */
#include <vector>
#include <math.h>               /* ceil */

#include "thread_pool.h"
#include "cpu_mapping.h"        /* get_cpu_id */
#include "join_params.h"         /* constant parameters */
#include "join.h"

//#include "result_file_names.h"

const char * TIMING_CSV = "timing-results.csv";
const char * THREAD_RESULTS_CSV = "individual-thread-results.csv";

pthread_mutex_t results_lock;

ThreadPool::ThreadPool(int numThreads_, Relation &relR_, Relation &relS_, Hashtable &ht_, int taskSize_, SafeQueue &buildQ_, SafeQueue &probeQ_, PcmMonitor &pcmMonitor_) {

    // Assign constructor arguments to class members.
    numThreads = numThreads_;
    relR = &relR_;
    relS = &relS_;
    ht = &ht_;
    taskSize = taskSize_;
    buildQ = &buildQ_;
    probeQ = &probeQ_;
    pcmMonitor = &pcmMonitor_;

    // Initializations.
    phase = 1;
    currentTupIndex = 1;
    joinResults = (JoinResults *) malloc(sizeof(JoinResults) * numThreads);
}

void ThreadPool::populateQueues() {

    double buildTasks =  ceil((double) relR->num_tuples / taskSize);
    double probeTasks =  ceil((double) relS->num_tuples / taskSize);
    int start = 0;
    int end = 0;

    // Build the BUILD PHASE Queue.
    for (int i = 0; i < buildTasks; i++) {
        if ((start + taskSize) <= relR->num_tuples) {
            end = start + (taskSize - 1);
        } else {
            end = relR->num_tuples - 1;
        }

        QueueTask task;
        task.startTupleIndex = start;
        task.endTupleIndex = end;
        task.function = &build;

        buildQ->enqueue(task);
        start += taskSize;
    }

    start = 0;
    end = 0;

    // Build the PROBE PHASE Queue.
    for (int i = 0; i < probeTasks; i++) {
        if ((start + taskSize) <= relS->num_tuples) {
            end = start + (taskSize - 1);
        } else {
            end = relR->num_tuples - 1;
        }

        QueueTask task;
        task.startTupleIndex = start;
        task.endTupleIndex = end;
        task.function = &probe;

        probeQ->enqueue(task);
        start += taskSize;
    }
}

void ThreadPool::saveTimingResults() {
    // Micro Seconds
    double totalDiffUsec = ((ts.endTime).tv_sec*1000000L + (ts.endTime).tv_usec) - ((ts.startTime).tv_sec*1000000L+(ts.startTime).tv_usec);
    double buildDiffUsec = (((ts.buildPhaseEnd).tv_sec*1000000L + (ts.buildPhaseEnd).tv_usec)
                            - ((ts.startTime).tv_sec*1000000L+(ts.startTime).tv_usec));
    double probeDiffUsec = (((ts.endTime).tv_sec*1000000L + (ts.endTime).tv_usec)
                            - ((ts.buildPhaseEnd).tv_sec*1000000L+(ts.buildPhaseEnd).tv_usec));
    // Seconds
    double totalDiffSec = totalDiffUsec * 0.000001;
    double buildDiffSec = buildDiffUsec * 0.000001;
    double probeDiffSec = probeDiffUsec * 0.000001;
    // Minutes
    double totalDiffMin = totalDiffSec/60;

    if (TIMING_CSV) {
        std::fstream file(TIMING_CSV, std::ios::app);
        file << "total-seconds,build-seconds,probe-seconds\n";
        file << totalDiffSec;
        file << ",";
        file << buildDiffSec;
        file << ",";
        file << probeDiffSec;
        file << "\n";
    }
}

void ThreadPool::freeThreadsIfBuildQueueEmpty() {
    if (buildQ->isQueueEmpty()) {
        for (int i = 0; i <= 14; i++) {
            pcmMonitor->cv[i].notify_one();
        }
    }
}

void ThreadPool::freeThreadsIfProbeQueueEmpty() {
    if (probeQ->isQueueEmpty()) {
        for (int i = 0; i <= 14; i++) {
            pcmMonitor->cv[i].notify_one();
        }
    }
}

bool ThreadPool::checkThreadStatusDuringBuild(ThreadArg &args) {
    if (!pcmMonitor->shouldThreadStop(args.tid)) {
        return true; // Thread is good to continue working.
    } else {
        std::cout << "thread " << args.tid << " is haulting work on build." << std::endl;
        // lock.
        std::unique_lock<std::mutex> lk(pcmMonitor->mutx[args.tid]);
        // wait.
        pcmMonitor->cv[args.tid].wait(lk);
        // has been notified.
        std::cout << "thread " << args.tid << " is resuming work in build." << std::endl;

        return true;
    }
}

bool ThreadPool::checkThreadStatusDuringProbe(ThreadArg &args) {
    if (!pcmMonitor->shouldThreadStop(args.tid)) {
        return true; // Thread is good to continue working.
    } else {
        std::cout << "thread " << args.tid << " is haulting work on probe." << std::endl;
        // lock.
        std::unique_lock<std::mutex> lk(pcmMonitor->mutx[args.tid]);
        // wait.
        pcmMonitor->cv[args.tid].wait(lk);
        // has been notified.
        std::cout << "thread " << args.tid << " is resuming work in probe." << std::endl;
        return true;
    }
}

void saveIndividualThreadResults(ThreadArg &args) {
    pthread_mutex_lock(&results_lock);

    if (THREAD_RESULTS_CSV) {
        std::fstream file(THREAD_RESULTS_CSV, std::ios::app);
        file << sched_getcpu();
        file << ",";
        file << args.completedTasks;
        file << ",";
        file << args.matches;
        file << ",";
        file << args.matchTasks;
        file << ",";
        file << args.nonMatchTasks;
        file << "\n";
        file.close();
    }
    pthread_mutex_unlock(&results_lock);
}

void ThreadPool::run(ThreadArg * args) {
//  std::cout << "Thread #" << args->tid << ": on CPU " << sched_getcpu() << "\n";

    init_bucket_buffer(&args->overflowBuf);
    if (args->tid == 1) {
        gettimeofday(&(ts.startTime), NULL);
    }

    while (checkThreadStatusDuringBuild(*args)) {
        if (buildQ->dequeue(*(args->task))) {
            (args->task->function)(*args);
        } else { break; }
    }

//    freeThreadsIfBuildQueueEmpty();
//    pthread_barrier_wait(args->barrier);
    if (args->tid == 0) {
        gettimeofday(&(ts.buildPhaseEnd), NULL);
    }

    if (!probeQ->isQueueEmpty()){
        while (checkThreadStatusDuringProbe(*args)) {
            if (probeQ->dequeue(*(args->task))) {
                (args->task->function)(*args);
            } else { break; }
        }
    }

    freeThreadsIfProbeQueueEmpty();
    pthread_barrier_wait(args->barrier);
    if (args->tid == 0) {
        gettimeofday(&(ts.endTime), NULL);
    }

    args->nonMatchTasks = (args->completedTasks - args->matchTasks);
    saveIndividualThreadResults(*args);
    free_bucket_buffer(args->overflowBuf); // Clean-up the overflow buffers.
}


void ThreadPool::start() {

    int i;
    cpu_set_t set;             // Linux struct representing set of CPUs.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_barrier_t barrier;
    pthread_t tid[numThreads];

    ThreadArg args[numThreads];
    QueueTask tasks[numThreads];
    BucketBuffer overflowBuf[numThreads];

    // Initialize thread barrier.
    if (pthread_barrier_init(&barrier, NULL, numThreads) != 0) {
        printf("Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < numThreads; i++) {
        int cpu_idx = get_cpu_id(i);
        CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
        CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.

        ChainedTupleBuffer * chainedTupBuf = chainedtuplebuffer_init();

        // Thread arg set-up.
        args[i].tid = i;
        args[i].matches = 0;
        args[i].completedTasks = 0;
        args[i].matchTasks = 0;
        args[i].ht = ht;
        args[i].relR = relR;
        args[i].relS = relS;
        args[i].taskSize = taskSize;
        args[i].task = &tasks[i];
        args[i].overflowBuf = overflowBuf;
        args[i].barrier = &barrier;

        args[i].threadJoinResults = &(joinResults[i]);
        args[i].threadJoinResults->chainedTupBuf = chainedTupBuf;
        args[i].threadJoinResults->numResults = 0;

        threads.emplace_back(&ThreadPool::run, this, &args[i]);
        int rv = pthread_setaffinity_np(threads.back().native_handle(), sizeof(cpu_set_t), &set);
        if (rv != 0) {
          std::cerr << "Error calling pthread_setaffinity_np: " << rv << "\n";
        }
    }

    stop();
    saveTimingResults();
    pthread_barrier_destroy(&barrier);
}

void ThreadPool::stop() {
    for (std::thread & t : threads) {
        t.join();
    }
}

void ThreadPool::saveJoinedRelationToFile() {
    std::cout << "Saving timing results to file." << std::endl;
    std::cout << numThreads << " Threads." << std::endl;

    int i, j, numResults;
    const char * filename = "result.tbl";
    FILE * fp = fopen(filename, "w");

    fprintf(fp, "KEY, R_VAL, S_VAL\n");

    for (i = 0; i < numThreads; i++) {
        ChainedTupleBuffer * cb = joinResults[i].chainedTupBuf;
        numResults = joinResults[i].numResults;
        cb_begin(cb);

        for (j = 0; j < numResults; j++) {
            ChainTuple * tup = cb_read_next(cb);
            fprintf(fp, "%ld, %ld, %ld\n", (long)tup->key, (long)tup->rPayload, (long)tup->sPayload);
        }
    }
    fclose(fp);
}
