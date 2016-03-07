
.PHONY: run

run: speed.bin
	./speed.bin

speed.bin: speed.cpp srv.cpp cli.cpp
	g++ --std=c++11 -Wpedantic -Werror -O2 -g3 speed.cpp srv.cpp cli.cpp -o speed.bin

