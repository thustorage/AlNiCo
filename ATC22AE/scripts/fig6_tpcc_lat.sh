

func(){
    echo "-----------fig6 $1-----------" >> ATC_fig6

    echo "---Throughput: FORMAT [total txns, abort rate]---" >> ATC_fig6
    cat ATC_p2_tpcc_lat_$1.txt | grep "throughput5" | tee -a ./ATC_fig6

    echo "---new_order latency: FORMAT [avg,50,90,99,999]---" >> ATC_fig6
    cat latency_tpcc_new_order_$1.txt | grep avg,50,90,99,999 | tee -a ./ATC_fig6

    echo "---new_order latency: FORMAT [avg,50,90,99,999]---" >> ATC_fig6 
    cat latency_tpcc_delivery_$1.txt | grep avg,50,90,99,999 | tee -a ./ATC_fig6
}

echo "clean the old file"
cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
rm ATC_p2_tpcc_lat_*.txt
rm ATC_fig6

cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 
./ATC_p2_tpcc_lat_alnico.sh
./ATC_p2_tpcc_lat_NetSTO.sh
./ATC_p2_tpcc_lat_schism.sh
./ATC_p2_tpcc_lat_strife.sh


cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
func alnico
func NetSTO
func schism
func strife