
TARGET=ycsb_bench
BINARY=ycsb_bench-net
size=16
col=20
zipf=(1.2)

FLAGS='NDEBUG=1'

./mount_hugepages.sh 49152

cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto
rm tp_listen_schism.txt

cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test
rm latency_ycsb_latency.txt # new order is the rpc

cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto

# ./tpcc_bench-occ -t21 -i2pl -n -w2 -r1000

write=(50)

for((i=0;i<${#write[*]};i+=1));
do
for((j=0;j<${#zipf[*]};j+=1));
    do
        ww=${write[$i]}
        zz=${zipf[$j]} 
        # make clean
        FLAGS='NDEBUG=1 MAX_THREAD=20 YCSBplusT=1 DYNAMIC=1 CPUBASED=4 STRIFE=1 YCSBTZIPF='$zz' YCSBSIZE='$size' YCSBCOL='$col'' 
        make -j $TARGET $FLAGS 
        mv $TARGET $BINARY

        # cd /home/ljr/tpcc/TxSys/script/
        # ./ycsb_pic_zipf.sh $ww $zz 51

        # NetTx
        cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
        ./hot_pic_zipf_partition.sh $ww $zz $size $col
        
        cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 

        ./ycsb_bench-net -t20 -mA -idefault -g 

        ssh root@10.0.2.111 "pkill NICTX_client"
        ssh root@10.0.2.113 "pkill NICTX_client"
        # rm $TARGET
        rm $BINARY 
    done
done

mv /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto/tp_listen_schism.txt /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/tp_listen_schism.txt
mv /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test/latency_ycsb_latency.txt /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/latency_dynamic_schism_lat.txt

./mount_hugepages.sh 0