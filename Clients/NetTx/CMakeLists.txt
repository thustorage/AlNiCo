cmake_minimum_required(VERSION 3.1)
project(TxSys)

set(CMAKE_CXX_STANDARD 11)
set (CMAKE_C_FLAGS "-g -O3  -Wall -Wno-unused -Wno-format -Wno-sign-compare -Wno-deprecated-declarations -fPIC")
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11")

add_subdirectory(utils)
add_subdirectory(test)
add_subdirectory(rdma_rpc)
add_subdirectory(tx_rpc)
add_subdirectory(NicTxn)
add_subdirectory(transaction)
add_subdirectory(workload_tpcc)
add_subdirectory(storage)
add_subdirectory(switch_msg)
add_subdirectory(strife)