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

#include "info.h"
#include "makeinfo.h"

void ProcessDehackedFiles(int argc, char** argv);

extern char* 				statename[];
extern char*				mt_names[];
extern char* 				flagnames1[32];
extern char* 				flagnames2[32];
extern mobjinfo_t			mobjinfo[];

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
	fprintf(f, "//**    ##   ##    ##    ##   ##   ####     ####   ###     ###\n");
	fprintf(f, "//**    ##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####\n");
	fprintf(f, "//**     ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##\n");
	fprintf(f, "//**     ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##\n");
	fprintf(f, "//**      ###   ##    ##   ###    ##  ##   ##  ##  ##       ##\n");
	fprintf(f, "//**       #    ##    ##    #      ####     ####   ##       ##\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**    $""Id:$\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**    Copyright (C) 1999-2006 Jānis Legzdiņš\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**    This program is free software; you can redistribute it and/or\n");
	fprintf(f, "//**  modify it under the terms of the GNU General Public License\n");
	fprintf(f, "//**  as published by the Free Software Foundation; either version 2\n");
	fprintf(f, "//**  of the License, or (at your option) any later version.\n");
	fprintf(f, "//**\n");
	fprintf(f, "//**    This program is distributed in the hope that it will be useful,\n");
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
//	WriteClasses
//
//==========================================================================

void WriteClasses()
{
	FILE*		f;
	int			i;
	int			j;
	int			flags;
	int			flags2;
	const char *parent;

	FILE* cf = fopen("deh_game/classes.vc", "w");
	if (!cf)
	{
		printf("Cannot open mobjinfo.vc\n");
		return;
	}

	WriteHeader(cf);
	fprintf(cf, "\n");

	for (i = 0; i < NUMMOBJTYPES; i++)
	{
		fprintf(cf, "#include \"%s.vc\"\n", mt_names[i]);

		char fname[256];
		sprintf(fname, "deh_game/%s.vc", mt_names[i]);
		f = fopen(fname, "w");

		WriteHeader(f);
		fprintf(f, "\n");

		flags = mobjinfo[i].flags;
		flags2 = mobjinfo[i].flags2;

		//  ------------- Class declaration ------------
		parent = "Actor";
		//if (i == 0)
		//	parent = "PlayerPawn";
		fprintf(f, "class %s : %s", mt_names[i], parent);
		if (mobjinfo[i].doomednum > 0)
		{
			fprintf(f, "\n\t__mobjinfo__(%d)", mobjinfo[i].doomednum);
		}
		fprintf(f, ";\n\n");

		//  ------------ OnMapSpawn method -----------
		bool no_monsters = flags & MF_COUNTKILL || mobjinfo[i].doomednum == 3006;

		//  ------------ DefaultProperties -------------
		fprintf(f, "defaultproperties\n");
		fprintf(f, "{\n");

		//	Misc params
		if (mobjinfo[i].spawnhealth != 1000)
			fprintf(f, "\tHealth = %d;\n", mobjinfo[i].spawnhealth);
		if (mobjinfo[i].spawnhealth != 1000 && mobjinfo[i].xdeathstate)
			fprintf(f, "\tGibsHealth = -%d;\n", mobjinfo[i].spawnhealth);
		if (mobjinfo[i].radius != 20 * FRACUNIT)
			fprintf(f, "\tRadius = %.1f;\n", (float)mobjinfo[i].radius / (float)FRACUNIT);
		if (mobjinfo[i].height != 16 * FRACUNIT)
			fprintf(f, "\tHeight = %.1f;\n", (float)mobjinfo[i].height / (float)FRACUNIT);
		if (mobjinfo[i].mass != 100)
			fprintf(f, "\tMass = %.1f;\n", mobjinfo[i].mass == 0x7fffffff ? 99999.0 : (float)mobjinfo[i].mass);
		if (mobjinfo[i].speed)
		{
			if (mobjinfo[i].speed < 100)
				fprintf(f, "\tStepSpeed = %.1f;\n", (float)mobjinfo[i].speed);
			else
				fprintf(f, "\tSpeed = %.1f;\n", 35.0 * (float)mobjinfo[i].speed / (float)FRACUNIT);
		}
		if (mobjinfo[i].reactiontime != 8)
		{
			fprintf(f, "\tReactionCount = %d;\n", mobjinfo[i].reactiontime);
		}
		if (mobjinfo[i].painchance)
			fprintf(f, "\tPainChance = %f;\n", (float)mobjinfo[i].painchance / 256.0);
		if (mobjinfo[i].damage)
			fprintf(f, "\tMissileDamage = %d;\n", mobjinfo[i].damage);

		//	Translucency
		if (flags & MF_SHADOW)
			fprintf(f, "\tAlpha = 0.1;\n");
		else if (flags & MF_TRANSLUCENT)
			fprintf(f, "\tAlpha = 0.666;\n");

		//	Translation
		if (flags & MF_TRANSLATION)
			fprintf(f, "\tTranslation = %d;\n", (flags & MF_TRANSLATION) >> MF_TRANSSHIFT);

		//	Clear replaced flags
		flags &= ~(MF_SHADOW);
		flags &= ~(MF_TRANSLUCENT|MF_TRANSLATION);

		//	Flags
		for (j = 0; j < 32; j++)
		{
			if (flags & (1 << j))
			{
				if ((1 << j) == MF_NOCLIP)
				{
					fprintf(f, "\tbColideWithThings = false;\n");
					fprintf(f, "\tbColideWithWorld = false;\n");
				}
				else
					fprintf(f, "\t%s = true;\n", flagnames1[j]);
			}
		}
		for (j = 0; j < 32; j++)
		{
			if (flags2 & (1 << j))
			{
				fprintf(f, "\t%s = true;\n", flagnames2[j]);
			}
		}
		if (no_monsters)
		{
			fprintf(f, "\tbMonster = true;\n");
			fprintf(f, "\tbActivateMCross = true;\n");
			fprintf(f, "\tbActivatePushWall = true;\n");
		}
		if (flags & MF_MISSILE)
		{
			fprintf(f, "\tbActivatePCross = true;\n");
			fprintf(f, "\tbNoTeleport = true;\n");
		}

		//	States
		if (mobjinfo[i].spawnstate)
			fprintf(f, "\tIdleState = %s;\n", statename[mobjinfo[i].spawnstate]);
		if (mobjinfo[i].seestate)
			fprintf(f, "\tSeeState = %s;\n", statename[mobjinfo[i].seestate]);
		if (i == 0)
		{
			if (mobjinfo[i].missilestate)
				fprintf(f, "\tMeleeState = %s;\n", statename[mobjinfo[i].missilestate]);
			fprintf(f, "\tMissileState = S_PLAY_ATK2;\n");
		}
		else
		{
			if (mobjinfo[i].meleestate)
				fprintf(f, "\tMeleeState = %s;\n", statename[mobjinfo[i].meleestate]);
			if (mobjinfo[i].missilestate)
				fprintf(f, "\tMissileState = %s;\n", statename[mobjinfo[i].missilestate]);
		}
		if (mobjinfo[i].painstate)
			fprintf(f, "\tPainState = %s;\n", statename[mobjinfo[i].painstate]);
		if (mobjinfo[i].crashstate)
			fprintf(f, "\tCrashState = %s;\n", statename[mobjinfo[i].crashstate]);
		if (mobjinfo[i].deathstate)
			fprintf(f, "\tDeathState = %s;\n", statename[mobjinfo[i].deathstate]);
		if (mobjinfo[i].xdeathstate)
			fprintf(f, "\tGibsDeathState = %s;\n", statename[mobjinfo[i].xdeathstate]);
		if (mobjinfo[i].raisestate)
			fprintf(f, "\tRaiseState = %s;\n", statename[mobjinfo[i].raisestate]);

		//	Sounds
		if (mobjinfo[i].seesound)
			fprintf(f, "\tSightSound = \'%s\';\n", sfx[mobjinfo[i].seesound]);
		if (mobjinfo[i].activesound)
			fprintf(f, "\tActiveSound = \'%s\';\n", sfx[mobjinfo[i].activesound]);
		if (mobjinfo[i].attacksound)
			fprintf(f, "\tAttackSound = \'%s\';\n", sfx[mobjinfo[i].attacksound]);
		if (mobjinfo[i].painsound)
			fprintf(f, "\tPainSound = \'%s\';\n", sfx[mobjinfo[i].painsound]);
		if (mobjinfo[i].deathsound)
			fprintf(f, "\tDeathSound = \'%s\';\n", sfx[mobjinfo[i].deathsound]);

		fprintf(f, "}\n");
		fclose(f);
	}
	fclose(cf);
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
	WriteClasses();

	return 0;
}
