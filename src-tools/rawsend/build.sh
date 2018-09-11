


g++ -O3 --std=c++14 multi.cpp -o multi.bin  -lboost_system

g++ -O3 --std=c++14 rawsend.cpp -o rawsend.bin
sudo setcapnet.sh ./rawsend.bin

