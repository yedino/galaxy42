#ifndef C_API_WOT_HPP
#define C_API_WOT_HPP

#include "libs1.hpp"

#include "api_crypto_base.hpp"

/**

publickey: n=3249054109820938409328585209582309482309482309483204823048230948320483240238409324823423143284234, e=59059495439549543239458353485395534

fingerprint: ACE9221E 960E287E 88C146A2 D59FAB67 F2844784

nym: ACE9221E 960E287E 88C146A2 D59FAB67 F2844784

*/

typedef std::string t_nym_id; ///< a simple ID that allows to identify an ID inside my program

class c_api_wot {
	public:

		void set_distrust(t_nym_id guy); ///< do not trust this guy from now on, e.g. we discovered that he is cheating/trolling

		bool check_is_allowed_for_groupshuffle(t_nym_id guy); ///< return: do we trust this guy enough to work with him in a Group Shuffle

};

/**

Possible near-future changes: 
- t_nym_id could require to be used as smart pointer.

*/

/**

TODO:
implement all member functions

have a working way to add a person's pubkey and get his nym
have a way to get pubkey by nym and viceversa
have a way to learn about private key

*/

#endif // include guard

