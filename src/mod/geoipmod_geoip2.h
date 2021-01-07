/*
* remod:    geoipmod_geoip2.h
* date:     2014, 2019, 2021
* author:   degrave
*
* GeoIP2 driver
*/

#ifndef __GEOIPMOD_GEOIP2_H__
#define __GEOIPMOD_GEOIP2_H__

#include "errno.h"
#include "maxminddb.h"
#include "geoipmod.h"

namespace remod
{
    namespace geoip
    {
        struct geoip2 : geoip
        {
            MMDB_s mmdb;
            bool loaded;

            geoip2() : loaded(false) {}

            bool loaddb(const char *path)
            {
                const char *fname = findfile(path, "r");

                if (!fname)
                {
                    conoutf(CON_ERROR, "Geoip: can not load %s", path);
                    return false;
                }

                int status = MMDB_open(fname, MMDB_MODE_MMAP, &mmdb);
                if (status != MMDB_SUCCESS)
                {
                    conoutf(CON_ERROR, "Geoip: can not load %s: %s", path, MMDB_strerror(status));
                    if (status == MMDB_IO_ERROR)
                    {
                        conoutf(CON_ERROR, "Geoip: IO error: %s", strerror(errno));
                    }
                    return false;
                }

                loaded = true;

                conoutf(CON_INFO, "Geoip: database \"%s\"", fname);

                return true;
            }

            bool isloaded()
            {
                return loaded;
            }

            const char *getcountry(const char *addr)
            {
                if (!loaded)
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
        };
    } // namespace geoip
} // namespace remod
#endif