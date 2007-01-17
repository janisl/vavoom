//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id:$
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
//**
//**	Dehacked patch parsing
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

int					initial_health;
int					initial_ammo;
int					bfg_cells;
int					soulsphere_max;
int					soulsphere_health;
int					megasphere_health;
int					god_health;

bool					Hacked;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char*					Patch;
static char*					PatchPtr;
static char*					String;
static int						value;

static TArray<VState*>			States;
static TArray<VState*>			CodePtrStates;
static TArray<VMethod*>			StateActions;

static VClass*					GameInfoClass;

static TArray<FReplacedString>	SfxNames;
static TArray<FReplacedString>	MusicNames;
static TArray<FReplacedString>	SpriteNames;
static VLanguage*				EngStrings;

static const char* OrigSpriteNames[] = {
	"TROO","SHTG","PUNG","PISG","PISF","SHTF","SHT2","CHGG","CHGF","MISG",
	"MISF","SAWG","PLSG","PLSF","BFGG","BFGF","BLUD","PUFF","BAL1","BAL2",
	"PLSS","PLSE","MISL","BFS1","BFE1","BFE2","TFOG","IFOG","PLAY","POSS",
	"SPOS","VILE","FIRE","FATB","FBXP","SKEL","MANF","FATT","CPOS","SARG",
	"HEAD","BAL7","BOSS","BOS2","SKUL","SPID","BSPI","APLS","APBX","CYBR",
	"PAIN","SSWV","KEEN","BBRN","BOSF","ARM1","ARM2","BAR1","BEXP","FCAN",
	"BON1","BON2","BKEY","RKEY","YKEY","BSKU","RSKU","YSKU","STIM","MEDI",
	"SOUL","PINV","PSTR","PINS","MEGA","SUIT","PMAP","PVIS","CLIP","AMMO",
	"ROCK","BROK","CELL","CELP","SHEL","SBOX","BPAK","BFUG","MGUN","CSAW",
	"LAUN","PLAS","SHOT","SGN2","COLU","SMT2","GOR1","POL2","POL5","POL4",
	"POL3","POL1","POL6","GOR2","GOR3","GOR4","GOR5","SMIT","COL1","COL2",
	"COL3","COL4","CAND","CBRA","COL6","TRE1","TRE2","ELEC","CEYE","FSKU",
	"COL5","TBLU","TGRN","TRED","SMBT","SMGT","SMRT","HDB1","HDB2","HDB3",
	"HDB4","HDB5","HDB6","POB1","POB2","BRS1","TLMP","TLP2", 0
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	GetLine
//
//==========================================================================

static bool GetLine()
{
	guard(GetLine);
	do
	{
		if (!*PatchPtr)
		{
			return false;
		}

		String = PatchPtr;

		while (*PatchPtr && *PatchPtr != '\n')
		{
			PatchPtr++;
		}
		if (*PatchPtr == '\n')
		{
			*PatchPtr = 0;
			PatchPtr++;
		}

		if (*String == '#')
		{
			*String = 0;
			continue;
		}

		while (*String && *String <= ' ')
		{
			String++;
		}
	} while (!*String);

	return true;
	unguard;
}

//==========================================================================
//
//	ParseParam
//
//==========================================================================

static bool ParseParam()
{
	guard(ParseParam);
	char	*val;

	if (!GetLine())
	{
		return false;
	}

	val = strchr(String, '=');

	if (!val)
	{
		return false;
	}

	value = atoi(val + 1);

	do
	{
		*val = 0;
		val--;
	}
	while (val >= String && *val <= ' ');

	return true;
	unguard;
}

//==========================================================================
//
//	GetClassFieldInt
//
//==========================================================================

static int GetClassFieldInt(VClass* Class, const char* FieldName, int Idx = 0)
{
	guard(GetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	vint32* Ptr = (vint32*)(Class->Defaults + F->Ofs);
	return Ptr[Idx];
	unguard;
}

//==========================================================================
//
//	SetClassFieldInt
//
//==========================================================================

static void SetClassFieldInt(VClass* Class, const char* FieldName,
	int Value, int Idx = 0)
{
	guard(SetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	vint32* Ptr = (vint32*)(Class->Defaults + F->Ofs);
	Ptr[Idx] = Value;
	unguard;
}

//==========================================================================
//
//	GetClassFieldState
//
//==========================================================================

static VState* GetClassFieldState(VClass* Class, const char* FieldName,
	int Idx = 0)
{
	guard(GetClassFieldState);
	VField* F = Class->FindFieldChecked(FieldName);
	VState** Ptr = (VState**)(Class->Defaults + F->Ofs);
	return Ptr[Idx];
	unguard;
}

//==========================================================================
//
//	SetClassFieldState
//
//==========================================================================

static void SetClassFieldState(VClass* Class, const char* FieldName,
	VState* Value, int Idx = 0)
{
	guard(SetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	VState** Ptr = (VState**)(Class->Defaults + F->Ofs);
	Ptr[Idx] = Value;
	unguard;
}

//==========================================================================
//
//	GetClassFieldClass
//
//==========================================================================

static VClass* GetClassFieldClass(VClass* Class, const char* FieldName,
	int Idx = 0)
{
	guard(GetClassFieldClass);
	VField* F = Class->FindFieldChecked(FieldName);
	VClass** Ptr = (VClass**)(Class->Defaults + F->Ofs);
	return Ptr[Idx];
	unguard;
}

//==========================================================================
//
//	ReadThing
//
//==========================================================================

static void ReadThing(int num)
{
	num--; // begin at 0 not 1;
/*	if (num >= NUMMOBJTYPES || num < 0)
	{
		dprintf("WARNING! Invalid thing num %d\n", num);
		while (ParseParam());
		return;
	}*/

	while (ParseParam())
	{
/*		if (!strcmp(String ,"ID #"))	    			mobjinfo[num].doomednum   =value;
		else if (!strcmp(String, "Initial frame"))		mobjinfo[num].spawnstate  =value;
		else if (!strcmp(String, "Hit points"))	    	mobjinfo[num].spawnhealth =value;
		else if (!strcmp(String, "First moving frame"))	mobjinfo[num].seestate    =value;
		else if (!strcmp(String, "Alert sound"))	    mobjinfo[num].seesound    =value;
		else if (!strcmp(String, "Reaction time"))   	mobjinfo[num].reactiontime=value;
		else if (!strcmp(String, "Attack sound"))	    mobjinfo[num].attacksound =value;
		else if (!strcmp(String, "Injury frame"))	    mobjinfo[num].painstate   =value;
		else if (!strcmp(String, "Pain chance"))     	mobjinfo[num].painchance  =value;
		else if (!strcmp(String, "Pain sound")) 		mobjinfo[num].painsound   =value;
		else if (!strcmp(String, "Close attack frame"))	mobjinfo[num].meleestate  =value;
		else if (!strcmp(String, "Far attack frame"))	mobjinfo[num].missilestate=value;
		else if (!strcmp(String, "Death frame"))	    mobjinfo[num].deathstate  =value;
		else if (!strcmp(String, "Exploding frame"))	mobjinfo[num].xdeathstate =value;
		else if (!strcmp(String, "Death sound")) 		mobjinfo[num].deathsound  =value;
		else if (!strcmp(String, "Speed"))	    		mobjinfo[num].speed       =value;
		else if (!strcmp(String, "Width"))	    		mobjinfo[num].radius      =value;
		else if (!strcmp(String, "Height"))	    		mobjinfo[num].height      =value;
		else if (!strcmp(String, "Mass"))	    		mobjinfo[num].mass	      =value;
		else if (!strcmp(String, "Missile damage"))		mobjinfo[num].damage      =value;
		else if (!strcmp(String, "Action sound"))		mobjinfo[num].activesound =value;
		else if (!strcmp(String, "Bits"))	    		mobjinfo[num].flags       =value;
		else if (!strcmp(String, "Respawn frame"))		mobjinfo[num].raisestate  =value;
		else */dprintf("WARNING! Invalid mobj param %s\n", String);
	}
}

//==========================================================================
//
//	ReadSound
//
//==========================================================================

static void ReadSound(int)
{
	guard(ReadSound);
	while (ParseParam())
	{
		if (!strcmp(String, "Offset"));				//Lump name offset - can't handle
		else if (!strcmp(String, "Zero/One"));		//Singularity - removed
		else if (!strcmp(String, "Value"));			//Priority
		else if (!strcmp(String, "Zero 1"));		//Lump num - can't be set
		else if (!strcmp(String, "Zero 2"));		//Data pointer - can't be set
		else if (!strcmp(String, "Zero 3"));		//Usefulness - removed
		else if (!strcmp(String, "Zero 4"));		//Link - removed
		else if (!strcmp(String, "Neg. One 1"));	//Link pitch - removed
		else if (!strcmp(String, "Neg. One 2"));	//Link volume - removed
		else dprintf("WARNING! Invalid sound param %s\n", String);
	}
	unguard;
}

//==========================================================================
//
//	ReadState
//
//==========================================================================

static void ReadState(int num)
{
	guard(ReadState);
	//	Check index.
	if (num >= States.Num() || num < 0)
	{
		dprintf("WARNING! Invalid state num %d\n", num);
		while (ParseParam());
		return;
	}

	//	State 0 is a special state.
	if (num == 0)
	{
		while (ParseParam());
		return;
	}

	while (ParseParam())
	{
		if (!strcmp(String, "Sprite number"))
		{
			States[num]->SpriteName = OrigSpriteNames[value];
			States[num]->SpriteIndex = VClass::FindSprite(OrigSpriteNames[value]);
		}
		else if (!strcmp(String, "Sprite subnumber"))
		{
			if (value & 0x8000)
			{
				value &= 0x7fff;
				value |= FF_FULLBRIGHT;
			}
			States[num]->Frame = value;
		}
		else if (!strcmp(String, "Duration"))
		{
			States[num]->Time = value < 0 ? value : value / 35.0;
		}
		else if (!strcmp(String, "Next frame"))
		{
			if (value >= States.Num() || value < 0)
			{
				dprintf("WARNING! Invalid next state %d\n", value);
			}
			else
			{
				States[num]->NextState = States[value];
			}
		}
		else if (!strcmp(String, "Unknown 1"))
		{
			States[num]->Misc1 = value;
		}
		else if (!strcmp(String, "Unknown 2"))
		{
			States[num]->Misc2 = value;
		}
		else if (!strcmp(String, "Action pointer"))
		{
			dprintf("WARNING! Tried to set action pointer.\n");
		}
		else
		{
			dprintf("WARNING! Invalid state param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadSpriteName
//
//==========================================================================

static void ReadSpriteName(int)
{
	guard(ReadSpriteName);
	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Offset"));	//	Can't handle
		else dprintf("WARNING! Invalid sprite name param %s\n", String);
	}
	unguard;
}

//==========================================================================
//
//	ReadAmmo
//
//==========================================================================

static void ReadAmmo(int num)
{
	guard(ReadAmmo);
	//	Check index.
	if (num >= 4 || num < 0)
	{
		dprintf("WARNING! Invalid ammo num %d\n", num);
		while (ParseParam());
		return;
	}

	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Max ammo"))
		{
			SetClassFieldInt(GameInfoClass, "maxammo", value, num);
		}
		else if (!VStr::ICmp(String, "Per ammo"))
		{
			SetClassFieldInt(GameInfoClass, "clipammo", value, num);
		}
		else
		{
			dprintf("WARNING! Invalid ammo param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadWeapon
//
//==========================================================================

static void ReadWeapon(int num)
{
	guard(ReadWeapon);
	//	Check index.
	if (num < 0 || num > 8)
	{
		dprintf("WARNING! Invalid weapon num %d\n", num);
		while (ParseParam());
		return;
	}

	VClass* Weapon = GetClassFieldClass(GameInfoClass, "WeaponClasses", num);
	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Ammo type"))
		{
			SetClassFieldInt(Weapon, "Ammo", value);
		}
		else if (!VStr::ICmp(String, "Deselect frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "UpState", States[value]);
			}
		}
		else if (!VStr::ICmp(String, "Select frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "DownState", States[value]);
			}
		}
		else if (!VStr::ICmp(String, "Bobbing frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "ReadyState", States[value]);
			}
		}
		else if (!VStr::ICmp(String, "Shooting frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "AttackState", States[value]);
			}
		}
		else if (!VStr::ICmp(String, "Firing frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "FlashState", States[value]);
			}
		}
		else
		{
			dprintf("WARNING! Invalid weapon param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadPointer
//
//==========================================================================

static void ReadPointer(int num)
{
	guard(ReadPointer);
	if (num < 0 || num >= CodePtrStates.Num())
	{
		dprintf("WARNING! Invalid pointer\n");
		while (ParseParam());
		return;
	}

	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Codep Frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid source state %d\n", value);
			}
			else
			{
				CodePtrStates[num]->Function = StateActions[value];
			}
		}
		else
		{
			dprintf("WARNING! Invalid pointer param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadCheats
//
//==========================================================================

static void ReadCheats(int)
{
	guard(ReadCheats);
	//	Old cheat handling is removed
	while (ParseParam());
	unguard;
}

//==========================================================================
//
//	ReadMisc
//
//==========================================================================

static void ReadMisc(int)
{
	guard(ReadMisc);
	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Initial Health"))
		{
			SetClassFieldInt(GameInfoClass, "INITIAL_HEALTH", value);
		}
		else if (!VStr::ICmp(String, "Initial Bullets"))
		{
			SetClassFieldInt(GameInfoClass, "INITIAL_AMMO", value);
		}
		else if (!VStr::ICmp(String, "Max Health"));
		else if (!VStr::ICmp(String, "Max Armor"));
		else if (!VStr::ICmp(String, "Green Armor Class"));
		else if (!VStr::ICmp(String, "Blue Armor Class"));
		else if (!VStr::ICmp(String, "Max Soulsphere"))
		{
			SetClassFieldInt(GameInfoClass, "SOULSPHERE_MAX", value);
		}
		else if (!VStr::ICmp(String, "Soulsphere Health"))
		{
			SetClassFieldInt(GameInfoClass, "SOULSPHERE_HEALTH", value);
		}
		else if (!VStr::ICmp(String, "Megasphere Health"))
		{
			SetClassFieldInt(GameInfoClass, "MEGASPHERE_HEALTH", value);
		}
		else if (!VStr::ICmp(String, "God Mode Health"))
		{
			SetClassFieldInt(GameInfoClass, "GOD_HEALTH", value);
		}
		else if (!VStr::ICmp(String, "IDFA Armor"));		//	Cheat removed
		else if (!VStr::ICmp(String, "IDFA Armor Class"));	//	Cheat removed
		else if (!VStr::ICmp(String, "IDKFA Armor"));		//	Cheat removed
		else if (!VStr::ICmp(String, "IDKFA Armor Class"));//	Cheat removed
		else if (!VStr::ICmp(String, "BFG Cells/Shot"))
		{
			SetClassFieldInt(GameInfoClass, "BFGCELLS", value);
		}
		else if (!VStr::ICmp(String, "Monsters Infight"));	//	What's that?
		else dprintf("WARNING! Invalid misc %s\n", String);
	}
	unguard;
}

//==========================================================================
//
//	FindString
//
//==========================================================================

static void FindString(const char* oldStr, const char* newStr)
{
	guard(FindString);
	//	Sounds
	bool SoundFound = false;
	for (int i = 0; i < SfxNames.Num(); i++)
	{
		if (SfxNames[i].Old == oldStr)
		{
			SfxNames[i].New = newStr;
			SfxNames[i].Replaced = true;
			//	Continue, because other sounds can use the same sound
			SoundFound = true;
		}
	}
	if (SoundFound)
	{
		return;
	}

	//	Music
	bool SongFound = false;
	for (int i = 0; i < MusicNames.Num(); i++)
	{
		if (MusicNames[i].Old == oldStr)
		{
			MusicNames[i].New = newStr;
			MusicNames[i].Replaced = true;
			//	There could be duplicates
			SongFound = true;
		}
	}
	if (SongFound)
	{
		return;
	}

	//	Sprite names
	for (int i = 0; i < SpriteNames.Num(); i++)
	{
		if (SpriteNames[i].Old == oldStr)
		{
			SpriteNames[i].New = newStr;
			SpriteNames[i].Replaced = true;
			return;
		}
	}

	VName Id = EngStrings->GetStringId(oldStr);
	if (Id != NAME_None)
	{
		GLanguage.ReplaceString(Id, newStr);
		return;
	}

	dprintf("Not found old \"%s\" new \"%s\"\n", oldStr, newStr);
	unguard;
}

//==========================================================================
//
//	ReadText
//
//==========================================================================

static void ReadText(int oldSize)
{
	guard(ReadText);
	char	*lenPtr;
	int		newSize;
	char	*oldStr;
	char	*newStr;
	int		len;

	lenPtr = strtok(NULL, " ");
	if (!lenPtr)
	{
		return;
	}
	newSize = atoi(lenPtr);

	oldStr = new char[oldSize + 1];
	newStr = new char[newSize + 1];

	len = 0;
	while (*PatchPtr && len < oldSize)
	{
		if (*PatchPtr == '\r')
		{
			PatchPtr++;
			continue;
		}
		oldStr[len] = *PatchPtr;
		PatchPtr++;
		len++;
	}
	oldStr[len] = 0;

	len = 0;
	while (*PatchPtr && len < newSize)
	{
		if (*PatchPtr == '\r')
		{
			PatchPtr++;
			continue;
		}
		newStr[len] = *PatchPtr;
		PatchPtr++;
		len++;
	}
	newStr[len] = 0;

	FindString(oldStr, newStr);

	delete[] oldStr;
	delete[] newStr;

	GetLine();
	unguard;
}

//==========================================================================
//
//	LoadDehackedFile
//
//==========================================================================

static void LoadDehackedFile(const char *filename)
{
	guard(LoadDehackedFile);
	char*	Section;
	char*	numStr;
	int		i = 0;

	dprintf("Hacking %s\n", filename);

	FILE* f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);
	Patch = new char[len + 1];
	fread(Patch, 1, len, f);
	Patch[len] = 0;
	fclose(f);
	PatchPtr = Patch;

	GetLine();
	while (*PatchPtr)
	{
		Section = strtok(String, " ");
		if (!Section)
			continue;

		numStr = strtok(NULL, " ");
		if (numStr)
		{
			i = atoi(numStr);
		}

		if (!strcmp(Section, "Thing"))
		{
			ReadThing(i);
		}
		else if (!strcmp(Section, "Sound"))
		{
			ReadSound(i);
		}
		else if (!strcmp(Section, "Frame"))
		{
			ReadState(i);
		}
		else if (!strcmp(Section, "Sprite"))
		{
			ReadSpriteName(i);
		}
		else if (!strcmp(Section, "Ammo"))
		{
			ReadAmmo(i);
		}
		else if (!strcmp(Section, "Weapon"))
		{
			ReadWeapon(i);
		}
		else if (!strcmp(Section, "Pointer"))
		{
			ReadPointer(i);
		}
		else if (!strcmp(Section, "Cheat"))
		{
			ReadCheats(i);
		}
		else if (!strcmp(Section, "Misc"))
		{
			ReadMisc(i);
		}
		else if (!strcmp(Section, "Text"))
		{
			ReadText(i);
		}
		else
		{
			dprintf("Don't know how to handle \"%s\"\n", String);
			GetLine();
		}
	}
	delete[] Patch;
	unguard;
}

//==========================================================================
//
//	ProcessDehackedFiles
//
//==========================================================================

void ProcessDehackedFiles()
{
	guard(ProcessDehackedFiles);
	int p = GArgs.CheckParm("-deh");
	if (!p)
	{
		return;
	}

	VClass* ActorClass = VClass::FindClass("Actor");
	States.Append(NULL);
	StateActions.Append(NULL);
	for (VState* S = ActorClass->States; S; S = S->Next)
	{
		States.Append(S);
		if (S->Function || S->InClassIndex + 1 == 738)
		{
			CodePtrStates.Append(S);
		}
		StateActions.Append(S->Function);
	}

	GameInfoClass = VClass::FindClass("MainGameInfo");

	GSoundManager->GetSoundLumpNames(SfxNames);
	P_GetMusicLumpNames(MusicNames);
	VClass::GetSpriteNames(SpriteNames);
	EngStrings = new VLanguage();
	EngStrings->LoadStrings("en");

	Hacked = true;

	while (++p != GArgs.Count() && GArgs[p][0] != '-')
	{
		LoadDehackedFile(GArgs[p]);
	}

	GSoundManager->ReplaceSoundLumpNames(SfxNames);
	P_ReplaceMusicLumpNames(MusicNames);
	VClass::ReplaceSpriteNames(SpriteNames);

	States.Clear();
	CodePtrStates.Clear();
	StateActions.Clear();
	SfxNames.Clear();
	MusicNames.Clear();
	SpriteNames.Clear();
	delete EngStrings;
	unguard;
}
