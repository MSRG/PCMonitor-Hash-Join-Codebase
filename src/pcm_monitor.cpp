
#include "pcm_monitor.h"
#include "cpu_mapping.h"        /* get_cpu_id */

using namespace pcm;

static PCM * pcmInstance;
//static int totalCores = 16;
static int ipcState[16];
static CoreCounterState coreBeforeState[16];
static CoreCounterState coreAfterState[16];

PcmMonitor::PcmMonitor(int joinCores_,  int totalCores_) {
    joinCores = joinCores_;
    totalCores = totalCores_;
    monitoring = false;
}

void PcmMonitor::setUpMonitoring() {

    uint32 core = 0;
    PCM * m = PCM::getInstance();
    PCM::ErrorCode status;
    pcmInstance = m;

    if (pcmInstance->good()) {
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
}

void PcmMonitor::checkpointPerformanceCounters() {

    uint32 core = 0;

    for (int i = 0; i < totalCores; i++) {
        coreAfterState[i] = getCoreCounterState(core);
        core++;
    }

}

void PcmMonitor::runMonitoring() {
    uint32 core = 0;

    while (monitoring) {
        core = 0;
//        for (int i = 0; i < totalCores; i++) {
//            std::cout << "FOR LOOP: " << i << std::endl;
//            coreAfterState[i] = getCoreCounterState(core);
//            core++;
//        }
        checkpointPerformanceCounters();
//        std::cout << "monitoring...\n" << std::endl;
    }
    std::cout << "done monitoring!\n" << std::endl;
//    stopMonitoring();
}

void PcmMonitor::startMonitorThread() {
    cpu_set_t set;             // Linux struct representing set of CPUs.
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    int cpu_idx = get_cpu_id(14);
    std::cout << "CPU ID for monitor: " << cpu_idx << std::endl;
    CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
    CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.

    pcmThreads.emplace_back(&PcmMonitor::runMonitoring, this);
    int rv = pthread_setaffinity_np(pcmThreads.back().native_handle(), sizeof(cpu_set_t), &set);
    if (rv != 0) {
      std::cerr << "Error calling pthread_setaffinity_np: " << rv << "\n";
    }

}

void PcmMonitor::joinMonitorThread() {
    for (std::thread & t : pcmThreads) {

        t.join();
    }
    std::cout << "Join Monitor thread" << std::endl;
}

void PcmMonitor::setMonitoringToFalse() {
    std::cout << "set monitoring to false" << std::endl;
    monitoring = false;
}

void PcmMonitor::stopMonitoring() {
    joinMonitorThread();

//    uint32 core = 0;
//
//    for (int i = 0; i < totalCores; i++) {
//        coreAfterState[i] = getCoreCounterState(core);
//        core++;
//    }

    pcmInstance->cleanup();
}