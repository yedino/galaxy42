g++ --std=c++14 -O0 --no-inline -Wall a.cpp -o program.bin -lboost_system -lboost_thread -lpthread -g3 -fno-omit-frame-pointer
perf record -g    ./program.bin
perf report -g graph,0.5,callee

