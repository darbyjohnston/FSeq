cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0048 NEW)

project(
    FSeq
    VERSION 1.0
    DESCRIPTION "FSeq is a C library for working with sequences of files."
    LANGUAGES C)

set(FSEQ_BUILD_BIN FALSE CACHE STRING "Whether to build the utilities")
set(FSEQ_BUILD_TESTS FALSE CACHE STRING "Whether to build the tests")
set(FSEQ_GCOV FALSE CACHE BOOL "Enable gcov code coverage")

if(UNIX)
    if(CMAKE_COMPILER_IS_GNUCXX)
        if(FSEQ_GCOV)
            include(CodeCoverage)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
        endif()
    endif()
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_library(fseq fseq.h fseq.c)

if(FSEQ_BUILD_BIN)
    add_executable(fseqls fseqls.c)
    target_link_libraries(fseqls fseq)
    add_executable(fseqCreateRandom fseqCreateRandom.c)
    target_link_libraries(fseqCreateRandom fseq)
endif()
if(FSEQ_BUILD_TESTS)
    enable_testing()
    add_executable(fseqTest fseqTest.c)
    target_link_libraries(fseqTest fseq)
    add_test(fseqTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fseqTest${CMAKE_EXECUTABLE_SUFFIX})
endif()

install(
    TARGETS fseq
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(FILES fseq.h DESTINATION include)
