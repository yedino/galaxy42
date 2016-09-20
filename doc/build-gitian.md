
### Usage

Use the command in top-directory:
`./build-gitian`
and follow the instructions.

### Gitian FAQ and common problems

Q: Error `lxc-execute: cgroupfs failed to detect cgroup metadata`
A: Aptitude install (as root of course, on the computer that is trying to run gbuild or lxc-execute) a package "cgroupfs-mount".

### How this works, call tree


./install and so on - to get dependencies for Gitian building

./build-gitian
	bash functions:
	works in ~/var-gitian/ ! (logs are there etc)
	gitian_builder_download() - downloads Gitian (e.g. in our version)
	gitian_builder_make_lxc_image() - ask Gitian to create image of LXC container
		.../gitian-builder/bin/bin/make-base-vm
			...
			deboostrap (using our configured DEBOOSTRAP_DIR fix)
			---> LXC image create
	target_download() - downloads the project version from Git
	gitian_run_build() - run main Gitian build
		.../gitian-builder/gbuild
			inside LXC:
			we have Galaxy (in some version!) in /home/ubuntu/build/galaxy42/
				run script as in .yml ./contrib/gitian-descriptors/main-build/galaxy-windows.yml
					./galaxy42/contrib/gitian-descriptors/main-build/galaxy-windows-script
						download boost
						build boost
						download sodium
						build sodium
						./do of Galaxy
							cmake
								build sodiumpp (our version) -> cmake
							build actually our program

Q: Error ` No such file or directory - failed to get real path for '.../target-foo-amd64`

A: Either you did not create the VM image yet (e.g. with `bin/make-base-vm --lxc --arch amd64 --suite xenial`),
or maybe you did not set the env variable `LXC_SUITE` (e.g. with `export LXC_SUITE=trusty` in `~/.bashrc`).

`
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
`



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



Q:	Fail to get xenial image from ubuntu server:
	I: Retrieving Release 
	E: Failed getting release file http://127.0.0.1:3142/archive.ubuntu.com/ubuntu/dists/xenial/Release 
	Error occured, will exit (to create Xenial image (do you have the Xenial template?))

A:	On some distributions (e.g kali linux) apt-cacher-ng is disabled at system startup and there is a need to start it manually:
	$ sudo /etc/init.d/apt-cacher-ng start


Q: Apt issue inside lxc-gitian machine: 403 Access to cache prohibited  (gitian-builder/var/install.log)
	```./bin/gbuild:21:in `system!': failed to run on-target -u root apt-get update > var/install.log 2>&1 (RuntimeError)```


A: Could be solved with changing apt-cacher configuration:
```
   uncomment "allowed_hosts = *" in apt-cacher.conf (/etc/apt-cacher/apt-cacher.conf)
   restart apt-cacher:
   $ service apt-cacher restart
```
