#ifndef POOL_HASHJOIN_PCM_THREAD_POOL_H
#define POOL_HASHJOIN_PCM_THREAD_POOL_H

#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>

#include "types.h"
#include "pcm_monitor.h"
#include "safe_queue.h"

class ThreadPool {


//        SafeQueue probeQ;

    public:

//        std::buildQueue<QueueTask> buildQ;
//        std::probeQueue<QueueTask> probeQ;
        bool buildDone, probeDone, tsBuild, tsProbe, tsEnd;
        Relation *relR, *relS;
        Hashtable *ht;
        JoinResults *joinResults;
        SafeQueue *buildQ;
        Timestamps *ts;
        struct timeval startTime;
        int numThreads, taskSize, currentTupIndex, phase;
        PcmMonitor *pcmMonitor;

        ThreadPool(int numThreads, Relation *relR_, Relation *relS_, Hashtable *ht_, int taskSize_, SafeQueue &buildQ_);
        void start();
        void buildQueue();
        void readQueue();
        void run(ThreadArg * param);
        void getTask(QueueTask &task);
        bool buildTasksRemaining();
        bool probeTasksRemaining();
        void stop();
        void saveJoinedRelationToFile();

    private:
        std::vector<std::thread> threads;
        std::mutex queue_mutex;                  // Prevents data races to the job queue
        std::mutex timer_mutex;                  // Prevents data races to the job queue
        std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination
};

#endif