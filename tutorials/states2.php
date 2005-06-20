<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<title>Adding Sprites to States</title>
	<link rel="stylesheet" href="../styles.css" type="text/css" />
</head>
<body>

<h1>Adding Sprites to States.vc or [VaVoomC states vs. DDF states]</h1>

<P>By Michael “LHutz” Israel</P>
<P>(For this tutorial VaVoomC is shown next to DDF—to make seeing how to convert easier.)</P>

<P>To add graphics into VaVoom you must add entries into states.vc that tells VaVoom where the lumps are located in the wad and how the lumps relate to each other. (The first graphic, the one that ends in ‘a’, is coded as ‘0’ in states.vc)</P>

<P>For example, UZIGA0 would be written in states.vc as:</P>
<pre>    S_UZIG(‘UZIG’, 0, -1.0, S_NULL) { } | UZIG:A:-1:NORMAL:NOTHING;</pre>
<P>Because there is only one image, –1.0 instructs VaVoom, that it is a still image.</P>
<P>S_NULL instructs VaVoom that it is the only image in the sequence.</P>

<P>If you had three images: firea0, fireb0, and firec0 that showed for three ticks you would write the following code:</P>

<pre>    S_FIRE(‘FIRE’, 0, 3.0 / 35.0, S_FIRE1) { }  |  FIRE:A:3:NORMAL:NOTHING,
    S_FIRE1(‘FIRE’, 1, 3.0 / 35.0, S_FIRE2) { } |  FIRE:B:3:NORMAL:NOTHING,
    S_FIRE2(‘FIRE’, 2, 3.0 / 35.0, S_FIRE) { }  |  FIRE:C:3:NORMAL:NOTHING;</pre>

<P>The second number tells what image to use; think of it as a=0, b=1, and so on.</P>
<P>The 3.0 / 35.0 tells the computer that the image lasts for 3 ticks, and that 35 ticks takes about a second. (The speed in ticks should always be divided by 35.)</P>

<P>VaVoom takes care of rotation angles—you just need to tell it what image to use.</P>

<P>It you include –1.0 and S_NULL at any time it will cause the last image to remain on the screen forever. (Let’s say you want to render BLODA0, BLODB0, BLODC0, and leave BLODC0 on the screen forever type to following.)</P>

<pre>    S_BLOD(‘BLOD’, 0, 3.0 / 35.0, S_BLOD1) { }   | BLOD:A:3:NORMAL:NOTHING
    S_BLOD1(‘BLOD’, 1, 3.0 / 35.0, S_BLOD2) { }  | BLOD:B:3:NORMAL:NOTHING
    S_BLOD2(‘BLOD’, 2, -1.0, S_NULL) { }         | BLOD:C:-1:NORMAL:NOTHING;</pre>

<P>The { } holds special commands that you may want to use to customize an image; such as lighting or weapon effects.</P>

<pre>    S_UZIG(‘UZIG’, 0, 1.0 / 35.0, S_UZIG) { A_WeaponReady(); } | UZIG:A:NORMAL:READY;</pre>
<P>Tells the computer that S_UZIG is a UZI that is a ready weapon.</P>

<pre>    S_LITE(‘LAMP’, 0  |  FF_FULLBRIGHT, 2.0 / 35.0, S_LITE1) { } |  LAMP:A:2:BRIGHT:NOTHING,
    S_LITE1(‘LAMP’, 1 |  FF_FULLBRIGHT, 2.0 / 35.0, S_LITE)  { } |  LAMP:B:2:BRIGHT:NOTHING;</pre>
<P>Tells the computer that the graphic is always bright. [On the left is VaVoomC, and on the right is DDF.]</P>

<P>Maybe later I might write about using models. &lt;&lt;DDF is not explained because this is a VaVoomC tutorial.&gt;&gt;</P>
<P>Any questions e-mail me at <A HREF="mailto:roy5050@hotmail.com">roy5050@hotmail.com</A>  {Any complaints or suggestions should only be addressed to me.}</P>

<P>E.D.G.E. Site: <a href="http://edge.sourceforge.net/">http://edge.sourceforge.net/</a></P>
<P>P.S.: If you found this tutorial useful; help with programming the DDF to VaVoomC converter.</P>
<P>(If any one from the E.D.G.E. team wants to join forces with VaVoom, and help create a killer engine.)</P>

</body>
</html>
