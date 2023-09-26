//
// Created by Sofia Tijanic on 2023-06-19.
//

#ifndef POOL_HASHJOIN_PCM_JOIN_H
#define POOL_HASHJOIN_PCM_JOIN_H

#include "types.h"
#include "tuple_buffer.h"

void allocate_hashtable(Hashtable ** ppht, uint64_t nbuckets);

void init_bucket_buffer(BucketBuffer ** ppbuf);

void free_bucket_buffer(BucketBuffer * buf);

void build(ThreadArg &args);

void probe(ThreadArg &args);

#endif //POOL_HASHJOIN_PCM_JOIN_H
