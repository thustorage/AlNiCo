add_library(switch_msg STATIC 
ControlConnection.cpp 
SwitchTxCommon.cpp
TxFlowSteering.cpp
)

target_include_directories(switch_msg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(switch_msg utils storage ljrpc pthread)