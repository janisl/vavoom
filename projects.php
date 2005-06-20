<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<link rel="stylesheet" href="styles.css" type="text/css" />
</head>
<body>
<div align="center">
<h1 align="center">Projects using the Vavoom engine</h1>
<p> </p>
<?
	mysql_connect ('localhost', vavoom, AgapoLizzy);
	mysql_select_db (vavoom_projects);
	$result = mysql_query ("SELECT * FROM projects");
	if ($row = mysql_fetch_array($result))
	{
		do
		{
			echo("<h2>" . $row["name"] . "</h2>");
			echo("<div>by " . $row["author"] . "</div>");
			echo("<div>Type: " . $row["type"] . ", Game: " . $row["games"] . ", Status: " . $row["status"]);
			if ($row["url"] != "") 
				echo(", <a href=\"".$row["url"]."\" target=\"_blank\">Website</a>");
			if ($row["download"] != "")
				echo(", <a href=\"".$row["download"]."\" target=\"_blank\">Download</a>");
			echo("</div>");
			echo("<div>" . $row["description"] . "</div>");
		} while($row = mysql_fetch_array($result));
	}
?>
</body>
</html>
