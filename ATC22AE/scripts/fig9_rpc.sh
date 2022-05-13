


colll=(128 256 512 1024 2048 4096 8192)

echo "clean the old file"
cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
rm ATC_p5_*.txt
rm latencyn*.txt

cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto
./ATC_p5_overhead_alnico_tp.sh
./ATC_p5_overhead_RPC_tp.sh
./ATC_p5_overhead_alnico_lat.sh
./ATC_p5_overhead_RPC_lat.sh

func(){
    echo "---$1 latency: FORMAT [avg,50,90,99,999]---" >> ATC_fig9 
    echo "---SIZE: [128 256 512 1024 2048 4096 8192]---" >> ATC_fig9
    for((j=0;j<${#colll[*]};j+=1));
    do
        col=${colll[$j]} 
        cat latency$1_rpc_$col.txt | grep avg,50,90,99,999 | tee -a ./ATC_fig9
    done 
}

cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/

echo "-----------fig9 tp-----------" >> ATC_fig9

echo "---nic_rpc---" >> ATC_fig9
cat ATC_p5_overhead_alnico_tp.txt | grep "throughput4" | tee -a ./ATC_fig9

echo "---cpu_rpc---" >> ATC_fig9
cat ATC_p5_overhead_RPC_tp.txt | grep "throughput4" | tee -a ./ATC_fig9

echo "-----------fig9 lat-----------" >> ATC_fig9

func nic
func null
