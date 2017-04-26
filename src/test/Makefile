
.PHONY: run all

run:
	( cd ../../build && make && ./test-debug.elf --gtest_filter=*cable*.* )

all:
	( cd ../../build && make && ./test-debug.elf )

