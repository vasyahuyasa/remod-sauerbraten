/*
* remod:    remod.cpp
* date:     2007
*
* some routines
*/



#include "remod.h"

namespace remod
{
using namespace server;

// Find best frager
clientinfo* findbest(vector<clientinfo *> &a)
{
    int bestfrags = a[0]->state.frags;
    int bfrager = 0;
    loopv(a)
    {
        if(a[i]->state.frags > bestfrags)
        {
            bestfrags = a[i]->state.frags;
            bfrager = i;
        }
    }

    clientinfo *ci = a.remove(bfrager);
    return ci;
}

bool playerexists(int *pcn)
{
    int cn = (int)*pcn;
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->clientnum==cn)
        {
            return true;
        }
    }
    return false;
}

// convert player name to cn if exists or -1
// imported from client
int parseplayer(const char *arg)
{
    char *end;
    int n = strtol(arg, &end, 10);
    if(*arg && !*end)
    {
        if(!playerexists(&n)) return -1;
        return n;
    }
    // try case sensitive first
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(!strcmp(arg, ci->name)) return ci->clientnum;
    }
    // nothing found, try case insensitive
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(!strcasecmp(arg, ci->name)) return ci->clientnum;
    }
    return -1;
}

bool ismaster(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    return (ci && ci->privilege >= PRIV_MASTER);
}

bool isadmin(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    return (ci && ci->privilege >= PRIV_ADMIN);
}

bool isspectator(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    return (ci && ci->state.state==CS_SPECTATOR);
}

char* concatpstring(char *d, const char *s) {
	char *tmp = newstring(d);
	DELETEA(d);
	size_t lt = strlen(tmp);
	size_t ls = strlen(s);
	d = new char[lt +ls + 1];
	strncpy(d, tmp, lt);
	strncpy(&d[lt], s, ls);
	d[lt +ls] = '\0';
	DELETEA(tmp);
	return d;
}

}