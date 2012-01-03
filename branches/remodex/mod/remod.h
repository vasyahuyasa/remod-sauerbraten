#ifndef __REMOD_H__
#define __REMOD_H__

#include "fpsgame.h"

typedef vector<char *> extensionslist;
extern bool addextension(const char *name);
extern const extensionslist* getextensionslist();

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
    void setteam(int *pcn, const char *team);

    char* concatpstring(char *d, const char *s);

}
#endif
