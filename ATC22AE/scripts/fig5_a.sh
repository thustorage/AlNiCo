



# clean the old file

echo "clean the old file"
cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
rm ATC_p1_a_tpcc_low_*.txt


echo "running"
cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 
./ATC_p1_a_tpcc_low_NetSTO.sh
./ATC_p1_a_tpcc_low_schism.sh
./ATC_p1_a_tpcc_low_strife.sh
./ATC_p1_a_tpcc_low_alnico.sh

echo "file is in /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/"

cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/

echo "-----------fig5_a-----------" >> ATC_fig5
echo "FORMAT [total txns, abort rate]" >> ATC_fig5
echo "ORDER [NetSTO, schism, strife, alnico]" >> ATC_fig5
cat ATC_p1_a_tpcc_low_NetSTO.txt | grep "throughput5" | tee -a ./ATC_fig5
cat ATC_p1_a_tpcc_low_schism.txt | grep "throughput5" | tee -a ./ATC_fig5
cat ATC_p1_a_tpcc_low_strife.txt | grep "throughput5" | tee -a ./ATC_fig5
cat ATC_p1_a_tpcc_low_alnico.txt | grep "throughput5" | tee -a ./ATC_fig5
echo "=========================================================" >> ATC_fig5
echo " " >> ATC_fig5


