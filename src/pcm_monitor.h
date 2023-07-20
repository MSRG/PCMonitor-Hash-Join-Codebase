#ifndef POOL_HASHJOIN_PCM_PCM_MONITOR_H
#define POOL_HASHJOIN_PCM_PCM_MONITOR_H

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
        void joinMonitorThread();
        void setMonitoringToFalse();
        void stopMonitoring();

    private:
        std::pair<int,int> ipcStats[14];
        std::pair<int,int> l3CacheStats[14];
        int threadStrikes[14];
        int test;
        bool threadStop[14];
        bool monitoring;
        int totalCores;
        std::vector<std::thread> pcmThreads;
};

#endif