//
// Created by Sofia Tijanic on 2023-06-19.
//

#ifndef POOL_HASHJOIN_PCM_RELATION_GENERATOR_H
#define POOL_HASHJOIN_PCM_RELATION_GENERATOR_H

#include "types.h"

int create_relation_R(Relation *relation, uint64_t num_tuples);

int create_relation_S(Relation *relation, uint64_t num_tuples, int taskSize, int skew);

#endif //POOL_HASHJOIN_PCM_RELATION_GENERATOR_H
