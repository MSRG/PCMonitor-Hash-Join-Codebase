#ifndef POOL_HASHJOIN_PCM_THREAD_POOL_H
#define POOL_HASHJOIN_PCM_THREAD_POOL_H

#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>

#include "queue.h"
#include "types.h"

typedef struct arg_thread arg_thread;
typedef struct queueTask queueTask;

class ThreadPool {
    bool done;
    Queue *queue;
    pthread_t *tid;
    relation_t *relR;
    relation_t *relS;
    int numThreads, taskSize, remainingTuples, currentTupIndex, phasesRemaining, currentPhase;

    public:
        ThreadPool(int numThreads, Queue &queue, relation_t *relR_, relation_t *relS_);
        void start();
        void run(arg_thread * param);
        void getTask(queueTask &task);
        void build(queueTask &task);
        void probe(queueTask &task);
        bool tasksRemaining();
        void stop();
//        void QueueJob();
//        void busy();

    private:
        std::vector<std::thread> threads;
        std::mutex queue_mutex;                  // Prevents data races to the job queue
        std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination
};

typedef void (ThreadPool::*PoolMemFn)(queueTask &task);

struct queueTask {
    int          size;
    int          startTupleIndex;
    int          endTupleIndex;
    PoolMemFn   function;

};

#endif