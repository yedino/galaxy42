
#include "sodium_tests.hpp"

// F = void(buffin*, buffout*, buffin_size, buffout_size) F have to be thread-safe


int sodium_tests_main() {
    if (sodium_init() < 0) {
        std::cerr << "Can not init sodium." << std::endl;
            return 1;
    }

    // poly auth
    std::array<unsigned char, crypto_onetimeauth_KEYBYTES> key;
	
    std::fill_n(key.data(), crypto_onetimeauth_KEYBYTES, 0xfd);
//     crypto_onetimeauth_keygen(key); // was introduced in libsodium 1.0.12.
    auto poly = [&key](unsigned char* buff_in, unsigned char* buff_out, size_t in_size, size_t){
        crypto_onetimeauth(buff_out, buff_in, in_size, key.data());
    };
    {
        crypto_test<decltype(poly)> crypt(poly, "poly_auth_results.txt");
        crypt.test_buffer_size([](unsigned char* buff, unsigned char*, size_t size, size_t){std::fill_n(buff, size, 0xfd);});
    }

    // poly verify
    auto poly_verify = [&key](unsigned char* buff_in, unsigned char* buff_out, size_t in_size, size_t){
        if (crypto_onetimeauth_verify(buff_out, buff_in, in_size, key.data()) != 0) {
           std::cerr << "Poly verification fail" << std::endl;
           std::abort();
        }
    };
    auto init_buffers = [&poly, &key](unsigned char* buff_in, unsigned char* buff_out, size_t in_size, size_t out_size ){
        std::fill_n(buff_in, in_size, 0xfd);
        poly(buff_in, buff_out, in_size, out_size);
    };

    {
        crypto_test<decltype(poly_verify)> crypt(poly_verify, "poly_verify_results.txt");
        crypt.test_buffer_size(init_buffers);
    }

    return 0;
}

#ifdef ANTINET_PART_OF_YEDINO

int main() {
	return sodium_tests_main();
}

#endif


