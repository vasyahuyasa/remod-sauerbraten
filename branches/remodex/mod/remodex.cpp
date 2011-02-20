/*
* remod:    remodex.cpp
* date:     2011
* author:   degrave
*
* remodex tools
*/

#include "cube.h"
#include "fpsgame.h"
#include "remodex.h"

namespace remodex
{
    using namespace server;
    //using namespace server;

    // extendet ammo -1 use default ammo
    int ammoex[NUMGUNS] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
    int armourtypeex = -1; // -1 default, 0 blue, 1 green, 2 yellow
    int armourex = -1; // -1 default armour num
    int health = 0; // 0 default health
    int gunselect = -1;
    // multyply damage, 0 no damage, 1 normal damage
    int damagescale[NUMGUNS] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

    // ammo control
    void setammo(int *wep, int *ammo)
    {
        if(*wep>-1 && *wep<NUMGUNS)
        {
            ammoex[*wep] = *ammo;
        }
    }

    int getammo(int wep)
    {
        if(wep>-1 && wep<NUMGUNS)
        {
            return ammoex[wep];
        } else return -1;
    }

    // armour control
    void setarmour(int *n)
    {
        if(*n>-1)
        {
            armourex = *n;
        }
    }

    void setarmourtype(int *type)
    {
        if(*type>=(A_BLUE-1) && *type<=A_YELLOW)
        {
            armourtypeex = *type;
        }
    }

    int getarmour()
    {
        return armourex;
    }

    int getarmourtype()
    {
        return armourtypeex;
    }

    // health
    void sethealth(int *n)
    {
        if(*n>0) health = *n;
    }

    int gethealth()
    {
        return(health);
    }

    // gunselect
    void setgunselect(int *n)
    {
        if(*n>=-1 && *n<NUMGUNS)
        {
            gunselect = *n;
        }
    }

    int getgunselect()
    {
        return gunselect;
    }

    // damage control
    void setdamagescale(int *wep, int *s)
    {
        if(*wep>=0 && *wep<NUMGUNS)
        {
            damagescale[*wep] = *s;
        }
    }

    int getdamagescale(int wep)
    {
        if(wep>=0 && wep<NUMGUNS)
        {
            return(damagescale[wep]);
        } else return 1;
    }

    // arena mode
    VAR(arenaspawndelay, 0, 5000, INT_MAX); // millis to spawn after round win (default 5 sec)
    int arenawin = -1; // millis when last men standing

    void arenasendspawn()
    {
        loopv(clients)
        {
            clientinfo *ci = getinfo(i), *cq = ci;
            if(ci && ci->state.state!=CS_SPECTATOR)
            {
                if(!ci->clientmap[0] && !ci->mapcrc)
                {
                    ci->mapcrc = -1;
                    checkmaps(-1);
                }
                if(cq->state.lastdeath)
                {
                    flushevents(cq, cq->state.lastdeath + DEATHMILLIS);
                    cq->state.respawn();
                }
                cleartimedevents(cq);
                sendspawn(cq);
                ci->state.state = CS_ALIVE;
            }
        }
    }



    bool arenacheckwin()
    {
        int stand = 0; // people remain
        loopv(clients)
        {
            clientinfo *ci = getinfo(i);
            if(ci && ci->state.state != CS_DEAD && ci->state.state != CS_SPECTATOR) stand++;
        }
        if(stand<=1)
        {
        loopv(clients)
        {
            clientinfo *ci = getinfo(i);
            if(ci)
            {
                defformatstring(msg)("clients[%i].state=%i", ci->clientnum, ci->state.state);
                sendservmsg(msg);
            }
        }

           return true;
        }
        if(!m_teammode) return false; // check for team mode
        string teamname = "";
        loopv(clients)
        {
            clientinfo *ci = getinfo(i);
            if(ci && ci->state.state != CS_DEAD && ci->state.state != CS_SPECTATOR)
            {
                if(teamname[0]) // first run
                {
                    strcpy(teamname, ci->team);
                }
                else
                {
                    if(strcmp(teamname, ci->team) != -1) return false; // difirent teams
                }
            }
        }
        return true; // all in one team;
    }

    void arenamodeupdate()
    {
        if(arenawin > -1) // round ended wait for start
        {
            if(arenawin <= (gamemillis - arenaspawndelay)) // wait time elpsed
            {
                arenasendspawn();
                arenawin = -1;
            }
        }
        else // check win condition
        {
            if(arenacheckwin()) // one man or team stand
            {
                sendservmsg("arena winw");
                arenawin = gamemillis;
            }
        }
    }

    // bindings
    COMMANDN(ammo, setammo, "ii");
    COMMANDN(armourtype, setarmourtype, "i");
    COMMANDN(armour, setarmour, "i");
    COMMANDN(health, sethealth, "i");
    COMMANDN(gunselect, setgunselect, "i");
    COMMANDN(damagescale, setdamagescale, "ii");

}
