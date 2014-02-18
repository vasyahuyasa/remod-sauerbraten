#include "commandhandler.h"
#include "commandev.h"
#include "remod.h"

#ifdef IRC
#include "irc.h"
#endif

namespace remod {

/* ----------------------- PRIVATE METHODS ----------------------- */

/**
 * checks if param is valid integer
 */
bool is_int(const char* param) {
	return (strcmp(param, "0") == 0 || atol(param) != 0);
}


/**
 * returns list of params parsed via description or null if failed
 */
bool parsecommandparams(const char *descr, const char *params, vector<cmd_param> &res)
{
	bool required_params = true; //current parameter is required
	unsigned long descr_len = strlen(descr);
	unsigned long params_len = strlen(params);
	const char *params_end = params + params_len;//last element of params string
	const char *p = params; //current position in params string

	bool validated = true;

	for (unsigned int i = 0; i < descr_len; i++) {
		char ch = descr[i]; //type of current parameter
		if (ch == '|') {
			required_params = false; //next parameters are not required
		} else {
			cmd_param cp;
			if ( (long) p >= (long) params_end) { // no more parameters
				if (!required_params) {
					cp.type = ch;
					cp.param = newstring("");
					res.add(cp);
					continue; //parameter is not required. add empty string
				} else {
					return false; //parameter is required. error.
				}
			}

			const char *param_end_pos; //first character after current parameter in params string
			const char *next_param_start_pos; //first character of next parameter in params string
			if (ch == 's') { //if string - use whole paramstring as parameter
				param_end_pos = next_param_start_pos = params_end;
			} else {
				param_end_pos = strchr(p, ' '); //parameter ends by space

				if (param_end_pos) { // there is space in p
					next_param_start_pos = param_end_pos;
					while (next_param_start_pos[0] == ' ') {// fuck the multiple spaces up
						next_param_start_pos++;
					}
				} else {
					param_end_pos = next_param_start_pos = params_end; // no more spaces, we reached the end of params string
				}
			}
			unsigned long fact_size = param_end_pos - p; //length of param
			char *param = newstring(p, fact_size); //current parameter

			p = next_param_start_pos;

			int cn;

			//valdating param
			switch (ch) {
			case 'i': //integer
				if (!is_int(param)) {
					validated = false;
				}
				break;
			case 'f': //float
				if (strcmp(param, "0") != 0 && atof(param) == 0) {
					validated = false;
				}
				break;
			case 'b': //boolean
				if (strcmp(param, "1") == 0 || strcmp(param, "true") == 0 || strcmp(param, "y") == 0) {
					DELETEA(param);
					param = newstring("1");
				} else if (strcmp(param, "0") == 0 || strcmp(param, "false") == 0 || strcmp(param, "n") == 0) {
					DELETEA(param);
					param = newstring("0");
				} else {
					validated = false;
				}
				break;
			case 'c': //client num or player name
				if (!is_int(param)) return false;
				cn = parseplayer(param);
				if (cn == -1) {
					validated = false;
				} else {
					DELETEA(param);
					param = newstring(intstr(cn));
				}
				break;
			case 'p': //client num for specified player or -1 for all
				if (!is_int(param)) return false;
				if (strcmp(param, "-1") != 0) {
					cn = parseplayer(param);
					if (cn == -1) {
						validated = false;
					} else {
						DELETEA(param);
						param = newstring(intstr(cn));
					}
				}
				break;
			case 'w': //word
				break; //nothing to do
			case 't': //time (integer - seconds  or mm:ss)
				char *colon_pos = strchr(param, ':');
				if (colon_pos != 0) {

					string min, sec;
					strncpy(min, param, (unsigned long) (colon_pos - param));
					strcpy(sec, colon_pos+1);

					if (!is_int(min) || !is_int(sec)) {
						validated = false;
					} else {
						DELETEA(param);
						param = newstring(intstr(atoi(min)*60+atoi(sec)));
					}
				} else {
					if (!is_int(param)) {
						validated = false;
					}
				}
				break;
			}
			if (!validated) {
				DELETEA(param);
				break;
			} else {
				cp.type = ch;
				cp.param = newstring(param);
				DELETEA(param);
				res.add(cp);
			}
		}

	}
	if (!validated) {
		for (int i = 0; i < res.length(); i++) {
			DELETEA(res[i].param);
		}
		return false;
	}
	return true;
}

/**
 * Search command handler for cmd_name in handlers
 */
int find_command_handler(const char *cmd_name, vector<cmd_handler*> &handlers) {
	//searching for command with name cmd_name in registered handlers
	loopv(handlers)
	{
	    cmd_handler *h = handlers[i];
	    if(strcmp(cmd_name, h->cmd_name) == 0)
            return i;
	}
	return -1;
}

/**
 * Parses command parameters string cmd_params in accordance with handler->cmd_descr.
 * Caller is a reference to player (his cn) or irc user (username) who called this command
 * Executes command handler->cmd_func with parsed parameters
 * Returns returned by cmd_func value (usually 0) or -1 due to usage error or -2 due to permissions error
 */
int execute_command(cmd_handler *handler, const char *caller, const char *cmd_params) {
	//parsing params string
	vector<cmd_param> params;
	bool parsed = parsecommandparams(handler->cmd_descr, cmd_params, params);
	if (!parsed) {
		return -1;
	}
	//creating cmd string
	char* cmd = newstring("");
	concatpstring(&cmd, 3, handler->cmd_func, " ", escapestring(caller));//first parameter is always cn of player
	//!!! escapestring should be used without DELETEA

	loopv(params)
	{
		concatpstring(&cmd, " ");
		if (params[i].type == 's' || params[i].type == 'w' || strlen(params[i].param) == 0) { //for correct string transition in list it should be escaped by "
			concatpstring(&cmd, escapestring(params[i].param));
		} else {
			concatpstring(&cmd, params[i].param);
		}
		DELETEA(params[i].param);
	}


	//calling command
	int ret = execute(cmd);

	DELETEA(cmd);
	return ret;
}

/**
 * Loops through list of commands available for player/user with specified permission. Common for server and irc
 */
void common_loopcommands(const char *var, int *permission, const char *body, vector<cmd_handler*> &handlers)
{
	ident *id = newident(var);
	if (id->type != ID_ALIAS)
		return;

    identstack stack;
	int j = 0;
	loopv(handlers)
	{
		cmd_handler *handler = handlers[i];
		if (handler->cmd_permissions <= *permission) {

			if (j) {
                if(id->valtype == VAL_STR) delete[] id->val.s;
                else id->valtype = VAL_STR;
                ::cleancode(*id);
                id->val.s = newstring(handler->cmd_name);
			} else {
			    tagval t;
			    t.setstr(newstring(handler->cmd_name));
			    ::pusharg(*id, t, stack);
			    id->flags &= ~IDF_UNKNOWN;
			}

			execute(body);
			j++;
		}
	}

	if (j)
		poparg(*id);
}


/**
 * Return help string for command; Common for server and irc
 */
void common_commandhelp(vector<cmd_handler*> &handlers, const char* cmd_name)
{
	//searching for command with name cmd_name in registered handlers
	loopv(handlers)
	{
	    cmd_handler *h = handlers[i];
	    if(strcmp(cmd_name, h->cmd_name) == 0)
        {
            result(h->cmd_help);
            return;
        }
	}
}

/**
 * Registers handler function for command; Common for server and irc
 */
void common_registercommand(vector<cmd_handler*> &handlers, const char* cmd_name, const char* cmd_func, int *cmd_perm, const char* cmd_descr, const char* cmd_help)
{
	if (cmd_name[0] && cmd_func[0]) {
		cmd_handler *cmd = new cmd_handler;
		cmd->cmd_descr = newstring(cmd_descr);
		cmd->cmd_name = newstring(cmd_name);
		cmd->cmd_func = newstring(cmd_func);
		cmd->cmd_help = newstring(cmd_help);
		cmd->cmd_permissions = *cmd_perm;

		//find position to add command - for sorted list
		loopv(handlers)
		{
		    if(strcmp(cmd_name, handlers[i]->cmd_name) < 0)
            {
                // we have registered command with such name
                handlers.insert(i, cmd);
                return;
            }
		}

		// register new command
		handlers.add(cmd);
	}
}

/**
 * Delete command; Common for server and irc
 */
void common_unregistercommand(vector<cmd_handler*> &handlers, const char* cmd_name)
{
	//searching for command with name cmd_name in registered handlers
	loopv(handlers)
	{
	    cmd_handler *h = handlers[i];
	    if(strcmp(cmd_name, h->cmd_name) == 0)
        {
            DELETEA(h->cmd_descr);
            DELETEA(h->cmd_func);
            DELETEA(h->cmd_help);
            DELETEA(h->cmd_name);
            handlers.remove(i);
            return;
        }
	}
}

/* ----------------------- PUBLIC METHODS ----------------------- */


//-----------------------------------------------------------------
// SERVER COMMANDS

/**
 * Server command handlers
 */
vector<cmd_handler*> cmd_handlers;


/**
 * returns player's permissions.
 * if function grantperm is defined  uses it result
 */
int getperm_(int *cn) {

    int perm = 1; // no permission

    // default permission
    if (isadmin(cn)) {
		perm = 3;
	} else if (ismaster(cn)) {
		perm = 2;
	} else {
		perm = 1;
	}

	//check if grantperm exists and above default permission
	ident *i = getident("grantperm");
	if (i && i->type == ID_ALIAS) {
		char *cmd = newstring("grantperm ");
		concatpstring(&cmd, intstr(*cn));

		int grantedperm = execute(cmd);
		DELETEA(cmd);
		if(grantedperm>perm) perm = grantedperm;
	}

    return perm;
}

void getperm(int *cn) {
	intret(getperm_(cn));
}

/**
 * checks if player has required permissions
 */
bool checkperm(int cn, int perm) {
	return getperm_(&cn) >= perm;
}


void oncommand(int cn, const char *cmd_name, const char *cmd_params)
{
	int found = find_command_handler(cmd_name, cmd_handlers);

	if (found == -1) {
		//unknown command
		remod::onevent(ONCOMMANDUNKNOWN, "is", cn, cmd_name);
		return;
	}
	cmd_handler *handler = cmd_handlers[found];

	//checking for permission
	if (!checkperm(cn, handler->cmd_permissions))
	{
		//permission error
		remod::onevent(ONCOMMANDPERMERROR, "is", cn, cmd_name);
		return;
	}

	int ret = execute_command(handler, intstr(cn), cmd_params);

	if (ret == -1) {
		//usage error - cannot parse cmd_params in accordance with cmd_descr
		remod::onevent(ONCOMMANDUSAGEERROR, "is", cn, cmd_name);
	} else if(ret == -2) {
		//permission error
		remod::onevent(ONCOMMANDPERMERROR, "is", cn, cmd_name);
	}
}

void commandhelp(const char* cmd_name)
{
	common_commandhelp(cmd_handlers, cmd_name);
}

/**
 * Registers handler function for command
 */
void registercommand(const char* cmd_name, const char* cmd_func, int *cmd_perm, const char* cmd_descr, const char* cmd_help)
{
	common_registercommand(cmd_handlers, cmd_name, cmd_func, cmd_perm, cmd_descr, cmd_help);
}

/**
 * Delete command
 */
void unregistercommand(const char* cmd_name)
{
	common_unregistercommand(cmd_handlers, cmd_name);
}


/**
 * Register server command. Registered command could be called  as #name param1 param2
 * Before executing system checks permissions for player called this command and the correspondence of given arguments to declared type mask.
 * The type mask contains of a sequence of declaring type chars that correspond to the arguments and a delimiter "|" to separate required arguments from not required
 * Supported argument types:
 * - "i" - argument must be integer
 * - "s" - string, MUST BE the last argument! Otherwise system could not parse arguments string correctly
 * - "w" - single word
 * - "c" - client number or player name
 * - "p" - client number or player name or "-1"
 * - "b" - boolean ("0" or "1")
 * - "f" - float
 * - "t" - time  mm:ss or just integer count of seconds
 * - "|" - delimiter between required and not required arguments
 * If validating parameter string fails , oncommandusageerror event is triggered
 * If system doesn't found the command , oncommandunknown event is triggered
 * If permission check fails, oncommandpermerror event is triggered
 * @group event
 * @arg1 command name
 * @arg2 function to execute when command is called
 * @arg3 required permission level to execute this command: 1 for any player, 2 for admin, 3 for master (could be changed, see getperm function)
 * @arg4 type mask, declares types of command arguments and their necessity
 * @arg5 help string
 * @example registercommand "ban" cmd_ban 3 "w|s" "ban [cn|ip] [reason] ^f1Add permanent ban" // arg1 (cn or ip) is required, arg2 (reason) is not required
 */
COMMAND(registercommand, "ssiss");

/**
 * Unregister server command
 * @group event
 * @arg1 command name
 * @example unregistercommand "ban"
 */
COMMAND(unregistercommand, "s");

/**
 * Return help string for command (see: registercommand)
 * @group event
 * @arg1 command_name
 * @return help string
 * @example commandhelp "bans" // return  "ban [cn|ip] [reason] ^f1Add permanent ban"
 */
COMMAND(commandhelp, "s");

/**
 * Return integer permission for player with specified cn
 * Checks if cubescript function grantperm exists and calls it with cn (there the custom rules for special ip for instance could be set, e.g. player connected from "192.168.*.*" may always have master permissions)
 * If no, default values are: 1 for any player, 2 for admin, 3 for master
 * @group event
 * @arg1 cn
 * @return integer value for permission
 */
COMMAND(getperm, "i");

/**
 * Loops through registered server commands with specified permission for executing. Useful for displaying all available server commands
 * @group event
 * @arg1 iterator variable name
 * @arg2 required permission value
 * @arg3 code to execute
 * @example loopcommands cmd 3 [ echo $cmd ] //echoes all commands available to master
 */
ICOMMAND(loopcommands, "sis", (char *var, int *permissions, char *body), common_loopcommands(var, permissions, body, cmd_handlers));


//-----------------------------------------------------------------
// IRC COMMANDS

#ifdef IRC

vector<cmd_handler*> irc_cmd_handlers; //IRC command handlers


/**
 * returns user's permissions.
 * if function irc_grantperm is defined  uses it result
 */
int irc_getperm_(char *usr) {

    int perm = 0; // no permission at start

    switch(irc_user_state(usr))
    {
        case OP:
        case ADMIN:
        case OWNER: perm = 2; break;

        case VOICE:
        case HALFOP: perm = 1; break;

        case NONE:
        case ERR:
        default: perm = 0; break;
    }

	//check if irc_grantperm exists
	ident *i = getident("irc_grantperm");
	if (i && i->type == ID_ALIAS) {
		char *cmd = newstring("irc_grantperm ");

		concatpstring(&cmd, escapestring(usr));

		int getperm = execute(cmd);
		DELETEA(cmd);

		// assign if granted permission more than default
		if(getperm>perm) perm = getperm;
	}

    return perm;
}

void irc_getperm(char *usr) {
	intret(irc_getperm_(usr));
}

/**
 * checks if user has required permissions
 */
bool irc_checkperm(char *usr, int perm) {
	return irc_getperm_(usr) >= perm;
}

void irc_oncommand(const char* user, const char* cmd_name, const char* cmd_params)
{
	int found = find_command_handler(cmd_name, irc_cmd_handlers);

	if (found == -1) {
		//unknown command
		remod::onevent(IRC_ONCOMMANDUNKNOWN, "ss", user, cmd_name);
		return;
	}

	cmd_handler *handler = irc_cmd_handlers[found];

	//checking for permission
	if (!irc_checkperm((char*) user, handler->cmd_permissions))
	{
		//permission error
		remod::onevent(IRC_ONCOMMANDPERMERROR, "ss", user, cmd_name);
		return;
	}

	int ret = execute_command(handler, user, cmd_params);
	if (ret == -1) {
		//usage error - cannot parse cmd_params in accordance with cmd_descr
		remod::onevent(IRC_ONCOMMANDUSAGEERROR, "ss", user, cmd_name);
	} else if(ret == -2) {
		//permission error
		remod::onevent(IRC_ONCOMMANDPERMERROR, "ss", user, cmd_name);
	}
}

void irc_commandhelp(const char* cmd_name)
{
	common_commandhelp(irc_cmd_handlers, cmd_name);
}
/**
 * Registers handler function for command
 */
void irc_registercommand(const char* cmd_name, const char* cmd_func, int *cmd_perm, const char* cmd_descr, const char* cmd_help)
{
	common_registercommand(irc_cmd_handlers, cmd_name, cmd_func, cmd_perm, cmd_descr, cmd_help);
}

/**
 * Delete command
 */
void irc_unregistercommand(const char* cmd_name)
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

#endif

}
