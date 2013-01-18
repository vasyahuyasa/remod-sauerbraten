/*
* remod:    cryptomod.cpp
* date:     2011
* author:   degrave
*
* some hash cubescript bindings
*/

#include "md5/md5.h"
#include "cube.h"
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

    // Return md5 hash
    void cs_md5(const char *str)
    {
        md5_state_t state;
        md5_byte_t digest[16];
        char hex_output[16*2 + 1];
        int di;

        md5_init(&state);
        md5_append(&state, (const md5_byte_t *)str, strlen(str));
        md5_finish(&state, digest);
        for(di = 0; di < 16; di++)
        {
            #if BYTE_ORDER == LITTLE_ENDIAN
            hex_output[di*2]     = "0123456789abcdef"[((digest[di]>>4)&0xF)]; // Hight 4 bytes
            hex_output[di*2+1]   = "0123456789abcdef"[((digest[di])&0xF)   ]; // Low 4 bytes
            #else
            hex_output[di*2]     = "0123456789abcdef"[((digest[di])&0x0F)  ]; // Hight 4 bytes
            hex_output[di*2+1]   = "0123456789abcdef"[((digest[di]>>4)&0xF)]; // Low 4 bytes
            #endif
        }
        hex_output[16*2] = '\0';
        result(hex_output);
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
}
