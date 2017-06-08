
.PHONY: run all

all:
	( cd ../../build && make )

run:
	( cd ../../build && make && ./unittests.elf --gtest_filter=*priv* )

run-all:
	( cd ../../build && make && ./unittests.elf )


