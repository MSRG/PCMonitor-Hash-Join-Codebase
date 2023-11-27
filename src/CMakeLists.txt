add_executable(main main.cpp)

add_library(join join.cpp join.h)
add_library(relation_generator relation_generator.cpp relation_generator.h)
add_library(cpu_mapping cpu_mapping.cpp cpu_mapping.h)
add_library(thread_pool thread_pool.cpp thread_pool.h)
add_library(pcm_monitor pcm_monitor.cpp pcm_monitor.h)
add_library(safe_queue safe_queue.cpp safe_queue.h)
add_library(fine_grained_queue fine_grained_queue.cpp fine_grained_queue.h)
add_library(global_hash_table global_hash_table.cpp global_hash_table.h)

target_link_libraries(
        main
        fine_grained_queue
        safe_queue
        pcm_monitor
        join
        relation_generator
        global_hash_table
        thread_pool
        cpu_mapping
        pthread
        numa
        pcm
)

