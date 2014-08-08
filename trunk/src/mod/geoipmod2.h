namespace remod
{
namespace geoip
{
    void loadgeoip(const char *path, bool geocity = false);
    const char *getcountry(char *addr);
    const char *getcity(char *addr);
}
}
