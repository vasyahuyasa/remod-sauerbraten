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

vector<event *> events;   // events queue

typedef vector<evt_handler> eventHandlers; //Event handlers
eventHandlers handlers[NUMEVENTS];

// Conver event name to string
char *event2str(eventType type)
{
    if((type >= 0) && (type < NUMEVENTS))
        return newstring(eventNames[type]);
    else
        return newstring("");
}

eventType str2event(const char *name)
{
    loopi(NUMEVENTS)
        if(strcmp(name, eventNames[i]) == 0) return (eventType)i;
    return CUSTOMEVENT;
}

event* storeevent(eventType etype, const char *custom, const char *fmt, va_list vl)
{
    event *e = new event;
    e->evt_type = etype;

    if(etype == CUSTOMEVENT)
        if(custom && custom[0])
            e->custom = newstring(custom);
        else
            // costom event not defined
            return NULL;

    e->fmt = newstring(fmt);

    // store params
    if(fmt)
        while(char c = *(fmt++))
        {
            evt_param *param = new evt_param;
            param->type = c;
            switch(c)
            {
                case 'i':
                {
                    param->value_i = new int;
                    *param->value_i = va_arg(vl, int);
                    break;
                }


                case 's':
                {
                    param->value_s = newstring(va_arg(vl, char*));
                    break;
                }

                case 'f':
                case 'd':
                {
                    param->value_d = new double;
                    *param->value_d  = va_arg(vl, double);
                    break;
                }

                default:
                {
                    conoutf("unknown format parameter \"%c\"", c);
                    va_arg(vl, int);
                    break;
                }
            }
        }
    return e;
}

void *getarg(vector<evt_param *> &params)
{
    if(params.length())
    {
        void *value;
        evt_param *param = params.remove(0);
        switch(param->type)
        {
            case 'i':
            {
                int *i = new int;
                value = i;
                *i = *param->value_i;
                break;
            }

            case 's':
            {
                value = newstring(param->value_s);
                break;
            }

            case 'd':
            case 'f':
            {
                double *d = new double;
                value = d;
                *d = *param->value_d;
                break;
            }

            default:
                value = NULL;
                break;
        }

        // free resources
        if(param->value) delete param->value;
        delete param;

        return value;
    }
    return NULL;
}

void addevent(eventType etype, const char *custom, const char *fmt, va_list vl)
{
    if(event *e = storeevent(etype, NULL, fmt, vl))
        events.add(e);
}

//Add script callback to event
void addhandler(const char *evt_type, const char *callbackcmd)
{
    if(evt_type && evt_type[0] && callbackcmd && callbackcmd[0])
    {
        eventType etype = str2event(evt_type);
        evt_handler eh;
        eh.evt_type = etype;
        if(etype == CUSTOMEVENT) eh.custom = newstring(evt_type);
        eh.evt_cmd = newstring(callbackcmd);
        handlers[etype].add(eh);
    }
}

void delhandler(const char* evt_type, const char *cmd)
{
    eventType etype = str2event(evt_type);
    loopv(handlers[etype])
    {
        evt_handler &eh = handlers[etype][i];
        if(strcmp(cmd, eh.evt_cmd) == 0)
        {
            DELETEA(eh.evt_cmd);
            if(etype == CUSTOMEVENT) DELETEA(eh.custom);
            handlers[etype].remove(i);
        }
    }
}

bool ishandle(eventType etype)
{
    return handlers[etype].length();
}

#if 1
//Debug
void dumphandlers()
{
    loopi(NUMEVENTS)
        if(handlers[i].length())
            loopvj(handlers[i])
            {
                evt_handler &eh = handlers[i][j];
                conoutf("Handler %s = %s\n", event2str((eventType)eh.evt_type), eh.evt_cmd);
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
    loopi(NUMEVENTS)
        handlers[i].shrink(0);
}

void triggerEvent(event *ev)
{
    switch(ev->evt_type)
    {
        case ONCOMMAND:
        {
            int *cn = (int*)getarg(ev->params);
            const char *command_str = (const char*)getarg(ev->params);;

            //splitting command_string to command_name and command_params
            char *command_name;
            char *command_params;

            const char *spacepos = strstr(command_str, " ");
            if(!spacepos)
            {
                command_name = newstring(command_str);
                command_params = newstring("");
            }
            else
            {
                command_params = newstring(spacepos+1);
                command_name = newstring(command_str, (size_t) (spacepos - command_str));
            }

            // execute command
            remod::oncommand(*cn, command_name, command_params);

            delete(cn);
            DELETEA(command_str);
            DELETEA(command_name);
            DELETEA(command_params);
            return;
        }

        #ifdef IRC
        case IRC_ONCOMMAND:
        {
            //getting username
            const char *user = (const char*)getarg(ev->params);
            const char *command_str = (const char*)getarg(ev->params);;

            //splitting command_string to command_name and command_params
            char *command_name;
            char *command_params;

            const char *spacepos = strstr(command_str, " ");
            if(!spacepos)
            {
                command_name = newstring(command_str);
                command_params = newstring("");
            }
            else
                {
                command_params = newstring(spacepos+1);
                command_name = newstring(command_str, (size_t) (spacepos - command_str));
            }

            // execute irc command
            remod::irc_oncommand(user, command_name, command_params);

            DELETEA(user);
            DELETEA(command_str);
            DELETEA(command_name);
            DELETEA(command_params);

            return;
        }
        #endif

        default:
        {
            if(ishandle(ev->evt_type))
            {

            }
        }
    }
}

//Trigger spescified event
void triggerEvent(eventType etype, const char *custom,  const char *fmt, va_list vl)
{
    if((etype < 0) || (etype >= NUMEVENTS)) return;

    //if oncommand
    if(etype == ONCOMMAND)
    {
    	//getting cn
		int cn = va_arg(vl, int);
		const char *command_str = newstring(va_arg(vl, const char *));

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

    	return;

    } //if irc_oncommand
    #ifdef IRC
    else if (etype == IRC_ONCOMMAND)
    {
    	//getting username
		const char *user = newstring(va_arg(vl, const char *));
		const char *command_str = newstring(va_arg(vl, const char *));

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

    	return;
    }
    #endif
    //If handler defined
    else if (ishandle(etype))
    {
    	int paramcount = strlen(fmt);
    	char *evparams = newstring("");
        //Check params
        if(paramcount>0)
        {
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
        }

        //Process handlers
        if(etype != CUSTOMEVENT) // standart event
        {
            loopv(handlers[etype])
            {
                evt_handler &eh = handlers[etype][i];
                char *evcmd = newstring(eh.evt_cmd);
                concatpstring(&evcmd, evparams);
                execute(evcmd);
                DELETEA(evcmd);
            }
        }
        else if(custom && custom[0]) // custom user script triggered event
        {
            loopv(handlers[etype])
            {
                evt_handler &eh = handlers[etype][i];
                if(strcmp(custom, eh.custom) == 0)
                {
                    char *evcmd = newstring(eh.evt_cmd);
                    concatpstring(&evcmd, evparams);
                    execute(evcmd);
                    DELETEA(evcmd);
                }
            }
        }

        DELETEA(evparams);
    }
}

// add event to queue
void onevent(eventType etype, const char *fmt, ...)
{
    if((etype < 0) || (etype >= NUMEVENTS)) return;

    va_list vl;
    va_start(vl, fmt);
    //addevent(etype, NULL, fmt, vl);
    triggerEvent(etype, NULL, fmt, vl);
    va_end(vl);
}

// execute event instantly
void oneventi(eventType etype, const char *fmt, ...)
{
    if((etype < 0) || (etype >= NUMEVENTS)) return;

    va_list vl;
    va_start(vl, fmt);
    triggerEvent(etype, NULL, fmt, vl);
    va_end(vl);
}

bool onevent(const char *evt_type, const char *fmt, ...)
{
    eventType etype = str2event(evt_type);
    va_list vl;
    va_start(vl, fmt);

    // depricated api usage
    if(etype != CUSTOMEVENT) conoutf("remod::onevent(\"%s\" ...) is depricated", evt_type);

    //addevent(etype, etype == CUSTOMEVENT ? evt_type : NULL, fmt, vl);
    triggerEvent(etype, etype == CUSTOMEVENT ? evt_type : NULL, fmt, vl);

    va_end(vl);
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
