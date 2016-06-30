[![Build Status](https://travis-ci.org/yedino/galaxy42.svg?branch=master)](https://travis-ci.org/yedino/galaxy42)
[![Coverage Status](https://coveralls.io/repos/github/yedino/galaxy42/badge.svg?branch=master)](https://coveralls.io/github/yedino/galaxy42?branch=master)
![Pre-pre-alpha](http://p.meshnet.pl/p/tgx.svg)
[![IRC #antinet icann.irc.meshnet.pl](http://p.meshnet.pl/p/tgh.svg)](http://h.forum.meshnet.pl/viewtopic.php?f=14&p=144#p144)

* * *

This program is not yet ready nor secure, do not use it in this version. It is Copyrighted, see [LICENCE.txt](./LICENCE.txt)

该程序还没有准备好，也不安全，在这个版本中不使用它. 它是有版权的，读文件 [LICENCE.txt](./LICENCE.txt)

* * *

This program creates an secure connection between 2 computers (like VPN).
 * access e.g. your home laptop from work, or connect servers in LAN easily
 * you get *own virtual IPv6* to which you own the private key so no one can impersonate you or steal your address
 * compatible with all programs that just support IPv6
 * authenticated end-to-end
 * encrypted end-to-end
 * only one of the computers needs to have public Internet IP address (in future none will need)
 * (not yet officially released) option to mesh many computers together

To use it from Command Line, simply:

on 1st computer run command: `./tunserver.elf`
the program will start and will **print you on screen what is the virtual IP address** that is assigned to it by our program.

on 2nd computer run command: `./tunserver.elf --peer YOURIP:9042-VIRTUALIP1`
where YOURIP must be the "normal" IP address of 1st computer as seen from 2nd computer, e.g. the Internet address of it,
and VIRTUALIP1 is the virtual IP that was displayed on 1st computer.

That is all!
Now your computers are connected together, try ping6 VIRTUALIP1 from 2nd computer, all connections (browser, ftp, ssh etc)
should work too.

Backup: your **private keys** that give ownership of your virtual IP address by default are in `~/.config/antinet/`
(or just the `galaxy42/wallet` there).

This program will allow to do much much more in future (full public mesh) but this are plans for advanced testers (see below).

* * *

## Security:

The source code is NOT yet reviewed, expect it to have bugs and exploits. For now we recommend to use it only in isolated VM
or better yet on a separated test computer.

### Security of binary install:

Currently we do not yet offer binary install. But once we do, they will be properly signed by root of trust.

### Security of binary auto-updates:

Not implemented yet.

### Security of source code:

You can confirm that source code is indeed created by the developers - git tags are signed, and after them (e.g. on some work in progress branch)
all following git commits (including git merges) are also signed.

* **Root of trust**: all work here will be authorized by main key, that is currently by key:
AB58 9383 1B9A EE6C EE17  6DA0 B4B4 5712 5445 3AF5 (Tigusoft Admin <admin@tigusoft.pl>).
How ever, you should of course first check with some other channels if that is the fingerprint of owner of this project.

* First, import the GPG keys. They are here in dir doc/pubkey/. How ever make sure the commit with keys is itself signed (e.g. git tag) by root of trust. You can import the keys there, though we are responsible ONLY for security of keys listed in gitallsign/ and only for purposes stated there (tag vs commit).

* You can **simply use a tagged version and done** - just check the GPG signature on the git tag (`git tag`, `git tag -v v0........`),
they should be usually signed directly by the root of trust.

Or else, for not-tagged versions:

* The files in gitallsign/ contain lists which GPG key is allowed to be which git Author, and can he sign commits or tags.
(In future this will be automatically checked with our script, for now you must do it by hand).
* Of course check who in fact created this files (git log / git blame) and is it signed by the root of trust. 
* Check if at least the latest git tag is signed (and is it signed by person that is allowed to sign-tags in this project, and does the GPG key
actually match the commit author as seen in git log).
* Each commit (including merge commits) since version you are checking, back to the latest tag, should be also signed (also you should check,
is it signed by person allowed to sign-commits this times, and does everything match ditto).

Script that can help speed up this process is being written, e.g. one good version of it is:
`sha512 3ed9cf88d3d78ced3a7326a05bc954695cb36b5802abca309745196bfe498e679a2f91c1df3e4ffcef852e3a203406959d565f5e78a8364d0438e99a6e9a7ad0 ` of file `antinet/tools/git_pretty_signature.py` in project Antinet. (New versions will be better, though review the code before runnig it anyway).

### Organizations and projects

Galaxy42 - the main network routing.
Antinet - the research project including Galaxy42, simulations for it, tokens for it, and everything else.
Yedino - the bigger entity that manages creation of this projects and controlls most of copyrights and official issues.

* * *
* * *
* * *

Advanced topics - for developers and for future.

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

Q: How to use Galaxy42?
A: [use] See top of this README for instructions - or just run the program and follow instructions on screen.

Q: How to commit my work using Git?
A: {git.model} First see {devel}.
Create a branch, work there using `git commit -s` to sign each commit. Make a pull request.
Project technical maintainer will the git merge -s your code.
Do NOT rebase our code on top of work - or if you do so then you need to:
sign again all commit using e.g. `git reset --hard` and `git cherry-pick` and `git commit --ammend -s`.
See if all commits are signed using e.g. `git log --show-signature`.
We will merge your commit using e.g. `git merge -S --verify-signatures`.
As result, each and every commit (and merge commits) will be GPG signed.
In addition, tags (that we created with `git tag -s`) will be signed too, by proper developer.

Q: How to develop Galaxy42?
A: [devel] Read most of questions here first, including [devel.demo], [devel.contact].

Q: What is the licence?
A: See the LICENCE-by-Antinet.txt file and other such files.

Q: How to develop Galaxy42?
A: [devel] See LICENCE information first. Read most of questions here first, including [devel.demo], [devel.contact].

Q: How to contact developers?
A: [devel.contact] (Disclaimer: this is NOT a contact for legal matters - all such messages will be ignored; Wait for reply at least 12 hours;
On IRC user names are not protected, and do not trust no one. We will never ask you to give us any passwords or run some untrusted not signed code!)
Contact us on IRC server icann.irc.meshnet.pl on channel #meshnet or #antinet (that for now is also about Galaxy42).

Q: How to see a demo how program works?
A: [devel.demo] First build the program, and then choose one of methods:

1. Run program as ./tunserver.elf --devel --demo foo to run test foo. Existing tests include test "foo",
test "bar", and other real tests.
[TODO] You can see list of the possible tests by running it with "help" demo,
so as:
./tunserver.elf --devel --demo help

2. In directory ./config crete a file config/demo.conf. In this file write a single line like `demo=bar`;
Then run the program with `make run` or with `./tunserver.elf --devel`
It should run a demo "bar" and write bar on screen (read the debug messages).
If you change that text in file to `demo=foo` then it runs test Foo.
If something does not work then see debug to figure it out.
You can now write other demo name in the config file and run tests again.
Set demo to "list" or "help" to see list of possible demo options [TODO].

3. If you just run program with `--devel` it will run the hardcoded demo,
as set in code in variable `g_demoname_default`.
If you start working on a new branch you could set it there to other value
to run your test - see [devel.newthing]

Q: How to develop something new?
A: [devel.newthing] (First read [devel]) and then create git branch named like "wip_galaxy_SOMENAME"
where SOMENAME is a nice name summarizing on what you do work.
Then create also a demo for it (see how other demos are done, see function run_mode_developer,
see demo `developer_tests::wip_galaxy_route_doublestar` that can be used in simulation of few nodes etc)

Q: How to debug a crash, memory leak, etc, how to run in valgrind/gdb
A: [debug.nocap] You can use gdb and valgrind, but if you have a problem there that the CAP privilages level
block your debug tool from working (as is the case with some hardened platforms) then if you can run
a test, or a demo [devel.demo] that does not require net CAP privilages etc,
then use the binary file that is not trying to get CAP rights,
the file nocap-* binary, e.g.: `nocap-tunserver.elf`
e.g.: `valgrind  ./nocap-tunserver.elf --devel`
e.g.: `valgrind  ./nocap-tunserver.elf --devel  --demo foo`




