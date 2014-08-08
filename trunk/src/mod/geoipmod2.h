namespace remod
{
namespace geoip
{
    void loadgeoip(const char *path, bool geocity = false);
    const char *getcountry(const char *addr);
    const char *getcity(const char *addr);
}
}
