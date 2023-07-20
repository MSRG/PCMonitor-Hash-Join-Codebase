//
// Created by Sofia Tijanic on 2023-06-19.
//

// Libraries:
#include <stdio.h>
#include <dlfcn.h>
#include <iostream>
#include <getopt.h>

// Files:
#include "types.h"
#include "config.h"
#include "join.h"
#include "thread_pool.h"
#include "relation_generator.h"
#include "safe_queue.h"
//#include "pcm_monitor.h"

using namespace std;

/**
 * Data structure for command line arguments.
*/
struct CmdParams {
    uint64_t rSize;
    uint64_t sSize;
    uint64_t totalCores;
    uint64_t taskSize;
    double skew;
};

void print_help();

void parse_args(int argc, char **argv, CmdParams * cmdParams);

int main(int argc, char **argv) {

    Relation relR;
    Relation relS;
    SafeQueue buildQ;
    SafeQueue probeQ;

    /* Command line parameters */
    CmdParams cmdParams;
    cmdParams.rSize      = 100000000;
    cmdParams.sSize      = 100000000;
    cmdParams.skew       = 0;
    cmdParams.totalCores = 14;
    cmdParams.taskSize   = 10;
    parse_args(argc, argv, &cmdParams);

    // Create relation R.
    fprintf(stdout,
            "[INFO] Creating relation R of size = %.3lf, num of tuples = %lu... \n",
            (double) sizeof(Tuple) * cmdParams.rSize/1024.0/1024.0,
            (unsigned long)cmdParams.rSize);

    create_relation(&relR, cmdParams.rSize, cmdParams.rSize);

    // Create relation S.
    fprintf(stdout,
            "[INFO] Creating relation S of size = %.3lf, num of tuples = %lu... \n",
            (double) sizeof(Tuple) * cmdParams.sSize/1024.0/1024.0,
            (unsigned long)cmdParams.sSize);

    create_relation(&relS, cmdParams.sSize, cmdParams.sSize);

    printf("[INFO] Initializing PCM Monitor...\n");
    PcmMonitor pcmMonitor(cmdParams.totalCores);
    pcmMonitor.setUpMonitoring();
    pcmMonitor.startMonitorThread();

    printf("[INFO] Initializing Hashtable...\n");
    Hashtable * ht;
    uint32_t numBuckets = (relR.num_tuples / BUCKET_SIZE); // BUCKET_SIZE = 2
    allocate_hashtable(&ht, numBuckets);

    printf("[INFO] Initializing ThreadPool...\n");
    ThreadPool threadPool(cmdParams.totalCores, relR, relS, *ht, cmdParams.taskSize, buildQ, probeQ, pcmMonitor);
    threadPool.buildQueue();
    threadPool.start();

    //    int a = 10;
    //    int b = 15;
    //    threadPool.testFunction(a, b);

    printf("[INFO] Stopping ThreadPool...\n");
    pcmMonitor.setMonitoringToFalse();
    pcmMonitor.stopMonitoring();

#ifdef SAVE_RELATIONS_TO_FILE
    threadPool.saveJoinedRelationToFile();
#endif

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
                {"r-size",  required_argument, 0, 'r'},
                {"s-size",  required_argument, 0, 's'},
                {"skew",    required_argument, 0, 'z'},
            };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "r:s:k",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;
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
            case 'k':
                cmdParams->skew = atof(optarg);
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