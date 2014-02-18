#ifndef __RCONMOD_H__
#define __RCONMOD_H__

#ifndef WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#ifdef WIN32
#include "windows.h"
#define socklen_t int
#endif

namespace remod
{
namespace rcon
{

extern char *rconip;
extern int rconenable;
extern char *rconpass;

void init(int port);
void sendmsg(const char *msg, int len);
void sendmsg(const char *msg);
void update();

// supported protocols
enum { RCON_UDP, RCON_TCP, NUMPROTOS };

// Peer struct
struct rconpeer
{
    int socket;
    struct sockaddr_in addr;
    bool logined;
    int connectmillis;
};

struct rconserver
{
    // interface
    virtual void sendmsg(const char *msg, int len) = 0;
    virtual void update() = 0;
};

}
}

#endif
