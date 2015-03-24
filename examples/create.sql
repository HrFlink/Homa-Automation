 mysqldump rms trafficalarm --compact --no-data


SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `trafficalarm` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `router` varchar(50) NOT NULL,
  `ifname` varchar(50) NOT NULL,
  `alarmlevel` tinyint(4) DEFAULT NULL,
  `state` tinyint(4) DEFAULT NULL,
  `statetime` bigint(20) DEFAULT NULL,
  `maxbits` bigint(20) DEFAULT NULL,
  `maxtime` bigint(20) DEFAULT NULL,
  `minbits` bigint(20) DEFAULT NULL,
  `mintime` bigint(20) DEFAULT NULL,
  `inminbits` bigint(20) DEFAULT NULL,
  `inmintime` bigint(20) DEFAULT NULL,
  `inmaxtime` bigint(20) DEFAULT NULL,
  `inmaxbits` bigint(20) DEFAULT NULL,
  `inserted` bigint(20) DEFAULT NULL,
  `curinbits` bigint(20) DEFAULT NULL,
  `curoutbits` bigint(20) DEFAULT NULL,
  `active` tinyint(4) DEFAULT NULL,
  `ignored` tinyint(4) DEFAULT NULL,
  `severity` tinyint(4) DEFAULT NULL,
  `grouptext` varchar(30) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=24072 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

