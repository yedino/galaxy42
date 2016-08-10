
# hand-made Makefile (not from cmake)

all: hello.elf

clean:
	rm -f hello.elf *.o *.a

.PHONY: run

run: hello.elf
	./hello.elf

main.o: main.cpp
	g++ --std=c++14 -c main.cpp -Wall -o main.o

bar.o: bar.cpp
	g++ --std=c++14 -c bar.cpp -Wall -o bar.o

baz.o: baz.cpp
	g++ --std=c++14 -c baz.cpp -Wall -o baz.o

hello.elf: main.o bar.o baz.o
	g++ main.o bar.o baz.o -l boost_filesystem -l boost_system -lz -lsodium -o hello.elf

