#!/usr/bin/perl -w


#    Copyright (C) 2015 Henrik Vestergaard
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
#
#   Main Chub engine.
#   Henrik Vestergaard 2014-2015

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

	my $sql = "select idSOURCE, COMMAND, NAME, DISPLAY_LINE, DISPLAY_COL from SOURCE";
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
     	while( my($id, $command, $name, $display_line, $display_col) = $sth_selNew->fetchrow_array) {
		my $result = qx "$command";
print "		qx /home/pi/ha/Project/Action/sendText -c $display_col -r $display_line -t '$name: $result'";
		qx "/home/pi/ha/Project/Action/sendText -c $display_col -r $display_line -t '$name: $result'";
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
