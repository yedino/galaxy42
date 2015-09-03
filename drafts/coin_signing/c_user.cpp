#include "c_user.hpp"

void c_user::send_token (const c_user &user, size_t amount) {

}

void c_user::recieve_token (size_t amount) {

}

void c_user::emit_tokens (size_t t) {
	c_token emitted_token = mint.emit_token();
	wallet.add_token(emitted_token);
}
