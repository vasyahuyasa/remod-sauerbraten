#ifndef _RCONMOD_UDP_H_
#define _RCONMOD_UDP_H_

#include "rconmod.h"

#define MAXRCONPEERS 32
#define MAXBUF 1024*60

namespace remod
{
namespace rcon
{

struct rconserver_udp : rconserver
{
    // state of rcon
    bool active;

    // list of peers
    rconpeer rconpeers[MAXRCONPEERS];

    // listen socket
    int sock;
    struct sockaddr_in addr;
    struct sockaddr_in fromaddr;
    int addrlen;
    char buf[MAXBUF];

    // private
private:
    bool havenl(char *msg);
    bool addpeer(struct sockaddr_in addr);
    void uppeer(struct sockaddr_in addr);
    void logout();
    void logout(struct sockaddr_in addr);
    bool logined(struct sockaddr_in addr, char *msg);
    void sendmsg(const char *msg);

public:
    // interface
    rconserver_udp(int port);
    void sendmsg(const char *msg, int len);
    void update();
};

}
}

#endif
