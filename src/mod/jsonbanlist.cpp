#include "jsonbanlist.h"

EXTENSION(JSONBANLIST);

namespace remod
{
    namespace jsonbanlist
    {
        vector<jsonbanlist*> jsonbanlists;

        // jsonsource
        jsonsource::jsonsource(char *url)
        {
        }

        bool jsonsource::parseurl()
        {
            char *urlstart = url;
            char *urlend = url + strlen(url);

            // cut http if present
            if(strstr(urlstart, "http://"))
                urlstart += strlen("http://");

            //
        }

        bool jsonsource::seturl(char *addr)
        {

        }

        ENetAddress jsonsource::getaddress()
        {
            return address;
        }

        // jsonbanlist
        jsonbanlist::jsonbanlist(const char *listname, bool timed, uint interval)
        {
            name = newstring(listname);
            timedbans = timed;
            updateinterval = interval;
        }

        void jsonbanlist::update(time_t now)
        {
            if(now > (lastupdate + 60*updateinterval))
            {
                // TODO UPDATE LIST
            }
        }

        // functions
        void update()
        {
            time_t now = time(0);

            loopv(jsonbanlists)
            {
                jsonbanlist *jbl = jsonbanlists[0];
                jbl->update(now);
            }
        }

        void addjsonbanlist(char *listname, bool timed, uint interval, char *source)
        {
            jsonsource *js = new jsonsource;

        }

/**
 * Create json banlist (not implemented)
 * @group server
 * @arg1 list name
 * @arg2 1 - get all bans, 0 - only permanent bans
 * @arg3 update interval in minutes
 * @arg4 http source
 */
COMMANDN(jsonbanlist, addjsonbanlist, "siis");

    }
}

