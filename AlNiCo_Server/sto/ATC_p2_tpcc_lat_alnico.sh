
TARGET=tpcc_bench
BINARY=tpcc_bench-occ



./mount_hugepages.sh 49152
# thread=(2 4 6 8 10 12 14 16 18 20)
client=(6 8 12 16 20 24)
# client=(8 24)
async=(2 3)
# async=(3)

cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test
rm latency_tpcc_new_order.txt
rm latency_tpcc_delivery.txt

cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto

for((i=0;i<${#client[*]};i+=1));
do
    nn=${client[$i]}
    FLAGS='NDEBUG=1 OBSERVE_C_BALANCE=1 FINE_GRAINED=1 INLINED_VERSIONS=1 MAX_THREAD=20 ASYNCTXNUM=1 CLIENT_THREAD='$nn''
    make -j $TARGET $FLAGS 
    mv $TARGET $BINARY

    cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script

    ./tpcc_pic_cpu_client.sh 20 100 $nn 1

    cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 

    ./tpcc_bench-occ -t20 -idefault -x  -w2 -r1000  | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p2_tpcc_lat_alnico.txt

    ssh root@10.0.2.111 "pkill NICTX_client"
    ssh root@10.0.2.113 "pkill NICTX_client"
    rm $TARGET
    rm $BINARY 
done


for((i=0;i<${#async[*]};i+=1));
do
    aa=${async[$i]}
    # make clean
    FLAGS='NDEBUG=1 OBSERVE_C_BALANCE=1 FINE_GRAINED=1 INLINED_VERSIONS=1 MAX_THREAD=20 ASYNCTXNUM='$aa' CLIENT_THREAD=24'
    make -j $TARGET $FLAGS 
    mv $TARGET $BINARY

    cd /home/ljr/AlNiCo-ATC22/Clients/NetTx/script
    ./tpcc_pic_cpu_client.sh 20 100 24 $aa

    cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 

    ./tpcc_bench-occ -t20 -idefault -x  -w2 -r1000  | tee -a /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/ATC_p2_tpcc_lat_alnico.txt
    
    ssh root@10.0.2.111 "pkill NICTX_client"
    ssh root@10.0.2.113 "pkill NICTX_client"
    rm $TARGET
    rm $BINARY 
done


mv /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test/latency_tpcc_new_order.txt /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/latency_tpcc_new_order_alnico.txt
mv /home/ljr/AlNiCo-ATC22/Clients/NetTx/build/test/latency_tpcc_delivery.txt /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/latency_tpcc_delivery_alnico.txt

./mount_hugepages.sh 0