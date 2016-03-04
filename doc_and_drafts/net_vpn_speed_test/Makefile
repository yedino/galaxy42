
.PHONY: run

run: speed.bin
	./speed.bin

speed.bin: speed.cpp
	g++ --std=c++11 -Wpedantic -Werror -O2 -g3 speed.cpp -o speed.bin

