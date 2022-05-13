#!/bin/bash
###
 # @Author: Junru Li
 # @Date: 2022-03-21 09:45:14
 # @LastEditors: Junru Li
 # @LastEditTime: 2022-03-21 09:46:22
 # @FilePath: /NetTx/script/loop_pressure.sh
 # @Description: 
### 

while (true)
do
    ib_send_bw -n 100000000 -s 4096 -m 2048  -d mlx5_0
    ib_read_bw -n 100000000 -s 4096 -m 2048  -d mlx5_0
    ib_write_bw -n 100000000 -s 4096 -m 2048  -d mlx5_0
    ib_write_lat -n 1000 -s 4096 -m 2048  -d mlx5_0
    ib_read_lat -n 1000 -s 4096 -m 2048  -d mlx5_0

    sleep 2s
    ib_send_bw -n 100000000 -s 4096 -m 2048  10.0.2.117 -d mlx5_0
    sleep 2s
    ib_read_bw -n 100000000 -s 4096 -m 2048  -d mlx5_0  10.0.2.117
    sleep 2s
    ib_write_bw -n 100000000 -s 4096 -m 2048  -d mlx5_0 10.0.2.117
    sleep 2s
    ib_write_lat -n 1000 -s 4096 -m 2048  -d mlx5_0 10.0.2.117
    sleep 2s
    ib_read_lat -n 1000 -s 4096 -m 2048  -d mlx5_0 10.0.2.117
    
done