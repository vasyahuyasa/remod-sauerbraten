--flagrun
CREATE TABLE IF NOT EXISTS `flagrun` (
  `mode` tinyint(3) unsigned NOT NULL,
  `map` varchar(20) COLLATE latin1_general_ci NOT NULL,
  `name` varchar(20) COLLATE latin1_general_ci NOT NULL,
  `time` int(10) unsigned NOT NULL
  PRIMARY KEY (`mode`,`map`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

--spy
CREATE TABLE IF NOT EXISTS `spy` (
  `ip` int(4) NOT NULL,
  `name` varchar(20) COLLATE latin1_general_ci NOT NULL,
  `lastseen` int(10) unsigned NOT NULL
  PRIMARY KEY (`ip`,`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
