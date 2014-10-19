--
-- auth
--

CREATE TABLE IF NOT EXISTS `auth` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(30) NOT NULL,
  `pubkey` varchar(49) NOT NULL,
  `rights` VARCHAR(1) NOT NULL,
  `enabled` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

--
-- flagrun
--

CREATE TABLE IF NOT EXISTS `flagrun` (
  `mode` tinyint(3) unsigned NOT NULL,
  `map` varchar(20),
  `name` varchar(20),
  `time` int(10) unsigned NOT NULL,
  PRIMARY KEY (`mode`,`map`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

--
-- spy
--

CREATE TABLE IF NOT EXISTS `spy` (
  `ip` int(4) NOT NULL,
  `name` varchar(20) NOT NULL,
  `lastseen` int(10) unsigned NOT NULL,
  `cnt` int(10) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`ip`,`name`),
  KEY `name` (`name`),
  KEY `ip` (`ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

--
-- matchinfo
--

CREATE TABLE IF NOT EXISTS matchinfo ( 
    id        INTEGER PRIMARY KEY
                      NOT NULL,
    mode      INTEGER NOT NULL,
    map       TEXT    NOT NULL,
    players   INTEGER,
    date      INTEGER NOT NULL,
    teamscore TEXT,
    server    TEXT,
    demo      TEXT 
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

CREATE TABLE IF NOT EXISTS playerstats ( 
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

