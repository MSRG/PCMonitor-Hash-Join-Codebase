#ifndef POOL_HASHJOIN_PCM_PCM_MONITOR_H
#define POOL_HASHJOIN_PCM_PCM_MONITOR_H

#include <mutex>
#include <thread>
#include <condition_variable>
#include "cpucounters.h"    /* intel perf counters monitor */

class PcmMonitor {

    bool firstCheckpointDone;

    public:
        PcmMonitor(int totalCores_, bool corePausing_, char* path_, int id);
        ~PcmMonitor(void);
        void setUpMonitoring();
        void allowAllThreadsToContinue();
        void makeStopDecisions();
        void createResultsFolder();
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
        std::condition_variable cv[15];
        std::mutex mutx[15];

    private:
        int id;
        bool memBandwidthFlag;
        char *path;
        std::pair<double,double> ipcStats[15];
        std::pair<int,int> l2CacheStats[15];
        std::pair<double,double> rmbStats[15];
        std::pair<double,double> lmbStats[15];
        int threadStrikes[15];
        bool threadStop[15];
        bool monitoring;
        int totalCores;
        bool corePausing;
        std::vector<std::thread> pcmThreads;
};

#endif