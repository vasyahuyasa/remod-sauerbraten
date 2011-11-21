--flagrun
CREATE TABLE flagrun (mode UNSIGNED TINYINT, map VARCHAR(20), name VARCHAR(20), time UNSIGNED INTEGER, unique(mode, map));

--spy
CREATE TABLE spy (ip INTEGER, name VARCHAR(20), lastseen UNSIGNED INTEGER, unique(ip, name));
