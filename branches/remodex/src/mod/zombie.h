/*
* remod:    remodex.cpp
* date:     2011
* author:   degrave
*
* zombie mode header
*/

#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include "game.h"
#include "fpsgame.h"

namespace remodex
{
    using namespace server;

    bool iszombie(clientinfo *ci);
    void zombiestate(clientinfo *ci);
}
#endif
