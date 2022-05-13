TARGET=ycsb_bench
BINARY=ycsb_bench-net
size=16
# col=20


./mount_hugepages.sh 49152

cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test
rm latencynic_*.txt
cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto

source ./ATC_p5_source.sh


for((j=0;j<${#colll[*]};j+=1));
    do
        ww=50
        zz=0.99
        col=${colll[$j]} 
        FLAGS='NDEBUG=1 MAX_THREAD=20 YCSBplusT=1 NULLRPC=1 YCSBTZIPF='$zz' YCSBSIZE='$size' ASYNCTXNUM=1 YCSBCOL=20 CLIENT_THREAD=10' 
        make -j $TARGET $FLAGS 
        mv $TARGET $BINARY

        # cd /home/ljr/tpcc/TxSys/script/
        # ./ycsb_pic_zipf.sh $ww $zz 51
        # NetTx

        cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
        # ./ycsb_pic_zipf.sh $ww $zz $size $col
        ./p5_nic_lat.sh $ww $zz $size $col
        
        cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 

        # gdb ./ycsb_bench-net
        # gdb -ex=r --args  ./ycsb_bench-net -t20 -mA -idefault -g | tee -a p5_nic.txt
        # ./ycsb_bench-net -t20 -mA -idefault -g | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p5_overhead_alnico_lat.txt
        ./ycsb_bench-net -t20 -mA -idefault -g

        ssh root@10.0.2.111 "pkill NICTX_client"
        ssh root@10.0.2.113 "pkill NICTX_client"
        # rm $TARGET
        rm $BINARY 
    done

mv /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test/latencynic_*.txt  /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/

./mount_hugepages.sh 0