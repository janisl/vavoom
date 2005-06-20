<html>
<head>
	<title>Vavoom - the most advanced Doom source port</title>
	<link rel="stylesheet" href="styles.css" type="text/css" />
</head>
<?php
$Lang = $_GET['Lang'];
if ($Lang == "Lat")
{
	?>
	</head>
	<frameset cols="150,*" border="0" frameborder="no">
		<frame src="menu_l.php" name="menu" scrolling="no" noresize="noresize" />
		<frame src="news.php?" name="main" noresize="noresize" />
	</frameset>
	<noframes>
	This site requires browser that supports frames; unfortunately,
	you don't have one. I highly recommend you to download the
	latest version of
	<a href="http://www.microsoft.com/ie">Microsoft Internet Explorer</a> or
	<a href="http://www.netscape.com">Netscape Navigator</a>.
	</noframes>
	<?php
}
else if ($Lang == "Eng")
{
	?>
	</head>
	<frameset cols="150,*" border="0" frameborder="no">
		<frame src="menu.php" name="menu" scrolling="no" noresize="noresize" />
		<frame src="news.php?" name="main" noresize="noresize" />
	</frameset>
	<noframes>
	This site requires browser that supports frames; unfortunately,
	you don't have one. I highly recommend you to download the
	latest version of
	<a href="http://www.microsoft.com/ie">Microsoft Internet Explorer</a> or
	<a href="http://www.netscape.com">Netscape Navigator</a>.
	</noframes>
	<?php
}
else
{
	?>
	<body marginheight="0" marginwidth="0">

	<table WIDTH="100%" HEIGHT="100%"><tr ALIGN=CENTER VALIGN=CENTER><td>
		<table>
		<tr ALIGN=CENTER VALIGN=CENTER>
			<td COLSPAN="2"><img SRC="images/logo/vavoom.jpg" BORDER=0 height=97 width=403></td>
		</tr>
		<tr height="64">
			<td>
			<center><a href="index.php?Lang=Lat" target="_top"><img SRC="images/language/latvian.jpg" BORDER=0 height=34 width=50></a></center>
			</td>
			<td>
			<center><a href="index.php?Lang=Eng" target="_top"><img SRC="images/language/english.jpg" BORDER=0 height=34 width=50></a></center>
			</td>
		</tr>
		</table>
	</td></tr></table>

	</body>
	<?php
}
?>
</html>
