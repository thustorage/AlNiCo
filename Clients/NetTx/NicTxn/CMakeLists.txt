add_library(NicTxn STATIC 
model_update.cpp
tpcc_procedure.cpp
cross_all.cpp
nictxn_rpc.cpp
nictxn_tx.cpp
nictxn_as_client.cpp
nictxn_as_server.cpp
virtual_fpga.cpp
fpga.cpp
)

target_include_directories(NicTxn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(NicTxn utils tx_rpc transaction)