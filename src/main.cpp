//
// Created by Sofia Tijanic on 2023-06-19.
//

// Libraries:
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <iostream>
#include <getopt.h>
#include <cmath>
#include <unistd.h>
#include <mutex>

#include "sys/types.h"
#include "sys/sysinfo.h"

// Files:
#include "types.h"
#include "config.h"
#include "join.h"
#include "thread_pool.h"
#include "relation_generator.h"
//#include "safe_queue.h"
#include "fine_grained_queue.h"
//#include "global_hash_table.h"
using namespace std;

std::mutex globalHtMutex; // Global mutex for global hash table.
std::mutex globalMemMutex; // Global mutex for global hash table.
long long usedMem;

/**
 * Data structure for command line arguments.
*/
struct CmdParams {
    uint64_t rSize;
    uint64_t sSize;
    uint64_t totalCores;
    uint64_t coresToMonitor;
    uint64_t taskSize;
    bool corePausing;   // Allow cores to stop depending on performance counter info.
    bool programPMU;    // Program the PMU.
    bool shareHt;       // Share global hash table among threads.
    int  hjThreads;     // Number of hash join kernel threads.
    int skew;
    int id;
};

void print_help();

void parse_args(int argc, char **argv, CmdParams * cmdParams);

// ***********************************************************************************************************
//                                   Physical Memory Used
// ***********************************************************************************************************

// in GB
void updateUsedMemoryCustom(long long memUpdate) {
    unique_lock<mutex> lock(globalMemMutex);
    usedMem += memUpdate;
    lock.unlock();
}

// in GB
int getUsedMemoryCustom() {
    return usedMem;
}

// in GB
bool isMemAvailable(int memUpdate) {
    unique_lock<mutex> lock(globalMemMutex);

    if ((usedMem + memUpdate) > 500) { return false; }
    else {
        usedMem += memUpdate;
        return true;
    }
    lock.unlock();
}

long long getUsedMemory(int id, int checkpoint) {

    struct sysinfo memInfo;

    sysinfo (&memInfo);
    long long totalVirtualMem = memInfo.totalram;
    //Add other values in next statement to avoid int overflow on right hand side...
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;

    long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
    //Add other values in next statement to avoid int overflow on right hand side...
    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;

    long long physMemUsed = memInfo.totalram - memInfo.freeram;
    //Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;

//    std::cout << "ID: " << id << ", checkpoint: " << checkpoint << " ==> Physical memory currently being used: " << physMemUsed << std::endl;

    return physMemUsed;
}


int parseLine(char* line) {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}


int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}


// ***********************************************************************************************************
//                                   Create relations in parallel
// ***********************************************************************************************************

void create_relations(Relation &relR, Relation &relS, uint64_t rSize, uint64_t sSize, int skew, int taskSize) {

    RelCreationThreadArg relThreadArgs[2];
    std::vector<std::thread> threads;

    cpu_set_t set;                  // Linux struct representing set of CPUs.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_t tid[2];

// *** RELATION R ***
    fprintf(stdout,
            "\n[INFO] Creating relation R:\n-- Number of tuples = %lu,\n-- %.3lf mebibytes,\n-- %.3lf bytes,\n-- %.3lf GB\n\n",
            (unsigned long)rSize,
            (double) sizeof(Tuple) * rSize/1024.0/1024.0,
            (double) sizeof(Tuple) * rSize,
            (double) sizeof(Tuple) * rSize/1000000000
            );

    int cpu_idx = 6;
    CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
    CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.

    relThreadArgs[0].relSize = rSize;
    relThreadArgs[0].relation = &relR;

    pthread_create(&tid[0], &attr, create_relation_R, (void*)&relThreadArgs[0]);

// *** RELATION S ***
    fprintf(stdout,
            "\n[INFO] Creating relation S:\n-- Number of tuples = %lu,\n-- %.3lf mebibytes,\n-- %.3lf bytes,\n-- %.3lf GB\n\n",
            (unsigned long)sSize,
            (double) sizeof(Tuple) * sSize/1024.0/1024.0,
            (double) sizeof(Tuple) * sSize,
            (double) sizeof(Tuple) * sSize/1000000000
            );

   cpu_idx = 7;
   CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
   CPU_SET(cpu_idx, &set);         // Add CPU cpu to set.

    relThreadArgs[1].relSize = sSize;
    relThreadArgs[1].relation = &relS;
    relThreadArgs[1].skew = skew;
    relThreadArgs[1].taskSize = taskSize;

    pthread_create(&tid[1], &attr, create_relation_S, (void*)&relThreadArgs[1]);

    for (int i = 0; i < 2; i++)
       pthread_join(tid[i], NULL);
}


// ***********************************************************************************************************
//                                   Thread-based hash join
// ***********************************************************************************************************

void getGlobalHt(GlobalHashTable * globalHt, int numBuckets) {

    unique_lock<mutex> lock(globalHtMutex);

    // Hash table exists and is ready to use!
    if (globalHt->ready) { return; }

    // Not built, and no one is building it, so I will build it.
    if (!globalHt->ready && !globalHt->beingBuilt) {
        globalHt->beingBuilt = true;
        allocate_hashtable(&globalHt->ht, numBuckets);
        return;
    }

    // Someone else is building the hash table, it is not ready.
    if (globalHt->beingBuilt && !globalHt->ready) {
        lock.unlock();
        // Wait until the hash table is ready.
        while (!globalHt->ready) {}
        return;
    }
}


void threaded_hash_join(HashJoinThreadArg * args) {

    int id                      = args->tid;
    uint64_t rSize              = args->rSize;
    uint64_t sSize              = args->sSize;
    uint64_t totalCores         = args->totalCores;
    uint64_t taskSize           = args->taskSize;
    uint64_t coresToMonitor     = args->coresToMonitor;
    bool corePausing            = args->corePausing;
    bool programPMU             = args->programPMU;
    GlobalHashTable *globalHt   = args->globalht;
    int skew                    = args->skew;
    bool shareHt                = args->shareHt;

    int memRequiredGB = 0;
    bool skipBuild = false;
    long long usedMem, memRequired, memAvailable;
    double numOfBuildTasks, numOfProbeTasks;
    Relation relR;
    Relation relS;

    numOfBuildTasks = ceil(double(rSize) / double(taskSize));
    numOfProbeTasks = ceil(double(sSize) / double(taskSize));

    FineGrainedQueue buildQ(numOfBuildTasks);
    FineGrainedQueue probeQ(numOfProbeTasks);

    fprintf(stdout,
            "\n[INFO] %lu cores being monitored:\n-- Task size = %lu,\n-- Number of build tasks = %f,\n-- Number of probe tasks = %f,\n-- corePausing = %i.\n",
            totalCores,
            taskSize,
            numOfBuildTasks,
            numOfProbeTasks,
            corePausing
            );

    memRequired = ((double) sizeof(Tuple) * sSize) + ((double) sizeof(Tuple) * rSize) + ((rSize / BUCKET_SIZE) * sizeof(Bucket));
    memRequiredGB = memRequired/1000000000;
    memAvailable = 500 - getUsedMemoryCustom(); // 500 GB available.
    std::cout << "Required memory for this hash join = " << memRequiredGB << " GB." << std::endl;
    std::cout << "Available memory for this hash join = " << memAvailable << " GB." << std::endl;

#if MONITOR_MEMORY==1 // -------------------- MEMORY USE MONITORING -------------------------
    while (!isMemAvailable(memRequiredGB)) { }
#endif  // ----------------------------------------------------------------------------------

    create_relations(relR, relS, rSize, sSize, skew, taskSize);

    // Create results path
    char * path;
    const char * resultsDir = "../results/";
    // get datetime and convert to string.
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y-");
    oss << id;
    auto datetimeStr = oss.str();
    const char * datetime = datetimeStr.c_str();
    // put path together.
    path = new char[strlen(resultsDir) + strlen(datetime) + 1];
    strcpy(path, resultsDir);
    strcat(path, datetime);
    // make new directory named after datetime in results folder.
    int check = mkdir(path, 0777);
    if (check) { // check if directory is created or not
        printf("Unable to create directory for saving results.\n");
        exit(1);
    }
    // Update path to be used by other functions to save files.
    const char * tmp = "/";
    strcat(path, tmp);


    printf("[INFO] Initializing PCM Monitor...\n");
    PcmMonitor pcmMonitor(totalCores, coresToMonitor, corePausing, path, id);

#if USE_PCM==1
    pcmMonitor.setUpMonitoring();
#endif

    // -------------------- Share hash table -------------------------
    if (shareHt) {
        // Get the global hash table.
        uint64_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
        getGlobalHt(globalHt, numBuckets);

        printf("[INFO] Starting Monitoring...\n");
        if (programPMU) { pcmMonitor.startMonitorThread(); }

        printf("[INFO] Initializing ThreadPool...\n");
        ThreadPool threadPool(totalCores, relR, relS, *globalHt, taskSize, buildQ, probeQ, pcmMonitor, path, id);

        threadPool.populateQueues();
        threadPool.start();

#if USE_PCM==1
        pcmMonitor.setMonitoringToFalse();
        pcmMonitor.stopMonitoring();
#endif

#if SAVE_RELATIONS_TO_FILE==1
            threadPool.saveJoinedRelationToFile();
#endif

        std::cout << "DONE. BYE!" << std::endl;

    // --------------- Each thread uses an individual hash table --------------------
    } else {

        GlobalHashTable privateHashTable;
        Hashtable * ht;
        privateHashTable.ht = ht;
        privateHashTable.ready = false;
        privateHashTable.beingBuilt = false;

        privateHashTable.beingBuilt = true;
        uint64_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
        allocate_hashtable(&privateHashTable.ht, numBuckets);

#if USE_PCM==1
        printf("[INFO] Starting Monitoring...\n");
        pcmMonitor.startMonitorThread();
#endif

        printf("[INFO] Initializing ThreadPool...\n");
        ThreadPool threadPool(totalCores, relR, relS, privateHashTable, taskSize, buildQ, probeQ, pcmMonitor, path, id);

        threadPool.populateQueues();
        threadPool.start();

#if USE_PCM==1
        pcmMonitor.setMonitoringToFalse();
        pcmMonitor.stopMonitoring();
#endif

#if SAVE_RELATIONS_TO_FILE==1
        threadPool.saveJoinedRelationToFile();
#endif
        std::cout << "DONE. BYE!" << std::endl;
    }
}


void threaded_hash_join_copy(HashJoinThreadArg * args) {

    int id                      = args->tid;
    uint64_t rSize              = args->rSize;
    uint64_t sSize              = args->sSize;
    uint64_t totalCores         = args->totalCores;
    uint64_t taskSize           = args->taskSize;
    uint64_t coresToMonitor     = args->coresToMonitor;
    bool corePausing            = args->corePausing;
    bool programPMU             = args->programPMU;
    GlobalHashTable *globalHt   = args->globalht;
    int skew                    = args->skew;
    bool shareHt                = args->shareHt;

    int memRequiredGB = 0;
    bool skipBuild = false;
    long long usedMem, memRequired, memAvailable;
    double numOfBuildTasks, numOfProbeTasks;
    Relation relR;
    Relation relS;

    numOfBuildTasks = ceil(double(rSize) / double(taskSize));
    numOfProbeTasks = ceil(double(sSize) / double(taskSize));

    FineGrainedQueue buildQ(numOfBuildTasks);
    FineGrainedQueue probeQ(numOfProbeTasks);

    fprintf(stdout,
            "\n[INFO] %lu cores being monitored:\n-- Task size = %lu,\n-- Number of build tasks = %f,\n-- Number of probe tasks = %f,\n-- corePausing = %i.\n",
            totalCores,
            taskSize,
            numOfBuildTasks,
            numOfProbeTasks,
            corePausing
            );

//    memRequired = ((double) sizeof(Tuple) * sSize) + ((double) sizeof(Tuple) * rSize) + ((rSize / BUCKET_SIZE) * sizeof(Bucket));
//    memRequiredGB = memRequired/1000000000;
//    memAvailable = 500 - getUsedMemoryCustom(); // 500 GB available.
//    std::cout << "Required memory for this hash join = " << memRequiredGB << " GB." << std::endl;
//    std::cout << "Available memory for this hash join = " << memAvailable << " GB." << std::endl;
//
//#if MONITOR_MEMORY==1 // -------------------- MEMORY USE MONITORING -------------------------
////    while ((400000000000 - getUsedMemory(id, 0)) < memRequired) { }
//    while (!isMemAvailable(memRequiredGB)) { }
//#endif  // ----------------------------------------------------------------------------------

    create_relations(relR, relS, rSize, sSize, skew, taskSize);

    // Create results path
    char * path;
    const char * resultsDir = "../results/";
    // get datetime and convert to string.
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y-");
//    oss << std::put_time(&tm, "%d-%m-%Y--%H:%M:%S--");
    oss << id;
    auto datetimeStr = oss.str();
    const char * datetime = datetimeStr.c_str();
    // put path together.
    path = new char[strlen(resultsDir) + strlen(datetime) + 1];
    strcpy(path, resultsDir);
    strcat(path, datetime);
    // make new directory named after datetime in results folder.
    int check = mkdir(path, 0777);
    if (check) { // check if directory is created or not
        printf("Unable to create directory for saving results.\n");
        exit(1);
    }
    // Update path to be used by other functions to save files.
    const char * tmp = "/";
    strcat(path, tmp);

    printf("[INFO] Initializing PCM Monitor...\n");
    PcmMonitor pcmMonitor(totalCores, coresToMonitor, corePausing, path, id);
#if USE_PCM==1
    pcmMonitor.setUpMonitoring();
#endif

    // -------------------- Share hash table -------------------------
//    if (shareHt) {
//        // Get the global hash table.
//        uint64_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
//        getGlobalHt(globalHt, numBuckets);
//
//        printf("[INFO] Starting Monitoring...\n");
//        if (programPMU) { pcmMonitor.startMonitorThread(); }
//
//        printf("[INFO] Initializing ThreadPool...\n");
//        ThreadPool threadPool(totalCores, relR, relS, *globalHt, taskSize, buildQ, probeQ, pcmMonitor, path, id);
//
//        threadPool.populateQueues();
//        threadPool.start();
//
//        if (programPMU) {
//            pcmMonitor.setMonitoringToFalse();
//            pcmMonitor.stopMonitoring();
//        }
//
//        #if SAVE_RELATIONS_TO_FILE==1
//            threadPool.saveJoinedRelationToFile();
//        #endif
//
//        std::cout << "free stuff.." << std::endl;
//    //    free(relR.tuples);
//    //    free(relS.tuples);
//    //    free(path);
//    //    deallocate_hashtable(*globalht->ht);
//        std::cout << "DONE! BYE!" << std::endl;
//
//    // --------------- Each thread uses an individual hash table --------------------
//    } else {
//
//        GlobalHashTable privateHashTable;
//        Hashtable * ht;
//        privateHashTable.ht = ht;
//        privateHashTable.ready = false;
//        privateHashTable.beingBuilt = false;
//
//        privateHashTable.beingBuilt = true;
//        uint64_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
//        allocate_hashtable(&privateHashTable.ht, numBuckets);
//
//        printf("[INFO] Starting Monitoring...\n");
//        if (programPMU) { pcmMonitor.startMonitorThread(); }
//
//        printf("[INFO] Initializing ThreadPool...\n");
//        ThreadPool threadPool(totalCores, relR, relS, privateHashTable, taskSize, buildQ, probeQ, pcmMonitor, path, id);
//
//        threadPool.populateQueues();
//        threadPool.start();
//
//        if (programPMU) {
//            pcmMonitor.setMonitoringToFalse();
//            pcmMonitor.stopMonitoring();
//        }
//
//        #if SAVE_RELATIONS_TO_FILE==1
//            threadPool.saveJoinedRelationToFile();
//        #endif
//
//        std::cout << "free stuff.." << std::endl;
//        //    free(relR.tuples);
//        //    free(relS.tuples);
//        //    free(path);
//        //    deallocate_hashtable(*globalht->ht);
//        std::cout << "DONE! BYE!" << std::endl;
//
//    }
}

/*
* Function for testing L2 cache.
* Function to perform computation on a 256KB data structure.
*/
void* performComputationOnLargeDataStructure(void* arg) {

    int threadID = *((int*)arg);

    // Set thread affinity to core 0
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
        std::cerr << "Error setting thread affinity for thread " << threadID << std::endl;
    }

    std::cout << "Thread " << threadID << " is executing on core 0." << std::endl;

   // Size of the data structure in bytes (256KB)
    const int sizeInBytes = 256 * 1024;

    // Allocate memory for the data structure (assuming it's an array of integers)
    int* dataStructure = new int[sizeInBytes / sizeof(int)];

    // Assuming the data structure is an array of integers
    int numElements = sizeInBytes / sizeof(int);

    // Initialize the array with random values for demonstration purposes
    for (int i = 0; i < numElements; ++i) {
        dataStructure[i] = rand() % 100; // Random values between 0 and 99
    }

    int sum = 0;
    for (int j = 0; j < 100000; j++) {
        sum = 0;
        // Perform computation (sum all elements in the array)
        for (int i = 0; i < numElements; ++i) {
            sum += dataStructure[i];
        }
    }

    // Print the result (you can modify this part based on your actual computation)
    std::cout << "Sum of elements in the data structure: " << sum << std::endl;

    delete[] dataStructure;
}


// ***********************************************************************************************************
//                                                Main
// ***********************************************************************************************************
int main(int argc, char **argv) {

  /* Command line parameters */
    CmdParams cmdParams;
    cmdParams.rSize             = 1000000;
    cmdParams.sSize             = 1000000;
    cmdParams.skew              = 0;
    cmdParams.totalCores        = 15;
    cmdParams.coresToMonitor    = 15;
    cmdParams.taskSize          = 10;
    cmdParams.corePausing       = false;
    cmdParams.programPMU        = true;
    cmdParams.id                = 0;
    cmdParams.hjThreads         = 0;
    cmdParams.shareHt           = false;
    parse_args(argc, argv, &cmdParams);

    // **************************** THREAD-BASED HASH JOINS ************************************************************
    if (cmdParams.hjThreads > 0) {
        usedMem = 0;
        GlobalHashTable globalHashTable;
        Hashtable ht;
        globalHashTable.ht = &ht;
        globalHashTable.ready = false;
        globalHashTable.beingBuilt = false;

        std::vector<std::thread> threads(cmdParams.hjThreads);
        HashJoinThreadArg args[cmdParams.hjThreads];
        pthread_t tid[cmdParams.hjThreads];

        cpu_set_t set;            // Linux struct representing set of CPUs.
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        for (int i = 0; i < cmdParams.hjThreads; i++) {
            CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
            CPU_SET(i, &set);               // Add CPU cpu to set.

            args[i].rSize           = cmdParams.rSize;
            args[i].sSize           = cmdParams.sSize;
            args[i].totalCores      = cmdParams.totalCores;
            args[i].coresToMonitor  = cmdParams.coresToMonitor;
            args[i].taskSize        = cmdParams.taskSize;
            args[i].corePausing     = cmdParams.corePausing;
            args[i].programPMU      = cmdParams.programPMU;
            args[i].skew            = cmdParams.skew;
            args[i].globalht        = &globalHashTable;
            args[i].shareHt         = cmdParams.shareHt;
            args[i].tid             = i;

            threads[i] = thread (threaded_hash_join, &args[i]);
        }
        for (auto& th : threads) { th.join(); }

        std::cout << "Joined threads, done!" << std::endl;
        return 0;

    } else {  // **************************** PROCESS-BASED HASH JOINS *************************************************

        long long usedMem, memRequired, memAvailable;
        double numOfBuildTasks, numOfProbeTasks;
        Relation relR;
        Relation relS;

        numOfBuildTasks = ceil(double(cmdParams.rSize) / double(cmdParams.taskSize));
        numOfProbeTasks = ceil(double(cmdParams.sSize) / double(cmdParams.taskSize));

        FineGrainedQueue buildQ(numOfBuildTasks);
        FineGrainedQueue probeQ(numOfProbeTasks);

        fprintf(stdout,
                "\n[INFO] %lu cores being monitored:\n-- Task size = %lu,\n-- Number of build tasks = %f,\n-- Number of probe tasks = %f,\n-- corePausing = %i.\n",
                cmdParams.totalCores,
                cmdParams.taskSize,
                numOfBuildTasks,
                numOfProbeTasks,
                cmdParams.corePausing
                );

        memRequired = ((double) sizeof(Tuple) * cmdParams.sSize) + ((double) sizeof(Tuple) * cmdParams.rSize) + ((cmdParams.rSize / BUCKET_SIZE) * sizeof(Bucket));
        memAvailable = 500000000000 - getUsedMemory(cmdParams.id, 0);
        std::cout << "Required memory for this hash join = " << memRequired/1000000000 << " GB." << std::endl;
        std::cout << "Available memory for this hash join = " << memAvailable/1000000000 << " GB." << std::endl;

#if MONITOR_MEMORY==1 // -------------------- MEMORY USE MONITORING ------------------------
        while ((400000000000 - getUsedMemory(cmdParams.id, 0)) < memRequired) { }
#endif // -----------------------------------------------------------------------------------

        create_relations(relR, relS, cmdParams.rSize, cmdParams.sSize, cmdParams.skew, cmdParams.taskSize);

        // Create results path
        char * path;
        const char * resultsDir = "../results/";
        // get datetime and convert to string.
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d-%m-%Y--");
        oss << cmdParams.id;
        auto datetimeStr = oss.str();
        const char * datetime = datetimeStr.c_str();
        // put path together.
        path = new char[strlen(resultsDir) + strlen(datetime) + 1];
        strcpy(path, resultsDir);
        strcat(path, datetime);
        // make new directory named after datetime in results folder.
        int check = mkdir(path, 0777);
        // check if directory is created or not
        if (check) {
            printf("Unable to create directory for saving results.\n");
            exit(1);
        }
        // Update path to be used by other functions to save files.
        const char * tmp = "/";
        strcat(path, tmp);

        PcmMonitor pcmMonitor(cmdParams.totalCores, cmdParams.coresToMonitor, cmdParams.corePausing, path, cmdParams.id);

#if USE_PCM==1
        printf("[INFO] Initializing PCM Monitor...\n");
        pcmMonitor.setUpMonitoring();
#endif
        printf("[INFO] Initializing Hashtable...\n");
        Hashtable * individualHt;
        uint64_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
        allocate_hashtable(&individualHt, numBuckets);

        GlobalHashTable individualGlobalHt; // have to create one of these to pass into thread pool.
        individualGlobalHt.ready = false;
        individualGlobalHt.beingBuilt = false;
        individualGlobalHt.ht = individualHt;

#if USE_PCM==1
        printf("[INFO] Starting Monitoring...\n");
        pcmMonitor.startMonitorThread();
#endif

// THIS WAS USED FOR MULTI-THREAD L2 CACHE EXPERIMENTS.
// ------------------------------------------------------------------------------------
//    const int numThreads = 12; // Change the number of threads as needed
//
//    pthread_t threads[numThreads];
//    int threadIDs[numThreads];
//
//    // Create threads
//    for (int i = 0; i < numThreads; ++i) {
//        threadIDs[i] = i;
//        if (pthread_create(&threads[i], nullptr, performComputationOnLargeDataStructure, &threadIDs[i]) != 0) {
//            std::cerr << "Error creating thread " << i << std::endl;
//            return 1; // Return with an error code
//        }
//    }
//    // Wait for threads to finish
//    for (int i = 0; i < numThreads; ++i) {
//        if (pthread_join(threads[i], nullptr) != 0) {
//            std::cerr << "Error joining thread " << i << std::endl;
//            return 1; // Return with an error code
//        }
//    }
//
//    std::cout << "All threads have finished execution." << std::endl;
// ------------------------------------------------------------------------------------

        printf("[INFO] Initializing ThreadPool...\n");
        ThreadPool threadPool(cmdParams.totalCores, relR, relS, individualGlobalHt, cmdParams.taskSize, buildQ, probeQ, pcmMonitor, path, cmdParams.id);
        threadPool.populateQueues();
        threadPool.start();

#if USE_PCM==1
        pcmMonitor.setMonitoringToFalse();
        pcmMonitor.stopMonitoring();
#endif

#if SAVE_RELATIONS_TO_FILE==1
        threadPool.saveJoinedRelationToFile();
#endif
        std::cout << "DONE. BYE!" << std::endl;
    return 0;

    }
}

void print_help(const char * progname) {
    printf("Usage: %s [options]\n", progname);
    printf("\
    Configuration options, with default values in [] :             \n\
       -n --nthreads=<N>  Number of threads to use <N> [2]                    \n\
       -r --r-size=<R>    Number of tuples in build relation R <R> [128000000]\n\
       -z --skew=<z>      Zipf skew parameter for probe relation S <z> [0.0]  \n\
    \n");
}

void parse_args(int argc, char **argv, CmdParams * cmdParams) {
    int c;

    while(1) {
        static struct option long_options[] =
            {
                {"r-size",          required_argument, 0, 'r'},
                {"s-size",          required_argument, 0, 's'},
                {"total-cores",     required_argument, 0, 'c'},
                {"monitor-cores",   required_argument, 0, 'n'},
                {"task-size",       required_argument, 0, 't'},
                {"hj-threads",      required_argument, 0, 'l'},
                {"share-hash-table",required_argument, 0, 'h'},
                {"skew",            required_argument, 0, 'k'},
                {"core-pausing",    required_argument, 0, 'p'},
                {"program-pmu",     required_argument, 0, 'm'},
                {"id",              required_argument, 0, 'i'},
            };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "r:s:c:t", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1) {
            break;
        }
        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;
            case 'r':
                cmdParams->rSize = atol(optarg);
                break;
            case 's':
                cmdParams->sSize = atol(optarg);
                break;
            case 'c':
                cmdParams->totalCores = atof(optarg);
                break;
            case 'n':
                cmdParams->coresToMonitor = atof(optarg);
                break;
            case 't':
                cmdParams->taskSize = atof(optarg);
                break;
            case 'l':
                cmdParams->hjThreads = atof(optarg);
                break;
            case 'h':
                cmdParams->shareHt = atof(optarg);
                break;
            case 'k':
                cmdParams->skew = atof(optarg);
                break;
            case 'p':
                cmdParams->corePausing = atof(optarg);
                break;
            case 'm':
                cmdParams->programPMU = atof(optarg);
                break;
            case 'i':
                cmdParams->id = atof(optarg);
                break;
            default:
                break;
        }
    }
    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        printf ("non-option arguments: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        printf ("\n");
    }
}