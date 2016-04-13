
One time installation
=====================

install the programs in:

start_sim_namespaces/

After each boot
===============

Create the virtual containers (namespaces) with the virtual cards:

./start-containers.sh a 20

Where "a" is the name of group, and 20 is the number of containers to create.

For each execution of test
==========================

You can take a shell and enter one of the created namespaces using:
sudo netpriv_enter a5
where "a5" means: "a" is the name of group as you created, and "5" is the number of container.
In console where you executed this command you will now see other network stack.
Test with:

ip a (should show only lo card, and a virtual card)
ping meshnet.pl (or any other ping outside, should work)
ping 8.8.8.8 (if ping by name is not working, then you could need to edit /etc/resolve.conf or improve the scripts to configure better DNS resolver in the containers)

