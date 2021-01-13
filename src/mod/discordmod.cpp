#include "dlfcn.h"
#include "discordmod.h"
#include "commandev.h"

namespace remod
{
    namespace discord
    {
        SVAR(discordbottoken, "");

        int initplugin(const char *path, pluginapi *api)
        {
            void *dynlib = dlopen(path, RTLD_NOW);
            if (!dynlib)
            {
                // TODO print error
                return 1;
            }

            void *fversion = dlsym(dynlib, "version");
            if (!fversion)
            {
                // TODO print error
                return 1;
            }

            void *frun = dlsym(dynlib, "run");
            if (!frun)
            {
                // TODO print error
                return 1;
            }

            void *fsendmessage = dlsym(dynlib, "sendmessage");
            if (!fsendmessage)
            {
                // TODO print error
                return 1;
            }

            void *flasterror = dlsym(dynlib, "lasterror");
            if (!flasterror)
            {
                // TODO print error
                return 1;
            }

            api->version = (int (*)())fversion;
            api->run = (int (*)(messagecallback *, const char *))frun;
            api->sendmessage = (int (*)(const char *, const char *))fsendmessage;
            api->lasterror = (const char *(*)())flasterror;

            return 0;
        }


    } // namespace discord
} // namespace remod