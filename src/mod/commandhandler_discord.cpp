/**
 * commandhandler_discord.cpp
 *
 *  Created on: Jan 10, 2022
 *      Author: degrave
 */

#include "commandhandler_common.h"
#include "commandev.h"
#include "discord/libdiscord.h"
#include "commandhandler_discord.h"

namespace remod
{
    namespace discord
    {
        bool initialized = false;
        char *channelid = NULL;

        vector<cmd_handler *> discord_cmd_handlers;

        void addrelay(char *token, char *channel)
        {
            if (initialized)
            {
                printf("discord: alredy running for channel %s\n", channelid);
                return;
            }

            channelid = newstring(channel);

            messagecallback cb = &onmessage;

            if (discord_run(cb, token, channelid) != 0)
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
            if (!initialized)
            {
                return;
            }

            static uchar ubuf[1024 * 10];
            size_t len = strlen(msg);
            size_t carry = 0;
            while (carry < len)
            {
                size_t numu = encodeutf8(ubuf, sizeof(ubuf) - 1, &((const uchar *)msg)[carry], len - carry, &carry);
                if (carry >= len)
                {
                    ubuf[numu++] = '\0';
                }
            }

            discord_sendmessage(channelid, (char *)ubuf);
        }

        /**
         * returns user's permissions : 0, 1 or 2.
         * if function discord_grantperm is defined uses it result
         */
        int user_permission_level(char *discord_user) {}

        bool user_has_permission(char *discord_user, int perm)
        {
            return user_permission_level(discord_user) >= perm;
        }

        void slashcommand_handler(char *author_username, char *author_mentoin_string, char *channel_id, char *cmd_name, char *concated_input)
        {
            conoutf("author_username = %s author_mentoin_string = %s channel_id = %s cmd_name = %s concated_input = %s", author_username, author_mentoin_string, channel_id, cmd_name, concated_input);

            int cmd_index = find_command_handler(cmd_name, discord_cmd_handlers);

            if (cmd_index == -1)
            {
                conoutf("command %s not found", cmd_name);
                //unknown command
                //remod::onevent(ONCOMMANDUNKNOWN, "is", cn, cmd_name);
                return;
            }

            cmd_handler *handler = discord_cmd_handlers[cmd_index];

            int ret = execute_command(handler, author_username, concated_input);

            if (ret == -1)
            {
                conoutf("command %s usage error", cmd_name);
                //usage error - cannot parse cmd_params in accordance with cmd_descr
                //remod::onevent(ONCOMMANDUSAGEERROR, "is", cn, cmd_name);
            }
            else if (ret == -2)
            {
                conoutf("command %s permission error", cmd_name);
                //permission error
                //remod::onevent(ONCOMMANDPERMERROR, "is", cn, cmd_name);
            }
        }

        void register_slashcommand(char *cmd_name, char *cmd_func, int *cmd_perm, char *cmd_descr, char *cmd_help)
        {
            conoutf("register_slashcommand name=%s func=%s perm=%d, descr=%s help=%s", cmd_name, cmd_func, *cmd_perm, cmd_descr, cmd_help);

            common_registercommand(discord_cmd_handlers, cmd_name, cmd_func, cmd_perm, cmd_descr, cmd_help);

            command_option options[] = {
                {
                    .name = newstring("name"),
                    .required = 1,
                    .option_t = option_type_stirng,
                },
                // {
                //     .name = "cn",
                //     .required = 1,
                //     .option_t = option_type_integer,
                // },
                // {
                //     .name = "flag",
                //     .required = 1,
                //     .option_t = option_type_bool,
                // },
            };

            GoSlice go_options = {
                .data = options,
                .len = 1,
                .cap = 1,
            };

            commandhandler handler = &slashcommand_handler;

            discord_register_command(newstring(cmd_name), newstring(cmd_help), go_options, handler);
        }

        void discord_unregister_slashcommand(const char *cmd_name)
        {
            common_unregistercommand(discord_cmd_handlers, cmd_name);
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
         * Register slash command for discord bot, has the same behavior as registercommand
         * Triggers discord_oncommandusageerror, discord_oncommandunknown or discord_oncommandpermerror event if fails
         * Supported argument types:
         *   - "i" - argument must be integer
         *   - "s" - string, MUST BE the last argument! Otherwise system could not parse arguments string correctly
         *   - "c" - client number or player name
         *   - "p" - client number or player name or "-1"
         *   - "b" - boolean ("0" or "1")
         *   - "t" - time  mm:ss or just integer count of seconds
         *   - "|" - delimiter between required and not required arguments
         * Every argument can have name. Name should be placed in round brackets right after argument.
         * @group event
         * @arg1 command name
         * @arg2 function to execute when command is called
         * @arg3 required permission level (default: 0 for all, 1 operator, 2 admin/owner)
         * @arg4 extended type mask
         * @arg5 help string
         * @example discord_registercommand "ban" discordcmd_ban 2 "w(cn or ip)|s(reason)" "Add permanent ban"
         */
        COMMANDN(discord_registercommand, register_slashcommand, "ssiss");

        /**
         * Unregister irc command
         * @group event
         * @arg1 command name
         * @example discord_unregistercommand "ban"
         */
        COMMANDN(discord_unregistercommand, discord_unregister_slashcommand, "s");
    }
}