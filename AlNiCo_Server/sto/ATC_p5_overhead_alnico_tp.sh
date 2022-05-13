TARGET=ycsb_bench
BINARY=ycsb_bench-net
size=16
# col=20

FLAGS='NDEBUG=1'

./mount_hugepages.sh 49152

# ./tpcc_bench-occ -t21 -i2pl -n -w2 -r1000
# zipf=(0.99)
# write=(5)
source ./ATC_p5_source.sh
# zipf=(0.8)
# write=(5)
for((j=0;j<${#colll[*]};j+=1));
    do
        ww=50
        zz=0.99
        col=${colll[$j]} 
        FLAGS='NDEBUG=1 MAX_THREAD=20 YCSBplusT=1 NULLRPC=1  YCSBTZIPF='$zz' YCSBSIZE='$size' ASYNCTXNUM=4 YCSBCOL=20 CLIENT_THREAD=24' 
        make -j $TARGET $FLAGS 
        mv $TARGET $BINARY

        # cd /home/ljr/tpcc/TxSys/script/
        # ./ycsb_pic_zipf.sh $ww $zz 51

        # NetTx
        cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
        # ./ycsb_pic_zipf.sh $ww $zz $size $col
        ./p5_nic.sh $ww $zz $size $col
        

        cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 

        # gdb ./ycsb_bench-net
        # gdb -ex=r --args  ./ycsb_bench-net -t20 -mA -idefault -g | tee -a p5_nic.txt
        ./ycsb_bench-net -t20 -mA -idefault -g | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p5_overhead_alnico_tp.txt

        ssh root@10.0.2.111 "pkill NICTX_client"
        ssh root@10.0.2.113 "pkill NICTX_client"
        # rm $TARGET
        rm $BINARY 
    done

./mount_hugepages.sh 0