#ifndef __REMODEX_H__
#define __REMODEX_H__

namespace remodex
{
    enum { GUN_FIST = 0, GUN_SG, GUN_CG, GUN_RL, GUN_RIFLE, GUN_GL, GUN_PISTOL, GUN_FIREBALL, GUN_ICEBALL, GUN_SLIMEBALL, GUN_BITE, GUN_BARREL, NUMGUNS };
    enum { A_BLUE, A_GREEN, A_YELLOW };
    int getammo(int wep);
    int getarmour();
    int getarmourtype();
    int gethealth();
    int getgunselect();
    int getdamagescale(int wep);
    void arenamodeupdate();
}
#endif
