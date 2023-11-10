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
using namespace std;

/**
 * Data structure for command line arguments.
*/
struct CmdParams {
    uint64_t rSize;
    uint64_t sSize;
    uint64_t totalCores;
    uint64_t taskSize;
    bool corePausing;   // Allow cores to stop depending on performance counter info.
    bool programPMU;    // Program the PMU.
    int skew;
    int id;
    int hashJoinThreads;
};

void print_help();

void parse_args(int argc, char **argv, CmdParams * cmdParams);

// Physical Memory currently used
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

// Physical Memory currently used by current process

int parseLine(char* line) {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(){ // Note: this value is in KB!
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

// *** Create relations in parallel ** //
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
            "\n[INFO] Creating relation R:\n-- Number of tuples = %lu,\n-- %.3lf mebibytes,\n-- %.3lf bytes,\n-- %.3lf GB\n\n",
            (unsigned long)rSize,
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

void hashjoin(HashJoinThreadArg * args) {

    int id                      = args->tid;
    uint64_t rSize              = args->rSize;
    uint64_t sSize              = args->sSize;
    uint64_t totalCores         = args->totalCores;
    uint64_t taskSize           = args->taskSize;
    bool corePausing            = args->corePausing;
    bool programPMU             = args->programPMU;
    GlobalHashTable *globalht   = args->globalht;
    int skew                    = args->skew;

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

//    getUsedMemory(cmdParams.id, 0);

    memRequired = ((double) sizeof(Tuple) * sSize) + ((double) sizeof(Tuple) * rSize) + ((rSize / BUCKET_SIZE) * sizeof(Bucket));
    memAvailable = 500000000000 - getUsedMemory(id, 0);
    std::cout << "Required memory for this hash join = " << memRequired/1000000000 << " GB." << std::endl;
    std::cout << "Available memory for this hash join = " << memAvailable/1000000000 << " GB." << std::endl;

#if MONITOR_MEMORY==1
    // -------------------- MEMORY USE MONITORING -------------------------
    while ((400000000000 - getUsedMemory(id, 0)) < memRequired) { }
    // --------------------------------------------------------------------
#endif

    create_relations(relR, relS, rSize, sSize, skew, taskSize);
//    getUsedMemory(cmdParams.id, 1);

    char * path;
    const char * resultsDir = "../results/";

    // get datetime and convert to string.
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y--%H:%M:%S--");
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
    PcmMonitor pcmMonitor(totalCores, corePausing, path, id);
    if (programPMU) { pcmMonitor.setUpMonitoring(); }

    // This is where we check if hash table already exists.
//    if (!globalht->built) { // Hash table is not built.
//        if (!globalht->inCreation) { // No one is building it yet.
//
//            printf("[INFO] Creating Global Hashtable...\n");
//            globalht->inCreation = true;
//            uint64_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
//            allocate_hashtable(&globalht->ht, numBuckets);
//
//            printf("[INFO] Starting Monitoring...\n");
//            if (programPMU) { pcmMonitor.startMonitorThread(); }
//
//            printf("[INFO] Initializing ThreadPool...\n");
//            ThreadPool threadPool(totalCores, relR, relS, *globalht, taskSize, buildQ, probeQ, pcmMonitor, path, id, skipBuild);
//
//            threadPool.populateQueues();
//            threadPool.start();
//
//            if (programPMU) {
//                pcmMonitor.setMonitoringToFalse();
//                pcmMonitor.stopMonitoring();
//            }
//        } else { // Create a new hash table just for this join.

            printf("[INFO] Global Hashtable not ready, making my own...\n");
            GlobalHashTable ownGlobalht;
            Hashtable * ht;
            uint64_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
            allocate_hashtable(&ht, numBuckets);
            ownGlobalht.ht = ht;
            ownGlobalht.built = false;
            ownGlobalht.inCreation = false;

            printf("[INFO] Starting Monitoring...\n");
            if (programPMU) { pcmMonitor.startMonitorThread(); }

            printf("[INFO] Initializing ThreadPool...\n");
            ThreadPool threadPool(totalCores, relR, relS, ownGlobalht, taskSize, buildQ, probeQ, pcmMonitor, path, id, skipBuild);

            printf("[INFO] Populating Queues...\n");
            threadPool.populateQueues();

            printf("[INFO] Start Hash Join...\n");
            threadPool.start();

            if (programPMU) {
                pcmMonitor.setMonitoringToFalse();
                pcmMonitor.stopMonitoring();
            }
//        }
//    } else {
//        std::cout << "[INFO] I am using the Global Hashtable..." << std::endl;
//        skipBuild = true;
//
//        printf("[INFO] Starting Monitoring...\n");
//        if (programPMU) { pcmMonitor.startMonitorThread(); }
//
//        printf("[INFO] Initializing ThreadPool...\n");
//        ThreadPool threadPool(totalCores, relR, relS, *globalht, taskSize, buildQ, probeQ, pcmMonitor, path, id, skipBuild);
//
//        threadPool.populateQueues();
//        threadPool.start();
//
//        if (programPMU) {
//            pcmMonitor.setMonitoringToFalse();
//            pcmMonitor.stopMonitoring();
//        }
//    }

#if SAVE_RELATIONS_TO_FILE==1
    threadPool.saveJoinedRelationToFile();
#endif

    std::cout << "free stuff.." << std::endl;
//    free(relR.tuples);
//    free(relS.tuples);
//    free(path);
//    deallocate_hashtable(*globalht->ht);
    std::cout << "DONE! BYE!" << std::endl;
}

int main(int argc, char **argv) {

    /* Command line parameters */
    CmdParams cmdParams;
    cmdParams.rSize             = 1000000;
    cmdParams.sSize             = 1000000;
    cmdParams.skew              = 0;
    cmdParams.totalCores        = 14;
    cmdParams.taskSize          = 10;
    cmdParams.corePausing       = false;
    cmdParams.programPMU        = true;
    cmdParams.id                = 0;
    cmdParams.hashJoinThreads   = 1;
    parse_args(argc, argv, &cmdParams);

    bool threadSleep = true;

    // creating threads for multiple hash joins
//    int numHashJoinThreads = 5;
    std::vector<std::thread> threads(cmdParams.hashJoinThreads);
    HashJoinThreadArg args[cmdParams.hashJoinThreads];
    pthread_t tid[cmdParams.hashJoinThreads];

    // global hash table
    GlobalHashTable globalHashTable;
    Hashtable ht;
    globalHashTable.built = false;
    globalHashTable.inCreation = false;
    globalHashTable.ht = &ht;

    cpu_set_t set;            // Linux struct representing set of CPUs.
    pthread_attr_t attr;
    pthread_attr_init(&attr);


    for (int i = 0; i < cmdParams.hashJoinThreads; i++) {
        CPU_ZERO(&set);                 // Clears set, so that it contains no CPUs.
        CPU_SET(i, &set);               // Add CPU cpu to set.

        args[i].rSize       = cmdParams.rSize;
        args[i].sSize       = cmdParams.sSize;
        args[i].totalCores  = cmdParams.totalCores;
        args[i].taskSize    = cmdParams.taskSize;
        args[i].corePausing = cmdParams.corePausing;
        args[i].programPMU  = cmdParams.programPMU;
        args[i].skew        = cmdParams.skew;
        args[i].globalht    = &globalHashTable;
        args[i].tid         = i;

        threads[i] = thread (hashjoin, &args[i]);

//        if (threadSleep) {
//            sleep(100);
//            threadSleep = false;
//        }
    }

    for (auto& th : threads) {
        th.join();
    }

    return 0;
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
                {"task-size",       required_argument, 0, 't'},
                {"skew",            required_argument, 0, 'k'},
                {"core-pausing",    required_argument, 0, 'p'},
                {"program-pmu",     required_argument, 0, 'm'},
                {"id",              required_argument, 0, 'i'},
                {"hj-threads",      required_argument, 0, 'h'},
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
            case 't':
                cmdParams->taskSize = atof(optarg);
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
            case 'h':
                cmdParams->hashJoinThreads = atof(optarg);
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