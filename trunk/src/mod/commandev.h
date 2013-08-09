#ifndef __COMMANDEV_H__
#define __COMMANDEV_H__

#include "fpsgame.h"

enum eventType { ONADDBOT = 0, ONBOTLIMIT, ONCLEARBANS, ONCLEARDEMOS, ONCOMMAND,
                 ONCOMMANDPERMERROR, ONCOMMANDUNKNOWN, ONCOMMANDUSAGEERROR,
                 ONCONNECT, ONDEATH, ONDELBOT, ONDISCONNECT, ONDROPFLAG,
                 ONEDITMODE, ONEDITMUTE, ONFRAG, ONGETDEMO, ONGETMAP, ONIMISSION,
                 ONINVISFLAG, ONKICK, ONLISTDEMOS, ONMAPSTART, ONMAPVOTE, ONMASTERMODE,
                 ONMODMAP, ONMUTE, ONNEWMAP, ONPAUSEGAME, ONRECORDDEMO, ONRESETFLAG,
                 ONRETURNFLAG, ONSAVEDEMO, ONSAYTEAM, ONSCOREFLAG, ONSETMASTER, ONSETTEAM,
                 ONSPAWN, ONSPECTATOR, ONSTOPDEMO, ONSUICIDE, ONSWITCHMODEL, ONSWITCHNAME,
                 ONSWITCHTEAM, ONTAKEFLAG, ONTEAMKILL, ONTEXT,
                 IRC_ONCOMMAND, IRC_ONCOMMANDPERMERROR, IRC_ONCOMMANDUNKNOWN,
                 IRC_ONCOMMANDUSAGEERROR, IRC_ONMSG, IRC_ONPRIVMSG,
                 CUSTOMEVENT, NUMEVENTS };

static const char * const eventNames[] = {
                 "onaddbot", "onbotlimit", "onclearbans", "oncleardemos", "oncommand",
                 "oncommandpermerror", "oncommandunknown", "oncommandusageerror",
                 "onconnect", "ondeath", "ondelbot", "ondisconnect", "ondropflag",
                 "oneditmode", "oneditmute", "onfrag", "ongetdemo", "ongetmap", "onimission",
                 "oninvisflag", "onkick", "onlistdemos", "onmapstart", "onmapvote", "onmastermode",
                 "onmodmap", "onmute", "onnewmap", "onpausegame", "onrecorddemo", "onresetflag",
                 "onreturnflag", "onsavedemo", "onsayteam", "onscoreflag", "onsetmaster", "onsetteam",
                 "onspawn", "onspectator", "onstopdemo", "onsuicide", "onswitchmodel", "onswitchname",
                 "onswitchteam", "ontakeflag", "onteamkill", "ontext",
                 "irc_oncommand", "irc_oncommandpermerror", "irc_oncommandunknown",
                 "irc_oncommandusageerror", "irc_onmsg", "irc_onprivmsg",
                 "custom_event", "number of events" };

namespace remod
{
    char *event2str(eventType type);
    eventType str2event(const char *name);

    struct evt_handler
    {
        eventType       evt_type;   // type of event
        const char*     custom;     // custom named event, null by default
        const char*     evt_cmd;    // callback command
    };

    bool onevent(const char *evt_type, const char *fmt, ...); // depricated
    void onevent( eventType etype, const char *fmt, ...);
}
#endif
