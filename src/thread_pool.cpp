
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

//#include <mutex>
//#include <condition_variable>

#include <vector>

#include "thread_pool.h"
#include "cpu_mapping.h"        /* get_cpu_id */
#include "join_params.h"         /* constant parameters */
//#include "types.h"

pthread_mutex_t queue_p_lock;

struct arg_thread {
    int32_t tid;
    hashtable_t *ht;
    relation_t *relR;
    relation_t *relS;
    Queue *queue;
    queueTask  *task;
    pthread_barrier_t *barrier;
//    int64_t             num_results;
//    int                 num_threads;
    threadresult_t *threadResults; /* results of the thread */

#ifdef MEASURE_LATENCY
    struct timeval start, buildPhaseEnd, probePhaseEnd, end;
#endif
};

ThreadPool::ThreadPool(int nthreads, Queue &queue_, relation_t *relR_, relation_t *relS_) {
    numThreads = nthreads;
    pthread_t threadIds[numThreads];
    tid = threadIds;
    queue = &queue_;
    remainingTuples = 1;
    relR = relR_;
    relS = relS_;
    taskSize = 10;

    remainingTuples = relR->num_tuples;
    currentTupIndex = 0;
    currentPhase = 1;
    done = false;
}

void ThreadPool::build(queueTask &task) {
    std::cout << "building" << std::endl;
    std::cout << "R = " << relR->num_tuples << std::endl;
    return;
}

void ThreadPool::probe(queueTask &task) {
    std::cout << "probing" << std::endl;
    return;
}

void ThreadPool::getTask(queueTask &task) {

    pthread_mutex_lock(&queue_p_lock);
    int step;

    step = ((remainingTuples >= taskSize) ? (taskSize) : (remainingTuples));

    std::cout << "remaining = " << remainingTuples << std::endl;
    std::cout << "step = " << step << std::endl;

    task.startTupleIndex = currentTupIndex;
    task.endTupleIndex = currentTupIndex + step;
    task.size = step;

    currentTupIndex += step;
    remainingTuples -= step;

    if (remainingTuples == 0) {
        std::cout << "remaining was 0, reset to " << relS->num_tuples << std::endl;
        currentPhase += 1;
        remainingTuples = relS->num_tuples;
        currentTupIndex = 0;
    }

    if (currentPhase == 1) {
        task.function = &ThreadPool::build;
        pthread_mutex_unlock(&queue_p_lock);
        return;
    }
    if (currentPhase == 2) {
        task.function = &ThreadPool::probe;
        pthread_mutex_unlock(&queue_p_lock);
        return;
    }
    if (currentPhase == 3) {
        // done!
        std::cout << "done!!" << std::endl;
        done = true;
        pthread_mutex_unlock(&queue_p_lock);
        return;
    }
}

bool ThreadPool::tasksRemaining() {
    return !done;
}


void ThreadPool::run(arg_thread * args) {

        while (tasksRemaining()) {
            getTask(*(args->task));
            (this->*(args->task->function))(*(args->task));
        }
        pthread_barrier_wait(args->barrier);
}

void ThreadPool::start() {

    int i;
    cpu_set_t set;                        // Linux struct representing set of CPUs.
    pthread_attr_t attr;
   // Initializes the thread attributes object pointed to by attr with default attribute values.
    pthread_attr_init(&attr);
    pthread_barrier_t barrier;
    arg_thread args[numThreads];
    queueTask tasks[numThreads];
    pthread_t tid[numThreads];
    void * argument[numThreads];

    // Initialize thread barrier.
    if(pthread_barrier_init(&barrier, NULL, numThreads) != 0) {
        printf("Couldn't create the barrier\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < numThreads; i++) {

        int cpu_idx = get_cpu_id(i);
        CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
        CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.
//        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set); // Bind a thread to a specific core.

        // Thread arg set-up.
        args[i].tid = i;
//        args[i].ht = ht;
//        args[i].relR = relR;
//        args[i].relS = relS;
//        args[i].threadResults = &(joinresult->resultlist[i]);
        args[i].queue = queue;
        args[i].task = &tasks[i];
        args[i].barrier = &barrier;

//        std::thread t0([&]() { work(a.val); });
//        assert(pthread_setaffinity_np(t0.native_handle(), sizeof(cpu_set_t),
//                                &cpu_set_1) == 0);

        int a = 5;
//        std::thread t1(&ThreadPool::run, this, std::ref(a));
//        argument[i] = (void*)&args[i];
        threads.emplace_back(&ThreadPool::run, this, &args[i]);
//        DWORD_PTR dw = SetThreadAffinityMask(threads.back().native_handle(), DWORD_PTR(1) << i);
        int rv = pthread_setaffinity_np(threads.back().native_handle(), sizeof(cpu_set_t), &set);
        if (rv != 0) {
          std::cerr << "Error calling pthread_setaffinity_np: " << rv << "\n";
        }

//        int rv = pthread_create(&tid[i], &attr, start_thread, (void*)this);
//        if (rv) {
//            printf("[ERROR] return code from pthread_create() is %d\n", rv);
//            exit(-1);
//        }
//        else { std::cout << "\nCreated thread with threadID: " << tid[i] << std::endl; }
    }

//    t1.join();

    std::cout << "back in main " << std::endl;
//    stop();

    // later
    for (std::thread & t : threads) {
        t.join();
    }
    std::cout << "DONE! " << std::endl;
    pthread_barrier_destroy(&barrier);

}

void ThreadPool::stop() {
    std::cout << "in stop " << std::endl;


   for (int i = 0; i < numThreads; i++) {
        pthread_join(tid[i], NULL);
        /* sum up results */
//        result += args[i].num_results;
    }
}