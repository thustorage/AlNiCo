add_library(storage STATIC mystorage.cpp)
target_include_directories(storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  /home/ljr/tpcc/libcuckoo/install/include/)
target_link_libraries(storage utils)
