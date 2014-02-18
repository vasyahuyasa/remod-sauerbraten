/*
* remod:    serverctrl.cpp
* date:     2007
* author:   degrave
*
* additional cubescript functions
*/

#include <time.h>
#include "commandev.h"
#include "commandhandler.h"
#include "fpsgame.h"
#include "remod.h"
#include "banlist.h"

#ifndef WIN32
#include <arpa/inet.h>
#endif

#include "version.inc"

//Remod
extern remod::banlist::banmanager *bm;

namespace remod
{

    using namespace server;

void getname(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo((int)*cn);
    if(ci)
    {
        result(ci->name);
    }
}

void getip(int *pcn) {
	int cn = (int) *pcn;
	if (cn < (getnumclients()) && cn >= 0) {
		in_addr addr;
		addr.s_addr = getclientip(cn);
		char *res = inet_ntoa(addr);
		result(res);
	}
}



void getfrags(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.frags);
    }
}

void getdeaths(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.deaths);
    }
}

void getteamkills(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.teamkills);
    }
}

void getaccuracy(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.damage*100/max(ci->state.shotdamage,1));
    }
}

void getflags(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->state.flags);
    }
}

void version()
{
    string txt;
    formatstring(txt)("Remod %s %s (build %s %s) %s/%s", REMOD_CODENAME, REMOD_VERSION, __DATE__, __TIME__, REMOD_SYSTEM, REMOD_ARCH);
    result(txt);
}

void getteam(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        result(ci->team);
    }
}

void kick(int *pcn, int *pexpire, char *actorname)
{
    int cn = (int)*pcn;
    int expire = (int)*pexpire;
    if (!pexpire || expire<=0) expire = 4*60*60000; //4 hours - default
    expire += totalmillis;  //add current uptime
    remod::oneventi(ONKICK, "ii", -1, cn);
    if(strlen(actorname) == 0) actorname = newstring("console");
    //server::kick(cn, actorname, expire);
    server::addban(cn, actorname, expire);
    uint ip = getclientip(cn);
    server::kickclients(ip);
}

void spectator(int *st, int *pcn)
{
    int spectator = (int)*pcn;
    int val = (int)*st;
    clientinfo *spinfo = (clientinfo *)getclientinfo(spectator); // no bots
    if(!spinfo || (spinfo->state.state==CS_SPECTATOR ? val : !val)) return;

    if(spinfo->state.state!=CS_SPECTATOR && val)
    {
        if(spinfo->state.state==CS_ALIVE) suicide(spinfo);
        if(smode) smode->leavegame(spinfo);
        spinfo->state.state = CS_SPECTATOR;
        spinfo->state.timeplayed += lastmillis - spinfo->state.lasttimeplayed;
        if(!spinfo->local && !spinfo->privilege) aiman::removeai(spinfo);
    }
    else if(spinfo->state.state==CS_SPECTATOR && !val)
    {
        spinfo->state.state = CS_DEAD;
        spinfo->state.respawn();
        spinfo->state.lasttimeplayed = lastmillis;
        aiman::addclient(spinfo);
        if(spinfo->clientmap[0] || spinfo->mapcrc) checkmaps();
    }
    sendf(-1, 1, "ri3", N_SPECTATOR, spectator, val);
    if(!val && !hasmap(spinfo)) rotatemap(true);
    return;
}

void _suicide(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        suicide(ci);
    }
}

void pm(int *pcn, char *msg)
{
    int cn = (int)*pcn;
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->clientnum == cn)
        {
            if(ci->state.aitype != AI_NONE) return;
            sendf(cn, 1, "ris", N_SERVMSG, msg);
        }
    }
}

void saytonormal(char *msg)
{
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->connected && ci->privilege == PRIV_NONE)
        {
            pm(&ci->clientnum, msg);
        }
    }
}

void saytomaster(char *msg)
{
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->connected && ci->privilege == PRIV_MASTER)
        {
            pm(&ci->clientnum, msg);
        }
    }
}

void saytoadmin(char *msg)
{
    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->connected && ci->privilege == PRIV_ADMIN)
        {
            pm(&ci->clientnum, msg);
        }
    }
}

void _mastermode(int *mm)
{
    mastermode = (int)*mm;
    remod::onevent(ONMASTERMODE, "ii", -1, mastermode);
    if(mastermode>=MM_PRIVATE)
    {
        loopv(clients) allowedips.add(getclientip(clients[i]->clientnum));
    }
    sendf(-1, 1, "rii", N_MASTERMODE, mastermode);
}

void setteam(int *pcn, const char *team)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(!ci && !strcmp(ci->team, team)) return;
    remod::onevent(ONSETTEAM, "is", cn, team);
    if(!smode || smode->canchangeteam(ci, ci->team, team))
    {
        if(ci->state.state==CS_ALIVE) suicide(ci);
        copystring(ci->team, team, MAXTEAMLEN+1);
    }
    aiman::changeteam(ci);
    sendf(-1, 1, "riisi", N_SETTEAM, cn, ci->team, 1);
}

void getping(int *pcn)
{
    int cn=(int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(ci->ping);
    }
}

void getonline(int *pcn)
{
    int cn = (int)*pcn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        intret(totalmillis-ci->connectmillis);
    }
}

void _getteamscore(char *team)
{
    if(smode && smode->hidefrags())
    {
        intret(smode->getteamscore(team));
    }
    else
    {
        int teamfrags = 0;
        loopv(clients) if(clients[i]->team[0])
        {
            clientinfo *ci = clients[i];
            if(ci && (strcmp(team, ci->team) == 0)) teamfrags+=ci->state.frags;
        }
        intret(teamfrags);
    }

}

void getrank(int *pcn)
{
    int cn=(int)*pcn;
    vector<clientinfo *> uplayers, splayers; //unsorted sorted

    loopv(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->state.state != CS_SPECTATOR) uplayers.add(ci);
        //conoutf("%i %s[%i] %i frags", i, ci->name, ci->clientnum, ci->state.frags);
    }

    while(uplayers.length()>0)
    {
        clientinfo *ci = findbest(uplayers);
        splayers.add(ci);
    }

    loopv(splayers)
    {
        if(splayers[i]->clientnum == cn)
        {
            intret(i+1);
            return;
        }
    }
    intret(-1);
}

void mute(int *pcn, int *val)
{
    int cn = (int)*pcn;
    bool v = (bool)*val;

    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        if(ci->state.ext.muted != v)
        {
            ci->state.ext.muted = v;
            remod::onevent(ONMUTE, "ii", v ? 1 : 0, cn);
        }
    }
}

void ismuted(int *icn)
{
    int cn = (int)*icn;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    intret(ci && ci->state.ext.muted);
}

void formatmillis(const char *fmt, int *millis)
{
    // %i - milliseconds, %s - seconds, %m - minutes, %h - hours, %d - days
    int mseconds, seconds, minutes, hours, days;
    vector<char> s;

    days = (int)*millis/(1000*60*60*24);
    hours = ((int)*millis/(1000*60*60))-(days*24);
    minutes = ((int)*millis/(1000*60))-(days*24*60+hours*60);
    seconds = ((int)*millis/1000)-(days*24*60*60+hours*60*60+minutes*60);
    mseconds  = *millis - (days*24*60*60+hours*60*60+minutes*60+seconds)*1000;

    while(*fmt)
    {
        int c = *fmt++;
        if(c == '%')
        {
            int i = *fmt++;
            switch(i)
            {
            case 'i':
            {
                const char *smseconds = intstr(mseconds);
                while(*smseconds) s.add(*smseconds++);
                break;
            }

            case 's':
            {
                const char *sseconds = newstring(2);
                sprintf((char*)sseconds, "%02d", seconds);
                while(*sseconds) s.add(*sseconds++);
                // xxx DELETEA(sseconds);
                break;
            }

            case 'm':
            {
                const char *sminutes = newstring(2);
                sprintf((char*)sminutes, "%02d", minutes);
                while(*sminutes) s.add(*sminutes++);
                // xxx DELETEA(sminutes);
                break;
            }

            case 'h':
            {
                const char *shours = newstring(2);
                sprintf((char*)shours, "%02d", hours);
                while(*shours) s.add(*shours++);
                // xxx DELETEA(shours);
                break;
            }

            case 'd':
            {
                const char *sdays;
                sdays = intstr(days);
                while(*sdays) s.add(*sdays++);
                break;
            }

            default:
                s.add(i);
            }
        }
        else s.add(c);
    }
    s.add('\0');
    result(s.getbuf());
}


// based on looplist (command.cpp)
void loopbans(ident *name, ident *ip, ident *expire, ident *actor, ident *actorip, const uint *body)
{
    ident *idents[5];
    identstack stack[5];

    in_addr addr_victim;
    in_addr addr_actor;

    idents[0] = name;
    idents[1] = ip;
    idents[2] = expire;
    idents[3] = actor;
    idents[4] = actorip;

    loopv(bannedips)
    {
        ban &b = bannedips[i];

        addr_victim.s_addr  = b.ip;
        addr_actor.s_addr   = b.actorip;

        if(i)
        {
            // set ident values
            loopi(5)
            {
                if(idents[i]->valtype == VAL_STR) delete[] idents[i]->val.s;
                else idents[i]->valtype = VAL_STR;
                cleancode(*idents[i]);
            }

            idents[0]->val.s = newstring(b.name);
            idents[1]->val.s = newstring(inet_ntoa(addr_victim));
            idents[2]->val.s = newstring(intstr(b.expire));
            idents[3]->val.s = newstring(b.actor);
            idents[4]->val.s = newstring(inet_ntoa(addr_actor));
        }
        else
        {
            // init idents
            tagval t[5];

            t[0].setstr(newstring(b.name));
            t[1].setstr(newstring(inet_ntoa(addr_victim)));
            t[2].setstr(newstring(intstr(b.expire)));
            t[3].setstr(newstring(b.actor));
            t[4].setstr(newstring(inet_ntoa(addr_actor)));

            ::pusharg(*idents[0], t[0], stack[0]);
            ::pusharg(*idents[1], t[1], stack[1]);
            ::pusharg(*idents[2], t[2], stack[2]);
            ::pusharg(*idents[3], t[3], stack[3]);
            ::pusharg(*idents[4], t[4], stack[4]);

            idents[0]->flags &= ~IDF_UNKNOWN;
            idents[1]->flags &= ~IDF_UNKNOWN;
            idents[2]->flags &= ~IDF_UNKNOWN;
            idents[3]->flags &= ~IDF_UNKNOWN;
            idents[4]->flags &= ~IDF_UNKNOWN;
        }
        execute(body);
    }

    if(bannedips.length())
    {
        loopi(5)
        {
            ::poparg(*idents[i]);
        }
    }
}

/**
 * converts integer unix time to string due to format.
 */
void timef(int *t, const char *format) {
	time_t now = *t;
	struct tm *timeinfo;
	string buf;

	timeinfo = localtime(&now);
	strftime(buf, MAXSTRLEN, format, timeinfo);
	result(buf);
}

// system time format see http://www.cplusplus.com/reference/clibrary/ctime/strftime/
void systimef(const char *format)
{
    time_t now;
    struct tm *timeinfo;
    string buf;

    time(&now);
    timeinfo = localtime(&now);
    strftime(buf, MAXSTRLEN, format, timeinfo);
    result(buf);
}

// directory with maps for load
SVAR(mapdir, "");

// load map from file system
// name - map name without extension
void loadmap(const char *name)
{
    if(!m_edit) return;

    string fname = "";
    string buf = "";
    copystring(buf, mapdir);
    if(mapdir[0])
    {
        int slen = strlen(buf);
        if(buf[slen] != '/' && buf[slen] != '\\' && slen+1 < (int)sizeof(buf)) { buf[slen] = '/'; buf[slen+1] = '\0'; }
    }
    formatstring(fname)("%s%s.ogz", buf, name);

    if(server::mapdata) DELETEP(server::mapdata);
    server::mapdata = openfile(fname, "rb");

    // status message
    string msg;
    if(!server::mapdata)
    {
        formatstring(msg)("[failed to open %s, map do not exist on the server]", name);
    }
    else
    {
        formatstring(msg)("[map %s was uploaded to server, \"/getmap\" to receive it]", name);
    }
    sendservmsg(msg);
}

void savemap(const char *name)
{
    if(!m_edit) return;

    stream *data;
    string fname = "";
    string buf = "";
    copystring(buf, mapdir);
    if(mapdir[0])
    {
        int slen = strlen(buf);
        if(buf[slen] != '/' && buf[slen] != '\\' && slen+1 < (int)sizeof(buf)) { buf[slen] = '/'; buf[slen+1] = '\0'; }
    }
    formatstring(fname)("%s%s.ogz", buf, name);

    // status message
    string msg;

    if(server::mapdata)
    {
        data = openfile(fname, "wb");
        if(!data)
        {
            formatstring(msg)("[failed to open %s for writing]", fname);
        }
        else
        {
            // copy data
            long len = server::mapdata->size();
            uchar fbuf[len];

            server::mapdata->seek(0, SEEK_SET);
            server::mapdata->read(fbuf, len); // copy file to buffer
            data->seek(0, SEEK_SET);
            data->write(fbuf, len); // write buffer to file

            // close file
            data->close();
            DELETEP(data);
            formatstring(msg)("[map %s was saved]", name);
        }
    }
    else
    {
        formatstring(msg)("[no map to save]");
    }
    sendservmsg(msg);
}

// from client.cpp
void listclients()
{
    vector<char> buf;
    string cn;
    int numclients = 0;
    loopv(clients) if(clients[i])
    {
        formatstring(cn)("%d", clients[i]->clientnum);
        if(numclients++) buf.add(' ');
        buf.put(cn, strlen(cn));
    }
    buf.add('\0');
    result(buf.getbuf());
}

void getvalue(const char* ident, const char* def) {
	const char *alias = getalias(ident);
	result(alias && strcmp("", alias) ? alias : def);
}

void editmute(int *pcn, int *val)
{
    int cn = (int)*pcn;
    bool v = (bool)*val;

    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        if(ci->state.ext.editmuted != v)
        {
            ci->state.ext.editmuted = v;
            remod::onevent(ONEDITMUTE, "ii", v ? 1 : 0, cn);
        }
    }
}

void iseditmuted(int *cn)
{
    clientinfo *ci = (clientinfo *)getinfo(*cn);
    intret(ci && ci->state.ext.editmuted);
}

void uptimef(const char *fmt)
{
    // max correct uptime 134 years
    // %s - seconds, %m - minutes, %h - hours, %d - days, %y
    uint seconds, minutes, hours, days, years;
    vector<char> s;

    years = totalsecs/(60*60*24*365); // dont count leap year
    days = (totalsecs/(60*60*24))%365;
    hours = (totalsecs/(60*60))%24;
    minutes = (totalsecs/(60))%60;
    seconds = totalsecs%60;

    while(*fmt)
    {
        int c = *fmt++;
        if(c == '%')
        {
            int i = *fmt++;
            switch(i)
            {
                case 's':
                {
                    const char *sseconds = newstring(2);
                    sprintf((char*)sseconds, "%02d", seconds);
                    while(*sseconds) s.add(*sseconds++);
                    // xxx DELETEA(sseconds);
                    break;
                }

                case 'm':
                {
                    const char *sminutes = newstring(2);
                    sprintf((char*)sminutes, "%02d", minutes);
                    while(*sminutes) s.add(*sminutes++);
                    // xxx DELETEA(sminutes);
                    break;
                }

                case 'h':
                {
                    const char *shours = newstring(2);
                    sprintf((char*)shours, "%02d", hours);
                    while(*shours) s.add(*shours++);
                    // xxx DELETEA(shours);
                    break;
                }

                case 'd':
                {
                    const char *sdays;
                    sdays = intstr(days);
                    while(*sdays) s.add(*sdays++);
                    break;
                }

                case 'y':
                {
                    const char *syears;
                    syears = intstr(years);
                    while(*syears) s.add(*syears++);
                    break;
                }

                default:
                    s.add(i);
            }
        }
        else s.add(c);
    }
    s.add('\0');
    result(s.getbuf());
}

/**
 * converts string ip to integer value
 */
int ip2int(char *ip) {
	int i = 0;
	for (int j = 0; j < 4; j++) {
		short ioctet;
		if (!ip) {
			ioctet = 0xFF;
		} else {
			char *dot = strchr(ip, '.');

			size_t l = dot ? ((size_t) dot - (size_t) ip) : strlen(ip);
			char *octet = newstring(ip, l);

			ioctet = atoi(octet);
			if (ioctet == 0 && strcmp(octet, "0")) {
				ioctet = 0xFF;
			}

			ip = dot ? dot + 1 : 0;
			DELETEA(octet);
		}
		i = (i << 8) | ioctet;
	}
	return i;
}

/**
 * convert integer value of ip to string
 */
void int2ip(int *i) {
	string ip;
	int ii = *i;
	sprintf(ip, "%d.%d.%d.%d", (ii & 0xFF000000) >> 24, (ii & 0xFF0000) >> 16, (ii & 0xFF00) >> 8, ii & 0xFF);
	result(ip);
}



/**
 * checks if ip matches mask
 * ip = "127.0.0.1",	mask = "127.0.0.1"		--  true
 * ip = "192.168.1.23",	mask = "192.168.1.*"	--  true
 * ip = "192.168.1.23",	mask = "192.168.1.255"	--  true
 * ip = "192.168.1.23",	mask = "192.168.1."		--  true
* ip = "192.168.1.23",	mask = "192.168."		--  true
 * ip = "127.0.0.1",	mask = "128.0.0.1"		--  false
 */
bool checkipbymask(char *ip, char *mask)
{
	int ipint = ip2int(ip);
	int maskint = ip2int(mask);

	for(int i = 3; i >= 0; i--) {
		short ip_octet = (ipint >> i*8) & 0xFF;
		short mask_octet = (maskint >> i*8) & 0xFF;
		if (mask_octet != 0xFF && (mask_octet != ip_octet)) {
			return false;
		}
	}
	return true;
}


void looppermbans(ident *ip, ident *mask, ident *reason, const uint *body)
{
    identstack stack[3];
    in_addr addr_ip;
    in_addr addr_mask;

    loopv(bm->localbanlist()->bans)
    {
        remod::banlist::baninfo *b = bm->localbanlist()->bans[i];
        addr_ip.s_addr   = b->ip;
        addr_mask.s_addr = b->mask;

        if(i)
        {
            loopi(3)
            {
                if(ip->valtype == VAL_STR) delete[] ip->val.s;
                else ip->valtype = VAL_STR;

                if(mask->valtype == VAL_STR) delete[] mask->val.s;
                else mask->valtype = VAL_STR;

                if(reason->valtype == VAL_STR) delete[] reason->val.s;
                else reason->valtype = VAL_STR;

                ::cleancode(*ip);
                ::cleancode(*mask);
                ::cleancode(*reason);

                ip->val.s     = newstring(inet_ntoa(addr_ip));
                mask->val.s   = newstring(inet_ntoa(addr_mask));
                reason->val.s = newstring(b->reason);
            }
        }
        else
        {
            // init idents
            tagval t[3];

            t[0].setstr(newstring(inet_ntoa(addr_ip)));
            t[1].setstr(newstring(inet_ntoa(addr_mask)));
            t[2].setstr(newstring(b->reason));

            ::pusharg(*ip    , t[0], stack[0]);
            ::pusharg(*mask  , t[1], stack[1]);
            ::pusharg(*reason, t[2], stack[2]);

            ip    ->flags &= ~IDF_UNKNOWN;
            mask  ->flags &= ~IDF_UNKNOWN;
            reason->flags &= ~IDF_UNKNOWN;
        }

        execute(body);
    }

    if(bm->localbanlist()->length())
    {
        ::poparg(*ip);
        ::poparg(*mask);
        ::poparg(*reason);
    }
}


// Shitcode below, get list of compiled in extensions
void getextensions()
{
    const extensionslist *extensions = getextensionslist();

    vector<char> buf; // damned vector
    string ext;
    int numext = 0;
    for(int i = 0; i<extensions->length(); i++) if(extensions->getbuf()[i]) // :`( - ugly
    {
        formatstring(ext)("%s", extensions->getbuf()[i]);
        if(numext++) buf.add(' ');
        buf.put(ext, strlen(ext));
    }
    buf.add('\0');
    result(buf.getbuf());
}



//comparator for sortlist
static inline int sortlist_compare(char **x, char **y) {
	return strcmp(*x, *y);
}

struct keyvalue {
	char *key;
	char *value;
};

//comparator for sorttwolists
static inline int sorttwolists_compare(const void *x, const void *y) {
	return strcmp(((keyvalue*) x)->key, ((keyvalue*) y)->key);
}

/**
 * Rearranges items in values list to have the same order as sorted items in keys
 */
void sorttwolists(char *keys, char *values) {
	vector<char*> keys_list;
	explodelist(keys, keys_list);

	vector<char*> values_list;
	explodelist(values, values_list);



	int len = (keys_list.length() > values_list.length()) ? values_list.length() : keys_list.length();
	keyvalue kv[len];

	loopi(len) {
		kv[i].key = keys_list[i];
		kv[i].value = values_list[i];
	}

	qsort(kv, (unsigned int) len, sizeof (keyvalue), sorttwolists_compare);


	vector<char*> res;
	loopi(len) {
		res.add(kv[i].value);
	}

	char *r = conc(res.buf, res.length(), true);
	result(r);
	DELETEA(r);

	loopv(keys_list) {
		DELETEA(keys_list[i]);
	}
	loopv(values_list) {
		DELETEA(values_list[i]);
	}
}

void setclientvar(int cn, const char *key, char *value)
{
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        //ci->state.vars.access(key, val);
        //ci->vars[newstring(key)] = newstring(value);
        ci->ext.vars.set(key, value);
    }
}

void getclientvar(int cn, const char *key)
{
    /*
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        char **pval = ci->state.vars.access(key);
        if(pval)
        {
            DELETEP(val);
            val = *pval;
        }

        char *val;
        val = ci->vars[key];
        conoutf("val=%u", val);
        if(val && val[0]) result(val);
    }
    */

    char *res = NULL;
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        res = ci->ext.vars.get(key);
    }

    if(res)
    {
        result(res);
    }
    else
    {
        result("");
    }
}

void listclientvar(int cn)
{
    clientinfo *ci = (clientinfo *)getinfo(cn);
    if(ci)
    {
        //enumerate(ci->vars, char *, val, conoutf("vars[x]=%s", val));
        loopi(VARBOXMAXNODES)
        {
            if(ci->ext.vars.nodes[i])
            {
                conoutf("vars[%s] = %s", ci->ext.vars.nodes[i]->key, ci->ext.vars.nodes[i]->val);
            }
            else
            {
                break;
            }
        }
    }
}

void resetteamkills(int *cn)
{
    clientinfo *ci = (clientinfo *)getclientinfo(*cn);
    if(ci)
    {
        ci->state.teamkills = 0;
        uint ip = getclientip(*cn);
        loopv(teamkills) if(teamkills[i].ip == ip)
        {
            teamkills[i].teamkills = 0;
            return;
        }
    }
}

void escapeitems(tagval *v, int n, int start) {
	for (int i = start; i < n; i++)
	    {
	    	if (v[i].type == VAL_STR) {
	    		char *s = v[i].s;
	    		v[i].s = newstring(escapestring(s));
	    		DELETEA(s);
	    	}
	    }
}

void listconcat(tagval *v, int n) {
	escapeitems(v, n, 0);
	concat(v, n);
}

void listadd(tagval *v, int n) {
	escapeitems(v, n, 1);
	concat(v, n);
}

char* unescape(char *s)
{
    int len = strlen(s);
    char *d = newstring(len);
    d[unescapestring(d, s, &s[len])] = '\0';
    return d;
}

void listat(tagval *args, int numargs)
{
	at(args, numargs);

	char *res = commandret->s;
    commandret->setstr(unescape(res));
    DELETEA(res);
}

// set privelege PRIV_NONE, PRIV_MASTER, PRIV_ADMIN
void setpriv(int *cn, char *s)
{
    clientinfo *ci = (clientinfo *)getclientinfo(*cn);
    if(!ci) return;

    int priv = PRIV_NONE;

    if(isdigit(s[0]))
    {
        priv = clamp(parseint(s), (int)PRIV_NONE, (int)PRIV_ADMIN);
    }
    else
    {
        switch(s[0])
        {
            case 'n': priv = PRIV_NONE; break;
            case 'm': priv = PRIV_MASTER; break;
            case 'a': priv = PRIV_ADMIN; break;
            default: priv = PRIV_NONE;
        }
    }

    if(priv == PRIV_AUTH) priv = PRIV_MASTER;

    remod::setmaster(ci, priv);
}

void getpos(int *cn)
{
    clientinfo *ci = (clientinfo *)getclientinfo(*cn);
    if(!ci || ci->state.state == CS_SPECTATOR) return;
    defformatstring(pos)("%s %s %s", floatstr(ci->state.o.x), floatstr(ci->state.o.y), floatstr(ci->state.o.z));
    result(pos);
}

/**
 * Set client personal variable for session (limit of variables is 128)
 * @group player
 * @arg1 client number
 * @arg2 key
 * @arg3 value
 */
ICOMMAND(setvar, "iss", (int *cn, const char *key, char *value), setclientvar(*cn, key, value));

/**
 * Get client personal variable
 * @group player
 * @arg1 client number
 * @arg2 key
 * @return value
 */
ICOMMAND(getvar, "is", (int *cn, const char *key), getclientvar(*cn, key));

/**
 * Print all defined client variables
 * @group player
 * @return print list to console, can not be used in scripts
 */
ICOMMAND(listvar, "i", (int *cn), listclientvar(*cn));

//Cube script binds

/**
 * Get name of player
 * @group player
 * @arg1 client number
 */
COMMAND(getname, "i");

/**
 * Return current map name
 * @group server
 */
ICOMMAND(getmap, "", (), result(smapname));

/**
 * Get current mode
 * @group server
 * @return number of current mode. See $MODENAMES to get string
 * @example (at $MODENAMES (getmode))
 */
ICOMMAND(getmode, "", (), intret(gamemode));

/**
 * Get player's ip as integer (see ip2int f converting to string)
 * @group server
 * @arg1 client number
 * @return integer ip
 * @example ip2int (getip $cn)
 */
COMMAND(getip, "i");

/**
 * Get frags count of player
 * @group player
 * @arg1 client number
 */
COMMAND(getfrags, "i");

/**
 * Get deaths count of player
 * @group player
 * @arg1 client number
 */
COMMAND(getdeaths, "i");

/**
 * Get count of player's teamkills
 * @group player
 * @arg1 client number
 */
COMMAND(getteamkills, "i");

/**
 * Get accuracy of player
 * @group player
 * @arg1 client number
 */
COMMAND(getaccuracy, "i");

/**
 * Get count of flags scored by player
 * @group player
 * @arg1 client number
 */
COMMAND(getflags, "i");
//COMMAND(getretflags, "i"); unimplemented

/**
 * Get current mastermode as number
 * @group server
 * @return 0 (public) or 1 (vote) or 2 (locked) or 3 (private)
 */
ICOMMAND(getmastermode, "", (), intret(mastermode));

/**
 * Get current mastermode's name
 * @group server
 * @return "public" or "vote" or "locked" or "private"
 */
ICOMMAND(getmastermodename, "i", (int *mm), result(mastermodename((int)*mm, "unknown")));

/**
 * Check if specified player is master
 * @group player
 * @arg1 client number
 * @return 0 or 1
 */
ICOMMAND(ismaster, "i", (int*cn), intret(ismaster(cn) ? 1 : 0));

/**
 * Check if specified player is admin
 * @group player
 * @arg1 client number
 * @return 0 or 1
 */
ICOMMAND(isadmin, "i", (int *cn), intret(isadmin(cn) ? 1 : 0));

/**
 * Check if specified player is spectator
 * @group player
 * @arg1 client number
 * @return 0 or 1
 */
ICOMMAND(isspectator, "i", (int *cn), intret(isspectator(cn) ? 1 : 0));

/**
 * Check if specified player in edit mode
 * @group player
 * @arg1 client number
 * @return 0 or 1
 */
ICOMMAND(isediting, "i", (int *cn), intret(isediting(cn) ? 1 : 0));

/**
 * Get this server's version
 * @group server
 */
COMMAND(version, "");

/**
 * Get team of specified player
 * @group player
 * @arg1 client number
 * @return mostly "good" or "evil"
 */
COMMAND(getteam,"i");

/**
 * Disconnect specified player
 * @group player
 * @arg1 client number
 */
ICOMMAND(disconnect, "i", (int *cn), disconnect_client(*cn, DISC_NONE));

/**
 * Kick player for specified time
 * @group player
 * @arg1 client number
 * @arg2 ban's time in seconds
 * @arg3 actor name (who kicked, to display in logs)
 * @example kick "0" "100500" "irc-bot"
 */
COMMAND(kick, "iis");

/**
 * Spectate or unspectate player
 * @group player
 * @arg1 1 to spectate, 0 to unspectate
 * @arg2 client number
 */
COMMAND(spectator, "ii");

/**
 * Change map
 * @group server
 * @arg1 map name
 */
ICOMMAND(map, "s", (char *name), sendf(-1, 1, "risii", N_MAPCHANGE, name, gamemode, 1); server::changemap(name, gamemode));

/**
 * Change map and mode
 * @group server
 * @arg1 map name
 * @arg2 mode number (see: getmode and $MODENAMES)
 */
ICOMMAND(mapmode, "si", (char *name, int *mode), sendf(-1, 1, "risii", N_MAPCHANGE, name, *mode, 1); server::changemap(name, *mode));

/**
 * Force player to hate himself and to want to die
 * @group player
 * @arg1 client number
 */
COMMANDN(suicide, _suicide, "i");

/**
 * Add bot wih specified skills level
 * @group server
 * @arg1 skills level
 * @example addbot "120" // yeah, try to beat it!
 */
ICOMMAND(addbot, "i", (int *s), aiman::addai(*s, -1));

/**
 * Delete last added bot
 * @group server
 */
ICOMMAND(delbot, "", (), aiman::deleteai());

/**
 * Send server message (for all players to see)
 * @group server
 * @arg1 message
 */
ICOMMAND(say, "C", (char *msg), sendservmsg(msg));

/**
 * Send private message to player
 * @group server
 * @arg1 client number
 * @arg2 message
 */
COMMAND(pm, "is");

/**
 * Say message to all unprivileged players
 * @group server
 * @arg1 message
 */
COMMAND(saytonormal, "C");

/**
 * Say message to the player connected as master if he exists
 * @group server
 * @arg1 message
 */
COMMAND(saytomaster, "C");

/**
 * Say message to the player connected as admin if he exists
 * @group server
 * @arg1 message
 */
COMMAND(saytoadmin, "C");

/**
 * Set master mode
 * @group server
 * @arg1 master mode number
 */
COMMANDN(mastermode, _mastermode, "i");

/**
 * is server paused or not
 */
VARF(pause, 0, 0, 1, server::pausegame(pause));

/**
 * Clear all player bans created by /kick or #kick (it's not the same as permbans)
 * @group server
 */
ICOMMAND(clearbans, "", (), remod::onevent(ONCLEARBANS, "i", -1); bannedips.shrink(0); sendservmsg("cleared all bans"));

/**
 * Force the player to specified team
 * @group player
 * @arg1 client number
 * @arg2 team (usually "good" or "evil")
 */
COMMAND(setteam, "is");

/**
 * Get ping of player
 * @group player
 * @arg1 client number
 * @return ping
 */
COMMAND(getping, "i");

/**
 * Get player's connection time
 * @group player
 * @arg1 client number
 * @return number of milliseconds
 */
COMMAND(getonline, "i");

/**
 * Get score of the team
 * @group server
 * @arg1 team
 */
COMMANDN(getteamscores, _getteamscore, "s");

/**
 * Get rank of player (based on his score)
 * @group player
 * @arg1 client number
 * @return rank
 */
COMMAND(getrank, "i");

/**
 * Clear gbans (bans from masterserver)
 * @group server
 */
COMMAND(cleargbans, "");

/**
 * Get count of connected players
 * @group player
 */
ICOMMAND(numclients, "", (), intret(numclients(-1, false, true, false)));

/**
 * Check if player with specified cn exists
 * @group player
 * @arg1 client number
 * @return 0 or 1
 */
ICOMMAND(playerexists, "i", (int *cn), intret(playerexists(cn)));

/**
 * Mute or unmute player
 * @group player
 * @arg1 client number
 * @arg2 "1" for mute, "0" for unmute
 */
COMMAND(mute, "ii");

/**
 * Check if player is muted
 * @group player
 * @arg1 client number
 * @return 1 if muted, 0 if unmuted
 */
COMMAND(ismuted, "i");

/**
 * Format milliseconds. Format string items:
 * - %i - milliseconds
 * - %s - seconds (with leading 0)
 * - %m - minutes (with leading 0)
 * - %h - hours (with leading 0)
 * - %d - days
 * @group server
 * @arg1 format string
 * @arg2 milliseconds
 * @return time as string
 * @example formatmillis "%d days, %h:%m:%s and %i milliseconds" $uptime
 */
COMMAND(formatmillis, "si");

/**
 * Search player by name and return his client number
 * @group player
 * @arg1 player's name
 * @return player's cn or -1 if not found
 * @example getcn "unnamed" // returns cn of the first unnamed it finds
 */
ICOMMAND(getcn, "s", (char *name), int cn = parseplayer(name); intret(cn));

/**
 * Halt the server with specified exit code
 * @group server
 * @arg1 error code
 * @example halt 0
 */
ICOMMAND(halt, "i", (int *err), exit(*err));

/**
 * Check if ip corresponds to ip mask
 * @group server
 * @arg1 ip (string)
 * @arg2 ip mask (string)
 * @return 0 or 1
 * @example checkipbymask "192.168.0.1" "192.168.*.*" //returns 1
 * @example checkipbymask "192.168.0.1" "192.168." //returns 1
 */
ICOMMAND(checkipbymask, "ss", (char *ip, char *mask), intret(checkipbymask(ip, mask) ? 1 : 0));

/**
 * Loop through bans
 * @group server
 * @arg1 banned player name
 * @arg2 player ip
 * @arg3 time of expiration of ban
 * @arg4 actor name
 * @arg5 actor ip
 * @arg6 body of function to loop
 */
ICOMMAND(loopbans,
         "rrrrre",
         (ident *name, ident *ip, ident *expire, ident *actor, ident *actorip, uint *body),
         loopbans(name, ip, expire, actor, actorip, body));

/**
 * Delete ban with specified number (see loopbans)
 * @group server
 * @arg1 number of ban
 */
ICOMMAND(delban, "i", (int *n), if(bannedips.inrange(*n)) bannedips.remove(*n));

/**
 * Convert integer unix time to string due to format
 * @group server
 * @arg1 unix timestamp (number of milliseconds)
 * @arg2 format string (as in C function strftime)
 * @return string
 */
COMMAND(timef, "is");

/**
 * Return formatted system time )see timef)
 * @group server
 * @arg1 format string
 * @return system time string
 */
COMMAND(systimef, "s");

/**
 * Set file to write logs
 * @group server
 * @arg1 file path
 */
COMMAND(setlogfile, "s");

/**
 * Write string to log
 * @group server
 * @arg1 string
 */
ICOMMAND(echo, "C", (char *s), conoutf("%s", s));

/**
 * Load saved from coopedit map
 * @group server
 * @arg1 map name
 */
COMMAND(loadmap, "s");

/**
 * Save current map on server (for coopedit)
 * @group server
 * @arg1 map name
 */
COMMAND(savemap, "s");

/**
 * Returns list of client numbers of connected players
 * @group player
 * @return list of cn
 */
COMMAND(listclients, "");

/**
 * Checks if ident with specified name exists (defined)
 * @group server
 * @arg1 ident name (i.e. $var1)
 * @return 1 if exists else 0
 */
ICOMMAND(identexists, "s", (const char *name), intret(identexists(name)));

/**
 * Returns value of variable with specified name or default value
 * @group server
 * @arg1 ident name (i.e. $var1)
 * @arg2 default value
 * @return $variable if it exists and not empty, otherwise default value
 */
COMMAND(getvalue, "ss");

/**
 * Evaluate string as cube script
 * @group server
 * @arg1 body of function
 * @return returned value
 */
ICOMMAND(eval, "C", (char *s), executeret(s));

/**
 * Ignore specified client changes in coop edit mode
 * @group player
 * @arg1 client number
 * @arg2 1 or 0
 */
COMMAND(editmute, "ii");

/**
 * Check if player is editmuted
 * @group player
 * @arg1 client number
 * @return 1 if editmuted, 0 if not
 */
COMMAND(iseditmuted, "i");

/**
 * Formats server's uptime
 * @group server
 * @arg1 format string
 * @return string
 */
COMMAND(uptimef, "s");

/**
 * Get integer representation of ip (to save it in db for example)
 * @group server
 * @arg1 ip as string (i.e. "192.168.1.1")
 * @return integer
 */
ICOMMAND(ip2int, "s", (char *ip), intret(ip2int(ip)));

/**
 * Transforms integer representation of ip to string
 * @group server
 * @arg1 integer representation of ip
 * @return string ip (i.e. 192.168.1.2)
 */
COMMAND(int2ip, "i");

/**
 * Permanently ban player
 * @group player
 * @arg1 player's name
 * @arg2 reason
 */
COMMANDN(permban, addpban, "ss");

/**
 * Loop permanent bans list
 * @group player
 * @arg1 banned ip variable
 * @arg2 network mask variable
 * @arg3 reason variable
 * @arg4 body of function to execute while iterating the list
 */

ICOMMAND(looppermbans,
         "rrre",
         (ident *ip, ident *mask, ident *reason, uint *body),
         looppermbans(ip, mask, reason, body));

/**
 * Delete permanent ban with specified number
 * @group player
 * @arg1 number of record in permban list
 */
ICOMMAND(delpermban, "i", (int *n), bm->localbanlist()->remove(*n));

/**
 * Get list of compiled extensions in remod
 * @group server
 * @return list of extensions as string
 */
COMMANDN(getextensions, getextensions, "");

/**
 * Save permbans, writting them to file
 * @group server
 */
COMMAND(writebans, "");

/**
 * Returns values list items in the same order as sorted keys
 * @group server
 * @arg1 keys list
 * @arg2 values list
 * @return rearranged values list
 * @example sorttwolists "2 1 4 5" "b a q k" // returns "a b q k"
 */
COMMANDN(sorttwolists, sorttwolists, "ss");

/**
* Reload authkeys (using $authfile variable)
* @group server
*/
COMMAND(reloadauth, "");

/**
* Change game speed, can be used as variable
* @group server
*/
ICOMMAND(gamespeed, "iN$", (int *val, int *numargs, ident *id),
    {
        if(*numargs > 0) forcegamespeed(clampvar(id, *val, 10, 1000));
        else if(*numargs < 0) intret(server::gamespeed);
        else printvar(id, server::gamespeed);
    });

/**
 * Reset client teamkill counter
 * @group server
 * @arg1 client number
 */
COMMAND(resetteamkills, "i");


/**
 * Concat arguments in a list with escaping cubescript special characters in variables (concat+escape)
 * @group server
 * @arg1 .. argN  arguments
 * @return list with escaped items
 * @example  a = (listconcat "b" "c")
 */
COMMAND(listconcat, "V");

/**
 * Concat arguments in a list with escaping cubescript special characters in variables (concat+escape)
 * @group server
 * @arg1 list
 * @arg2 index
 * @return unescaped list item
 * @example  i = (listat $l "1")
 */
COMMAND(listat, "si1V");

/**
 * Add escaped elements to the list
 * @group server
 * @arg1 list
 * @arg2 .. argN  arguments
 * @return list with escaped items
 * @example  a = (listadd $a "b" "c")
 */
COMMAND(listadd, "V");

/**
 * Set client's privelege
 * @group server
 * @arg1 cn
 * @arg2 privelege: 0 or n - NONE, 1 or 2 or m - MASTER, 3 or a - ADMIN
 */
COMMAND(setpriv, "is");

/**
 * Get client position
 * @group player
 * @arg1 cn
 * @return x,y,z coordinates in float format, return empty if client not exist or spectator
 */
COMMAND(getpos, "i");

/**
 * Get specified weapon accuracy
 * @group player
 * @arg1 client number
 * @arg2 gun ("FI" = 0, "SG", "CG", "RL", "RI", "GL", "PI" = 6)
 */
ICOMMAND(getwepaccuracy, "ii", (int *cn, int *gun), intret(getwepaccuracy(*cn, *gun)));
}
