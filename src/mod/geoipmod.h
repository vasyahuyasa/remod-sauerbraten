#ifndef __GEOIPMOD_H__
#define __GEOIPMOD_H__

#include "fpsgame.h"
#include "geoipmod_legacy.h"
#include "geoipmod_geoip2.h"

namespace remod
{
    namespace geoip
    {
        struct geoip
        {
            virtual bool loaddb(const char *path);
            virtual bool isloaded();
            virtual const char *getcountry(const char *addr);
        };
    }    
}
#endif
