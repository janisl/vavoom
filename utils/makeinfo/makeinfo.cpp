//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id$
//**
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
//**
//**	This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**	This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************

#include "makeinfo.h"

void ProcessDehackedFiles(int argc, char** argv);
void MarkSpecialWeaponStates(void);

extern char*				sprnames[];
extern char* 				statename[];
extern char*				mt_names[];
extern char*				snd_names[];
extern char* 				flagnames1[32];
extern char* 				flagnames2[32];
extern char*				weapon_names[];
extern char*				ammo_names[];
extern state_action_info_t	StateActionInfo[];
extern int					numstates;
extern int					nummobjtypes;
extern int					numweapons;
extern int					num_sfx;
extern state_t				states[];
extern mobjinfo_t			mobjinfo[];
extern weaponinfo_t			weaponinfo[];
extern sfxinfo_t			sfx[];
extern string_def_t			strings[];
extern string_def_t			txtlumps[];
extern map_info_t			map_info[];
extern int					maxammo[];
extern int					perammo[];
extern int					numammo;
extern int					initial_health;
extern int					initial_ammo;
extern int					bfg_cells;
extern int					soulsphere_max;
extern int					soulsphere_health;
extern int					megasphere_health;
extern int					god_health;
extern int					shadow;
extern int					altshadow;

bool						Hacked;

//==========================================================================
//
//	MarkWeaponState
//
//==========================================================================

void MarkWeaponState(int num)
{
	if (num && !states[num].weapon_state)
	{
		states[num].weapon_state = true;
		MarkWeaponState(states[num].nextstate);
	}
}

//==========================================================================
//
//	MarkWeaponStates
//
//==========================================================================

void MarkWeaponStates(void)
{
	int			i;

	for (i = 0; i < numweapons; i++)
    {
		MarkWeaponState(weaponinfo[i].upstate);
		MarkWeaponState(weaponinfo[i].downstate);
		MarkWeaponState(weaponinfo[i].readystate);
		MarkWeaponState(weaponinfo[i].atkstate);
		MarkWeaponState(weaponinfo[i].holdatkstate);
		MarkWeaponState(weaponinfo[i].flashstate);
	}
	// Mark states set by code
	MarkSpecialWeaponStates();
}

//==========================================================================
//
//	WriteHeader
//
//==========================================================================

void WriteHeader(FILE *f)
{
	fprintf(f, "//**************************************************************************\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**	##   ##    ##    ##   ##   ####     ####   ###     ###\n");
	fprintf(f, "//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####\n");
	fprintf(f, "//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##\n");
	fprintf(f, "//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##\n");
	fprintf(f, "//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##\n");
	fprintf(f, "//**	   #    ##    ##    #      ####     ####   ##       ##\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**\t$""Id:$\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**	This program is free software; you can redistribute it and/or\n");
	fprintf(f, "//**  modify it under the terms of the GNU General Public License\n");
	fprintf(f, "//**  as published by the Free Software Foundation; either version 2\n");
	fprintf(f, "//**  of the License, or (at your option) any later version.\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**	This program is distributed in the hope that it will be useful,\n");
	fprintf(f, "//**  but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	fprintf(f, "//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	fprintf(f, "//**  GNU General Public License for more details.\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**************************************************************************\n");
}

//==========================================================================
//
//	WriteFooter
//
//==========================================================================

void WriteFooter(FILE *f)
{
	fprintf(f, "//**************************************************************************\n");
	fprintf(f, "//\n");
	fprintf(f, "//\t$""Log:$\n");
	fprintf(f, "//**************************************************************************\n");
}

//==========================================================================
//
//	WriteStates
//
//==========================================================================

void WriteStates(void)
{
	FILE*		f;
	int			i;

	f = fopen("states.vc", "w");
    if (!f)
    {
    	printf("Cannot open states.vc\n");
        return;
	}

	WriteHeader(f);
	fprintf(f, "//**\n");
	fprintf(f, "//**\tStates table\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**\tGenerated by makeinfo.\n");
	fprintf(f, "//**\n");
    fprintf(f, "//**************************************************************************\n");
	fprintf(f, "\n");

/*	fprintf(f, "class Actor:Entity\n{\n");
    for (i=1; StateActionInfo[i].fname; i++)
    {
    	if (!StateActionInfo[i].weapon_action)
        {
			fprintf(f, "\tvoid %s(void);\n", StateActionInfo[i].fname);
        }
    }
	fprintf(f, "}\n\n");

	fprintf(f, "class Weapon:ViewEntity\n{\n");
    for (i=1; StateActionInfo[i].fname; i++)
    {
    	if (StateActionInfo[i].weapon_action)
        {
			fprintf(f, "\tvoid %s(void);\n", StateActionInfo[i].fname);
        }
    }
	fprintf(f, "}\n\n");*/

	fprintf(f, "__states__(Actor)\n{\n");
	bool in_weapon = false;
//	fprintf(f, "\tS_NULL {\"\", 0, -1, NULL, S_NULL}\n");//- Dehacked fails
	for (i=0; i<numstates; i++)
    {
		if (in_weapon && !states[i].weapon_state)
		{
			fprintf(f, "}\n__states__(Actor)\n{\n");
			in_weapon = false;
		}
		if (!in_weapon && states[i].weapon_state)
		{
			fprintf(f, "}\n__states__(Weapon)\n{\n");
			in_weapon = true;
		}

		fprintf(f, "\t%s(\'%s\', %d",
			statename[i], sprnames[states[i].sprite],
			states[i].frame & 0x7fff);
		if (states[i].frame & 0x8000)
        {
        	fprintf(f, " | FF_FULLBRIGHT");
        }
		if (states[i].model_name)
		{
			fprintf(f, ", \'%s\', %d", states[i].model_name, states[i].model_frame);
		}
		if (states[i].tics == -1)
			fprintf(f, ", -1.0");
		else
			fprintf(f, ", %d.0 / 35.0", states[i].tics);
//		fprintf(f, ", %s", states[i].action_num ?
//			StateActionInfo[states[i].action_num].fname : "NULL");
		fprintf(f, ", %s", statename[states[i].nextstate]);
		if (states[i].misc1 || states[i].misc2)
        {
			fprintf(f, ", %.1f, %.1f", (float)states[i].misc1, (float)states[i].misc2);
		}
		fprintf(f, ") { ");
		if (states[i].action_num)
			fprintf(f, "%s(); ", StateActionInfo[states[i].action_num].fname);
		fprintf(f, "}\n");
    }
	fprintf(f, "}\n\n");
    WriteFooter(f);
    fclose(f);
}

//==========================================================================
//
//	WriteMobjInfo
//
//==========================================================================

void WriteMobjInfo(void)
{
	FILE*		f;
	int			i;
    int			j;
	int			firstflag;
    int			flags;
	int			flags2;
    const char *parent;

	f = fopen("mobjinfo.vc", "w");
    if (!f)
    {
    	printf("Cannot open mobjinfo.vc\n");
        return;
	}

	WriteHeader(f);
	fprintf(f, "//**\n");
	fprintf(f, "//**\tMobj info table\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**\tGenerated by makeinfo.\n");
	fprintf(f, "//**\n");
    fprintf(f, "//**************************************************************************\n");
	fprintf(f, "\n");

	for (i = 0; i < nummobjtypes; i++)
    {
    	//	A standard comment before function
		fprintf(f, "//==========================================================================\n");
		fprintf(f, "//\n");
		fprintf(f, "//\t%s\n", mt_names[i]);
		fprintf(f, "//\n");
		fprintf(f, "//==========================================================================\n");
		fprintf(f, "\n");

        flags = mobjinfo[i].flags;
        flags2 = mobjinfo[i].flags2;

		//  ------------- Class declaration ------------
#if 0
		if (flags & MF_MISSILE)
			parent = "Missile";
		else
#endif
			parent = "Actor";
		fprintf(f, "class %s:%s\n", mt_names[i], parent);
		fprintf(f, "{\n");

		//  ------------ DefaultProperties -------------
		fprintf(f, "\tdefaultproperties\n");
		fprintf(f, "\t{\n");

		if (mobjinfo[i].classname)
	    	fprintf(f, "\t\tClassName = \"%s\";\n", mobjinfo[i].classname);

		//	Misc params
		if (mobjinfo[i].spawnhealth)
			fprintf(f, "\t\tHealth = %d;\n", mobjinfo[i].spawnhealth);
		if (mobjinfo[i].spawnhealth && mobjinfo[i].xdeathstate)
#ifdef NODEH
			fprintf(f, "\t\tGibsHealth = -%d;\n", mobjinfo[i].spawnhealth >> 1);
#else
			fprintf(f, "\t\tGibsHealth = -%d;\n", mobjinfo[i].spawnhealth);
#endif
		if (mobjinfo[i].radius)
			fprintf(f, "\t\tRadius = %.1f;\n", (float)mobjinfo[i].radius / (float)FRACUNIT);
		if (mobjinfo[i].height)
			fprintf(f, "\t\tHeight = %.1f;\n", (float)mobjinfo[i].height / (float)FRACUNIT);
		if (mobjinfo[i].mass)
			fprintf(f, "\t\tMass = %.1f;\n", mobjinfo[i].mass == 0x7fffffff ? 99999.0 : (float)mobjinfo[i].mass);
		if (mobjinfo[i].speed)
			fprintf(f, "\t\tSpeed = %.1f;\n", 35.0 * (mobjinfo[i].speed < 100 ? (float)mobjinfo[i].speed : (float)mobjinfo[i].speed / (float)FRACUNIT));
		if (mobjinfo[i].reactiontime)
        {
			fprintf(f, "\t\tReactionCount = %d;\n", mobjinfo[i].reactiontime);
		}
		if (mobjinfo[i].painchance)
			fprintf(f, "\t\tPainChance = %d;\n", mobjinfo[i].painchance);
		if (mobjinfo[i].damage)
			fprintf(f, "\t\tMissileDamage = %d;\n", mobjinfo[i].damage);

        //	Flag replacements
		if (flags2 & MF2_DONTDRAW)
			fprintf(f, "\t\tTranslucency = 100;\n");
		else if (flags & MF_SHADOW)
			fprintf(f, "\t\tTranslucency = %d;\n", shadow);
		else if (flags & MF_ALTSHADOW && altshadow)
			fprintf(f, "\t\tTranslucency = %d;\n", altshadow);
#ifndef STRIFE
		else if (flags & MF_TRANSLUCENT)
			fprintf(f, "\t\tTranslucency = 33;\n");
#endif
#ifdef STRIFE
        if (flags & 0x70000000)
			fprintf(f, "\t\tTranslation = %d;\n", (flags >> 28) & 7);
#else
        if (flags & MF_TRANSLATION)
			fprintf(f, "\t\tTranslation = %d;\n", (flags & MF_TRANSLATION) >> MF_TRANSSHIFT);
#endif

		//	Clear replaced flags
        flags &= ~(MF_SHADOW|MF_NOTDMATCH);
		if (altshadow) flags &= ~MF_ALTSHADOW;
#ifdef STRIFE
		flags &= ~(0x70000000);
#else
		flags &= ~(MF_TRANSLUCENT|MF_TRANSLATION);
#endif
        flags2 &= ~(MF2_DONTDRAW);

        //	Flags
		if (flags)
        {
			fprintf(f, "\t\tFlags = ");
			firstflag = 1;
    	    for (j=0; j<32; j++)
        	{
        		if (flags & (1 << j))
                {
                	if (!firstflag)
                    {
                    	fprintf(f, "|");
					}
                    firstflag = 0;
                    fprintf(f, "%s", flagnames1[j]);
                }
			}
	        fprintf(f, ";\n");
		}
		if (flags2)
        {
			fprintf(f, "\t\tFlags2 = ");
			firstflag = 1;
    	    for (j=0; j<32; j++)
        	{
        		if (flags2 & (1 << j))
                {
                	if (!firstflag)
                    {
                    	fprintf(f, "|");
					}
                    firstflag = 0;
                    fprintf(f, "%s", flagnames2[j]);
                }
			}
	        fprintf(f, ";\n");
		}

		//	States
        if (mobjinfo[i].spawnstate)
			fprintf(f, "\t\tSpawnState = %s;\n", statename[mobjinfo[i].spawnstate]);
        if (mobjinfo[i].seestate)
			fprintf(f, "\t\tSeeState = %s;\n", statename[mobjinfo[i].seestate]);
        if (mobjinfo[i].meleestate)
			fprintf(f, "\t\tMeleeState = %s;\n", statename[mobjinfo[i].meleestate]);
        if (mobjinfo[i].missilestate)
			fprintf(f, "\t\tMissileState = %s;\n", statename[mobjinfo[i].missilestate]);
        if (mobjinfo[i].painstate)
			fprintf(f, "\t\tPainState = %s;\n", statename[mobjinfo[i].painstate]);
        if (mobjinfo[i].crashstate)
			fprintf(f, "\t\tCrashState = %s;\n", statename[mobjinfo[i].crashstate]);
        if (mobjinfo[i].deathstate)
			fprintf(f, "\t\tDeathState = %s;\n", statename[mobjinfo[i].deathstate]);
        if (mobjinfo[i].xdeathstate)
			fprintf(f, "\t\tGibsDeathState = %s;\n", statename[mobjinfo[i].xdeathstate]);
        if (mobjinfo[i].raisestate)
			fprintf(f, "\t\tRaiseState = %s;\n", statename[mobjinfo[i].raisestate]);

		//	Sounds
        if (mobjinfo[i].seesound)
			fprintf(f, "\t\tSightSound = \'%s\';\n", sfx[mobjinfo[i].seesound].tagName);
		if (mobjinfo[i].activesound)
			fprintf(f, "\t\tActiveSound = \'%s\';\n", sfx[mobjinfo[i].activesound].tagName);
		if (mobjinfo[i].attacksound)
			fprintf(f, "\t\tAttackSound = \'%s\';\n", sfx[mobjinfo[i].attacksound].tagName);
		if (mobjinfo[i].painsound)
			fprintf(f, "\t\tPainSound = \'%s\';\n", sfx[mobjinfo[i].painsound].tagName);
		if (mobjinfo[i].deathsound)
			fprintf(f, "\t\tDeathSound = \'%s\';\n", sfx[mobjinfo[i].deathsound].tagName);

		//  Effects
		if (mobjinfo[i].effects)
			fprintf(f, "\t\tEffects = %s;\n", mobjinfo[i].effects);

		fprintf(f, "\t}\n");

        //  ------------ OnMapSpawn method -----------
		bool no_deathmatch = flags & MF_NOTDMATCH && mobjinfo[i].doomednum != -1;
#ifdef NODEH
		bool no_monsters = flags & MF_COUNTKILL;
#else
		bool no_monsters = flags & MF_COUNTKILL || mobjinfo[i].doomednum == 3006;
#endif
		if (no_deathmatch || no_monsters || mobjinfo[i].extra)
		{
			fprintf(f, "\n");
    	    fprintf(f, "\tvoid OnMapSpawn(mthing_t *mthing)\n");
        	fprintf(f, "\t{\n");

			if (no_deathmatch)
    	    	fprintf(f, "\t\tif (deathmatch)\n\t\t{\n\t\t\tRemoveMobjThinker(this);\n\t\t\treturn;\n\t\t}\n");
			if (no_monsters)
        		fprintf(f, "\t\tif (nomonsters)\n\t\t{\n\t\t\tRemoveMobjThinker(this);\n\t\t\treturn;\n\t\t}\n");

			//	Calling of start function
			fprintf(f, "\t\t::OnMapSpawn(mthing);\n");

			//	Static lights
			if (mobjinfo[i].extra)
				fprintf(f, "\t\t%s\n", mobjinfo[i].extra);

	        //	End of function
    	    fprintf(f, "\t}\n");
		}

		//  End of class
		fprintf(f, "};\n");
		fprintf(f, "\n");
    }

	fprintf(f, "__mobjinfo__\n{\n");
	for (i=0; i<nummobjtypes; i++)
    {
		if (mobjinfo[i].doomednum > 0)
		{
	    	fprintf(f, "\t{ %d, %s }\n", mobjinfo[i].doomednum, mt_names[i]);
		}
    }
    fprintf(f, "}\n\n");
    WriteFooter(f);
	fclose(f);
}

//==========================================================================
//
//	WriteWeaponInfo
//
//==========================================================================

void WriteWeaponInfo(void)
{
	FILE*		f;
	int			i;

	f = fopen("wpninfo.vc", "w");
    if (!f)
    {
    	printf("Cannot open wpninfo.vc\n");
        return;
	}

	WriteHeader(f);
	fprintf(f, "//**\n");
	fprintf(f, "//**\tWeaponn info table\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**\tGenerated by makeinfo.\n");
	fprintf(f, "//**\n");
    fprintf(f, "//**************************************************************************\n");
	fprintf(f, "\n");

	for (i=0; i<(numweapons == 18 ? 9 : numweapons); i++)
    {
        fprintf(f, "void %s(player_t *player);\n", weapon_names[i]);
	}
    fprintf(f, "\n");

	fprintf(f, "weapon_func_t\tweaponinfo[] =\n{\n");
	for (i=0; i<(numweapons == 18 ? 9 : numweapons); i++)
    {
        fprintf(f, "\t%s,\n", weapon_names[i]);
    }
    fprintf(f, "};\n\n");

	if (!altshadow)
    {
        fprintf(f, "int weapon_ammo_type[] =\n{\n");
		for (i=0; i<(numweapons == 18 ? 9 : numweapons); i++)
    	{
        	fprintf(f, "\t%s,\n", ammo_names[weaponinfo[i].ammo]);
	    }
    	fprintf(f, "};\n\n");
	}

	for (i=0; i<(numweapons == 18 ? 9 : numweapons); i++)
    {
    	//	A standard comment before function
		fprintf(f, "//==========================================================================\n");
		fprintf(f, "//\n");
		fprintf(f, "//\t%s\n", weapon_names[i]);
		fprintf(f, "//\n");
		fprintf(f, "//==========================================================================\n");
		fprintf(f, "\n");

        //	Start of function
        fprintf(f, "void %s(player_t *player)\n", weapon_names[i]);
        fprintf(f, "{\n");

        if (altshadow)
	        fprintf(f, "\tplayer->w_mana = %s;\n", ammo_names[weaponinfo[i].ammo]);
		else
	        fprintf(f, "\tplayer->w_ammo = %s;\n", ammo_names[weaponinfo[i].ammo]);
		fprintf(f, "\tplayer->w_upstate = %s;\n", statename[weaponinfo[i].upstate]);
		fprintf(f, "\tplayer->w_downstate = %s;\n", statename[weaponinfo[i].downstate]);
		fprintf(f, "\tplayer->w_readystate = %s;\n", statename[weaponinfo[i].readystate]);
		fprintf(f, "\tplayer->w_atkstate = %s;\n", statename[weaponinfo[i].atkstate]);
		fprintf(f, "\tplayer->w_holdatkstate = %s;\n", statename[weaponinfo[i].holdatkstate]);
		fprintf(f, "\tplayer->w_flashstate = %s;\n", statename[weaponinfo[i].flashstate]);

        if (numweapons == 18)
        {
	        fprintf(f, "\tplayer->w2_ammo = %s;\n", ammo_names[weaponinfo[9+i].ammo]);
			fprintf(f, "\tplayer->w2_upstate = %s;\n", statename[weaponinfo[9+i].upstate]);
			fprintf(f, "\tplayer->w2_downstate = %s;\n", statename[weaponinfo[9+i].downstate]);
			fprintf(f, "\tplayer->w2_readystate = %s;\n", statename[weaponinfo[9+i].readystate]);
			fprintf(f, "\tplayer->w2_atkstate = %s;\n", statename[weaponinfo[9+i].atkstate]);
			fprintf(f, "\tplayer->w2_holdatkstate = %s;\n", statename[weaponinfo[9+i].holdatkstate]);
			fprintf(f, "\tplayer->w2_flashstate = %s;\n", statename[weaponinfo[9+i].flashstate]);
        }

        //	End of function
        fprintf(f, "}\n");
        fprintf(f, "\n");
	}
    WriteFooter(f);
	fclose(f);
}

#ifndef NODEH

//==========================================================================
//
//  WriteStrings
//
//==========================================================================

static void WriteStrings(void)
{
	int			i;
	FILE		*f;

	f = fopen("strings.vc", "w");
	WriteHeader(f);
	fprintf(f, "\n");
    for (i = 0; strings[i].macro; i++)
    {
		fprintf(f, "#define %s\t\t\"%s\"\n", strings[i].macro,
           	strings[i].new_val ? strings[i].new_val : strings[i].def_val);
    }
    fprintf(f, "\n");
    WriteFooter(f);
	fclose(f);
}

//==========================================================================
//
//	WriteMisc
//
//==========================================================================

static void WriteMisc(void)
{
	FILE		*f;
	int			i;

   	f = fopen("gendefs.vc", "w");

	WriteHeader(f);
    fprintf(f, "\n");
	fprintf(f, "int\t\tINITIAL_HEALTH = %d;\n", initial_health);
	fprintf(f, "int\t\tINITIAL_AMMO = %d;\n", initial_ammo);
	fprintf(f, "int\t\tBFGCELLS = %d;\n", bfg_cells);
	fprintf(f, "int\t\tSOULSPHERE_MAX = %d;\n", soulsphere_max);
	fprintf(f, "int\t\tSOULSPHERE_HEALTH = %d;\n", soulsphere_health);
	fprintf(f, "int\t\tMEGASPHERE_HEALTH = %d;\n", megasphere_health);
	fprintf(f, "int\t\tGOD_HEALTH = %d;\n", god_health);

    if (numammo)
    {
    	fprintf(f, "\nint\t\tmaxammo[] = { ");
    	for (i = 0; i < numammo; i++)
        {
			if (i) fprintf(f, ", ");
            fprintf(f, "%d", maxammo[i]);
        }
        fprintf(f, " };\n");

    	fprintf(f, "int\t\tclipammo[] = { ");
    	for (i = 0; i < numammo; i++)
        {
			if (i) fprintf(f, ", ");
            fprintf(f, "%d", perammo[i]);
        }
        fprintf(f, " };\n");
    }

    fprintf(f, "\n");
    WriteFooter(f);
	fclose(f);
}

//==========================================================================
//
//  WriteTxtLumps
//
//==========================================================================

static void WriteTxtLumps(void)
{
	int			i;
	FILE		*f;

    for (i = 0; txtlumps[i].macro; i++)
    {
		f = fopen(txtlumps[i].macro, "w");
		fprintf(f, "%s", txtlumps[i].new_val ? txtlumps[i].new_val : txtlumps[i].def_val);
		fclose(f);
    }

	f = fopen("sfxinfo.txt", "w");
    for (i = 1; i < num_sfx; i++)
    {
    	fprintf(f, "%-32s%-12s%-4d %-4d %d\n", sfx[i].tagName,
    		sfx[i].lumpname[0] ? sfx[i].lumpname : "?",
			sfx[i].priority, sfx[i].numchannels, sfx[i].changePitch);
    }
	fclose(f);

	f = fopen("sndinfo.txt", "w");
    for (i = 1; i < num_sfx; i++)
    {
    	fprintf(f, "%-32s%-12s\n", sfx[i].tagName,
    		sfx[i].lumpname[0] ? sfx[i].lumpname : "?");
    }
	fclose(f);

	if (map_info[0].name[0])
    {
    	f = fopen("mapinfo.txt", "w");
#ifdef DOOM2
    	for (i = 0; i < 32; i++)
        {
        	fprintf(f, "map MAP%02d \"%s\"\nnext MAP%02d\nSKY1 SKY%d 0\nMusic %s\n\n",
            	i + 1, map_info[i].name, (i == 29 || i == 31) ? 1 : i + 2,
            	i < 11 ? 1 : i < 20 ? 2 : 3, map_info[i].song);
        }
#else
    	for (i = 0; i < 36; i++)
        {
        	fprintf(f, "map E%dM%d \"%s\"\nnext E%dM%d\nSKY1 SKY%d 0\nMusic %s\n\n",
            	(i / 9) + 1, (i % 9) + 1, map_info[i].name,
            	(i / 9) + 1, (i % 9) > 6 ? 1 : (i % 9) + 2,
            	i < 11 ? 1 : i < 20 ? 2 : 3, map_info[i].song);
        }
#endif
        fclose(f);
    }
}

#endif

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char** argv)
{
#ifndef NODEH
	ProcessDehackedFiles(argc, argv);
#endif
	MarkWeaponStates();
	WriteStates();
    WriteMobjInfo();
    WriteWeaponInfo();
#ifndef NODEH
	if (Hacked)
	{
	    WriteStrings();
    	WriteMisc();
		WriteTxtLumps();
	}
#endif

	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.15  2002/01/15 18:28:58  dj_jl
//	Some property names with logical words starting with capital letter.
//
//	Revision 1.14  2002/01/12 18:06:34  dj_jl
//	New style of state functions, some other changes
//	
//	Revision 1.13  2002/01/11 18:21:49  dj_jl
//	Started to use names in progs
//	
//	Revision 1.12  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//	
//	Revision 1.11  2001/12/27 17:45:17  dj_jl
//	Removed spawnhealth
//	
//	Revision 1.10  2001/12/12 19:20:30  dj_jl
//	States using methods
//	
//	Revision 1.9  2001/12/03 19:28:41  dj_jl
//	Using defaultproperties, not constructors
//	
//	Revision 1.8  2001/11/09 14:40:32  dj_jl
//	Initialization in constructors
//	
//	Revision 1.7  2001/10/22 17:28:02  dj_jl
//	Removed mobjinfo index constants
//	
//	Revision 1.6  2001/10/18 17:42:19  dj_jl
//	Seperate class for missiles
//	
//	Revision 1.5  2001/10/02 17:40:48  dj_jl
//	Possibility to declare function's code inside class declaration
//	
//	Revision 1.4  2001/09/27 17:04:39  dj_jl
//	Effects and static lights in mobjinfo, mobj classes
//	
//	Revision 1.3  2001/09/20 16:33:14  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
