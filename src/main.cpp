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
#include "relation_generator.h"

using namespace std;

/**
 * Data structure for command line arguments.
*/
struct param_t {
    uint64_t r_size;
    uint64_t s_size;
    double skew;
};

void print_help();

void parse_args(int argc, char **argv, param_t * cmd_params);

int main(int argc, char **argv) {
    relation_t relR;
    relation_t relS;
    result_t * results;
    uint32_t numThreadsCreateRel = 1;

    /* Command line parameters */
    param_t cmd_params;
    cmd_params.r_size   = 100;
    cmd_params.s_size   = 100;
    cmd_params.skew     = 0;
    parse_args(argc, argv, &cmd_params);

    // Create relation R.
    fprintf(stdout,
            "[INFO] Creating relation R of size = %.3lf, num of tuples = %lu... \n",
            (double) sizeof(tuple_t) * cmd_params.r_size/1024.0/1024.0,
            (unsigned long)cmd_params.r_size);

    create_relation(&relR, cmd_params.r_size, cmd_params.r_size);

    // Create relation S.
    fprintf(stdout,
            "[INFO] Creating relation S of size = %.3lf, num of tuples = %lu... \n",
            (double) sizeof(tuple_t) * cmd_params.s_size/1024.0/1024.0,
            (unsigned long)cmd_params.s_size);

    create_relation(&relS, cmd_params.s_size, cmd_params.s_size);

    // Initialize threads 14 and 15 to begin PCM monitoring (maybe make them wait
    // for a signal when hashjoin really begins.

    printf("[INFO] Running join algorithm...\n");

    results = join(&relR, &relS, cmd_params.skew);

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

void parse_args(int argc, char **argv, param_t * cmd_params) {

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
                cmd_params->r_size = atol(optarg);
                break;

            case 's':
                cmd_params->s_size = atol(optarg);
                break;
            case 'k':
                cmd_params->skew = atof(optarg);
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