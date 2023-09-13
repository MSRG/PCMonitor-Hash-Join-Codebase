#ifndef FINE_GRAINED_QUEUE
#define FINE_GRAINED_QUEUE

#include <iostream>
#include <vector>
#include <mutex>
#include <stdio.h>
#include <pthread.h>            /* pthread_ */

#include "types.h"

class FineGrainedQueue {

    public:
        FineGrainedQueue(double numOfTasks);
        void enqueue(QueueTask task);
        bool dequeue(QueueTask &task, int &lastTaskVectorPosition);
        bool isQueueEmpty();

    private:
        int numOfTasks;
//        std::vector<QueueTask> queueVector;
        std::vector<std::pair<std::pair<bool,FineGrainedLock>, QueueTask>> queueVector;

};

#endif