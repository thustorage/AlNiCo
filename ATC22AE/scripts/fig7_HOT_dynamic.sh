

func(){
    # echo "-----------fig7 $1-----------" >> ATC_fig7
    gnuplot -e "OUT='over_time_$1.pdf'" -e "IN='tp_listen_$1.txt'" ../scripts/plot.gp
    echo "---the dynamic tp of $1 is in tp_listen_$1---" >> ATC_fig7
}

echo "clean the old file"
cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
rm tp_listen_*.txt
rm ATC_fig7
rm latency_dynamic_*

cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto 

./ATC_p3_dynamic_alnico.sh
./ATC_p3_dynamic_NetSTO.sh
./ATC_p3_dynamic_schism.sh
./ATC_p3_dynamic_strife.sh


cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
func alnico
func NetSTO
func schism
func strife