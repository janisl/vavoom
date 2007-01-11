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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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
#include "info.h"

void ProcessDehackedFiles(int argc, char** argv);
void MarkSpecialWeaponStates();

extern char*				sprnames[];
extern char* 				statename[];
extern char*				mt_names[];
extern char*				snd_names[];
extern char* 				flagnames1[32];
extern char* 				flagnames2[32];
extern char*				weapon_names[];
extern char*				ammo_names[];
extern state_action_info_t	StateActionInfo[];
extern int					num_sfx;
extern state_t				states[];
extern mobjinfo_t			mobjinfo[];
extern weaponinfo_t			weaponinfo[];
extern sfxinfo_t			sfx[];
extern string_def_t			Strings[];
extern map_info_t			map_info1[];
extern map_info_t			map_info2[];

int		maxammo[] = {200, 50, 300, 50};
int		perammo[] = {10, 4, 20, 1};
int		numammo = 4;

int		initial_health = 100;
int		initial_ammo = 50;
int		bfg_cells = 40;
int		soulsphere_max = 200;
int		soulsphere_health = 100;
int		megasphere_health = 200;
int		god_health = 100;

bool						Hacked;
bool						Doom2;

//==========================================================================
//
//	FixupHeights
//
//==========================================================================

void FixupHeights()
{
	mobjinfo[MT1_MISC29].height = 72*FRACUNIT;
	mobjinfo[MT1_MISC30].height = 56*FRACUNIT;
	mobjinfo[MT1_MISC31].height = 48*FRACUNIT;
	mobjinfo[MT1_MISC32].height = 52*FRACUNIT;
	mobjinfo[MT1_MISC33].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC34].height = 52*FRACUNIT;
	mobjinfo[MT1_MISC35].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC36].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC37].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC39].height = 48*FRACUNIT;
	mobjinfo[MT1_MISC40].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC41].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC42].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC43].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC44].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC45].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC46].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC47].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC48].height = 120*FRACUNIT;
	mobjinfo[MT1_MISC50].height = 56*FRACUNIT;
	mobjinfo[MT1_MISC70].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC72].height = 56*FRACUNIT;
	mobjinfo[MT1_MISC73].height = 48*FRACUNIT;
	mobjinfo[MT1_MISC74].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC75].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC76].height = 96*FRACUNIT;
	mobjinfo[MT1_MISC77].height = 32*FRACUNIT;
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
	fprintf(f, "//**	Copyright (C) 1999-2006 Jānis Legzdiņš\n");
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

void WriteStates(FILE* f)
{
	fprintf(f, "__states__\n{\n");
	for (int i = 1; i < NUMSTATES; i++)
	{
		fprintf(f, "\t%s(\'%s\', %d",
			statename[i], sprnames[states[i].sprite],
			states[i].frame & 0x7fff);
		if (states[i].frame & 0x8000)
		{
			fprintf(f, " | FF_FULLBRIGHT");
		}
		if (states[i].tics == -1)
			fprintf(f, ", -1.0");
		else
			fprintf(f, ", %d.0 / 35.0", states[i].tics);
		fprintf(f, ", %s", statename[states[i].nextstate]);
		if (states[i].misc1 || states[i].misc2)
		{
			fprintf(f, ", %d, %d", states[i].misc1, states[i].misc2);
		}
		fprintf(f, ") { ");
		if (states[i].action_num)
			fprintf(f, "%s(); ", StateActionInfo[states[i].action_num].fname);
		fprintf(f, "}\n");
	}
	fprintf(f, "}\n\n");
}

//==========================================================================
//
//	WriteStates
//
//==========================================================================

void WriteStates()
{
	FILE* f = fopen("states.vc", "w");
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

	fprintf(f, "class DehActor : HackedActor;\n\n");
	WriteStates(f);
	fprintf(f, "defaultproperties\n{\n");
	fprintf(f, "\tSOULSPHERE_MAX = %d;\n", soulsphere_max);
	fprintf(f, "\tSOULSPHERE_HEALTH = %d;\n", soulsphere_health);
	fprintf(f, "\tMEGASPHERE_HEALTH = %d;\n", megasphere_health);
	fprintf(f, "}\n\n");

	fprintf(f, "class DehWeapon : HackedWeapon;\n\n");
	WriteStates(f);
	fprintf(f, "defaultproperties\n{\n}\n\n");

	WriteFooter(f);
	fclose(f);
}

//==========================================================================
//
//	WriteMobjInfo
//
//==========================================================================

void WriteMobjInfo()
{
	FILE*		f;
	int			i;
	int			j;
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

	for (i = 0; i < NUMMOBJTYPES; i++)
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
		parent = "DehActor";
		//if (i == 0)
		//	parent = "PlayerPawn";
		fprintf(f, "class %s:%s", mt_names[i], parent);
		if (mobjinfo[i].doomednum > 0)
		{
			fprintf(f, "\n\t__mobjinfo__(%d)", mobjinfo[i].doomednum);
		}
		fprintf(f, ";\n\n");

		//  ------------ OnMapSpawn method -----------
		bool no_monsters = flags & MF_COUNTKILL || mobjinfo[i].doomednum == 3006;

		//  ------------ DefaultProperties -------------
		fprintf(f, "\tdefaultproperties\n");
		fprintf(f, "\t{\n");

		if (flags & MF_SPECIAL)
		{
			mobjinfo[i].height  = 8 * FRACUNIT;
		}

		//	Misc params
		if (mobjinfo[i].spawnhealth)
			fprintf(f, "\t\tHealth = %d;\n", mobjinfo[i].spawnhealth);
		if (mobjinfo[i].spawnhealth && mobjinfo[i].xdeathstate)
			fprintf(f, "\t\tGibsHealth = -%d;\n", mobjinfo[i].spawnhealth);
		if (mobjinfo[i].radius)
			fprintf(f, "\t\tRadius = %.1f;\n", (float)mobjinfo[i].radius / (float)FRACUNIT);
		if (mobjinfo[i].height)
			fprintf(f, "\t\tHeight = %.1f;\n", (float)mobjinfo[i].height / (float)FRACUNIT);
		if (mobjinfo[i].mass)
			fprintf(f, "\t\tMass = %.1f;\n", mobjinfo[i].mass == 0x7fffffff ? 99999.0 : (float)mobjinfo[i].mass);
		if (mobjinfo[i].speed)
		{
			if (mobjinfo[i].speed < 100)
				fprintf(f, "\t\tStepSpeed = %.1f;\n", (float)mobjinfo[i].speed);
			else
				fprintf(f, "\t\tSpeed = %.1f;\n", 35.0 * (float)mobjinfo[i].speed / (float)FRACUNIT);
		}
		if (mobjinfo[i].reactiontime)
		{
			fprintf(f, "\t\tReactionCount = %d;\n", mobjinfo[i].reactiontime);
		}
		if (mobjinfo[i].painchance)
			fprintf(f, "\t\tPainChance = %f;\n", (float)mobjinfo[i].painchance / 256.0);
		if (mobjinfo[i].damage)
			fprintf(f, "\t\tMissileDamage = %d;\n", mobjinfo[i].damage);

		//	Translucency
		if (flags2 & MF2_DONTDRAW)
			fprintf(f, "\t\tTranslucency = 100;\n");
		else if (flags & MF_SHADOW)
			fprintf(f, "\t\tTranslucency = 90;\n");
		else if (flags & MF_TRANSLUCENT)
			fprintf(f, "\t\tTranslucency = 33;\n");

		//	Translation
		if (flags & MF_TRANSLATION)
			fprintf(f, "\t\tTranslation = %d;\n", (flags & MF_TRANSLATION) >> MF_TRANSSHIFT);

		//	Clear replaced flags
		flags &= ~(MF_SHADOW);
		flags &= ~(MF_TRANSLUCENT|MF_TRANSLATION);
		flags2 &= ~(MF2_DONTDRAW);

		//	Flags
		for (j = 0; j < 32; j++)
		{
			if (flags & (1 << j))
			{
				if ((1 << j) == MF_NOCLIP)
				{
					fprintf(f, "\t\tbColideWithThings = false;\n");
					fprintf(f, "\t\tbColideWithWorld = false;\n");
				}
				else
					fprintf(f, "\t\t%s = true;\n", flagnames1[j]);
			}
		}
		for (j = 0; j < 32; j++)
		{
			if (flags2 & (1 << j))
			{
				fprintf(f, "\t\t%s = true;\n", flagnames2[j]);
			}
		}
		if (no_monsters)
			fprintf(f, "\t\tbMonster = true;\n");

		//	States
		if (mobjinfo[i].spawnstate)
			fprintf(f, "\t\tIdleState = %s;\n", statename[mobjinfo[i].spawnstate]);
		if (mobjinfo[i].seestate)
			fprintf(f, "\t\tSeeState = %s;\n", statename[mobjinfo[i].seestate]);
		if (i == 0)
		{
			if (mobjinfo[i].missilestate)
				fprintf(f, "\t\tMeleeState = %s;\n", statename[mobjinfo[i].missilestate]);
			fprintf(f, "\t\tMissileState = S_PLAY_ATK2;\n");
		}
		else
		{
			if (mobjinfo[i].meleestate)
				fprintf(f, "\t\tMeleeState = %s;\n", statename[mobjinfo[i].meleestate]);
			if (mobjinfo[i].missilestate)
				fprintf(f, "\t\tMissileState = %s;\n", statename[mobjinfo[i].missilestate]);
		}
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

		fprintf(f, "\t}\n");

		//  End of class
		fprintf(f, "\n");
	}

	WriteFooter(f);
	fclose(f);
}

//==========================================================================
//
//	WriteWeaponInfo
//
//==========================================================================

void WriteWeaponInfo()
{
	FILE*		f;
	int			i;

	f = fopen("weapons.vc", "w");
	if (!f)
	{
		printf("Cannot open weapons.vc\n");
		return;
	}
	WriteHeader(f);
	fprintf(f, "//**\n");
	fprintf(f, "//**\tWeapons\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**\tGenerated by makeinfo.\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**************************************************************************\n");
	fprintf(f, "\n");

	for (i=0; i<NUM_WEAPONS; i++)
	{
		//	A standard comment before function
		fprintf(f, "//==========================================================================\n");
		fprintf(f, "//\n");
		fprintf(f, "//\t%s\n", weapon_names[i]);
		fprintf(f, "//\n");
		fprintf(f, "//==========================================================================\n");
		fprintf(f, "\n");

		//	Start of function
		fprintf(f, "class %s:DehWeapon;\n\n", weapon_names[i]);
		fprintf(f, "defaultproperties\n");
		fprintf(f, "{\n");

		fprintf(f, "\tAmmo = DoomDefs::%s;\n", ammo_names[weaponinfo[i].ammo]);
		fprintf(f, "\tUpState = %s;\n", statename[weaponinfo[i].upstate]);
		fprintf(f, "\tDownState = %s;\n", statename[weaponinfo[i].downstate]);
		fprintf(f, "\tReadyState = %s;\n", statename[weaponinfo[i].readystate]);
		fprintf(f, "\tAttackState = %s;\n", statename[weaponinfo[i].atkstate]);
		fprintf(f, "\tHoldAttackState = %s;\n", statename[weaponinfo[i].holdatkstate]);
		fprintf(f, "\tFlashState = %s;\n", statename[weaponinfo[i].flashstate]);

		//	End of function
		fprintf(f, "}\n");
		fprintf(f, "\n");
	}
	WriteFooter(f);
	fclose(f);
}

//==========================================================================
//
//	WriteMisc
//
//==========================================================================

static void WriteMisc()
{
	FILE		*f;
	int			i;

	f = fopen("gendefs.vc", "w");

	WriteHeader(f);
	fprintf(f, "\n");
	fprintf(f, "class MainGameInfo : MainGameInfoBase;\n");
	fprintf(f, "\n");
	fprintf(f, "defaultproperties\n");
	fprintf(f, "{\n");
	fprintf(f, "\tINITIAL_HEALTH = %d;\n", initial_health);
	fprintf(f, "\tINITIAL_AMMO = %d;\n", initial_ammo);
	fprintf(f, "\tBFGCELLS = %d;\n", bfg_cells);
	fprintf(f, "\tGOD_HEALTH = %d;\n", god_health);

	if (numammo)
	{
		for (i = 0; i < numammo; i++)
		{
			fprintf(f, "\tmaxammo[%d] = %d;\n", i, maxammo[i]);
		}

		for (i = 0; i < numammo; i++)
		{
			fprintf(f, "\tclipammo[%d] = %d;\n", i, perammo[i]);
		}
	}
	fprintf(f, "}\n");

	fprintf(f, "\n");
	WriteFooter(f);
	fclose(f);
}

//==========================================================================
//
//  WriteTxtLumps
//
//==========================================================================

static void WriteTxtLumps()
{
	int			i;
	FILE		*f;

	if (Doom2)
	{
		strcpy(sfx[sfx1_tink].tagName, "misc/chat2");
		strcpy(sfx[sfx1_radio].tagName, "misc/chat");
	}

	f = fopen("sndinfo.txt", "w");
	for (i = 1; i < num_sfx; i++)
	{
		fprintf(f, "%-32s%-12s\n", sfx[i].tagName,
			sfx[i].lumpname[0] ? sfx[i].lumpname : "?");
	}
	fclose(f);

	f = fopen("mapinfo.txt", "w");
	if (Doom2)
	{
		for (i = 0; i < 32; i++)
		{
			fprintf(f, "map map%02d %s\nnext map%02d\nsky1 sky%d 0\nmusic %s\n\n",
				i + 1, map_info2[i].Name, (i == 29 || i == 31) ? 1 : i + 2,
				i < 11 ? 1 : i < 20 ? 2 : 3, map_info2[i].song);
		}
	}
	else
	{
		for (i = 0; i < 36; i++)
		{
			fprintf(f, "map e%dm%d %s\nnext e%dm%d\nsky1 sky%d 0\nmusic %s\n\n",
				(i / 9) + 1, (i % 9) + 1, map_info1[i].Name,
				(i / 9) + 1, (i % 9) > 6 ? 1 : (i % 9) + 2,
				i < 11 ? 1 : i < 20 ? 2 : 3, map_info1[i].song);
		}
	}
	fclose(f);

	f = fopen("language.txt", "w");
	fprintf(f, "[en default]\n");
	for (i = 0; Strings[i].macro; i++)
	{
		fprintf(f, "%s = \"", Strings[i].macro);
		const char* c = Strings[i].new_val ? Strings[i].new_val : Strings[i].def_val;
		while (*c)
		{
			if (*c == '\"')
				fprintf(f, "\\\"");
			else if (*c == '\t')
				fprintf(f, "\\t");
			else if (*c == '\n' && c[1])
				fprintf(f, "\\n\"\n\t\"");
			else if (*c == '\n')
				fprintf(f, "\\n");
			else if (*c == '\r')
				fprintf(f, "\\r");
			else
				fprintf(f, "%c", *c);
			c++;
		}
		fprintf(f, "\";\n");
	}
	fclose(f);
}

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char** argv)
{
	for (int p = 1; p < argc; p++)
	{
		if (!stricmp(argv[p], "-doom2"))
		{
			Doom2 = true;
			break;
		}
	}

	ProcessDehackedFiles(argc, argv);
	WriteStates();
	WriteMobjInfo();
	WriteWeaponInfo();
	if (Hacked)
	{
		WriteMisc();
		WriteTxtLumps();
	}

	return 0;
}
