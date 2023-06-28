
#ifndef POOL_HASHJOIN_PCM_QUEUE_H
#define POOL_HASHJOIN_PCM_QUEUE_H

#include "types.h"
//#include "thread_pool.h"
#include <mutex>
#include <condition_variable>
#include <pthread.h>

//typedef void (&fptr)(queueTask &task);

class Queue {
//    int numThreads;
//    pthread_t *tid;
    int rSize, sSize, taskSize, remainingTuples, currentTupIndex, phasesRemaining, currentPhase;
    relation_t relR;
    relation_t relS;
    bool done;

    public:
        Queue(int r_Size, int s_Size, int task_Size, relation_t *rel_R, relation_t *rel_S);
        int getTask(queueTask_ &task);
        void QueueJob();
        void stop();
        void busy();
        bool tasksRemaining();
//        bool done;
    private:
        void ThreadLoop();
        bool should_terminate = false;           // Tells threads to stop looking for jobs
//        pthread_mutex_t queue_p_lock;
        std::mutex queue_mutex;                  // Prevents data races to the job queue
        std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination
//        std::vector<std::thread> threads;
//        std::queue<std::function<void()>> jobs;
};

#endif