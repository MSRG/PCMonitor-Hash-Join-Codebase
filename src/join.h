//
// Created by Sofia Tijanic on 2023-06-19.
//

#ifndef POOL_HASHJOIN_PCM_JOIN_H
#define POOL_HASHJOIN_PCM_JOIN_H

#include "types.h"

result_t * join(relation_t *relR, relation_t *relS, int skew);


#endif //POOL_HASHJOIN_PCM_JOIN_H
