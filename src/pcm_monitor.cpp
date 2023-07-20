//#include <sstream>
#include <iostream>
#include <fstream>

#include "pcm_monitor.h"
#include "cpu_mapping.h"        /* get_cpu_id */

const char * CACHE_CSV = "cache-results.csv";

using namespace pcm;

static PCM * pcmInstance;
static PCM::CustomCoreEventDescription MyEvents[4];
static CoreCounterState coreBeforeState[14];
static CoreCounterState coreAfterState[14];

pthread_mutex_t monitor_lock;

PcmMonitor::PcmMonitor(int totalCores_) {
    std::cout << "Initializing PCM Monitor." << std::endl;
    totalCores = totalCores_;
    monitoring = false;
    test = 7;

    for (int i = 0; i <= totalCores; i++) {
        threadStop[i] = false;
        threadStrikes[i] = 0;
        l3CacheStats[i].first = 0;
        l3CacheStats[i].second = 0;
    }
}

void PcmMonitor::setUpMonitoring() {

    uint32 core = 0;
    PCM * m = PCM::getInstance();
    PCM::ErrorCode status;
    pcmInstance = m;

    if (pcmInstance->good()) {
        std::cout << "pcmInstance is good." << std::endl;
        status = pcmInstance->program(PCM::DEFAULT_EVENTS, MyEvents);

    } else {
        status = pcmInstance->program();
        std::cout << "pcmInstance->good() FAILED:" << status << std::endl;
    }

     for (int i = 0; i < totalCores; i++) {
        coreBeforeState[i] = getCoreCounterState(core);
        core++;
    }
    monitoring = true;
    clearCsvFiles();
}

void PcmMonitor::allowAllThreadsToContinue() {
    for (int i = 0; i < totalCores; i++) {
        threadStop[i] = false;
    }
}

bool PcmMonitor::shouldThreadStop(int id) {
    return threadStop[id];
}


void PcmMonitor::makeStopDecisions() {
    int maxStrikesTolerance = 5;
    int maxStrikes = 0;
    int worstCore = 0;

    for (int i = 0; i < totalCores; i++) {
        // Find which thread has the most strikes currently.
        if (threadStrikes[i] > maxStrikes) {
            maxStrikes = threadStrikes[i];
            worstCore = i;
        }
        // Every thread that has less than tolerance strikes is allowed to run.
        if (threadStrikes[i] < maxStrikesTolerance) {
            threadStop[i] = false;
        }
    }

    // If the thread with the most strikes currently exceeds tolerance, stop it.
    if (maxStrikes >= maxStrikesTolerance) {
        threadStop[worstCore] = true;
    }

}

/*
 * Determine core status based on saved cache stats.
 *
 */
void PcmMonitor::analyzeCacheStats() {
    // possible actions: stop thread, start thread

    int maxDiff = 1;
    int threshold = 1000;
    int worstValue = 0;
    int worstCore = 0;
    int maxStrikes = 5;

    for (int i = 0; i < totalCores; i++) {

        // CASE A: Difference between previous and current value exceed max allowable diff.
        // Add a strike to each core with a high diff.
        // TODO: could alternatively just stop the core.
//        if ((l3CacheStats[i].second - l3CacheStats[i].first) > maxDiff) {
//            threadStrikes[i] += 1;
//            if (threadStrikes[i] == maxStrikes) {
//                threadStop[i] = false; // TODO: change to true.
//            }
//        }

        // CASE B: Current value is higher than pre-defined allowable threshold.
        // Stop the core(s) exceeding the threshold value.
        // TODO: could add a strike.
        if (l3CacheStats[i].second > threshold) {
            threadStrikes[i] += 1;
        } else {
            threadStrikes[i] -= 1;
        }

        // CASE C: Identify which core is performing the worst among all cores.
//        if (l3CacheStats[i].second > worstValue) {
//            worstValue = l3CacheStats[i].second;
//            worstCore = i;
//        }
    }
}

/*
 * Thread/core 14.
*/
void PcmMonitor::runMonitoring() {

    while (monitoring) {
        checkpointPerformanceCounters();
        analyzeCacheStats();
        makeStopDecisions();
        sleep(0.5);

//        std::cout << "monitoring" << std::endl;
    }
//    std::cout << "stopped monitoring" << std::endl;
}

/*
 * Thread/core 15.
*/
void PcmMonitor::runAnalyzing() {
    while (monitoring) {
//        analyzeCacheStats();
//        makeStopDecisions();
    }
}

/*
 * Clear the cvs file(s) used to save performance counter data.
 */
void PcmMonitor::clearCsvFiles() {
    std::ofstream file;
    file.open(CACHE_CSV, std::ofstream::out | std::ofstream::trunc);
    file.close();
}

/*
 * Saves cache performance counter values into both file and monitor data structure.
 */
void PcmMonitor::saveCacheValues() {
    uint32 core = 0;

    if (CACHE_CSV) {
        std::fstream file(CACHE_CSV, std::ios::app);

        for (int i = 0; i < totalCores; i++) {
            if (!i == 0) { file << ","; }
            int misses = getL2CacheMisses(coreBeforeState[i], coreAfterState[i]);
            file << misses;
            l3CacheStats[i].first = l3CacheStats[i].second;
            l3CacheStats[i].second = misses;
        }
        file << "\n";
        file.close();
    }
}


/*
 * Update the before and after counter states.
 *
 */
void PcmMonitor::checkpointPerformanceCounters() {

    uint32 core = 0;

    for (int i = 0; i < totalCores; i++) {
        coreAfterState[i] = getCoreCounterState(core);
        core++;
    }

    saveCacheValues();
    core = 0;

    for (int i = 0; i < totalCores; i++) {
        coreBeforeState[i] = getCoreCounterState(core);
        core++;
    }
}


void PcmMonitor::startMonitorThread() {

    int rv, cpu_idx;
    cpu_set_t set;                  // Linux struct representing set of CPUs.
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // THREAD #1 (ID = 14), COLLECT STATISTICS.
    cpu_idx = get_cpu_id(14);
    CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
    CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.

    pcmThreads.emplace_back(&PcmMonitor::runMonitoring, this);
    rv = pthread_setaffinity_np(pcmThreads.back().native_handle(), sizeof(cpu_set_t), &set);
    if (rv != 0) {
      std::cerr << "Error calling pthread_setaffinity_np: " << rv << "\n";
    }

    // THREAD #2 (ID = 15), ANALYZE STATISTICS.
    cpu_idx = get_cpu_id(15);
    CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
    CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.

    pcmThreads.emplace_back(&PcmMonitor::runAnalyzing, this);
    rv = pthread_setaffinity_np(pcmThreads.back().native_handle(), sizeof(cpu_set_t), &set);
    if (rv != 0) {
      std::cerr << "Error calling pthread_setaffinity_np: " << rv << "\n";
    }
}

void PcmMonitor::joinMonitorThread() {
    allowAllThreadsToContinue();

    std::cout << "joining monitoring threads." << std::endl;
    for (std::thread & t : pcmThreads) {
        t.join();
    }
}

void PcmMonitor::setMonitoringToFalse() {
    monitoring = false;
}

void PcmMonitor::stopMonitoring() {
    joinMonitorThread();

//    uint32 core = 0;
//    for (int i = 0; i < totalCores; i++) {
//        coreAfterState[i] = getCoreCounterState(core);
//        core++;
//    }
//    saveCacheValues();

    pcmInstance->cleanup();
}