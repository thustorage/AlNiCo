# Our Experience with Mellanox Innova-2
## 1.Configure Switches

### split the port for Innova-2 NIC
```shell
# ssh: the console of switches (Mellanox Onyx MSN2700)
# use the port X (X: an odd number, e.g., 31)
enable
configure terminal

interface ethernet 1/31
shutdown 
exit
interface ethernet 1/32
shutdown 
exit

interface ethernet 1/31
module-type qsfp-split-4
```

### configure 

```
# login Management Console website
# NICs and ROCE switch must use the same MTU (4200 in our experiments) 
Ports > port id > modify MTU to 4200 > Apply 
```

## 2.Configure NICs 

### 2.1 Burn your FPGA project to Innova2

Mellanox 

### 2.2 Insmod DMA driver and P2P driver

```shell
cd /path/Innova2/scripts
./insmod_nictxn.sh
```
- DMA driver allows to FPGA DMA memory.
    - Code: /path/Innova2/simple-dma.
- P2P (i.e., PCIe peer-to-peer communication) driver allows the NIC ASIC read/write FPGA, [Mellanox P2P](https://support.mellanox.com/s/article/howto-implement-peerdirect-client-using-mlnx-ofed).
    - Code: /path/Innova2/io_peer_mem.



