
.PHONY: run all

run:
	( cd ../../build && make && ./unittests.elf --gtest_filter=*cable*.* )

all:
	( cd ../../build && make && ./unittests.elf )

