 #!/usr/bin/env bash
 
 # Install dependencies to build antinet simulation
 # You need root to execute this script.

 SYSTEM_NAME=$(uname)
 
 if [ "$SYSTEM_NAME" == "Linux" ] 
 then
     # handle linux distributions
     SYSTEM_DISTRO=$(lsb_release -i -s)
     if [ "$SYSTEM_DISTRO" == "Debian" ]
     then
         sudo apt-get install liballegro4-dev libpng-dev libboost-all-dev liballeggl4-dev libcrypto++-dev
     elif [ "$SYSTEM_DISTRO" == "Ubuntu" ]
     then
         sudo apt-get install liballegro4-dev libpng-dev libboost-all-dev liballeggl4-dev libcrypto++-dev
     else
         echo "Unsupported Linux distribution."
     fi  
 else
     echo "Unspupported system."
 fi
