#ifndef __IRC_H__
#define __IRC_H__

#include "game.h"

enum { IRCC_NONE = 0, IRCC_JOINING, IRCC_JOINED, IRCC_KICKED, IRCC_BANNED };
enum { IRCCT_NONE = 0, IRCCT_AUTO };

//remod
enum usermode
{
    OP      = 1 << 1,
    HALFOP  = 1 << 2,
    VOICE   = 1 << 3,
    ADMIN   = 1 << 4,
    OWNER   = 1 << 5,
    NONE    = 1 << 6,
    ERR     = 0
};

struct user
{
    string nick;
    usermode state;
};

struct ircchan
{
    int state, type, relay, lastjoin, lastsync;
    string name, friendly, passkey;

    // remod
    vector<user> users;

#ifndef STANDALONE
    int updated;
    ircbuf buffer;
#endif
    ircchan() { reset(); }
    ~ircchan() { reset(); }

    void reset()
    {
        state = IRCC_NONE;
        type = IRCCT_NONE;
        relay = lastjoin = lastsync = 0;
        name[0] = friendly[0] = passkey[0] = 0;
#ifndef STANDALONE
        updated = 0;
        buffer.reset();
#endif
    }

    // remod
    void adduser(char *nick, usermode state)
    {
        if(!nick) return;
        user &u = users.add();
        strcpy(u.nick, nick);
        u.state=state;
    }

    void deluser(char *nick)
    {
        if(!nick) return;
        loopv(users)
            if(strcmp(users[i].nick, nick)==0)
            {
                users.remove(i);
                return;
            }
    }

    void setusermode(char *nick, usermode state)
    {
        if(!nick) return;
        loopv(users)
            if(strcmp(users[i].nick, nick)==0) users[i].state = state;
    }

    void resetusers()
    {
        //loopv(users) delete(&users[i]); // posible crash
        users.shrink(0);
    }

    bool rename(char *nick, char *newnick)
    {
        if(!nick || !newnick) return false;
        loopv(users)
            if(strcmp(users[i].nick, nick)==0)
            {
                strncpy(users[i].nick, newnick, 100);
                users[i].nick[100] = '\0';
                return true;
            }

        return false;
    }
};
enum { IRCT_NONE = 0, IRCT_CLIENT, IRCT_RELAY, IRCT_MAX };
enum { IRC_NEW = 0, IRC_DISC, IRC_WAIT, IRC_ATTEMPT, IRC_CONN, IRC_ONLINE, IRC_QUIT, IRC_MAX };

struct ircnet
{
    int type, state, port, lastattempt, lastactivity, lastping, inputcarry, inputlen;
    string name, serv, nick, ip, passkey, authname, authpass;
    ENetAddress address;
    ENetSocket sock;
    vector<ircchan> channels;
    uchar input[4096];

    // remod
    time_t lastpong;
    char *authcmd;

#ifndef STANDALONE
    int updated, lastseen, away;
    ircbuf buffer;
#endif

    ircnet() { reset(true); }
    ~ircnet() { reset(true); }

    void reset(bool start = false)
    {
        if(start)
        {
            type = IRCT_NONE;
            state = IRC_NEW;
            sock = ENET_SOCKET_NULL;
            address.host = ENET_HOST_ANY;
            address.port = 6667;
        }
        else state = IRC_DISC;
        inputcarry = inputlen = 0;
        port = lastattempt = lastactivity = lastping = 0;
        name[0] = serv[0] = nick[0] = ip[0] = passkey[0] = authname[0] = authpass[0] = 0;
        channels.shrink(0);

        // remod
        lastping = 0;
        lastpong = 0;
        authcmd = NULL;

#ifndef STANDALONE
        updated = IRCUP_NEW;
        lastseen = totalmillis;
        away = 0;
        buffer.reset();
#endif
    }
};

extern vector<ircnet *> ircnets;

extern ircnet *ircfind(const char *name);
extern void ircestablish(ircnet *n);
extern void ircsend(ircnet *n, const char *msg, ...);
extern void ircoutf(int relay, const char *msg, ...);
extern int ircrecv(ircnet *n);
extern void ircnewnet(int type, const char *name, const char *serv, int port, const char *nick, const char *ip = "", const char *passkey = "");
extern ircchan *ircfindchan(ircnet *n, const char *name);
extern bool ircjoin(ircnet *n, ircchan *c);
extern bool ircenterchan(ircnet *n, const char *name);
extern bool ircnewchan(int type, const char *name, const char *channel, const char *friendly = "", const char *passkey = "", int relay = 0);
extern void ircparse(ircnet *n);
extern void irccleanup();
extern bool ircaddsockets(ENetSocket &maxsock, ENetSocketSet &readset, ENetSocketSet &writeset);
extern void ircchecksockets(ENetSocketSet &readset, ENetSocketSet &writeset);
extern void ircslice();

//remod
extern usermode irc_user_state(char *nick);
extern bool resolverwait(const char *name, ENetAddress *address);
void ping();
void irc_checkserversockets();
#endif // __IRC_H__
