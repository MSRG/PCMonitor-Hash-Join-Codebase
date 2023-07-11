#ifndef POOL_HASHJOIN_PCM_PCM_MONITOR_H
#define POOL_HASHJOIN_PCM_PCM_MONITOR_H

#include "cpucounters.h" /* intel perf counters monitor */

class PcmMonitor {

    public:
        PcmMonitor(int joinCores_, int totalCores_);
        void setUpMonitoring();
        void runMonitoring();
        void checkpointPerformanceCounters();
        void startMonitorThread();
        void joinMonitorThread();
        void setMonitoringToFalse();
        void stopMonitoring();

    private:
        bool monitoring;
        int joinCores;
        int totalCores;
        std::vector<std::thread> pcmThreads;
};

#endif