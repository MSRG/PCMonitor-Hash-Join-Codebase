add_executable(main main.cpp)

add_library(join join.cpp join.h)

#add_library(relation_generator relation_generator.cpp relation_generator.h)

target_link_libraries(
        main
        join
        pthread
#        numa
)

