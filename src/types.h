//
// Created by Sofia Tijanic on 2023-06-19.
//

#ifndef POOL_HASHJOIN_PCM_TYPES_H
#define POOL_HASHJOIN_PCM_TYPES_H

#include <stdint.h>
#include <ctime>
#include <ratio>
#include <chrono>

#include "config.h"

// Tuple keys & values are 64-bits (8 bytes) each —> 16 bytes per tuple.
typedef int64_t intkey_t;
typedef int64_t value_t;

typedef struct bucket_t        bucket_t;
typedef struct hashtable_t     hashtable_t;
typedef struct bucket_buffer_t bucket_buffer_t;

typedef struct tuple_t    tuple_t;
typedef struct relation_t relation_t;

typedef struct result_t result_t;
typedef struct threadresult_t threadresult_t;

using namespace std::chrono;

/**************************** RELATION STRUCTS ****************************/

/**
* Type definition for a tuple, depending on KEY_8B a tuple can be 16B or 8B.
* KEY_8B is set in CMakeLists.txt.
*/
struct tuple_t {
    intkey_t key;       // int64_t = 8 bytes
    value_t  payload;   // int64_t = 8 bytes
};

/**
* Type definition for a tuple specifically used for the chained tuple buffer, defined in tuple_buffer.h.
*/
struct tuple_t_chain {
    intkey_t key;       // int64_t = 8 bytes
    value_t  payload;   // int64_t = 8 bytes
    struct timeval timestamp;
//    steady_clock::time_point tstamp;
//    system_clock::time_point tstamp;
    high_resolution_clock::time_point tstamp;
};

/**
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
struct relation_t {
    tuple_t * tuples;     // 8 bytes
    uint64_t  num_tuples; // 8 bytes
};

/**************************** HASHTABLE STRUCTS ****************************/

/**
 * Normal hashtable buckets.
 * if KEY_8B then key is 8B and sizeof(bucket_t) = 48B
 * else key is 16B and sizeof(bucket_t) = 32B
 */
struct bucket_t {
    volatile char     latch;
    /* 3B hole */
    uint32_t          count;
    tuple_t           tuples[BUCKET_SIZE];
    struct bucket_t * next;
};

/** Hashtable structure. */
struct hashtable_t {
    bucket_t * buckets;
    int32_t    num_buckets;
    uint32_t   hash_mask;
    uint32_t   skip_bits;
};

/** Pre-allocated bucket buffers are used for overflow-buckets. */
struct bucket_buffer_t {
    struct  bucket_buffer_t * next;
    uint32_t count;
    bucket_t buf[OVERFLOW_BUF_SIZE];
};



/**************************** RESULT STRUCTS ****************************/

/** Holds the join results of a thread */
struct threadresult_t {
    int  nresults;
    void *   results;
    uint32_t threadid;
};

/** Type definition for join results. */
struct result_t {
    int64_t          totalresults;
    threadresult_t * resultlist;
    int              nthreads;
};


#endif //POOL_HASHJOIN_PCM_TYPES_H
