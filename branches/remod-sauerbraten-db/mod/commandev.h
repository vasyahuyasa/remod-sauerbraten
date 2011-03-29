#ifndef __COMMANDEV_H__
#define __COMMANDEV_H__

#include "fpsgame.h"

namespace remod
{
    struct evt_handler
    {
        string evt_type;
        string evt_cmd;
    };

    bool iseat(const char *evt_type);
    bool onevent(const char *evt_type, const char *fmt, ...);
}
#endif
