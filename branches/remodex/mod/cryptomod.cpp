/*
* remod:    cryptomod.cpp
* date:     2011
* author:   degrave
*
* some hash cubescript bindings
*/


#include "cube.h"
#include "hashlibpp.h"
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

namespace remod {

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
        std::string stdhash;
        string hash;

        // used hashlib2plus examples
        // creating a wrapper object
        hashwrapper *myWrapper = new md5wrapper();

        // create a hash from a string
        stdhash = myWrapper->getHashFromString(str);

        // free memory
        delete myWrapper;

        // convert std::string to string
        strcpy (hash, stdhash.c_str());

        result(hash);
    }

    // Return SHA-1 160 bit hash
    void cs_sha1(const char *str)
    {
        std::string stdhash;
        string hash;

        hashwrapper *myWrapper = new sha1wrapper();
        stdhash = myWrapper->getHashFromString(str);
        delete myWrapper;

        strcpy (hash, stdhash.c_str());
        result(hash);
    }

    // Return SHA-2 256 bit hash
    void cs_sha256(const char *str)
    {
        std::string stdhash;
        string hash;

        hashwrapper *myWrapper = new sha256wrapper();
        stdhash = myWrapper->getHashFromString(str);
        delete myWrapper;

        strcpy (hash, stdhash.c_str());
        result(hash);
    }

    // Return SHA-2 384 bit hash
    void cs_sha384(const char *str)
    {
        std::string stdhash;
        string hash;

        hashwrapper *myWrapper = new sha384wrapper();
        stdhash = myWrapper->getHashFromString(str);
        delete myWrapper;

        strcpy (hash, stdhash.c_str());
        result(hash);
    }

    // Return SHA-2 512 bit hash
    void cs_sha512(const char *str)
    {
        std::string stdhash;
        string hash;

        hashwrapper *myWrapper = new sha512wrapper();
        stdhash = myWrapper->getHashFromString(str);
        delete myWrapper;

        strcpy (hash, stdhash.c_str());
        result(hash);
    }

    // Cube script bindings
    COMMANDN(tiger192, cs_tiger192, "s");
    COMMANDN(md5, cs_md5, "s");
    COMMANDN(sha1, cs_sha1, "s");
    COMMANDN(sha256, cs_sha256, "s");
    COMMANDN(sha384, cs_sha384, "s");
    COMMANDN(sha512, cs_sha512, "s");
}
