
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
#include <math.h>       /* ceil */

#include "thread_pool.h"
#include "cpu_mapping.h"        /* get_cpu_id */
#include "join_params.h"         /* constant parameters */
#include "join.h"

pthread_mutex_t queue_p_lock;

ThreadPool::ThreadPool(int numThreads_, Relation *relR_, Relation *relS_, Hashtable *ht_, int taskSize_, SafeQueue &buildQ_) {

    // Assign constructor arguments to class members.
    numThreads = numThreads_;
    relR = relR_;
    relS = relS_;
    ht = ht_;
    taskSize = taskSize_;
    buildQ = &buildQ_;

    // Initializations.
    phase = 1;
    currentTupIndex = 1;
    buildDone = false;
    probeDone = false;
    tsBuild = false;
    tsProbe = false;
    tsEnd = false;

    joinResults = (JoinResults *) malloc(sizeof(JoinResults) * numThreads);
}

void ThreadPool::buildQueue() {

    double buildTasks =  ceil((double) relR->num_tuples / taskSize);
    double probeTasks =  ceil((double) relS->num_tuples / taskSize);
    int start = 0;
    int end = 0;

    std::cout << "number of build tasks = " << buildTasks << std::endl;
    std::cout << "number of probe tasks = " << probeTasks << std::endl;

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

        // enqueue
        buildQ->enqueue(task);
        std::cout << "start = " << start << " end = " << end << "\n" << std::endl;
        start += taskSize;
    }

}

void ThreadPool::readQueue() {

//    while (!buildQ.isEmpty()) {
//        QueueTask task = buildQ.dequeue();
//        std::cout << "start = " << task.startTupleIndex << " end = " << task.endTupleIndex << "\n" << std::endl;
////        buildQ.pop();
//
//    }

}



void ThreadPool::getTask(QueueTask &task) {

    pthread_mutex_lock(&queue_p_lock);

    if (phase == 1) { // Building phase, using relR.

        task.startTupleIndex = currentTupIndex;
        task.function = &build;

        if ((currentTupIndex + taskSize) >= relR->num_tuples) { // All relR tuples have been dispatched.
            currentTupIndex = 1;
            phase = 2;
            buildDone = true;
        } else {
            currentTupIndex += taskSize;
        }
        pthread_mutex_unlock(&queue_p_lock);
        return;
    } else if (phase == 2) { // Probing phase, using relS.

        task.startTupleIndex = currentTupIndex;
        task.function = &probe;

        if ((currentTupIndex + taskSize) >= relS->num_tuples) { // All relS tuples have been dispatched.
            probeDone = true;
        } else {
            currentTupIndex += taskSize;
        }
    }
    pthread_mutex_unlock(&queue_p_lock);
    return;
}

bool ThreadPool::buildTasksRemaining() {
    return !buildDone;
}

bool ThreadPool::probeTasksRemaining() {
    return !probeDone;
}

void ThreadPool::run(ThreadArg * args) {

    init_bucket_buffer(&args->overflowBuf);

    pthread_barrier_wait(args->barrier);
    if (args->tid == 0) { gettimeofday(&(ts->startTime), NULL); }

    while (buildQ->dequeue(*(args->task))) {

//        buildQ->dequeue(*(args->task));
//        args->task = &qTask;

        // Performance Counter check.

//        getTask(*(args->task));
//        if (!buildQ->isEmpty()) {
        (args->task->function)(*args);
//        }
    }

    return;
    pthread_barrier_wait(args->barrier);
    if (args->tid == 0) { gettimeofday(&(ts->buildPhaseEnd), NULL); }

//    while (probeTasksRemaining()) {
//
//        // Performance Counter check.
//
//        getTask(*(args->task));
//        (args->task->function)(*args);
//    }
//
//    pthread_barrier_wait(args->barrier);
//    if (args->tid == 0) { gettimeofday(&ts->endTime, NULL); }

    free_bucket_buffer(args->overflowBuf); // Clean-up the overflow buffers.
}


//void ThreadPool::run(ThreadArg * args) {
//
//    init_bucket_buffer(&args->overflowBuf);
//
//    pthread_barrier_wait(args->barrier);
//    if (args->tid == 0) { gettimeofday(&(ts->startTime), NULL); }
//
//    while (buildTasksRemaining()) {
//
//        // Performance Counter check.
//
//        getTask(*(args->task));
//        (args->task->function)(*args);
//    }
//
//    pthread_barrier_wait(args->barrier);
//    if (args->tid == 0) { gettimeofday(&(ts->buildPhaseEnd), NULL); }
//
//    while (probeTasksRemaining()) {
//
//        // Performance Counter check.
//
//        getTask(*(args->task));
//        (args->task->function)(*args);
//    }
//
//    pthread_barrier_wait(args->barrier);
//    if (args->tid == 0) { gettimeofday(&ts->endTime, NULL); }
//
//    free_bucket_buffer(args->overflowBuf); // Clean-up the overflow buffers.
//}

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

//    pthread_barrier_wait(args->barrier);
//    saveTimingResults();

    stop();
    pthread_barrier_destroy(&barrier);
}

void ThreadPool::stop() {
    for (std::thread & t : threads) {
        t.join();
    }
    std::cout << "DONE! " << std::endl;
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
