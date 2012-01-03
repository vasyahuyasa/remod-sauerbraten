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
int find_command_handler(const char *cmd_name, vector<cmd_handler> &handlers) {
	//searching for command with name cmd_name in registered handlers
	int len = handlers.length();
	for (int i = 0; i < len; i++)
	{
		if (strcmp(cmd_name, handlers[i].cmd_name) == 0) {
			return i;
		}
	}
	return -1;
}

/**
 * Parses command parameters string cmd_params in accordance with handler->cmd_descr.
 * Caller is a reference to player (his cn) or irc user (username) who called this command
 * Executes command handler->cmd_func with parsed parameters
 * Returns returned by cmd_func value (usually 0) or -1 due to usage error or -2 due to permissions error
 */
int execute_command(cmd_handler &handler, const char *caller, const char *cmd_params) {
	//parsing params string
	vector<cmd_param> params;
	bool parsed = parsecommandparams(handler.cmd_descr, cmd_params, params);
	if (!parsed) {
		return -1;
	}
	//creating cmd string
	char* cmd = newstring("");
	cmd = concatpstring(cmd, handler.cmd_func);
	cmd = concatpstring(cmd, " ");
	cmd = concatpstring(cmd, caller); //first parameter is always cn of player
	loopv(params)
	{
		cmd = concatpstring(cmd, " ");
		if (params[i].type == 's' || strlen(params[i].param) == 0) { //for correct string transition in list it should be escaped by "
			cmd = concatpstring(cmd, "\"");
			cmd = concatpstring(cmd, params[i].param);
			cmd = concatpstring(cmd, "\"");
		} else {
			cmd = concatpstring(cmd, params[i].param);
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
void common_loopcommands(const char *var, int *permission, const char *body, vector<cmd_handler> &handlers)
{
	ident *id = newident(var);
	if (id->type != ID_ALIAS)
		return;
	int j = 0;
	loopi(handlers.length())
	{
		cmd_handler handler = handlers[i];
		if (handler.cmd_permissions <= *permission) {
			char *st = newstring(handler.cmd_name);
			if (j) {
				aliasa(id->name, st);
			} else {
				pushident(*id, st);
			}
			execute(body);
			j++;
		}
	}
	if (j)
		popident(*id);
}


/**
 * Return help string for command; Common for server and irc
 */
void common_commandhelp(vector<cmd_handler> &handlers, const char* cmd_name)
{
	cmd_handler handler;
	//searching for command with name cmd_name in registered handlers
	int i = 0;
	int len = handlers.length();
	while (i < len)
	{
		if (strcmp(cmd_name, handlers[i].cmd_name) == 0) {
			handler = handlers[i];
			result(handler.cmd_help);
			return;
		}
		i++;
	}
}

/**
 * Registers handler function for command; Common for server and irc
 */
void common_registercommand(vector<cmd_handler> &handlers, const char* cmd_name, const char* cmd_func, int *cmd_perm, const char* cmd_descr, const char* cmd_help)
{
	if (cmd_name[0] && cmd_func[0]) {
		cmd_handler cmd;
		cmd.cmd_descr = newstring(cmd_descr);
		cmd.cmd_name = newstring(cmd_name);
		cmd.cmd_func = newstring(cmd_func);
		cmd.cmd_help = newstring(cmd_help);
		cmd.cmd_permissions = *cmd_perm;
		//find position to add command - for sorted list
		int i = 0;
		bool b = false;
		while (i < handlers.length() && !b) {
			if (strcmp(cmd_name, handlers[i].cmd_name) < 0) {
				b = true;
			} else {
				i++;
			}
		}
		if (b) {
			handlers.insert(i, cmd);
		} else {
			handlers.add(cmd);
		}
	}
}

/**
 * Delete command; Common for server and irc
 */
void common_unregistercommand(vector<cmd_handler> &handlers, const char* cmd_name)
{
	//searching for command with name cmd_name in registered handlers
	for (int i = 0; i < handlers.length(); i++)
	{
		if (strcmp(cmd_name, handlers[i].cmd_name) == 0) {
			DELETEA(handlers[i].cmd_descr);
			DELETEA(handlers[i].cmd_name);
			DELETEA(handlers[i].cmd_func);
			DELETEA(handlers[i].cmd_help);
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
vector<cmd_handler> cmd_handlers;


/**
 * returns player's permissions.
 * if function grantperm is defined  uses it result
 */
int getperm_(int *cn) {

	//check if grantperm exists
	ident *i = getident("grantperm");
	if (i && i->type == ID_ALIAS) {
		char *cmd = newstring("grantperm ");
		cmd = concatpstring(cmd, intstr(*cn));
		int ret = execute(cmd);
		DELETEA(cmd);
		return ret;
	}
	if (isadmin(cn)) {
		return 2;
	} else if (ismaster(cn)) {
		return 3;
	} else {
		return 1;
	}
}

void getperm(int *cn) {
	result(intstr(getperm_(cn)));
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
		remod::onevent("oncommandunknown", "is", cn, cmd_name);
		return;
	}
	cmd_handler handler = cmd_handlers[found];

	//checking for permission

	if (!checkperm(cn, handler.cmd_permissions))
	{
		//permission error
		remod::onevent("oncommandpermerror", "is", cn, cmd_name);
		return;
	}

	int ret = execute_command(handler, intstr(cn), cmd_params);

	if (ret == -1) {
		//usage error - cannot parse cmd_params in accordance with cmd_descr
		remod::onevent("oncommandusageerror", "is", cn, cmd_name);
	} else if(ret == -2) {
		//permission error
		remod::onevent("oncommandpermerror", "is", cn, cmd_name);
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



COMMAND(registercommand, "ssiss");
COMMAND(unregistercommand, "s");
COMMAND(commandhelp, "s");
COMMAND(getperm, "i");
/**
 * Loops through registered server commands with permission "permissions" for executing
 */
ICOMMAND(loopcommands, "sis", (char *var, int *permissions, char *body), common_loopcommands(var, permissions, body, cmd_handlers));


//-----------------------------------------------------------------
// IRC COMMANDS

#ifdef IRC

vector<cmd_handler> irc_cmd_handlers; //IRC command handlers


/**
 * returns user's permissions.
 * if function irc_grantperm is defined  uses it result
 */
int irc_getperm_(char *usr) {
	//check if irc_grantperm exists
	ident *i = getident("irc_grantperm");
	if (i && i->type == ID_ALIAS) {
		char *cmd = newstring("irc_grantperm ");
		cmd = concatpstring(cmd, usr);
		int ret = execute(cmd);
		DELETEA(cmd);
		return ret;
	}
	if (irc_user_state(usr, VOICE)) {
		return 1;
	} else if (irc_user_state(usr, OP)) {
		return 2;
	} else {
		return 0;
	}
}

void irc_getperm(char *usr) {
	result(intstr(irc_getperm_(usr)));
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
		remod::onevent("irc_oncommandunknown", "ss", user, cmd_name);
		return;
	}

	cmd_handler handler = irc_cmd_handlers[found];

	//checking for permission
	string usr;
	strcpy(usr, user);
	if (!irc_checkperm(usr, handler.cmd_permissions))
	{
		//permission error
		remod::onevent("irc_oncommandpermerror", "ss", user, cmd_name);
		return;
	}

	int ret = execute_command(handler, user, cmd_params);
	if (ret == -1) {
		//usage error - cannot parse cmd_params in accordance with cmd_descr
		remod::onevent("irc_oncommandusageerror", "ss", user, cmd_name);
	} else if(ret == -2) {
		//permission error
		remod::onevent("irc_oncommandpermerror", "ss", user, cmd_name);
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

COMMAND(irc_registercommand, "ssiss");
COMMAND(irc_unregistercommand, "s");
COMMAND(irc_commandhelp, "s");
COMMAND(irc_getperm, "s");
/**
 * Loops through registered irc commands with permission "permissions" for executing
 */
ICOMMAND(irc_loopcommands, "sis", (char *var, int *permissions, char *body), common_loopcommands(var, permissions, body, irc_cmd_handlers));

#endif

}
