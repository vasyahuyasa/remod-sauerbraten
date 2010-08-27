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
         gi = GeoIP_open("GeoIP.dat", GEOIP_STANDARD);
         if (gi == NULL) conoutf(CON_ERROR, "Geoip: cannot inizialize\n");
            else conoutf("Geoip: initialized with 'GeoIP.dat'\n");
    }

    ~GeoIPtool()
    {
        GeoIP_delete(gi);
    }

    char *getcountry(char *host)
    {
        if(gi)
        {
            const  char *returncountryname = GeoIP_country_name_by_addr(gi,host);
            return (char*)returncountryname;
        } else return NULL;
    }
};

GeoIPtool *GIt = new GeoIPtool;

void getcountry(char *ip)
{
    char *country=NULL;
    country = GIt->getcountry(ip);
    if(country == NULL) country=ip;
    result(country);
}

//Remod cubescript bindings
COMMAND(getcountry,"s");

}
