add_library(strife STATIC 
strife_rpc.cpp
)

target_include_directories(strife PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(strife NicTxn)