/*
* remod:    geoip.cpp
* date:     2007
* author:   degrave
*
* GEOIP staff
*/



#ifdef GEOIP

#include "geoipmod.h"
#include "remod.h"

EXTENSION(GEOIP);

namespace remod
{

GeoIPtool *GIt = new GeoIPtool();

void loadgeoip(char *dbname)
{
    const char *fname = findfile(dbname, "r"); // full path

    if(GIt->loaddb(fname))
    {
        conoutf(CON_ERROR, "Geoip: loaded (db: \"%s\")", fname);
    }
    else
    {
        conoutf(CON_ERROR, "Geoip: can not load (db: \"%s\")", fname);
    }
}

void getcountry(char *ip)
{
    const char *country = NULL;
    country = GIt->getcountry(ip);
    if(!country) country = ip;
    result(country);
}

COMMAND(getcountry,"s");
COMMANDN(geodb, loadgeoip, "s");
ICOMMAND(isgeoip, "", (), intret(GIt->loaded()));

}
#endif
