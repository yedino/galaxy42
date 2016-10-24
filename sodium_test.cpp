#include <iostream>
#include <chrono>

#include <sodium.h>
#include <sodiumpp/sodiumpp.h>

#include <boost/locale.hpp>


void sodium_sha512() {

     unsigned char out[64];
     unsigned char in[10] = "aswerwerd";
     unsigned long long inlen = 6;

     int w = crypto_hash_sha512(out, in,inlen);
 
     for(size_t i = 0; i < 64; ++i) {
         std::cout << std::hex << (unsigned)out[i];
 
     }   
}




/*
  void test_sodiumpp () { 
     const size_t seconds_for_test_case=5;
     auto start_point = std::chrono::steady_clock::now();
     const std::string message(10240, 'm');
     const std::string nonce(crypto_box_NONCEBYTES, 'n');
     const std::string shared_key(crypto_stream_KEYBYTES, 'k');
 
     size_t encrypted_data_size = 0; 
     size_t number_of_loop = 0; 
     while (std::chrono::steady_clock::now() - start_point < std::chrono::seconds(seconds_for_test_case)) {
         auto encrypted = sodiumpp::crypto_stream_xor(message, nonce, shared_key);
         ++number_of_loop;
         encrypted_data_size += message.size();
     }    
     auto stop_point = std::chrono::steady_clock::now();
     unsigned int loop_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_point - start_point).count();
     std::cout << "xsalsa20 performance" << std::endl;
     std::cout << number_of_loop << " encryptions in " << loop_time_ms << " ms" << std::endl;
     std::cout << "Encrypted data size = " << encrypted_data_size << " bytes" << std::endl;
     std::cout << static_cast<double>(encrypted_data_size) / seconds_for_test_case / 1024 / 1024 << " MB per second" << std::endl;
 
     std::cout << "**************************************************" << std::endl;
     std::cout << "Encrypt using boxer" << std::endl;
     const auto locked_shared_key = sodiumpp::locked_string(shared_key);
     const sodiumpp::encoded_bytes nonce_zero =
         sodiumpp::encoded_bytes( string( t_crypto_nonce::constantbytes , char(0)), sodiumpp::encoding::binary);
     sodiumpp::boxer< t_crypto_nonce > boxer(
                     sodiumpp::boxer_base::boxer_type_shared_key(),
                     false,
                     locked_shared_key,
                     nonce_zero);
     start_point = std::chrono::steady_clock::now();
     while (std::chrono::steady_clock::now() - start_point < std::chrono::seconds(seconds_for_test_case)) {
         boxer.box(message);
         ++number_of_loop;
         encrypted_data_size += message.size();
     }    
     stop_point = std::chrono::steady_clock::now();
     loop_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_point - start_point).count();
     std::cout << number_of_loop << " encryptions in " << loop_time_ms << " ms" << std::endl;
     std::cout << "Encrypted data size = " << encrypted_data_size << " bytes" << std::endl;
     std::cout << static_cast<double>(encrypted_data_size) / seconds_for_test_case / 1024 / 1024 << " MB per second" << std::endl;



}
*/
int main () {

    if (sodium_init() == -1) {
        return 1;
    }

    sodium_sha512();
//    test_sodiumpp();

    boost::locale::generator gen;

    return 0;

}
