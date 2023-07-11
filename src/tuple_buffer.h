/**
 * @file    tuple_buffer.h
 * @author  Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date    Thu Nov  8 22:08:54 2012
 * @versiocb_next_writeposn $Id: tuple_buffer.h 3203 2013-01-21 22:10:35Z bcagri $
 *
 * @brief   Implements a chained-buffer storage model for tuples.
 *
 *
 */
//#ifndef TUPLE_BUFFER_H
//#define TUPLE_BUFFER_H

#include <stdlib.h>
#include <stdio.h>

#include "types.h"

#define CHAINEDBUFF_NUMTUPLESPERBUF (1024*1024)

/** If rid-pairs are coming from a sort-merge join then 1, otherwise for hash
    joins it is always 0 since output is not sorted. */
#define SORTED_MATERIALIZE_TO_FILE 0

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

static inline void cb_begin(ChainedTupleBuffer * cb) {
    cb->readpos = 0;
    cb->readlen = cb->writepos;
    cb->readcursor = cb->buf;
}

static inline void cb_begin_backwards(ChainedTupleBuffer * cb) {
    cb->readpos = cb->writepos - 1;
    cb->readcursor = cb->buf;
}

static inline ChainTuple * cb_read_next(ChainedTupleBuffer * cb) {
    if(cb->readpos == cb->readlen) {
        cb->readpos = 0;
        cb->readlen = CHAINEDBUFF_NUMTUPLESPERBUF;
        cb->readcursor = cb->readcursor->next;
    }

    return (cb->readcursor->tuples + cb->readpos ++);
}

static inline ChainTuple * cb_read_backwards(ChainedTupleBuffer * cb) {

    ChainTuple * res = (cb->readcursor->tuples + cb->readpos);

    if(cb->readpos == 0) {
        cb->readpos = CHAINEDBUFF_NUMTUPLESPERBUF - 1;
        cb->readcursor = cb->readcursor->next;
    }
    else {
        cb->readpos --;
    }

    return res;
}

static inline ChainTuple *cb_next_writepos(ChainedTupleBuffer * cb) {
    if (cb->writepos == CHAINEDBUFF_NUMTUPLESPERBUF) {
        TupleBuffer * newbuf = (TupleBuffer*) malloc(sizeof(TupleBuffer));
        posix_memalign ((void **)&newbuf->tuples,
                            CACHE_LINE_SIZE, sizeof(ChainTuple)
                            * CHAINEDBUFF_NUMTUPLESPERBUF);

        newbuf->next = cb->buf;
        cb->buf = newbuf;
        cb->writepos = 0;
        cb->numbufs ++;
    }

    return (cb->buf->tuples + cb->writepos ++);
}

static ChainedTupleBuffer * chainedtuplebuffer_init(void) {

    ChainedTupleBuffer * newcb = (ChainedTupleBuffer *)malloc(sizeof(ChainedTupleBuffer));
    TupleBuffer * newbuf = (TupleBuffer *) malloc(sizeof(TupleBuffer));

    newbuf->tuples = (ChainTuple *) malloc(sizeof(ChainTuple) * CHAINEDBUFF_NUMTUPLESPERBUF);
    newbuf->next   = NULL;

    newcb->buf      = newcb->readcursor = newcb->writecursor = newbuf;
    newcb->writepos = newcb->readpos = 0;
    newcb->numbufs  = 1;

    return newcb;
}

static void chainedtuplebuffer_free(ChainedTupleBuffer * cb) {
    TupleBuffer * tmp = cb->buf;

    while(tmp) {
        TupleBuffer * tmp2 = tmp->next;
        free(tmp->tuples);
        free(tmp);
        tmp = tmp2;
    }

    free(cb);
}

/** Descending order comparison */
static inline int __attribute__((always_inline)) thrkeycmp(const void * k1, const void * k2) {
    int val = ((ChainTuple *)k2)->key - ((ChainTuple *)k1)->key;
    return val;
}

/**
 * Works only when result_t->threadresult_t->results is of type chainedtuplebuffer_t.
 */
//static void write_result_relation(result_t * res, const char * filename) {
//
//// SORTED_MATERIALIZE_TO_FILE = 0 for hash join bc results are not sorted.
//#if SORTED_MATERIALIZE_TO_FILE
//    FILE * fp = fopen(filename, "w");
//    int i;
//    int64_t j;
//
////    fprintf(fp, "#KEY, VAL\n");
//
//    TupleChain threadorder[res->nthreads];
//
//    for (i = 0; i < res->nthreads; i++) {
//        int64_t nresults = res->resultlist[i].nresults;
//
//        if(nresults > 0) {
//            chainedtuplebuffer_t * cb = (chainedtuplebuffer_t *) res->resultlist[i].results;
//            cb_begin_backwards(cb);
//            TupleChain * tup = cb_read_backwards(cb);
//            threadorder[i].key = tup->key;
//            threadorder[i].payload = i; /* thread index */
//        }
//        else {
//            threadorder[i].key = 0;
//            threadorder[i].payload = -1;
//        }
//    }
//
//    /* just to output thread results sorted */
//    qsort(threadorder, res->nthreads, sizeof(TupleChain), thrkeycmp);
//
//    for(i = 0; i < res->nthreads; i++) {
//        int tid = threadorder[i].payload;
//
//        if(tid != -1) {
//            chainedtuplebuffer_t * cb = (chainedtuplebuffer_t *)
//                                        res->resultlist[tid].results;
//            int64_t nresults = res->resultlist[tid].nresults;
//            cb_begin_backwards(cb);
//
//            for (j = 0; j < nresults; j++) {
//                TupleChain * tup = cb_read_backwards(cb);
//                fprintf(fp, "%ld %ld\n", tup->key, tup->payload);
//            }
//        }
//    }
//
//    fclose(fp);
//
//#else
//    FILE * fp = fopen(filename, "w");
//    int i;
//    int64_t j;
//
//    for (i = 0; i < res->nthreads; i++) {
//
//        fprintf(fp, "thread: %d\n", i);
//
//        // char fname[256];
//        // sprintf(fname, "Thread-%d.tbl", i);
//        // fp = fopen(fname, "w");
//
//        chainedtuplebuffer_t * cb = (chainedtuplebuffer_t *) res->resultlist[i].results;
//        int64_t nresults = res->resultlist[i].nresults;
//
//        cb_begin_backwards(cb);
//
//        for (j = 0; j < nresults; j++) {
//            TupleChain * tup = cb_read_backwards(cb);
//            fprintf(fp, "%ld %ld\n", (long)tup->key, (long)tup->payload);
//        }
//    }
//    fclose(fp);
//
//#endif
//}

//#endif /* TUPLE_BUFFER_H */
