#ifndef __COMMANDEV_H__
#define __COMMANDEV_H__

#include "fpsgame.h"

enum eventType { ONADDBOT = 0, ONBOTLIMIT, ONCLEARBANS, ONCLEARDEMOS, ONCOMMAND,
                 ONCOMMANDPERMERROR, ONCOMMANDUNKNOWN, ONCOMMANDUSAGEERROR,
                 ONCONNECT, ONDEATH, ONDELBOT, ONDISCONNECT, ONDROPFLAG,
                 ONEDITMODE, ONGHOST, ONGHOSTTRIGGER,ONFRAG, ONGETDEMO, ONGETMAP, ONRECEIVEMAP, ONIMISSION, ONOVERTIME,
                 ONINVISFLAG, ONKICK, ONLISTDEMOS, ONMAPSTART, ONMAPVOTE, ONMASTERMODE,
                 ONMODMAP, ONMUTE, ONNEWMAP, ONPAUSEGAME, ONRESUMEGAME, ONRECORDDEMO, ONRESETFLAG,
                 ONRETURNFLAG, ONSAVEDEMO, ONSAYTEAM, ONSCOREFLAG, ONSETMASTER, ONSETTEAM,
                 ONSPAWN, ONSPECTATOR, ONSTOPDEMO, ONSUICIDE, ONSWITCHMODEL, ONSWITCHNAME,
                 ONSWITCHTEAM, ONTAKEFLAG, ONTEAMKILL, ONTEXT, ONMUTETRIGGER, ONMUTEMODETRIGGER,
                 IRC_ONCOMMAND, IRC_ONCOMMANDPERMERROR, IRC_ONCOMMANDUNKNOWN,
                 IRC_ONCOMMANDUSAGEERROR, IRC_ONMSG, IRC_ONPRIVMSG, ONFLOOD,
                 CUSTOMEVENT, NUMEVENTS };

static const char * const eventNames[] = {
                 "onaddbot", "onbotlimit", "onclearbans", "oncleardemos", "oncommand",
                 "oncommandpermerror", "oncommandunknown", "oncommandusageerror",
                 "onconnect", "ondeath", "ondelbot", "ondisconnect", "ondropflag",
                 "oneditmode", "onghost", "onghosttrigger","onfrag", "ongetdemo", "ongetmap", "onreceivemap", "onimission", "onovertime",
                 "oninvisflag", "onkick", "onlistdemos", "onmapstart", "onmapvote", "onmastermode",
                 "onmodmap", "onmute", "onnewmap", "onpausegame", "onresumegame", "onrecorddemo", "onresetflag",
                 "onreturnflag", "onsavedemo", "onsayteam", "onscoreflag", "onsetmaster", "onsetteam",
                 "onspawn", "onspectator", "onstopdemo", "onsuicide", "onswitchmodel", "onswitchname",
                 "onswitchteam", "ontakeflag", "onteamkill", "ontext", "onmutetrigger", "onmutemodetrigger",
                 "irc_oncommand", "irc_oncommandpermerror", "irc_oncommandunknown",
                 "irc_oncommandusageerror", "irc_onmsg", "irc_onprivmsg", "onflood",
                 "custom event", "number of events" };

namespace remod
{
    // event and its callback
    struct evt_handler
    {
        eventType evt_type;   // type of event
        const char *custom;   // custom named event, null by default
        const char *evt_cmd;  // callback command
    };

    // events queue
    struct evt_param
    {
        char type;       // i - int, s - string, f, d - float;
        union {          // param value
            int *value_i;
            char *value_s;
            double *value_d;
            void *value;
        };

        evt_param();
        ~evt_param();
    };

    struct event
    {
        eventType evt_type;
        const char *custom;
        const char *fmt;
        vector<evt_param *> params;

        event();
        ~event();
    };

    char *event2str(eventType type);
    eventType str2event(const char *name);
    void onevent(eventType etype, const char *fmt, ...);    // add event to queue
    void oneventi(eventType etype, const char *fmt, ...);   // execute event instantly
    void eventsupdate();
}
#endif
