
TARGET=tpcc_bench
BINARY=tpcc_bench-tictoc


./mount_hugepages.sh 49152
source ./ATC_p4_source.sh

for((i=0;i<${#thread[*]};i+=1));
do
    tt=${thread[$i]}
    # make clean
    FLAGS='NDEBUG=1 OBSERVE_C_BALANCE=1 FINE_GRAINED=1 INLINED_VERSIONS=1 MAX_THREAD='$tt' ASYNCTXNUM=4 CLIENT_THREAD=24'
    make -j $TARGET $FLAGS 
    mv $TARGET $BINARY

    cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
    ./tpcc_pic_cpu_client.sh $tt 100 24 4

    cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 
    ./tpcc_bench-tictoc -t$tt -idefault -x  -w2 -r1000  | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p4_tictoc_alnico.txt
    
    ssh root@10.0.2.111 "pkill NICTX_client"
    ssh root@10.0.2.113 "pkill NICTX_client"
    # rm $TARGET
    # rm $BINARY 
done


for((i=0;i<${#thread[*]};i+=1));
do
    tt=${thread[$i]}
    # make clean
    FLAGS='NDEBUG=1 OBSERVE_C_BALANCE=1 FINE_GRAINED=1 INLINED_VERSIONS=1 MAX_THREAD='$tt' ASYNCTXNUM=4 CLIENT_THREAD=24 CPUBASED=1 STRIFE=1 BASELINE=1'
    make -j $TARGET $FLAGS 
    mv $TARGET $BINARY

    cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
    ./tpcc_pic_cpu_client_SPNIC2.sh $tt 100 24 4

    cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 


    ./tpcc_bench-tictoc -t$tt -idefault -x  -w2 -r1000  | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p4_tictoc_colocate.txt
    
    ssh root@10.0.2.111 "pkill NICTX_client"
    ssh root@10.0.2.113 "pkill NICTX_client"
    # rm $TARGET
    # rm $BINARY 
done

for((i=0;i<${#thread[*]};i+=1));
do
    tt=${thread[$i]}
    # make clean
    FLAGS='NDEBUG=1 OBSERVE_C_BALANCE=1 FINE_GRAINED=1 INLINED_VERSIONS=1 MAX_THREAD='$tt' ASYNCTXNUM=4 CLIENT_THREAD=24 CPUBASED=2 STRIFE=1 BASELINE=1'
    make -j $TARGET $FLAGS 
    mv $TARGET $BINARY

    cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
    ./tpcc_pic_cpu_client_SPNIC2.sh $tt 100 24 4

    cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 


    ./tpcc_bench-tictoc -t$tt -idefault -x  -w2 -r1000  | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p4_tictoc_central.txt
    
    ssh root@10.0.2.111 "pkill NICTX_client"
    ssh root@10.0.2.113 "pkill NICTX_client"
    # rm $TARGET
    # rm $BINARY 
done



for((i=0;i<${#thread[*]};i+=1));
do
    tt=${thread[$i]}
    # make clean
    FLAGS='NDEBUG=1 OBSERVE_C_BALANCE=1 FINE_GRAINED=1 INLINED_VERSIONS=1 MAX_THREAD='$tt' ASYNCTXNUM=4 CLIENT_THREAD=24 CPUBASED=4 STRIFE=1 BASELINE=1'
    make -j $TARGET $FLAGS 
    mv $TARGET $BINARY

    cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
    ./tpcc_pic_cpu_client_SPNIC2.sh $tt 100 24 4

    cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 


    ./tpcc_bench-tictoc -t$tt -idefault -x  -w2 -r1000  | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p4_tictoc_NetSTO.txt
    
    ssh root@10.0.2.111 "pkill NICTX_client"
    ssh root@10.0.2.113 "pkill NICTX_client"
    # rm $TARGET
    # rm $BINARY 
done



./mount_hugepages.sh 0