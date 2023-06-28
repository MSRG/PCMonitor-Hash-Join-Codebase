
#include <iostream>
#include <fstream>
#include <functional>

#include "queue.h"
#include "thread_pool.h"
pthread_mutex_t queue_pp_lock;
pthread_mutex_t tasks_remaining_p_lock;

Queue::Queue(int rSize_, int sSize_, int taskSize_, relation_t *relR_, relation_t *relS_) {
    rSize = rSize_;
    sSize = sSize_;
    taskSize = taskSize_;
    relR = *relR_;
    relS = *relS_;
    remainingTuples = rSize_;
    currentTupIndex = 0;
    currentPhase = 1;
    done = false;

    std::cout << "Initializing queue." << std::endl;
    std::cout << "r size = " << relR.num_tuples <<  std::endl;
    std::cout << "s size = " << relS.num_tuples <<  std::endl;

}

int Queue::getTask(queueTask_ &task) {


    pthread_mutex_lock(&queue_pp_lock);
    std::cout << "get task" << std::endl;
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
        std::cout << "remaining was 0, reset to " << relS.num_tuples << std::endl;

        currentPhase += 1;
        remainingTuples = sSize;
        currentTupIndex = 0;
    }

    if (currentPhase == 1) {
        // on Build phase
//        return &tp->build;
//        return (std::bind(&ThreadPool::build, this));
        task.function = (void*)&ThreadPool::build;
        pthread_mutex_unlock(&queue_pp_lock);
        return 1;
    }
    if (currentPhase == 2) {
        task.function = (void*)&ThreadPool::probe;
        // on Probe phase
//        return ThreadPool::probe;
        pthread_mutex_unlock(&queue_pp_lock);
        return 2;
    }
    if (currentPhase == 3) {
        // done!
        std::cout << "done!!" << std::endl;
        done = true;
        pthread_mutex_unlock(&queue_pp_lock);
        return NULL;
    }

//    if (remainingTuples) {
//        std::cout << "get task 1" << std::endl;
//    remainingTuples -= taskSize;
//    }
//    else {
//    task.taskSize = -3;
//    }
}

bool Queue::tasksRemaining() {
    return !done;
}