cmake_minimum_required(VERSION 3.10)

project(Pool-HashJoin-PCM VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#include_directories(/home/sofia/Projects/CloudDB/pcm/src)

#add_library(pcm SHARED IMPORTED)
#set_property(TARGET pcm PROPERTY IMPORTED_LOCATION "/home/sofia/Projects/CloudDB/pcm/build/lib/libpcm.so")

add_subdirectory(src)

set(SOURCES
        src/join_params.h
#        src/cpu_mapping.h
#        src/cpu_mapping.cpp
#        src/relation_generator.h
#        src/relation_generator.cpp
        src/types.h
        src/join.h
        src/join.cpp
        src/config.h
        src/main.cpp
        )

add_executable(
        Pool-HashJoin-PCM
        ${SOURCES}
)

target_include_directories(
        Pool-HashJoin-PCM
        PUBLIC
        "${PROJECT_BINARY_DIR}/src"
)

target_link_libraries(
        Pool-HashJoin-PCM
#        pcm
        pthread
#        numa
)

target_compile_definitions(
        Pool-HashJoin-PCM
        PUBLIC KEY_8B=True
)




