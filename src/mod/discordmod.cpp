#include "discordmod.h"
#include "commandev.h"

namespace remod
{
    namespace discord
    {
        SVAR(discordtoken, "");

        void run()
        {
            messagecallback cb = &onmessage;

            int code = discord_run(cb, discordtoken);

            if (code != 0) {
                printf("can not start discord: %s\n", discord_lasterror());
                return;
            }

            printf("discord started\n");
        }

        void onmessage(char *author_username, char *author_mentoin_string, char *channel_id, char *content)
        {
            printf("%s: %s\n", author_mentoin_string, content);
        }
    } // namespace discord
} // namespace remod