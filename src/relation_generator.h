//
// Created by Sofia Tijanic on 2023-06-19.
//

#ifndef POOL_HASHJOIN_PCM_RELATION_GENERATOR_H
#define POOL_HASHJOIN_PCM_RELATION_GENERATOR_H

#include "types.h"

int create_relation(relation_t *relation, uint64_t num_tuples, uint32_t nthreads, uint64_t maxid);

#endif //POOL_HASHJOIN_PCM_RELATION_GENERATOR_H
