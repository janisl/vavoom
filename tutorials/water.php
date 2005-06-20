<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<title>Making Water for Vavoom Hexen</title>
	<link rel="stylesheet" href="../styles.css" type="text/css" />
</head>
<body>

<h1>Making Swimable Water for Vavoom Hexen</h1>

<p>I’m going to start from the top and remake the test level I made for
learning how to make water and I’ve written this presuming you know how to
use wad author if you don’t use the wad author tutorial in the help file.</p>
<p>Firstly make a polygon with 4 sides and a radius of ‘800’ then inside that
sector put another 4 sided polygon but this time with a radius of ‘400’. This
new sector will be our pool so we need to make it deeper so change its floor
height to ‘–200’ and while you have our pools properties up change the sector
tag to ‘1’ then click ‘OK’. </p>
<p>Next build a rectangular sector like I have below and bring up the sector
properties and change the floor and ceiling heights to ‘-16’ and there
textures to ‘X_005’ then change the lighting to ‘255’ now close the properties
box.</p>
<p><IMG SRC="../images/tutorials/water.gif"></p>
<p>Now the hardish bit, Bring up the properties of 1 of the sidedefs of our
third sector and select ‘Special&gt;3D floor( tag)’ and change the argument
that is available to ‘1’ then click ‘OK’. Now pick a different sidedef from
our third sector and select ‘Special&gt;Contents( type, trans, flags)’ then
click ‘OK’. Finally put in a player start then save and play your level.</p>
<p>Ok that’s about it you can change the values of the ‘Special&gt;Contents(
type, trans, flags)’ and see if any thing happens if you like or make the
water darker by changing the brightness of the third sector.</p>

</body>
</html>
