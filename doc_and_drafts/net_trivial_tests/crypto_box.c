#include <stdio.h>
#include <sodium.h>

#define MESSAGE (const unsigned char *) "crypto secret message"
#define MESSAGE_LEN 22
#define ADDITIONAL_DATA (const unsigned char *) "123456"
#define ADDITIONAL_DATA_LEN 6



int main(int argc, char *argv[]) {

    if(sodium_init() == -1) {
        return 1;
    }
    printf("libsoduim - OK\n");

    unsigned char nonce[crypto_aead_chacha20poly1305_NPUBBYTES];
    unsigned char key[crypto_aead_chacha20poly1305_KEYBYTES];
    unsigned char ciphertext[MESSAGE_LEN + crypto_aead_chacha20poly1305_ABYTES];
    unsigned long long ciphertext_len;

    randombytes_buf(key, sizeof key);
    randombytes_buf(nonce, sizeof nonce);

    printf("message: ");
    for (int i = 0; i < MESSAGE_LEN; ++i) {
        printf("%c",(char)MESSAGE[i]);
    } putchar('\n');

    crypto_aead_chacha20poly1305_encrypt(ciphertext, &ciphertext_len,
                                         MESSAGE, MESSAGE_LEN,
                                         ADDITIONAL_DATA, ADDITIONAL_DATA_LEN,
                                         NULL, nonce, key);
    printf("cipher: ");
    for (int i = 0; i < ciphertext_len; ++i) {
        printf("%c",(char)ciphertext[i]);
    } putchar('\n');



    unsigned char decrypted[MESSAGE_LEN];
    unsigned long long decrypted_len;
    if (crypto_aead_chacha20poly1305_decrypt(decrypted, &decrypted_len,
                                             NULL,
                                             ciphertext, ciphertext_len,
                                             ADDITIONAL_DATA,
                                             ADDITIONAL_DATA_LEN,
                                             nonce, key) != 0) {
        printf("decrypt fail: message forged!\n");
    }

    printf("message decrypted: ");
    for (int i = 0; i < decrypted_len; ++i) {
        printf("%c",(char)decrypted[i]);
    } putchar('\n');

    return 0;
}
