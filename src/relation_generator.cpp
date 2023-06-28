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

#include "relation_generator.h"
#include "cpu_mapping.h"        /* get_cpu_id() */
#include "join_params.h"

#define MALLOC(SZ) alloc_aligned(SZ+RELATION_PADDING) /*malloc(SZ+RELATION_PADDING)*/

bool rCreated = false;

struct create_arg_t {
    int                 skew;
    relation_t          rel;
    int64_t             firstkey;
    int64_t             maxid;
    uint64_t            ridstart;
    relation_t *        fullrel;
    volatile void *     locks;
//    pthread_barrier_t * barrier;
};

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
 * Create random unique keys starting from firstkey.
 */
void * random_unique_gen_thread(void * args) {
    create_arg_t * arg      = (create_arg_t *) args;
    relation_t *   rel      = & arg->rel;
    int64_t        firstkey = arg->firstkey;
    int64_t        maxid    = arg->maxid;
    uint64_t       ridstart = arg->ridstart;
    uint64_t i;

    /* for randomly seeding nrand48() */
    unsigned short state[3] = {0, 0, 0};
    unsigned int seed       = time(NULL) + * (unsigned int *) pthread_self();
    memcpy(state, &seed, sizeof(seed));

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key     = firstkey;
        rel->tuples[i].payload = ridstart + (i+1);

        if(firstkey == maxid)
            firstkey = 0;

        firstkey ++;
    }
}

void generate_relation(relation_t *rel, int64_t firstkey, int64_t maxid, int64_t ridstart) {
    uint64_t i;

    for (i = 0; i < rel->num_tuples; i++) {
        rel->tuples[i].key     = firstkey;
        rel->tuples[i].payload = ridstart + (i+1);

        if(firstkey == maxid) {firstkey = 0; }
        firstkey ++;
    }
}

/**
 * Write relation to a file.
 */
void write_relation(relation_t *rel, char const * filename) {
    FILE * fp = fopen(filename, "w");
    uint64_t i;

    fprintf(fp, "#KEY, VAL\n");
    for (i = 0; i < rel->num_tuples; i++) {
        fprintf(fp, "%li %li\n", rel->tuples[i].key, rel->tuples[i].payload);
    }
    fclose(fp);
}
#include <string>

/*
* Create relation using multiple threads.
* @relation: relR/relS passed by reference to create new relations.
* @num_tuples: r_size/s_size parameter
* @maxid: number of threads.
*/
int create_relation(relation_t *relation, uint64_t num_tuples, uint64_t maxid) {

    int rv;
    uint32_t i;
    uint64_t offset = 0;
    unsigned int pagesize;
    unsigned int npages;
    unsigned int npages_perthr;
    uint64_t ntuples_perthr;
    uint64_t ntuples_lastthr;
    int64_t             firstkey;
//    int64_t             maxid;
    uint64_t            ridstart;

    relation->num_tuples = num_tuples;

    // We need aligned allocation of items.
    relation->tuples = (tuple_t*) MALLOC(num_tuples * sizeof(tuple_t));
    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    firstkey       = (offset + 1) % maxid;
    ridstart       = offset;
    generate_relation(relation, firstkey, maxid, ridstart);



#ifdef SAVE_RELATIONS_TO_FILE
    char const *filename;
    if (rCreated) {
        filename = "S.tbl";
    } else {
        filename = "R.tbl";
    }
    write_relation(relation, filename);
#endif

    rCreated = true;
    return 0;
}

/*
* Create relation using multiple threads.
* @relation: relR/relS passed by reference to create new relations.
* @num_tuples: r_size/s_size parameter
* @ nthreads: number of threads
* @maxid: number of threads.
*/
int create_relation_multithread(relation_t *relation, uint64_t num_tuples, uint32_t nthreads, uint64_t maxid) {

    int rv;
    uint32_t i;
    uint64_t offset = 0;
    unsigned int pagesize;
    unsigned int npages;
    unsigned int npages_perthr;
    uint64_t ntuples_perthr;
    uint64_t ntuples_lastthr;

    create_arg_t args[nthreads];    // Custom data struct.
    pthread_t tid[nthreads];        // Thread IDs.
    cpu_set_t set;                  // Linux struct representing set of CPUs.
    pthread_attr_t attr;            // Pthread struct representing thread characteristics.
    //    pthread_barrier_t barrier;      //

    relation->num_tuples = num_tuples;

    /* we need aligned allocation of items */
    relation->tuples = (tuple_t*) MALLOC(num_tuples * sizeof(tuple_t));
    if (!relation->tuples) {
        perror("out of memory");
        return -1;
    }

    pagesize        = getpagesize();    // Returns current page size.
    // Calculate how many pages will be needed for relation.
    npages          = (num_tuples * sizeof(tuple_t)) / pagesize + 1;
    // Number of pager per thread.
    npages_perthr   = npages / nthreads;
    // Number of tuples per thread.
    ntuples_perthr  =  npages_perthr * (pagesize/sizeof(tuple_t));
    if(npages_perthr == 0) { ntuples_perthr = num_tuples / nthreads; }
    ntuples_lastthr = num_tuples - ntuples_perthr * (nthreads-1);

    pthread_attr_init(&attr);

//    rv = pthread_barrier_init(&barrier, NULL, nthreads);
//    if(rv != 0){
//        printf("[ERROR] Couldn't create the barrier\n");
//        exit(EXIT_FAILURE);
//    }

    volatile void * locks = (volatile void *)calloc(num_tuples, sizeof(char));

    for(i = 0; i < nthreads; i++) {

        int cpu_idx = get_cpu_id(i);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

        // rel holds the empty relation struct that was passed in from main.
        // Only a portion of the relation is passed in to each thread.
        // The portion is defined by rel.tuples beginning at the first tuple, and
        // num_tuples indicating how many tuples that thread is responsible for.
        args[i].firstkey       = (offset + 1) % maxid;
        args[i].maxid          = maxid;
        args[i].ridstart       = offset;
        args[i].rel.tuples     = relation->tuples + offset;
        args[i].rel.num_tuples = (i == nthreads-1) ? ntuples_lastthr : ntuples_perthr;
        args[i].fullrel = relation;
        args[i].locks   = locks;
//        args[i].barrier = &barrier;

        offset += ntuples_perthr;

        rv = pthread_create(&tid[i], &attr, random_unique_gen_thread, (void*)&args[i]);

        if (rv){
            fprintf(stderr, "[ERROR] pthread_create() return code is %d\n", rv);
            exit(-1);
        }
    }

    for(i = 0; i < nthreads; i++){
        pthread_join(tid[i], NULL);
    }

    /* randomly shuffle elements */
    /* knuth_shuffle(relation); */

    /* clean up */
    free((char*)locks);
//    pthread_barrier_destroy(&barrier);

#ifdef PERSIST_RELATIONS
    //    char * const tables[] = {"R.tbl", "S.tbl"};
//    static int rs = 0;
//    write_relation(relation, tables[(rs++)%2]);
    if (rCreated) {
        write_relation(relation, "S.tbl");
    } else {
        write_relation(relation, "R.tbl");
    }

#endif

    rCreated = true;
    return 0;
}

