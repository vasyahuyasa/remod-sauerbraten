#include "remod.h"
#include "discordmod.h"
#include "commandev.h"

namespace remod
{
    namespace discord
    {
        SVAR(discordtoken, "");

        void run()
        {
            printf("running discord with token \"%s\"\n", discordtoken);
            messagecallback cb = &onmessage;

            if (discord_run(cb, discordtoken) != 0) {
                printf("can not start discord: %s\n", discord_lasterror());
                return;
            }

            printf("discord started\n");
        }

        void onmessage(char *author_username, char *author_mentoin_string, char *channel, char *content)
        {
            remod::onevent(DISCORD_ONMSG, "ssss", author_username, author_mentoin_string, channel, content);
        }

        void discordsay(char *channel, char *msg) {
            if (discord_sendmessage(channel, msg) != 0) {
                printf("can not send message to discord: %s\n", discord_lasterror());
            }
        }

        COMMAND(discordsay, "ss");
    } // namespace discord
} // namespace remod