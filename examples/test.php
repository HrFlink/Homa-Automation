<html>
	<body>
	<style> table, th, td { border-collapse:collapse; border:1px solid black; } th, td { padding:5px; } </style>
</head>


		<?php
			$conn = mysql_connect('obidb2.nms.tele.dk','a63893','loJOdAPe');
			$db_selected = mysql_select_db("obi", $conn);
			$router = $_GET['router'];
			$interface = $_GET['interface'];
			$dslam = $_GET['dslamname'];
			$switch = $_GET['switchname'];

			function query($query) {
                                $raw_results = mysql_query($query) or die(mysql_error());
                                $num_results = mysql_num_rows($raw_results);
                                if (!$num_results){
                                        echo "0";
                                }else{
                                        Echo "<a href=showquery.php?foo=" ,urlencode($query), ">$num_results</a>";
                                }
				return $num_results;

			}

			function analyse_ri($router, $interface, $service) {

				if ($service == 'fiber') {
					$sql = "SELECT distinct lid FROM fcustomers WHERE (lid LIKE 'H%' or lid LIKE 'NA%') AND interface LIKE '$interface%' AND router = '$router' and desc1 not like '%mobil%'";
				} elseif ($service == 'mobile') {
					$sql = "SELECT distinct lid FROM fcustomers WHERE (desc1 LIKE '%mobil%' or desc1 like 'MD%') AND interface LIKE '$interface%' AND router = '$router'";
				} elseif ($service == 'dsl') {
					$sql = "SELECT distinct lid, portid FROM adman WHERE if1 LIKE '$interface%' AND router1 = '$router' AND state = 'active'";
				} else {
					$sql = "SELECT distinct lid, portid FROM adman WHERE if1 LIKE '$interface%' AND router1 = '$router' AND service_type like '%$service%' AND state = 'active'";
				}
				return query("$sql");
			}


			function analyse_dslam($dslam, $service) {
				if ($service == 'dsl') {
					$sql = "SELECT distinct lid FROM adman WHERE portid LIKE '$dslam-%' AND state = 'active'";
				} else {
					$sql = "SELECT distinct lid FROM adman WHERE portid LIKE '$dslam-%' AND service_type like '%$service%' AND state = 'active'";
				}
				return query("$sql");
			}

			function analyse_switch_fiber($switch, $service) {
				if ($service == 'fiber') {
					$sql = "SELECT lid FROM scustomers WHERE lid LIKE 'H%' AND switch = '$switch' and description not like '%mobil%'";
				} else {
					$sql = "SELECT lid FROM scustomers WHERE switch = '$switch' and description like '%mobil%'";
				}
				return query("$sql");
			}

			 function echo_fiber_table_head() {


                                echo   '<TABLE BORDER>
                                                <TR>
							<TD COLSPAN=2>Customers on switch </TD>
						</TR>
						<TR>
							<TD>Fiber (IP acces)</TD>
							<TD>Mobile (estimated)</TD>
						</TR>
						<TR>';

                        }

			function echo_ri_table_head() {


				echo   '<TABLE BORDER>
						<TR>
							<TD COLSPAN=4>Customers on Router/Card/Uplink</TD>
							<TD COLSPAN=7>Services</TD>
						</TR>
						<TR>
							<TD>Router Interface/Card</TD>
							<TD>DSLAM Customers</TD>
							<TD>Fiber Customers</TD>
							<TD>Mobile (estimated) (VRRP)</TD>
							<TD>Voice (VRRP)</TD>
							<TD>TDC TV (VRRP)</TD>
							<TD>Data</TD>
							<TD>Alarm Net</TD>
							<TD>QoS (Data)</TD>
							<TD>BSA</TD>
							<TD>Total affected</TD>
						</TR>
                                	       	<tr>';
			}

                        function echo_dslam_table_head() {

				echo   '<TABLE BORDER>
						<TR>
							<TD COLSPAN=2>Customers on DSLAM</TD>
							<TD COLSPAN=7>Services</TD>
						</TR>
						<TR>
							<TD>Name</TD>
							<TD>Customers</TD>
							<TD>Voice</TD>
							<TD>TV</TD>
							<TD>Data</TD>
							<TD>Alarm Net</TD>
							<TD>QoS (Data)</TD>
							<TD>BSA</TD>
							<TD>Total affected</TD>
						</TR>
						<tr>';

			}

			if (isset($_GET['ri'])){

				echo_ri_table_head();
                                echo   "<td>$router $interface</td>";
				echo     '<td>'; analyse_ri($router, $interface, 'dsl'); echo '</td>';
				echo     '<td>'; analyse_ri($router, $interface, 'fiber'); echo '</td>';
				echo     '<td>'; analyse_ri($router, $interface, 'mobile'); echo '</td>';
				echo     '<td>'; analyse_ri($router, $interface, 'Voice'); echo '</td>';
				echo     '<td>'; analyse_ri($router, $interface, 'TV'); echo '</td>';
				echo     '<td>'; $total = analyse_ri($router, $interface, 'Data'); echo '</td>';
				echo     '<td>'; $total += analyse_ri($router, $interface, 'Alarm Net'); echo '</td>';
				echo     '<td>'; $total += analyse_ri($router, $interface, 'QoS'); echo '</td>';
				echo     '<td>'; $total += analyse_ri($router, $interface, 'BSA'); echo '</td>';
				echo     "<td>$total</td>";
				echo   '</tr>';
				echo   '</table>';
				echo   '<br>VRRP: Servicen er ikke impacted fordi der er redundans. Mobil dog kun Huawei udstyr.';


			} elseif (isset($_GET['dslam'])){

				echo_dslam_table_head();
                                echo   "<td>$dslam</td>";
				echo     '<td>'; analyse_dslam($dslam, 'dsl'); echo '</td>';
				echo     '<td>'; $total = analyse_dslam($dslam, 'Voice'); echo '</td>';
				echo     '<td>'; $total += analyse_dslam($dslam, 'TV'); echo '</td>';
				echo     '<td>'; $total += analyse_dslam($dslam, 'Data'); echo '</td>';
				echo     '<td>'; $total += analyse_dslam($dslam, 'Alarm Net'); echo '</td>';
				echo     '<td>'; $total += analyse_dslam($dslam, 'QoS'); echo '</td>';
				echo     '<td>'; $total += analyse_dslam($dslam, 'BSA'); echo '</td>';
				echo     "<td>$total</td>";
				echo   '</tr>';
				echo   '</table>';

			} elseif (isset($_GET['switch'])){

				echo_fiber_table_head();
				echo '<td>'; analyse_switch_fiber($switch, 'fiber'); echo '</td>';
				echo '<td>'; analyse_switch_fiber($switch, 'mobile'); echo '</td>';
				echo '</td></tr></table><br>';

				$uplink_results = mysql_query("SELECT router, interface FROM scustomers WHERE switch = '$switch' and (router is not null or interface is not null)")
				or die(mysql_error());

				echo_ri_table_head();
				$ri_dsl_total = 0; $ri_fiber_total = 0; $ri_mobile_total = 0; $ri_voice_total = 0; $ri_tv_total = 0; $ri_data_total = 0; $ri_alarm_total = 0; $ri_qos_total = 0; $ri_bsa_total = 0; $ri_total_total = 0;
				while($uplinks = mysql_fetch_array($uplink_results)){
					$ri_dsl = 0; $ri_fiber = 0; $ri_mobile = 0; $ri_voice = 0; $ri_tv = 0; $ri_data = 0; $ri_alarm = 0; $ri_qos = 0; $ri_bsa = 0; $ri_total = 0;
					$router=$uplinks['router']; $interface=$uplinks['interface'];

                                	echo   "<td>$router $interface</td>";
					echo     '<td>'; $ri_dsl += analyse_ri($router, $interface, 'dsl'); $ri_dsl_total += $ri_dsl; echo '</td>';
					echo     '<td>'; $ri_fiber += analyse_ri($router, $interface, 'fiber'); $ri_fiber_total += $ri_fiber; echo '</td>';
					echo     '<td>'; $ri_mobile += analyse_ri($router, $interface, 'mobile'); $ri_mobile_total += $ri_mobile; echo '</td>';
					echo     '<td>'; $ri_voice += analyse_ri($router, $interface, 'Voice'); $ri_voice_total += $ri_voice; echo '</td>';
					echo     '<td>'; $ri_tv += analyse_ri($router, $interface, 'TV'); $ri_tv_total += $ri_tv; echo '</td>';
					echo     '<td>'; $ri_data += analyse_ri($router, $interface, 'Data'); $ri_total += $ri_data; $ri_data_total += $ri_data; echo '</td>';
					echo     '<td>'; $ri_alarm += analyse_ri($router, $interface, 'Alarm Net'); $ri_total += $ri_alarm; $ri_alarm_total += $ri_alarm; echo '</td>';
					echo     '<td>'; $ri_qos += analyse_ri($router, $interface, 'QoS'); $ri_total += $ri_qos; $ri_qos_total += $ri_soq; echo '</td>';
					echo     '<td>'; $ri_bsa += analyse_ri($router, $interface, 'BSA'); $ri_total += $ri_bsa; $ri_bsa_total += $ri_bsa; echo '</td>';
					echo     "<td>$ri_total</td>"; $ri_total_total += $ri_total;
					echo   '</tr>';
				}
				echo     "<td>Total</td>";
				echo     "<td>$ri_dsl_total</td>";
				echo     "<td>$ri_fiber_total</td>";
				echo     "<td>$ri_mobile_total</td>";
				echo     "<td>$ri_voice_total</td>";
				echo     "<td>$ri_tv_total</td>";
				echo     "<td>$ri_data_total</td>";
				echo     "<td>$ri_alarm_total</td>";
				echo     "<td>$ri_qos_total</td>";
				echo     "<td>$ri_bsa_total</td>";
				echo     "<td>$ri_total_total</td>";
				echo   '</table><br>';

				$dslam_results = mysql_query("SELECT dslam FROM scustomers WHERE switch = '$switch' and dslam is not null")
				or die(mysql_error());

				echo_dslam_table_head();
				$dslam_dsl_total = 0; $dslam_voice_total = 0; $dslam_tv_total = 0; $dslam_data_total = 0; $dslam_alarm_total = 0; $dslam_qos_total = 0; $dslam_bsa_total = 0; $dslam_total_total = 0;
				while($dslams = mysql_fetch_array($dslam_results)){
					$dslam_dsl = 0; $dslam_voice = 0; $dslam_tv = 0; $dslam_data = 0; $dslam_alarm = 0; $dslam_qos = 0; $dslam_bsa = 0; $dslam_total = 0;
					$dslam=$dslams[dslam];

					echo   "<td>$dslam</td>";
					echo     '<td>'; $dslam_dsl += analyse_dslam($dslam, 'dsl'); $dslam_dsl_total += $dslam_dsl; echo '</td>';
					echo     '<td>'; $dslam_voice += analyse_dslam($dslam, 'Voice'); $dslam_total += $dslam_voice; $dslam_voice_total += $dslam_voice; echo '</td>';
					echo     '<td>'; $dslam_tv += analyse_dslam($dslam, 'TV'); $dslam_total += $dslam_tv; $dslam_tv_total += $dslam_tv; echo '</td>';
					echo     '<td>'; $dslam_data += analyse_dslam($dslam, 'Data'); $dslam_total += $dslam_data; $dslam_data_total += $dslam_data; echo '</td>';
					echo     '<td>'; $dslam_alarm += analyse_dslam($dslam, 'Alarm Net'); $dslam_total += $dslam_alarm; $dslam_alarm_total += $dslam_alarm; echo '</td>';
					echo     '<td>'; $dslam_qos += analyse_dslam($dslam, 'QoS'); $dslam_total += $dslam_qos; $dslam_qos_total += $dslam_qos; echo '</td>';
					echo     '<td>'; $dslam_bsa += analyse_dslam($dslam, 'BSA'); $dslam_total += $dslam_bsa; $dslam_bsa_total += $dslam_bsa; echo '</td>';
					echo     "<td>$dslam_total</td>"; $dslam_total_total += $dslam_total;
					echo   '</tr>';
				}
				echo     "<td>Total</td>";
				echo     "<td>$dslam_dsl_total</td>";
				echo     "<td>$dslam_voice_total</td>";
				echo     "<td>$dslam_tv_total</td>";
				echo     "<td>$dslam_data_total</td>";
				echo     "<td>$dslam_alarm_total</td>";
				echo     "<td>$dslam_qos_total</td>";
				echo     "<td>$dslam_bsa_total</td>";
				echo     "<td>$dslam_total_total</td>";
				echo   '</table><br>';
			}
		?>
	</body>
</html>
