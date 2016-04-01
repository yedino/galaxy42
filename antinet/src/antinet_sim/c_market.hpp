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

/// Result of our market operation, e.g. how much we sold, how much we earned
struct return_info {
	/// How many units-B (e.g. USD) did we now got/send in totall.
	/// E.g. that we right now in totall earned 100 usd, or that we right now spent 100 usd.
	t_price total_price;

	/// How many units-A (e.g. bitcoins) we executed here.
	/// The rest was probably added to the market.
	t_amount executed;
};


/// @brief The class executes a simple local exchange-market,
/// @TODO Decide how to signal an "no operation" error, e.g. no last price yet.
class c_market {
private:
	struct c_offer {
		c_offer ();

		t_amount all_offers_amount;

		unordered_map<t_trader, t_amount> values;

		void add (const t_trader &, t_amount);

		t_amount execute (const t_trader &, t_amount &);

		void print ();

		size_t number_of_traders ();
	};

	typedef map<t_price, c_offer, greater<t_price>> t_offers; ///< the state of offers

	t_offers buy_offers;
	t_offers sell_offers;
	t_price last_price;

public:
	c_market ();// : last_price(t_price() - 1) { }

	return_info buy (const t_trader &, t_price, t_amount);

	return_info sell (const t_trader &, t_price, t_amount);

	t_price get_best_buy_offer ();

	t_price get_best_sell_offer ();

	t_price get_last_price (); ///< what was the last price

	/* for debug & auto_test only */
	void print ();

	list<t_price> get_buy_prices ();

	list<t_price> get_sell_prices ();

	/* --------------------------- */

	vector<offer> get_buy_offers ();

	vector<offer> get_sell_offers ();
};


#endif //C_MARKET_HPP_
