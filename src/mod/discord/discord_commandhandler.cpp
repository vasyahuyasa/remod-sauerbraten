/**
 * discord_commandhandler.cpp
 *
 *  Created on: Jan 10, 2022
 *      Author: degrave
 */

#include "commandhandler.h"
#include "commandev.h"

namespace remod
{
    vector<cmd_handler *> discord_cmd_handlers;

    /**
     * returns user's permissions : 0, 1 or 2.
     * if function discord_grantperm is defined uses it result
     */
    int discord_user_permission_level(char *discord_user) {}

    bool discord_user_has_permission(char *discord_user, int perm)
    {
        return discord_user_permission_level(discord_user) >= perm;
    }

    void discord_slashcommand_handler(const char *discord_user, const char *cmd_name, const char *cmd_params){}
    {
    }

    void discord_register_slashcommand(const char *cmd_name, const char *cmd_func, int *cmd_perm, const char *cmd_descr, const char *cmd_help)
    {
        common_registercommand(discord_cmd_handlers, cmd_name, cmd_func, cmd_perm, cmd_descr, cmd_help);
    }

    void irc_unregister_slashcommand(const char *cmd_name)
    {
        common_unregistercommand(irc_cmd_handlers, cmd_name);
    }

    /**
     * Register command for IRC bot, has the same behavior as registercommand
     * Triggers irc_oncommandusageerror, irc_oncommandunknown or irc_oncommandpermerror event if fails
     *  * Supported argument types:
     * - "i" - argument must be integer
     * - "s" - string, MUST BE the last argument! Otherwise system could not parse arguments string correctly
     * - "w" - single word
     * - "c" - client number or player name
     * - "p" - client number or player name or "-1"
     * - "b" - boolean ("0" or "1")
     * - "t" - time  mm:ss or just integer count of seconds
     * - "|" - delimiter between required and not required arguments
     * @group event
     * @arg1 command name
     * @arg2 function to execute when command is called
     * @arg3 required permission level (default: 0 for all, 1 for voiced user/half op, 2 for admin/op/owner) see irc_getperm
     * @arg4 type mask, see registercommand
     * @arg5 help string
     * @example discord_registercommand "ban" discordcmd_ban 2 "w(cn/ip)|s(reason)" "Add permanent ban"
     */
    COMMAND(discord_registercommand, "ssiss");

    /**
     * Unregister irc command
     * @group event
     * @arg1 command name
     * @example discord_unregistercommand "ban"
     */
    COMMAND(discord_unregistercommand, "s");
}