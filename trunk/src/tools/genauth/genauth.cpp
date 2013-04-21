#include "cube.h"

char getrndchar()
{
    string alphabet = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOSDFGHJKLZXCVBNM1234567890";
    size_t len = strlen(alphabet);
    size_t n = rand()%len;
    return alphabet[n];
}

// random secret key
char *gensecret()
{
    const size_t maxlen = 100;
    char *secret = newstring(maxlen);
    srand(time(NULL));
    size_t len = rand()%maxlen + 5; // secret len 5...99

    #ifdef WIN32
    loopi(len) secret[i] = getrndchar();
    #else
    FILE *f = fopen("/dev/urandom", "r");
    if(!f) { printf("can not open /dev/urandom\n"); return ""; }
    fread(secret, len, 1, f);
    fclose(f);
    #endif

    secret[len] = 0;
    return secret;
}

void genauthkey(const char *secret)
{
    vector<char> privkey, pubkey;
    genprivkey(secret, privkey, pubkey);
    printf("private key: %s\n", privkey.getbuf());
    printf("public key: %s\n", pubkey.getbuf());
}

int main()
{
    char *s = gensecret();
    genauthkey(s);
    return 0;
}
