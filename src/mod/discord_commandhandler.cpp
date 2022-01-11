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

    void discord_slashcommand_handler(const char *discord_user, const char *cmd_name, const char *cmd_params) {}

    void discord_register_slashcommand(const char *cmd_name, const char *cmd_func, int *cmd_perm, const char *cmd_descr, const char *cmd_help)
    {
        common_registercommand(discord_cmd_handlers, cmd_name, cmd_func, cmd_perm, cmd_descr, cmd_help);
    }

    void irc_unregister_slashcommand(const char *cmd_name)
    {
        common_unregistercommand(discord_cmd_handlers, cmd_name);
    }

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
    COMMAND(discord_registercommand, "ssiss");

    /**
     * Unregister irc command
     * @group event
     * @arg1 command name
     * @example discord_unregistercommand "ban"
     */
    COMMAND(discord_unregistercommand, "s");
}