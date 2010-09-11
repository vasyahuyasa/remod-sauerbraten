#include "commandev.h"

namespace remod
{
vector<const char *> eat; //Do not allow server process that events
vector<evt_handler> evt_handlers; //Event handlers


void addeat(const char *evt_type)
{
    loopv(eat)
    {
        const char *e = eat[i];
        if(strcmp(evt_type, e) == 0) return;
    }
    eat.add(evt_type);
}

void deleat(const char *evt_type)
{
    loopv(eat)
    {
        const char *e = eat[i];
        if(strcmp(evt_type, e) == 0) eat.remove(i);
    }
}

bool iseat(const char *evt_type)
{
    loopv(eat)
    {
        const char *e = eat[i];
        if(strcmp(evt_type, e) == 0) return true;
    }
    return false;
}

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
        printf("Handler %s = %s\n", evt_handlers[i].evt_type, evt_handlers[i].evt_cmd);
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

    //If handler defined
    if(ishandle(evt_type))
    {
        int paramcount = strlen(fmt);

        //Check params
        if(paramcount>0)
        {
            string str; //Temp string
            va_list vl;
            va_start(vl, fmt);

            //Convert params to string
            for(int i=0; i<paramcount; i++)
            {
                strcat(evparams, " ");
                switch(fmt[i])
                {
                case 'i':
                    strcat(evparams, itoa(va_arg(vl, int), str, 10));
                    break;
                case 's':
                    strcat(evparams, "\"");
                    strcat(evparams, va_arg(vl, const char *));
                    strcat(evparams, "\"");
                    break;
                case 'd':
                    strcat(evparams, gcvt(va_arg(vl, double), 5, str));
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

    if(iseat(evt_type))
    {
        return true;
    }
    else
    {
        return false;
    }

}

COMMAND(addeat, "i");
COMMAND(deleat, "i");
COMMAND(addhandler, "ss");
COMMAND(delhandler, "ss");
COMMAND(clearhandlers, "");

}
