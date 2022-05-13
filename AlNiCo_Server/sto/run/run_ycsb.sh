#!/bin/bash
###
 # @Author: your name
 # @Date: 2021-04-07 21:00:23
 # @LastEditTime: 2021-07-13 10:38:07
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /sto/run_ycsb.sh
### 

# METARUN="yes" run/run_ycsb_a.sh
# METARUN="yes" run/run_ycsb_b.sh
# METARUN="yes" run/run_ycsb_c.sh
# sudo shutdown -h +1
TARGET=tpcc_bench
BINARY=tpcc_bench-occ
FLAGS='NDEBUG=1 FINE_GRAINED=1'
make -j $TARGET $FLAGS 
mv $TARGET $BINARY

./mount_hugepages.sh 49152
./tpcc_bench-occ -t20 -mA -itictoc -g -x
./mount_hugepages.sh 0
