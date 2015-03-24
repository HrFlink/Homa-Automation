create table trafficalarm (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, router varchar(50) not null, ifname varchar(50) not null, alarmlevel tinyint(4), inserted bigint(20), state tinyint(4), statetime bigint(20), maxbits bigint(20), maxtime bigint(20), minbits bigint(20), mintime bigint(20) );

LTER TABLE trafficalarm ADD curinbits bigint(20);
ALTER TABLE trafficalarm ADD curoutbits bigint(20);
ALTER TABLE trafficalarm ADD active tinyint(4);
ALTER TABLE trafficalarm ADD ignored tinyint(4);
ALTER TABLE trafficalarm ADD severity tinyint(4);
ALTER TABLE trafficalarm ADD grouptext varchar(30);
