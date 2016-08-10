

### Hacking

Quick command:

`
debian@dragon:~/gitian-builder$ ( cd ~ ; rm -rf ~/galaxy42/ ; git clone https://github.com/rfree-d/galaxy42.git ; cd ~/galaxy42 ; git reset --hard HEAD ; git clean -xdf ; git checkout wip_gitian_hello ;  git fetch  --all ; git merge --ff-only  origin/wip_gitian_hello ; git log -1 ; echo ; echo ; ) && cd ~/gitian-builder/ &&   ./bin/gbuild    --allow-sudo     ../galaxy42/contrib/gitian-descriptors/test-TEMP/galaxy-linux.yml && on-target sha256sum build/galaxy42/qa/gitian-helloworld-src/hello.elf
`



### Gitian FAQ and common problems

Q: Error ` No such file or directory - failed to get real path for '.../target-foo-amd64`

A: Either you did not create the VM image yet (e.g. with `bin/make-base-vm --lxc --arch amd64 --suite xenial`),
or maybe you did not set the env variable `LXC_SUITE` (e.g. with `export LXC_SUITE=trusty` in `~/.bashrc`).


lxc-execute: No such file or directory - failed to get real path for '/home/debian/gitian-builder/target-foo-amd64'
lxc-execute: failed to mount rootfs
lxc-execute: failed to setup rootfs for 'gitian'
lxc-execute: Error setting up rootfs mount after spawn
lxc-execute: failed to setup the container
lxc-execute: invalid sequence number 1. expected 2
lxc-execute: failed to spawn 'gitian'

lxc-execute: No such file or directory - failed to get real path for '/home/debian/gitian-builder/target--amd64'
lxc-execute: No such file or directory - failed to get real path for '/home/debian/gitian-builder/target-trusty-amd64'
lxc-execute: No such file or directory - failed to get real path for '/home/debian/gitian-builder/target-xenial-amd64'
lxc-execute: No such file or directory - failed to get real path for '/home/debian/gitian-builder/target-jessie-amd64'


Q: Error `The container appears to be already running!` or `failed (98) to create the command service point` when you try to use
the LXC, e.g. from gbuild (in LXC mode).

A: As the error says, other instance is probably running. Maybe you did `on-target` on other console.

`
lxc-execute: failed (98) to create the command service point /var/lib/lxc/gitian/command
lxc-execute: ##
lxc-execute: # The container appears to be already running!
lxc-execute: ##
lxc-execute: failed to initialize the container
./bin/gbuild:21:in lxcsystem!': failed to run make-clean-vm --suite trusty --arch amd64 (RuntimeError)
from ./bin/gbuild:57:in lxcbuild_one_configuration'
from ./bin/gbuild:285:in lxcblock (2 levels) in <main>'
from ./bin/gbuild:280:in lxceach'
from ./bin/gbuild:280:in lxcblock in <main>'
from ./bin/gbuild:278:in lxceach'
from ./bin/gbuild:278:in lxc<main>'
`


