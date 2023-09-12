#ifndef POOL_HASHJOIN_PCM_PCM_MONITOR_H
#define POOL_HASHJOIN_PCM_PCM_MONITOR_H

#include <mutex>
#include <thread>
#include <condition_variable>
#include "cpucounters.h"    /* intel perf counters monitor */

class PcmMonitor {

    public:
        PcmMonitor(int totalCores_);
        void setUpMonitoring();
        void allowAllThreadsToContinue();
        void makeStopDecisions();
        void clearCsvFiles();
        bool shouldThreadStop(int id);
        void runMonitoring();
        void analyzeCacheStats();
        void runAnalyzing();
        void checkpointPerformanceCounters();
        void startMonitorThread();
        void saveCacheValues();
        void saveIpcValues();
        void saveMemoryBandwidthValues();
        void joinMonitorThread();
        void setMonitoringToFalse();
        void stopMonitoring();
        std::condition_variable cv[14];
        std::mutex mutx[14];

    private:
        std::pair<double,double> ipcStats[14];
        std::pair<int,int> l2CacheStats[14];
        std::pair<double,double> rmbStats[14];
        std::pair<double,double> lmbStats[14];
        int threadStrikes[14];
        int test;
        bool threadStop[14];
        bool monitoring;
        int totalCores;
        std::vector<std::thread> pcmThreads;
};

#endif