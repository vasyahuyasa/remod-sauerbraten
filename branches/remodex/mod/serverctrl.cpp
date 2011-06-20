/*
* remod:    serverctrl.cpp
* date:     2007
* author:   degrave
*
* additional cubescript functions
*/


#include "fpsgame.h"
#include "commandev.h"
#include "commandhandler.h"
#include "remod.h"

#ifndef WIN32
#include <arpa/inet.h>
#endif

#include "version.inc"

//Remod
namespace remod
{

void getname(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        result(ci->name);
    }
}

void getip(int *pcn)
{
    int cn=(int)*pcn;
    if(cn<(getnumclients()) && cn>=0)
    {
        in_addr addr;
        addr.s_addr = getclientip(cn);
        char *ip = inet_ntoa(addr);
        result(ip);
    }
}

void getfrags(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.frags);
    }
}

void getdeaths(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.deaths);
    }
}

void getteamkills(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.teamkills);
    }
}

void getaccuracy(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.damage*100/max(ci->state.shotdamage,1));
    }
}

void getflags(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.flags);
    }
}

void version()
{
    string txt;
    formatstring(txt)("Remod %s (build %s %s) %s/%s", REMOD_VERSION, __DATE__, __TIME__, REMOD_SYSTEM, REMOD_ARCH);
    result(txt);
}

void getteam(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        result(ci->team);
    }
}

void kick(int *pcn, int *pexpire, char *actorname)
{
    int cn=(int)*pcn;
    int expire = (int)*pexpire;
    if (!pexpire || expire<=0) expire = 4*60*60000; //4 hours - default
    expire += totalmillis;  //add current uptime
        remod::onevent("onkick", "ii", -1, cn);
    if(strlen(actorname) == 0) actorname = newstring("console");
    server::kick(cn, actorname, expire);
    }

void spectator(int *st, int *pcn)
{
    int spectator = (int)*pcn;
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

void _suicide(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        suicide(ci);
    }
}

void pm(int *pcn, char *msg)
{
    int cn = (int)*pcn;
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->clientnum == cn)
        {
            sendf(cn, 1, "ris", N_SERVMSG, msg);
        }
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
    if(mastermode>=MM_PRIVATE)
    {
        loopv(clients) allowedips.add(getclientip(clients[i]->clientnum));
    }
    sendf(-1, 1, "rii", N_MASTERMODE, mastermode);
}

void setteam(int *pcn, const char *team)
{
    int cn=(int)*pcn;
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

void getping(int *pcn)
{
    int cn=(int)*pcn;
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

void getrank(int *pcn)
{
    int cn=(int)*pcn;
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

void mute(int *pcn, int *val)
{
    int cn = (int)*pcn;
    bool v = (bool)*val;

    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        if(ci->state.muted != v)
        {
            ci->state.muted = v;
            remod::onevent("onmute", "ii", v ? 1 : 0, cn);
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
    // %i - milliseconds, %s - seconds, %m - minutes, %h - hours, %d - days
    int mseconds, seconds, minutes, hours, days;
    vector<char> s;

    days = (int)*millis/(1000*60*60*24);
    hours = ((int)*millis/(1000*60*60))-(days*24);
    minutes = ((int)*millis/(1000*60))-(days*24*60+hours*60);
    seconds = ((int)*millis/1000)-(days*24*60*60+hours*60*60+minutes*60);
    mseconds  = *millis - (days*24*60*60+hours*60*60+minutes*60+seconds)*1000;

    while(*fmt)
    {
        int c = *fmt++;
        if(c == '%')
        {
            int i = *fmt++;
            switch(i)
            {
            	case 'i':
				{
					const char *smseconds = intstr(mseconds);
					while(*smseconds) s.add(*smseconds++);
					break;
				}

                case 's':
                {
                    const char *sseconds = newstring(2);
                    sprintf((char*)sseconds, "%02d", seconds);
                    while(*sseconds) s.add(*sseconds++);
                    // xxx DELETEA(sseconds);
                    break;
                }

                case 'm':
                {
                    const char *sminutes = newstring(2);
                    sprintf((char*)sminutes, "%02d", minutes);
                    while(*sminutes) s.add(*sminutes++);
                    // xxx DELETEA(sminutes);
                    break;
                }

                case 'h':
                {
                    const char *shours = newstring(2);
                    sprintf((char*)shours, "%02d", hours);
                    while(*shours) s.add(*shours++);
                    // xxx DELETEA(shours);
                    break;
                }

                case 'd':
                {
                    const char *sdays;
                    sdays = intstr(days);
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

void setmastercmd(int *val, int *pcn)
{
    bool v = (bool)*val;
    int cn = (int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        const char *name = "";
        if(!v)
        {
            if(ci->privilege>PRIV_NONE)
            {
                name = privname(ci->privilege);
                revokemaster(ci);
            }  else return;
        }
        else
        {
            if(ci->privilege>=PRIV_MASTER) return;
            loopv(clients) if(clients[i]->privilege>PRIV_NONE) revokemaster(clients[i]);
            ci->privilege = PRIV_MASTER;
            name = privname(ci->privilege);
        }

        remod::onevent("onsetmaster", "iiss", ci->clientnum, v ? 1:0, "", "");

        string msg;
        formatstring(msg)("%s %s %s", colorname(ci, NULL), v ? "claimed" : "relinquished", name);
        sendservmsg(msg);

        mastermode = MM_OPEN;
        allowedips.shrink(0);
        currentmaster = val ? ci->clientnum : -1;
        sendf(-1, 1, "ri4", N_CURRENTMASTER, currentmaster, currentmaster >= 0 ? ci->privilege : 0, mastermode);
    }
}


/**
 * checks if ip matches mask
 * ip = "127.0.0.1",	mask = "127.0.0.1"		--  true
 * ip = "192.168.1.23",	mask = "192.168.1.*"	--  true
 * ip = "127.0.0.1",	mask = "128.0.0.1"		--  false
 */
bool checkipbymask(char *ip, char *mask) {
	string istr;
	string mstr;

	strcpy(istr, ip);
	strcpy(mstr, mask);

	bool b = true;

	while (b && strlen(istr) > 0) {
		char *idot = strchr(istr, '.');
		string iseg;
		if (idot) {
			int c = idot-istr;
			strncpy(iseg, istr, c);
			iseg[c] = '\0';
			strcpy(istr, idot+1);
		} else {
			strcpy(iseg, istr);
			istr[0] = '\0';
		}

		char *mdot = strchr(mstr, '.');
		string mseg;
		if (mdot) {
			int c = mdot-mstr;
			strncpy(mseg, mstr, c);
			mseg[c] = '\0';
			strcpy(mstr, mdot+1);
		} else {
			strcpy(mseg, mstr);
			mstr[0] = '\0';
		}

		if (strcmp(iseg, mseg) != 0 && strcmp(mseg, "*") != 0) {
			b = false;
		}
	}
	return b;
}

void loopbans(const char *name, const char *ip, const char *expire, const char *actor, const char *actorip, const char *body)
{
	ident* idents[5];
	idents[0] = newident(name);
	idents[1] = newident(ip);
	idents[2] = newident(expire);
	idents[3] = newident(actor);
	idents[4] = newident(actorip);

	loopi(5)
	{
		if (idents[i]->type != ID_ALIAS) return;
	}
	in_addr addr;
	loopv(bannedips)
	{
		ban b = bannedips[i];
		if (i) {
			aliasa(idents[0]->name, newstring(b.name));
			addr.s_addr = b.ip;
			aliasa(idents[1]->name, newstring(inet_ntoa(addr)));
			aliasa(idents[2]->name, newstring(intstr(b.expire)));
			aliasa(idents[3]->name, newstring(b.actor));
			addr.s_addr = b.actorip;
			aliasa(idents[4]->name, newstring(inet_ntoa(addr)));
		} else {
			pushident(*idents[0], newstring(b.name));
			addr.s_addr = b.ip;
			pushident(*idents[1], newstring(inet_ntoa(addr)));
			pushident(*idents[2], newstring(intstr(b.expire)));
			pushident(*idents[3], newstring(b.actor));
			addr.s_addr = b.actorip;
			pushident(*idents[4], newstring(inet_ntoa(addr)));
		}
		execute(body);
	}

	if (bannedips.length())
	{
        loopi(5)
        {
			popident(*idents[i]);
		}
	}
}

// system time format see http://www.cplusplus.com/reference/clibrary/ctime/strftime/
void systimef(const char *format)
{
    time_t now;
    struct tm *timeinfo;
    string buf;

    time(&now);
    timeinfo = localtime(&now);
    strftime(buf, MAXSTRLEN, format, timeinfo);
    result(buf);
}

//Cube script binds
COMMAND(getname, "i");
ICOMMAND(getmap, "", (), result(smapname));
ICOMMAND(getmode, "", (), intret(gamemode));
COMMAND(getip, "i");
COMMAND(getfrags, "i");
COMMAND(getdeaths, "i");
COMMAND(getteamkills, "i");
COMMAND(getaccuracy, "i");
COMMAND(getflags, "i");
//COMMAND(getretflags, "i"); unimplemented
ICOMMAND(getmastermode, "", (), intret(mastermode));
ICOMMAND(getmastermodename, "i", (int *mm), result(mastermodename((int)*mm, "unknown")));

ICOMMAND(ismaster, "i", (int*cn), intret(ismaster(cn) ? 1 : 0));
ICOMMAND(isadmin, "i", (int *cn), intret(isadmin(cn) ? 1 : 0));
ICOMMAND(isspectator, "i", (int *cn), intret(isspectator(cn) ? 1 : 0));
COMMAND(version, "");
COMMAND(getteam,"i");
ICOMMAND(disconnect, "i", (int *cn), disconnect_client(*cn, DISC_NONE));
COMMAND(kick, "iis");
COMMAND(spectator, "ii");
ICOMMAND(map, "s", (char *name), sendf(-1, 1, "risii", N_MAPCHANGE, name, gamemode, 1); server::changemap(name, gamemode));
ICOMMAND(mapmode, "si", (char *name, int *mode), sendf(-1, 1, "risii", N_MAPCHANGE, name, *mode, 1); server::changemap(name, *mode));
COMMANDN(suicide, _suicide, "i");
ICOMMAND(addbot, "i", (int *s), aiman::addai(*s, -1));
ICOMMAND(delbot, "", (), aiman::deleteai());
ICOMMAND(say, "C", (char *msg), sendservmsg(msg));
COMMAND(pm, "is");
COMMAND(saytonormal, "C");
COMMAND(saytomaster, "C");
COMMAND(saytoadmin, "C");
COMMANDN(mastermode, _mastermode, "i");
VARF(pause, 0, 0, 1, server::pausegame(pause));
ICOMMAND(clearbans, "", (), remod::onevent("onclearbans", ""); bannedips.shrink(0); sendservmsg("cleared all bans"));
COMMAND(setteam, "is");
COMMAND(getping, "i");
COMMAND(getonline, "i");
COMMANDN(getteamscores, _getteamscore, "s");
COMMAND(getrank, "i");
COMMAND(addgban, "s");
COMMAND(cleargbans, "");
ICOMMAND(numclients, "", (), intret(numclients(-1, false, true, false)));
ICOMMAND(playerexists, "i", (int *cn), intret(playerexists(cn)));
COMMAND(mute, "ii");
COMMAND(ismuted, "i");
COMMAND(formatmillis, "si");
ICOMMAND(getcn, "s", (char *name), int cn = parseplayer(name); intret(cn));
ICOMMAND(halt, "i", (int *err), exit(*err));
COMMANDN(setmaster, setmastercmd, "ii");
ICOMMAND(checkipbymask, "ss", (char *ip, char *mask), intret(checkipbymask(ip, mask) ? 1 : 0));
ICOMMAND(loopbans,
		"ssssss",
		(char *name, char *ip, char *expire, char *actor, char *actorip, char *body),
		loopbans(name, ip, expire, actor, actorip, body));
ICOMMAND(delban, "i", (int *n), if(bannedips.inrange(*n)) bannedips.remove(*n));
COMMAND(systimef, "s");
COMMAND(setlogfile, "s");
ICOMMAND(echo, "C", (char *s), conoutf("%s", s));
}
