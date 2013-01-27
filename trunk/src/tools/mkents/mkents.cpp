//*****************/
//   degrave 2013
//   remod
//*****************/

#include "cube.h"
#include "world.h"

namespace game
{
const char* gameident() { return "cube"; }
}

void conoutf(int, char const*, ...)
{
}

void cutogz(char *s)
{
    char *ogzp = strstr(s, ".ogz");
    if(ogzp) *ogzp = '\0';
}

static void fixent(entity &e, int version)
{
    if(version <= 10 && e.type >= 7) e.type++;
    if(version <= 12 && e.type >= 8) e.type++;
    if(version <= 14 && e.type >= ET_MAPMODEL && e.type <= 16)
    {
        if(e.type == 16) e.type = ET_MAPMODEL;
        else e.type++;
    }
    if(version <= 20 && e.type >= ET_ENVMAP) e.type++;
    if(version <= 21 && e.type >= ET_PARTICLES) e.type++;
    if(version <= 22 && e.type >= ET_SOUND) e.type++;
    if(version <= 23 && e.type >= ET_SPOTLIGHT) e.type++;
    if(version <= 30 && (e.type == ET_MAPMODEL || e.type == ET_PLAYERSTART)) e.attr1 = (int(e.attr1)+180)%360;
    if(version <= 31 && e.type == ET_MAPMODEL) { int yaw = (int(e.attr1)%360 + 360)%360 + 7; e.attr1 = yaw - yaw%15; }
}

bool loadents(const char *fname, vector<entity> &ents, uint *crc)
{
    string pakname, mapname, mcfgname, ogzname;
    copystring(mapname, fname, 100);
    cutogz(mapname);
    formatstring(ogzname)("%s.ogz", mapname);
    path(ogzname);
    stream *f = opengzfile(ogzname, "rb");
    if(!f) return false;
    octaheader hdr;
    if(f->read(&hdr, 7*sizeof(int))!=int(7*sizeof(int))) { conoutf(CON_ERROR, "map %s has malformatted header", ogzname); delete f; return false; }
    lilswap(&hdr.version, 6);
    if(memcmp(hdr.magic, "OCTA", 4) || hdr.worldsize <= 0|| hdr.numents < 0) { conoutf(CON_ERROR, "map %s has malformatted header", ogzname); delete f; return false; }
    if(hdr.version>MAPVERSION) { conoutf(CON_ERROR, "map %s requires a newer version of Cube 2: Sauerbraten", ogzname); delete f; return false; }
    compatheader chdr;
    if(hdr.version <= 28)
    {
        if(f->read(&chdr.lightprecision, sizeof(chdr) - 7*sizeof(int)) != int(sizeof(chdr) - 7*sizeof(int))) { conoutf(CON_ERROR, "map %s has malformatted header", ogzname); delete f; return false; }
    }
    else
    {
        int extra = 0;
        if(hdr.version <= 29) extra++;
        if(f->read(&hdr.blendmap, sizeof(hdr) - (7+extra)*sizeof(int)) != int(sizeof(hdr) - (7+extra)*sizeof(int))) { conoutf(CON_ERROR, "map %s has malformatted header", ogzname); delete f; return false; }
    }

    if(hdr.version <= 28)
    {
        lilswap(&chdr.lightprecision, 3);
        hdr.blendmap = chdr.blendmap;
        hdr.numvars = 0;
        hdr.numvslots = 0;
    }
    else
    {
        lilswap(&hdr.blendmap, 2);
        if(hdr.version <= 29) hdr.numvslots = 0;
        else lilswap(&hdr.numvslots, 1);
    }

    loopi(hdr.numvars)
    {
        int type = f->getchar(), ilen = f->getlil<ushort>();
        f->seek(ilen, SEEK_CUR);
        switch(type)
        {
            case ID_VAR: f->getlil<int>(); break;
            case ID_FVAR: f->getlil<float>(); break;
            case ID_SVAR: { int slen = f->getlil<ushort>(); f->seek(slen, SEEK_CUR); break; }
        }
    }

    string gametype;
    copystring(gametype, "fps");
    bool samegame = true;
    int eif = 0;
    if(hdr.version>=16)
    {
        int len = f->getchar();
        f->read(gametype, len+1);
    }
    if(strcmp(gametype, game::gameident()))
    {
        samegame = false;
        conoutf(CON_WARN, "WARNING: loading map from %s game, ignoring entities except for lights/mapmodels", gametype);
    }
    if(hdr.version>=16)
    {
        eif = f->getlil<ushort>();
        int extrasize = f->getlil<ushort>();
        f->seek(extrasize, SEEK_CUR);
    }

    if(hdr.version<14)
    {
        f->seek(256, SEEK_CUR);
    }
    else
    {
        ushort nummru = f->getlil<ushort>();
        f->seek(nummru*sizeof(ushort), SEEK_CUR);
    }
    // remod
    samegame = true;
    loopi(min(hdr.numents, MAXENTS))
    {
        entity &e = ents.add();
        f->read(&e, sizeof(entity));
        lilswap(&e.o.x, 3);
        lilswap(&e.attr1, 5);
        fixent(e, hdr.version);
        if(eif > 0) f->seek(eif, SEEK_CUR);
        if(samegame)
        {
            entities::readent(e, NULL, hdr.version);
        }
        else if(e.type>=ET_GAMESPECIFIC || hdr.version<=14)
        {
            ents.pop();
            continue;
        }
    }

    if(crc)
    {
        f->seek(0, SEEK_END);
        *crc = f->getcrc();
    }

    delete f;

    return true;
}

// writeents
// (c) 2011 Thomas
// hopmod 
bool writeents(const char *mapname, vector<entity> &ents, uint mapcrc)
{
    string file;
    formatstring(file)("%s", mapname);

    stream *mapi = opengzfile(path(file), "w+b");

    if (!mapi) return false;

    mapi->putstring("MAPENTS");
    mapi->put(mapcrc);
    mapi->put(ents.length());
    mapi->put(0);

    loopv(ents)
    {
        entity &e = ents[i];

        mapi->put(e.type);
        mapi->put(e.attr1);
        mapi->put(e.attr2);
        mapi->put(e.attr3);
        mapi->put(e.attr4);
        mapi->put(e.attr5);
        mapi->put(e.reserved);
        loopk(3) mapi->put(e.o[k]);

        mapi->putlil(0);
    }

    mapi->put(0);
    mapi->put(ents.length());

    mapi->close();
    delete mapi;
    return true;
}

bool mkents(char *fname)
{
    string name, ogzname, entsname;
    copystring(name, fname, 100);
    cutogz(name);

    formatstring(ogzname)("%s.ogz", name);
    formatstring(entsname)("%s.ents", name);

    //printf("processing %s... ", name);

    if(!fileexists(ogzname, "rb"))
    {
        printf("file %s do not exists\n", name);
        return false;
    }

    vector<entity> ments;
    uint mcrc;
    if(!loadents(ogzname, ments, &mcrc))
    {
        printf("error loadents %s.\n", ogzname);
        return false;
    }

    if(!writeents(entsname, ments, mcrc))
    {
        printf("error writeents %s.\n", entsname);
        return false;
    }
    else
        printf("sucessfull %s.\n", entsname);

    return true;
}

void usage(char *name)
{
    printf("Usage: %s file1.ogz [file2.ogz ...]\n", name);
}

int main(int argc, char *argv[])
{
    if(argc == 1)
    {
        usage(argv[0]);
        return 0;
    }

    loopi(argc-1)
    {
        string mapname;
        int argnum = i+1;
        if(*argv[argnum])
        {
            char *arg = argv[argnum];
            copystring(mapname, arg);
            cutogz(mapname);
            mkents(mapname);
        }
    }
}
