#ifndef _SHA256_H
#define _SHA256_H
#include <string>

typedef unsigned char uint8;
typedef unsigned long int uint32;

struct sha256_context {
	uint32 total[2];
	uint32 state[8];
	uint8 buffer[64];
};

void sha256_starts (sha256_context *ctx);

void sha256_update (sha256_context *ctx, uint8 *input, uint32 length);

void sha256_finish (sha256_context *ctx, uint8 digest[32]);

template <typename T>
T sha256 (const std::string &input) {
	unsigned char digest[32];
	memset(digest, 0, 32);
	sha256_context ctx;
	sha256_starts(&ctx);
	sha256_update(&ctx, (unsigned char *)input.c_str(), input.size());
	sha256_finish(&ctx, digest);

	T dec = 0;
	for (int i = 0; i < 32; ++i) {
		dec <<= 8;
		dec += (short)digest[i];
	}
	return dec;
}

template <>
std::string sha256 (const std::string &input);
#endif // _SHA256_H