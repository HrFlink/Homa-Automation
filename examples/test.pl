#!/usr/local/bin/perl -w

# $Id: $
# Find and manage links with too low or too high traffic
# Henrik Vestergaard TDC 2014

use strict;
use Getopt::Std;
use Data::Dumper;
use DBI;
use cfg;
use NTI::COMMON;
use NTI::RMS;
use Sys::Syslog qw(:DEFAULT setlogsock);
use SNMP;
use Error qw( :try );

my $rms = new NTI::RMS;
setlogsock( $cfg::logsock ) if($cfg::logsock);
openlog("checkTrafficAlarm","pid","user");

my $params = join " ",@ARGV;
my $debug = 1;
if ( defined($ENV{'DEBUG'} ) ) {
        $debug = $ENV{'DEBUG'};
        $params .= " ENV(DEBUG)=$ENV{'DEBUG'}";
}
my $doDB = 0;
my $pr1 = 0;
my $doUpdate = 0;

my $calibrate_days = 8;
my $alarmlevel_default = 50;
my $tooLowSumTraffic = 500000000;
my $tooLowTraffic = 150000000;

my $mailTo = 'heves@tdc.dk';

my $mailHead = '';
my $mailTail = '';
$mailTail .= "";
$mailTail .= "";

my $mailBody = '';

sub resolveName{
	my $router=shift;
	my $ip =  `host $router`;

	if ($ip =~ /(\d+\.\d+\.\d+\.\d+)/ ) {
		$ip = $1;
	}

	my $newname =  `host $ip`;

	if ($newname =~ /pointer (\w+\.\w+\.\w+\.\w+\.\w+)\./ ) {
		$newname = $1;
	}

	if ($newname) { return $newname } else { return $router }

}

sub raiseAlarm{
  my $router=resolveName(shift);
  my $interface_direction=shift;
  my $message=shift;
  my $snmpsession = new SNMP::Session(RemotePort => 162, Version => '2c' , DestHost => 'nnmi1.nms.tele.dk', Community => 'public');
  $snmpsession->trap(oid => '.1.3.6.1.4.1.11.2.17.1.0.20001009'
          ,uptime => 1234
          ,[['.1.3.6.1.4.1.11.2.17.2.2.0', '',$router,'OCTETSTR']
          ,['.1.3.6.1.4.1.11.2.17.2.3.0', '',$interface_direction,'OCTETSTR']
          ,['.1.3.6.1.4.1.11.2.17.2.4.0', '',$message." ".$router, 'OCTETSTR']]);
}

sub lowerAlarm{
  my $router=resolveName(shift);
  my $interface_direction=shift;
  my $message=shift;
  my $snmpsession = new SNMP::Session(RemotePort => 162, Version => '2c' , DestHost => 'nnmi1.nms.tele.dk', Community => 'public');
  $snmpsession->trap(oid => '.1.3.6.1.4.1.11.2.17.1.0.20001008'
          ,uptime => 1234
          ,[['.1.3.6.1.4.1.11.2.17.2.2.0', '',$router,'OCTETSTR']
          ,['.1.3.6.1.4.1.11.2.17.2.3.0', '',$interface_direction,'OCTETSTR']
          ,['.1.3.6.1.4.1.11.2.17.2.4.0', '',$message." ".$router,'OCTETSTR']] );
}

sub finishMail{

        my $text = shift;
        if(!$mailBody){
                return;
        }
        open (MAIL,"| /usr/sbin/sendmail -t") || die ("Unable to open sendmail: ".$!."\n");
        print MAIL "From: RMS <mie\@appl03.remie.nms.tele.dk>\n";
        print MAIL "To: $mailTo\n";
        print MAIL "Subject: Low traffic detected $text\n\n";
        print MAIL $mailHead;
        print MAIL $mailBody;
        print MAIL $mailTail;
        close(MAIL);

}

sub addMail{
        my $router = shift;
        my $interface = shift;
        my $dir = shift;
				my $text = shift;

        $mailBody = "Low $dir traffic detected on $router $interface\n";
        $mailBody .= "http://remie.nms.tele.dk/cgi-bin/router.cgi?router=$router;ifname=$interface;range=7400\n\n";
				&finishMail($text);
}




#Nothing to configure below
my %opts;
getopts('d:u', \%opts);
$debug = $opts{d} if ( defined($opts{d}) );
$doDB = 1 if ( defined($opts{u}) );
$pr1 = $debug & 0x1;

$|= 1 if($debug);

sub help{
        print <<XX;

        -d nn
                Debug (sets flush)
                        0x1     Print a line for each if

        -u
                Update database



XX
}

my $dbh;
my ($sth_selNew, $sth_upd);
sub initDB{ $dbh = DBI->connect("DBI:mysql:rms;host=$cfg::db_host", $cfg::db_user, $cfg::db_pass, { RaiseError => 1, AutoCommit => 1 } );

				$sth_selNew = $dbh->prepare('SELECT trafficalarm.id,trafficalarm.inserted, trafficalarm.router,trafficalarm.ifname, trafficalarm.state,trafficalarm.statetime,alarmlevel, maxbits, maxtime, minbits, mintime, inmaxbits, inmaxtime, inminbits, inmintime, inbits, outbits, trafficalarm.curinbits, trafficalarm.curoutbits, trafficalarm.ignored, severity, grouptext FROM trafficalarm LEFT JOIN interface USING(router,ifname);');
				$sth_upd = $dbh->prepare('UPDATE trafficalarm SET inserted=?, state=?, alarmlevel=?, statetime=?, maxbits=?, maxtime=?, minbits=?, mintime=?, inmaxbits=?, inmaxtime=?, inminbits=?, inmintime=?, curinbits=?, curoutbits=?, severity=?, ignored=?, grouptext=? WHERE id=?')if($doDB);
}

sub finDB{

        return if(!$doDB);
}

sub alarmHandle {
        my $router = resolveName(shift);
        my $interface = shift;
        my $dir = shift;

				&addMail($router, $interface, $dir, 'ALARM');
}

sub alarmClear  {
        my $router = resolveName(shift);
        my $interface = shift;

        &addMail($router, $interface, '', 'CLEAR');

}

# ****** new interfaces (manual) ******
# mysql rms
# insert into trafficalarm (router, ifname) values ('router', 'interface');

# ****** state values ******
# 0 or null new interface
# 1 calibrating
# 2 normal monitoring state
# 3 alarm raising
# 4 alarm rasied


my $stillOk = 1;
try{
        $rms->log("info","Start $params");
        initDB();
        my $t = time();
				my $order;
				my $dStr = localtime($t);

     		$sth_selNew->execute();
     		while( my($id,$inserted, $router,$ifname,$state,$statetime, $alarmlevel, $maxbits, $maxtime, $minbits, $mintime,$inmaxbits, $inmaxtime, $inminbits, $inmintime, $inbits, $outbits, $curinbits, $curoutbits, $ignored, $severity, $grouptext)= $sth_selNew->fetchrow_array) {

						if (!$state) { # new interface added - initiate record
								$inserted = $t;
								$state = 1;
								$statetime = $t;
								$maxbits = 0;
								$maxtime = 0;
								$minbits = 999999999999;
								$mintime = 0;
								$inmaxbits = 0;
								$inmaxtime = 0;
								$inminbits = 999999999999;
								$inmintime = 0;
								$curinbits = 0;
								$curoutbits = 0;
								$ignored = 0;          # set to 1, if an interface should not be monitored
								if (!$alarmlevel) { $alarmlevel = $alarmlevel_default };
								if (($router =~ /fws/) or ($router =~ /nqe\d\./) or ($router =~ /nxc/)) { $grouptext = "Teknet" } else { $grouptext = "IP Backbone" }
								$severity = 2;
						}
						if (!$inbits) { $inbits = $curinbits };
						if (!$outbits) { $outbits = $curoutbits };
						my $dStr = localtime($t) . " $id, $router, $ifname state: $state, $statetime out max: $maxbits, $maxtime out min: $minbits, $mintime in max: $inmaxbits, $inmaxtime in min: $inminbits, $inmintime current in: $inbits current out: $outbits";
						if ($inserted == $t) { $dStr .= " record initiated"; }
						if (($curinbits == $inbits) && ($curoutbits == $outbits)) {
								$dStr .= " same traffic - skipping";
         				printf"$dStr\n"if($pr1);
            		$sth_upd->execute($inserted, $state, $alarmlevel, $statetime, $maxbits, $maxtime, $minbits, $mintime, $inmaxbits, $inmaxtime, $inminbits, $inmintime, $inbits, $outbits, $severity, $ignored, $grouptext, $id)if($doDB);
								next;
						}

						if ($t > $inserted + $calibrate_days * 24 * 60 * 60 ) { # monitoring

									if (($maxbits + $inmaxbits < $tooLowSumTraffic) || ($ignored) || ($maxbits < $tooLowTraffic) || ($inmaxbits < $tooLowTraffic))  {
                          $dStr .= " too low traffic or ignored bit set - ignoring";
                          printf"$dStr\n"if($pr1);
                          next;
                  }
									if ($severity == 1) { $order = "Tilkald: " } else { $order = "Send FAS til: " }

                  $dStr .= " state: monitoring";
                  if ($state == 1) {
											$state++;
											$statetime = $t;
									}

									if ($inbits < $inminbits * (1 - $alarmlevel/100)) {
                      if ($state < 4) {
													$state++;
													$statetime = $t;
                      		if ($state == 3) { $dStr .= " low traffic in ALARM raising"; }
                      		if (($state == 4) && ($t - $statetime > 900)){
															$dStr .= " low traffic in ALARM raised";
															raiseAlarm("$router", "$ifname", "LOW traffic on $ifname inbound. $order $grouptext");
															#raiseAlarm("$router", "$ifname inbound", "LOW traffic. $order Henrik tester");
															alarmHandle($router, $ifname, 'inbound');
													}
											}
									} else {

		                  if ($outbits < $minbits * (1 - $alarmlevel/100)) {
    		                  if ($state < 4) {
															$state++;
															$statetime = $t;
        		              		if ($state == 3) { $dStr .= " low traffic out ALARM raising"; }
                      				if (($state == 4) && ($t - $statetime > 900)){
																	$dStr .= " low traffic out ALARM raised";
																	raiseAlarm("$router", "$ifname", "LOW traffic on $ifname outbound. $order $grouptext");
																	#raiseAlarm("$router", "$ifname outbound", "LOW traffic. $order Henrik tester");
																	alarmHandle($router, $ifname, 'outbound');
															}
													}
                		  } else {

		                      if ($inbits > $inmaxbits * (1 + ($alarmlevel)/100)) {
 		                      if ($state < 3) {
																$state++;
																$statetime = $t;
																if ($state == 3) { $dStr .= " high traffic in WARNING"; }
																}
		                      } else {

															if ($outbits > $maxbits * (1 + $alarmlevel/100)) {
																	if ($state < 3) {
																			$state++;
																			$statetime = $t;
        		                     			if ($state == 3) { $dStr .= " high traffic out WARNING"; }
																	}
                		      		} else {

																	if ($state > 2) {
																			if ($state == 4) {
																				&alarmClear($router, $ifname);
																				lowerAlarm("$router", "$ifname", "LOW traffic on $ifname. $order $grouptext");
																				#lowerAlarm("$router", "$ifname", "LOW traffic. $order Henrik tester");

																			}
																			$state = 2;   # alarm cleared;
																			$statetime = $t;
        		                     			$dStr .= " ALARM/WARNING cleared";
																	}
															}
												}
										}
								}
						}

						if ($state < 3) { #calibrating

								if ($state == 1) { $dStr .= " state: initial calibrating"; };
								if ( $inbits < $inminbits ) { #lower in traffic
										$inminbits = $inbits; $inmintime = $t;
										$dStr .= ", low in traffic adjusted";
								}
								if ( $outbits < $minbits ) { #lower out traffic
										$minbits = $outbits; $mintime = $t;
										$dStr .= ", low out traffic adjusted";
								}
								if ( $inbits > $inmaxbits ) { #higher in traffic
										$inmaxbits = $inbits; $inmaxtime = $t;
										$dStr .= ", high in traffic adjusted";
								}
								if ( $outbits > $maxbits ) { #higher out traffic
										$maxbits = $outbits; $maxtime = $t;
										$dStr .= ", high out traffic adjusted";
								}
						}

            $sth_upd->execute($inserted, $state, $alarmlevel, $statetime, $maxbits, $maxtime, $minbits, $mintime, $inmaxbits, $inmaxtime, $inminbits, $inmintime, $inbits, $outbits, $severity, $ignored, $grouptext, $id)if($doDB);
            printf"$dStr\n"if($pr1);
		}
}

otherwise{
        my $e = shift;
        print "Otherwise $e\n";
        $rms->log("err","Otherwise $e");
        $stillOk = 0;
}

finally{
        $rms->log("info","Finished");
        finDB() if($stillOk);
        $dbh->disconnect();
};
