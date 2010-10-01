#include "fpsgame.h"
#include "commandev.h"

#ifndef WIN32
#include <arpa/inet.h>
#endif

//Remod
namespace remod
{
using namespace server;

//Additional cubescript functions
void getname(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        result(ci->name);
    }
}

void getmap()
{
    result(smapname);
}

void getmode()
{
    intret(gamemode);
}

void getip(int *cn)
{
    if((int)*cn<(getnumclients()) && (int)*cn>=0)
    {
        in_addr addr;
        addr.s_addr = getclientip((int)*cn);
        char *ip = inet_ntoa(addr);
        result(ip);
    }
}

void getfrags(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        intret(ci->state.frags);
    }
}

void getdeaths(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        intret(ci->state.deaths);
    }
}

void getteamkills(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        intret(ci->state.teamkills);
    }
}

void getaccuracy(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        intret(ci->state.damage*100/max(ci->state.shotdamage,1));
    }
}

void getflags(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)cn);
    if(ci)
    {
        intret(ci->state.flags);
    }
}

void getmastermode()
{
    intret(mastermode);
}

void getmastermodename(int *mm)
{
    result(mastermodename((int)*mm, "unknown"));
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

void version()
{
    string txt;
    formatstring(txt)("Remod $Rev$ (build %s %s)", __DATE__, __TIME__);
    result(txt);
}

void getteam(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        result(ci->team);
    }
}

void disconnect(int *cn)
{
    disconnect_client((int)*cn, DISC_NONE);
}

void kick(int *cn)
{
    ban &b = bannedips.add();
    b.time = totalmillis;
    b.ip = getclientip((int)*cn);
    allowedips.removeobj(b.ip);
    disconnect_client((int)*cn, DISC_KICK);
    remod::onevent("onkick", "ii", -1, (int)*cn);
}

void spectator(int *st, int *cn)
{
    int spectator = (int)*cn;
    int val = (int)*st;
    clientinfo *spinfo = (clientinfo *)getclientinfo(spectator); // no bots
    if(!spinfo || (spinfo->state.state==CS_SPECTATOR ? val : !val)) return;
    if(spinfo->state.state!=CS_SPECTATOR && val)
    {
        if(spinfo->state.state==CS_ALIVE) suicide(spinfo);
        if(smode) smode->leavegame(spinfo);
        spinfo->state.state = CS_SPECTATOR;
        spinfo->state.timeplayed += lastmillis - spinfo->state.lasttimeplayed;
        if(!spinfo->local && !spinfo->privilege) aiman::removeai(spinfo);
    }
    else if(spinfo->state.state==CS_SPECTATOR && !val)
    {
        spinfo->state.state = CS_DEAD;
        spinfo->state.respawn();
        spinfo->state.lasttimeplayed = lastmillis;
        aiman::addclient(spinfo);
        if(spinfo->clientmap[0] || spinfo->mapcrc) checkmaps(-1);
        sendf(-1, 1, "ri", N_MAPRELOAD);
    }
    sendf(-1, 1, "ri3", N_SPECTATOR, spectator, val);
    if(!val && mapreload && !spinfo->privilege && !spinfo->local) sendf(spectator, 1, "ri", N_MAPRELOAD);
}

void map(char *name)
{
    sendf(-1, 1, "risii", N_MAPCHANGE, name, gamemode, 1);
    changemap(name, gamemode);
}

void mapmode(char *name, int *mode)
{
    sendf(-1, 1, "risii", N_MAPCHANGE, name, mode, 1);
    changemap(name, (int)*mode);
}

void _suicide(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        suicide(ci);
    }
}

void addbot(int *s)
{
    if(!aiman::addai((int)*s, -1))
    {
        //Couldn't add bot
    }
}

void delbot()
{
    if(!aiman::deleteai())
    {
        //Can't delete any bot
    }
}

void say(char *msg)
{
    sendservmsg(msg);
}

void pm(int *cn, char *msg)
{
    if(clients.inrange((int)*cn))
    {
        sendf((int)*cn, 1, "ris", N_SERVMSG,msg);
    }
}

void saytonormal(char *msg)
{
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->connected && ci->privilege == PRIV_NONE)
        {
            pm(&ci->clientnum, msg);
        }
    }
}

void saytomaster(char *msg)
{
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->connected && ci->privilege == PRIV_MASTER)
        {
            pm(&ci->clientnum, msg);
        }
    }
}

void saytoadmin(char *msg)
{
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->connected && ci->privilege == PRIV_ADMIN)
        {
            pm(&ci->clientnum, msg);
        }
    }
}

void _mastermode(int *mm)
{
    mastermode = (int)*mm;
    remod::onevent("onmastermode", "ii", -1, mastermode);
    if((int)*mm>=MM_PRIVATE)
    {
        loopv(clients) allowedips.add(getclientip(clients[i]->clientnum));
    }
    sendf(-1, 1, "rii", N_MASTERMODE, mastermode);
}

void clearbans()
{
    remod::onevent("onclearbans", "");
    bannedips.shrink(0);
    sendservmsg("cleared all bans");
}

void setteam(int *cn, const char *team)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(!ci && !strcmp(ci->team, team)) return;
    remod::onevent("onsetteam", "is", cn, team);
    if(!smode || smode->canchangeteam(ci, ci->team, team))
    {
        if(ci->state.state==CS_ALIVE) suicide(ci);
        copystring(ci->team, team, MAXTEAMLEN+1);
    }
    aiman::changeteam(ci);
    sendf(-1, 1, "riisi", N_SETTEAM, (int)*cn, ci->team, 1);
}


void getping(int*cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)cn);
    if(ci)
    {
        intret(ci->ping);
    }
}

void getonline(int*cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        intret(ci->connectmillis);
    }
}

void _getteamscore(char *team)
{
    if(smode && smode->hidefrags())
    {
        intret(smode->getteamscore(team));
    }
    else
    {
        int teamfrags = 0;
        loopv(clients) if(clients[i]->team[0])
        {
            clientinfo *ci = clients[i];
            if(ci && (strcmp(team, ci->team) == 0)) teamfrags+=ci->state.frags;
        }
        intret(teamfrags);
    }

}

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

void getrank(int *cn)
{
    vector<clientinfo *> uplayers, splayers; //unsorted sorted

    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->state.state != CS_SPECTATOR) uplayers.add(ci);
        conoutf("%i %s[%i] %i frags", i, ci->name, ci->clientnum, ci->state.frags);
    }

    while(uplayers.length()>0)
    {
        clientinfo *ci = findbest(uplayers);
        splayers.add(ci);
    }

    loopv(splayers)
    {
        if(splayers[i]->clientnum == (int)*cn)
        {
            intret(i+1);
            return;
        }
    }
    intret(-1);
}

//Cube script binds
COMMAND(getname, "i");
COMMAND(getmap, "");
COMMAND(getmode, "");
COMMAND(getip, "i");
COMMAND(getfrags, "i");
COMMAND(getdeaths, "i");
COMMAND(getteamkills, "i");
COMMAND(getaccuracy, "i");
COMMAND(getflags, "i");
//COMMAND(getretflags, "i"); unimplemented
COMMAND(getmastermode, "");
COMMAND(getmastermodename, "i");
ICOMMAND(ismaster, "i", (int *cn), intret(ismaster(cn) ? 1 : 0));
ICOMMAND(isadmin, "i", (int *cn), intret(isadmin(cn) ? 1 : 0));
ICOMMAND(isspectator, "i", (int *cn), intret(isspectator(cn) ? 1 : 0));
COMMAND(version, "");

COMMAND(getteam,"i");
COMMAND(disconnect, "i");
COMMAND(kick, "i");
COMMAND(spectator, "ii");
COMMAND(map, "s");
COMMAND(mapmode, "si");
COMMANDN(suicide, _suicide, "i");
COMMAND(addbot, "i");
COMMAND(delbot, "");

COMMAND(say, "C");
COMMAND(pm, "is");
COMMAND(saytonormal, "C");
COMMAND(saytomaster, "C");
COMMAND(saytoadmin, "C");
COMMANDN(mastermode, _mastermode, "i");
VARF(pause, 0, 0, 1, server::pausegame(pause));
COMMAND(clearbans, "");
COMMAND(setteam, "is");
COMMAND(getping, "i");
COMMAND(getonline, "i");
COMMANDN(getteamscore, _getteamscore, "s");
COMMAND(getrank, "i");

}
