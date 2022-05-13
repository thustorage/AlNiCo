###
 # @Author: your name
 # @Date: 2021-08-01 21:35:34
 # @LastEditTime: 2022-01-12 21:37:30
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /sto/tpcc_pic.sh
### 

TARGET=ycsb_bench
BINARY=ycsb_bench-net
size=16
# col=20

FLAGS='NDEBUG=1'

./mount_hugepages.sh 49152

# ./tpcc_bench-occ -t21 -i2pl -n -w2 -r1000
source ./ATC_p5_source.sh
# colll=(128 256)
# write=(50)
# zipf=(0.8)
# write=(5)

cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test
rm latencynull_*.txt
cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto

for((j=0;j<${#colll[*]};j+=1));
    do
        ww=50
        zz=0.99
        col=${colll[$j]} 
        # make clean
        FLAGS='NDEBUG=1 MAX_THREAD=20 YCSBplusT=1 NULLRPC=1 CPUBASED=4 STRIFE=1 YCSBTZIPF='$zz' YCSBSIZE='$size' ASYNCTXNUM=1 YCSBCOL=20 CLIENT_THREAD=10' 
        make -j $TARGET $FLAGS 
        mv $TARGET $BINARY

        # cd /home/ljr/tpcc/TxSys/script/
        # ./ycsb_pic_zipf.sh $ww $zz 51

        # NetTx
        cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
        ./p5_null_rpc_lat.sh $ww $zz $size $col
        # ./ycsb_pic_zipf_partition.sh $ww $zz $size $col

        cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 

        # gdb ./ycsb_bench-net
        # ./ycsb_bench-net -t20 -mA -idefault -g | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p5_overhead_RPC_lat.txt
        ./ycsb_bench-net -t20 -mA -idefault -g

        ssh root@10.0.2.111 "pkill NICTX_client"
        ssh root@10.0.2.113 "pkill NICTX_client"
        # rm $TARGET
        rm $BINARY 
    done

mv /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test/latencynull_*.txt  /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/

./mount_hugepages.sh 0