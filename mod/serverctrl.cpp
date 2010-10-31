#include "fpsgame.h"
#include "commandev.h"
#include "remod.h"

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

void getip(char *name)
{
    int cn = parseplayer(name);
    if(cn<(getnumclients()) && cn>=0)
    {
        in_addr addr;
        addr.s_addr = getclientip(cn);
        char *ip = inet_ntoa(addr);
        result(ip);
    }
}

void getfrags(char *name)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.frags);
    }
}

void getdeaths(char *name)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.deaths);
    }
}

void getteamkills(char *name)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.teamkills);
    }
}

void getaccuracy(char *name)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.damage*100/max(ci->state.shotdamage,1));
    }
}

void getflags(char *name)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
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

void version()
{
    string txt;
    formatstring(txt)("Remod $Rev$ (build %s %s)", __DATE__, __TIME__);
    result(txt);
}

void getteam(char *name)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        result(ci->team);
    }
}

void disconnect(int *cn)
{
    disconnect_client((int)*cn, DISC_NONE);
}

void kick(char *name)
{
    int cn = parseplayer(name);
    ban &b = bannedips.add();
    b.time = totalmillis;
    b.ip = getclientip(cn);
    allowedips.removeobj(b.ip);
    disconnect_client(cn, DISC_KICK);
    remod::onevent("onkick", "ii", -1, cn);
}

void spectator(int *st, char *name)
{
    int spectator = parseplayer(name);;
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

void _suicide(char *name)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
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

void setteam(char *name, const char *team)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(!ci && !strcmp(ci->team, team)) return;
    remod::onevent("onsetteam", "is", cn, team);
    if(!smode || smode->canchangeteam(ci, ci->team, team))
    {
        if(ci->state.state==CS_ALIVE) suicide(ci);
        copystring(ci->team, team, MAXTEAMLEN+1);
    }
    aiman::changeteam(ci);
    sendf(-1, 1, "riisi", N_SETTEAM, cn, ci->team, 1);
}

void getping(char *name)
{
    int cn = parseplayer(name);
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->ping);
    }
}

void getonline(int *pcn)
{
    int cn = (int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(totalmillis-ci->connectmillis);
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

void getrank(char *name)
{
    int cn = parseplayer(name);
    vector<clientinfo *> uplayers, splayers; //unsorted sorted

    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->state.state != CS_SPECTATOR) uplayers.add(ci);
        //conoutf("%i %s[%i] %i frags", i, ci->name, ci->clientnum, ci->state.frags);
    }

    while(uplayers.length()>0)
    {
        clientinfo *ci = findbest(uplayers);
        splayers.add(ci);
    }

    loopv(splayers)
    {
        if(splayers[i]->clientnum == cn)
        {
            intret(i+1);
            return;
        }
    }
    intret(-1);
}

void _addgban(const char *name)
{
    addgban(name);
}

void _cleargbans()
{
    cleargbans();
}

void _numclients()
{
    intret(numclients(-1, false, true, false));
}

void playerexists(char *name)
{
    int cn = parseplayer(name);
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->clientnum==cn)
        {
            intret(1);
            return;
        }
    }
    intret(0);
}

void mute(char *name, char *val)
{
    int cn = parseplayer(name);
    if(val[0]) //if player specified
    {
        int i = atoi(val);
        clientinfo *ci = (clientinfo *)getinfo(cn);
        if(ci)
        {
            ci->state.muted=i;
            remod::onevent("onmute", "ii", cn, i ? 1 : 0);
        }
    }
}

void ismuted(int *icn)
{
    int cn = (int)*icn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    intret(ci && ci->state.muted);
}

void formatmillis(char *fmt, int *millis)
{
    // %s - seconds, %m - minutes, %h - hours, %d - days
    int seconds, minutes, hours, days;
    vector<char> s;

    days = (int)*millis/(1000*60*60*24);
    hours = ((int)*millis/(1000*60*60))-(days*24);
    minutes = ((int)*millis/(1000*60))-(days*24*60+hours*60);
    seconds = ((int)*millis/1000)-(days*24*60*60+hours*60*60+minutes*60);

    while(*fmt)
    {
        int c = *fmt++;
        if(c == '%')
        {
            int i = *fmt++;
            switch(i)
            {
                case 's':
                {
                    const char *sseconds = intstr(seconds);
                    while(*sseconds) s.add(*sseconds++);
                    break;
                }

                case 'm':
                {
                    const char *sminutes = intstr(minutes);
                    while(*sminutes) s.add(*sminutes++);
                    break;
                }

                case 'h':
                {
                    const char *shours = intstr(hours);
                    while(*shours) s.add(*shours++);
                    break;
                }

                case 'd':
                {
                    const char *sdays = intstr(days);
                    while(*sdays) s.add(*sdays++);
                    break;
                }

                default: s.add(i);
            }
        } else s.add(c);
    }
    s.add('\0');
    result(s.getbuf());
}

void getcn(char *name)
{
    int cn = parseplayer(name);
    intret(cn);
}

//Cube script binds
COMMAND(getname, "i");
COMMAND(getmap, "");
COMMAND(getmode, "");
COMMAND(getip, "s");
COMMAND(getfrags, "s");
COMMAND(getdeaths, "s");
COMMAND(getteamkills, "s");
COMMAND(getaccuracy, "s");
COMMAND(getflags, "s");
//COMMAND(getretflags, "i"); unimplemented
COMMAND(getmastermode, "");
COMMAND(getmastermodename, "i");
ICOMMAND(ismaster, "i", (int*cn), intret(ismaster(cn) ? 1 : 0));
ICOMMAND(isadmin, "i", (int *cn), intret(isadmin(cn) ? 1 : 0));
ICOMMAND(isspectator, "i", (int *cn), intret(isspectator(cn) ? 1 : 0));
COMMAND(version, "");

COMMAND(getteam,"s");
COMMAND(disconnect, "i");
COMMAND(kick, "s");
COMMAND(spectator, "is");
COMMAND(map, "s");
COMMAND(mapmode, "si");
COMMANDN(suicide, _suicide, "s");
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
COMMAND(setteam, "ss");
COMMAND(getping, "s");
COMMAND(getonline, "i");
COMMANDN(getteamscore, _getteamscore, "s");
COMMAND(getrank, "s");
COMMANDN(addgban, _addgban, "s");
COMMANDN(cleargbans, _cleargbans, "");
COMMANDN(numclients, _numclients, "");
COMMAND(playerexists, "s");
COMMAND(mute, "ss");
COMMAND(ismuted, "i");
COMMAND(formatmillis, "si");
COMMAND(getcn, "s");
}
