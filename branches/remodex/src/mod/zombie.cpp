/*
* remod:    remodex.cpp
* date:     2011
* author:   degrave
*
* zombie mode
*/

#include "remod.h"
#include "zombie.h"

namespace remodex
{
    VAR(zombiehealth, 0, 0, INT_MAX);

    bool iszombie(clientinfo *ci)
    {
        // all zombies in evil team
        return (strcmp(ci->team, "evil") == 0);
    }

    // set zombie weapon
    void zombiestate(clientinfo *ci)
    {
        // set ammo and armour
        loopi(NUMGUNS) ci->state.ammo[i] = 0;
        ci->state.ammo[GUN_FIST] = 1;
        ci->state.gunselect = GUN_FIST;
        ci->state.armour = 0;

        // set zombie health
        if(zombiehealth) ci->state.health = zombiehealth;
    }
}
