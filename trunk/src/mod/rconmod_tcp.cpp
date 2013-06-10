/*
* remod:    rconmod_tcp.cpp
* date:     2013
* author:   degrave
*
* remot control via telnet
*/

#include <stdio.h>

#ifndef WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#ifdef WIN32
#include "windows.h"
#define socklen_t int
#endif

#include "fpsgame.h"
#include "rconmod.h"
#include "remod.h"

#define MAXBUF 1024*60

EXTENSION(RCON);

extern int execute(const char *p);
namespace remod
{

namespace rcon
{

// wait connections
VAR(rconenable, 0, 0, 1);

// rcon password
SVAR(rconpass, "");

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
// for setsockopt() SO_REUSEADDR, below
#ifdef WIN32
char yes = 1;
#else
int yes = 1;
#endif

int addrlen;

void init(int port = 27070)
{
    if(rconenable == 0) return;

    // clear the master and temp sets
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    // get the listener
    if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        conoutf("Rcon: create socket error");
        rconenable = 0;
        return;
    }

    // "address already in use" error message
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        conoutf("Rcon: address already in use");
        rconenable = 0;
        return;
    }

    // bind
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);
    memset(&(serveraddr.sin_zero), '\0', 8);
    if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        conoutf("Rcon: socket bind error");
        rconenable = 0;
        return;
    }

    // listen
    if(listen(listener, 10) == -1)
    {
        conoutf("Rcon: socket listen error");
        rconenable = 0;
        return;
    }

    // add the listener to the master set
    FD_SET(listener, &master);
    // keep track of the biggest file descriptor
    fdmax = listener;

    conoutf("Rcon: [tcp] listen on port %d", port);
}

void update()
{
    if(rconenable == 0) return;

    read_fds = master;
    if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
    {
        conoutf("Rcon: select error");
        rconenable = 0;
        return;
    }

    // check new connections
    if(FD_ISSET(listener, &read_fds))
    {
        // handle new connections
        addrlen = sizeof(clientaddr);
        if((newfd = accept(listener, (struct sockaddr *)&clientaddr, (socklen_t *)&addrlen)) == -1)
        {
            conoutf("Rcon: socket accept error");
            rconenable = 0;
            return;
        }
        else
        {
            FD_SET(newfd, &master); // add to master set
            if(newfd > fdmax) fdmax = newfd; // keep track of the maximum
            conoutf("Rcon: new session [%s:%i]", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

            // add peer to list
            rconpeer *peer = new rconpeer();
            peer->socket = newfd;
            peer->addr = clientaddr;
            peer->logined = false;
            rconpeers.add(peer);
        }
    }

    // check
    loopv(rconpeers)
    {
        rconpeer &peer = *rconpeers[i];
        if(FD_ISSET(peer.socket, &read_fds))
        {
            // clear buffer
            memset(&buf, '\0', sizeof(buf));

            // handle data from a client
            if((nbytes = recv(peer.socket, buf, sizeof(buf), 0)) <= 0)
            {
                // got error or connection closed by client
                if(nbytes == 0)
                    // connection closed
                    //printf("%s: socket %d hung up\n", argv[0], i);
                    conoutf("Rcon: %s disconected", inet_ntoa(peer.addr.sin_addr));
                else
                {
                    conoutf("Rcon: recv error");
                    rconenable = 0;
                }

                #ifdef WIN32
                closesocket(peer.socket);
                #else
                close(peer.socket);
                #endif

                FD_CLR(peer.socket, &master);
                rconpeers.remove(i);
            }
            else
            {
                if(peer.logined)
                {
                    // execute command
                    buf[nbytes] = '\0';
                    execute(buf);
                }
                else
                {
                    // try login
                    if(strncmp(buf, rconpass, strlen(rconpass)) == 0)
                    {
                        // succefull
                        peer.logined = true;
                        conoutf("Rcon: new peer [%s:%i]", inet_ntoa(peer.addr.sin_addr), ntohs(peer.addr.sin_port));
                    }
                }
            }
        }
    }
}

// send message to peers
void sendmsg(const char *msg, int len)
{
    char *data;

    // message must ends with \n
    if (msg[len-1] != '\n')
    {
        data = newstring(msg, len);
        data[len] = '\n';
        len++;
    }
    else
    {
        data = newstring(msg);
    }

    // send text
    loopv(rconpeers)
    {
        rconpeer &peer = *rconpeers[i];
        if(peer.logined)
            if(send(peer.socket, data, len, 0) == -1);
    }

    DELETEA(data);
}

void sendmsg(const char *msg)
{
    sendmsg(msg, strlen(msg));
}

}
}
