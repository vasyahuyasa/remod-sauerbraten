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
