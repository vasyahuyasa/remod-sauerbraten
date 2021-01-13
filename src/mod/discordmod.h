/*
* remod:    discordmod.h
* date:     2021
* author:   degrave
*
* Discord plugin interface
*/

#ifndef __DISCORD_H__
#define __DISCORD_H__

#include "discord/discord_plugin.h"

namespace remod
{
    namespace discord
    {
        struct pluginapi {
            int (*version)();
            int (*run)(messagecallback *msgcb, const char * token);
            int (*sendmessage)(const char *channel,const char *text);
            const char *(*lasterror)();
        };

        int initplugin(const char *path, pluginapi *api);
    }
} // namespace remod

#endif