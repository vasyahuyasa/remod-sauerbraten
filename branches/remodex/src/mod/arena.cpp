/*
* remod:    arena.cpp
* date:     2011
* author:   degrave
*
* arena mode
*/

#include "cube.h"
#include "fpsgame.h"

namespace remodex
{
    using namespace server;

    VAR(arenaspawndelay, 0, 5000, INT_MAX); // millis to spawn after round win (default 5 sec)
    int arenawin = -1;  // millis when last men standing
    bool onemaninfo = false; // show warning that only one man on server

    void arenasendspawn()
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i], *cq = ci;
            if(ci->state.state!=CS_SPECTATOR)
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
                cq->state.health = cq->state.maxhealth;
                ci->state.state = CS_ALIVE;
                sendspawn(cq);
            }
        }
    }

    bool arenacheckwin()
    {
        int total = 0;  // total count of not spectating players
        int stand = 0;  // count of alive players
        loopv(clients)
        {
            if(clients[i]->state.state != CS_SPECTATOR)
            {
                total++;

                if(clients[i]->state.state != CS_DEAD)
                {
                    stand++;
                }
            }
        }

        // show warning if less than 2 players
        if(total<2)
        {
            // dont show message multiply times
            if(!onemaninfo)
            {
                sendservmsg("Wait for other players to start");
                onemaninfo = true;
            }

            return false;
        }
        onemaninfo = false; // show warning again if needed

        // debug
        if(stand<=1)
        {
            sendservmsg("Last man standing!");

            return true;
        }

        if(!m_teammode) return false; // team modes checks
        string teamname;
        teamname[0] = 0;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci && ci->state.state != CS_DEAD && ci->state.state != CS_SPECTATOR)
            {
                if(teamname[0]) // first run
                {
                    strcpy(teamname, ci->team);
                }
                else
                {
                    if(strcmp(teamname, ci->team) != 0) return false; // diffirent teams
                }
            }
        }

        // all remain players in one team
        return true;
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
                sendservmsg("arena win, one man(team) stand");
                arenawin = gamemillis;
            }
        }
    }
}
