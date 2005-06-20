<html>
<head>
	<title>Bookings</title>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
</head>
<body>
<div align="center">
<h1 align="center">Bookings</h1>

<form action="tst.php" method="post" enctype="multipart/form-data">
	<p>From: <input type="text" name="FromDate" />
	To: <input type="text" name="ToDate" />
	<input type="submit" value="Go!">
</form>

<?php

function PrintBookingsTable($dFrom, $dTo)
{
	$DaySecs = 60 * 60 * 24;

	echo("Showwing bookings from " . date("Y-m-d", $dFrom) . " to " .
		date("Y-m-d", $dTo) . "<br>");

	mysql_connect('localhost', 'vavoom_tycoons', 'secretes');
	mysql_select_db('vavoom_tycoonshotel');

	//	Get rooms list.
	$result = mysql_query("SELECT * FROM `rooms`");
	while ($row = mysql_fetch_array($result))
	{
		$Rooms[$row["Number"]]["Number"] = $row["Number"];
		$Rooms[$row["Number"]]["Type"] = $row["Type"];
		$Rooms[$row["Number"]]["Persons"] = $row["Persons"];
		$Rooms[$row["Number"]]["Beds"] = $row["Beds"];
	}

	$result = mysql_query("SELECT `CheckIn`, `CheckOut`, `Room`, `Source` FROM `bookings`;");
	while ($row = mysql_fetch_array($result))
	{
		$BkFrom = strtotime($row["CheckIn"]);
		$BkTo = strtotime($row["CheckOut"]);
		if ($BkFrom >= $dTo || $BkTo <=$dFrom)
			continue;
		if ($BkFrom < $dFrom)
			$BkFrom = $dFrom;
		if ($BkTo > $dTo)
			$BkTo = $dTo;
		for ($Day = $BkFrom; $Day < $BkTo; $Day += $DaySecs)
		{
			$Booked[$row["Room"]][$Day] = $row["Source"];
		}
	}

	//	Print rooms table header.
	echo("<table border=1 cellspacing=0 cellpading=0 style='border-collapse:collapse;" .
		"border:none;mso-border-alt:solid windowtext .5pt;mso-padding-alt:0cm 5.4pt 0cm 5.4pt'>" .
		"<tr><td width=300 style='border:solid windowtext .5pt'></td>");
	foreach ($Rooms as $R)
	{
		echo("<td width=120 align=center style='border:solid windowtext .5pt'><b>" .
			$R["Number"] . "<br>" .
			($R["Type"] == 0 ? "Studio" : ($R["Type"] . " bedr.")) . "<br>" .
			$R["Persons"] . "/" . $R["Beds"] . "</b></td>");
	}
	for ($Day = $dFrom; $Day < $dTo; $Day += $DaySecs)
	{
		echo("</tr><tr>");
		echo("<td width=300 style='border:solid windowtext .5pt'>" . date("Y-m-d", $Day) . "</td>");
		foreach ($Rooms as $R)
		{
			echo("<td style='border:solid windowtext .5pt;background:#" .
				($Booked[$R["Number"]][$Day] == "Angie" ? "FF0000" :
				$Booked[$R["Number"]][$Day] != "" ? "FF7F7F" : "FFFFFF") .
				"'>" . $Booked[$R["Number"]][$Day] ."</td>");
		}
	}
	echo("</tr></table>");
}

if ($_REQUEST['FromDate'] != "" && $_REQUEST['ToDate'] != "")
{
	//	Dates passed in URL.
	PrintBookingsTable(strtotime($_REQUEST['FromDate']), strtotime($_REQUEST['ToDate']));
}
	
?>
</body>
</html>
