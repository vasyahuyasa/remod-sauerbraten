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
        if(*wep>-1 && *wep<GUN_PISTOL)
        {
            ammoex[*wep] = *ammo;
        }
    }

    int getammo(int wep)
    {
        if(wep>-1 && wep<GUN_PISTOL)
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

    // bindings
    COMMANDN(ammo, setammo, "ii");
    COMMANDN(armourtype, setarmourtype, "i");
    COMMANDN(armour, setarmour, "i");
    COMMANDN(health, sethealth, "i");
    COMMANDN(gunselect, setgunselect, "i");
    COMMANDN(damagescale, setdamagescale, "ii");

    ICOMMAND(getammo, "i", (int *wep), intret(getammo(*wep)));
    ICOMMAND(getarmourtype, "", (), intret(getarmourtype()));
    ICOMMAND(getarmour, "", (), intret(getarmour()));
    ICOMMAND(gethealth, "", (), intret(gethealth()));
    ICOMMAND(getgunselect, "", (), intret(getgunselect()));
    ICOMMAND(getdamagescale, "i", (int *wep), intret(getdamagescale(*wep)));
}
