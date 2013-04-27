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
    if(!f) { printf("Can not open /dev/urandom\n"); exit(1); }
    fread(secret, len, 1, f);
    fclose(f);
    #endif

    secret[len] = 0;
    return secret;
}

void genauthkey(const char *secret, const char* name)
{
    vector<char> privkey, pubkey;
    genprivkey(secret, privkey, pubkey);
    printf("Private key(client):\nauthkey \"%s\" \"%s\" \"domain\"\n", name, privkey.getbuf());
    printf("Public key(server):\nadduser \"%s\" \"domain\" \"%s\" \"m\"\n", name, pubkey.getbuf());
}

// programm help
void help(const char *progname)
{
    printf("Usage: %s NAME\n", progname);
}

// check for user name
char *checkparams(int argc, char* argv[])
{
    char *progname;
    char *name;

    progname = argv[0];

    if(argc<2)
    {
        help(progname);
        exit(1);
    }

    name = argv[1];
    return name;
}

int main(int argc, char* argv[])
{
    char *name = checkparams(argc, argv);
    char *s = gensecret();
    genauthkey(s, name);
    return 0;
}
