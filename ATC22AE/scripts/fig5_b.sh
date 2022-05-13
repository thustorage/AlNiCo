


echo "clean the old file"
cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
rm ATC_p1_b_tpcc_zipf_*.txt


echo "running"
cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 
./ATC_p1_b_tpcc_zipf_NetSTO.sh
./ATC_p1_b_tpcc_zipf_schism.sh
./ATC_p1_b_tpcc_zipf_strife.sh
./ATC_p1_b_tpcc_zipf_alnico.sh

echo "file is in /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/"

cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/



echo "-----------fig5_b-----------" >> ATC_fig5
echo "FORMAT [total txns, abort rate]" >> ATC_fig5
echo "ORDER [NetSTO, schism, strife, alnico]" >> ATC_fig5
cat ATC_p1_b_tpcc_zipf_NetSTO.txt | grep "throughput5" | tee -a ./ATC_fig5
cat ATC_p1_b_tpcc_zipf_schism.txt | grep "throughput5" | tee -a ./ATC_fig5
cat ATC_p1_b_tpcc_zipf_strife.txt | grep "throughput5" | tee -a ./ATC_fig5
cat ATC_p1_b_tpcc_zipf_alnico.txt | grep "throughput5" | tee -a ./ATC_fig5
echo "=========================================================" >> ATC_fig5
echo " " >> ATC_fig5