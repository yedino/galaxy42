// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include <boost/filesystem.hpp>
#include "../c_ip46_addr.hpp"


TEST(c_ip46_addr, compare_same) {
	c_ip46_addr addr1a( "192.168.1.2" , 9042 );
	c_ip46_addr addr1b( "192.168.1.2" , 9042 );
	_mark(addr1a);
	
	EXPECT_EQ(addr1a, addr1b);
}

TEST(c_ip46_addr, compare_sort) {
	vector<c_ip46_addr> tab; // all elements, are here in the expected sorted order
	vector<int> someports({0,1,2,1049,1050,1051,1099,9999,65535}); // various IP (tcp/udp) ports

	for (auto port:someports) tab.push_back( c_ip46_addr( "0.0.0.0" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "1.0.0.0" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "191.200.200.200" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.1.1.1" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.1.1" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.49.1" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.49.99" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.50.49" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.50.50" , port ) );  // "middle"
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.50.51" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.50.255" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.51.1" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.51.49" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.51.51" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.51.200" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.200.0" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "192.168.200.1" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "200.168.200.1" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "200.168.200.50" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "200.168.200.200" , port ) );
	for (auto port:someports) tab.push_back( c_ip46_addr( "255.255.255.255" , port ) );

	_info("Testing IPs count: " << tab.size());

	for (int i=0; i<tab.size(); ++i) {
		EXPECT_TRUE(tab.at(i) == tab.at(i));
		EXPECT_FALSE(tab.at(i) < tab.at(i));
		for (int j=0; j<tab.size(); ++j) {
			if (i<j)  EXPECT_TRUE(   tab.at(i) <  tab.at(j) );
			if (i>j)  EXPECT_TRUE(!( tab.at(i) <  tab.at(j) ));

			if (i==j) EXPECT_TRUE(   tab.at(i) == tab.at(j) );
			if (i!=j) EXPECT_TRUE(!( tab.at(i) == tab.at(j) ));
		}
	}

}

