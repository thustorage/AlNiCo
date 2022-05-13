

catdata(){
    echo "-----------fig5_d_e_f-----------" >> ATC_fig5
    echo "FORMAT [total txns, abort rate]" >> ATC_fig5
    echo "---$1 [w: 5 50 95]---" >> ATC_fig5
    cat ATC_p1_def_ycsb_$1.txt | grep "throughput5" | tee -a ./ATC_fig5
    echo " " >> ATC_fig5
}


echo "clean the old file"
cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
rm ATC_p1_def_ycsb_*.txt

echo "running"
cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 
./ATC_p1_def_ycsb_NetSTO.sh
./ATC_p1_def_ycsb_schism.sh
./ATC_p1_def_ycsb_strife.sh
./ATC_p1_def_ycsb_alnico.sh


cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
catdata NetSTO
catdata schism
catdata strife
catdata alnico
echo "=========================================================" >> ATC_fig5


