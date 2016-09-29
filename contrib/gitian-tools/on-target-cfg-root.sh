
#!/bin/bash
printf "Running gitian configuration (the INSTALL) for USER=$USER on HOSTNAME=$HOSTNAME (this should be run inside Gitian)"
printf "Going to install packages:"

apt-get install aptitude less mc vim nano elinks wget irssi colordiff tree sudo -y

echo "ubuntu ALL=(ALL)NOPASSWD:/bin/su" > /etc/sudoers.d/gitian-sudo-su


