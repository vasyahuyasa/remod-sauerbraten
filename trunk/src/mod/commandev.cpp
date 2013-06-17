/*
* remod:    commandev.cpp
* date:     2007
* author:   degrave, stormchild
*
* events and events handlers
*/


#include "commandev.h"
#include "commandhandler.h"
#include "remod.h"

extern char *strreplace(const char *s, const char *oldval, const char *newval);

namespace remod
{
vector<evt_handler> evt_handlers; //Event handlers



//Add script callback to event
void addhandler(const char *evt_type, const char *callbackcmd)
{
    if(evt_type && evt_type[0] && callbackcmd && callbackcmd[0])
    {
        evt_handler eh;
        eh.evt_cmd = newstring(callbackcmd);
        eh.evt_type = newstring(evt_type);
        evt_handlers.add(eh);
    }

}

void delhandler(const char* evt_type, const char *cmd)
{
    for(int i=0; i<evt_handlers.length(); i++)
    {
    	evt_handler eh = evt_handlers[i];
        if ((strcmp(eh.evt_type, evt_type) == 0) && (strcmp(eh.evt_cmd, cmd) == 0)) {
        	DELETEA(eh.evt_cmd);
        	DELETEA(eh.evt_type);
            evt_handlers.remove(i);
        }
    }
}

bool ishandle(const char *evt_type)
{
    for(int i=0; i<evt_handlers.length(); i++)
    {
        if(strcmp(evt_type, evt_handlers[i].evt_type) == 0)
        {
            return true;
        }
    }
    return false;
}

#if 1
//Debug
void dumphandlers()
{
    for(int i=0; i<evt_handlers.length(); i++)
    {
        conoutf("Handler %s = %s\n", evt_handlers[i].evt_type, evt_handlers[i].evt_cmd);
    }
}
/**
 * Print all event handlers (debug mode only)
 * @group event
 * @example dumphandlers
 */
COMMAND(dumphandlers, "");
#endif

void clearhandlers()
{
    evt_handlers.shrink(0);
}



//Return true - eat server handler, false allow server to handle
bool onevent(const char *evt_type, const char *fmt, ...)
{
    //if oncommand
    if (strcmp(evt_type, "oncommand") == 0)
    {
    	//getting cn
    	va_list vl;
		va_start(vl, fmt);
		int cn = va_arg(vl, int);
		const char *command_str = newstring(va_arg(vl, const char *));
		va_end(vl);

		//splitting command_string to command_name and command_params
		char *command_name;
		char *command_params;



		const char *spacepos = strstr(command_str, " ");
		if (!spacepos) {
			command_name = newstring(command_str);
			command_params = newstring("");
		} else {
			command_params = newstring(spacepos+1);
			command_name = newstring(command_str, (size_t) (spacepos - command_str));
		}


		//calling server command
		remod::oncommand(cn, command_name, command_params);

		DELETEA(command_name);
		DELETEA(command_params);
		DELETEA(command_str);

    	//eat it!
    	return true;

    } //if irc_oncommand
    #ifdef IRC
    else if (strcmp(evt_type, "irc_oncommand") == 0)
    {
    	//getting username
    	va_list vl;
		va_start(vl, fmt);
		const char *user = newstring(va_arg(vl, const char *));
		const char *command_str = newstring(va_arg(vl, const char *));
		va_end(vl);

		//splitting command_string to command_name and command_params
		char *command_name;
		char *command_params;

		const char *spacepos = strstr(command_str, " ");
		if (!spacepos) {
			command_name = newstring(command_str);
			command_params = newstring("");
		} else {
			command_params = newstring(spacepos+1);
			command_name = newstring(command_str, (size_t) (spacepos - command_str));
		}

		//calling irc command
		remod::irc_oncommand(user, command_name, command_params);

		DELETEA(user);
		DELETEA(command_str);
		DELETEA(command_name);
		DELETEA(command_params);
    	//eat it!
    	return true;

    }
    #endif
    //If handler defined
    else if (ishandle(evt_type))
    {
    	int paramcount = strlen(fmt);
    	char *evparams = newstring("");
        //Check params
        if(paramcount>0)
        {
            va_list vl;
            va_start(vl, fmt);

            //Convert params to string
            for(int i=0; i<paramcount; i++)
            {
                const char* p;
                switch(fmt[i])
                {
                case 'i':
                	concatpstring(&evparams, 2, " ", intstr(va_arg(vl, int)));
                    break;
                case 's':
                    p = va_arg(vl, const char *);
                    if (p) {
                    	concatpstring(&evparams, 2, " ", escapestring(p));
                    } else {
                    	concatpstring(&evparams, "\"\"");
                    }
                    break;
                case 'f':
                case 'd':
                	concatpstring(&evparams, 2, " ", floatstr(va_arg(vl, double)));
                    break;
                default:
                    //Read and forgot
                    va_arg(vl, int);
                    break;
                }
            }
            va_end(vl);
        }

        //Process handlers
        for(int i=0; i<evt_handlers.length(); i++)
        {
            if(strcmp(evt_type, evt_handlers[i].evt_type) == 0)
            {
            	char *evcmd = newstring(evt_handlers[i].evt_cmd);

            	concatpstring(&evcmd, evparams);

                execute(evcmd);

                DELETEA(evcmd);
            }
        }
        DELETEA(evparams);
    }

    return false;

}


/**
 * Add server event handler to specified event
 * @group event
 * @arg1 event name
 * @arg2 callback function
 * @example addhandler "onconnect" [ echo (format "CONNECT %1(%3)" (getname $arg1) $arg1 ) ]
 */
COMMAND(addhandler, "ss");

/**
 * Add server event handler to specified event. Parameters to callback function depend of event
 * @group event
 * @arg1 event name
 * @arg2 callback function
 * @example delhandler "onconnect" log_onconnect
 */
COMMAND(delhandler, "ss");

/**
 * Clear all server events handlers
 * @group event
 */
COMMAND(clearhandlers, "");

}