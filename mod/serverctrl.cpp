#include "fpsgame.h"
#include "serverctrl.h"

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

}
