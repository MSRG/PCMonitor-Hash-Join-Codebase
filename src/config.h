//
// Created by Sofia Tijanic on 2023-06-19.
//

#ifndef POOL_HASHJOIN_PCM_CONFIG_H
#define POOL_HASHJOIN_PCM_CONFIG_H

/** Analysis **/

#define SAVE_JOIN_RESULTS 1
#define MEASURE_LATENCY 1
#define PERF_COUNTERS 0
#define SAVE_RELATIONS_TO_FILE 1

/** Algorithm Parameters **/

#define BUCKET_SIZE 2
#define OVERFLOW_BUF_SIZE 1024


#endif //POOL_HASHJOIN_PCM_CONFIG_H
