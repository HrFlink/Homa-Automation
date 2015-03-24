#!/usr/lib/perl5/site_perl/5.8.8/perl -w
use DBI;
use strict;

my $dbh = DBI->connect("DBI:mysql:obi;host=87.48.145.150", "a63893", "loJOdAPe", { RaiseError => 1, AutoCommit => 1 }) or die "Connect to DB";

$dbh->do("truncate scustomers");
$dbh->do("truncate fcustomers");
my $customersNQZ = $dbh->prepare("insert ignore into scustomers (switch, lid, description) values (?, ?, ?)");
my $dslamNQZ = $dbh->prepare("insert ignore into scustomers (switch, dslam) values (?, ?)");
my $routerNQZ = $dbh->prepare("insert ignore into scustomers (switch, router, interface) values (?, ?, ?)");
my $insertRouter = $dbh->prepare("insert ignore into fcustomers (lid, interface, vlan, router, desc1) values (?, ?, ?, ?, ?)");



qx(wget "http://remie.nms.tele.dk/cgi-bin/rms.cgi" -q -O rms.cgi);

open FILE,"rms.cgi" or die "Cannot read the file rms.cgi: $!\n";
while (my $line = <FILE>){
        while ($line =~ /router=(\w+nq[zue]\d+)/g) {


                qx(wget "http://remie.nms.tele.dk/cgi-bin/router.cgi?router=$1" -q -O dummy);
                my $router=$1;

print "Scanning $router\n";

                open ROUTERFILE,"dummy" or die "Cannot read the file dummy: $!\n";
                while (my $input = <ROUTERFILE>){
                        if ($input =~ m/(\w+nqz\d+).*Ethernet\d+\S\d+<.*router=(\w+xda\d+|\w+xdd\d+)/g) {
                                $dslamNQZ->execute($1, $2);
                        }
                        elsif ($input =~ m/(\w+nqz\d+).*router=(\w+nqu\d+|\w+\w+nqe\d+);ifname=(.e-\d+\S\d+\S\d+);/) {
                                $routerNQZ->execute($1, $2, $3);
                        }
                        elsif ($input =~ m/(\w+nqz\d+).*Ethernet\d+\S\d+<.*(EM\w+|HB\w+|HX\w+|MD\w+|NA\w+).*-\s(.*)<\/td><\/tr>/g) {
                                $customersNQZ->execute($1, $2, $3);
                        }
                        elsif ($input =~ m/(.e-\d+\S\d+\S\d+).*(HB\w+|HX\w+|MD\w+|NA\w+)(.*)/g) {
#                       elsif ($input =~ m/router=(\w+nqe\d+|\w+nqu\d+).*(.e-\d+\S\d+\S\d+).*(HB\w+|HX\w+|MD\w+).*-\s(\w+\s\w+).*VLAN(\d+)/g) {
#                       elsif ($input =~ m/router=(\w+nqe\d+|\w+nqu\d+).*(.e-\d+\S\d+\S\d+)\.(\d+)     .*(HB\w+|HX\w+|MD\w+).*-\s(\w+\s\w+).*VLAN(\d+)/g) {
                                $insertRouter->execute($2, $1, '', $router, $3);
                        }
                }

        }
}


