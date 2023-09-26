//
// Created by Sofia Tijanic on 2023-06-19.
//

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include <pthread.h>            /* pthread_* */
#include <string.h>             /* memset */
#include <stdio.h>              /* printf */
#include <stdlib.h>             /* memalign */
#include <sys/time.h>           /* gettimeofday */
#include <chrono>

#include "join.h"
#include "lock.h"               /* lock, unlock */
#include "join_params.h"         /* constant parameters */
#include "cpu_mapping.h"        /* get_cpu_id */

//typedef struct arg_t arg_t;

#ifndef NEXT_POW_2
/**
 *  compute the next number, greater than or equal to 32-bit unsigned v.
 *  taken from "bit twiddling hacks":
 *  http://graphics.stanford.edu/~seander/bithacks.html
 */
#define NEXT_POW_2(V)                           \
    do {                                        \
        V--;                                    \
        V |= V >> 1;                            \
        V |= V >> 2;                            \
        V |= V >> 4;                            \
        V |= V >> 8;                            \
        V |= V >> 16;                           \
        V++;                                    \
    } while(0)
#endif

#ifndef HASH
#define HASH(X, MASK, SKIP) (((X) & MASK) >> SKIP)
#endif

/**
 * Allocates a hashtable of NUM_BUCKETS and inits everything to 0.
 * @param ht pointer to a Hashtable pointer
 */
void allocate_hashtable(Hashtable ** ppht, uint64_t nbuckets) {

    Hashtable * ht;
    ht              = (Hashtable*)malloc(sizeof(Hashtable));
    ht->num_buckets = nbuckets;
    NEXT_POW_2((ht->num_buckets));

    std::cout << "Hash table info:\n-- Num of buckets = " << nbuckets << "\n-- Size = " << (ht->num_buckets * sizeof(Bucket))/1000000000 << " GB.\n" << std::endl; ;

    // Allocate hashtable buckets cache line aligned.
    // Allocate (ht->num_buckets * sizeof(Bucket)) bytes and place the address of the allocated memory
    // in (void**)&ht->buckets. The address of the allocated memory will be a multiple of CACHE_LINE_SIZE.
    if (posix_memalign((void**)&ht->buckets, CACHE_LINE_SIZE, ht->num_buckets * sizeof(Bucket))) {
        perror("[Hashtable Building]: Aligned allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    // ht->buckets: pointer to block of memory to fill
    // 0: value to set
    // ht->num_buckets * sizeof(Bucket): number of bytes to be set to the value.
    memset(ht->buckets, 0, ht->num_buckets * sizeof(Bucket));
    ht->skip_bits = 0; /* the default for modulo hash */
    ht->hash_mask = (ht->num_buckets - 1) << ht->skip_bits;
    *ppht = ht;
}

/**
 * Initializes a new BucketBuffer for later use in allocating
 * buckets when overflow occurs.
 *
 * @param ppbuf [in,out] bucket buffer to be initialized
 */
void init_bucket_buffer(BucketBuffer ** ppbuf) {
    BucketBuffer * overflowbuf;
    overflowbuf = (BucketBuffer*) malloc(sizeof(BucketBuffer));
    overflowbuf->count = 0;
    overflowbuf->next  = NULL;

    *ppbuf = overflowbuf;
}

/**
 * Returns a new Bucket from the given BucketBuffer.
 * If the BucketBuffer does not have enough space, then allocates
 * a new BucketBuffer and adds to the list.
 *
 * @param result [out] the new bucket
 * @param buf [in,out] the pointer to the BucketBuffer pointer
 */
static inline void get_new_bucket(Bucket ** result, BucketBuffer ** buf) {
    if((*buf)->count < OVERFLOW_BUF_SIZE) {
        *result = (*buf)->buf + (*buf)->count;
        (*buf)->count ++;
    }
    else {
        /* need to allocate new buffer */
        BucketBuffer * new_buf = (BucketBuffer*)malloc(sizeof(BucketBuffer));
        new_buf->count = 1;
        new_buf->next  = *buf;
        *buf    = new_buf;
        *result = new_buf->buf;
    }
}

/** De-allocates all the BucketBuffer */
void free_bucket_buffer(BucketBuffer * buf) {
    do {
        BucketBuffer * tmp = buf->next;
        free(buf);
        buf = tmp;
    } while(buf);
}

void build(ThreadArg &args) {

    uint64_t i;
    QueueTask task = *args.task;
    Hashtable ht = *args.ht;
    BucketBuffer *overflowBuf = args.overflowBuf;
    uint64_t startTup = task.startTupleIndex;
    uint64_t endTup = task.endTupleIndex;

//    std::cout << "building" << std::endl;
//    std::cout << "start = " << startTup << " end = " << endTup << "\n" << std::endl;

    const uint64_t hashmask = args.ht->hash_mask;
    const uint64_t skipbits = args.ht->skip_bits;

    // Loop through all tuples assigned to this thread.
    for (i = startTup; i <= endTup; i++) {

        Tuple * dest;         // dest tuple.
        Bucket * curr, * nxt; // cur, next buckets.

        intkey_t idx = HASH(args.relR->tuples[i].key, hashmask, skipbits);

        // Copy the tuple to appropriate hash bucket.
        // If full, follow nxt pointer to find correct place.
        curr = ht.buckets+idx;
        lock(&curr->latch);
        nxt = curr->next;

        if (curr->count == BUCKET_SIZE) {
            if (!nxt || nxt->count == BUCKET_SIZE) {
                Bucket * b;
                get_new_bucket(&b, &overflowBuf);
                curr->next = b;
                b->next    = nxt;
                b->count   = 1;
                dest       = b->tuples;
            } else {
                dest = nxt->tuples + nxt->count;
                nxt->count ++;
            }
        } else {
            dest = curr->tuples + curr->count;
            curr->count ++;
        }
        *dest = args.relR->tuples[i]; // Store the tuple in bucket.
        unlock(&curr->latch);
    }
    return;
}

void probe(ThreadArg &args) {

    QueueTask task = *args.task;
    Hashtable ht = *args.ht;
    BucketBuffer *overflowBuf = args.overflowBuf;
    uint64_t startTup = task.startTupleIndex;
    uint64_t endTup = task.endTupleIndex;

//    std::cout << "probing" << std::endl;
//    std::cout << "start = " << startTup << " end = " << endTup << "\n" << std::endl;

    uint64_t i, j;
    int64_t matches = 0;
    int64_t matchesExist = 1;      // did this to be able to compare with matches.
    int64_t matchesPerKey = 0;
    const uint64_t hashmask = args.ht->hash_mask;
    const uint64_t skipbits = args.ht->skip_bits;

    args.completedTasks += 1;

    // Loop through all tuples this thread was assigned.
    for (i = startTup; i <= endTup; i++) {
        matchesPerKey = 0;
        // Calculate hash.
        intkey_t idx = HASH(args.relS->tuples[i].key, hashmask, skipbits);
        // Use hash as offset to find relevant bucket.
        Bucket * b = ht.buckets+idx;

        do {
            for (j = 0; j < b->count; j++) {
                if (args.relS->tuples[i].key == b->tuples[j].key) {
                    args.matches += 1;
                    matches ++;
                    matchesPerKey ++;
                    ChainTuple * chainTup = cb_next_writepos(args.threadJoinResults->chainedTupBuf);
                    chainTup->key        = args.relS->tuples[i].key;    /* key */
                    chainTup->sPayload  = args.relS->tuples[i].payload; /* S-rid */
                    chainTup->rPayload  = b->tuples[j].payload;  /* R-rid */
                }
            }
            b = b->next;    // Follow overflow pointer.
        } while (b);        // AKA while there is a valid next pointer.
    }
    if (matches >= matchesExist) {
        args.matchTasks += 1;
    }
    args.threadJoinResults->numResults += matches;
    return;
}

