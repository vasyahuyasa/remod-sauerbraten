#ifndef __GEOIPMOD_H__
#define __GEOIPMOD_H__

#include "fpsgame.h"


namespace remod
{
    namespace geoip
    {
        struct geoip
        {
            virtual bool loaddb(const char *path) { return false; }
            virtual bool isloaded() { return false; }
            virtual const char *getcountry(const char *addr) { return NULL; }
        };
    } // namespace geoip
} // namespace remod
#endif
