/*
* remod:    commandev.cpp
* date:     2007
* author:   degrave, stormchild
*
* events and events handlers
*/


#include "commandev.h"
#include "commandhandler.h"
namespace remod
{
vector<evt_handler> evt_handlers; //Event handlers



//Add script callback to event
void addhandler(const char *evt_type, const char *callbackcmd)
{
    if(evt_type[0] && callbackcmd[0])
    {
        evt_handler eh;
        strcpy(eh.evt_type, evt_type);
        strcpy(eh.evt_cmd, callbackcmd);
        evt_handlers.add(eh);
    }

}

void delhandler(const char* evt_type, const char *cmd)
{
    for(int i=0; i<evt_handlers.length(); i++)
    {
        if((strcmp(evt_handlers[i].evt_type, evt_type) == 0) && (strcmp(evt_handlers[i].evt_cmd, cmd) == 0))
            evt_handlers.remove(i);
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
COMMAND(dumphandlers, "");
#endif

void clearhandlers()
{
    evt_handlers.shrink(0);
}

//Return true - eat server handler, false allow server to handle
bool onevent(const char *evt_type, const char *fmt, ...)
{
    string evcmd = "";
    string evparams = "";

    //if oncommand
    if (strcmp(evt_type, "oncommand") == 0)
    {
    	//getting cn
    	va_list vl;
		va_start(vl, fmt);
		int cn = va_arg(vl, int);
		string command_str;
		strncpy(command_str, va_arg(vl, const char *), 220);
		va_end(vl);

		//splitting command_string to command_name and command_params
		string command_name, command_params;

		char* spacepos = strstr(command_str, " ");
		if (!spacepos) {
			strcpy(command_name, command_str);
			command_params[0] = '\0';
		} else {
			strcpy(command_params, spacepos+1);
			spacepos[0] = '\0';
			strcpy(command_name, command_str);
		}
		//calling server command
		remod::oncommand(cn, command_name, command_params);

    	//eat it!
    	return true;

    } //if irc_oncommand
    #ifdef IRC
    else if (strcmp(evt_type, "irc_oncommand") == 0)
    {
    	//getting username
    	va_list vl;
		va_start(vl, fmt);
		string user;

		strncpy(user, va_arg(vl, const char *), 220);

		string command_str;
		strncpy(command_str, va_arg(vl, const char *), 220);
		va_end(vl);

		//splitting command_string to command_name and command_params
		string command_name, command_params;

		char* spacepos = strstr(command_str, " ");
		if (spacepos == 0) {
			strcpy(command_name, command_str);
			strcpy(command_params, "");
		} else {
			strcpy(command_params, spacepos+1);
			strcpy(spacepos, "");
			strcpy(command_name, command_str);
		}

		//calling irc command
		remod::irc_oncommand(user, command_name, command_params);

    	//eat it!
    	return true;

    }
    #endif
    //If handler defined
    else if (ishandle(evt_type))
    {
    	int paramcount = strlen(fmt);
        //Check params
        if(paramcount>0)
        {
            va_list vl;
            va_start(vl, fmt);

            //Convert params to string
            for(int i=0; i<paramcount; i++)
            {
                strcat(evparams, " ");
                const char* p;
                switch(fmt[i])
                {
                case 'i':
                    strcat(evparams, intstr(va_arg(vl, int)));
                    break;
                case 's':
                    strcat(evparams, "\"");
                    p = va_arg(vl, const char *);
                    if (p) {
                    	strcat(evparams, p);
                    }
                    strcat(evparams, "\"");
                    break;
                case 'd':
                    strcat(evparams, floatstr(va_arg(vl, double)));
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
                strcat(evcmd, evt_handlers[i].evt_cmd);
                strcat(evcmd, evparams);
                execute(evcmd);
                evcmd[0] = '\0';
            }
        }
    }

    return false;

}

COMMAND(addhandler, "ss");
COMMAND(delhandler, "ss");
COMMAND(clearhandlers, "");

}
