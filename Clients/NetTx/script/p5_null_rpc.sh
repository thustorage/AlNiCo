#!/bin/bash
###
 # @Author: your name
 # @Date: 2021-03-12 10:55:05
 # @LastEditTime: 2022-01-12 21:36:53
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /TxSys/script/nictx_client.sh
### 
cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/utils/
rm -f CMakeLists.txt
cp CMakeLists_nictxn.in CMakeLists.txt

sed -i s#GGGGG1#ASYNCTXNUM=4#g CMakeLists.txt
sed -i s#GGGGG2#RANGEOFKEY_STORAGE=12500000#g CMakeLists.txt
sed -i s#GGGGG3#RANGEOFKEY=12500000#g CMakeLists.txt
sed -i s#GGGGG4#MAX_THREAD=20#g CMakeLists.txt
sed -i s#GGGGG5#NEWORDERP=100#g CMakeLists.txt
sed -i s#GGGGG6#YCSBplusT#g CMakeLists.txt
sed -i s#GGGGG7#YCSBWRITE=$1#g CMakeLists.txt
sed -i s#GGGGG8#YCSBZIPF=$2#g CMakeLists.txt
sed -i s#GGGGG9#CLIENT_THREAD=24#g CMakeLists.txt
sed -i s#YYYYY0#YCSBSIZE=$3#g CMakeLists.txt
sed -i s#YYYYY1#YCSBCOL=20#g CMakeLists.txt
sed -i s#YYYYY2#SPNIC2#g CMakeLists.txt
sed -i s#YYYYY3#NULLRPC#g CMakeLists.txt

sed -i s#YYYYY4#RPCSIZE=$4#g CMakeLists.txt

cd ../build
cmake ..
make -j
cd ../script

ssh root@10.0.2.111 "pkill NICTX_client"
ssh root@10.0.2.113 "pkill NICTX_client"
ssh root@10.0.2.111 "pkill strife_client"
ssh root@10.0.2.113 "pkill strife_client"
# ssh root@10.0.2.116 "pkill NICTX_client"

./init_memcached.sh

# ssh root@10.0.2.115 "cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test; ./NICTX_client 2 1 >/dev/null 2>&1 &"
# ssh root@10.0.2.116 "cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test; ./NICTX_client 3 1 >/dev/null 2>&1 &"
echo "go"
ssh root@10.0.2.113 "cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test; ./NICTX_client 1 0 >/dev/null 2>&1 &"
ssh root@10.0.2.111 "cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test; ./NICTX_client 2 0 >/dev/null 2>&1 &"



