#ifndef __GEOIPMOD2_H__
#define __GEOIPMOD2_H__

namespace remod
{
namespace geoip
{
void loadgeoip(const char *path);
const char *getcountry(const char *addr);
const char *getcity(const char *addr);
} // namespace geoip
} // namespace remod
#endif