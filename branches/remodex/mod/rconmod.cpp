/*
* remod:    rconmod.cpp
* date:     2007
* author:   degrave
*
* remot control
*/



#include "rconmod.h"

extern int execute(const char *p);
namespace remod
{

namespace rcon
{

#define MAXRCONPEERS 64

//Peer struct
struct rconpeer
{
    struct sockaddr_in addr;
    bool logined;
};

//State of rcon
bool active;

//List of peers
rconpeer rconpeers[MAXRCONPEERS];

//Listen socket information
int sock;
struct sockaddr_in addr;
struct sockaddr_in fromaddr;
int addrlen;
int recvlen;
char buf[1024*60]; // Max recive

//password for use rcon
string password;

string s;

//Check for \n in end of line
bool havenl(char *msg)
{
    for(int c = *msg; c; c = *++msg)
    {
        if(c=='\n') return true;
    }
    return false;
}

//Add peer to list of logined peers
bool addpeer(struct sockaddr_in addr)
{
    char *ipstr;
    string msg;
    for(int i=0; i<MAXRCONPEERS; i++)
    {
        if(rconpeers[i].logined==false)
        {
            rconpeers[i].addr=addr;
            rconpeers[i].logined=true;
            ipstr = inet_ntoa(addr.sin_addr);
            formatstring(msg)("Rcon: new peer [%s]\n", ipstr);
            sendmsg(msg);
            return true;
        }
    }
    return false;
}

//Update peer info when recive any data
void uppeer(struct sockaddr_in addr)
{
    //List all peers and update sockaddr information
    for(int i=0; i<MAXRCONPEERS; i++)
    {
        if(rconpeers[i].addr.sin_addr.s_addr==addr.sin_addr.s_addr && rconpeers[i].logined)
        {
            rconpeers[i].addr = addr;
        }
    }
}

//Log out all peers
void logout()
{
    sendmsg("Logout");
    for(int i=0; i<MAXRCONPEERS; i++)
    {
        rconpeers[i].logined = false;
    }
}

//Check if peer logined or login him with msg password
bool logined(struct sockaddr_in addr, char *msg)
{
    //Check all connected peers by ip and login
    for(int i=0; i<MAXRCONPEERS; i++)
    {
        if(rconpeers[i].addr.sin_addr.s_addr==addr.sin_addr.s_addr && rconpeers[i].logined)
        {
            //Update peer information and return
            uppeer(addr);
            return true;
        }
    }

    //If we have correct password login peer, but he be logined only on next call this function
    if(strncmp(password, msg, strlen(password))==0)
    {
        //if we don't have more slots logout all peers
        if(!addpeer(addr))
        {
            logout();
            addpeer(addr);
        }
        return false;
    }
    else
    {
        //Peer not logined
        return false;
    }
}

//Init rcon module
void init(int port=27070)
{
    sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if(sock<0)
    {
        active=false;
        conoutf("Rcon: cannot create socket\n");
    }
    else
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            conoutf("Rcon: connot bind socket\n");
            active=false;
        }
        else
        {
            addrlen = sizeof(fromaddr);
#ifdef WIN32
            u_long sockflag=1;
            ioctlsocket(sock, FIONBIO, &sockflag);
#else
            fcntl(sock, F_SETFL, O_NONBLOCK);
#endif
            active=true;
            for(int i=0; i<MAXRCONPEERS; i++)
            {
                rconpeers[i].logined=false;
            }
            conoutf("Rcon: inizialized on port %d\n",port);
        }
    }
}

// Set rcon password
void setpassword(char *pwd)
{
    strcpy(password, pwd);
    conoutf("Rcon pass: %s\n", password);
}

//Send message to all logined peers
void sendmsg(char *msg)
{
    //always send message with \n
    if(!havenl(msg))
    {
        formatstring(s)("%s\n", msg);
    }
    else formatstring(s)("%s", msg);

    //Send raw text
    for(int i=0; i<MAXRCONPEERS; i++)
    {
        if(rconpeers[i].logined)
        {
            sendto(sock, s, strlen(s), 0, (struct sockaddr *)&rconpeers[i].addr, addrlen);
        }
    }
}

//update on every server frame
void update()
{
    if(active)
    {
        recvlen=recvfrom(sock, buf, 1024, 0, (struct sockaddr*)&fromaddr, (socklen_t*)&addrlen);
        if(recvlen>0)
        {
            if(logined(fromaddr, buf))
            {
                buf[recvlen] = '\0';
                execute(buf);
            }
        }
    }
}

COMMANDN(rconpass, setpassword, "s");

}
}
