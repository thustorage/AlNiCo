
#!/bin/bash
# for smartNIC
set -x
name=132

ssh root@10.0.2.112 "ifconfig enp'$name's0f0 192.168.5.112/21 up"
ssh root@10.0.2.112 "ifconfig enp'$name's0f0 mtu 4200"


ssh root@10.0.2.112 "ifconfig enp'$name's0f1 192.168.6.112/21 up"
ssh root@10.0.2.112 "ifconfig enp'$name's0f1 mtu 4200"

# ssh root@10.0.2.112 "ifconfig ens2 192.168.4.112/21 up"
# ssh root@10.0.2.112 "ifconfig ens2 mtu 4200"

echo 112over

ssh root@10.0.2.111 "ifconfig ens2 192.168.4.111/21 up"
ssh root@10.0.2.111 "ifconfig ens2 mtu 4200"
ssh root@10.0.2.113 "ifconfig ens2 192.168.4.113/21 up"
ssh root@10.0.2.113 "ifconfig ens2 mtu 4200"