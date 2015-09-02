#include "c_market.hpp"

void c_market::c_offer::print () {
	for (auto &it : values)
		cout << it.first << ' ' << it.second << '\n';
}

void c_market::c_offer::add (const t_trader &trader, t_amount amount) {
	unordered_map<t_trader, t_amount>::iterator found = values.find(trader);
	if (found == values.end())
		values.insert({trader, amount});
	else
		found->second += amount;

	all_offers_amount += amount;
}

t_amount c_market::c_offer::execute (const t_trader &trader, t_amount &amount) {
	t_amount executed, all_executed = 0;
	list<unordered_map<t_trader, t_amount>::iterator> to_delete;
	for (auto it = values.begin(); amount > 0 && it != values.end(); ++it) {
		executed = min(it->second, amount);
		all_executed += executed;
		it->second -= executed; // notify it->first about this!
		amount -= executed;
		all_offers_amount -= executed; // TODO optimization
		if (it->second == 0)
			to_delete.push_back(it); // deleting "zero c_offer"
	}

	for (auto &it : to_delete)
		values.erase(it);

	return all_executed;
}

inline size_t c_market::c_offer::number_of_traders () { return values.size(); }


void c_market::print () {
	if (buy_offers.empty())
		cout << "there is no buy offers right now\n";
	else {
		cout << "BUY OFFERS:\n";
		for (auto it = buy_offers.begin(); it != buy_offers.end(); ++it) {
			cout << "price: " << it->first << '\n';
			it->second.print();
		}
	}

	if (sell_offers.empty())
		cout << "there is no sell offers right now\n";
	else {
		cout << "SELL OFFERS:\n";
		for (auto it = sell_offers.begin(); it != sell_offers.end(); ++it) {
			cout << "price: " << it->first << '\n';
			it->second.print();
		}
	}
	cout << "---------------------------------------------\n";
}

return_info c_market::buy (const t_trader &trader, t_price price, t_amount amount) {
	last_price = price;
	list<t_offers::iterator> to_delete;
	return_info result = {0, amount};
	for (t_offers::iterator it = sell_offers.begin(); it != sell_offers.end() && price >= it->first && amount > 0; ++it) {
		result.total_price += it->first * (it->second.execute(trader, amount));
		if (it->second.number_of_traders() == 0)
			to_delete.push_back(it);

	}
	result.executed -= amount;
	if (amount > 0) {
		t_offers::iterator found = buy_offers.find(price);
		if (found == buy_offers.end()) {
			c_offer tmp;
			tmp.add(trader, amount);
			buy_offers.insert({price, tmp});
		} else
			found->second.add(trader, amount);
	}

	for (auto &i : to_delete)
		sell_offers.erase(i);

	return result;
}

return_info c_market::sell (const t_trader &trader, t_price price, t_amount amount) {
	last_price = price;
	list<t_offers::iterator> to_delete;
	return_info result = {0, amount};
	for (t_offers::iterator it = buy_offers.begin(); it != buy_offers.end() && price <= it->first && amount > 0; ++it) {
		result.total_price += it->first * (it->second.execute(trader, amount));
		if (it->second.number_of_traders() == 0)
			to_delete.push_back(it);
	}

	result.executed -= amount;
	if (amount > 0) {
		t_offers::iterator found = sell_offers.find(price);
		if (found == sell_offers.end()) {
			c_offer tmp;
			tmp.add(trader, amount);
			sell_offers.insert({price, tmp});
		} else
			found->second.add(trader, amount);
	}

	for (auto &i : to_delete)
		buy_offers.erase(i);

	return result;
}

list<t_price> c_market::get_buy_prices () {
	list<t_price> prices;
	for (auto &it : buy_offers)
		prices.push_back(it.first);

	return prices;
}

list<t_price> c_market::get_sell_prices () {
	list<t_price> prices;
	for (auto &it : sell_offers)
		prices.push_back(it.first);

	return prices;
}

vector<offer> c_market::get_buy_offers () {
	vector<offer> offers;
	for (auto &it : buy_offers)
		offers.push_back({it.first, it.second.all_offers_amount});

	return offers;
}

vector<offer> c_market::get_sell_offers () {
	vector<offer> offers;
	for (auto &it : sell_offers)
		offers.push_back({it.first, it.second.all_offers_amount});

	return offers;
}

t_price c_market::get_best_buy_offer () {
	return (sell_offers.begin() != sell_offers.end() ? sell_offers.begin()->first : t_price() - 1);
}

t_price c_market::get_best_sell_offer () {
	return (buy_offers.begin() != buy_offers.end() ? buy_offers.begin()->first : t_price() - 1);
}

inline t_price c_market::get_last_price () { return last_price; }
