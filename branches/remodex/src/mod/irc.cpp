#ifdef IRC

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include "irc.h"
#include "remod.h"

// remod
EXTENSION(IRC);
VAR(verbose, 0, 0, 6);
SVAR(consoletimefmt, "%c");
SVAR(irccommandchar, "");
time_t clocktime = 0;

// remod
VAR(ircpingdelay, 0, 60, 600); // delay in seconds between network ping

char *gettime(char *format)
{
    struct tm *t;
    static string buf;

    t = localtime (&clocktime);
    strftime (buf, sizeof (buf) - 1, format, t);
    return buf;
}

void console(int type, const char *s, ...)
{
    defvformatstring(sf, s, s);
    string osf, psf, fmt;
    formatstring(fmt)(consoletimefmt);
    filtertext(osf, sf);
    formatstring(psf)("%s %s", gettime(fmt), osf);
    printf("%s\n", osf);
    //fflush(stdout);
}

vector<ircnet *> ircnets;

ircnet *ircfind(const char *name)
{
    if(name && *name)
    {
        loopv(ircnets) if(!strcmp(ircnets[i]->name, name)) return ircnets[i];
    }
    return NULL;
}

void ircprintf(ircnet *n, int relay, const char *target, const char *msg, ...)
{
    defvformatstring(str, msg, msg);
    string s;
    if(target && *target && strcasecmp(target, n->nick))
    {
        ircchan *c = ircfindchan(n, target);
        if(c)
        {
            // remod
            formatstring(s)("\fs\fa[%s:%s]\fS", n->name, c->name);

            /*
            formatstring(s)("\fs\fa[%s:%s]\fS", n->name, c->name);
            if(n->type == IRCT_RELAY && c->relay >= relay)
                server::srvmsgf(relay > 1 ? -2 : -3, "\fs\fa[%s]\fS %s", c->friendly, str);
            */
        }
        else
        {
            formatstring(s)("\fs\fa[%s:%s]\fS", n->name, target);
        }
    }
    else
    {
        formatstring(s)("\fs\fa[%s]\fS", n->name);
    }
    console(0, "%s %s", s, str); // console is not used to relay
}

void ircestablish(ircnet *n)
{
    if(!n) return;
    n->lastattempt = clocktime;
    if(n->address.host == ENET_HOST_ANY)
    {
        // remod
        //ircprintf(n, 4, NULL, "looking up %s:[%d]...", n->serv, n->port);
        conoutf("Irc: looking up %s:[%d]...", n->serv, n->port);

        if(!resolverwait(n->serv, &n->address))
        {
            // remod
            //ircprintf(n, 4, NULL, "unable to resolve %s:[%d]...", n->serv, n->port);
            conoutf("Irc: unable to resolve %s:[%d]...", n->serv, n->port);

            n->state = IRC_DISC;
            return;
        }
    }

    ENetAddress address = { ENET_HOST_ANY, enet_uint16(n->port) };
    if(*n->ip && enet_address_set_host(&address, n->ip) < 0) ircprintf(n, 4, NULL, "failed to bind address: %s", n->ip);
    n->sock = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
    if(n->sock != ENET_SOCKET_NULL && *n->ip && enet_socket_bind(n->sock, &address) < 0)
    {
        // remod
        //ircprintf(n, 4, NULL, "failed to bind connection socket: %s", n->ip);
        conoutf("Irc: failed to bind connection socket: %s", n->ip);

        address.host = ENET_HOST_ANY;
    }

    // Remod hack, sometimes irc bot block client connections
    // set socket timeout to 5 seconds
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if(setsockopt(n->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) conoutf("Irc: setsockopt SO_RCVTIMEO failed");
    if(setsockopt(n->sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) conoutf("Irc: setsockopt SO_SNDTIMEO failed");

    if(n->sock == ENET_SOCKET_NULL || connectwithtimeout(n->sock, n->serv, n->address) < 0)
    {
        // remod
        //ircprintf(n, 4, NULL, n->sock == ENET_SOCKET_NULL ? "could not open socket to %s:[%d]" : "could not connect to %s:[%d]", n->serv, n->port);
        conoutf(n->sock == ENET_SOCKET_NULL ? "Irc: could not open socket to %s:[%d]" : "could not connect to %s:[%d]", n->serv, n->port);

        if(n->sock != ENET_SOCKET_NULL)
        {
            enet_socket_destroy(n->sock);
            n->sock = ENET_SOCKET_NULL;
        }
        n->state = IRC_DISC;
        return;
    }
    n->state = IRC_ATTEMPT;

    // remod
    //ircprintf(n, 4, NULL, "connecting to %s:[%d]...", n->serv, n->port);
    conoutf("Irc: connecting to %s:[%d]...", n->serv, n->port);
}

void ircsend(ircnet *n, const char *msg, ...)
{
    if(!n) return;
    defvformatstring(str, msg, msg);
    if(n->sock == ENET_SOCKET_NULL) return;

    // remod
    if(verbose >= 2) conoutf(0, "Irc: [%s] >>> %s", n->name, str);

    concatstring(str, "\n");
    ENetBuffer buf;
    uchar ubuf[512];
    int len = strlen(str), carry = 0;
    while(carry < len)
    {
        int numu = encodeutf8(ubuf, sizeof(ubuf)-1, &((uchar *)str)[carry], len - carry, &carry);
        if(carry >= len) ubuf[numu++] = '\n';
        loopi(numu) switch(ubuf[i])
        {
            case '\v': ubuf[i] = '\x01'; break;
            case '\f': ubuf[i] = '\x03'; break;
        }
        buf.data = ubuf;
        buf.dataLength = numu;
        enet_socket_send(n->sock, NULL, &buf, 1);
    }
}

VAR(ircfilter, 0, 1, 2);

void converttext(char *dst, const char *src)
{
    int colorpos = 0; char colorstack[10];
    memset(colorstack, 'u', sizeof(colorstack)); //indicate user color
    for(int c = *src; c; c = *++src)
    {
        if(c == '\f')
        {
            c = *++src;
            if(c == 'z')
            {
                c = *++src;
                if(c) ++src;
            }
            else if(c == '[' || c == '(')
            {
                const char *end = strchr(src, c == '[' ? ']' : ')');
                src += end ? end-src : strlen(src);
            }
            else if(c == 's') { if(colorpos < (int)sizeof(colorstack)-1) colorpos++; continue; }
            else if(c == 'S') { if(colorpos > 0) --colorpos; c = colorstack[colorpos]; }
            int oldcolor = colorstack[colorpos]; colorstack[colorpos] = c;
            switch(c)
            {
                case 'g': case '0': case 'G': *dst++ = '\f'; *dst++ = '0'; *dst++ = '3'; break; // green
                case 'b': case '1': case 'B': *dst++ = '\f'; *dst++ = '1'; *dst++ = '2'; break; // blue
                case 'y': case '2': case 'Y': *dst++ = '\f'; *dst++ = '0'; *dst++ = '3'; break; // yellow
                case 'r': case '3': case 'R': *dst++ = '\f'; *dst++ = '0'; *dst++ = '4'; break; // red
                case 'a': case '4': *dst++ = '\f'; *dst++ = '1'; *dst++ = '4'; break; // grey
                case 'm': case '5': case 'M': *dst++ = '\f'; *dst++ = '1'; *dst++ = '3'; break; // magenta
                case 'o': case '6': case 'O': *dst++ = '\f'; *dst++ = '0'; *dst++ = '7'; break; // orange
                case 'c': case '9': case 'C': *dst++ = '\f'; *dst++ = '1'; *dst++ = '0'; break; // cyan
                case 'v': *dst++ = '\f'; *dst++ = '0'; *dst++ = '6'; break; // violet
                case 'p': *dst++ = '\f'; *dst++ = '0'; *dst++ = '6'; break; // purple
                case 'n': *dst++ = '\f'; *dst++ = '0'; *dst++ = '5'; break; // brown
                case 'd': case 'A': *dst++ = '\f'; *dst++ = '0'; *dst++ = '1'; break; // dark grey
                case 'u': case 'w': case '7': case 'k': case '8': *dst++ = '\f'; *dst++ = '0'; *dst++ = '0'; break;
                default: colorstack[colorpos] = oldcolor; break;
            }
            continue;
        }
        if(iscubeprint(c) || iscubespace(c)) *dst++ = c;
    }
    *dst = '\0';
}

void ircoutf(int relay, const char *msg, ...)
{
    defvformatstring(src, msg, msg);
    mkstring(str);
    switch(ircfilter)
    {
        case 2: filtertext(str, src); break;
        case 1: converttext(str, src); break;
        case 0: default: copystring(str, src); break;
    }
    loopv(ircnets) if(ircnets[i]->sock != ENET_SOCKET_NULL && ircnets[i]->type == IRCT_RELAY && ircnets[i]->state == IRC_ONLINE)
    {
        ircnet *n = ircnets[i];
#if 0 // workaround for freenode's crappy dropping all but the first target of multi-target messages even though they don't state MAXTARGETS=1 in 005 string..
        mkstring(s);
        loopvj(n->channels) if(n->channels[j].state == IRCC_JOINED && n->channels[j].relay >= relay)
        {
            ircchan *c = &n->channels[j];
            if(s[0]) concatstring(s, ",");
            concatstring(s, c->name);
        }
        if(s[0]) ircsend(n, "PRIVMSG %s :%s", s, str);
#else
        loopvj(n->channels) if(n->channels[j].state == IRCC_JOINED && n->channels[j].relay >= relay)
            ircsend(n, "PRIVMSG %s :%s", n->channels[j].name, str);
#endif
    }
}

int ircrecv(ircnet *n)
{
    if(!n) return -1;
    if(n->sock == ENET_SOCKET_NULL) return -2;
    int total = 0;
    enet_uint32 events = ENET_SOCKET_WAIT_RECEIVE;
    while(enet_socket_wait(n->sock, &events, 0) >= 0 && events)
    {
        ENetBuffer buf;
        buf.data = n->input + n->inputlen;
        buf.dataLength = sizeof(n->input) - n->inputlen;
        int len = enet_socket_receive(n->sock, NULL, &buf, 1);
        if(len <= 0) return -3;
        loopi(len) switch(n->input[n->inputlen+i])
        {
            case '\x01': n->input[n->inputlen+i] = '\v'; break;
            case '\x03': n->input[n->inputlen+i] = '\f'; break;
            case '\v': case '\f': n->input[n->inputlen+i] = ' '; break;
        }
        n->inputlen += len;
        int carry = 0, decoded = decodeutf8(&n->input[n->inputcarry], n->inputlen - n->inputcarry, &n->input[n->inputcarry], n->inputlen - n->inputcarry, &carry);
        if(carry > decoded)
        {
            memmove(&n->input[n->inputcarry + decoded], &n->input[n->inputcarry + carry], n->inputlen - (n->inputcarry + carry));
            n->inputlen -= carry - decoded;
        }
        n->inputcarry += decoded;
        total += decoded;
    }
    return total;
}

void ircnewnet(int type, const char *name, const char *serv, int port, const char *nick, const char *ip, const char *passkey)
{
    if(!name || !*name || !serv || !*serv || !port || !nick || !*nick) return;
    ircnet *m = ircfind(name);
    if(m)
    {
        if(m->state != IRC_DISC) conoutf("ircnet %s already exists", m->name);
        else ircestablish(m);
        return;
    }
    ircnet &n = *ircnets.add(new ircnet);
    n.type = type;
    n.state = IRC_NEW;
    n.sock = ENET_SOCKET_NULL;
    n.port = port;
    n.lastattempt = 0;
    copystring(n.name, name);
    copystring(n.serv, serv);
    copystring(n.nick, nick);
    copystring(n.ip, ip);
    copystring(n.passkey, passkey);
    n.address.host = ENET_HOST_ANY;
    n.address.port = n.port;
    n.input[0] = n.authname[0] = n.authpass[0] = 0;

    // remod
    //ircprintf(&n, 4, NULL, "added %s %s (%s:%d) [%s]", type == IRCT_RELAY ? "relay" : "client", name, serv, port, nick);
    conoutf("Irc: added irc %s %s (%s:%d) [%s]", type == IRCT_RELAY ? "relay" : "client", name, serv, port, nick);
}

ICOMMAND(ircaddclient, "ssisss", (const char *n, const char *s, int *p, const char *c, const char *h, const char *z), {
    ircnewnet(IRCT_CLIENT, n, s, *p, c, h, z);
});

/**
 * Connect to IRC server
 * @group irc
 * @arg1 name of network
 * @arg2 server
 * @arg3 port
 * @arg4 bot nick
 * @arg5 server ip
 * @arg6 server password
 * @example ircaddrelay gamesurge GameConnect.NL.EU.GameSurge.net 6667 rb10 // connect to gamesurge network
 */
ICOMMAND(ircaddrelay, "ssisss", (const char *n, const char *s, int *p, const char *c, const char *h, const char *z), {
    ircnewnet(IRCT_RELAY, n, s, *p, c, h, z);
});
ICOMMAND(ircserv, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    if(!s || !*s) { ircprintf(n, 4, NULL, "current server is: %s", n->serv); return; }
    copystring(n->serv, s);
});
ICOMMAND(ircport, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    if(!s || !*s || !parseint(s)) { ircprintf(n, 4, NULL, "current port is: %d", n->port); return; }
    n->port = parseint(s);
});
ICOMMAND(ircnick, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    if(!s || !*s) { ircprintf(n, 4, NULL, "current nickname is: %s", n->nick); return; }
    copystring(n->nick, s);
});
ICOMMAND(ircbind, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    if(!s || !*s) { ircprintf(n, 4, NULL, "currently bound to: %s", n->ip); return; }
    copystring(n->ip, s);
});
ICOMMAND(ircpass, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    if(!s || !*s) { ircprintf(n, 4, NULL, "current password is: %s", n->passkey && *n->passkey ? "<set>" : "<not set>"); return; }
    copystring(n->passkey, s);
});
ICOMMAND(ircauth, "sss", (const char *name, const char *s, const char *t), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    if(!s || !*s || !t || !*t) { ircprintf(n, 4, NULL, "current authority is: %s (%s)", n->authname, n->authpass && *n->authpass ? "<set>" : "<not set>"); return; }
    copystring(n->authname, s);
    copystring(n->authpass, t);
});
ICOMMAND(ircconnect, "s", (const char *name), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    if(n->state != IRC_DISC) { ircprintf(n, 4, NULL, "network already already active"); return; }
    ircestablish(n);
});

ircchan *ircfindchan(ircnet *n, const char *name)
{
    if(n && name && *name)
    {
        loopv(n->channels) if(!strcasecmp(n->channels[i].name, name))
            return &n->channels[i];
    }
    return NULL;
}

bool ircjoin(ircnet *n, ircchan *c)
{
    if(!n || !c) return false;
    if(n->state != IRC_ONLINE)
    {
        // remod
        //ircprintf(n, 4, NULL, "cannot join %s until connection is online", c->name);
        conoutf("Irc: cannot join %s until connection is online", n->name);

        return false;
    }
    if(*c->passkey) ircsend(n, "JOIN %s :%s", c->name, c->passkey);
    else ircsend(n, "JOIN %s", c->name);
    c->state = IRCC_JOINING;
    c->lastjoin = clocktime;
    c->lastsync = 0;
    return true;
}

bool ircenterchan(ircnet *n, const char *name)
{
    if(!n) return false;
    ircchan *c = ircfindchan(n, name);
    if(!c)
    {
        // remod
        //ircprintf(n, 4, NULL, "no channel called %s available", name);
        conoutf("Irc: ircnet %s has no channel called %s ready", n->name, name);
        return false;
    }
    return ircjoin(n, c);
}

bool ircnewchan(int type, const char *name, const char *channel, const char *friendly, const char *passkey, int relay)
{
    if(!name || !*name || !channel || !*channel) return false;
    ircnet *n = ircfind(name);
    if(!n)
    {
        conoutf("no such ircnet: %s", name);
        return false;
    }
    ircchan *c = ircfindchan(n, channel);
    if(c)
    {
        // remod
        //ircprintf(n, 4, NULL, "channel %s already exists", c->name);
        conoutf("Irc: %s channel %s already exists", n->name, c->name);

        return false;
    }
    ircchan &d = n->channels.add();
    d.state = IRCC_NONE;
    d.type = type;
    d.relay = relay;
    d.lastjoin = d.lastsync = 0;
    copystring(d.name, channel);
    copystring(d.friendly, friendly && *friendly ? friendly : channel);
    copystring(d.passkey, passkey);
    if(n->state == IRC_ONLINE) ircjoin(n, &d);

    // remod
    //ircprintf(n, 4, NULL, "added channel: %s", d.name);
    conoutf("Irc: %s added channel %s", n->name, d.name);

    return true;
}

/**
 * Join channel on specified network
 * @group irc
 * @arg1 name of network
 * @arg2 channel
 * @arg3 friendly (useless Red eclipse legacy, should be 0)
 * @arg4 channel password
 * @arg5 relay (useless Red eclipse legacy, should be 0)
 * @example ircaddchan gamesurge #my-servers 0 "hpass" // join password protected channel
 */
ICOMMAND(ircaddchan, "ssssi", (const char *n, const char *c, const char *f, const char *z, int *r), {
    ircnewchan(IRCCT_AUTO, n, c, f, z, *r);
});
ICOMMAND(ircjoinchan, "ssssi", (const char *n, const char *c, const char *f, const char *z, int *r), {
    ircnewchan(IRCCT_NONE, n, c, f, z, *r);
});
ICOMMAND(ircpasschan, "sss", (const char *name, const char *chan, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    ircchan *c = ircfindchan(n, chan);
    if(!c) { ircprintf(n, 4, NULL, "no such channel: %s", chan); return; }
    if(!s || !*s) { ircprintf(n, 4, NULL, "channel %s current password is: %s", c->name, c->passkey && *c->passkey ? "<set>" : "<not set>"); return; }
    copystring(c->passkey, s);
});
ICOMMAND(ircrelaychan, "sss", (const char *name, const char *chan, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    ircchan *c = ircfindchan(n, chan);
    if(!c) { ircprintf(n, 4, NULL, "no such channel: %s", chan); return; }
    if(!s || !*s) { ircprintf(n, 4, NULL, "channel %s current relay level is: %d", c->name, c->relay); return; }
    c->relay = parseint(s);
});
ICOMMAND(ircfriendlychan, "sss", (const char *name, const char *chan, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    ircchan *c = ircfindchan(n, chan);
    if(!c) { ircprintf(n, 4, NULL, "no such channel: %s", chan); return; }
    if(!s || !*s) { ircprintf(n, 4, NULL, "channel %s current friendly name is: %s", c->name, c->friendly); return; }
    copystring(c->friendly, s);
});

void ircprocess(ircnet *n, char *user[3], int g, int numargs, char *w[])
{
    if(!strcasecmp(w[g], "NOTICE") || !strcasecmp(w[g], "PRIVMSG"))
    {
        if(numargs > g+2)
        {
            bool ismsg = strcasecmp(w[g], "NOTICE");
            int len = strlen(w[g+2]);
            if(w[g+2][0] == '\v' && w[g+2][len-1] == '\v')
            {
                char *p = w[g+2];
                p++;
                const char *word = p;
                p += strcspn(p, " \v\0");
                if(p-word > 0)
                {
                    char *q = newstring(word, p-word);
                    p++;
                    const char *start = p;
                    p += strcspn(p, "\v\0");
                    char *r = p-start > 0 ? newstring(start, p-start) : newstring("");
                    if(ismsg)
                    {
                        if(!strcasecmp(q, "ACTION"))
                        {
                            //ircprintf(n, 1, g ? w[g+1] : NULL, "\fv* %s %s", user[0], r);
                        }
                        else
                        {
                            ircprintf(n, 4, g ? w[g+1] : NULL, "\fr%s requests: %s %s", user[0], q, r);

                            if(!strcasecmp(q, "VERSION"))
                                ircsend(n, "NOTICE %s :\vVERSION %s v%s-%s %d bit (%s), %s\v", user[0], RE_NAME, RE_VER_STR, RE_PLATFORM, RE_ARCH, RE_RELEASE, RE_URL);
                            else if(!strcasecmp(q, "PING")) // eh, echo back
                                ircsend(n, "NOTICE %s :\vPING %s\v", user[0], r);
                        }
                    }
                    else ircprintf(n, 4, g ? w[g+1] : NULL, "\fr%s replied: %s %s", user[0], q, r);
                    DELETEA(q); DELETEA(r);
                }
            }
            else if(ismsg)
            {
                // remod
                const char *ftext; // command buffer

                const char *p = w[g+2];
                if (n->type == IRCT_RELAY && g &&
                		((strcasecmp(w[g+1], n->nick) &&
								!strncasecmp(w[g+2], n->nick, strlen(n->nick)) &&
								strchr(":;, .\t", w[g+2][strlen(n->nick)]) &&
								(p += strlen(n->nick))) ||
                		(irccommandchar &&
                				strlen(irccommandchar) &&
                				!strncasecmp(w[g+2], irccommandchar, strlen(irccommandchar)) &&
                				(p += strlen(irccommandchar)) )))
                {
                    while(p && (*p == ':' || *p == ';' || *p == ',' || *p == '.' || *p == ' ' || *p == '\t')) p++;

                    // remod
                    if(p && *p)
                    {
                        // hightlighted message to bot
                        // user[0]='degrave'
                        // user[1]='~ezhi'
                        // user[2]='degry.lamer.gamesurge'
                        // p='qqq'
                        ircprintf(n, 0, w[g+1], "\fa<\fw%s\fa>\fw %s", user[0], p);

                        //irc_oncommand "sender" "p a r a m s"
                        ftext = newstring(p);
                        //server::filtercstext(ftext);
                        remod::onevent(IRC_ONCOMMAND, "ss", user[0], ftext);
                        DELETEA(ftext);
                    }
                }
                else
                {
                    // remod
                    //ircprintf(n, 1, g ? w[g+1] : NULL, "\fa<\fw%s\fa>\fw %s", user[0], w[g+2]);
                    // normal and private message
                    // in private w[2]==n->nick
                    // user[0]='degrave'
                    // user[1]='~ezhi'
                    // user[2]='degry.lamer.gamesurge'
                    // w[0]='degrave!~ezhi@degry.lamer.gamesurge'
                    // w[1]='PRIVMSG'
                    // w[2]='#rb-servers'
                    // w[3]='a'

                    //server::filtercstext(ftext);
                    ftext = newstring(w[g+2]);
                    remod::onevent(strcasecmp(w[g+1], n->nick) ? IRC_ONMSG : IRC_ONPRIVMSG, "ss", user[0], ftext);
                    DELETEA(ftext);
                }
            }
            else ircprintf(n, 2, g ? w[g+1] : NULL, "\fo-%s- %s", user[0], w[g+2]);
        }
    }
    else if(!strcasecmp(w[g], "NICK"))
    {
	// remod
        loopv(n->channels)
            if(n->channels[i].rename(user[0], w[g+1])) break;

        if(numargs > g+1)
        {
            if(!strcasecmp(user[0], n->nick)) copystring(n->nick, w[g+1]);
            ircprintf(n, 3, NULL, "\fm%s (%s@%s) is now known as %s", user[0], user[1], user[2], w[g+1]);
        }
    }
    else if(!strcasecmp(w[g], "JOIN"))
    {
        if(numargs > g+1)
        {
            ircchan *c = ircfindchan(n, w[g+1]);
            if(c && !strcasecmp(user[0], n->nick))
            {
                c->state = IRCC_JOINED;
                c->lastjoin = c->lastsync = clocktime;
            }

            // remod
            //ircprintf(n, 3, w[g+1], "\fg%s (%s@%s) has joined", user[0], user[1], user[2]);
            if(strcmp(n->nick, user[0]) != 0)
                c->adduser(user[0], NONE);
        }
    }
    else if(!strcasecmp(w[g], "PART"))
    {
        if(numargs > g+1)
        {
            ircchan *c = ircfindchan(n, w[g+1]);
            if(c && !strcasecmp(user[0], n->nick))
            {
                c->state = IRCC_NONE;
                c->lastjoin = clocktime;
                c->lastsync = 0;
            }

            // remod
            //ircprintf(n, 3, w[g+1], "\fo%s (%s@%s) has left", user[0], user[1], user[2]);
            c->deluser(user[0]);
        }
    }
    else if(!strcasecmp(w[g], "QUIT"))
    {
        // remod
        //if(numargs > g+1) ircprintf(n, 3, NULL, "\fr%s (%s@%s) has quit: %s", user[0], user[1], user[2], w[g+1]);
        //else ircprintf(n, 3, NULL, "\fr%s (%s@%s) has quit", user[0], user[1], user[2]);
        loopv(n->channels)
            n->channels[i].deluser(user[0]);
    }
    else if(!strcasecmp(w[g], "KICK"))
    {
        if(numargs > g+2)
        {
            ircchan *c = ircfindchan(n, w[g+1]);
            if(c && !strcasecmp(w[g+2], n->nick))
            {
                c->state = IRCC_KICKED;
                c->lastjoin = clocktime;
                c->lastsync = 0;
            }

            // remod
            //ircprintf(n, 3, w[g+1], "\fr%s (%s@%s) has kicked %s from %s", user[0], user[1], user[2], w[g+2], w[g+1]);
            c->deluser(user[0]);
        }
    }
    else if(!strcasecmp(w[g], "MODE"))
    {
        if(numargs > g+2)
        {
            mkstring(modestr);
            loopi(numargs-g-2)
            {
                if(i) concatstring(modestr, " ");
                concatstring(modestr, w[g+2+i]);
            }
            ircprintf(n, 4, w[g+1], "\fr%s (%s@%s) sets mode: %s %s", user[0], user[1], user[2], w[g+1], modestr);

            // remod
            ircchan *c = ircfindchan(n, w[g+1]);
            if(c)
            {
                if(w[g+2][0]=='-')      // -
                    c->setusermode(w[g+3], NONE);
                else                    // +
                {
                    switch(w[g+2][1])   // +o +v
                    {
                        case 'o': c->setusermode(w[g+3], OP); break;
                        case 'v': c->setusermode(w[g+3], VOICE); break;
                        case 'm': break; // moderated channel >_<
                        default: c->setusermode(w[g+3], NONE); break;
                    }
                }
            }
        }
        else if(numargs > g+1)
            ircprintf(n, 4, w[g+1], "\fr%s (%s@%s) sets mode: %s", user[0], user[1], user[2], w[g+1]);
    }
    else if(!strcasecmp(w[g], "PING"))
    {
        if(numargs > g+1)
        {
            //ircprintf(n, 4, NULL, "%s PING %s", user[0], w[g+1]);
            ircsend(n, "PONG %s", w[g+1]);
        }
        else
        {
            ircprintf(n, 4, NULL, "%s PING", user[0]);
            ircsend(n, "PONG %d", clocktime);
        }
    }
    // remod
    else if(!strcasecmp(w[g], "PONG"))
    {
        n->lastpong = clocktime;
    }
    else
    {
        int numeric = *w[g] && *w[g] >= '0' && *w[g] <= '9' ? atoi(w[g]) : 0, off = 0;
        mkstring(s);
        #define irctarget(a) (!strcasecmp(n->nick, a) || *a == '#' || ircfindchan(n, a))
        char *targ = numargs > g+1 && irctarget(w[g+1]) ? w[g+1] : NULL;
        if(numeric)
        {
            off = numeric == 353 ? 2 : 1;
            if(numargs > g+off+1 && irctarget(w[g+off+1]))
            {
                targ = w[g+off+1];
                off++;
            }
        }
        else concatstring(s, user[0]);
        for(int i = g+off+1; numargs > i && w[i]; i++)
        {
            if(s[0]) concatstring(s, " ");
            concatstring(s, w[i]);
        }
        if(numeric) switch(numeric)
        {
            case 1:
            {
                if(n->state == IRC_CONN)
                {
                    n->state = IRC_ONLINE;
                    ircprintf(n, 4, NULL, "\fbnow connected to %s as %s", user[0], n->nick);
                    if(*n->authname && *n->authpass) ircsend(n, "PRIVMSG %s :%s", n->authname, n->authpass);

                    // remod
                    if(n->authcmd && n->authcmd[0]) ircsend(n, "%s", n->authcmd);
                }
                break;
            }

            // remod
            case 353:
            {
                // char *s - list of nicks
                char *nick;
                char *pnick;
                mkstring(s2);
                strcpy(s2, s); // dublicate users line
                usermode state=NONE;

                ircchan *c = ircfindchan(n, w[g+3]);
                c->resetusers();

                nick = strtok(s2, " ");
                while(nick!= NULL)
                {
                    switch(nick[0])
                    {
			case '~':
                        case '*': state = OWNER; break;
			case '&':
                        case '!': state = ADMIN; break;
                        case '@': state = OP; break;
                        case '%': state = HALFOP; break;
                        case '+': state = VOICE; break;
                        default: state = NONE; break;
                    }

                    if(state != NONE)
                    {
                        int len = strlen(nick);
                        strncpy(nick, &nick[1], len-1);
                        nick[len-1] = '\0';
                    }

                    pnick=newstring(nick);
                    c->adduser(pnick, state);

                    nick = strtok(NULL, " ");
                }
                break;
            }
            case 433:
            {
                if(n->state == IRC_CONN)
                {
                    concatstring(n->nick, "_");
                    ircsend(n, "NICK %s", n->nick);
                }
                break;
            }
            case 471:
            case 473:
            case 474:
            case 475:
            {
                ircchan *c = ircfindchan(n, w[g+2]);
                if(c)
                {
                    c->state = IRCC_BANNED;
                    c->lastjoin = clocktime;
                    c->lastsync = 0;
                    if(c->type == IRCCT_AUTO)
                        ircprintf(n, 4, w[g+2], "\fbwaiting 5 mins to rejoin %s", c->name);
                    else ircprintf(n, 4, NULL, "\fbbanned from channel: %s", c->name);
                }
                break;
            }
            default: break;
        }
        if(s[0]) ircprintf(n, 4, targ, "\fw%s %s", w[g], s);
        else ircprintf(n, 4, targ, "\fw%s", w[g]);
    }
}

void ircparse(ircnet *n)
{
    const int MAXWORDS = 25;
    char *w[MAXWORDS], *p = (char *)n->input, *start = p, *end = &p[n->inputcarry];
    loopi(MAXWORDS) w[i] = NULL;
    while(p < end)
    {
        bool full = false;
        int numargs = 0, g = 0;
        while(iscubespace(*p)) { if(++p >= end) goto cleanup; }
        start = p;
        if(*p == ':') { g = 1; ++p; }
        for(;;)
        {
            const char *word = p;
            if(*p == ':') { word++; full = true; } // uses the rest of the input line then
            while(*p != '\r' && *p != '\n' && (full || *p != ' ')) { if(++p >= end) goto cleanup; }

            if(numargs < MAXWORDS) w[numargs++] = newstring(word, p-word);

            if(*p == '\n' || *p == '\r') { ++p; start = p; break; }
            else while(*p == ' ') { if(++p >= end) goto cleanup; }
        }
        if(numargs)
        {
            char *user[3] = { NULL, NULL, NULL };
            if(g)
            {
                char *t = w[0], *u = strrchr(t, '!');
                if(u)
                {
                    user[0] = newstring(t, u-t);
                    t = u + 1;
                    u = strrchr(t, '@');
                    if(u)
                    {
                        user[1] = newstring(t, u-t);
                        if(*u++) user[2] = newstring(u);
                        else user[2] = newstring("*");
                    }
                    else
                    {
                        user[1] = newstring("*");
                        user[2] = newstring("*");
                    }
                }
                else
                {
                    user[0] = newstring(t);
                    user[1] = newstring("*");
                    user[2] = newstring("*");
                }
            }
            else
            {
                user[0] = newstring("*");
                user[1] = newstring("*");
                user[2] = newstring(n->serv);
            }
            if(numargs > g) ircprocess(n, user, g, numargs, w);
            loopi(3) DELETEA(user[i]);
        }
    cleanup:
        loopi(MAXWORDS) DELETEA(w[i]);
    }
    int parsed = start - (char *)n->input;
    if(parsed > 0)
    {
        memmove(n->input, start, n->inputlen - parsed);
        n->inputcarry -= parsed;
        n->inputlen -= parsed;
    }
}

void ircdiscon(ircnet *n, const char *msg = NULL)
{
    if(!n) return;
    if(msg) ircprintf(n, 4, NULL, "disconnected from %s (%s:[%d]): %s", n->name, n->serv, n->port, msg);
    else ircprintf(n, 4, NULL, "disconnected from %s (%s:[%d])", n->name, n->serv, n->port);
    enet_socket_destroy(n->sock);
    n->state = IRC_DISC;
    n->sock = ENET_SOCKET_NULL;
    n->lastattempt = clocktime;
}

void irccleanup()
{
    loopv(ircnets) if(ircnets[i]->sock != ENET_SOCKET_NULL)
    {
        ircnet *n = ircnets[i];
        ircsend(n, "QUIT :%s, %s", RE_NAME, RE_URL);
        ircdiscon(n, "shutdown");
    }
}

void ircslice()
{
    clocktime = time(NULL);

    loopv(ircnets)
    {
        ircnet *n = ircnets[i];
        if(n->sock != ENET_SOCKET_NULL && n->state > IRC_DISC)
        {
            switch(n->state)
            {
                case IRC_ATTEMPT:
                {
                    if(*n->passkey) ircsend(n, "PASS %s", n->passkey);
                    ircsend(n, "NICK %s", n->nick);
                    ircsend(n, "USER %s +iw %s :%s v%s-%s %d bit (%s)", RE_UNAME, RE_UNAME, RE_NAME, RE_VER_STR, RE_PLATFORM, RE_ARCH, RE_RELEASE);
                    n->state = IRC_CONN;
                    loopvj(n->channels)
                    {
                        ircchan *c = &n->channels[j];
                        c->state = IRCC_NONE;
                        c->lastjoin = clocktime;
                        c->lastsync = 0;
                    }
                    break;
                }
                case IRC_ONLINE:
                {
                    loopvj(n->channels)
                    {
                        ircchan *c = &n->channels[j];
                        if(c->type == IRCCT_AUTO && c->state != IRCC_JOINED && (!c->lastjoin || clocktime-c->lastjoin >= (c->state != IRCC_BANNED ? 5 : 300)))
                            ircjoin(n, c);
                    }
                    // fall through
                }
                case IRC_CONN:
                {
                    if(n->state == IRC_CONN && clocktime-n->lastattempt >= 60) ircdiscon(n, "connection attempt timed out");
                    else switch(ircrecv(n))
                    {
                        case -3: ircdiscon(n, "read error"); break;
                        case -2: ircdiscon(n, "connection reset"); break;
                        case -1: ircdiscon(n, "invalid connection"); break;
                        case 0: break;
                        default: ircparse(n); break;
                    }
                    break;
                }
                default:
                {
                    ircdiscon(n, "encountered unknown connection state");
                    break;
                }
            }
        }
        else if(!n->lastattempt || clocktime-n->lastattempt >= 60) ircestablish(n);
    }

    // remod
    ping();
}

// remod
usermode irc_user_state(char *nick)
{
    if(!nick) return ERR;

    loopv(ircnets)
    {
        loopvj(ircnets[i]->channels)
        {
            loopvk(ircnets[i]->channels[j].users)
            {
                //conoutf("compare '%s' - '%s'",ircnets[i]->channels[j].users[k].nick, nick);
                if(strcmp(ircnets[i]->channels[j].users[k].nick, nick)==0)
                {
                    return ircnets[i]->channels[j].users[k].state;
                }

            }
        }
    }
    return ERR;
}

void ircisop(char *name)
{
    intret((irc_user_state(name) == OP) || (irc_user_state(name) == OWNER) || (irc_user_state(name) == ADMIN) || (irc_user_state(name) == HALFOP));
}

void ircisvoice(char *name)
{
    intret(irc_user_state(name) == VOICE);
}

void ircsayto(char *to, char *msg)
{

    loopv(ircnets)
    {
        ircnet *in = ircnets[i];
        ircsend(in, "PRIVMSG %s :%s", to, msg);
    }
}

void ircaction(char *msg)
{
    loopv(ircnets)
    {
        loopvj(ircnets[i]->channels)
        {
            char *to = ircnets[i]->channels[j].name;
            ircnet *in = ircnets[i];
            ircsend(in, "PRIVMSG %s :%sACTION %s%s", to, "\001\0", msg, "\001\0");
        }
    }
}

void ping()
{
    // don't ping
    if(ircpingdelay == 0) return;

    loopv(ircnets)
    {
        ircnet *in = ircnets[i];
        if(in->state == IRC_ONLINE && (clocktime - in->lastping) > ircpingdelay)
        {
            ircsend(in, "PING %u", clocktime);
            in->lastping = clocktime;
        }
    }
}

void ircauthcmd(char *name, char *cmd)
{
    ircnet *n = ircfind(name);
    if(!n) { conoutf("no such ircnet: %s", name); return; }
    if(n->authcmd) DELETEA(n->authcmd);
    n->authcmd = newstring(cmd);
}

#if 1
void irc_dumpnicks()
{
    loopv(ircnets)
        loopvj(ircnets[i]->channels)
            loopvk(ircnets[i]->channels[j].users)
                printf("ircnets[%i]->channels[%i].users[%i]=%s stat=%d\n", i, j, k, ircnets[i]->channels[j].users[k].nick, ircnets[i]->channels[j].users[k].state);
}
COMMAND(irc_dumpnicks, "");
#endif

/**
 * Check if specified nick in IRC have OP status
 * @group irc
 * @arg1 nick
 * @return 1 or 0
 * @example echo (ircisop "mib_4565") // check if guy have OP
 */
COMMAND(ircisop, "s");

/**
 * Check if specified nick in IRC have VOICE
 * @group irc
 * @arg1 nick
 * @return 1 or 0
 * @example echo (ircisvoice "mib_4565") // check if guy have VOICE
 */
COMMAND(ircisvoice, "s");

/**
 * Send text to all IRC channels
 * @group irc
 * @arg1 text
 */
ICOMMAND(ircsay, "s", (const char *msg ), { ircoutf(0, "%s", msg); });

/**
 * Send text to specified destination (nick or channel)
 * @group irc
 * @arg1 destination
 * @arg2 text
 * @example ircsayto "mib_4564" "Go away mib_4564" // send private message
 */
COMMAND(ircsayto, "ss");

/**
 * The server is an action message.
 * @group irc
 * @arg1 message
 * @example ircaction "waves hello", What it looks like: *server waves hello
 */
COMMAND(ircaction, "s");

/**
 * Authenticate commmand after connect to the IRC network.
 * @group irc
 * @arg1 network name
 * @arg2 command
 * @example ircauthcmd "gamesurge" "authserv auth rbserver dJ84H8dh"
 */
COMMAND(ircauthcmd, "ss");

#endif
