add_executable(main main.cpp)

add_library(join join.cpp join.h)
add_library(relation_generator relation_generator.cpp relation_generator.h)
add_library(cpu_mapping cpu_mapping.cpp cpu_mapping.h)
add_library(thread_pool thread_pool.cpp thread_pool.h)
add_library(queue queue.cpp queue.h)


target_link_libraries(
        main
        join
        relation_generator
        cpu_mapping
        thread_pool
        queue
        pthread
        numa
)

