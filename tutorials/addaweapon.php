<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<title>Adding Sprites to States</title>
	<link rel="stylesheet" href="../styles.css" type="text/css" />
</head>
<body>
<h1>Adding a new weapon</h1>
<P>Tutorial by Robvoom - 11/01/2004</P>
<pre>
*** Just to make clear : this is a brief tutorial to point you in the right direction for adding a simple weapon. Simple means no fancy effect like ejecting bullets, reload etc...Also this tut does not cover the vavoom language so you must learn commands by youself (for starting you can learn by copying the source of existing things) ***


1st follow the directives in "vavmref.txt - chapter 6 making a mod"

If all is right you can compile "clprogs.dat" & "svprogs.dat" running the batch file "compile.bat" (Hint: copy in your progs directory the vavoom compiler VCC.EXE)

Now you can edit the files needed for adding a weapon.

We will add a weapon called "m16" that uses bullets and is located after the chaingun.


Locate the files "doomdefs.vc" (in your progs root).
Open the files with notepad (or any other text editor you like) and add this line just after the comments (ie: "//") :

#define m16

Then search for "#ifdef GRENADE_LAUNCHER..." in the "enum" section and add these lines:

#ifdef m16
	wp_m16,
#endif

Save and close the file.

Locate the file "classes.vc" under the directory "progs/game1" and open it.

Search for the string '#include "ItemWeaponChaingun.vc"' and add after it:

#include "ItemWeaponm16.vc"

Search for the string '#include "WeaponChaingun.vc"' and add after it:

#include "Weaponm16.vc"

Save and close the file.

In the "progs/game1" folder copy and paste the file "ItemWeaponChaingun.vc" and rename it "ItemWeaponm16.vc"

In the "progs/game1" folder copy and paste the file "WeaponChaingun.vc" and rename it "Weaponm16.vc"

Open the file "ItemWeaponm16.vc" and edit "__states__" section to meet your sprite/s.

Save and close the file.

Open the file "Weaponm16.vc" and edit "__states__" section to meet your sprite/s. Also under the "void A_FireCGun(void)" section you can edit the sound of the weapon and how the weapon fire.

Name of "States" must be unique
s so modifiy "__states__" section to meet this requirement.

Also you must modify "defaultproperties" section.

Save and close the file.

For having the weapon in all maps you can also add a cheat so you can recall the weapon (on this section i'm not sure how cheats really works but theses mods have worked for me).

Locate the file "cheat.vc" under the directory "progs/server" and open it.

Search for the string "#define GIMME_SUPERSHOTGUN		0x00000040" and add after it:

#define GIMME_m16		0x00000041

Search for the string "i1 |= GIMME_CHAINGUN;" and add after it:

	if (Cmd_CheckParm("m16"))
		i1 |= GIMME_m16;

Search for the string "player.WeaponOwned[wp_chaingun] = true;" and add after it:

	if (i1 & GIMME_m16)
		player.WeaponOwned[wp_m16] = true;

Save and close the file.

Locate the file "wpninfo.vc" under the directory "progs/server" and open it.

Search for theses lines:
	#ifdef GRENADE_LAUNCHER
	class WeaponGrenade;
	#endif

 and add after it:

	#ifdef m16
	class Weaponm16;
	#endif

At the end of the "classid WeaponClasses[] = {" section add :

	#ifdef m16
	 Weaponm16,
	#endif


At the end of the "int weapon_ammo_type[] = {" section add :

	#ifdef m16
         am_clip,
	#endif

Save and close the file.

Locate the file "wpnbase.vc" under the directory "progs/server" and open it.

Search for the string "void ChangeWeapon(Player player, int newweapon)" :

Under this section look for :

	#ifdef GRENADE_LAUNCHER
	 if (newweapon == wp_missile && player.WeaponOwned[wp_grenade]
		&& player.ReadyWeapon != wp_grenade)
	 {
		newweapon = wp_grenade;
	 }
	#endif


And add this :

	#ifdef m16
  	if (newweapon == wp_chaingun && player.WeaponOwned[wp_m16]
		&& player.ReadyWeapon != wp_m16)
 	 {
		newweapon = wp_m16;
 	 }
	#endif	

Now the weapon m16 is after the chaingun.

You can now compile the progs if no error occured during compiling you can open your wad containing the sprites, sounds, etc. and add the two files "clprogs.dat" & "svprogs.dat".
I've used for doing this XVE (you can look for it at Doomworld.com).

After the wad is completed with progs you can run Vavoom with the "-devgame mymoddir" where mymoddir is where the wad is.

When the game run call the console and type "gimme all" or "gimme m16" and after the chaingun there will be your new weapon (well maybe ;D ).

There are numerous lacks on this tut so if you want modify it and repost to the vavoom-engine.com.

Thanks to Janis for Vavoom and for pointing me in the right direction.</pre>

</body>
</html>
