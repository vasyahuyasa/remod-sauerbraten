/*
* remod:    discordmod.h
* date:     2021
* author:   degrave
*
* Discord plugin interface
*/

#ifndef __DISCORD_H__
#define __DISCORD_H__

#include "discord/libdiscord.h"

namespace remod
{
    namespace discord
    {
        void run();
        void onmessage(char *author_username, char *author_mentoin_string, char *channel_id, char *content);
    }
} // namespace remod

#endif