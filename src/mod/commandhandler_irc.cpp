#include "commandhandler_common.h"
#include "commandev.h"
#include "remod.h"
#include "irc.h"

namespace remod
{

	vector<cmd_handler *> irc_cmd_handlers; //IRC command handlers

	/**
	 * returns user's permissions.
	 * if function irc_grantperm is defined  uses it result
	 */
	int irc_getperm_(char *usr)
	{

		int perm = 0; // no permission at start

		switch (irc_user_state(usr))
		{
		case OP:
		case ADMIN:
		case OWNER:
			perm = 2;
			break;

		case VOICE:
		case HALFOP:
			perm = 1;
			break;

		case NONE:
		case ERR:
		default:
			perm = 0;
			break;
		}

		//check if irc_grantperm exists
		ident *i = getident("irc_grantperm");
		if (i && i->type == ID_ALIAS)
		{
			char *cmd = newstring("irc_grantperm ");

			concatpstring(&cmd, escapestring(usr));

			int getperm = execute(cmd);
			DELETEA(cmd);

			// assign if granted permission more than default
			if (getperm > perm)
				perm = getperm;
		}

		return perm;
	}

	void irc_getperm(char *usr)
	{
		intret(irc_getperm_(usr));
	}

	/**
	 * checks if user has required permissions
	 */
	bool irc_checkperm(char *usr, int perm)
	{
		return irc_getperm_(usr) >= perm;
	}

	void irc_oncommand(const char *user, const char *cmd_name, const char *cmd_params)
	{
		int found = find_command_handler(cmd_name, irc_cmd_handlers);

		if (found == -1)
		{
			//unknown command
			remod::onevent(IRC_ONCOMMANDUNKNOWN, "ss", user, cmd_name);
			return;
		}

		cmd_handler *handler = irc_cmd_handlers[found];

		//checking for permission
		if (!irc_checkperm((char *)user, handler->cmd_permissions))
		{
			//permission error
			remod::onevent(IRC_ONCOMMANDPERMERROR, "ss", user, cmd_name);
			return;
		}

		int ret = execute_command(handler, user, cmd_params);
		if (ret == -1)
		{
			//usage error - cannot parse cmd_params in accordance with cmd_descr
			remod::onevent(IRC_ONCOMMANDUSAGEERROR, "ss", user, cmd_name);
		}
		else if (ret == -2)
		{
			//permission error
			remod::onevent(IRC_ONCOMMANDPERMERROR, "ss", user, cmd_name);
		}
	}

	void irc_commandhelp(const char *cmd_name)
	{
		common_commandhelp(irc_cmd_handlers, cmd_name);
	}
	/**
	 * Registers handler function for command
	 */
	void irc_registercommand(const char *cmd_name, const char *cmd_func, int *cmd_perm, const char *cmd_descr, const char *cmd_help)
	{
		common_registercommand(irc_cmd_handlers, cmd_name, cmd_func, cmd_perm, cmd_descr, cmd_help);
	}

	/**
	 * Delete command
	 */
	void irc_unregistercommand(const char *cmd_name)
	{
		common_unregistercommand(irc_cmd_handlers, cmd_name);
	}

	/**
	 * Register command for IRC bot, has the same behavior as registercommand
	 * Triggers irc_oncommandusageerror, irc_oncommandunknown or irc_oncommandpermerror event if fails
	 * @group event
	 * @arg1 command name
	 * @arg2 function to execute when command is called
	 * @arg3 required permission level (default: 0 for all, 1 for voiced user/half op, 2 for admin/op/owner) see irc_getperm
	 * @arg4 type mask, see registercommand
	 * @arg5 help string
	 * @example irc_registercommand "ban" irccmd_ban 2 "w|s" "ban [cn|ip] [reason]. Add permanent ban"
	 */

	COMMAND(irc_registercommand, "ssiss");

	/**
	 * Unregister irc command
	 * @group event
	 * @arg1 command name
	 * @example irc_unregistercommand "ban"
	 */
	COMMAND(irc_unregistercommand, "s");

	/**
	 * Return help string for irc command (see: irc_registercommand)
	 * @group event
	 * @arg1 command_name
	 * @return help string
	 * @example irc_commandhelp "bans" // return  "ban [cn|ip] [reason]. Add permanent ban"
	 */
	COMMAND(irc_commandhelp, "s");

	/**
	 * Return integer permission for irc user with specified connection_string
	 * Checks if cubescript function irc_grantperm exists and calls it with connection_string (there could be the custom rules for special users)
	 * If no, default values are:  0 for all, 1 for voiced user/half op, 2 for admin/op/owner
	 * @group event
	 * @arg1 connection_string
	 * @return integer value for permission
	 */
	COMMAND(irc_getperm, "s");

	/**
	 * Loops through registered irc commands with specified permission for executing. Useful for displaying all available server commands
	 * @group event
	 * @arg1 iterator variable name
	 * @arg2 required permission value
	 * @arg3 code to execute
	 * @example irc_loopcommands cmd 2 [ echo $cmd ] //echoes all commands available to op
	 */
	ICOMMAND(irc_loopcommands, "sis", (char *var, int *permissions, char *body), common_loopcommands(var, permissions, body, irc_cmd_handlers));
}
