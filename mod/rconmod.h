#ifndef __RCONMOD_H__
#define __RCONMOD_H__
#include <cstdio>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/fcntl.h>
#endif

#ifdef WIN32
#include "windows.h"
#define socklen_t int
#endif

#include "fpsgame.h"

namespace remod
{

namespace rcon
{

void init(int port);
void setpassword(char *pwd);
void sendmsg(char *msg);
void update();

}
}

#endif
