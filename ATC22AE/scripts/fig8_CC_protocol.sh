

thread=(2 4 6 8 10 12 14 16 18 20)

catdata(){
    echo "---$1---" >> ATC_fig8
    for((j=0;j<${#thread[*]};j+=1));
    do
        num=${thread[$j]}
        cat ATC_p4_$1.txt | grep "THREAD($num) throughput5" >> ATC_fig8
    done
}

echo "clean the old file"
cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
rm ATC_p4_*.txt

cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto
./ATC_p4_occ.sh
./ATC_p4_tictoc.sh
./ATC_p4_cicada.sh
./ATC_p4_2pl_new_order_only.sh

cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/

echo "---Throughput: FORMAT [total txns, abort rate]---" >> ATC_fig8
echo "---THREAD: [2 4 6 8 10 12 14 16 18 20]---" >> ATC_fig8

echo "-----------fig8 OCC-----------" >> ATC_fig8
catdata occ_alnico
catdata occ_central
catdata occ_colocate
catdata occ_NetSTO

echo "-----------fig8 tictoc----------" >> ATC_fig8
catdata tictoc_alnico
catdata tictoc_central
catdata tictoc_colocate
catdata tictoc_NetSTO

echo "-----------fig8 Cicada-----------" >> ATC_fig8
catdata cicada_alnico
catdata cicada_central
catdata cicada_colocate
catdata cicada_NetSTO


echo "-----------fig8 2pl----------" >> ATC_fig8
catdata 2pl_alnico
catdata 2pl_central
catdata 2pl_colocate
catdata 2pl_NetSTO

