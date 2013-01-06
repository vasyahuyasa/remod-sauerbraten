/*
* remod:    remod.cpp
* date:     2007
*
* some routines
*/



#include "remod.h"

EXTENSION(REMOD);

extensionslist *extensions = NULL;

bool addextension(const char *name)
{
    if(!extensions) extensions = new extensionslist;
    char *s = newstring(name);
    extensions->add(s);
    return false;
}

const extensionslist* getextensionslist()
{
    return extensions;
}

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

// Write permbans to disk
SVAR(banfile, "permbans.cfg");

void loadbans()
{
    execfile(banfile);
}

void writebans()
{
    const char *fname;
    fname = findfile(banfile, "w");

    FILE *f = fopen(fname, "w");

    if(f)
    {
        union { uchar b[sizeof(enet_uint32)]; enet_uint32 i; } ip, mask;
        string maskedip;

        fprintf(f, "// This file was generated automaticaly\n// Do not edit it while server running\n\n");

        loopv(permbans)
        {
            permban b = permbans[i];

            ip.i = b.ip;
            mask.i = b.mask;

            maskedip[0] = '\0';

            // generate masked ip (ex. 234.345.45)
            loopi(4)
            {
                if(mask.b[i] != 0x00)
                {
                    if(i) strcat(maskedip, ".");
                    strcat(maskedip, intstr(ip.b[i]));
                }
                else  break;
            }

            fprintf(f,"permban %s \"%s\"\n", maskedip, b.reason);
        }

        fclose(f);
    }
    else
    {
        conoutf("Can not open \"%s\" for writing bans");
    }
}

}
