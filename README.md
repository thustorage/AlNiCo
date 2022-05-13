# AlNiCo

This repository contains the artifact for the ATC'22 paper: "AlNiCo: SmartNIC-accelerated Contention-aware Request Scheduling for Transaction Processing".

## Evaluation Environment

### Hardware

We evaluate all experiments in three machines. One is a server, two are clients.
* Server (10.0.2.110) uses the dual-port 25Gbps Innova-2 SmartNIC (our FPGA image has been burned to it).
* Two Clients (10.0.2.111 and 10.0.2.113) use the 100Gbps Mellanox ConnectX-5.

Each machine has two 12-core Xeon E5-2650 v4 2.20GHz CPU sockets, PCIe 3.0 interface, and 128GB memory. 

## Before the start

**Access to our evaluation server:**

See HOTCRP for the access information.
After logging into our server, run `cd /home/ljr/AlNiCo-ATC22/`. This is path of our codebases.

Before evaluating AlNiCo, please run `w` to check if anyone else is using it, to avoid resource contention. And please close ssh connection when not conducting evalation. If you have any question, please contact with me via lijr19@mails.tsinghua.edu.cn


## Directory structure

```
AlNiCo-ATC22
|---- ATC22AE 
    |---- outfile                   # evaluation output files
    |---- scripts                   # main evaluation scripts
|---- AlNiCo_Server/sto        # codes of the transaction server
|---- Clients/NetTx            # codes of the transaction client
|---- Innova2                  
    |---- FPGA                      # on-nic scheduler project
    |---- scheduler                 # HLS codes of scheduler
    |---- io_peer_mem               # io_peer_mem driver
    |---- simple-dma                # DMA driver
    |---- scripts                   # Using Innova2 on our machine
```
## Functionality of codes

* `Clients/rdma_rpc/, AlNiCo_Server/nic-sched/rpc_*.[h/cpp]`: network overview (RDMA-based RPC) (Sec3.1).
* `Clinets/transaction/Transaction.h, Clinets/NicTxn`: of issue transactions via generating tx procedure (Sec3.2.1).
* `Innova2/FPGA/scheduler/`: on-nic scheduler in FPGA (Sec3.2.2 - Sec3.2.4).
* `AlNiCo_Server/nic-sched/model_update.h`: servers to poll new requests from NIC and to update the feedback to the FPGA buffer (Sec3.2.1 Sec3.3.2).
* `AlNiCo_Server/nic-sched/worker_buf.[h/cpp]`: interfaces of worker to rocord the aborted/blocking keys.


## kick-the-tires 

0. ssh to our server

1. run a simple evaluation

```shell
scl enable devtoolset-7 zsh # env for STO
cd /home/ljr/AlNiCo-ATC22/ATC22AE/scripts/
./fig5_a.sh # you can get the data of fig5_a in ATC_fig5
```


**Note: please run `scl enable devtoolset-7 zsh` at each time you login to the server, to set the default compiler.** 


## Main Figures

### Scripts 

**You need to use the scripts directly from the [ATC22AE/scripts]. These scripts will build and conduct experiments automatically.**

```
|---- clean_data_all.sh                 # remove the outfile 
|---- fig5_throughput.sh                # overall throughput (Sec5.2)
|---- fig6_tpcc_lat.sh                  # tpcc latency (Sec5.2)
|---- fig7_HOT_dynamic.sh               # dynamic workloads HOT (Sec5.3)
|---- fig8_CC_protocol.sh               # four CC protocols (Sec5.4 Sec5.5)
|---- fig9_rpc.sh                       # null rpc throughput and latency (Sec5.6)
|---- fig10_zipf_varing.sh              # YCSB with Zipf varying (Sec5.6)
```

First, run `cd ATC22AE/scripts`

Second, run `scl enable devtoolset-7 zsh`

Third, run `./clean_data_all.sh` to remove the output in ATC22AE/outfile.

### Figuer5 

execute `./fig5_throughout.sh`, which produces `../outfile/ATC_fig5` includes the throughput of TPC-C (fig5_a, fig5_b and fig5_c) and YCSB-T (NetSTO_d_e_f, schism_d_e_f, strife_d_e_f and alnico_d_e_f). 

About 40 mins.

### Figure7

execute `./fig7_HOT_dynamic.sh`, which produces `../outfile/tp_listen_XXX.txt` (XXX is the system name: AlNiCo, NetSTO, Schism, Strife) and corresponding `over_time_XXX.pdf`: the throughput over time under YCSB-HOT (The first 4s is system warm-up time). 

About 15 mins.

### Other Figures

#### Figure6 

execute `./fig6_tpcc_lat.sh`, which produces `../outfile/ATC_fig6`: the total throughput, New-order lat, and Delivery lat in TPC-C.

About 60 mins.

#### Figure8

execute `./fig8_CC_protocol.sh`, which produces `../outfile/ATC_fig8`: the throughput of TPC-C using different scheduling versions (AlNiCo, NetSTO, +Co-worker-scheduler, Dedicated schedulers) with four concurrency control protocols (OCC tictoc Cicada 2pl) (2 warehouses).

About 4 hours for all evaluation in Figure8. Also, you can execute `./fig_CC_single.sh {1|2|3|4}` (1: OCC, 2: TicToc, 3: Cicada, 4: 2PL) to run the evaluation of specific protocols. (Each one about 1 hour).

#### Figure9

execute `./fig9_rpc.sh`, which produces `../outfile/ATC_fig9`: the throughput and latency of RPC performance with varying request sizes.

About 20-30 mins.

#### Figure10

execute `./fig10_zipf_varing.sh`, which produces `../outfile/ATC_fig10`: the throughput and contention detection accuracy rate of YCSB-T (w:50) workloads with different Zipf $\theta $.

About 20-30 mins.