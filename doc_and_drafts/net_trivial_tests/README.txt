
This will crete a trivial VPN like setup.

---

IP addresses.
You have nodes A, B, and maybe more.

They have normal IP addresses over which they will PEER together (e.g. in LAN) and they should be 
connected together in LAN (or over internet or any other IP. Currently probably it is required to be ipv4).

They will also give them our own "virtual" IP addresses.
This virtual addresses should be IPv6, in fd00/8, or better in fd42/16.

We asume here:
node A has LAN IP for peering 192.168.0.104, and will be configured to have virtual IP fd42:0db8:1::
node B has LAN IP for peering 192.168.0.57,  and will be configured to have virtual IP fd42:0db8:2::
if needed then more:
node C has LAN IP for peering 192.168.0.107, and will be configured to have virtual IP fd42:0db8:3::

---

One time setup:

Copy somewhere to root e.g. /root/, and customize the script "keep-fd42-config"
enter there your IP and directory in which you work on this code here,
then run this file - as ROOT, run it as bash command:
  nohup keep-fd42-config &>/dev/null  &
(in the proper directory of course) - it will run in background.
You need to restart this script after reboot. You can stop it with killall command for example.
In future this should be done in other way.

---

Using this tests: build all:
./build


After a secon, the script keep-fd42-config running in background should give you CAP flags so that you can run the program without need for root.

Then run:
  on node A (192.168.0.104) run: ./tunproxy -p  192.168.0.57:9004 -s 9004 # this is the server
  on node B (192.168.0.57)  run: ./tunproxy -p 192.168.0.104:9004 -c 192.168.0.104:9004
Optionally also:
  on node C (192.168.0.107) run: ./tunproxy -p 192.168.0.104:9004 -c 192.168.0.104:9004
(of course adjust the IPs of nodes A/B to be as you have in your setup).
Nodes IP .104 and .57 will peer on port UDP 9004 and both will listen, and both will send to eachother's such port.

Verify: ip a should show the new tun device, with the IP address like "fd42:0db8:1::" as you configured.
Verify: from node A you can ping node B by command: ping6 fd42:0db8:2::
Verify: from node B you can ping node A by command: ping6 fd42:0db8:1::
(Ping between A<->C will not work untill we implement array of peers on server)
(Ping between B<->C and other peers will not work untill we implement direct at least in star layout)

---

Running bandwidth tests:
	on node A run: ./server_udp.bin 12006 ipv6  # this will be te server of tests, it will print statistics
	on node B run: ./client_udp.bin fd42:0db8:1:: 12006 # this will be a client of test
optionally:
	on node C run: ./client_udp.bin fd42:0db8:1:: 12006 # this will be a client of test
	and same on other test nodes


