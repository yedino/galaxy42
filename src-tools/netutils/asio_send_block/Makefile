all:
	g++ --std=c++14 -O3 -Wall asio_send_block.cpp -o asio_send_block -lboost_system -lboost_thread -lpthread
clean:
	rm asio_send_block
run: asio_send_block
	./asio_send_block  127.0.0.1 9000  500   foo 1500 -1
