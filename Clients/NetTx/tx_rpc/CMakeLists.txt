add_library(tx_rpc STATIC 
tx_rpc.cpp
tx_exe.cpp
)

target_include_directories(tx_rpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tx_rpc utils ljrpc transaction)