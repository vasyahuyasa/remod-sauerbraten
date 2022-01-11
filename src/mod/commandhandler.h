/*
 * commandhandler.h
 *
 *  Created on: Dec 1, 2010
 *      Author: stormchild
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include "fpsgame.h"

namespace remod
{
	/**
	 * Here information about command is stored
	 */
    struct cmd_handler
    {
        char* cmd_name; //Command name
        char* cmd_func; //Callback function
        char* cmd_help; //help string
        char* cmd_descr; //String containing description of command params: i - integer, b - boolean, c - valid cn or player name, s - string, | - separator between obligatory and non-obligatory params.  ci|s
        int cmd_permissions; //1 - all players, 2 - admins, 3 - masters
    };

    struct cmd_param
    {
    	char* param;
    	char type;
    };

    void common_registercommand(vector<cmd_handler*> &handlers, const char* cmd_name, const char* cmd_func, int *cmd_perm, const char* cmd_descr, const char* cmd_help);

    void common_unregistercommand(vector<cmd_handler*> &handlers, const char* cmd_name);

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
