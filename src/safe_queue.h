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
        bool isQueueEmpty();

    private:
        std::queue<QueueTask> q;
};

#endif