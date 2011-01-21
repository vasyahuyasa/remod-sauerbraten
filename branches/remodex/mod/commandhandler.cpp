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
bool parsecommandparams(const char *descr, const char *params, vector<cmd_param> *res)
{
	bool oblig_params = true; //current parameter is obligatory
	int len = strlen(descr);
	string p; //temporary variable containing param-string
	strcpy(p, params);
	for (int i = 0; i < len; i++) {
		char ch = descr[i];
		if (ch == '|') {
			oblig_params = false;
		} else {
			if (strlen(p) == 0) { //no more parameters
				if (!oblig_params) {
					cmd_param cp;
					cp.type = ch;
					strcpy(cp.param, "");
					res->add(cp);
					continue; //parameter is not required. add empty string
				} else {
					return false; //parameter is required. error.
				}
			}

			string param;
			char* space_pos = strchr(p, ' '); //searching space character in
			if (ch == 's' || space_pos == 0) { //if parameter is string or space not found
				strcpy(param, p); //residual param string as current parameter
				p[0] = '\0';
			} else {
				int j = 0;
				while (strchr(space_pos+1, ' ') == space_pos+1) { // fuck up multiple spaces
					space_pos = space_pos+1;
					j++;
				}
				if (j > 0) {
					strncpy(space_pos-j, "", j);
				}
				//param = all_before_space in p
				//p = all_after_space in p
				strncpy(param, p, space_pos-p);
				param[space_pos-p] = '\0';
				strcpy(p, space_pos+1);
			}

			int cn;
			//valdating param
			switch (ch) {
			case 'i': //integer
				if (!is_int(param)) {
					return false;
				}
				break;
			case 'f': //float
				if (strcmp(param, "0") != 0 && atof(param) == 0) {
					return false;
				}
				break;
			case 'b': //boolean
				if (strcmp(param, "1") == 0 || strcmp(param, "true") == 0 || strcmp(param, "y") == 0) {
					strcpy(param, "1");
				} else if (strcmp(param, "0") == 0 || strcmp(param, "false") == 0 || strcmp(param, "n") == 0) {
					strcpy(param, "0");
				} else {
					return false;
				}
				break;
			case 'c': //client num or player name
				cn = parseplayer(param);
				if (cn == -1) {
					return false;
				}
				strcpy(param, intstr(cn));
				break;
			case 'p': //client num for specified player or -1 for all
				if (strcmp(param, "-1") != 0) {
					cn = parseplayer(param);
					if (cn == -1) {
						return false;
					}
					strcpy(param, intstr(cn));
				}
				break;
			case 'w': //word
				break;
			case 't': //time (integer - seconds  or mm:ss)
				char *colon_pos = strchr(param, ':');
				if (colon_pos != 0) {

					string min, sec;
					strcpy(sec, colon_pos+1);
					strcpy(colon_pos, "");
					strcpy(min, param);
					if (!is_int(min) || !is_int(sec)) {
						return false;
					} else {
						strcpy(param, intstr(atoi(min)*60+atoi(sec)));
					}
				} else {
					if (!is_int(param)) {
						return false;
					}
				}
				break;
			}
			cmd_param cp;
			cp.type = ch;
			strcpy(cp.param, param);
			res->add(cp);
		}
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
	bool parsed = parsecommandparams(handler.cmd_descr, cmd_params, &params);
	if (!parsed) {
		return -1;
	}
	//creating cmd string
	string cmd;
	strcpy(cmd, handler.cmd_func);
	strcat(cmd, " ");
	strcat(cmd, caller); //first parameter is always cn of player
	loopv(params)
	{
		strcat(cmd, " ");
		if (params[i].type == 's') { //for correct string transition in list one must escape it with "
			strcat(cmd, "\"");
			strcat(cmd, params[i].param);
			strcat(cmd, "\"");
		} else {
			strcat(cmd, params[i].param);
		}

	}
	//calling command
	int ret = execute(cmd);
	cmd[0] = '\0';
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
	for (int i = 0; i < handlers.length(); i++)
	{
		cmd_handler handler = handlers[i];
		if (handler.cmd_permissions <= *permission) {
			if (j) {
				aliasa(id->name, newstring(handler.cmd_name));
			} else {
				pushident(*id, newstring(handler.cmd_name));
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
	bool found = false;
	int i = 0;
	int len = handlers.length();
	while (i < len && !found)
	{
		if (strcmp(cmd_name, handlers[i].cmd_name) == 0) {
			handler = handlers[i];
			found = true;
		}
		i++;
	}

	if (found) {
		result(handler.cmd_help);
	}
}

/**
 * Registers handler function for command; Common for server and irc
 */
void common_registercommand(vector<cmd_handler> &handlers, const char* cmd_name, const char* cmd_func, int *cmd_perm, const char* cmd_descr, const char* cmd_help)
{
	if (cmd_name[0] && cmd_func[0]) {
		cmd_handler cmd;
		strcpy(cmd.cmd_descr, cmd_descr);
		strcpy(cmd.cmd_name, cmd_name);
		strcpy(cmd.cmd_func, cmd_func);
		strcpy(cmd.cmd_help, cmd_help);
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
		string cmd;
		strcpy(cmd, "grantperm ");
		strcat(cmd, intstr(*cn));
		int ret = execute(cmd);
		cmd[0] = '\0';
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
	char *r = newstring(intstr(getperm_(cn)));
	result(r);
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
		string cmd;
		strcpy(cmd, "irc_grantperm ");
		strcat(cmd, usr);
		int ret = execute(cmd);
		cmd[0] = '\0';
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
