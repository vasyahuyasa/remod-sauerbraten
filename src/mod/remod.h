#ifndef __REMOD_H__
#define __REMOD_H__

#include "game.h"
#include "varbox.h"



// worlio.cpp
extern void cutogz(char *s);

//command.cpp
extern void at(tagval *args, int numargs);
extern void concat(tagval *v, int n);
extern int unescapestring(char *dst, const char *src, const char *end);

// remod.cpp
typedef vector<char *> extensionslist;
extern bool addextension(const char *name);
char *conc(char **w, int n, bool space);
extern const extensionslist* getextensionslist();
extern char *authfile;

void reloadauth();

#define EXTENSION(name) bool __dummyext_##name = addextension(#name)

namespace server
{
    struct clientinfo;

    void filtercstext(char *str);
    bool checkpban(uint ip);
    void addban(int cn, char* actorname, int expire);
    void addpban(char *name, const char *reason);
    void sendservmsgf(const char *fmt, ...);
}

namespace remod
{
    // mutemode
    enum  { MUTEMODE_NONE = 0, MUTEMODE_SPECS, MUTEMODE_PLAYERS, MUTEMODE_MASTERS, NUMMUTEMODE };
    typedef server::clientinfo clientinfo;

    struct extstate
    {
        bool muted;
        bool ghost;

        int lastmutetrigger;
        int lastghosttrigger;

        struct
        {
            int shotdamage;
            int damage;
        } guninfo[NUMGUNS];

        void reset();
    };

    struct extinfo
    {
        varbox vars;
    };

    extern char *mapdir;

    clientinfo* findbest(vector<clientinfo *> &a);
    bool playerexists(int *pcn);
    int parseplayer(const char *arg);
    bool ismaster(int *cn);
    bool isadmin(int *cn);
    bool isspectator(int *cn);
    bool isediting(int *cn);
    void concatpstring(char** str, const char *piece);
    void concatpstring(char** str, int count,  ...);
    void loadbans();
    void writebans();
    bool loadents(const char *fname, vector<entity> &ents, uint *crc);
    bool writeents(const char *mapname, vector<entity> &ents, uint mapcrc);
    void setmaster(clientinfo *ci, int priv);
    int getwepaccuracy(int cn, int gun);
    bool checkmutemode(clientinfo *ci);
    void pausegame(bool val, clientinfo *ci = NULL);
    void checkresume();
    int getteamscore(const char *team);
    bool isteamsequalscore();
    void rename(int cn, const char* name);
    void sendmapto(int cn);
    bool iseditcommand(int type);
    void ghost(int cn, bool v);
}
#endif
