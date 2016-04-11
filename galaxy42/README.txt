
Small experimental network in category of Meshnet, VPN, IP-hash.

Title: Galaxy42
Status: Experimental, pre-beta, NOT reviewed, NOT secure.

Do not use this network for any purpose at all (yet!) it contains
bugs, probably including vulns. Could be ok to try it in a VM.

Intended ONLY for developers (for now).

Finall version will be aimed to all users on Windows, Linux, Mac and
other systems.


NETWORK TYPE:

* Meshnet - it will be possible to set up the net with minimal config 
(and later zeroconf), no nodes are centralized.

* VPN - it will provide end-to-end encryption and authentication, 
and IP access even to places with no own traditional Internet-accessible IP.

* IP-Hash - the IP address is hash (e.g. of public key) and therefore
it can be assigned in some decentralized way (e.g. created and owned by user,
as hash of his own public-key to which only he has private-key).

This network is similar in principle to Cjdns network.


GOALS:

We aim with it to test possible solutions to some challenges discovered in Cjdns:

1) Not optimal speed in even 1-to-1 connection over Gigabit link (e.g. on Linux).
2) Not close to optimal global speed, probably because of routing creating not so good routes.
3) Not stable connections.

Optionally, for the main Antinet project we plan to test:

4) Possible integrated micro "payments" between nodes, especially "paying" just with
own resources (I route for you - if you route for me) to allow node owner to demand
cooperation from other participants of the network if they want to get his help.



