#ifndef __GEOIPMOD_H__
#define __GEOIPMOD_H__

#include "fpsgame.h"
#include "GeoIP.h"

namespace remod
{
    class GeoIPtool
    {
        private:
        GeoIP *gi;

        public:
        GeoIPtool()
        {
            gi = GeoIP_open("GeoIP.dat", GEOIP_MEMORY_CACHE);
        }

        GeoIPtool(char *dbname)
        {

            gi = GeoIP_open(dbname, GEOIP_MEMORY_CACHE);
        }

        ~GeoIPtool()
        {
            GeoIP_delete(gi);
        }

        bool loaded()
        {
            if(gi != NULL) return true; else return false;
        }

        char *getcountry(char *host)
        {
            if(gi)
            {
                const char *name;
                name = GeoIP_country_name_by_addr(gi, host);
                return (char*)name;
            } else return NULL;
        }
    };
}
#endif
