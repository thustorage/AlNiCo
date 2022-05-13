add_library(workload_tpcc STATIC tpcc_loader.cpp)
target_include_directories(workload_tpcc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(workload_tpcc utils storage)