<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<title>Adding Sprites to States</title>
	<link rel="stylesheet" href="../styles.css" type="text/css" />
</head>
<body>

<h1>Adding Sprites to States.vc</h1>
<P>By Michael Israel</P>

<P>In order to add a sprite to states.vc for example:</P>
<P>Shela0<br />
Shelb0<br />
Shelc0</P>

<P>Lampa0</P>

<P>You want the Shel to animation for 5 ticks</P>

<pre>    S_SHEL(‘SHEL’, 0, 5.0 / 35.0, S_SHEL1) { }
    S_SHEL1(‘SHEL’, 1, 5.0 / 35.0, S_SHEL2) { }
    S_SHEL2(‘SHEL’, 2, 5.0 / 35.0, S_SHEL) { }</pre>

<P>To create Lamp as a static image type the following code</P>

<pre>    S_LAMP(‘LAMP’, 0, -1, S_NULL) { }</pre>

<P>The format for a sprite is:</P>

<pre>OBJECT_NAME (ImageName, Offset, Speed, NextFrame) { “maybe a specialized function to be executed for this object.” }</pre>

</body>
</html>
