/*
* remod:    geoip.cpp
* date:     2007
* author:   degrave
*
* GEOIP staff
*/



#ifdef GEOIP

#include "geoipmod.h"

namespace remod
{

GeoIPtool *GIt;

void loadgeoip(char *dbname)
{
    GIt = new GeoIPtool(dbname);
    if(GIt->loaded())
    {
        conoutf(CON_ERROR, "Geoip: loaded (db: '%s')", dbname);
    }
    else
    {
        conoutf(CON_ERROR, "Geoip: can not load (db: '%s')", dbname);
    }
}

void getcountry(char *ip)
{
    char *country=NULL;
    country = GIt->getcountry(ip);
    if(country == NULL) country=ip;
    result(country);
}

COMMAND(getcountry,"s");
COMMANDN(geodb, loadgeoip, "s");
ICOMMAND(isgeoip, "", (), intret(GIt->loaded()));

}
#endif
