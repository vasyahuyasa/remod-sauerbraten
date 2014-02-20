//void init(int port);
//void sendmsg(const char *msg, int len);
//void sendmsg(const char *msg);
//void update();

#include "rconmod.h"
#include "rconmod_tcp.h"
#include "rconmod_udp.h"
#include "remod.h"

EXTENSION(RCON);

namespace remod
{
namespace rcon
{
// listen address
SVAR(rconip, "");

// protocol 0-udp/1-tcp
VAR(rconproto, RCON_UDP, 0, NUMPROTOS-1);

// wait connections
VAR(rconenable, 0, 0, 1);

// rcon password
SVAR(rconpass, "");

rconserver *srv;

void init(int port)
{
    if(!rconenable) return;

    switch(rconproto)
    {
        case RCON_TCP:
            srv = new rconserver_tcp(port);
            break;

        case RCON_UDP:
        default:
            srv = new rconserver_udp(port);
    }
}

void sendmsg(const char *msg, int len)
{
    if(srv)
        srv->sendmsg(msg, len);
}

void sendmsg(const char *msg)
{
    if(srv)
        srv->sendmsg(msg, strlen(msg));
}

void update()
{
    if(rconenable && srv)
        srv->update();
}

}
}
