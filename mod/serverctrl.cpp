#include "fpsgame.h"

//Remod
namespace remod
{
    using namespace server;


    //Additional cubescript functions
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
        clientinfo *ci = (clientinfo *)getinfo((int)*cn);
        if(ci)
        {
            intret(ci->state.flags);
        }
    }

    void getmastermode()
    {
        intret(gamemode);
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
    }

    void spectator(int *val, int *cn)
    {
        clientinfo *spinfo = (clientinfo *)getclientinfo((int)*cn); // no bots
        if(!spinfo || (spinfo->state.state==CS_SPECTATOR ? (int)*val : !(int)*val)) return;

        if(spinfo->state.state!=CS_SPECTATOR && (int)*val)
        {
            if(spinfo->state.state==CS_ALIVE) suicide(spinfo);
            if(smode) smode->leavegame(spinfo);
            spinfo->state.state = CS_SPECTATOR;
            spinfo->state.timeplayed += lastmillis - spinfo->state.lasttimeplayed;
            if(!spinfo->local && !spinfo->privilege) aiman::removeai(spinfo);
        }
        else if(spinfo->state.state==CS_SPECTATOR && !(int)*val)
        {
            spinfo->state.state = CS_DEAD;
            spinfo->state.respawn();
            spinfo->state.lasttimeplayed = lastmillis;
            aiman::addclient(spinfo);
            if(spinfo->clientmap[0] || spinfo->mapcrc) checkmaps(-1);
            sendf(-1, 1, "ri", N_MAPRELOAD);
        }
        sendf(-1, 1, "ri3", N_SPECTATOR, spectator, (int)*val);
        if(!(int)*val && mapreload && !spinfo->privilege && !spinfo->local) sendf((int)*cn, 1, "ri", N_MAPRELOAD);
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
        if(!aiman::addai((int)*s, 0))
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

    //Cube script binds
    COMMAND(getmap, "");
	COMMAND(getmode, "");
	COMMAND(getip, "i");
	//COMMAND(getcountry, "s"); unimplemented
	COMMAND(getfrags, "i");
	COMMAND(getdeaths, "i");
	COMMAND(getteamkills, "i");
	COMMAND(getaccuracy, "i");
	COMMAND(getflags, "i");
	//COMMAND(getretflags, "i"); unimplemented
    COMMAND(getmastermode, "");
	COMMAND(getmastermodename, "i");
	ICOMMAND(ismaster, "i", (int *cn), intret(ismaster((int*)cn) ? 1 : 0));
	ICOMMAND(isadmin, "i", (int *cn), intret(isadmin((int*)cn) ? 1 : 0));
	ICOMMAND(isspectator, "i", (int *cn), intret(isspectator((int*)cn) ? 1 : 0));
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


}
