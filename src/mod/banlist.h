/*
 * jsonbanlist.h
 *
 *  Created on: Jan 2, 2014
 *  Author: degrave
 */

#include "remod.h"

namespace remod
{
    namespace banlist
    {
        struct baninfo
        {
            union
            {
                enet_uint32 ip;
                uchar ipoctet[sizeof(enet_uint32)];
            };

            union
            {
                enet_uint32 mask;
                uchar maskoctet[sizeof(enet_uint32)];
            };
            time_t expire;
            time_t time;
            string admin;
            string reason;

            baninfo();
        };

        struct banlist
        {
            char *name;
            vector<baninfo*> bans;

            banlist();
            banlist(char *listname);

            void add(baninfo *ban);
            baninfo* remove(int n);
            size_t length();
        };

        class banmanager
        {
            vector<banlist*> banlists;

            public:
            banmanager();
            banlist* getbanlist(char *name);
            banlist* localbanlist();
            baninfo* getban(char *listname, size_t n);
            bool banlistexists(char *name);
            void addban(char *listname, enet_uint32 ip, enet_uint32 mask, time_t expire, time_t time, const char *admin, const char *reason);
            bool delban(char *listname, int id);
            bool checkban(enet_uint32 ip);
            void parseipstring(char *ipstring, enet_uint32 &destip, enet_uint32 &destmask);
        };


    }
}
