add_executable(cmake_test test_main.cpp)
add_executable(NICTX_client NICTX_client.cpp)
add_executable(strife_client strife_client.cpp)
add_executable(simple hash_function.cpp)
add_executable(lsize L_size.cpp)
add_executable(hash hash_test.cpp)


target_link_libraries(cmake_test utils NicTxn)
target_link_libraries(NICTX_client utils NicTxn)
target_link_libraries(strife_client utils strife)
target_link_libraries(simple utils strife)
target_link_libraries(lsize utils strife)
target_link_libraries(hash utils strife)

