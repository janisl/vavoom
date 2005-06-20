<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<link rel="stylesheet" href="styles.css" type="text/css" />
</head>
<script LANGUAGE="JavaScript">

function shotwindow(name, width, height)
{
	contents =
		'<head><title>Screenshot</title></head>' +
		'<body bgcolor="#000000">' +
		'<a href="javascript:window.close()">' +
		'<img src="images/shots/'+name+'" border = 0></a>';
	param = "width=" + (width + 20) + ",height=" + (height + 30) + ",resize=0";
	shot = window.open("", "", param);
	shot.document.write(contents);
	shot.document.close();
}
</script>
<body>

<center>
<a href="#" onclick="shotwindow('shot00.jpg',800,600)"><img src="images/shots/shot00_t.jpg" height="120" width="160" /></a>
<a href="#" onclick="shotwindow('shot01.jpg',800,600)"><img src="images/shots/shot01_t.jpg" height="120" width="160" /></a>
<a href="#" onclick="shotwindow('shot02.jpg',800,600)"><img src="images/shots/shot02_t.jpg" height="120" width="160" /></a>
<br>
<a href="#" onclick="shotwindow('shot03.jpg',800,600)"><img src="images/shots/shot03_t.jpg" height="120" width="160" /></a>
<a href="#" onclick="shotwindow('shot04.jpg',800,600)"><img src="images/shots/shot04_t.jpg" height="120" width="160" /></a>
<a href="#" onclick="shotwindow('shot05.jpg',800,600)"><img src="images/shots/shot05_t.jpg" height="120" width="160" /></a>
<br>
<a href="#" onclick="shotwindow('shot06.jpg',800,600)"><img src="images/shots/shot06_t.jpg" height="120" width="160" /></a>
<a href="#" onclick="shotwindow('shot07.jpg',800,600)"><img src="images/shots/shot07_t.jpg" height="120" width="160" /></a>
<a href="#" onclick="shotwindow('shot08.jpg',800,600)"><img src="images/shots/shot08_t.jpg" height="120" width="160" /></a>
<br>
</center>

</body>
</html>
