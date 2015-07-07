/*
* remod:    remod.cpp
* date:     2007
*
* some routines
*/



#include "commandev.h"
#include "remod.h"
#include "banlist.h"

EXTENSION(REMOD);

extensionslist *extensions = NULL;

bool addextension(const char *name)
{
    if(!extensions) extensions = new extensionslist;
    char *s = newstring(name);
    extensions->add(s);
    return false;
}

const extensionslist* getextensionslist()
{
    return extensions;
}

char *conc(char **w, int n, bool space)
{
    int len = space ? max(n-1, 0) : 0;
    loopj(n) len += (int)strlen(w[j]);
    char *r = newstring("", len);
    loopi(n)
    {
        strcat(r, w[i]);  // make string-list out of all arguments
        if(i==n-1) break;
        if(space) strcat(r, " ");
    }
    return r;
}

// local auth
SVAR(authfile, "auth.cfg");

void reloadauth()
{
    server::clearusers();
    execfile(authfile);
}

// sleep which not blocked by pause
struct sleepcmd
{
    int delay, millis, flags;
    char *command;
};

// not blocked by pause sleep command
vector<sleepcmd> asleepcmds;

extern int identflags;
remod::banlist::banmanager *bm = new remod::banlist::banmanager;

void addasleep(int *msec, char *cmd)
{
    sleepcmd &s = asleepcmds.add();
    s.delay = max(*msec, 1);
    s.millis = totalmillis;
    s.command = newstring(cmd);
    s.flags = identflags;
}

/**
 * Wait certain milliseconds, not blocked by pause
 * @group server
 * @arg1 millis
 */
COMMANDN(asleep, addasleep, "is");

void checkasleep(int millis)
{
    loopv(asleepcmds)
    {
        sleepcmd &s = asleepcmds[i];
        if(millis - s.millis >= s.delay)
        {
            char *cmd = s.command; // execute might create more sleep commands
            s.command = NULL;
            int oldflags = identflags;
            identflags = s.flags;
            execute(cmd);
            identflags = oldflags;
            delete[] cmd;
            if(asleepcmds.inrange(i) && !asleepcmds[i].command) asleepcmds.remove(i--);
        }
    }
}

void clearasleep(bool clearoverrides)
{
    int len = 0;
    loopv(asleepcmds) if(asleepcmds[i].command)
    {
        if(clearoverrides && !(asleepcmds[i].flags&IDF_OVERRIDDEN)) asleepcmds[len++] = asleepcmds[i];
        else delete[] asleepcmds[i].command;
    }
    asleepcmds.shrink(len);
}

void clearasleep_(int *clearoverrides)
{
    clearasleep(*clearoverrides!=0 || identflags&IDF_OVERRIDDEN);
}

/**
 * Clear asleep queue
 * @group server
 * @arg1 clearoverrides
 */
COMMANDN(clearasleep, clearasleep_, "i");

namespace server
{
    void filtercstext(char *str)
    {
        for(char *c = str; c && *c; c++)
        {
            if (*c == '\"') { *c = '\''; }
        }
    }

    bool checkpban(uint ip)
    {
        return bm->checkban(ip);
    }

    // remod implementation of addban
    void addban(int cn, char* actorname, int expire)
    {
        int actor = remod::parseplayer(actorname);
        clientinfo *vic = getinfo(cn);
        clientinfo *act = getinfo(actor);
        if(vic)
        {
            uint ip = getclientip(cn);
            allowedips.removeobj(ip);
            ban b;
            b.ip = ip;
            b.time = totalmillis;
            b.expire = totalmillis + expire;
            strcpy(b.name, vic->name);
            b.actor[0] = '\0';
            b.actorip = 0;

            if(act)
            {
                strcpy(b.actor, act->name);
                b.actorip = getclientip(cn);
            }
            else
            	strcpy(b.actor, actorname);

            loopv(bannedips) if(b.expire < bannedips[i].expire) { bannedips.insert(i, b); return; }
            bannedips.add(b);
        }
    }

    void addpban(char *name, const char *reason)
    {
        enet_uint32 ip, mask;
        bm->parseipstring(name, ip, mask);
        bm->addban(NULL, ip, mask, 0, time(0), "server", reason);

        loopvrev(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->local || ci->privilege >= PRIV_ADMIN) continue;
            if(checkpban(getclientip(ci->clientnum)))
            {
                remod::oneventi(ONKICK, "ii", -1, ci->clientnum);
                disconnect_client(ci->clientnum, DISC_IPBAN);
            }
        }
    }

    /*
    void addpban(char *name, const char *reason)
    {

        union
        {
            uchar b[sizeof(enet_uint32)];
            enet_uint32 i;
        } ip, mask;
        ip.i = 0;
        mask.i = 0;
        char *next = name;
        int n;

        if(*next)
            loopi(4)
            {
                n = strtol(next, &next, 10);
                ip.b[i] = n;
                mask.b[i] = 0xFF;
                if(*next && *next == '.') next++;
                if(!*next || *next == '/') break;
            }

        // CIDR
        if(*next && *next == '/' && next++)
        {
            n = strtol(next, NULL, 10);
            conoutf("next = %s, n = %i", next, n);
            mask.i = ~0;
            mask.i <<= (32-n);
            mask.i = htonl(mask.i);
        }

        // add ban and kick banned
        allowedips.removeobj(ip.i);

        permban b;
        b.ip    = ip.i;
        b.mask  = mask.i;
        strcpy(b.reason, reason);
        b.reason[MAXSTRLEN-1] = '\0'; // to avoid problems in future
        permbans.add(b);

        loopvrev(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->local || ci->privilege >= PRIV_ADMIN) continue;
            if(checkpban(getclientip(ci->clientnum)))
            {
                remod::oneventi(ONKICK, "ii", -1, ci->clientnum);
                disconnect_client(ci->clientnum, DISC_IPBAN);
            }
        }


    }*/
}


namespace remod
{

    using namespace server;

void extstate::reset()
{
    muted = false;
    ghost = false;

    lastmutetrigger= 0;
    lastghosttrigger = 0;

    loopi(NUMGUNS)
    {
        guninfo[i].damage = 0;
        guninfo[i].shotdamage = 0;
    }
}

// Find best frager
clientinfo* findbest(vector<clientinfo *> &a)
{
    int bestfrags = a[0]->state.frags;
    int bfrager = 0;
    loopv(a)
    {
        if(a[i]->state.frags > bestfrags)
        {
            bestfrags = a[i]->state.frags;
            bfrager = i;
        }
    }

    clientinfo *ci = a.remove(bfrager);
    return ci;
}

bool playerexists(int *pcn)
{
    int cn = (int)*pcn;
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->clientnum==cn)
        {
            return true;
        }
    }
    return false;
}

// convert player name to cn if exists or -1
// imported from client
int parseplayer(const char *arg)
{
    char *end;
    int n = strtol(arg, &end, 10);
    if(*arg && !*end)
    {
        if(!playerexists(&n)) return -1;
        return n;
    }
    // try case sensitive first
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(!strcmp(arg, ci->name)) return ci->clientnum;
    }
    // nothing found, try case insensitive
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(!strcasecmp(arg, ci->name)) return ci->clientnum;
    }
    return -1;
}

bool ismaster(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    return (ci && ci->privilege >= PRIV_MASTER);
}

bool isadmin(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    return (ci && ci->privilege >= PRIV_ADMIN);
}

bool isspectator(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    return (ci && ci->state.state==CS_SPECTATOR);
}

bool isediting(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    return (ci && ci->state.state==CS_EDITING);
}

void concatpstring(char** str, int count, ...) {

	size_t len = strlen(*str);
	va_list ap;
	va_start(ap, count);
	for (int i = 0; i < count; i++) {
		const char *s = va_arg(ap, const char*);
		len += strlen(s);
	}
	va_end(ap);

	char *res = newstring(*str, len);
	va_start(ap, count);
	char *p = res + strlen(*str);
	for (int i = 0; i < count; i++) {
		const char *s = va_arg(ap, const char*);
		strcpy(p, s);
		p += strlen(s);
	}
	va_end(ap);

	DELETEA(*str);

	*str = res;

/*
	char *tmp = newstring(d);
	DELETEA(d);
	size_t lt = strlen(tmp);
	size_t ls = strlen(s);
	d = new char[lt +ls + 1];
	strncpy(d, tmp, lt);
	strncpy(&d[lt], s, ls);
	d[lt +ls] = '\0';
	DELETEA(tmp);
	return d;*/

}

void concatpstring(char** str, const char *piece) {
	return concatpstring(str, 1, piece);
}

// Write permbans to disk
SVAR(banfile, "permbans.cfg");

void loadbans()
{
    execfile(banfile);
}

void writebans()
{
    const char *fname = findfile(banfile, "w");
    stream *f = openutf8file(fname, "w");

    if(f)
    {
        string maskedip;

        f->printf("// This file was generated automaticaly\n// Do not edit it while server running\n\n");
/*
        loopv(permbans)
        {
            permban b = permbans[i];
            ip.i = b.ip;
            maskedip[0] = '\0';

            // generate masked ip (ex. 234.345.45.2/32)
            enet_uint32 tmsk = ntohl(b.mask);
            int cidrmsk = 0;
            while(tmsk)
            {
                tmsk <<= 1;
                cidrmsk++;
            }
            formatstring(maskedip)("%i.%i.%i.%i/%i", ip.b[0], ip.b[1], ip.b[2], ip.b[3], cidrmsk);

            f->printf("permban %s \"%s\"\n", maskedip, b.reason);
        }
        */
        loopv(bm->localbanlist()->bans)
        {
            remod::banlist::baninfo *b = bm->localbanlist()->bans[i];
            maskedip[0] = '\0';

            // generate masked ip (ex. 234.345.45.2/32)
            enet_uint32 tmsk = ntohl(b->mask);
            int cidrmsk = 0;
            while(tmsk)
            {
                tmsk <<= 1;
                cidrmsk++;
            }
            formatstring(maskedip)("%i.%i.%i.%i/%i", b->ipoctet[0], b->ipoctet[1], b->ipoctet[2], b->ipoctet[3], cidrmsk);
            f->printf("permban %s \"%s\"\n", maskedip, b->reason);
        }

        f->close();
    }
    else
    {
        conoutf("Can not open \"%s\" for writing bans", fname);
    }
}

// hopmod entitys
//--------------------
// MAPENTS
// uint CRC
// int numents
//     numents entytys
//--------------------
// (c) 2011 Thomas
bool loadents(const char *fname, vector<entity> &ents, uint *crc)
{
    string mapname, ogzname, entsname;
    copystring(mapname, fname, 100);
    cutogz(mapname);
    formatstring(ogzname)("%s/%s.ogz", remod::mapdir, mapname);
    formatstring(entsname)("%s/%s.ents", remod::mapdir, mapname);
    path(ogzname);
    path(entsname);

    // if map exists on server
    // use vanilla server ents loader
    if(fileexists(ogzname, "rb"))
        return ::loadents(fname, ents, crc);

    // if we don't have full map
    // use hopmod short entyty files
    stream *f = opengzfile(path(entsname), "r+b");
        if (!f) return false;

        if (f->getchar() != 'M' || f->getchar() != 'A' || f->getchar() != 'P' ||
            f->getchar() != 'E' || f->getchar() != 'N' || f->getchar() != 'T' || f->getchar() != 'S')
        {
            delete f;
            return false;
        }

        *crc = f->get<uint>();
        int elen = f->get<int>();

        if (f->get<int>() != 0)
        {
            delete f;
            return false;
        }

        loopi(elen)
        {
            entity e;
            e.type  = f->get<uchar>();
            e.attr1 = f->get<short>();
            e.attr2 = f->get<short>();
            e.attr3 = f->get<short>();
            e.attr4 = f->get<short>();
            e.attr5 = f->get<short>();
            e.reserved = f->get<uchar>();
            loopk(3) e.o[k] = f->get<float>();

            ents.add(e);

            if (f->getlil<int>() != 0)
            {
                ents.shrink(0);
                delete f;
                return false;
            }
        }

        if (f->get<int>() != 0 || f->get<int>() != elen)
        {
            ents.shrink(0);
            delete f;
            return false;
        }

        delete f;
        return true;
}

bool writeents(const char *mapname, vector<entity> &ents, uint mapcrc)
{
    string file;
    formatstring(file)("mapinfo/%s.ents", mapname);

    stream *mapi = opengzfile(path(file), "w+b");

    if (!mapi) return false;

    mapi->putstring("MAPENTS");
    mapi->put(mapcrc);
    mapi->put(ents.length());
    mapi->put(0);

    loopv(ents)
    {
        entity &e = ents[i];

        mapi->put(e.type);
        mapi->put(e.attr1);
        mapi->put(e.attr2);
        mapi->put(e.attr3);
        mapi->put(e.attr4);
        mapi->put(e.attr5);
        mapi->put(e.reserved);
        loopk(3) mapi->put(e.o[k]);

        mapi->putlil(0);
    }

    mapi->put(0);
    mapi->put(ents.length());

    mapi->close();
    delete mapi;
    return true;
}

// set client's privelege
void setmaster(clientinfo *ci, int priv)
{
    if(!ci || ci->privilege == priv) return;

    string msg;
    const char *name = "";

    priv = clamp(priv, (int)PRIV_NONE, (int)PRIV_ADMIN);
    if(ci->privilege != PRIV_NONE)
    {
        name = privname(ci->privilege);
        formatstring(msg)("%s relinquished %s", colorname(ci), name);
        sendservmsg(msg);
        remod::onevent(ONSETMASTER, "iisss", ci->clientnum, 0, "", "", "");
    }

    ci->privilege = priv;

    // check if anyone have priveledge
    bool hasmaster = false;
    bool modechanged = false;
    loopv(clients) if(clients[i]->local || clients[i]->privilege >= PRIV_MASTER) hasmaster = true;
    if(!hasmaster)
    {
        mastermode = MM_OPEN;
        allowedips.shrink(0);
        modechanged = true;
    }

    // send list of priveledges
    packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
    putint(p, N_CURRENTMASTER);
    putint(p, mastermode);
    loopv(clients) if(clients[i]->privilege >= PRIV_MASTER)
    {
        putint(p, clients[i]->clientnum);
        putint(p, clients[i]->privilege);
    }
    putint(p, -1);
    sendpacket(-1, 1, p.finalize());

    if(modechanged) remod::onevent(ONMASTERMODE, "ii", -1, mastermode);

    // check if client get any privelge
    if(ci->privilege != PRIV_NONE)
    {
        name = privname(ci->privilege);
        formatstring(msg)("%s claimed %s", colorname(ci), name);
        sendservmsg(msg);
        remod::onevent(ONSETMASTER, "iisss", ci->clientnum, ci->privilege, "", "", "");
    }

    checkpausegame();
}

// wepon accuracy
int getwepaccuracy(int cn, int gun)
{
    int acc = 0;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci && gun>0 && gun<=NUMGUNS)
        acc = ci->state.ext.guninfo[gun].damage*100/max(ci->state.ext.guninfo[gun].shotdamage, 1);
    return(acc);
}

// current mutemode
VAR(mutemode, 0, 0, NUMMUTEMODE-1);

// return 1 if player can't talk, 0 if allowed
bool checkmutemode(clientinfo *ci)
{
    bool muted = false;
    switch(mutemode)
    {
    case MUTEMODE_SPECS:
        {
            if(ci->state.state == CS_SPECTATOR && !ci->privilege)
            {
                muted = true;
            }
            break;
        }
    case MUTEMODE_PLAYERS:
        {
            if(!ci->privilege)
            {
                muted = true;
            }
            break;
        }
    case MUTEMODE_MASTERS:
        {
            if(ci->privilege != PRIV_ADMIN)
            {
                muted = true;
            }
            break;
        }

    case MUTEMODE_NONE:
    default:
        break;
    }
    return muted;
}

// delay in seconds before unpause game
VAR(resumedelay, 0, 0, 100);

int resumetime;
_VAR(resumetimer, resumetimer, 0, 0, 1, IDF_READONLY);
void pausegame(bool val, clientinfo *ci)
{
    // pause game if val = 1
    if(val)
    {
        resumetimer = 0;
        server::pausegame(val, ci);
    }
    else
    {
        if(resumedelay == 0)
        {
            server::pausegame(val, ci);
            return;
        }

        if(resumetimer || !server::gamepaused) return;

        // start resume game counter
        resumetime = totalmillis + resumedelay*1000;
        resumetimer = 1;
        onevent(ONRESUMEGAME, "i", ci ? ci->clientnum : -1);
    }
}

void checkresume()
{
    if(!server::gamepaused || (resumetimer == 0)) return;

    // unpause game
    if(resumetime <= totalmillis)
    {
        resumetimer = 0;
        server::pausegame(false);
    }
}

int getteamscore(const char *team)
{
    int score = 0;
    if(smode && smode->hidefrags())
    {
        score = smode->getteamscore(team);
    }
    else
    {
        loopv(clients)
        if(clients[i]->state.state != CS_SPECTATOR && clients[i]->team[0])
        {
            clientinfo *ci = clients[i];
            if(ci && (strcmp(team, ci->team) == 0))
                score += ci->state.frags;
        }
    }
    return score;
}

bool isteamsequalscore()
{
    int goodscore = getteamscore("good");
    int evilscore = getteamscore("evil");
    return (goodscore == evilscore);
}

void rename(int cn, const char* name)
{
    // don't rename bots
    if(cn < 0 || cn >= 128) return;

    clientinfo *ci = (clientinfo *)getinfo(cn);

    // invalid cn or empty new name
    if(!ci || name == NULL || name[0] == 0) return;

    char newname[MAXNAMELEN + 1];
    newname[MAXNAMELEN] = 0;
    filtertext(newname, name, false, MAXNAMELEN);

    putuint(ci->messages, N_SWITCHNAME);
    sendstring(newname, ci->messages);

    vector<uchar> buf;
    putuint(buf, N_SWITCHNAME);
    sendstring(newname, buf);

    packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
    putuint(p, N_CLIENT);
    putint(p, ci->clientnum);
    putint(p, buf.length());
    p.put(buf.getbuf(), buf.length());
    sendpacket(ci->clientnum, 1, p.finalize(), -1);

    string oldname;
    copystring(oldname, ci->name);
    copystring(ci->name, newname);

    remod::onevent(ONSWITCHNAME, "iss", ci->clientnum, ci->name, oldname);
}

static void freegetmap(ENetPacket *packet)
{
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->getmap == packet) ci->getmap = NULL;
    }
}

void sendmapto(int cn)
{
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(!ci || !m_edit) return;

    if(!mapdata) conoutf("no map to send to %s(%i)", ci->name, cn);
    else if(ci->getmap) conoutf("already sending map to %s(%i)", ci->name, cn);
    else
    {
        // remod
        remod::onevent(ONGETMAP, "i", cn);

        sendservmsgf("[%s is getting the map]", colorname(ci));
        if((ci->getmap = sendfile(cn, 2, mapdata, "ri", N_SENDMAP)))
            ci->getmap->freeCallback = freegetmap;
        ci->needclipboard = totalmillis ? totalmillis : 1;
    }
}

bool iseditcommand(int type)
{
    switch(type)
    {
        case N_EDITF:
        case N_EDITT:
        case N_EDITM:
        case N_FLIP:
        case N_COPY:
        case N_PASTE:
        case N_ROTATE:
        case N_REPLACE:
        case N_DELCUBE:
        case N_REMIP:
        case N_SENDMAP:
            return true;

        default:
            return false;
    }
}

void ghost(int cn, bool v)
{
    clientinfo *ci = getinfo(cn);
    if(!ci) return;
    ci->state.ext.ghost = v;
    onevent(ONGHOST, "ii", cn, v ? 1 : 0);
}

}
