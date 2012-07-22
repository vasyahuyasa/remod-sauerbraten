/*
* remod:    authmod.cpp
* date:     2012
* author:   degrave
*
* local auth
*/

#include <enet/time.h>

#include "authmod.h"
#include "cube.h"
#include "remod.h"

namespace remod
{
    uint nextauthreq;

    // reimplementation
    void authchallenged(uint id, const char *val, const char *desc)
    {
        clientinfo *ci = server::findauth(id);
        if(!ci) return;
        sendf(ci->clientnum, 1, "risis", N_AUTHCHAL, desc, id, val);
    }

    void tryauth(clientinfo *ci, const char *user, const char *desc)
    {
        if(strcmp(desc, "remod") == 0)
        {
            // local auth
            if(!nextauthreq) nextauthreq = 1;
            ci->authreq = nextauthreq++;
            filtertext(ci->authname, user, false, 100);
            authserv::reqauth(ci->authreq, ci->authname, desc);
        }
        else
        {
            // normal auth
            server::tryauth(ci, user);
        }
    }

    void answerchallenge(clientinfo *ci, uint id, char *val, const char *desc)
    {
        if(strcmp(desc, "remod") == 0)
        {
            if(ci->authreq != id) return;
            for(char *s = val; *s; s++)
            {
                if(!isxdigit(*s)) { *s = '\0'; break; }
            }

            authserv::confauth(id, val);
        }
        else
        {
            server::answerchallenge(ci, id, val);
        }
    }
}

#define AUTH_TIME (60*1000)
#define AUTH_THROTTLE 1000
#define AUTH_LIMIT 100

//  emulation of auth server
namespace authserv
{
    hashtable<char *, userinfo> users;
    vector<authreq> authreqs;
    enet_uint32 lastauth;

    void purgeauths()
    {
        int expired = 0;
        loopv(authreqs)
        {
            if(ENET_TIME_DIFFERENCE(totalmillis, authreqs[i].reqtime) >= AUTH_TIME)
            {
                server::authfailed(authreqs[i].id);
                freechallenge(authreqs[i].answer);
                expired = i + 1;
            }
            else break;
        }
        if(expired > 0) authreqs.remove(0, expired);
    }

    void reqauth(uint id, char *name, const char *desc)
    {
        if(ENET_TIME_DIFFERENCE(totalmillis, lastauth) < AUTH_THROTTLE) return;

        lastauth = totalmillis;

        purgeauths();

        conoutf("Auth: attempting \"%s\" as %u", name, id);

        userinfo *u = users.access(name);
        if(!u)
        {
            server::authfailed(id);
            return;
        }

        if(authreqs.length() >= AUTH_LIMIT)
        {
            server::authfailed(authreqs[0].id);
            freechallenge(authreqs[0].answer);
            authreqs.remove(0);
        }

        authreq &a = authreqs.add();
        a.reqtime = totalmillis;
        a.id = id;
        uint seed[3] = { randomMT(), totalmillis, randomMT() };
        static vector<char> buf;
        buf.setsize(0);
        a.answer = genchallenge(u->pubkey, seed, sizeof(seed), buf);

        remod::authchallenged(id, buf.getbuf(), desc);
    }

    void confauth(uint id, const char *val)
    {
        purgeauths();

        loopv(authreqs) if(authreqs[i].id == id)
        {
            if(checkchallenge(val, authreqs[i].answer))
            {
                server::authsucceeded(id);
                conoutf("Auth: succeeded %u", id);
            }
            else
            {
                server::authfailed(id);
                conoutf("Auth: failed %u", id);
            }
            freechallenge(authreqs[i].answer);
            authreqs.remove(i--);
            return;
        }
        server::authfailed(id);
    }

    void adduser(char *name, char *pubkey)
    {
        name = newstring(name);
        userinfo &u = users[name];
        u.name = name;
        u.pubkey = parsepubkey(pubkey);
    }

    void clearusers()
    {
        enumerate(users, userinfo, u, { delete[] u.name; freepubkey(u.pubkey); });
        users.clear();
    }

    SVAR(authfile, "auth.cfg");

    void reloadauth()
    {
        clearusers();
        execfile(authfile);
    }

    /**
    * Add local authkey
    * @group server
    * @arg1 name
    * @arg2 pubkey
    */
    COMMAND(adduser, "ss");

    /**
    * Reload authkeys (using $authfile variable)
    * @group server
    */
    COMMAND(reloadauth, "");
}
