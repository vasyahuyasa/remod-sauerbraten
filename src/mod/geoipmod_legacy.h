/*
* remod:    geoipmod_legacy.h
* date:     2007, 2021
* author:   degrave
*
* legacy GEOIP support, former geoipmod.h
*/

#ifndef __GEOIPMOD_LEGACY_H__
#define __GEOIPMOD_LEGACY_H__

#include "GeoIP.h"
#include "geoipmod.h"

namespace remod
{
    namespace geoip
    {
        struct geoiplegacy : geoip
        {
            GeoIP *gi;

            geoiplegacy() : gi(NULL) {}

            ~geoiplegacy()
            {
                GeoIP_delete(gi);
            }

            bool loaddb(const char *path)
            {
                gi = GeoIP_open(path, GEOIP_STANDARD | GEOIP_MEMORY_CACHE);

                return gi != NULL;
            }

            bool isloaded()
            {
                return gi != NULL;
            }

            const char *getcountry(const char *addr)
            {
                if (!isloaded())
                {
                    return NULL;
                }

                return GeoIP_country_name_by_addr(gi, addr);
            }
        };
    } // namespace geoip
} // namespace remod

#endif
