#build:
    cmake .
    make

#dependences 

    
    bitcoin-api-cpp --  https://github.com/minium/bitcoin-api-cpp

    libjson-rpc-cpp --  https://github.com/cinemast/libjson-rpc-cpp
                        or simpliest on Debian:
                        sudo apt-get install libjsonrpccpp-dev libjsonrpccpp-tools

    cmake           --  https://cmake.org/download/
    curl            --  https://curl.haxx.se/libcurl/
    jsoncpp         --  https://github.com/open-source-parsers/jsoncpp
                        from Deabiam repository 
                        sudo apt-get install cmake libcurl4-openssl-dev libjsoncpp-dev
