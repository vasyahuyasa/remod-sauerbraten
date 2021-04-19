#include "fpsgame.h"

extern ENetAddress serveraddress;

// stuff defined in vanilla code that we need in authservers.cpp, but may also be needed in fpsgame/server.cpp
namespace server
{
    extern char *serverauth;
    struct userkey
    {
        char *name;
        char *desc;

        userkey() : name(NULL), desc(NULL) {}
        userkey(char *name, char *desc) : name(name), desc(desc) {}
    };

    static inline uint hthash(const userkey &k) { return ::hthash(k.name); }
    static inline bool htcmp(const userkey &x, const userkey &y) { return !strcmp(x.name, y.name) && !strcmp(x.desc, y.desc); }

    struct userinfo : userkey
    {
        void *pubkey;
        int privilege;

        userinfo() : pubkey(NULL), privilege(PRIV_NONE) {}
        ~userinfo() { delete[] name; delete[] desc; if(pubkey) freepubkey(pubkey); }
    };
    extern hashset<userinfo> users;
    void authfailed(clientinfo *ci);
    bool trykick(clientinfo *ci, int victim, const char *reason = NULL, const char *authname = NULL, const char *authdesc = NULL, int authpriv = PRIV_NONE, bool trial = false);
    bool setmaster(clientinfo *ci, bool val, const char *pass = "", const char *authname = NULL, const char *authdesc = NULL, int authpriv = PRIV_MASTER, bool force = false, bool trial = false);
    void connected(clientinfo *ci);
}

namespace remod
{
    extern void processauthserverinput(const char *desc, const char *cmd, int cmdlen, const char *args);

    namespace auth
    {
        static inline bool resolverwait(const char *name, ENetAddress *address)
        {
            return enet_address_set_host(address, name) >= 0;
        }

        struct authserver
        {
            string name; // = key domain
            string hostname;
            int port, privilege;
            ENetAddress address;
            ENetSocket sock;
            int connecting, connected, lastconnect, lastupdate;
            vector<char> in, out;
            int inpos, outpos;

            authserver() : port(0), privilege(PRIV_NONE),
        #ifdef __clang__
                        address((ENetAddress){ENET_HOST_ANY, ENET_PORT_ANY}),
        #else
                        address({ENET_HOST_ANY, ENET_PORT_ANY}),
        #endif
                        sock(ENET_SOCKET_NULL), connecting(0), connected(0), lastconnect(0), lastupdate(0), inpos(0), outpos(0)
            {
                name[0] = hostname[0] = 0;
            }

            void disconnect()
            {
                if(sock != ENET_SOCKET_NULL)
                {
                    enet_socket_destroy(sock);
                    sock = ENET_SOCKET_NULL;
                }

                out.setsize(0);
                in.setsize(0);
                outpos = inpos = 0;

                address.host = ENET_HOST_ANY;
                address.port = ENET_PORT_ANY;

                lastupdate = connecting = connected = 0;
            }

            void connect()
            {
                if(sock!=ENET_SOCKET_NULL) return;
                if(!hostname[0]) return;
                if(address.host == ENET_HOST_ANY)
                {
                    if(isdedicatedserver()) logoutf("looking up %s...", hostname);
                    address.port = port;
                    if(!resolverwait(hostname, &address)) return;
                }
                sock = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
                if(sock == ENET_SOCKET_NULL)
                {
                    if(isdedicatedserver()) logoutf("could not open socket for auth server %s", name);
                    return;
                }
                if(serveraddress.host == ENET_HOST_ANY || !enet_socket_bind(sock, &serveraddress))
                {
                    enet_socket_set_option(sock, ENET_SOCKOPT_NONBLOCK, 1);
                    if(!enet_socket_connect(sock, &address)) return;
                }
                enet_socket_destroy(sock);
                if(isdedicatedserver()) logoutf("could not connect to auth server %s", name);
                sock = ENET_SOCKET_NULL;
                return;
            }

            bool request(const char *req)
            {
                if(sock == ENET_SOCKET_NULL)
                {
                    connect();
                    if(sock == ENET_SOCKET_NULL) return false;
                    lastconnect = connecting = totalmillis ? totalmillis : 1;
                }

                if(out.length() >= 4096) return false;

                out.put(req, strlen(req));
                return true;
            }

            bool requestf(const char *fmt, ...)
            {
                defvformatstring(req, fmt, fmt);
                return request(req);
            }

            void processinput()
            {
                if(inpos >= in.length()) return;

                char *input = &in[inpos], *end = (char *)memchr(input, '\n', in.length() - inpos);
                while(end)
                {
                    *end = '\0';

                    const char *args = input;
                    while(args < end && !iscubespace(*args)) args++;
                    int cmdlen = args - input;
                    while(args < end && iscubespace(*args)) args++;

                    processauthserverinput(name, input, cmdlen, args);

                    end++;
                    inpos = end - in.getbuf();
                    input = end;
                    end = (char *)memchr(input, '\n', in.length() - inpos);
                }

                if(inpos >= in.length())
                {
                    in.setsize(0);
                    inpos = 0;
                }
            }

            void flushoutput()
            {
                if(connecting && totalmillis - connecting >= 60000)
                {
                    logoutf("could not connect to auth server %s", name);
                    disconnect();
                }
                if(out.empty() || !connected) return;

                ENetBuffer buf;
                buf.data = &out[outpos];
                buf.dataLength = out.length() - outpos;
                int sent = enet_socket_send(sock, NULL, &buf, 1);
                if(sent >= 0)
                {
                    outpos += sent;
                    if(outpos >= out.length())
                    {
                        out.setsize(0);
                        outpos = 0;
                    }
                }
                else disconnect();
            }

            void flushinput()
            {
                if(in.length() >= in.capacity())
                    in.reserve(4096);

                ENetBuffer buf;
                buf.data = in.getbuf() + in.length();
                buf.dataLength = in.capacity() - in.length();
                int recv = enet_socket_receive(sock, NULL, &buf, 1);
                if(recv > 0)
                {
                    in.advance(recv);
                    processinput();
                }
                else disconnect();
            }
        };

        extern hashnameset<authserver> servers;
    }

    using server::clientinfo;

    void authfailed(uint id, const char *desc);
    void authsucceeded(uint id, const char *desc);
    void authchallenged(uint id, const char *val, const char *desc);
    bool tryauth(clientinfo *ci, const char *user, const char *desc);
    bool answerchallenge(clientinfo *ci, uint id, char *val, const char *desc);
}
