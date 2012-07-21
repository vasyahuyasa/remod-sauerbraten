#ifndef __AUTHMOD_H__
#define __AUTHMOD_H__

#include "cube.h"
#include "remod.h"

namespace remod
{
    void authchallenged(uint id, const char *val, const char *desc);
    void tryauth(clientinfo *ci, const char *user, const char *desc);
    void answerchallenge(clientinfo *ci, uint id, char *val, const char *desc);
}

namespace authserv
{
    struct userinfo
    {
        char *name;
        void *pubkey;
    };

    struct authreq
    {
        enet_uint32 reqtime;
        uint id;
        void *answer;
    };

    // auth server emulation
    void reqauth(uint id, char *user, const char *desc);
    void confauth(uint id, const char *val);
    void reloadauth();
}
#endif
