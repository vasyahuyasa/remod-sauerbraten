//Remod
namespace remod
{
    using namespace server;

    void getmap();
    void getmode();
    void getip(int *cn);
    void getfrags(int *cn);
    void getdeaths(int *cn);
    void getteamkills(int *cn);
    void getaccuracy(int *cn);
    void getflags(int *cn);
    void getmastermode();
    void getmastermodename(int *mm);
    bool ismaster(int *cn);
    bool isadmin(int *cn);
    bool isspectator(int *cn);
    void version();
}
