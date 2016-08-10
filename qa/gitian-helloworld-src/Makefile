
# hand-made Makefile (not from cmake)

all: hello.elf

clean:
	rm -f hello.elf *.o *.a

.PHONY: run

run: hello.elf
	./hello.elf

hello.elf:
	g++ --std=c++14 main.cpp -Wall -o hello.elf

