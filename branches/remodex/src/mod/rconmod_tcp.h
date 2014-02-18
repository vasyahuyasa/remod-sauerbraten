#ifndef _RCONMOD_TCP_H_
#define _RCONMOD_TCP_H_

#include "fpsgame.h"
#include "rconmod.h"

#define MAXBUF 1024*60

namespace remod
{
namespace rcon
{

struct rconserver_tcp : rconserver
{
    // list of clients
    vector<rconpeer*> rconpeers;

    fd_set master;      // master file descriptor list
    fd_set read_fds;    // temp file descriptor list for select()
    struct sockaddr_in serveraddr; // server address
    struct sockaddr_in clientaddr; // client address
    int fdmax;          // maximum file descriptor number
    int listener;       // listening socket descriptor
    int newfd;          // newly accept()ed socket descriptor
    char buf[MAXBUF];   // buffer for client data
    int nbytes;
    struct timeval tv;  // timeout for select

// for setsockopt() SO_REUSEADDR, below
#ifdef WIN32
    char yes;
#else
    int yes;
#endif

    int addrlen;

    // interface
    rconserver_tcp(int port);
    void sendmsg(const char *msg, int len);
    void update();
};

}
}
#endif // _RCONMOD_TCP_H_
