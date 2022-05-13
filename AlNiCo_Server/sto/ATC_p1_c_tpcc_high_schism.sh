

TARGET=tpcc_bench
BINARY=tpcc_bench-occ
# FLAGS='NDEBUG=1 OBSERVE_C_BALANCE=1 FINE_GRAINED=1 INLINED_VERSIONS=1 MAX_THREAD=20'
# make -j $TARGET $FLAGS 



./mount_hugepages.sh 0
./mount_hugepages.sh 49152
# thread=(2 4 6 8 10 12 14 16 18 20)
# client=(6 8 16 20 24)
client=(24)
# async=(2 3 4)
async=(4)



for((i=0;i<${#async[*]};i+=1));
do
    aa=${async[$i]}
    # make clean
    FLAGS='NDEBUG=1 OBSERVE_C_BALANCE=1 FINE_GRAINED=1  CPUBASED=4 STRIFE=1 INLINED_VERSIONS=1 MAX_THREAD=20 ASYNCTXNUM='$aa' CLIENT_THREAD=24'
    make -j $TARGET $FLAGS 
    mv $TARGET $BINARY

    cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
    ./tpcc_pic_cpu_client_SPNIC2.sh 20 100 24 $aa

    cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 

    ./tpcc_bench-occ -t20 -idefault -x -w2 -r1000  | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p1_c_tpcc_high_schism.txt
    
    ssh root@10.0.2.111 "pkill NICTX_client"
    ssh root@10.0.2.113 "pkill NICTX_client"
    # rm $TARGET
    # rm $BINARY 
done

./mount_hugepages.sh 0