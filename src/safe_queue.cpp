#include <thread>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include <pthread.h>            /* pthread_ */
#include <string.h>             /* memset */
#include <stdio.h>              /* printf */
#include <stdlib.h>             /* memalign */
#include <sys/time.h>           /* gettimeofday */
#include <chrono>
#include <functional>
#include <sys/time.h>           /* gettimeofday */
#include <vector>
#include <math.h>               /* ceil */

#include "safe_queue.h"

pthread_mutex_t queue_lock;

SafeQueue::SafeQueue() {
    // Constructor
}

/**
 * Add task to queue.
*/
void SafeQueue::safeEnqueue(QueueTask task) {
    pthread_mutex_lock(&queue_lock);
    q.push(task);       // push method pushes a copy of task.
    pthread_mutex_unlock(&queue_lock);
}

/**
 * Pop off the least recently inserted task object in the queue.
*/
bool SafeQueue::safeDequeue(QueueTask &task) {

    pthread_mutex_lock(&queue_lock);

    if (!q.empty()) {
        QueueTask qTask = q.front();
        task.startTupleIndex = qTask.startTupleIndex;
        task.endTupleIndex = qTask.endTupleIndex;
        task.function = qTask.function;
        q.pop();
        pthread_mutex_unlock(&queue_lock);
        return true;
    }
    pthread_mutex_unlock(&queue_lock);
    return false;
}

/**
 * Return empty status of queue.
*/
bool:: SafeQueue::isQueueEmpty() {
    return q.empty();
}