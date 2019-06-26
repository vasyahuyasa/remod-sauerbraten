#include "banlist.h"

namespace remod
{
    namespace banlist
    {
        baninfo::baninfo()
        {
            ip      = 0;
            mask    = 0;
            expire  = 0;
            time    = 0;
            admin[0] = '\0';
            reason[0] = '\0';
        }

        // banlist
        banlist::banlist()
        {
            name = NULL;
        }

        banlist::banlist(char *listname)
        {
            if(listname)
                name = newstring(listname);
        }

        void banlist::add(baninfo *ban)
        {
            if(ban)
                bans.add(ban);
        }

        baninfo* banlist::remove(int n)
        {
            if(bans.inrange(n))
                return bans.remove(n);
            return NULL;
        }

        size_t banlist::length()
        {
            return bans.length();
        }

        // banmanager
        banmanager::banmanager()
        {
            // create local banlist
            banlist *bl = new banlist;
            banlists.add(bl);
        }

        // Try to find banlist by name. If banlist absent create new one if name is null
        // or empty string return local banlist
        banlist* banmanager::getbanlist(char *name)
        {
            // local banlist without name
            if(!name || !name[0])
            {
                return banlists[0]; 
            }
                        
            banlist *bl = NULL;

            // banlists[0]->name is always is null need to avoid it          
            loopv(banlists)
                if(banlists[i]->name && strcmp(banlists[i]->name, name))
                {
                    bl = banlists[i];
                    break;
                } 

            return bl != NULL ? bl : new banlist(name);
        }

        banlist* banmanager::localbanlist()
        {
            return banlists[0];
        }

        baninfo* banmanager::getban(char *listname, size_t n)
        {
            if(!banlistexists(listname)) return NULL;

            banlist* bl = getbanlist(listname);
            if(bl->bans.inrange(n))
                return bl->bans[n];
            else
                return NULL;
        }

        bool banmanager::banlistexists(char *name)
        {
            // local ban list
            if(!name || !name[0]) return true;

            banlist *bl;
            loopv(banlists)
            {
                bl = banlists[i];
                if(bl->name && (strcmp(bl->name, name) == 0))
                    return true;
            }
            return false;
        }

        void banmanager::addban(char *listname, enet_uint32 ip, enet_uint32 mask, time_t expire, time_t time, const char *admin, const char *reason)
        {
            baninfo *b = new baninfo;
            b->ip = ip;
            b->mask = mask;
            expire = expire;
            time = time;
            if(admin)
                strcpy(b->admin, admin);
            if(reason)
                strcpy(b->reason, reason);

            banlist *bl = getbanlist(listname);
            bl->add(b);

            // if banlist with listname not exist then add to it
            if(!banlistexists(listname)) {
                banlists.add(bl);                                
            }
        }

        bool banmanager::delban(char *listname, int id)
        {
            if(!banlistexists(listname)) return false;
            
            banlist *bl = getbanlist(listname);
            if(bl->bans.inrange(id))
            {
                bl->bans.remove(id);
                return true;
            }
            else
                return false;
        }

        bool banmanager::checkban(enet_uint32 ip)
        {
            banlist *bl;
            baninfo *b;
            time_t now = time(0);
            loopv(banlists)
            {
                bl = banlists[i];
                loopv(bl->bans)
                {
                    b = bl->bans[i];
                    if(((ip & b->mask) == (b->ip & b->mask)) && (b->expire == 0 || b->expire >= now))
                        return true;
                }
            }
            return false;
        }

        void banmanager::parseipstring(char *ipstring, enet_uint32 &destip, enet_uint32 &destmask)
        {
            union
            {
                uchar b[sizeof(enet_uint32)];
                enet_uint32 i;
            } ip, mask;
            ip.i = 0;
            mask.i = 0;
            char *next = ipstring;
            int n;

            if(*next)
                loopi(4)
                {
                    n = strtol(next, &next, 10);
                    ip.b[i] = n;
                    mask.b[i] = 0xFF;
                    if(*next && *next == '.') next++;
                    if(!*next || *next == '/') break;
                }

            // CIDR
            if(*next && *next == '/' && next++)
            {
                n = strtol(next, NULL, 10);
                mask.i = ~0;
                mask.i <<= (32-n);
                mask.i = htonl(mask.i);
            }

            destip = ip.i;
            destmask = mask.i;
        }
    }
}
