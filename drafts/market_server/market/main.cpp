#include <allegro.h>
#include <iostream>
#include "c_market.hpp"
#include <thread>

using namespace std;

void manual_test () {
	c_market m;
	string command, trader;
	int amount, price;
	while (true) {
		cin >> command;
		if (command == "buy") {
			cin >> trader >> price >> amount;
			m.buy(trader, price, amount);
		} else if (command == "sell") {
			cin >> trader >> price >> amount;
			m.sell(trader, price, amount);
		} else if (command == "print") {
			m.print();
		}
	}
}

string random_string (const int len = 5) {
	string s;
	constexpr static const char alphanum[] = "0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	static const size_t size = sizeof(alphanum) - 1;

	for (int i = 0; i < len; ++i)
		s += alphanum[rand() % (size)];

	return s;
}

void auto_test () {
	srand(time(nullptr));
	test1:
	{
		c_market m;
		for (int i = 0; i < 5000; ++i) {
			m.buy(random_string(), i + 1, i);
		}
		for (int i = 4999; i >= 0; --i) {
			m.sell(random_string(1), i + 1, i);
		}
		if (m.get_buy_prices().size() != 0 || m.get_sell_prices().size() != 0) {
			cout << "===========WRONG=ANSWER=ON=TEST1=============\n";
		} else
			cout << "=================TEST1==PASSED===============\n";
	}
	test2:
	{
		c_market m;
		for (int i = 0; i < 4999; ++i) {
			m.buy(random_string(), i + 1, i);
		}
		for (int i = 4999; i >= 0; --i) {
			m.sell(random_string(), i + 1, i);
		}
		auto l = m.get_sell_prices();
		if (l.size() != 1 || *l.begin() != 5000 || m.get_buy_prices().size() != 0)
			cout << "===========WRONG=ANSWER=ON=TEST2=============\n";
		else
			cout << "=================TEST2==PASSED===============\n";
	}
	test3:
	{
		c_market m;
		for (int i = 0; i < 5000; ++i) {
			m.sell(random_string(), i + 1, i);
		}
		for (int i = 4999; i >= 0; --i) {
			m.buy(random_string(), i + 1, i);
		}
		if (m.get_buy_prices().size() != 0 || m.get_sell_prices().size() != 0) {
			cout << "===========WRONG=ANSWER=ON=TEST3=============\n";
		} else
			cout << "=================TEST3==PASSED===============\n";
	}
	test4:
	{
		c_market m;
		list<t_price> poss_prices;
		for (int i = 1; i < 5000; i += rand() % 100 + 1) {
			m.buy(random_string(), i, i);
			poss_prices.push_back(i);
		}
		list<t_price> prices = m.get_buy_prices();
		auto it = poss_prices.begin();
		auto it2 = prices.rbegin();
		for (; it != poss_prices.end() && it2 != prices.rend(); ++it, ++it2) {

			if ((*it) != (*it2)) {
				cout << "===========WRONG=ANSWER=ON=TEST4=============\n";
				goto test5;
			}
		}
		if (poss_prices.size() != prices.size())
			cout << "===========WRONG=ANSWER=ON=TEST4=============\n";

		else
			cout << "=================TEST4==PASSED===============\n";
	}
	test5:
	{
		c_market m;
		for (int i = 0; i < 10000; ++i) {
			m.buy(random_string(), i + 1, i);
			m.sell(random_string(), i + 1, i);
		}
		if (m.get_buy_prices().size() != 0 || m.get_sell_prices().size() != 0) {
			cout << "===========WRONG=ANSWER=ON=TEST5=============\n";
		} else
			cout << "=================TEST5==PASSED===============\n";
	}
	test6:
	{
		c_market m;
		int sum = 0;
		for (int i = 1; i < 100; ++i) {
			m.buy(random_string(), i, i);
			sum += i;
		}
		m.sell(random_string(), 100, sum);
		auto l = m.get_sell_prices();
		if (m.get_buy_prices().size() != 99 || l.size() != 1 || *l.begin() != 100) {
			cout << "===========WRONG=ANSWER=ON=TEST6=============\n";
		} else
			cout << "=================TEST6==PASSED===============\n";
	}
	test7:
	{
		c_market m;
		int sum = 0;
		for (int i = 1; i < 100; ++i) {
			m.buy(random_string(), i, i);
			sum += i;
		}
		m.sell(random_string(), 1, sum);
		if (m.get_buy_prices().size() != 0 || m.get_sell_prices().size() != 0) {
			cout << "===========WRONG=ANSWER=ON=TEST7=============\n";
		} else
			cout << "=================TEST7==PASSED===============\n";
	}
	test8:
	{
		c_market m;
		int sum = 0;
		for (int i = 1; i < 100; ++i) {
			m.buy(random_string(), i, i);
			sum += i;
		}
		m.sell(random_string(), 3, sum);
		auto l = m.get_sell_prices();
		if (m.get_buy_prices().size() != 2 || l.size() != 1 || *l.begin() != 3) {
			cout << "===========WRONG=ANSWER=ON=TEST8=============\n";
		} else
			cout << "=================TEST8==PASSED===============\n";
	}
	test9:
	{
		c_market m;
		int sum = 0;
		for (int i = 1; i < 100; ++i) {
			m.buy(random_string(), i, i);
			sum += i;
		}
		m.sell(random_string(), 50, 100);
		m.sell(random_string(), 98, 97);
		auto l = m.get_buy_prices();
		if (l.size() != 97 || m.get_sell_prices().size() != 0) {
			cout << "===========WRONG=ANSWER=ON=TEST9=============\n";
		} else
			cout << "=================TEST9==PASSED===============\n";
	}
	test10:
	{
		c_market m;
		if (m.get_sell_prices().size() != 0 || m.get_sell_prices().size() != 0) {
			cout << "===========WRONG=ANSWER=ON=TEST10============\n";
		} else
			cout << "=================TEST10=PASSED===============\n";
	}
	test11:
	{
		c_market m;
		for (int i = 1; i < 10; ++i) {
			m.buy(random_string(), i, i);
			auto offers = m.get_buy_offers();
			if (offers.size() != i) {
				cout << "===========WRONG=ANSWER=ON=TEST11============\n";
				goto test12;
			}
			for (int j = 1; j < i; ++j) {
				if ((offers[j - 1].amount - 1 != offers[j].amount) || (offers[j - 1].price - 1 != offers[j].price)) {
					cout << "===========WRONG=ANSWER=ON=TEST11============\n";
					goto test12;
				}
			}
			offers = m.get_sell_offers();
			if (offers.size() != 0) {
				cout << "===========WRONG=ANSWER=ON=TEST11============\n";
				goto test12;
			}
		}
		cout << "=================TEST11=PASSED===============\n";
	}
	test12:
	{
		c_market m;
		for (int i = 1; i < 10; ++i) {
			m.sell(random_string(), i, i);
			auto offers = m.get_sell_offers();
			if (offers.size() != i) {
				cout << "===========WRONG=ANSWER=ON=TEST12============\n";
				goto test13;
			}
			for (int j = 1; j < i; ++j) {
				if ((offers[j - 1].amount - 1 != offers[j].amount) || (offers[j - 1].price - 1 != offers[j].price)) {
					cout << "===========WRONG=ANSWER=ON=TEST12============\n";
					goto test13;
				}
			}
			offers = m.get_buy_offers();
			if (offers.size() != 0) {
				cout << "===========WRONG=ANSWER=ON=TEST12============\n";
				goto test13;
			}
		}
		cout << "=================TEST12=PASSED===============\n";
	}
	test13:
	{
		//    c_market m;
		//    const size_t size = 5;
		//    pair<int, int> r[size];
		//    for (int i = 0; i < size; ++i) {
		//      r[i].first = (int)(rand() % size + 1);
		//      r[i].second = (int)(rand() % size + 1);
		//    }
		//
		//    for (int i = 1; i < size; ++i) {
		//      m.buy(random_string(), r[i].first, r[i].second);
		//    }
		//    m.print();
		//    for (int i = 1; i < size; ++i) {
		//      cout << "chce sprzedaz " << r[i].second << " towaru, kazdy za " << r[i].first << '\n';
		//      m.sell(random_string(), r[i].first, r[i].second);
		//      m.print();
		//    }
	}
	test14:
	{
		c_market m;
		const size_t size = 60000;
		pair<int, int> r[size];
		for (int i = 0; i < size; ++i) {
			r[i].first = (int)(rand() % size + 1);
			r[i].second = (int)(rand() % size + 1);
		}

		for (int i = 1; i < size; ++i) {
			m.buy(random_string(), r[i].first, r[i].second);
		}
		for (int i = 1; i < size; ++i) {
			if (m.get_best_sell_offer() == -1)
				break;
			m.sell(random_string(), m.get_best_sell_offer(), size);
		}
		if (m.get_buy_offers().size() != 0)
			cout << "===========WRONG=ANSWER=ON=TEST14============\n";
		else
			cout << "=================TEST14=PASSED===============\n";
	}
	test15:
	{
		c_market m;
		int sum = 0;
		int tmp;
		for (int i = 0; i < 10000; ++i) {
			tmp = rand() % 50 + 2;
			sum += tmp;
			m.buy(random_string(), tmp, tmp);
		}
		m.sell(random_string(), 1, sum + 1);
		auto offers = m.get_sell_offers();
		if (offers.size() != 1 || offers.begin()->price != 1 || offers.begin()->amount != 1 ||
		    m.get_buy_offers().size() != 0) {
			cout << "===========WRONG=ANSWER=ON=TEST15============\n";
		} else {
			cout << "=================TEST15=PASSED===============\n";
		}
	}
}

void stress_test () {
	c_market m;
	const size_t size = 300000000;
	const size_t hop = size / 100;
	const size_t ratio = 10;
	for (size_t i = 0; i < size; ++i) {
		if (rand() & 1) {
			m.buy(random_string(), rand() % ratio, rand() % ratio);
		} else {
			m.sell(random_string(), rand() % ratio, rand() % ratio);
		}
		auto a = m.get_buy_offers();
		auto b = m.get_sell_offers();
		auto c = m.get_sell_prices();
		auto d = m.get_buy_prices();
		if (i % hop == 0) {
			cout << i / hop << '/' << size / hop << '\n';
		}
	}
	cout << "==============STRESS=TEST=PASSED=============\n";
}

int main (int argc, char *argv[]) {

	return 0;
}