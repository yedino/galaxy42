
One time installation
=====================

install the programs in:

start_sim_namespaces/

(This is usually availalbe as installer-option, e.g. in linux in the top install.sh)

After each boot
===============

Create the virtual containers (namespaces) with the virtual cards:

./start-containers.sh a 20

Where "a" is the name of group, and 20 is the number of containers to create.

Expected result: it will show lots of debug, and from host (the normal console)
you can see many created virtual cards in command `ip a`, e.g.:

```
335: prva3xH: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether 7a:29:03:85:ec:69 brd ff:ff:ff:ff:ff:ff
    inet 192.168.3.61/30 scope global prva3xH
       valid_lft forever preferred_lft forever
    inet6 fe80::7829:3ff:fe85:ec69/64 scope link
       valid_lft forever preferred_lft forever
337: prva4xH: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether 9e:4b:b8:8d:24:53 brd ff:ff:ff:ff:ff:ff
    inet 192.168.4.61/30 scope global prva4xH
       valid_lft forever preferred_lft forever
    inet6 fe80::9c4b:b8ff:fe8d:2453/64 scope link
       valid_lft forever preferred_lft forever
```

For each execution of test
==========================

You can take any shell, and there enter one of the created namespaces using:
sudo netpriv_enter a5
where "a5" means: "a" is the name of group as you created, and "5" is the number of container.

In console where you executed this command you will now see other network stack.
Test it there, e.g.:

ip a (should show only lo card, and a virtual card)
ping meshnet.pl (or any other ping outside, should work)
ping 8.8.8.8 (if ping by name is not working, then you could need to edit /etc/resolve.conf or improve the scripts to configure better DNS resolver in the containers)

