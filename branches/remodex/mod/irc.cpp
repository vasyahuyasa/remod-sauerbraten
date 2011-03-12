#ifdef IRC
#include "irc.h"

//REmod
VAR(verbose, 0, 0, 6);
SVAR(consoletimefmt, "%c");

char *gettime(char *format)
{
    time_t ltime;
    struct tm *t;

    ltime = time (NULL);
    t = localtime (&ltime);

    static string buf;
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

void ircestablish(ircnet *n)
{
    if(!n) return;
    n->lastattempt = totalmillis;
    if(n->address.host == ENET_HOST_ANY)
    {
        conoutf("Irc: looking up %s:[%d]...", n->serv, n->port);
        if(!resolverwait(n->serv, &n->address))
        {
            conoutf("Irc: unable to resolve %s:[%d]...", n->serv, n->port);
            n->state = IRC_DISC;
            return;
        }
    }

    ENetAddress address = { ENET_HOST_ANY,  n->port };
    if(*n->ip && enet_address_set_host(&address, n->ip) < 0) conoutf("Irc: failed to bind address: %s", n->ip);
    n->sock = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
    if(n->sock != ENET_SOCKET_NULL && *n->ip && enet_socket_bind(n->sock, &address) < 0)
    {
        conoutf("Irc: failed to bind connection socket: %s", n->ip);
        address.host = ENET_HOST_ANY;
    }
    if(n->sock == ENET_SOCKET_NULL || connectwithtimeout(n->sock, n->serv, n->address) < 0)
    {
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
    conoutf("Irc: connecting to %s:[%d]...", n->serv, n->port);
}

void ircsend(ircnet *n, const char *msg, ...)
{
    if(!n) return;
    defvformatstring(str, msg, msg);
    if(n->sock == ENET_SOCKET_NULL) return;
    if(verbose >= 2) conoutf(0, "Irc: [%s] >>> %s", n->name, str);
    concatstring(str, "\n");
    ENetBuffer buf;
    buf.data = str;
    buf.dataLength = strlen((char *)buf.data);
    enet_socket_send(n->sock, NULL, &buf, 1);
}

VAR(ircfilter, 0, 1, 2);

void converttext(char *dst, const char *src)
{
    int colorpos = 0; char colorstack[10];
    loopi(10) colorstack[i] = 'u'; //indicate user color
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
            else if(c == 's') { colorpos++; continue; }
            else if(c == 'S') { c = colorstack[--colorpos]; }
            int oldcolor = colorstack[colorpos]; colorstack[colorpos] = c;
            switch(c)
            {
                case 'g': case '0': *dst++ = '\x03'; *dst++ = '0'; *dst++ = '3'; break; // green
                case 'b': case '1': *dst++ = '\x03'; *dst++ = '0'; *dst++ = '2'; break; // blue
                case 'y': case '2': *dst++ = '\x03'; *dst++ = '0'; *dst++ = '8'; break; // yellow
                case 'r': case '3': *dst++ = '\x03'; *dst++ = '0'; *dst++ = '4'; break; // red
                case 'a': case '4': *dst++ = '\x03'; *dst++ = '1'; *dst++ = '5'; break; // grey
                case 'm': case '5': *dst++ = '\x03'; *dst++ = '1'; *dst++ = '3'; break; // magenta
                case 'o': case '6': *dst++ = '\x03'; *dst++ = '0'; *dst++ = '7'; break; // orange
                case 'c': case '9': *dst++ = '\x03'; *dst++ = '1'; *dst++ = '2'; break; // cyan
                case 'v': case 'A': *dst++ = '\x03'; *dst++ = '0'; *dst++ = '6'; break; // violet
                case 'p': case 'B': *dst++ = '\x03'; *dst++ = '0'; *dst++ = '6'; break; // purple
                case 'n': case 'C': *dst++ = '\x03'; *dst++ = '0'; *dst++ = '5'; break; // brown
                case 'u': case 'w': case '7': case 'k': case '8': *dst++ = '\x0f'; break;
                default: colorstack[colorpos] = oldcolor; break;
            }
            continue;
        }
        if(isspace(c) || isprint(c)) *dst++ = c;
    }
    *dst = '\0';
}

void ircoutf(int relay, const char *msg, ...)
{
    defvformatstring(src, msg, msg); mkstring(str);
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

int ircrecv(ircnet *n, int timeout)
{
    if(!n) return -1;
    if(n->sock == ENET_SOCKET_NULL) return -1;
    enet_uint32 events = ENET_SOCKET_WAIT_RECEIVE;
    ENetBuffer buf;
    int nlen = strlen((char *)n->input);
    buf.data = ((char *)n->input)+nlen;
    buf.dataLength = sizeof(n->input)-nlen;
    if(enet_socket_wait(n->sock, &events, timeout) >= 0 && events)
    {
        int len = enet_socket_receive(n->sock, NULL, &buf, 1);
        if(len <= 0)
        {
            enet_socket_destroy(n->sock);
            return -1;
        }
        buf.data = ((char *)buf.data)+len;
        ((char *)buf.data)[0] = 0;
        buf.dataLength -= len;
        return len;
    }
    return 0;
}

void ircnewnet(int type, const char *name, const char *serv, int port, const char *nick, const char *ip, const char *passkey)
{
    if(!name || !*name || !serv || !*serv || !port || !nick || !*nick) return;
    ircnet *m = ircfind(name);
    if(m)
    {
        if(m->state != IRC_DISC) conoutf("Irc: ircnet %s already exists", m->name);
        else ircestablish(m);
        return;
    }
    ircnet &n = *ircnets.add(new ircnet);
    n.type = type;
    n.state = IRC_DISC;
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
    conoutf("Irc: added irc %s %s (%s:%d) [%s]", type == IRCT_RELAY ? "relay" : "client", name, serv, port, nick);
}

ICOMMAND(ircaddclient, "ssisss", (const char *n, const char *s, int *p, const char *c, const char *h, const char *z), {
    ircnewnet(IRCT_CLIENT, n, s, *p, c, h, z);
});
ICOMMAND(ircaddrelay, "ssisss", (const char *n, const char *s, int *p, const char *c, const char *h, const char *z), {
    ircnewnet(IRCT_RELAY, n, s, *p, c, h, z);
});
ICOMMAND(ircserv, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    if(!s || !*s) { conoutf("Irc: %s current server is: %s", n->name, n->serv); return; }
    copystring(n->serv, s);
});
ICOMMAND(ircport, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    if(!s || !*s || !parseint(s)) { conoutf("Irc: %s current port is: %d", n->name, n->port); return; }
    n->port = parseint(s);
});
ICOMMAND(ircnick, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    if(!s || !*s) { conoutf("Irc: %s current nickname is: %s", n->name, n->nick); return; }
    copystring(n->nick, s);
});
ICOMMAND(ircbind, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    if(!s || !*s) { conoutf("Irc: %s currently bound to: %s", n->name, n->ip); return; }
    copystring(n->ip, s);
});
ICOMMAND(ircpass, "ss", (const char *name, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    if(!s || !*s) { conoutf("%s current password is: %s", n->name, n->passkey && *n->passkey ? "<set>" : "<not set>"); return; }
    copystring(n->passkey, s);
});
ICOMMAND(ircauth, "sss", (const char *name, const char *s, const char *t), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    if(!s || !*s || !t || !*t) { conoutf("Irc: %s current authority is: %s (%s)", n->name, n->authname, n->authpass && *n->authpass ? "<set>" : "<not set>"); return; }
    copystring(n->authname, s);
    copystring(n->authpass, t);
});
ICOMMAND(ircconnect, "s", (const char *name), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    if(n->state != IRC_DISC) { conoutf("Irc: ircnet %s is already connected", n->name); return; }
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
    if(n->state == IRC_DISC)
    {
        conoutf("Irc: ircnet %s is not connected", n->name);
        return false;
    }
    if(*c->passkey) ircsend(n, "JOIN %s :%s", c->name, c->passkey);
    else ircsend(n, "JOIN %s", c->name);
    c->state = IRCC_JOINING;
    c->lastjoin = totalmillis;
    return true;
}

bool ircenterchan(ircnet *n, const char *name)
{
    if(!n) return false;
    ircchan *c = ircfindchan(n, name);
    if(!c)
    {
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
        conoutf("Irc: no such ircnet: %s", name);
        return false;
    }
    ircchan *c = ircfindchan(n, channel);
    if(c)
    {
        conoutf("Irc: %s already has channel %s", n->name, c->name);
        return false;
    }
    ircchan &d = n->channels.add();
    d.state = IRCC_NONE;
    d.type = type;
    d.relay = relay;
    d.lastjoin = 0;
    copystring(d.name, channel);
    copystring(d.friendly, friendly && *friendly ? friendly : channel);
    copystring(d.passkey, passkey);
    if(n->state != IRC_DISC) ircjoin(n, &d);
    conoutf("Irc: %s added channel %s", n->name, d.name);
    return true;
}

ICOMMAND(ircaddchan, "ssssi", (const char *n, const char *c, const char *f, const char *z, int *r), {
    ircnewchan(IRCCT_AUTO, n, c, f, z, *r);
});
ICOMMAND(ircjoinchan, "ssssi", (const char *n, const char *c, const char *f, const char *z, int *r), {
    ircnewchan(IRCCT_NONE, n, c, f, z, *r);
});
ICOMMAND(ircpasschan, "sss", (const char *name, const char *chan, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    ircchan *c = ircfindchan(n, chan);
    if(!c) { conoutf("Irc: no such %s channel: %s", n->name, chan); return; }
    if(!s || !*s) { conoutf("Irc: %s channel %s current password is: %s", n->name, c->name, c->passkey && *c->passkey ? "<set>" : "<not set>"); return; }
    copystring(c->passkey, s);
});
ICOMMAND(ircrelaychan, "sss", (const char *name, const char *chan, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    ircchan *c = ircfindchan(n, chan);
    if(!c) { conoutf("Irc: no such %s channel: %s", n->name, chan); return; }
    if(!s || !*s) { conoutf("Irc: %s channel %s current relay level is: %d", n->name, c->name, c->relay); return; }
    c->relay = parseint(s);
});
ICOMMAND(ircfriendlychan, "sss", (const char *name, const char *chan, const char *s), {
    ircnet *n = ircfind(name);
    if(!n) { conoutf("Irc: no such ircnet: %s", name); return; }
    ircchan *c = ircfindchan(n, chan);
    if(!c) { conoutf("Irc: no such %s channel: %s", n->name, chan); return; }
    if(!s || !*s) { conoutf("Irc: %s channel %s current friendly name is: %s", n->name, c->name, c->friendly); return; }
    copystring(c->friendly, s);
});

void ircprintf(ircnet *n, int relay, const char *target, const char *msg, ...)
{
    defvformatstring(str, msg, msg);
    mkstring(s);
    if(target && *target && strcasecmp(target, n->nick))
    {
     	ircchan *c = ircfindchan(n, target);
        if(c)
        {
            formatstring(s)("\fs\fa[%s:%s]\fS", n->name, c->name);
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

void ircprocess(ircnet *n, char *user[3], int g, int numargs, char *w[])
{
    if(!strcasecmp(w[g], "NOTICE") || !strcasecmp(w[g], "PRIVMSG"))
    {

        if(numargs > g+2)
        {
            bool ismsg = strcasecmp(w[g], "NOTICE");
            int len = strlen(w[g+2]);
            if(w[g+2][0] == '\001' && w[g+2][len-1] == '\001')
            {
                char *p = w[g+2];
                p++;
                const char *word = p;
                p += strcspn(p, " \001\0");
                if(p-word > 0)
                {
                    char *q = newstring(word, p-word);
                    p++;
                    const char *start = p;
                    p += strcspn(p, "\001\0");
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
                                ircsend(n, "NOTICE %s :\001VERSION %s v%.2f-%s (%s), %s\001", user[0], ENG_NAME, float(ENG_VERSION)/100.f, ENG_PLATFORM, ENG_RELEASE, ENG_URL);
                            else if(!strcasecmp(q, "PING")) // eh, echo back
                                ircsend(n, "NOTICE %s :\001PING %s\001", user[0], r);
                        }
                    }
                    else ircprintf(n, 4, g ? w[g+1] : NULL, "\fr%s replied: %s %s", user[0], q, r);
                    DELETEA(q); DELETEA(r);
                }
            }
            else if(ismsg)
            {
                //Remod
                char ftext[MAXTRANS]; // command buffer

                if(n->type == IRCT_RELAY && g && strcasecmp(w[g+1], n->nick) && !strncasecmp(w[g+2], n->nick, strlen(n->nick)))
                {
                    const char *p = &w[g+2][strlen(n->nick)];
                    bool used_delimiter = false; //use delimiters when dialing to bot
                    while(p && (*p == ':' || *p == ';' || *p == ',' || *p == '.' || *p == ' ' || *p == '\t')) {
                    	used_delimiter = true;
                    	p++;
                    };
                    if(used_delimiter && p && *p)
                    {
                        // hightlighted message to bot
                        // user[0]='degrave'
                        // user[1]='~ezhi'
                        // user[2]='degry.lamer.gamesurge'
                        // p='qqq'
                        ircprintf(n, 0, w[g+1], "\fa<\fw%s\fa>\fw %s", user[0], p);

                        //irc_oncommand "sender" "p a r a m s"
                        server::filtercstext(ftext, p);
                        remod::onevent("irc_oncommand", "ss", user[0], ftext);
                    }
                }
                else
                {
                    // normal and private message
                    // in private w[2]==n->nick
                    // user[0]='degrave'
                    // user[1]='~ezhi'
                    // user[2]='degry.lamer.gamesurge'
                    // w[0]='degrave!~ezhi@degry.lamer.gamesurge'
                    // w[1]='PRIVMSG'
                    // w[2]='#rb-servers'
                    // w[3]='a'

                    //ircprintf(n, 1, g ? w[g+1] : NULL, "\fa<\fw%s\fa>\fw %s", user[0], w[g+2]);
                    //Remod
                    server::filtercstext(ftext, w[g+2]);
                    if(strcasecmp(w[g+1], n->nick)) // normal msg
                    {
                        remod::onevent("irc_onmsg", "ss", user[0], ftext);
                    }
                    else
                    {
                        remod::onevent("irc_onprivmsg", "ss", user[0], ftext);
                    }
                }
            }
            else ircprintf(n, 2, g ? w[g+1] : NULL, "\fo-%s- %s", user[0], w[g+2]);
        }
    }
    else if(!strcasecmp(w[g], "NICK"))
    {
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
                c->lastjoin = totalmillis;
            }
            //ircprintf(n, 3, w[g+1], "\fg%s (%s@%s) has joined", user[0], user[1], user[2]);

            // remod
            if(strcmp(n->nick, user[0]) != 0)
            {
                c->adduser(user[0], NONE);
            }
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
                c->lastjoin = totalmillis;
            }
            //ircprintf(n, 3, w[g+1], "\fo%s (%s@%s) has left", user[0], user[1], user[2]);

            // remod
            c->deluser(user[0]);
        }
    }
    else if(!strcasecmp(w[g], "QUIT"))
    {
        //if(numargs > g+1) ircprintf(n, 3, NULL, "\fr%s (%s@%s) has quit: %s", user[0], user[1], user[2], w[g+1]);
        //else ircprintf(n, 3, NULL, "\fr%s (%s@%s) has quit", user[0], user[1], user[2]);

        // remod
            loopv(n->channels)
            {
                n->channels[i].deluser(user[0]);
            }
    }
    else if(!strcasecmp(w[g], "KICK"))
    {
        if(numargs > g+2)
        {
            ircchan *c = ircfindchan(n, w[g+1]);
            if(c && !strcasecmp(w[g+2], n->nick))
            {
                c->state = IRCC_KICKED;
                c->lastjoin = totalmillis;
            }
            //ircprintf(n, 3, w[g+1], "\fr%s (%s@%s) has kicked %s from %s", user[0], user[1], user[2], w[g+2], w[g+1]);

            // remod
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
                if(w[g+2][0]=='-') // -
                {
                    c->setusermode(w[g+3], NONE);
                }
                else // +
                {
                    switch(w[g+2][1]) // +o +v
                    {
                        case 'o': c->setusermode(w[g+3], OP); break;
                        case 'v': c->setusermode(w[g+3], VOICE); break;
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
            int t = time(NULL);
            ircprintf(n, 4, NULL, "%s PING (empty)", user[0]);
            ircsend(n, "PONG %d", t);
        }
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
            case 001:
            {
                if(n->state == IRC_CONN)
                {
                    n->state = IRC_ONLINE;
                    ircprintf(n, 4, NULL, "\fbnow connected to %s as %s", user[0], n->nick);
                    if(*n->authname && *n->authpass) ircsend(n, "PRIVMSG %s :%s", n->authname, n->authpass);
                }
                break;
            }

            //Remod
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
                        case '*': state = OWNER; break;
                        case '!': state = ADMIN; break;
                        case '@': state = OP; break;
                        case '+': state = VOICE; break;
                        case '%': state = HALFOP; break;
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
                    c->lastjoin = totalmillis;
                    if(c->type == IRCCT_AUTO)
                        ircprintf(n, 4, w[g+2], "\fbwaiting 5 mins to rejoin %s", c->name);
                }
                break;
            }
            default: break;
        }
        if(s[0]) ircprintf(n, 4, targ, "\fw%s %s", w[g], s);
        else ircprintf(n, 4, targ, "\fw%s", w[g]);
    }
}

void ircparse(ircnet *n, char *reply)
{
    const int MAXWORDS = 25;
    char *w[MAXWORDS], *p = reply;
    loopi(MAXWORDS) w[i] = NULL;
    while(p && *p)
    {
        while(p && (*p == '\n' || *p == '\r' || *p == ' ')) p++; // eat up all the crap
        const char *start = p;
        bool line = false;
        int numargs = 0, g = *p == ':' ? 1 : 0;
        if(g) p++;
        bool full = false;
        loopi(MAXWORDS)
        {
            if(!p || !*p) break;
            const char *word = p;
            if(*p == ':') full = true; // uses the rest of the input line then
            p += strcspn(p, full ? "\r\n" : " \r\n");

            char *s = NULL;
            if(p-word > (full ? 1 : 0))
            {
                if(full) s = newstring(word+1, p-word-1);
                else s = newstring(word, p-word);
            }
            else s = newstring("");
            w[numargs] = s;
            numargs++;

            if(*p == '\n' || *p == '\r') line = true;
            if(*p) p++; else break;
            if(line) break;
        }
        if(line && numargs)
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
                    }
                }
                else user[0] = newstring(t);
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
        loopi(MAXWORDS) DELETEA(w[i]);
        if(!line)
        {
            char *s = newstring(start); // can't copy a buffer into itself so dupe it first
            copystring(reply, s);
            DELETEA(s);
            return;
        }
    }
    *reply = 0;
}

void ircdiscon(ircnet *n)
{
    if(!n) return;
    ircprintf(n, 4, NULL, "disconnected from %s (%s:[%d])", n->name, n->serv, n->port);
    enet_socket_destroy(n->sock);
    n->state = IRC_DISC;
    n->sock = ENET_SOCKET_NULL;
    n->lastattempt = totalmillis;
}

void irccleanup()
{
    loopv(ircnets) if(ircnets[i]->sock != ENET_SOCKET_NULL)
    {
        ircnet *n = ircnets[i];
        ircsend(n, "QUIT :%s, %s", ENG_NAME, ENG_URL);
        ircdiscon(n);
    }
}

void ircslice()
{
    loopv(ircnets)
    {
        ircnet *n = ircnets[i];
        if(n->sock != ENET_SOCKET_NULL && n->state != IRC_DISC)
        {
            switch(n->state)
            {
                case IRC_ATTEMPT:
                {
                    if(*n->passkey) ircsend(n, "PASS %s", n->passkey);
                    ircsend(n, "NICK %s", n->nick);
                    ircsend(n, "USER %s +iw %s :%s", n->nick, n->nick, n->nick);
                    n->state = IRC_CONN;
                    loopvj(n->channels)
                    {
                        ircchan *c = &n->channels[j];
                        c->state = IRCC_NONE;
                        c->lastjoin = totalmillis;
                    }
                    break;
                }
                case IRC_ONLINE:
                {
                    loopvj(n->channels)
                    {
                        ircchan *c = &n->channels[j];
                        if(c->type == IRCCT_AUTO && c->state != IRCC_JOINED && (!c->lastjoin || totalmillis-c->lastjoin >= (c->state != IRCC_BANNED ? 5000 : 300000)))
                            ircjoin(n, c);
                    }
                    // fall through
                }
                case IRC_CONN:
                {
                    if(n->state == IRC_CONN && totalmillis-n->lastattempt >= 60000)
                    {
                        ircprintf(n, 4, NULL, "connection attempt timed out");
                        ircdiscon(n);
                    }
                    else switch(ircrecv(n))
                    {
                        case -1: ircdiscon(n); break; // fail
                        case 0: break;
                        default:
                        {
                            ircparse(n, (char *)n->input);
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    ircdiscon(n);
                    break;
                }
            }
        }
        else if(!n->lastattempt || totalmillis-n->lastattempt >= 60000) ircestablish(n);
    }
}

bool irc_user_state(char *nick, usermode state)
{
    loopv(ircnets)
    {
        loopvj(ircnets[i]->channels)
        {
            loopvk(ircnets[i]->channels[j].users)
            {
                conoutf("compare '%s' - '%s'",ircnets[i]->channels[j].users[k].nick, nick);
                if(strcmp(ircnets[i]->channels[j].users[k].nick, nick)==0)
                {
                    if(ircnets[i]->channels[j].users[k].state == state)
                        return true;
                        else
                        return false;
                }

            }
        }
    }
    return false;
}

void ircisop(char *name)
{
    intret(irc_user_state(name, OP));
}

void ircisvoice(char *name)
{
    intret(irc_user_state(name, VOICE));
}

void ircsayto(char *to, char *msg)
{

    loopv(ircnets)
    {
        ircnet *in = ircnets[i];
        ircsend(in, "PRIVMSG %s :%s", to, msg);
    }
}

#if 1
void irc_dumpnicks()
{
    loopv(ircnets)
    {
        loopvj(ircnets[i]->channels)
        {
            loopvk(ircnets[i]->channels[j].users)
            {
                printf("ircnets[%i]->channels[%i].users[%i]=%s stat=%d\n", i, j, k, ircnets[i]->channels[j].users[k].nick, ircnets[i]->channels[j].users[k].state);
            }
        }
    }
}
COMMAND(irc_dumpnicks, "");
#endif

COMMAND(ircisop, "s");
COMMAND(ircisvoice, "s");
ICOMMAND(ircconns, "", (void), { int num = 0; loopv(ircnets) if(ircnets[i]->state >= IRC_ATTEMPT) num++; intret(num); });
ICOMMAND(ircsay, "s", (const char *msg ), { ircoutf(0, msg); });
COMMAND(ircsayto, "ss");

#endif
