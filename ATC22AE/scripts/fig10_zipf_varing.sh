




echo "clean the old file"
cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
rm ATC_p6_*.txt
# rm ATC_fig9

# zipf=(0 0.2 0.4 0.6 0.8 1.0 1.2 1.4)

cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto
./ATC_p6_varing_zipf_alnico.sh
./ATC_p6_varing_zipf_NetSTO.sh
./ATC_p6_varing_zipf_rate.sh

cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/

echo "-----------fig10 tp-----------" >> ATC_fig10
echo "---ZIPF: [0 0.2 0.4 0.6 0.8 1.0 1.2 1.4]---" >> ATC_fig10
echo "---zipf tp alnico---" >> ATC_fig10
cat ATC_p6_varing_zipf_alnico.txt | grep "throughput4" | tee -a ./ATC_fig10

echo "---zipf tp NetSTO---" >> ATC_fig10
cat ATC_p6_varing_zipf_NetSTO.txt | grep "throughput4" | tee -a ./ATC_fig10

echo "---zipf contention detection accuracy rate---" >> ATC_fig10
cat ATC_p6_varing_zipf_rate.txt | tee -a ./ATC_fig10