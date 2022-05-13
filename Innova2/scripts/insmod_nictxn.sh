#!/bin/bash

insmod /home/ljr/dma/simple-dma/dma_drv.ko
insmod /home/ljr/dma/io_peer_mem/io_peer_mem.ko
sysctl vm.nr_hugepages=4096
ssh root@10.0.2.111 "sysctl vm.nr_hugepages=4096"
ssh root@10.0.2.113 "sysctl vm.nr_hugepages=4096"
