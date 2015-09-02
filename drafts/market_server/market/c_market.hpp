#ifndef C_MARKET_HPP_
#define C_MARKET_HPP_

#include <unordered_map>
#include <map>
#include <iostream>
#include <string>
#include <list>
#include <vector>

using std::string;
using std::cout;
using std::vector;
using std::greater;
using std::less;
using std::map;
using std::unordered_map;
using std::list;
using std::min;
typedef string t_trader;
typedef int t_currency;
typedef int t_amount;
typedef int t_price;

struct offer {
	t_price price;
	t_amount amount;
};

struct return_info {
	t_price total_price;
	t_amount executed;
};

class c_market {
private:
	struct c_offer {
		t_amount all_offers_amount;

		c_offer () : all_offers_amount(t_amount()) { }

		unordered_map<t_trader, t_amount> values;

		void add (const t_trader &, t_amount);

		t_amount execute (const t_trader &, t_amount &);

		void print ();

		unsigned long number_of_traders ();
	};

	typedef map<t_price, c_offer, greater<t_price>> t_offers;

	t_offers buy_offers;
	t_offers sell_offers;
	t_price last_price;

public:
	c_market () : last_price(t_price() - 1) { }

	return_info buy (const t_trader &, t_price, t_amount);

	return_info sell (const t_trader &, t_price, t_amount);

	t_price get_best_buy_offer ();

	t_price get_best_sell_offer ();

	t_price get_last_price ();

	/* for debug & auto_test only */
	void print ();

	list<t_price> get_buy_prices ();

	list<t_price> get_sell_prices ();

	/* --------------------------- */

	vector<offer> get_buy_offers ();

	vector<offer> get_sell_offers ();
};


#endif //C_MARKET_HPP_
