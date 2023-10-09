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
};

void print_help();

void parse_args(int argc, char **argv, CmdParams * cmdParams);

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

int main(int argc, char **argv) {

    double numOfBuildTasks, numOfProbeTasks;
    Relation relR;
    Relation relS;

    /* Command line parameters */
    CmdParams cmdParams;
    cmdParams.rSize         = 1000000;
    cmdParams.sSize         = 1000000;
    cmdParams.skew          = 0;
    cmdParams.totalCores    = 14;
    cmdParams.taskSize      = 10;
    cmdParams.corePausing   = false;
    cmdParams.programPMU    = true;
    cmdParams.id            = 0;
    parse_args(argc, argv, &cmdParams);

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

    create_relations(relR, relS, cmdParams.rSize, cmdParams.sSize, cmdParams.skew, cmdParams.taskSize);

    char * path;
    const char * resultsDir = "../results/";

    // get datetime and convert to string.
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y--%H:%M:%S--");
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

    printf("[INFO] Initializing PCM Monitor...\n");
    PcmMonitor pcmMonitor(cmdParams.totalCores, cmdParams.corePausing, path);
    if (cmdParams.programPMU) { pcmMonitor.setUpMonitoring(); }

    printf("[INFO] Initializing Hashtable...\n");
    Hashtable * ht;
    uint64_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
    allocate_hashtable(&ht, numBuckets);

    printf("[INFO] Starting Monitoring...\n");
    pcmMonitor.startMonitorThread();

    printf("[INFO] Initializing ThreadPool...\n");
    // RUN THE BUSY-CORES PROGRAM!
//    system("cd /home/sofia/Projects/CloudDB/busy-cores && ./run.sh &");
    ThreadPool threadPool(cmdParams.totalCores, relR, relS, *ht, cmdParams.taskSize, buildQ, probeQ, pcmMonitor, path);
    threadPool.populateQueues();
    threadPool.start();

    pcmMonitor.setMonitoringToFalse();
    pcmMonitor.stopMonitoring();

#if SAVE_RELATIONS_TO_FILE==1
    threadPool.saveJoinedRelationToFile();
#endif

    std::cout << "free stuff.." << std::endl;
    free(relR.tuples);
    free(relS.tuples);
    free(path);
    deallocate_hashtable(*ht);
//    std::cout << "DONE! BYE!" << std::endl;
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