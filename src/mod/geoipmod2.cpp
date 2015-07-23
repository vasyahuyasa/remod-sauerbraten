/*
* remod:    geoipmod2.cpp
* date:     2014
* author:   degrave
*
* GEOIP new staff
*/

#include "GeoIP.h"
#include "GeoIPCity.h"
#include "geoipmod2.h"
#include "remod.h"

EXTENSION(GEOIP);

namespace remod
{
namespace geoip
{

    static GeoIP *geoip = NULL;
    static GeoIP *geocity = NULL;

    void loadgeoip(const char *path, bool isgeocity)
    {
        GeoIP *gi;
        string dbtype;
        memset(dbtype, 0,sizeof(dbtype));

        const char *fname = findfile(path, "r"); // full path
        gi = GeoIP_open(fname, GEOIP_STANDARD | GEOIP_MEMORY_CACHE);
        if(isgeocity)
            strcpy(dbtype, "geocity");
        else
            strcpy(dbtype, "geoip");

        if(gi)
        {
            if(isgeocity)
                geocity = gi;
            else
                geoip = gi;

            conoutf(CON_ERROR, "Geoip: %s loaded (db: \"%s\")", dbtype, fname);
        }
        else
        {
            conoutf(CON_ERROR, "Geoip: can not load %s (db: \"%s\")", dbtype, fname);
        }
    }

    const char *getcountry(const char *addr)
    {
        const char *country_name = NULL;
        if(geoip)
        {
            GeoIPLookup gl;
            country_name = GeoIP_country_name_by_addr_gl(geoip, addr, &gl);
        }

        return country_name;
    }

    const char *getcity(const char *addr)
    {
        char *city = NULL;
        if(geocity)
        {
            //GeoIPRecord *gir = GeoIP_record_by_ipnum(geocity, ipnum);
            GeoIPRecord *gir = GeoIP_record_by_addr(geocity, addr);
            if(gir != NULL)
            {
                char *city = gir->city != NULL ? newstring(gir->city) : NULL;
                GeoIPRecord_delete(gir);
            }
        }

        return city;
    }

    /**
    * Load geoip database from specified path
    * @group server
    * @arg1 /path/to/geoip.db
    */
    ICOMMAND(geodb, "s", (const char *path),
             {
                 loadgeoip(path, false);
             });

    /**
    * Load geoip city database from specified path
    * @group server
    * @arg1 /path/to/geoipcity.db
    */
    ICOMMAND(geocitydb, "s", (const char *path),
             {
                 loadgeoip(path, true);
             });

    /**
    * Return country for specified ip
    * @group server
    * @arg1 ip
    * @return country
    */
    ICOMMAND(getcountry, "s", (const char *addr),
             {

                 const char *country = getcountry(addr);
                 result(country != NULL ? country : addr);
             });

    /**
    * Return city for specified ip
    * @group server
    * @arg1 ip
    * @return city
    */
    ICOMMAND(getcity, "s", (const char *addr),
            {
                const char *city = getcity(addr);
                result(city != NULL ? city : addr);
            });

    /**
    * Check if geoip is ready to use
    * @group server
    * @return 1 if is ready, otherwise 0
    */
    ICOMMAND(isgeoip, "", (), intret(geoip != NULL));

    /**
    * Check if geoip city is ready to use
    * @group server
    * @return 1 if is ready, otherwise 0
    */
    ICOMMAND(isgeocity, "", (), intret(geocity != NULL));
}
}

