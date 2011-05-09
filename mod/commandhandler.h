/*
 * commandhandler.h
 *
 *  Created on: Dec 1, 2010
 *      Author: stormchild
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include "fpsgame.h"

/*
 * from command.engine/cpp
 */
extern void aliasa(const char *name, char *action);
extern void pushident(ident &id, char *val);
extern void popident(ident &id);

namespace remod
{
	/**
	 * Here information about command is stored
	 */
    struct cmd_handler
    {
        string cmd_name; //Command name
        string cmd_func; //Callback function
        string cmd_help; //help string
        string cmd_descr; //String containing description of command params: i - integer, b - boolean, c - valid cn or player name, s - string, | - separator between obligatory and non-obligatory params.  ci|s
        int cmd_permissions; //1 - all players, 2 - admins, 3 - masters
    };

    struct cmd_param
    {
    	char param[512];
    	char type;
    };

    /**
     * called when command is triggered
     */
    void oncommand(int cn, const char* cmd_name, const char* cmd_params);

#ifdef IRC
	/**
	* called when irccommand is triggered
	*/
    void irc_oncommand(const char* user, const char* cmd_name, const char* cmd_params);
#endif

}
#endif /* COMMANDHANDLER_H_ */
