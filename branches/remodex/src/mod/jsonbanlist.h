/*
 * jsonbanlist.h
 *
 *  Created on: Jan 1, 2013
 *  Author: degrave
 */

// example of usage
// jsonbanlist hopmod 0 60 http://sauer.nomorecheating.org/hopmod/view_bans.php http://83.169.44.106/hopmod/gbans.php
// jsonbanlist ourbans 1 10 http://ourbans.butchers.su/bans.php?format=json

// curl -A "" http://sauer.nomorecheating.org/hopmod/gbans.php
// "id":24, "address":"190.43/16", "admin":"ADMIN", "expire":-1, "reason":"unknown", "time":1321377751, "expired":0
// id			- database ban id
// address	- bannedip range
// admin		- who added ban
// expire		- time when ban expire, -1 perm ban
// reson		- ban reason
// time		- when added
// expired		- field can be ignored

#ifndef __JSONBANLIST_H__
#define __JSONBANLIST_H__

#include "remod.h"
#include "banlist.h"

namespace remod
{
    namespace jsonbanlist
    {
        enum liststatus { ST_IDLE };

        class jsonsource
        {
            char *url;
            ENetAddress address;
            bool parseurl();

            public:
            jsonsource():url(NULL){ address.host = ENET_HOST_ANY; };
            jsonsource(char *url);
            bool seturl(char *addr);
            ENetAddress getaddress();
        };

        class jsonbanlist
        {
            ENetSocket sock;
            uchar input[4096];
            char *name;
            vector<jsonsource*> sources;
            bool timedbans;
            uint updateinterval;
            time_t lastupdate;
            liststatus status;
            remod::banlist::banlist *bl;

            public:
            jsonbanlist();
            jsonbanlist(const char *listname, bool timed, uint interval);
            void update(time_t now);
        };

        void update();
    }
}

#endif
