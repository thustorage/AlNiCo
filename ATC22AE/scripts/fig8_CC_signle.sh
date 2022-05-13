

thread=(2 4 6 8 10 12 14 16 18 20)

catdata(){
    echo "---$1---" >> ATC_fig8
    for((j=0;j<${#thread[*]};j+=1));
    do
        num=${thread[$j]}
        cat ATC_p4_$1.txt | grep "THREAD($num) throughput5" >> ATC_fig8
    done
}

help() {
    echo "Usage: $0 <CC protocol>"
    echo "  <CC protocol>: 1: OCC, 2: TicToc, 3: Cicada, 4: 2PL"
}

run() {
    echo "clean the old file"
    cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
    rm ATC_p4_$1_*.txt

    cd /home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto
    if [[ $1 == 2pl ]]; then
        ./ATC_p4_2pl_new_order_only.sh
    else
        ./ATC_p4_$1.sh
    fi 

    cd /home/ljr/AlNiCo-ATC22/ATC22AE/outfile/
    echo "---Throughput: FORMAT [total txns, abort rate]---" >> ATC_fig8
    echo "---THREAD: [2 4 6 8 10 12 14 16 18 20]---" >> ATC_fig8
    echo "-----------fig8 $1-----------" >> ATC_fig8
    catdata $1_alnico
    catdata $1_central
    catdata $1_colocate
    catdata $1_NetSTO
}

if [[ $# != 1 ]]; then
    help
    exit
fi 

if [[ $1 == 1 ]]; then
run occ

elif [[ $1 == 2 ]]; then
run tictoc


elif [[ $1 == 3 ]]; then
run cicada

elif [[ $1 == 4 ]]; then
run 2pl

fi


