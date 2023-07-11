#include "safe_queue.h"

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


pthread_mutex_t queue_lock;

SafeQueue::SafeQueue() {
    empty = false;

}

// Add an element to the queue.
void SafeQueue::enqueue(QueueTask task) {

    pthread_mutex_lock(&queue_lock);
    q.push(task);
    pthread_mutex_unlock(&queue_lock);

}

// Get the "front"-element.
bool SafeQueue::dequeue(QueueTask &task) {
    std::cout << "trying to dequeue" << std::endl;

//    std::unique_lock<std::mutex> lock(m);
    pthread_mutex_lock(&queue_lock);

    if (!q.empty()) {
        std::cout << "dequeue" << std::endl;

        QueueTask qTask = q.front();
        task.startTupleIndex = qTask.startTupleIndex;
        task.endTupleIndex = qTask.endTupleIndex;
        task.function = qTask.function;
        q.pop();
        pthread_mutex_unlock(&queue_lock);
        return true;
//        std::unique_lock<std::mutex> unlock(m);
    }
    pthread_mutex_unlock(&queue_lock);
    return false;
}

bool SafeQueue::isEmpty() {
//    std::unique_lock<std::mutex> lock(m);
    pthread_mutex_lock(&queue_lock);

    if (q.empty()) {
//        std::unique_lock<std::mutex> unlock(m);
        std::cout << "queue empty" << std::endl;
        pthread_mutex_unlock(&queue_lock);
        return true;
    }

    std::cout << "queue not empty" << std::endl;
    pthread_mutex_unlock(&queue_lock);
//    std::unique_lock<std::mutex> unlock(m);
    return false;
}