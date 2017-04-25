
.PHONY: run all

run:
	( cd ../../build && make && ./test-debug.elf --gtest_filter=stdplus_tab.* )

all:
	( cd ../../build && make && ./test-debug.elf )

