#include "remod.h"
#include "discordmod.h"
#include "commandev.h"

namespace remod
{
    namespace discord
    {
        bool initialized = false;
        char *channelid = NULL;

        void addrelay(char *token, char *channel)
        {
            if (initialized)
            {
                printf("discord: alredy running for channel %s\n", channelid);
                return;
            }

            channelid = newstring(channel);

            messagecallback cb = &onmessage;

            if (discord_run(cb, token) != 0)
            {
                printf("discord: can not run for channel %s: %s\n", channelid, discord_lasterror());
                return;
            }

            initialized = true;
            printf("discord: running for channel %s\n", channelid);
        }

        // TODO: call this function from go can be not thread safe
        void onmessage(char *author_username, char *author_mentoin_string, char *channel, char *content)
        {
            size_t namelen = strlen(author_username);
            uchar *cubename = (uchar *)malloc(namelen + 1);
            size_t cubenamelen = decodeutf8(cubename, namelen, (uchar *)author_username, namelen, 0);
            cubename[cubenamelen] = '\0';

            size_t msglen = strlen(content);
            uchar *cubemsg = (uchar *)malloc(msglen + 1);
            size_t cubemsglen = decodeutf8(cubemsg, msglen, (uchar *)content, msglen, 0);
            cubemsg[cubemsglen] = '\0';

            remod::onevent(DISCORD_ONMSG, "ssss", cubename, author_mentoin_string, channel, cubemsg);
            
            free(cubename);
            free(cubemsg);
        }

        void discordsay(char *msg)
        {
            discord_sendmessage(channelid, msg);
        }

        void discordsayto(char *channel, char *msg)
        {
            discord_sendmessage(channel, msg);
        }

        /**
         * Initialize discord bot with specified token and default relay channel
         * @group discord
         * @arg1 bot token
         * @arg2 channel id
         */
        COMMANDN(discordaddrelay, addrelay, "ss");

        /**
         * Send message to discord channel specified in discordaddrelay command
         * @group discord
         * @arg1 message
         */
        COMMAND(discordsay, "s");

        /**
         * Send message to specified discord channel
         * @group discord
         * @arg1 channel id
         * @arg2 message
         */
        COMMAND(discordsayto, "ss");

    } // namespace discord
} // namespace remod