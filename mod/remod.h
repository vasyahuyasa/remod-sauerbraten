#ifndef __REMOD_H__
#define __REMOD_H__

#include "fpsgame.h"

namespace remod
{
    using namespace server;

    clientinfo* findbest(vector<clientinfo *> &a);
    bool playerexists(int *pcn);
    int parseplayer(const char *arg);
    bool ismaster(int *cn);
    bool isadmin(int *cn);
    bool isspectator(int *cn);
}
#endif
