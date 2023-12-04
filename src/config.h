//
// Created by Sofia Tijanic on 2023-06-19.
//

#ifndef POOL_HASHJOIN_PCM_CONFIG_H
#define POOL_HASHJOIN_PCM_CONFIG_H

#define MONITOR_MEMORY 0

#define USE_PCM 1

/** Analysis **/
#define SAVE_RELATIONS_TO_FILE 0

#define SAVE_INDIVIDUAL_THREAD_TIME 0

/** Algorithm Parameters **/
#define BUCKET_SIZE 2
#define OVERFLOW_BUF_SIZE 1024

#endif //POOL_HASHJOIN_PCM_CONFIG_H
