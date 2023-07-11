#ifndef SAFE_QUEUE
#define SAFE_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

#include "types.h"

class SafeQueue {

    public:
        SafeQueue();
        void enqueue(QueueTask task);
        bool dequeue(QueueTask &task);
        bool isEmpty();

    private:
        bool empty;
        std::queue<QueueTask> q;
        mutable std::mutex m;
        std::mutex queue_m;
        pthread_mutex_t test_lock;

        std::condition_variable queue_cond;
};

#endif