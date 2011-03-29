#include "fpsgame.h"
#include "commandev.h"

//Remod import
#define mkstring(d) string d; d[0] = 0;

#define ENG_VERSION         90
#define ENG_NAME            "Red Eclipse"
#define ENG_RELEASE         "Beta Devel"
#define ENG_URL             "www.redeclipse.net"
#ifdef WIN32
#define ENG_PLATFORM        "win"
#elif defined(__APPLE__)
#define ENG_PLATFORM        "mac"
#else
#define ENG_PLATFORM        "nix"
#endif
#define ENG_DEVEL           true

enum { CON_MESG, CON_SELF, CON_GAMESPECIFIC };
enum { CON_EVENT, CON_MAX, CON_LO, CON_HI, CON_IMPORTANT };

enum { IRCC_NONE = 0, IRCC_JOINING, IRCC_JOINED, IRCC_KICKED, IRCC_BANNED };
enum { IRCCT_NONE = 0, IRCCT_AUTO };

// remod
enum usermode {
        OP = 1 << 1,
        HALFOP = 1 << 2,
        VOICE = 1 << 3,
        ADMIN = 1 << 4,
        OWNER = 1 << 5,
        NONE = 1 << 6
    };

struct user
{
    string nick;
    usermode state;
};

struct ircchan
{
    int state, type, relay, lastjoin;
    string name, friendly, passkey;

    vector<user> users;

#ifndef STANDALONE
    vector<char *> lines;
#endif

    ircchan() { reset(); }
    ~ircchan() { reset(); }

    void reset()
    {
        state = IRCC_NONE;
        type = IRCCT_NONE;
        relay = lastjoin = 0;
        name[0] = friendly[0] = passkey[0] = 0;
#ifndef STANDALONE
        loopv(lines) DELETEA(lines[i]);
        lines.shrink(0);
#endif
        resetusers();
    }

    void adduser(char *nick, usermode state)
    {
        user &u = users.add();
        strcpy(u.nick, nick);
        u.state=state;
    }

    void deluser(char *nick)
    {
        loopv(users)
        {
            if(strcmp(users[i].nick, nick)==0)
            {
                users.remove(i);
                return;
            }
        }
    }

    void setusermode(char *nick, usermode state)
    {
        loopv(users)
        {
            if(strcmp(users[i].nick, nick)==0) users[i].state = state;
        }
    }

    void resetusers()
    {
        //loopv(users) delete(&users[i]); // posible crash
        users.shrink(0);
    }
};
enum { IRCT_NONE = 0, IRCT_CLIENT, IRCT_RELAY, IRCT_MAX };
enum { IRC_DISC = 0, IRC_ATTEMPT, IRC_CONN, IRC_ONLINE, IRC_MAX };
struct ircnet
{
    int type, state, port, lastattempt;
    string name, serv, nick, ip, passkey, authname, authpass;
    ENetAddress address;
    ENetSocket sock;
    vector<ircchan> channels;
    vector<char *> lines;
    uchar input[4096];

    ircnet() { reset(); }
    ~ircnet() { reset(); }

    void reset()
    {
        type = IRCT_NONE;
        state = IRC_DISC;
        port = lastattempt = 0;
        name[0] = serv[0] = nick[0] = ip[0] = passkey[0] = authname[0] = authpass[0] = 0;
        channels.shrink(0);
        loopv(lines) DELETEA(lines[i]);
        lines.shrink(0);
    }
};

//Remod imported
extern int connectwithtimeout(ENetSocket sock, const char *hostname, const ENetAddress &remoteaddress);
//extern int connectwithtimeout(ENetSocket sock, const char *hostname, ENetAddress &remoteaddress);
extern bool resolverwait(const char *name, ENetAddress *address);

extern vector<ircnet *> ircnets;

extern bool irc_user_state(char *nick, usermode state);

extern ircnet *ircfind(const char *name);
extern void ircestablish(ircnet *n);
extern void ircsend(ircnet *n, const char *msg, ...);
extern void ircoutf(int relay, const char *msg, ...);
extern int ircrecv(ircnet *n, int timeout = 0);
extern char *ircread(ircnet *n);
extern void ircnewnet(int type, const char *name, const char *serv, int port, const char *nick, const char *ip = "", const char *passkey = "");
extern ircchan *ircfindchan(ircnet *n, const char *name);
extern bool ircjoin(ircnet *n, ircchan *c);
extern bool ircenterchan(ircnet *n, const char *name);
extern bool ircnewchan(int type, const char *name, const char *channel, const char *friendly = "", const char *passkey = "", int relay = 0);
extern void ircparse(ircnet *n, char *reply);
extern void ircdiscon(ircnet *n);
extern void irccleanup();
extern void ircslice();
