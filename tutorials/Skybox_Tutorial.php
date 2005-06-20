<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<title>Adding Sprites to States</title>
	<link rel="stylesheet" href="../styles.css" type="text/css" />
</head>
<body>

<h1>SKYBOXES FOR VAVOOM TUTORIAL</h1>
<P>BY FRANCISCO "FIREBRAND" ORTEGA,<br>
Korax' Heritage 2004/03/18
</P>

<pre>
This is a small tutorial describing how to make a skybox for the Vavoom
Engine, first I will try to explain what a skybox is, Skyboxes are used in
most modern games and give a complete wraparound sky and ground texture to
the game environment, old games such as original Doom used a 'drum' texture
that had no top or bottom, which makes the sky look "plain", the skybox
can give any level more realism and may help to give ambient to it.

Skyboxes use TGA image format, you should be aware of this before starting to
make one, because you can't use any other file format. It can have 32-bit
colors (16 million colors), this means that you don't have to use the game
palette you are working on (and makes full compatibility between games to use
them).

If you think a skybox as a cube in which your map is inside, you will
understand that it needs to have 6 different textures:

			- Front Texture
			- Back Texture
			- Left Texture
			- Right Texture 
			- Bottom Texture
			- Upper Texture

The textures need to have an order, imagine the next small square to be the
upper or bottom texture, the order is somewhat like this:

			------------------------
		       |	LEFT		|
		       |B	  TEXTURE      F|
		       |A T		     T R|
		       |C E		     E O|
		       |K X		     X N|
		       |  T		     T T|
		       |  U		     U  |
		       |  R		     R	|
		       |  E	RIGHT	     E	|
		       |	  TEXTURE    	|
			------------------------

Once you have done the textures and ordered them you still need to make a
definition lump for the engine to know where you are storing the images, load
them into the game and display the skybox, this lump is called SKYBOXES, you
can create it with any text editor like the notepad just save it with the
name "SKYBOXES.LMP" so you can insert it easily with Wintex, XWE, or any WAD editor, the
lump has a format like this one:

    rock_dawn <-- This is the name your skybox will have and this is the same
    {		  name you'll have to use to put it into a level.
        {
            // Right
            map textures/skies/rkdawnrt.tga
        }
        {
            // Front
            map textures/skies/rkdawnft.tga
        }
        {
            // Left
            map textures/skies/rkdawnlf.tga
        }
        {
            // Back
            map textures/skies/rkdawnbk.tga
        }
        {
            // Up
            map textures/skies/rkdawnup.tga
        }
        {
            // Down
            map textures/skies/rkdawndn.tga
        }
    } 


NOTE:   You'll have to use the same order shown here for EVERY skybox you
        want to do, the names can't be repeated, and you should put EVERY
        texture into the same directory you write in the lump, in this
        case the "textures" directory was created inside the "basev/game"
        directory, where "game" can be doom1, doom2, heretic, hexen, strife,
        etc. if you don't create it like this you'll get an error when
        loading the files.

Finally, you can make the game to know in which level to put the skyboxes by
modifying the MAPINFO lump into your WAD file, again, you can use any text
editor like the notepad, you'll have to make something like this:

map 1 "Winnowing Hall"
warptrans 1
next 2
cluster 1
skybox day_sky1 <-- This line is the one that says the name of the skybox to
                    use for the map
cdtrack 13

You should take care when drawing the skybox, as it needs to be continuous,
to add even more realism and to make it look like a real landscape, it's the
most difficult part (you can be sure I got a lot of problems with it when
doing the skyboxes myself), anyway, once you get enough practice with it you
can do some amazing looking skyboxes ;).

If you have any question regarding the skyboxes you can post it in the Korax'
Heritage forums or in the Vavoom Engine forums.

Happy Editing!!</pre>

</body>
</html>
