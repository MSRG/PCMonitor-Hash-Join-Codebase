#ifndef POOL_HASHJOIN_PCM_THREAD_POOL_H
#define POOL_HASHJOIN_PCM_THREAD_POOL_H

#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>

#include "types.h"
#include "pcm_monitor.h"
//#include "safe_queue.h"
#include "fine_grained_queue.h"

class ThreadPool {

    public:
        Relation *relR, *relS;
        GlobalHashTable *ht;
        JoinResults *joinResults;
        FineGrainedQueue *buildQ;
        FineGrainedQueue *probeQ;
        Timestamps ts;
        struct timeval startTime;
        int numThreads, taskSize;
        bool skipBuild;
//        int currentTupIndex, phase;
        PcmMonitor *pcmMonitor;
        ThreadPool(int numThreads, Relation &relR_, Relation &relS_, GlobalHashTable &ht_, int taskSize_, FineGrainedQueue &buildQ_, FineGrainedQueue &probeQ_, PcmMonitor &pcmMonitor_, char* path_, int id_, bool skipBuild_);
        void start();
        void saveTimingResults();
        void freeThreadsIfBuildQueueEmpty();
        void freeThreadsIfProbeQueueEmpty();
        bool checkThreadStatusDuringBuild(ThreadArg &args);
        bool checkThreadStatusDuringProbe(ThreadArg &args);
        void populateQueues();
        void readQueue();
        void run(ThreadArg * param);
        void stop();
        void saveJoinedRelationToFile();
        void saveIndividualThreadResults(ThreadArg &args);

    private:
        int id;
        char *path;
        std::vector<std::thread> threads;
        std::mutex queue_mutex;                  // Prevents data races to the job queue
        std::mutex timer_mutex;                  // Prevents data races to the job queue
        std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination
};

#endif