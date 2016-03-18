
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

One time setup of the setcap program - SETCAP

as root, install the scripts including "setcap_net_admin", by doing following:
as root take the files from here:
cp -i -v -r script/root /root/
  *** now review the scripts: install, and the setcap_net_admin there ***
and run the installer, from that directory:
cd /root/root/ && ./install
and then edit sudo rules, e.g. with command "visudo" as instructed by the install program.

After this, the user that you configured should be able to run the command like setcap_net_admin,
this command is already used by the build scripts you have here.

---

Usage: 

build the code:

./build
(now the programs that need it will request proper capabilities for themselves using e.g. setcap_net_admin
this happens automatically)

Then run:
  on node A (192.168.0.104) run: ./tunproxy.bin -p  192.168.0.57:9004 -s 9004 # this is the server
  on node B (192.168.0.57)  run: ./tunproxy.bin -p 192.168.0.104:9004 -c 192.168.0.104:9004
Optionally also:
  on node C (192.168.0.107) run: ./tunproxy.bin -p 192.168.0.104:9004 -c 192.168.0.104:9004
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


