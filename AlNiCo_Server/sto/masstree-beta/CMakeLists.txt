cmake_minimum_required(VERSION 3.8)
project(masstree)

set(CMAKE_CXX_STANDARD 11)

set(LIBMASSTREE_SOURCES
        kvio.cc
        misc.cc
        checkpoint.cc
        string_slice.cc)

set(LIBJSON_SOURCES
        json.cc
        string.cc
        straccum.cc
        str.cc
        msgpack.cc
        clp.c
        kvrandom.cc
        memdebug.cc
        kvthread.cc)

set(LIBKVTREE_SOURCES
        query_masstree.cc
        value_string.cc
        value_array.cc
        value_versioned_array.cc
        string_slice.cc)

set(MTTEST_SOURCES
        mttest.cc
        misc.cc
        checkpoint.cc
        testrunner.cc
        kvio.cc)

add_library(masstree ${LIBMASSTREE_SOURCES})
add_library(kvtree ${LIBKVTREE_SOURCES})
add_library(json ${LIBJSON_SOURCES})
add_library(dprint compiler.cc)

add_executable(mttest ${MTTEST_SOURCES})
target_link_libraries(mttest masstree json kvtree dprint)