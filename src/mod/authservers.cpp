#include "authservers.h"

namespace remod
{
    namespace auth
    {
        hashnameset<authserver> servers;

        authserver *addauthserver(const char *keydomain, const char *hostname, int *port, const char *priv)
        {
            authserver &a = servers[keydomain];
            copystring(a.name, keydomain);
            copystring(a.hostname, hostname);
            a.port = *port;
            switch(priv[0])
            {
                case 'a': case 'A': a.privilege = PRIV_ADMIN; break;
                case 'm': case 'M': default: a.privilege = PRIV_AUTH; break;
                case 'n': case 'N': a.privilege = PRIV_NONE; break;
            }
            return &a;
        }
        COMMAND(addauthserver, "ssis");

        bool requestf(const char *keydomain, const char *fmt, ...)
        {
            keydomain = newstring(keydomain);
            authserver *a = servers.access(keydomain);
            if(!a) return false;
            defvformatstring(req, fmt, fmt);
            return a->request(req);
        }
    }

    using server::clients;
    using server::users;
    using server::serverauth;
    using server::setmaster;

    clientinfo *findauth(uint id, const char *desc)
    {
        loopv(server::clients) if(server::clients[i]->authreq == id && !strcmp(server::clients[i]->authdesc, desc)) return server::clients[i];
        return NULL;
    }

    void authfailed(uint id, const char *desc)
    {
        server::authfailed(findauth(id, desc));
    }

    void authsucceeded(uint id, const char *desc)
    {
        clientinfo *ci = findauth(id, desc);
        if(!ci) return;
        int authserverprivilege = PRIV_AUTH;
        if(desc[0]) {
            auth::authserver *a = auth::servers.access(desc);
            if(!a) return;
            authserverprivilege = a->privilege;
        }
        ci->cleanauth(ci->connectauth!=0);
        if(ci->connectauth) server::connected(ci);
        if(ci->authkickvictim >= 0)
        {
            if(setmaster(ci, true, "", ci->authname, ci->authdesc, authserverprivilege, false, true))
                server::trykick(ci, ci->authkickvictim, ci->authkickreason, ci->authname, ci->authdesc, authserverprivilege);
            ci->cleanauthkick();
        }
        else if(ci->privilege >= authserverprivilege)
        {
            string msg;
            if(desc && desc[0]) formatstring(msg, "%s authenticated as '\fs\f5%s\fr' [\fs\f0%s\fr]", colorname(ci), ci->authname, desc);
            else formatstring(msg, "%s authenticated as '\fs\f5%s\fr'", colorname(ci), ci->authname);
            sendf(-1, 1, "ris", N_SERVMSG, msg);
        }
        else setmaster(ci, true, "", ci->authname, ci->authdesc, authserverprivilege);
    }

    void authchallenged(uint id, const char *val, const char *desc)
    {
        clientinfo *ci = findauth(id, desc);
        if(!ci) return;
        sendf(ci->clientnum, 1, "risis", N_AUTHCHAL, desc, id, val);
    }

    uint nextauthreq = 0;

    bool tryauth(clientinfo *ci, const char *user, const char *desc)
    {
        ci->cleanauth();
        if(!nextauthreq) nextauthreq = 1;
        ci->authreq = nextauthreq++;
        filtertext(ci->authname, user, false, false, 100);
        copystring(ci->authdesc, desc);
        if(desc[0] && !strcmp(desc, serverauth))
        {
            server::userinfo *u = server::users.access(server::userkey(ci->authname, ci->authdesc));
            if(u)
            {
                uint seed[3] = { ::hthash(serverauth) + detrnd(size_t(ci) + size_t(user) + size_t(desc), 0x10000), uint(totalmillis), randomMT() };
                vector<char> buf;
                ci->authchallenge = genchallenge(u->pubkey, seed, sizeof(seed), buf);
                sendf(ci->clientnum, 1, "risis", N_AUTHCHAL, desc, ci->authreq, buf.getbuf());
            }
            else ci->cleanauth();
        }
        else if(desc[0] ? !auth::requestf(desc, "reqauth %u %s\n", ci->authreq, ci->authname) : !requestmasterf("reqauth %u %s\n", ci->authreq, ci->authname))
        {
            ci->cleanauth();
            sendf(ci->clientnum, 1, "ris", N_SERVMSG, "not connected to authentication server");
        }
        if(ci->authreq) return true;
        if(ci->connectauth) disconnect_client(ci->clientnum, ci->connectauth);
        return false;
    }

    bool answerchallenge(clientinfo *ci, uint id, char *val, const char *desc)
    {
        if(ci->authreq != id || strcmp(ci->authdesc, desc))
        {
            ci->cleanauth();
            return !ci->connectauth;
        }
        for(char *s = val; *s; s++)
        {
            if(!isxdigit(*s)) { *s = '\0'; break; }
        }
        if(desc[0] && !strcmp(desc, serverauth))
        {
            if(ci->authchallenge && checkchallenge(val, ci->authchallenge))
            {
                server::userinfo *u = users.access(server::userkey(ci->authname, ci->authdesc));
                if(u)
                {
                    if(ci->connectauth) server::connected(ci);
                    if(ci->authkickvictim >= 0)
                    {
                        if(setmaster(ci, true, "", ci->authname, ci->authdesc, u->privilege, false, true))
                            server::trykick(ci, ci->authkickvictim, ci->authkickreason, ci->authname, ci->authdesc, u->privilege);
                    }
                    else setmaster(ci, true, "", ci->authname, ci->authdesc, u->privilege);
                }
            }
            ci->cleanauth();
        }
        else if(desc[0] ? !auth::requestf(desc, "confauth %u %s\n", id, val) : !requestmasterf("confauth %u %s\n", id, val))
        {
            ci->cleanauth();
            sendf(ci->clientnum, 1, "ris", N_SERVMSG, "not connected to authentication server");
        }
        return ci->authreq || !ci->connectauth;
    }

    void authserverdisconnected(const char *keydomain)
    {
        loopvrev(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->authreq && !strcmp(ci->authdesc, keydomain)) server::authfailed(ci);
        }
    }

    void processauthserverinput(const char *desc, const char *cmd, int cmdlen, const char *args)
    {
        uint id;
        string val;
        if(sscanf(cmd, "failauth %u", &id) == 1)
            authfailed(id, desc);
        else if(sscanf(cmd, "succauth %u", &id) == 1)
            authsucceeded(id, desc);
        else if(sscanf(cmd, "chalauth %u %255s", &id, val) == 2)
            authchallenged(id, val, desc);
    }
}