#ifndef __IRC_REMOD_H__
#define __IRC_REMOD_H__
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

extern usermode irc_user_state(char *nick);
extern int connectwithtimeout(ENetSocket sock, const char *hostname, const ENetAddress &remoteaddress);
extern bool resolverwait(const char *name, ENetAddress *address);
void ping();

#endif // __IRC_REMOD_H__
