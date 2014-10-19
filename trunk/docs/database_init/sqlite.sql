--auth
CREATE TABLE auth (id INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR(30), pubkey VARCHAR(49), rights VARCHAR(1), enabled TINYINT, unique(id, name));

--flagrun
CREATE TABLE flagrun (mode UNSIGNED TINYINT, map VARCHAR(20), name VARCHAR(20), time UNSIGNED INTEGER, unique(mode, map));

--spy
CREATE TABLE spy (ip INTEGER, name VARCHAR(20), lastseen UNSIGNED INTEGER, unique(ip, name));

--matchinfo
CREATE TABLE matchinfo ( 
    id        INTEGER PRIMARY KEY
                      NOT NULL,
    mode      INTEGER NOT NULL,
    map       TEXT    NOT NULL,
    players   INTEGER,
    date      INTEGER NOT NULL,
    teamscore TEXT,
    server    TEXT,
    demo      TEXT 
);

CREATE TABLE playerstats ( 
    id           INTEGER       PRIMARY KEY
                               NOT NULL,
    matchid      INTEGER       REFERENCES matchinfo ( id ),
    loginid      INTEGER,
    ip           INTEGER,
    name         TEXT,
    team         VARCHAR( 4 ),
    frags        INTEGER       NOT NULL,
    deaths       INTEGER       NOT NULL,
    flags        INTEGER,
    damage       INTEGER       NOT NULL,
    damagewasted INTEGER       NOT NULL,
    acc          INTEGER       NOT NULL,
    acc0         INTEGER,
    acc1         INTEGER,
    acc2         INTEGER,
    acc3         INTEGER,
    acc4         INTEGER,
    acc5         INTEGER,
    acc6         INTEGER,
    dmg0         INTEGER,
    dmg1         INTEGER,
    dmg2         INTEGER,
    dmg3         INTEGER,
    dmg4         INTEGER,
    dmg5         INTEGER,
    dmg6         INTEGER,
    waste0       INTEGER,
    waste1       INTEGER,
    waste2       INTEGER,
    waste3       INTEGER,
    waste4       INTEGER,
    waste5       INTEGER,
    waste6       INTEGER 
);

