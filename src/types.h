//
// Created by Sofia Tijanic on 2023-06-19.
//

#ifndef POOL_HASHJOIN_PCM_TYPES_H
#define POOL_HASHJOIN_PCM_TYPES_H

#include <stdint.h>
#include <ctime>
#include <ratio>
#include <chrono>
#include <condition_variable>

#include "config.h"

// Tuple keys & values are 64-bits (8 bytes) each —> 16 bytes per tuple.
typedef         int64_t             intkey_t;
typedef         int64_t             value_t;

typedef struct Bucket               Bucket;
typedef struct Hashtable            Hashtable;
typedef struct BucketBuffer         BucketBuffer;

typedef struct Tuple                Tuple;
typedef struct Relation             Relation;

typedef struct Result               Result;
typedef struct ThreadResult         ThreadResult;

typedef struct ThreadArg            ThreadArg;
typedef struct QueueTask            QueueTask;

typedef struct ChainedTupleBuffer   ChainedTupleBuffer;
typedef struct TupleBuffer          TupleBuffer;

typedef struct FineGrainedLock      FineGrainedLock;


using namespace std::chrono;

/**************************** RELATION STRUCTS ****************************/

/**
* Type definition for a tuple, depending on KEY_8B a tuple can be 16B or 8B.
* KEY_8B is set in CMakeLists.txt.
*/
struct Tuple {
    intkey_t key;       // int64_t = 8 bytes
    value_t  payload;   // int64_t = 8 bytes
};

/**
* Type definition for a tuple specifically used for the chained tuple buffer, defined in tuple_buffer.h.
*/
struct ChainTuple {
    intkey_t key;       // int64_t = 8 bytes
    value_t  rPayload;   // int64_t = 8 bytes
    value_t  sPayload;   // int64_t = 8 bytes
};

/**
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
struct Relation {
    Tuple * tuples;     // int64_t = 8 bytes
    uint64_t  num_tuples; // int64_t = 8 bytes
};


/**************************** HASHTABLE STRUCTS ****************************/


/**
 * Normal hashtable buckets.
 * if KEY_8B then key is 8B and sizeof(bucket_t) = 48B
 * else key is 16B and sizeof(bucket_t) = 32B
 */
struct Bucket {
    volatile char     latch;
    /* 3B hole */
    uint32_t          count;
    Tuple           tuples[BUCKET_SIZE];
    struct Bucket * next;
};

/** Hashtable structure. */
struct Hashtable {
    Bucket * buckets;
    int32_t    num_buckets;
    uint32_t   hash_mask;
    uint32_t   skip_bits;
};

/** Pre-allocated bucket buffers are used for overflow-buckets. */
struct BucketBuffer {
    struct  BucketBuffer * next;
    uint32_t count;
    Bucket buf[OVERFLOW_BUF_SIZE];
};


/**************************** RESULT STRUCTS ****************************/


/** Holds the join results of a thread */
struct ThreadResult {
    int      nresults;
    void *   results;
    uint32_t threadid;
};

/** Type definition for join results. */
struct JoinResults {
    int              numResults;
    ChainedTupleBuffer * chainedTupBuf;
};


/**************************** CHAINED TUPLE BUFFER ****************************/


struct TupleBuffer {
    ChainTuple * tuples;
    TupleBuffer * next;
};

struct ChainedTupleBuffer {
    TupleBuffer * buf;
    TupleBuffer * readcursor;
    TupleBuffer * writecursor;
    uint32_t writepos;
    uint32_t readpos;
    uint32_t readlen;
    uint32_t numbufs;
};


/**************************** THREAD POOL STRUCTS ****************************/

struct ThreadArg {
    int tid;
    int taskSize;
    int completedTasks;
    int matches;
    int nonMatchTasks;
    int matchTasks;
    int lastTaskVectorPosition;
    Hashtable *ht;
    Relation *relR;
    Relation *relS;
    QueueTask  *task;
    pthread_barrier_t *barrier;
    BucketBuffer *overflowBuf; // Overflow buffer for each thread.
    JoinResults *threadJoinResults;
    ThreadResult *threadResults; // Results of the thread.
    std::condition_variable condVar;
};

typedef void (*funcky)(ThreadArg&);

struct QueueTask {
    int          startTupleIndex;
    int          endTupleIndex;
    funcky function;
};


/**************************** THREAD POOL STRUCTS ****************************/


struct Timestamps {
    struct timeval startTime, buildPhaseEnd, endTime;
};


/**************************** TASK QUEUE STRUCTS ****************************/

struct FineGrainedLock {
    double padding1;
    double padding2;
    double padding3;
    pthread_mutex_t fineLock;
//    std::mutex fineLock;
};


#endif //POOL_HASHJOIN_PCM_TYPES_H
