//
// Created by Sofia Tijanic on 2023-06-19.
//

#include <pthread.h>            /* pthread_attr_setaffinity_np */
#include <string.h>             /* memcpy() */
#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include <stdio.h>              /* perror */
#include <stdlib.h>             /* posix_memalign */
#include <math.h>               /* fmod, pow */
#include <time.h>               /* time() */
#include <unistd.h>             /* getpagesize() */
#include <iostream>
#include <string>

#include "relation_generator.h"
#include "cpu_mapping.h"        /* get_cpu_id() */
#include "join_params.h"

//#define MALLOC(SZ) alloc_aligned(SZ+RELATION_PADDING) /*malloc(SZ+RELATION_PADDING)*/

#define MALLOC(SZ) alloc_aligned(SZ) /*malloc(SZ)*/

void * alloc_aligned(size_t size) {
//    std::cout << "Allocating " << size << " bytes of memory for relation, using alignment of " << CACHE_LINE_SIZE << std::endl;

    void * ret;
    int rv;
    // Allocates size bytes and places the address of the allocated memory in ret.
    // The address of the allocated memory will be a multiple of CACHE_LINE_SIZE, which must be a power of two and a multiple of sizeof(void *).
     rv = posix_memalign((void**)&ret, CACHE_LINE_SIZE, size);

    if (rv) {
        perror("[ERROR] alloc_aligned() failed: out of memory");
        return 0;
    }
    std::cout << "Done allocating! " << std::endl;
    return ret;
}

/**
 * Write relation to a file.
 */
void write_relation(Relation *rel, char const * filename) {
    FILE * fp = fopen(filename, "w");
    uint64_t i;

    std::cout << "Writing to file: " << filename << " " << rel->num_tuples << " tuples." << std::endl;

    fprintf(fp, "#KEY, VAL\n");
    for (i = 0; i < rel->num_tuples; i++) {
        fprintf(fp, "%li %li\n", rel->tuples[i].key, rel->tuples[i].payload);
    }
    fclose(fp);
}

// Function that threads execute to fill in the relation.
void *fillRelation(void *args) {
    RelFillThreadArg * arg = (RelFillThreadArg *) args;
    for (int64_t i = arg->start; i < arg->end; i++) {
        arg->relation->tuples[i].key     = i;
        arg->relation->tuples[i].payload = 14;
    }
}

/*
* Create relation R.
* @relation: relR passed by reference to create new relations.
* @num_tuples: r_size parameter
*/
//void *create_relation_R(Relation *relation, uint64_t num_tuples) {
void *create_relation_R(void * args) {
    RelCreationThreadArg * arg = (RelCreationThreadArg *) args;

    uint64_t i;
    arg->relation->num_tuples = arg->relSize;

    std::cout << "Rel R size = " << arg->relSize << " = " <<  arg->relation->num_tuples << std::endl;
//    std::cout << "Mallocing: rel size = " << arg->relSize << " size of tuple = " << sizeof(Tuple) << " when multiplied is: " << (arg->relSize * sizeof(Tuple)) << std::endl;

    // We need aligned allocation of items.
    arg->relation->tuples = (Tuple*) MALLOC(arg->relSize * sizeof(Tuple));
    if (!arg->relation->tuples) {
        perror("out of memory");
    }

//     Now fill in the relation in parallel.
    int numFillThreads = 14;
    uint64_t partitionSize = arg->relSize / numFillThreads - 1;
    uint64_t start = 0;
    uint64_t end = partitionSize;

    pthread_t threads[numFillThreads];
    RelFillThreadArg fillArgs[numFillThreads];

    for (int j = 0; j < numFillThreads; j++) {
        fillArgs[j].start = start;
        if (j == numFillThreads - 1) { fillArgs[j].end = arg->relSize; }
        else { fillArgs[j].end = end; }

        start += partitionSize;
        end = start + partitionSize;

        fillArgs[j].relation = arg->relation;
        pthread_create(&threads[j], NULL, *fillRelation, (void*)&fillArgs[j]);
    }

    for (int j = 0; j < numFillThreads; j++) {
        pthread_join(threads[j], NULL);
    }

//  Old single-threaded relation filling.
//    for (i = 0; i < arg->relation->num_tuples; i++) {
//        arg->relation->tuples[i].key     = i;
//        arg->relation->tuples[i].payload = i;
//    }

    std::cout << "Done filling relations!" << std::endl;

#if SAVE_RELATIONS_TO_FILE==1
    char const *filename;
    filename = "R.tbl";
    write_relation(arg->relation, filename);
#endif
}


/*
* Create relation S.
* @relation: relS passed by reference to create new relations.
* @num_tuples: r_size parameter
* @task_size: the size of tasks that threads will be taking at a time.
* @skew: the skew that will affect matches.
*/
//void *create_relation_S(Relation *relation, uint64_t num_tuples, int taskSize, int skew) {
void *create_relation_S(void * args) {
    RelCreationThreadArg * arg = (RelCreationThreadArg *) args;

    uint64_t i;
    int taskSizeCounter = arg->taskSize;
    int skewModeSwitcher = (arg->taskSize)/3+100000;
    int skewMode = 1;
    arg->relation->num_tuples = arg->relSize;

    std::cout << "Rel S size = " << arg->relSize << " = " <<  arg->relation->num_tuples << std::endl;

    // We need aligned allocation of items.
    arg->relation->tuples = (Tuple*) MALLOC(arg->relSize * sizeof(Tuple));
    if (!arg->relation->tuples) {
        perror("out of memory");
    }

    int mixer = 0;

    if (arg->skew) {
//        std::cout << "SKEW = " << arg->skew << " skew mode switcher " << skewModeSwitcher << std::endl;
        for (i = 0; i < arg->relation->num_tuples; i++) {
//            std::cout << "skewmode = " << skewMode << "skew mode switcher = " << skewModeSwitcher << std::endl;

            if (skewMode == 2) {
                arg->relation->tuples[i].key = i;                  // matches.
                skewModeSwitcher --;
                if (skewModeSwitcher != 0) { skewModeSwitcher --; }
            } else if (skewMode == 1) {
//                arg->relation->tuples[i].key = arg->relation->num_tuples + i;    // not matches.
                arg->relation->tuples[i].key = -1;
                skewModeSwitcher --;
            }
            arg->relation->tuples[i].payload = i;

            if (skewModeSwitcher == 0) {
                if (skewMode == 1) { skewMode = 2; }
                else { skewMode = 1; }
                skewModeSwitcher = (arg->taskSize)/3+100000;
            }

            // Switch skewMode every taskSize tuples.
//            taskSizeCounter --;
//            if (taskSizeCounter == 0) {
//                taskSizeCounter = 10;
//                mixer += 1;
//                if (skewMode == 1) {
//                    if (mixer < 5) {
//                        skewMode = 1;
//                    } else {
//                        mixer = 0;
//                        skewMode = 2;
//                    }
//                }
//                else if (skewMode == 2) {
//                    skewMode = 1;
//                }
//            }
        }
    } else {

//      Now fill in the relation in parallel.
        int numFillThreads = 14;
        uint64_t partitionSize = arg->relSize / numFillThreads - 1;
        uint64_t start = 0;
        uint64_t end = partitionSize;

        pthread_t threads[numFillThreads];
        RelFillThreadArg fillArgs[numFillThreads];

        for (int j = 0; j < numFillThreads; j++) {
            fillArgs[j].start = start;
            if (j == numFillThreads - 1) { fillArgs[j].end = arg->relSize; }
            else { fillArgs[j].end = end; }

            start += partitionSize;
            end = start + partitionSize;

            fillArgs[j].relation = arg->relation;
            pthread_create(&threads[j], NULL, *fillRelation, (void*)&fillArgs[j]);
        }

        for (int j = 0; j < numFillThreads; j++) {
            pthread_join(threads[j], NULL);
        }


//  Old single-threaded relation filling.
//        for (i = 0; i < arg->relation->num_tuples; i++) {
//            arg->relation->tuples[i].key     = i;
//            arg->relation->tuples[i].payload = i;
//        }
        std::cout << "Done filling relations!" << std::endl;
    }

#if SAVE_RELATIONS_TO_FILE==1
    char const *filename;
    filename = "S.tbl";
    write_relation(arg->relation, filename);
#endif
}


