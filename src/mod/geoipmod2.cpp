/*
* remod:    geoipmod2.cpp
* date:     2014, 2019
* author:   degrave
*
* GeoIP mod. Using maxminddb for translate IP to country name
*/

#ifdef GEOIP

#include <errno.h>
#include "maxminddb.h"
#include "geoipmod2.h"
#include "remod.h"

EXTENSION(GEOIP);

namespace remod
{

static MMDB_s mmdb;
static bool geoipAvailable = false;

void loadgeoip(const char *filename)
{
    const char *fname = findfile(filename, "r");

    if (!fname)
    {
        conoutf(CON_ERROR, "Geoip: can not load %s", filename);
        return;
    }

    int status = MMDB_open(fname, MMDB_MODE_MMAP, &mmdb);
    if (status != MMDB_SUCCESS)
    {
        conoutf(CON_ERROR, "Geoip: can not load %s: %s", filename, MMDB_strerror(status));
        if (status == MMDB_IO_ERROR)
        {
            conoutf(CON_ERROR, "Geoip: IO error: %s", strerror(errno));
        }
        return;
    }

    geoipAvailable = true;

    conoutf(CON_INFO, "Geoip: \"%s\" loaded", fname);
}

/**
 * Find country name by IP, if country can not be found NULL will be retunred
 */
const char *getcountry(const char *addr)
{
    if (!geoipAvailable)
    {
        return NULL;
    }

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(&mmdb, addr, &gai_error, &mmdb_error);

    if (gai_error != 0)
    {
        conoutf(CON_ERROR, "Geoip: error from getaddrinfo for %s - %s", addr, gai_strerror(gai_error));
        return NULL;
    }

    if (mmdb_error != MMDB_SUCCESS)
    {
        conoutf(CON_ERROR, "Geoip: error from libmaxminddb: %s", MMDB_strerror(mmdb_error));
        return NULL;
    }

    if (!result.found_entry)
    {
        return NULL;
    }

    MMDB_entry_data_s entry_data;
    int status = MMDB_get_value(&result.entry, &entry_data, "country", "names", "en", NULL);
    if (status != MMDB_SUCCESS)
    {
        conoutf(CON_ERROR, "Geoip: can not get entry value: %s", MMDB_strerror(status));
        return NULL;
    }

    if (!entry_data.has_data)
    {
        return NULL;
    }

    return newstring(entry_data.utf8_string, entry_data.data_size);
}

/**
 * Load geoip database from specified path
 * @group server
 * @arg1 /path/to/geoip.db
 */
COMMANDN(geodb, loadgeoip, "s");

/**
 * Return country for specified ip, if country can not be determinated return ip
 * @group server
 * @arg1 ip
 * @return country
 */
ICOMMAND(getcountry, "s", (const char *addr), { const char *country = getcountry(addr); result(country ? country : addr); });

/**
 * Check if geoip is ready to use
 * @group server
 * @return 1 if is ready, otherwise 0
 */
ICOMMAND(isgeoip, "", (), intret(geoipAvailable));

} // namespace remod

#endif