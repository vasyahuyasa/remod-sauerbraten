#ifndef __RCONMOD_H__
#define __RCONMOD_H__

namespace remod
{

namespace rcon
{

//Peer struct
struct rconpeer
{
    struct sockaddr_in addr;
    bool logined;
    int connectmillis;
};

void init(int port);
void sendmsg(char *msg, int len);
void sendmsg(char *msg);
void update();

}
}

#endif
