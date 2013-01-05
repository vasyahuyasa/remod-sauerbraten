#ifndef __REMOD_H__
#define __REMOD_H__

#include "fpsgame.h"

typedef vector<char *> extensionslist;
extern bool addextension(const char *name);
extern const extensionslist* getextensionslist();
extern void explodelist(const char *s, vector<char *> &elems);
extern char *conc(char **w, int n, bool space);

#define EXTENSION(name) bool __dummyext_##name = addextension(#name)

namespace remod
{
    using namespace server;

    clientinfo* findbest(vector<clientinfo *> &a);
    bool playerexists(int *pcn);
    int parseplayer(const char *arg);
    bool ismaster(int *cn);
    bool isadmin(int *cn);
    bool isspectator(int *cn);

    char* concatpstring(char *d, const char *s);
    void loadbans();
    void writebans();

}
#endif
