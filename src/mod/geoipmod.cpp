/*
* remod:    geoipmod.cpp
* date:     2007, 2021
* author:   degrave
*
* GEOIP staff
*/

#include "remod.h"
#include "geoipmod_geoip2.h"
#include "geoipmod_legacy.h"
#include "libgen.h"

#define LEGACY_EXTENSION ".dat"
#define LEGACY_EXTENSION_LEN 4

EXTENSION(GEOIP);

namespace remod
{
    namespace geoip
    {
        geoiplegacy legacygeo;
        geoip2 geo2;
        geoip *geo = NULL;

        int islegacydatabase(const char *path)
        {
            const char *filename = basename((char*)path);

            size_t namelen = strlen(filename);

            // filename extension is .dat then suppose it is legacy database otherwise suppose it is geoip 2 database
            return namelen < LEGACY_EXTENSION_LEN || strncasecmp(filename + namelen - LEGACY_EXTENSION_LEN, LEGACY_EXTENSION, LEGACY_EXTENSION_LEN) == 0;
        }

        void geodb(char *path)
        {
            if (strlen(path) == 0) {
                conoutf(CON_INFO, "Geoip: geoip is disabled because geodb = \"\"");
                return;
            }

            const char *fullpath = findfile(path, "r");

            geo = &geo2;
            if(islegacydatabase(fullpath)) {
                geo = &legacygeo;
            }

            if (geo->loaddb(fullpath))
            {
                conoutf(CON_INFO, "Geoip: use database \"%s\"", fullpath);
                return;
            }

            conoutf(CON_ERROR, "Geoip: can not load database \"%s\"", fullpath);
        }

        void getcountry(char *addr)
        {
            const char *country = geo->getcountry(addr);

            result(country ?: "Unknown");
        }

        /**
         * Return country for specified ip
         * @group server
         * @arg1 ip
         * @return country name
         */
        COMMAND(getcountry, "s");

        /**
         * Load geoip database from path. Geoip legacy and Geoip2 is supported, driver selected by file extension (.dat for geoip legacy and any other for geoip2). https://dev.maxmind.com/geoip/geoip2/geolite2/
         * @group server
         * @arg1 path to database file
         */
        COMMAND(geodb, "s");


        /**
         * Check if geoip is ready to use
         * @group server
         * @return 1 if is ready, otherwise 0
         */
        ICOMMAND(isgeoip, "", (), intret(geo->isloaded()));

    } // namespace geoip
} // namespace remod
