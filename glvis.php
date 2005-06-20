<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<link rel="stylesheet" href="styles.css" type="text/css" />
</head>
<body>

<h1>glVIS</h1>

<h2>Introduction</h2><div>
glVIS is a PVS (Potentially Visible Set) builder specially designed to
be used with OpenGL ports of the DOOM game engine. It extends the "GL-Friendly
Nodes" with a new lump named "GL_PVS". Basicly it's a subsector accept table
designed for rendering engine to skip the subsectors that can't be visible
from viewer's subsector. PVS is added to both versions of "GL Nodes".<br />
Currently only Vavoom and ZDoomGL uses PVS. Other source ports will ignore it.</div><br />

<h2>Downloads</h2><div>
<a href="http://www.downloads.orcishweb.com/vavoom/glvis/glvis15dos.zip">DOS binaries</a><br />
<a href="http://www.downloads.orcishweb.com/vavoom/glvis/glvis15win.zip">Windows binaries</a><br />
<a href="http://www.downloads.orcishweb.com/vavoom/glvis/glvis15src.zip">Source code</a></div><br />

<h2>Using glVIS</h2><div>
To create PVS for a wad file (e.g. FOO.WAD) simply type:
</div><br /><pre>
    glvis foo.wad
</pre><div>
glVIS first searches for a GWA file. If it finds it, it will work with
it. Otherwise it assumes that wad file contains "GL Nodes" and will work
with wad file. glVIS will save old file with extension .~gw for .gwa file
or .~wa for a .wad file.</div><br />

<h2>PVS specification</h2><div>
Loading PVS:
<ul>
<li>
	Check lump name, if it's not "GL_PVS", the data is missing. If you want,
	you can treat this case as empty lump, but you can also abort with error
	(like Vavoom does).
</li><li>
	Check lump size, if it's 0, it's empty lump. In this case you allocate
	<tt>((numsubsectors + 7) / 8) * numsubsectors</tt> bytes and fill them
	with 0xff.
</li><li>
	Otherwise PVS data is present, simply load it.
</li></ul>
Unlike REJECT, vis data for a subsector is aligned to byte. Offset in data
for subsector <tt>i</tt> is <tt>((numsubsectors + 7) / 8) * i</tt>. Usage
is similar to that of REJECT. Example of checking if subsector <tt>i</tt>
is visible:
</div><br /><pre>    byte *vis_data;&nbsp;&nbsp;&nbsp;	// PVS lump
    int view_sub;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;	// Num of subsector, where the player is
    byte *vis;
    vis = vis_data + (((numsubsectors + 7) / 8) * view_sub);
    ...
    if (vis[i >> 3] &amp; (1 &lt;&lt; (i &amp; 7)))
    {
        // Subsector is visible
        ...
    }
    else
    {
        // Subsector is not visible
        ...
    }
</pre>

<h2>Status</h2><div>
The current version of glVIS is 1.5. It has been tested and known to work with
numerous large wads, including Ultimate DOOM, DOOM II, Heretic and Hexen.<br />
<b>NOTE:</b> glVIS is very expensive, complete vis data for a large wad can
take several hours.</div><br />

<h2>Contact</h2><div>
Questions, bug reports, suggestsions, etc... about glVIS should be sent
to vavoom at vavoom-engine dot com</div><br />

</body>
</html>
