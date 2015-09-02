#include <memory>

using std::unique_ptr;

/**

@file: we propose following style

*/


// general api

class data { 
	public: 
};

class ctext : public data { 
	public: 
};

class encryption { 
	public:
		virtual unique_ptr<ctext> encrypt(const data &d)=0;
};


// rsa api

class ctext_rsa : public ctext { 
	public:
};

class encryption_rsa : public encryption { 
	public:
		virtual unique_ptr<ctext> encrypt(const data &d);
};

unique_ptr<ctext> encryption_rsa::encrypt(const data &d){ 
	unique_ptr<ctext> r( new ctext_rsa );
	return r;
}


int test_main() {
	ctext_rsa ct;

	encryption_rsa myenc_rsa;
	encryption & myenc = myenc_rsa;

}

