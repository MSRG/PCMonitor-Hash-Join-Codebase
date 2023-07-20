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

    public:
        Relation *relR, *relS;
        Hashtable *ht;
        JoinResults *joinResults;
        SafeQueue *buildQ;
        SafeQueue *probeQ;
        Timestamps ts;
        struct timeval startTime;
        int numThreads, taskSize, currentTupIndex, phase;
        PcmMonitor *pcmMonitor;

        ThreadPool(int numThreads, Relation &relR_, Relation &relS_, Hashtable &ht_, int taskSize_, SafeQueue &buildQ_, SafeQueue &probeQ_, PcmMonitor &pcmMonitor_);
        void testFunction(int a, int &c);
        void start();
        void saveTimingResults();
        bool checkThreadStatusDuringBuild(ThreadArg &args);
        bool checkThreadStatusDuringProbe(ThreadArg &args);
        void buildQueue();
        void readQueue();
        void run(ThreadArg * param);
        void stop();
        void saveJoinedRelationToFile();

    private:
        std::vector<std::thread> threads;
        std::mutex queue_mutex;                  // Prevents data races to the job queue
        std::mutex timer_mutex;                  // Prevents data races to the job queue
        std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination
};

#endif