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

#define MALLOC(SZ) alloc_aligned(SZ+RELATION_PADDING) /*malloc(SZ+RELATION_PADDING)*/

void * alloc_aligned(size_t size) {
    void * ret;
    int rv;
    rv = posix_memalign((void**)&ret, CACHE_LINE_SIZE, size);

    if (rv) {
        perror("[ERROR] alloc_aligned() failed: out of memory");
        return 0;
    }
    return ret;
}

/**
 * Write relation to a file.
 */
void write_relation(Relation *rel, char const * filename) {
    FILE * fp = fopen(filename, "w");
    uint64_t i;

    fprintf(fp, "#KEY, VAL\n");
    for (i = 0; i < rel->num_tuples; i++) {
        fprintf(fp, "%li %li\n", rel->tuples[i].key, rel->tuples[i].payload);
    }
    fclose(fp);
}

/*
* Create relation R.
* @relation: relR passed by reference to create new relations.
* @num_tuples: r_size parameter
*/
int create_relation_R(Relation *relation, uint64_t num_tuples) {

    uint32_t i;
    relation->num_tuples = num_tuples;

    // We need aligned allocation of items.
    relation->tuples = (Tuple*) MALLOC(num_tuples * sizeof(Tuple));
    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    for (i = 0; i < relation->num_tuples; i++) {
        relation->tuples[i].key     = i;
        relation->tuples[i].payload = i;
    }

#ifdef SAVE_RELATIONS_TO_FILE
    char const *filename;
    filename = "R.tbl";
    write_relation(relation, filename);
#endif
    return 0;
}


/*
* Create relation S.
* @relation: relS passed by reference to create new relations.
* @num_tuples: r_size parameter
* @task_size: the size of tasks that threads will be taking at a time.
* @skew: the skew that will affect matches.
*/
int create_relation_S(Relation *relation, uint64_t num_tuples, int taskSize, int skew) {

    uint32_t i;
    int taskSizeCounter = taskSize;
    int skewMode = 1;
    relation->num_tuples = num_tuples;

    // We need aligned allocation of items.
    relation->tuples = (Tuple*) MALLOC(num_tuples * sizeof(Tuple));
    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    int mixer = 0;

    if (skew) {
        for (i = 0; i < relation->num_tuples; i++) {
            if (skewMode == 2) {
                relation->tuples[i].key = i;                  // matches.
            } else if (skewMode == 1) {
                relation->tuples[i].key = num_tuples + i;    // not matches.
            }
            relation->tuples[i].payload = i;

            // Switch skewMode every taskSize tuples.
            taskSizeCounter --;

            if (taskSizeCounter == 0) {
                taskSizeCounter = 10;
                mixer += 1;
                if (skewMode == 1) {
                    if (mixer < 5) {
                        skewMode = 1;
                    } else {
                        mixer = 0;
                        skewMode = 2;
                    }
                }
                else if (skewMode == 2) {
                    skewMode = 1;
                }
            }
        }
    } else {
        for (i = 0; i < relation->num_tuples; i++) {
            relation->tuples[i].key     = i;
            relation->tuples[i].payload = i;
        }
    }

#ifdef SAVE_RELATIONS_TO_FILE
    char const *filename;
    filename = "S.tbl";
    write_relation(relation, filename);
#endif

    return 0;
}


