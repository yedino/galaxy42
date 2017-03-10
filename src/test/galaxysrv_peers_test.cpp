#include <galaxysrv_peers.hpp>
#include "gtest/gtest.h"
#include "libs1.hpp"

using t_parsed_peer=c_galaxysrv_peers::t_peering_reference_parse;
using namespace std;

TEST(galaxysrv_peers, parse_peer_reference_test) {
	c_galaxysrv_peers test_srv_peers;
	map<string, t_parsed_peer> peers;

	string str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:76.31.171.15:9042)"s;
	vector<string> id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"s};
	vector<string> cable = {"udp:76.31.171.15:9042"s};
	t_parsed_peer peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr)";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"};
	cable = {"bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:p.meshnet.pl:9042,cost=500)@(shm:test)@(tcp:[fe80::d44e]:9042)";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"};
	cable = {"udp:p.meshnet.pl:9042,cost=500", "shm:test", "tcp:[fe80::d44e]:9042"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr)@(udp:76.31.171.15:9042)";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"};
	cable = {"bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr", "udp:76.31.171.15:9042"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend@(udp:76.31.171.15:9042)";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend"};
	cable = {"udp:76.31.171.15:9042"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=10000,prio=10@(udp:76.31.171.15:9042)";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=10000,prio=10"};
	cable = {"udp:76.31.171.15:9042"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=-300";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=-300"};
	cable = {};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42::1234@(udp:foo.com:9042)@(email:tx@test.com)@(email:b@test.com)@(email:c@test.com)";
	id = {"fd42::1234"};
	cable = {"udp:foo.com:9042", "email:tx@test.com", "email:b@test.com", "email:c@test.com"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "anyone@(udp:p.meshnet.pl:9042)";
	id = {"anyone"};
	cable = {"udp:p.meshnet.pl:9042"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "anyone@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr,cost=99999)";
	id = {"anyone"};
	cable = {"bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr,cost=99999"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "VIRTUAL";
	id = {"VIRTUAL"};
	cable = {};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "VIRTUAL@(CABLE)";
	id = {"VIRTUAL"};
	cable = {"CABLE"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "VIRTUAL@(CABLE)@(CABLE)";
	id = {"VIRTUAL"};
	cable = {"CABLE", "CABLE"};
	peer = t_parsed_peer(id, cable);
	peers.insert(pair<string, t_parsed_peer>(str, peer));

	for(pair<string, t_parsed_peer> element : peers)
		EXPECT_EQ(test_srv_peers.parse_peer_reference(element.first), element.second);

	for(pair<string, t_parsed_peer> element1 : peers)
		for(pair<string, t_parsed_peer> element2 : peers)
			if(element1 != element2)
				EXPECT_NE(test_srv_peers.parse_peer_reference(element1.first), element2.second);


	map<string, t_parsed_peer> incorect_peers;

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f@(udp:76.31.171.15:9042)"s;
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"s};
	cable = {"udp:76.31.171.15:9042"s};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(itmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr)";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"};
	cable = {"bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr"};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:p.meshnet.pl:9042,cost=50)@(shm:test)@(tcp:[fe80::d44e]:9042)";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"};
	cable = {"udp:p.meshnet.pl:9042,cost=500", "shm:test", "tcp:[fe80::d44e]:9042"};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr)@(udp:76.31.171.15:9042)";
	id = {"d42:f6c4:9d19:f128:30df:b289:aef0:25f5"};
	cable = {"bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr", "udp:76.31.171.15:9042"};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend@(udp:76.31.171.15:9042)";
	id = {"fd42:f6c4:9d19:f128:3df:b289:aef0:25f5,friend"};
	cable = {"udp:76.31.171.15:9042"};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=10000,prio=10@(udp:76.31.171.15:9042)";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=10000,prio=1"};
	cable = {"udp:76.31.171.15:9042"};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=-300";
	id = {"fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=-300"};
	cable = {""};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "fd42::1234@(udp:foo.com:9042)@(email:tx@test.com)@(email:b@test.com)@(email:c@test.com)";
	id = {"fd42::1234"};
	cable = {"udp:foo.com:9042", "email:tx@test.com", "email:b@testcom", "email:c@test.com"};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "anyone@(udp:p.meshnet.pl:9042)";
	id = {"anyone", ""};
	cable = {"udp:p.meshnet.pl:9042"};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "anyone@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr,cost=99999)";
	id = {"anyone"};
	cable = {"bitmsgBM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr,cost=99999"};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	str = "VIRTUA";
	id = {"VIRTUAL"};
	cable = {};
	peer = t_parsed_peer(id, cable);
	incorect_peers.insert(pair<string, t_parsed_peer>(str, peer));

	for(pair<string, t_parsed_peer> element : incorect_peers)
		EXPECT_NE(test_srv_peers.parse_peer_reference(element.first), element.second);

	for(pair<string, t_parsed_peer> element1 : incorect_peers)
		for(pair<string, t_parsed_peer> element2 : incorect_peers)
			if(element1 != element2)
				EXPECT_NE(test_srv_peers.parse_peer_reference(element1.first), element2.second);

}

TEST(galaxysrv_peers, parse_peer_reference_throw_exceptions_test) {
	c_galaxysrv_peers test_srv_peers;
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL@)CABLE)"), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL$(CABLE)"), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL(CABLE)"), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL@(CABLE@(CABLE)"), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL@(CABLE)(CABLE)"), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL@(CABLE"), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL@(CABLE("), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL@CABLE)"), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL@(CABLE(CABLE)"), err_check_user);
	EXPECT_THROW(test_srv_peers.parse_peer_reference("VIRTUAL@(CABLE)CABLE)"), err_check_user);
}
