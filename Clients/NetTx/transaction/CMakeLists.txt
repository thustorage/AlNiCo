add_library(transaction STATIC 
Transaction.cpp
)
target_include_directories(transaction PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(transaction utils workload_tpcc storage ljrpc switch_msg)