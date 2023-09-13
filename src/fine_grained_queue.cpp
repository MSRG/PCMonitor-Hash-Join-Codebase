#include "fine_grained_queue.h"

//pthread_mutex_t enqueue_lock;
//pthread_mutex_t dequeue_lock;

std::mutex enqueue_mtx;           // mutex for critical section

FineGrainedQueue::FineGrainedQueue(double numOfTasks_) {
    std::cout << "hi" << std::endl;
    numOfTasks = numOfTasks_;
    // Constructor
}

/**
 * Add task to vector queue.
*/
void FineGrainedQueue::enqueue(QueueTask task) {
//    pthread_mutex_lock(&enqueue_lock);
    enqueue_mtx.lock();

    FineGrainedLock taskLock;                           // new lock for this task.
//    std::cout << sizeof(taskLock) << std::endl;       // 64 bytes.
    // Create pairs.
    std::pair<std::pair<bool, FineGrainedLock>, QueueTask> lockTaskPair; // new pair for the task vector.
    std::pair<bool, FineGrainedLock> lockPair;

    // Fill pairs.
    lockPair = std::make_pair(true, taskLock);
    lockTaskPair = std::make_pair(lockPair, task);      // populate pair with lock and task.

    // Push into event vector.
    queueVector.push_back(lockTaskPair);                // push method pushes a copy of task.

//    pthread_mutex_unlock(&enqueue_lock);
    enqueue_mtx.unlock();
}

/**
 * Pop off the least recently inserted task object in the queue.
*/
bool FineGrainedQueue::dequeue(QueueTask &task, int &lastTaskVectorPosition) {
    lastTaskVectorPosition += 1;

    while ((lastTaskVectorPosition < numOfTasks)  &&
        (pthread_mutex_trylock(&(queueVector[lastTaskVectorPosition].first.second.fineLock)) == -1)

        ) {
        lastTaskVectorPosition += 1;
    }
    if (lastTaskVectorPosition >= numOfTasks) { // all the tasks are taken or done.
        return false;
    }
    else {
        if (queueVector[lastTaskVectorPosition].first.first == true) {
            queueVector[lastTaskVectorPosition].first.first = false;
            QueueTask qTask = queueVector[lastTaskVectorPosition].second;
            task.startTupleIndex = qTask.startTupleIndex;
            task.endTupleIndex = qTask.endTupleIndex;
            task.function = qTask.function;
            return true;
        } else {
            return false;
        }
    }
}

bool FineGrainedQueue::isQueueEmpty() {
    // TODO Implement!
    return true;
}

