#!/usr/bin/perl -w

# Main Chub engine.
# Henrik Vestergaard 2014

use strict;
use DBI;
use Error qw( :try );

#Below: read frequence (in minutes) of source - valid numbers are: 1, 5, 15, 30, 60
my $freq=$ARGV[0];

#Clobal configurations:


#Nothing to configure below

my $dbh;
my ($sth_selNew, $sth_ins);
sub initDB{ 
	$dbh = DBI->connect("DBI:mysql:ha;host=127.0.0.1", "dbuser", "bravo1", { RaiseError => 1, AutoCommit => 1 } );

	my $sql = "select idSOURCE, COMMAND, NAME, DISPLAY_LINE from SOURCE";
	$sql .= " where FREQ <> 60" if ($freq == 30);
	$sql .= " where FREQ <> 60 and FREQ <> 30" if ($freq == 15);
	$sql .= " where FREQ = 1 or FREQ = 5" if ($freq == 5);
	$sql .= " where FREQ = 1" if ($freq == 1);
	$sth_selNew = $dbh->prepare($sql);
	$sth_ins = $dbh->prepare('INSERT INTO RESULT (TIME, SOURCE, VALUE) VALUES (FROM_UNIXTIME(?), ?, ?)');
}


try{
        initDB();
        my $t = time();
     	$sth_selNew->execute();
     	while( my($id, $command, $name, $display_line) = $sth_selNew->fetchrow_array) {
		my $result = qx "sudo python $command";

		if ($display_line) {
			my $line = $display_line - 1;
			qx "sudo python /home/pi/ha/Project/Action/dht_ldc_rgb.py $line 0 $name: $result";
		}
		$sth_ins->execute($t, $id, $result);
	}
}

otherwise{
        my $e = shift;
        print "Otherwise $e\n";
}

finally{
        $dbh->disconnect();
};
