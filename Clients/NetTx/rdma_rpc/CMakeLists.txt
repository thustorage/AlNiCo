add_library(ljrpc STATIC 
${CMAKE_CURRENT_SOURCE_DIR}/rpc/rpc.cpp
${CMAKE_CURRENT_SOURCE_DIR}/rdma/Resource.cpp
${CMAKE_CURRENT_SOURCE_DIR}/rdma/StateTrans.cpp
${CMAKE_CURRENT_SOURCE_DIR}/rdma/Operation.cpp
${CMAKE_CURRENT_SOURCE_DIR}/rpc/rpc_util.cpp
)

target_include_directories(ljrpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/rpc/include
${CMAKE_CURRENT_SOURCE_DIR}/rdma/include)

target_link_libraries(ljrpc utils storage ibverbs memcached) # no storage.