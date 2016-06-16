
---> Users, developers, read also the HOWTO section it contains friendly FAQ how to use/develop/hack this :) <---

Galaxy42 - Small experimental network in category of Meshnet, VPN, IP-hash.

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



=== HOWTO ===

Q: How to develop Galaxy42?
A: [devel] Read most of questions here first, including [devel.demo], [devel.contact].

Q: How to use Galaxy42?
A: [use] The program is in development, it's not yet suitable for any end-user. See [devel] if you are developer.

Q: How to contact developers?
A: [devel.contact] Contact us on IRC server icann.irc.meshnet.pl on channel #meshnet or #antinet (that for now is also about Galaxy42).

Q: How to see a demo how program works?
A: [devel.demo] First build the program, and then choose one of methods:

1. Run program as ./tunserver.elf --devel --demo foo to run test foo. Existing tests include test "foo",
test "bar", and other real tests.
[TODO] You can see list of the possible tests by running it with "help" demo,
so as:
./tunserver.elf --devel --demo help

2. In directory ./config crete a file config/demo.conf. In this file write a single line like "demo=bar";
Then run the program with "make run" or with "./tunserver.elf --devel"
It should run a demo "bar" and write bar on screen (read the debug messages).
If you change that text in file to "demo=foo" then it runs test Foo.
If something does not work then see debug to figure it out.
You can now write other demo name in the config file and run tests again.
Set demo to "list" or "help" to see list of possible demo options [TODO].

3. If you just run program with --devel it will run the hardcoded demo,
as set in code in variable g_demoname_default.
If you start working on a new branch you could set it there to other value
to run your test - see [devel.newthing]

Q: How to develop something new?
A: [devel.newthing] (First read [devel]) and then create git branch named like "wip_galaxy_SOMENAME"
where SOMENAME is a nice name summarizing on what you do work.
Then create also a demo for it (see how other demos are done, see function run_mode_developer,
see demo developer_tests::wip_galaxy_route_doublestar that can be used in simulation of few nodes etc)

Q: How to debug a crash, memory leak, etc, how to run in valgrind/gdb
A: [debug.nocap] You can use gdb and valgrind, but if you have a problem there that the CAP privilages level
block your debug tool from working (as is the case with some hardened platforms) then if you can run
a test, or a demo [devel.demo] that does not require net CAP privilages etc,
then use the binary file that is not trying to get CAP rights,
the file nocap-* binary, e.g.: nocap-tunserver.elf
e.g.: valgrind  ./nocap-tunserver.elf --devel
e.g.: valgrind  ./nocap-tunserver.elf --devel  --demo foo




