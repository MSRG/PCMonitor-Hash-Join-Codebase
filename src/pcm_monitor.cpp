#include <iostream>
#include <fstream>
#include <iostream>
#include <ctime>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "pcm_monitor.h"
//#include "cpu_mapping.h"        /* get_cpu_id */
#include "config.h"             /* ENABLE_CORE_PAUSING */

const char * CACHE_PATH = "cache-results.csv";
const char * IPC_CSV = "IPC-results.csv";
const char * MB_CSV = "MB-results.csv";
const char * THREAD_RESULTS_CSV_COPY = "individual-thread-results.csv"; // Defined in thread-pool.cpp

using namespace pcm;  

static PCM * pcmInstance;
static PCM::CustomCoreEventDescription MyEvents[4];
static CoreCounterState coreBeforeState[15];
static CoreCounterState coreAfterState[15];

PcmMonitor::PcmMonitor(int totalCores_, bool corePausing_, char * path_, int id_) {
//    std::cout << "Initializing PCM Monitor." << std::endl;
//    std::cout << "TOTAL CORES = " << totalCores << std::endl;

    monitoring = false;
    totalCores = totalCores_;
    corePausing = corePausing_;
    memBandwidthFlag = false;
    firstCheckpointDone = false;
    id = id_;

    std::cout << "hello i am ID number " << id << std::endl;

    this->path = new char[strlen(path_)+1];
    strcpy(this->path, path_);

    for (int i = 0; i < totalCores; i++) {
        threadStop[i] = false;
        threadStrikes[i] = 0;
        l2CacheStats[i].first = 0;
        l2CacheStats[i].second = 0;
        ipcStats[i].first = 0;
        ipcStats[i].second = 0;
        lmbStats[i].first = 0;
        lmbStats[i].second = 0;
        rmbStats[i].first = 0;
        rmbStats[i].second = 0;
    }
}

PcmMonitor::~PcmMonitor(void) {
//  std::cout << "PcmMonitor is being deleted" << std::endl;
   free(this->path);
}

void PcmMonitor::setUpMonitoring() {

    uint32 core = 0;
    PCM * m = PCM::getInstance();
    PCM::ErrorCode status;
    pcmInstance = m;

    if (pcmInstance->good()) {
        status = pcmInstance->program(PCM::DEFAULT_EVENTS, MyEvents);
        std::cout << "pcmInstance is good." << std::endl;

    } else {
        status = pcmInstance->program();
        std::cout << "pcmInstance->good() FAILED:" << status << std::endl;
    }

     for (int i = 0; i < totalCores; i++) {
        coreBeforeState[i] = getCoreCounterState(core);
        core++;
    }
    monitoring = true;
//    createResultsFolder();
//    clearCsvFiles();
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
    int maxStrikesTolerance = 10;

    // NOTE: core 0 is not allowed to stop.
    // HERE: set which cores are allowed to stop at all

    if (id == 1) { // pause: 1, 2, 3, 4
         for (int i = 1; i < 5; i++) {
             if (threadStrikes[i] > maxStrikesTolerance) {
                 threadStop[i] = true;
             } else if (threadStrikes[i] <= 0) {
                 threadStop[i] = false;
                 cv[i].notify_one(); // in case it is waiting.
             }
         }
    }

    if (id == 2) { // pause: 5, 6, 7, 8
         for (int i = 5; i < 9; i++) {
             if (threadStrikes[i] > maxStrikesTolerance) {
                 threadStop[i] = true;
             } else if (threadStrikes[i] <= 0) {
                 threadStop[i] = false;
                 cv[i].notify_one(); // in case it is waiting.
             }
         }
    }

    if (id == 3) { // pause: 9, 10, 11, 12
         for (int i = 9; i < 13; i++) {
             if (threadStrikes[i] > maxStrikesTolerance) {
                 threadStop[i] = true;
             } else if (threadStrikes[i] <= 0) {
                 threadStop[i] = false;
                 cv[i].notify_one(); // in case it is waiting.
             }
         }
    }

//    for (int i = 10; i < 14; i++) {
//        if (threadStrikes[i] > maxStrikesTolerance) {
//            threadStop[i] = true;
//        } else if (threadStrikes[i] <= 0) {
//            threadStop[i] = false;
//            cv[i].notify_one(); // in case it is waiting.
//        }
//    }
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
    double ipcThreshold = 0.4;

    for (int i = 0; i < totalCores; i++) {

        if (ipcStats[i].second > ipcThreshold) {
            threadStrikes[i] += 1;
        } else if (ipcStats[i].second <= ipcThreshold) {
            threadStrikes[i] -= 1;


        // CASE A: Difference between previous and current value exceed max allowable diff.
        // Add a strike to each core with a high diff.
        // TODO: could alternatively just stop the core.
//        if ((l2CacheStats[i].second - l2CacheStats[i].first) > maxDiff) {
//            threadStrikes[i] += 1;
//            if (threadStrikes[i] == maxStrikes) {
//                threadStop[i] = false; // TODO: change to true.
//            }
//        }

        // CASE B: Current value is higher than pre-defined allowable threshold.
        // Stop the core(s) exceeding the threshold value.
//        // TODO: could add a strike.
//        if (l2CacheStats[i].second > threshold) {
//            threadStrikes[i] += 1;
//        } else {
//            threadStrikes[i] -= 1;
//        }

        // CASE C: Identify which core is performing the worst among all cores.
//        if (l2CacheStats[i].second > worstValue) {
//            worstValue = l2CacheStats[i].second;
//            worstCore = i;
//        }
        }
    }
}


/*
 * Thread/core 15.
*/
void PcmMonitor::runMonitoring() {
    while (monitoring) {
        checkpointPerformanceCounters();
        analyzeCacheStats();
        if (corePausing) { makeStopDecisions(); }
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
//    memBandwidthFlag = false;
//    saveMemoryBandwidthValues();
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


void PcmMonitor::createResultsFolder() {
    char * path;

    // get datetime and convert to string.
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y--%H:%M:%S");
    auto datetimeStr = oss.str();
    const char * datetime = datetimeStr.c_str();

    // put path together.
    strcpy(path, "../results/");
    strcat(path, datetime);

    // make new directory named after datetime in results folder.
    int check = mkdir(path, 0777);
    // check if directory is created or not
    if (check) {
        printf("Unable to create directory\n");
        exit(1);
    }

    // save the new path as class member.
    const char * tmp = "/";
    strcat(path, tmp);
    this->path = new char[strlen(path)+1];
    strcpy(this->path, path);
}


/*
 * Clear the cvs file(s) used to save performance counter data.
 */
void PcmMonitor::clearCsvFiles() {

    std::ofstream file;

    // Cache file.
    file.open(CACHE_PATH, std::ofstream::out | std::ofstream::trunc);
    file.close();

    // IPC file.
    file.open(IPC_CSV, std::ofstream::out | std::ofstream::trunc);
    file.close();

    // Individual Thread Results file.
    file.open(THREAD_RESULTS_CSV_COPY, std::ofstream::out | std::ofstream::trunc);
    file.close();

    // Memory Bandwidth Results file.
    file.open(MB_CSV, std::ofstream::out | std::ofstream::trunc);
    file.close();
}

/*
 * Saves cache performance counter values into both file and monitor data structure.
 */
void PcmMonitor::saveCacheValues() {

    std::ofstream file(this->path + std::string(CACHE_PATH), std::ios_base::app);

    for (int i = 0; i < totalCores; i++) {
        if (!i == 0) { file << ","; }
        int misses = getL2CacheMisses(coreBeforeState[i], coreAfterState[i]);
        file << misses;
        l2CacheStats[i].first = l2CacheStats[i].second;
        l2CacheStats[i].second = misses;
    }
    file << "\n";
    file.close();
}

void PcmMonitor::saveIpcValues() {

    std::ofstream file(this->path + std::string(IPC_CSV), std::ios_base::app);

    for (int i = 0; i < totalCores; i++) {
        if (!i == 0) { file << ","; }
        double ipc = getIPC(coreBeforeState[i], coreAfterState[i]);
        file << ipc;
        ipcStats[i].first = ipcStats[i].second;
        ipcStats[i].second = ipc;
    }
    file << "\n";
    file.close();
}

void PcmMonitor::saveMemoryBandwidthValues() {

//    if (!memBandwidthFlag) {
        std::ofstream file(this->path + std::string(MB_CSV), std::ios_base::app);

        for (int i = 0; i < totalCores; i++) {
            if (!i == 0) { file << ","; }
            double localMemBdwth = getLocalMemoryBW(coreBeforeState[i], coreAfterState[i]);
            file << localMemBdwth;
            file << ",";
            double remoteMemBdwth = getRemoteMemoryBW(coreBeforeState[i], coreAfterState[i]);
            file << remoteMemBdwth;

            lmbStats[i].first = lmbStats[i].second;
            lmbStats[i].second = localMemBdwth;

            rmbStats[i].first = rmbStats[i].second;
            rmbStats[i].second = remoteMemBdwth;
        }
        file << "\n";
        file.close();
//        memBandwidthFlag = true;
//    }
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

    if (firstCheckpointDone) {  // ensures that at least one checkpoint is done so that there is a before and after val.

        saveCacheValues();
        saveIpcValues();
        saveMemoryBandwidthValues();
    }
    core = 0;

    for (int i = 0; i < totalCores; i++) {
        coreBeforeState[i] = getCoreCounterState(core);
        core++;
    }
    firstCheckpointDone = true;
}


void PcmMonitor::startMonitorThread() {

    int rv, cpu_idx;
    cpu_set_t set;                  // Linux struct representing set of CPUs.
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // THREAD #1 (ID = 54), COLLECT STATISTICS.
    cpu_idx = 15;
//    cpu_idx = get_cpu_id(15);
    CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
    CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.

    pcmThreads.emplace_back(&PcmMonitor::runMonitoring, this);
    rv = pthread_setaffinity_np(pcmThreads.back().native_handle(), sizeof(cpu_set_t), &set);
    if (rv != 0) {
      std::cerr << "Error calling pthread_setaffinity_np: " << rv << "\n";
    }

    // THREAD #2 (ID = 15), ANALYZE STATISTICS.
//    cpu_idx = get_cpu_id(15);
//    CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
//    CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.
//
//    pcmThreads.emplace_back(&PcmMonitor::runAnalyzing, this);
//    rv = pthread_setaffinity_np(pcmThreads.back().native_handle(), sizeof(cpu_set_t), &set);
//    if (rv != 0) {
//      std::cerr << "Error calling pthread_setaffinity_np: " << rv << "\n";
//    }
}

void PcmMonitor::joinMonitorThread() {
//    allowAllThreadsToContinue();
    for (std::thread & t : pcmThreads) {
        t.join();
    }
}

void PcmMonitor::setMonitoringToFalse() {
    monitoring = false;
}

void PcmMonitor::stopMonitoring() {
    joinMonitorThread();
    pcmInstance->cleanup();
}