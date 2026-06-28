#include <stdio.h>
#include <string.h>
#include <CommonCrypto/CommonDigest.h>

void auth_hash_password(const char *password, char *hash_out) {
    unsigned char hash[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(password, (CC_LONG)strlen(password), hash);
    
    // storing the 32bit in a 64bit string so it's easier to work with
    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_out + i * 2, "%02x", hash[i]);
    }
    hash_out[64] = '\0';
}

int auth_verify_password(const char *password, const char *stored_hash){
    char hashed_pass[65]; auth_hash_password(password, hashed_pass);
    return strncmp(hashed_pass, stored_hash, 64) == 0;
}