#include "openssl/sha.h"

#define SHA_1
#include "openssl/sha_locl.h"

void SHA1_Digest(const void *data, size_t len, unsigned char *digest) {
  SHA_CTX sha;

  SHA1_Init(&sha);
  SHA1_Update(&sha, data, len);
  SHA1_Final(digest, &sha);
}

