/*
* remod:    cryptomod.cpp
* date:     2011
* author:   degrave
*
* some hash cubescript bindings
*/

#include "cube.h"
#include "polarssl/md5.h"
#include "polarssl/sha1.h"
#include "polarssl/sha2.h"
#include "polarssl/sha4.h"
#include "remod.h"

EXTENSION(CRYPTO);

// shared/crypto.cpp
namespace tiger
{
    typedef unsigned long long int chunk;

    union hashval
    {
        uchar bytes[3*8];
        chunk chunks[3];
    };

    void hash(const uchar *str, int length, hashval &val);
}

namespace remod
{
    void digesttostr(char *dst, unsigned char *digest, size_t digestlen)
    {
        size_t i;
        for(i = 0; i < digestlen; i++)
        {
            #if BYTE_ORDER == LITTLE_ENDIAN
            dst[i*2]    = "0123456789abcdef"[((digest[i]>>4)&0xF)]; // Hi 4 bytes
            dst[i*2+1] = "0123456789abcdef"[((digest[i])&0xF)   ]; // Low 4 bytes
            #else
            dst[di*2]     = "0123456789abcdef"[((digest[i])&0x0F)  ]; // Hight 4 bytes
            dst[di*2+1]   = "0123456789abcdef"[((digest[i]>>4)&0xF)]; // Low 4 bytes
            #endif // BYTE_ORDER
        }
        dst[digestlen*2] = '\0';
    }

    // Return Tiger 192 bit hash
    void cs_tiger192(const char *str)
    {
        // staff from shared/crypto.cpp
        string ref;
        char *hash = &ref[0]; // get pointer to first element
        tiger::hashval hv;
        tiger::hash((uchar *)str, strlen(str), hv);
        loopi(sizeof(hv.bytes)/8)
        {
            #if BYTE_ORDER == LITTLE_ENDIAN
            tiger::chunk c = hv.chunks[i];

            *hash++ = "0123456789abcdef"[(c>>60)&0xF];
            *hash++ = "0123456789abcdef"[(c>>56)&0xF];
            *hash++ = "0123456789abcdef"[(c>>52)&0xF];
            *hash++ = "0123456789abcdef"[(c>>48)&0xF];
            *hash++ = "0123456789abcdef"[(c>>44)&0xF];
            *hash++ = "0123456789abcdef"[(c>>40)&0xF];
            *hash++ = "0123456789abcdef"[(c>>36)&0xF];
            *hash++ = "0123456789abcdef"[(c>>32)&0xF];
            *hash++ = "0123456789abcdef"[(c>>28)&0xF];
            *hash++ = "0123456789abcdef"[(c>>24)&0xF];
            *hash++ = "0123456789abcdef"[(c>>20)&0xF];
            *hash++ = "0123456789abcdef"[(c>>16)&0xF];
            *hash++ = "0123456789abcdef"[(c>>12)&0xF];
            *hash++ = "0123456789abcdef"[(c>>8 )&0xF];
            *hash++ = "0123456789abcdef"[(c>>4 )&0xF];
            *hash++ = "0123456789abcdef"[c&0xF];
            #else
            tiger::chunk c = hv.chunks[i];

            *hash++ = "0123456789abcdef"[c&0xF];
            *hash++ = "0123456789abcdef"[(c>>4 )&0xF];
            *hash++ = "0123456789abcdef"[(c>>8 )&0xF];
            *hash++ = "0123456789abcdef"[(c>>12)&0xF];
            *hash++ = "0123456789abcdef"[(c>>16)&0xF];
            *hash++ = "0123456789abcdef"[(c>>20)&0xF];
            *hash++ = "0123456789abcdef"[(c>>24)&0xF];
            *hash++ = "0123456789abcdef"[(c>>28)&0xF];
            *hash++ = "0123456789abcdef"[(c>>32)&0xF];
            *hash++ = "0123456789abcdef"[(c>>36)&0xF];
            *hash++ = "0123456789abcdef"[(c>>40)&0xF];
            *hash++ = "0123456789abcdef"[(c>>44)&0xF];
            *hash++ = "0123456789abcdef"[(c>>48)&0xF];
            *hash++ = "0123456789abcdef"[(c>>52)&0xF];
            *hash++ = "0123456789abcdef"[(c>>56)&0xF];
            *hash++ = "0123456789abcdef"[(c>>60)&0xF];
            #endif
        }

        *hash = '\0';

        result(ref);
    }

    // cubescript md5 hash
    void cs_md5(const char *str)
    {
        unsigned char digest[16];
        string res;
        md5((const unsigned char*)str, strlen(str), digest);
        digesttostr(res, digest, 16);
        result(res);
    }

    // cubescript sha1 hash
    void cs_sha1(const char *str)
    {
        unsigned char digest[20];
        string res;
        sha1((const unsigned char*)str, strlen(str), digest);
        digesttostr(res, digest, 20);
        result(res);
    }

    // cubescript sha2 256/348/512 bit hash
    void cs_sha256(const char *str)
    {
        unsigned char digest[32];
        string res;
        sha2((const unsigned char*)str, strlen(str), digest, 0);
        digesttostr(res, digest, 32);
        result(res);
    }

    void cs_sha384(const char *str)
    {
        unsigned char digest[64];
        string res;
        sha4((const unsigned char*)str, strlen(str), digest, 1);
        digesttostr(res, digest, 48);
        result(res);
    }

    void cs_sha512(const char *str)
    {
        unsigned char digest[64];
        string res;
        sha4((const unsigned char*)str, strlen(str), digest, 0);
        digesttostr(res, digest, 64);
        result(res);
    }

    /**
    * Calculate the tiger192 hash of a string
    * @group crypto
    * @arg1 string
    * @return returns tiger192 hash of string
    */
    COMMANDN(tiger192, cs_tiger192, "s");

    /**
    * Calculate the md5 hash of a string
    * @group crypto
    * @arg1 string
    * @return returns md5 hash of string
    * @example echo (md5 "test") // 098f6bcd4621d373cade4e832627b4f6
    */
    COMMANDN(md5, cs_md5, "s");

    /**
    * Calculate the sha1 hash of a string
    * @group crypto
    * @arg1 string
    * @return returns sha1 hash of string
    */
    COMMANDN(sha1, cs_sha1, "s");

    /**
    * Calculate the sha252 hash of a string
    * @group crypto
    * @arg1 string
    * @return returns sha252 hash of string
    */
    COMMANDN(sha256, cs_sha256, "s");

    /**
    * Calculate the sha384 hash of a string
    * @group crypto
    * @arg1 string
    * @return returns sha384 hash of string
    */
    COMMANDN(sha384, cs_sha384, "s");

    /**
    * Calculate the sha512 hash of a string
    * @group crypto
    * @arg1 string
    * @return returns sha512 hash of string
    */
    COMMANDN(sha512, cs_sha512, "s");
}
