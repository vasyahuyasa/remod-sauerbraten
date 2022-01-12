/*
 * commandhandler_irc.h
 *
 *  Created on: Dec 1, 2010
 *      Author: stormchild
 *  Renamed on Jan 12, 2022
 *      Author: degrave
 */
#ifndef COMMANDHANDLER_IRC_H_
#define COMMANDHANDLER_IRC_H_

#include "fpsgame.h"

namespace remod
{
	/**
	* called when irccommand is triggered
	*/
    void irc_oncommand(const char* user, const char* cmd_name, const char* cmd_params);
}
#endif /* COMMANDHANDLER_IRC_H_ */
