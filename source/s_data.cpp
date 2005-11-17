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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VRawSampleLoader : public VSampleLoader
{
public:
#pragma pack(1)
	struct FRawSoundHeader
	{
		word	Unknown;
		word	SampleRate;
		dword	DataSize;
	};
#pragma pack()

	void Load(sfxinfo_t&, FArchive&);
};

struct FPlayerSound
{
	int		ClassId;
	int		GenderId;
	int		RefId;
	int		SoundId;
};

enum ESoundType
{
	SNDTYPE_World = 0,
	SNDTYPE_Point = 1,
	SNDTYPE_Surround =2,

	SNDTYPE_Continuous = 4,
	SNDTYPE_Random = 8,
	SNDTYPE_Periodic = 12,
};

struct FAmbientSound
{
	dword		Type;		// type of ambient sound
	float		PeriodMin;	// # of tics between repeats
	float		PeriodMax;	// max # of tics for random ambients
	float		Volume;		// relative volume of sound
	float		Attenuation;
	FName		Sound;		// Logical name of sound to play
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void S_ParseSndinfo();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TArray<sfxinfo_t>	S_sfx;

VSampleLoader*		VSampleLoader::List;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VRawSampleLoader		RawSampleLoader;

static TArray<FName>		PlayerClasses;
static TArray<FName>		PlayerGenders;
static TArray<FPlayerSound>	PlayerSounds;
static int					NumPlayerReserves;
static float				CurrentChangePitch = 7.0 / 255.0;
static FAmbientSound*		AmbientSounds[256];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	AddSoundLump
//
//==========================================================================

static int AddSoundLump(FName TagName, int Lump)
{
	int id = S_sfx.AddZeroed();
	S_sfx[id].TagName = TagName;
	S_sfx[id].Data = NULL;
	S_sfx[id].Priority = 127;
	S_sfx[id].NumChannels = 2;
	S_sfx[id].ChangePitch = CurrentChangePitch;
	S_sfx[id].LumpNum = Lump;
	S_sfx[id].Link = -1;
	return id;
}

//==========================================================================
//
//	FindSound
//
//==========================================================================

static int FindSound(FName TagName)
{
	for (TArray<sfxinfo_t>::TIterator It(S_sfx); It; ++It)
	{
		if (It->TagName == TagName)
		{
			return It.GetIndex();
		}
	}
	return 0;
}

//==========================================================================
//
//	AddSound
//
//==========================================================================

static int AddSound(FName TagName, int Lump)
{
	int id = FindSound(TagName);

	if (id > 0)
	{
		// If the sound has already been defined, change the old definition
		sfxinfo_t* sfx = &S_sfx[id];

		//if (sfx->bPlayerReserve)
		//{
		//	SC_ScriptError("Sounds that are reserved for players cannot be reassigned");
		//}
		// Redefining a player compatibility sound will redefine the target instead.
		//if (sfx->bPlayerCompat)
		//{
		//	sfx = &S_sfx[sfx->link];
		//}
		if (sfx->bRandomHeader)
		{
			Z_Free(sfx->Sounds);
			sfx->Sounds = NULL;
		}
		sfx->LumpNum = Lump;
		sfx->bRandomHeader = false;
		sfx->Link = -1;
	}
	else
	{
		// Otherwise, create a new definition.
		id = AddSoundLump(TagName, Lump);
	}

	return id;
}

//==========================================================================
//
//	FindOrAddSound
//
//==========================================================================

static int FindOrAddSound(FName TagName)
{
	int id = FindSound(TagName);
	return id ? id : AddSoundLump(TagName, -1);
}

//==========================================================================
//
//	FindPlayerClass
//
//==========================================================================

static int FindPlayerClass(FName CName)
{
	for (int i = 0; i < PlayerClasses.Num(); i++)
		if (PlayerClasses[i] == CName)
			return i;
	return -1;
}

//==========================================================================
//
//	AddPlayerClass
//
//==========================================================================

static int AddPlayerClass(FName CName)
{
	int idx = FindPlayerClass(CName);
	return idx == -1 ? PlayerClasses.AddItem(CName) : idx;
}

//==========================================================================
//
//	FindPlayerGender
//
//==========================================================================

static int FindPlayerGender(FName GName)
{
	for (int i = 0; i < PlayerGenders.Num(); i++)
		if (PlayerGenders[i] == GName)
			return i;
	return -1;
}

//==========================================================================
//
//	AddPlayerGender
//
//==========================================================================

static int AddPlayerGender(FName GName)
{
	int idx = FindPlayerGender(GName);
	return idx == -1 ? PlayerGenders.AddItem(GName) : idx;
}

//==========================================================================
//
//	ParsePlayerSoundCommon
//
//	Parses the common part of playersound commands in SNDINFO
// (player class, gender, and ref id)
//==========================================================================

static void ParsePlayerSoundCommon(int& PClass, int& Gender, int& RefId)
{
	SC_MustGetString();
	PClass = AddPlayerClass(sc_String);
	SC_MustGetString();
	Gender = AddPlayerGender(sc_String);
	SC_MustGetString();
	RefId = FindSound(sc_String);
	if (!S_sfx[RefId].bPlayerReserve)
	{
		SC_ScriptError(va("%s has not been reserved for a player sound", sc_String));
	}
	SC_MustGetString();
}

//==========================================================================
//
//	FindPlayerSound
//
//==========================================================================

static int FindPlayerSound(int PClass, int Gender, int RefId)
{
	for (int i = 0; i < PlayerSounds.Num(); i++)
	{
		if (PlayerSounds[i].ClassId == PClass &&
			PlayerSounds[i].GenderId == Gender &&
			PlayerSounds[i].RefId == RefId)
		{
			return PlayerSounds[i].SoundId;
		}
	}
	return 0;
}

//==========================================================================
//
//	S_InitScript
//
//	Loads sound script lump or file, if param -devsnd was specified
//
//==========================================================================

void S_InitScript()
{
	guard(S_InitScript);
	int Lump;

	//	Sound 0 is empty sound.
	AddSoundLump(NAME_None, -1);

	//	Add Strife voices.
	for (Lump = W_IterateNS(-1, WADNS_Voices); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Voices))
	{
		char SndName[16];
		sprintf(SndName, "svox/%s", *FName(W_LumpName(Lump), FNAME_AddLower8));

		int id = AddSoundLump(SndName, Lump);
		S_sfx[id].ChangePitch = 0;
	}

	//	Load script SNDINFO
	for (Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (!stricmp(W_LumpName(Lump), "sndinfo"))
		{
			SC_OpenLumpNum(Lump);
			S_ParseSndinfo();
		}
	}

	//	Optionally parse script file.
	char filename[MAX_OSPATH];
	if (fl_devmode && FL_FindFile("scripts/sndinfo.txt", filename))
	{
		SC_OpenFile(filename);
		S_ParseSndinfo();
	}

	S_sfx.Shrink();
	unguard;
}

//==========================================================================
//
//	S_ParseSndinfo
//
//==========================================================================

static void S_ParseSndinfo()
{
	TArray<int>		list;

	while (SC_GetString())
	{
		if (*sc_String == '$')
		{
			if (!stricmp(sc_String, "$archivepath"))
			{
				// $archivepath <directory>
				//	Ignored.
				SC_MustGetString();
			}
			else if (!stricmp(sc_String, "$map"))
			{
				// $map <map number> <song name>
				SC_MustGetNumber();
				SC_MustGetString();
				if (sc_Number)
				{
					P_PutMapSongLump(sc_Number, sc_String);
				}
			}
			else if (!stricmp(sc_String, "$registered"))
			{
				// $registered
				//	Unused.
			}
			else if (!stricmp(sc_String, "$limit"))
			{
				// $limit <logical name> <max channels>
				SC_MustGetString();
				int sfx = FindOrAddSound(sc_String);
				SC_MustGetNumber();
				S_sfx[sfx].NumChannels = sc_Number < 0 ? 0 : sc_Number > 255 ? 255 : sc_Number;
			}
			else if (!stricmp(sc_String, "$pitchshift"))
			{
				// $pitchshift <logical name> <pitch shift amount>
				SC_MustGetString();
				int sfx = FindOrAddSound(sc_String);
				SC_MustGetNumber();
				S_sfx[sfx].ChangePitch = ((1 << (sc_Number < 0 ? 0 : sc_Number > 7 ? 7 : sc_Number)) - 1) / 255.0;
			}
			else if (!stricmp(sc_String, "$pitchshiftrange"))
			{
				// $pitchshiftrange <pitch shift amount>
				SC_MustGetNumber();
				CurrentChangePitch = ((1 << (sc_Number < 0 ? 0 : sc_Number > 7 ? 7 : sc_Number)) - 1) / 255.0;
			}
			else if (!stricmp(sc_String, "$alias"))
			{
				// $alias <name of alias> <name of real sound>
				SC_MustGetString();
				int sfxfrom = AddSound(sc_String, -1);
				SC_MustGetString();
				//if (S_sfx[sfxfrom].bPlayerCompat)
				//{
				//	sfxfrom = S_sfx[sfxfrom].link;
				//}
				S_sfx[sfxfrom].Link = FindOrAddSound(sc_String);
			}
			else if (!stricmp(sc_String, "$random"))
			{
				// $random <logical name> { <logical name> ... }
				list.Empty();
				SC_MustGetString();
				int id = AddSound(sc_String, -1);
				SC_MustGetStringName("{");
				while (SC_GetString() && !SC_Compare("}"))
				{
					int sfxto = FindOrAddSound(sc_String);
					list.AddItem(sfxto);
				}
				if (list.Num() == 1)
				{
					// Only one sound: treat as $alias
					S_sfx[id].Link = list[0];
				}
				else if (list.Num() > 1)
				{
					// Only add non-empty random lists
					S_sfx[id].Link = list.Num();
					S_sfx[id].Sounds = Z_CNew<int>(list.Num());
					memcpy(S_sfx[id].Sounds, &list[0], sizeof(int) * list.Num());
					S_sfx[id].bRandomHeader = true;
				}
			}
			else if (!stricmp(sc_String, "$playerreserve"))
			{
				// $playerreserve <logical name>
				SC_MustGetString();
				int id = AddSound(sc_String, -1);
				S_sfx[id].Link = NumPlayerReserves++;
				S_sfx[id].bPlayerReserve = true;
			}
			else if (!stricmp(sc_String, "$playersound"))
			{
				// $playersound <player class> <gender> <logical name> <lump name>
				int PClass, Gender, RefId;
				char FakeName[NAME_SIZE];
				size_t len;
				int id;

				ParsePlayerSoundCommon(PClass, Gender, RefId);
				len = strlen(*PlayerClasses[PClass]);
				memcpy(FakeName, *PlayerClasses[PClass], len);
				FakeName[len] = '|';
				FakeName[len + 1] = Gender + '0';
				strcpy(&FakeName[len + 2], *S_sfx[RefId].TagName);

				id = AddSoundLump(FakeName, W_CheckNumForName(sc_String));
				int PlrSndId = PlayerSounds.Add();
				PlayerSounds[PlrSndId].ClassId = PClass;
				PlayerSounds[PlrSndId].GenderId = Gender;
				PlayerSounds[PlrSndId].RefId = RefId;
				PlayerSounds[PlrSndId].SoundId = id;
			}
			else if (!stricmp(sc_String, "$playersounddup"))
			{
				// $playersounddup <player class> <gender> <logical name> <target sound name>
				int PClass, Gender, RefId, TargId;

				ParsePlayerSoundCommon(PClass, Gender, RefId);
				TargId = FindSound(sc_String);
				if (!S_sfx[TargId].bPlayerReserve)
				{
					SC_ScriptError(va("%s is not a player sound", sc_String));
				}
				int AliasTo = FindPlayerSound(PClass, Gender, TargId);
				int PlrSndId = PlayerSounds.Add();
				PlayerSounds[PlrSndId].ClassId = PClass;
				PlayerSounds[PlrSndId].GenderId = Gender;
				PlayerSounds[PlrSndId].RefId = RefId;
				PlayerSounds[PlrSndId].SoundId = AliasTo;
			}
			else if (!stricmp(sc_String, "$playeralias"))
			{
				// $playeralias <player class> <gender> <logical name> <logical name of existing sound>
				int PClass, Gender, RefId;

				ParsePlayerSoundCommon(PClass, Gender, RefId);
				int AliasTo = FindOrAddSound(sc_String);
				int PlrSndId = PlayerSounds.Add();
				PlayerSounds[PlrSndId].ClassId = PClass;
				PlayerSounds[PlrSndId].GenderId = Gender;
				PlayerSounds[PlrSndId].RefId = RefId;
				PlayerSounds[PlrSndId].SoundId = AliasTo;
			}
			else if (!stricmp(sc_String, "$singular"))
			{
				// $singular <logical name>
				SC_MustGetString();
				int sfx = FindOrAddSound(sc_String);
				S_sfx[sfx].bSingular = true;
			}
			else if (!stricmp(sc_String, "$ambient"))
			{
				// $ambient <num> <logical name> [point [atten] | surround | [world]]
				//			<continuous | random <minsecs> <maxsecs> | periodic <secs>>
				//			<volume>
				FAmbientSound* ambient, dummy;

				SC_MustGetNumber();
				if (sc_Number < 0 || sc_Number > 255)
				{
					GCon->Logf("Bad ambient index (%d)", sc_Number);
					ambient = &dummy;
				}
				else
				{
					ambient = new FAmbientSound;
					AmbientSounds[sc_Number] = ambient;
				}
				memset(ambient, 0, sizeof(FAmbientSound));

				SC_MustGetString();
				ambient->Sound = sc_String;
				ambient->Attenuation = 0;

				SC_MustGetString();
				if (SC_Compare("point"))
				{
					float attenuation;

					ambient->Type = SNDTYPE_Point;
					if (SC_CheckFloat())
					{
						attenuation = sc_Float;
						if (attenuation > 0)
						{
							ambient->Attenuation = attenuation;
						}
						else
						{
							ambient->Attenuation = 1;
						}
					}
					else
					{
						ambient->Attenuation = 1;
					}
					SC_MustGetString();
				}
				else if (SC_Compare("surround"))
				{
					ambient->Type = SNDTYPE_Surround;
					SC_MustGetString();
					ambient->Attenuation = -1;
				}
				else if (SC_Compare("world"))
				{
					// World is an optional keyword
					SC_MustGetString();
				}

				if (SC_Compare("continuous"))
				{
					ambient->Type |= SNDTYPE_Continuous;
				}
				else if (SC_Compare("random"))
				{
					ambient->Type |= SNDTYPE_Random;
					SC_MustGetFloat();
					ambient->PeriodMin = sc_Float;
					SC_MustGetFloat();
					ambient->PeriodMax = sc_Float;
				}
				else if (SC_Compare("periodic"))
				{
					ambient->Type |= SNDTYPE_Periodic;
					SC_MustGetFloat();
					ambient->PeriodMin = sc_Float;
				}
				else
				{
					GCon->Logf("Unknown ambient type (%s)", sc_String);
				}

				SC_MustGetFloat();
				ambient->Volume = sc_Float;
				if (ambient->Volume > 1)
					ambient->Volume = 1;
				else if (ambient->Volume < 0)
					ambient->Volume = 0;
			}
			continue;
		}
		else
		{
			FName TagName = sc_String;
			SC_MustGetString();
			AddSound(TagName, W_CheckNumForName(sc_String));
		}
	}
	SC_Close();
}

//==========================================================================
//
//	S_GetSoundID
//
//==========================================================================

int S_GetSoundID(FName Name)
{
	guard(S_GetSoundID);
	for (TArray<sfxinfo_t>::TIterator It(S_sfx); It; ++It)
	{
		if (It->TagName == Name)
		{
			return It.GetIndex();
		}
	}
	GCon->Logf("WARNING! Can't find sound %s", *Name);
	return 0;
	unguard;
}

//==========================================================================
//
//	S_GetSoundID
//
//==========================================================================

int S_GetSoundID(const char *name)
{
	guard(S_GetSoundID);
	for (TArray<sfxinfo_t>::TIterator It(S_sfx); It; ++It)
	{
//FIXME really case sensitive? What about ACS?
		if (!strcmp(*It->TagName, name))
		{
			return It.GetIndex();
		}
	}
	GCon->Logf("WARNING! Can't find sound named %s", name);
	return 0;
	unguard;
}

//==========================================================================
//
//	LookupPlayerSound
//
//==========================================================================

static int LookupPlayerSound(int ClassId, int GenderId, int RefId)
{
	int Id = FindPlayerSound(ClassId, GenderId, RefId);
	if (Id == 0 || (S_sfx[Id].LumpNum == -1 && S_sfx[Id].Link == -1))
	{
		// This sound is unavailable.
		if (GenderId)
		{
			// Try "male"
			return LookupPlayerSound(ClassId, 0, RefId);
		}
		if (ClassId)
		{
			// Try the default class.
			return LookupPlayerSound(0, GenderId, RefId);
		}
	}
	return Id;
}

//==========================================================================
//
//	ResolveSound
//
//==========================================================================

static int ResolveSound(int ClassID, int GenderID, int InSoundId)
{
	guard(ResolveSound);
	int sound_id = InSoundId;
	while (S_sfx[sound_id].Link != -1)
	{
		if (S_sfx[sound_id].bPlayerReserve)
		{
			sound_id = FindPlayerSound(ClassID, GenderID, sound_id);
		}
		else if (S_sfx[sound_id].bRandomHeader)
		{
			sound_id = S_sfx[sound_id].Sounds[rand() % S_sfx[sound_id].Link];
		}
		else
		{
			sound_id = S_sfx[sound_id].Link;
		}
	}
	return sound_id;
	unguard;
}

//==========================================================================
//
//	S_ResolveSound
//
//==========================================================================

int S_ResolveSound(int InSoundId)
{
	return ResolveSound(0, 0, InSoundId);
}

//==========================================================================
//
//	S_ResolveEntitySound
//
//==========================================================================

int S_ResolveEntitySound(FName ClassName, FName GenderName, FName SoundName)
{
	guard(S_ResolveEntitySound);
	int ClassId = FindPlayerClass(ClassName);
	if (ClassId == -1)
		ClassId = 0;
	int GenderId = FindPlayerGender(GenderName);
	if (GenderId == -1)
		GenderId = 0;
	int SoundId = S_GetSoundID(SoundName);
	return ResolveSound(ClassId, GenderId, SoundId);
	unguard;
}

#ifdef CLIENT

//==========================================================================
//
//	S_LoadSound
//
//==========================================================================

bool S_LoadSound(int sound_id)
{
	guard(S_LoadSound);
	if (!S_sfx[sound_id].Data)
	{
		FArchive* Ar = FL_OpenFileRead(va("sound/%s.flac", W_LumpName(S_sfx[sound_id].LumpNum)));
		if (!Ar)
			Ar = FL_OpenFileRead(va("sound/%s.wav", W_LumpName(S_sfx[sound_id].LumpNum)));
		if (!Ar)
			Ar = FL_OpenFileRead(va("sound/%s.raw", W_LumpName(S_sfx[sound_id].LumpNum)));
		if (!Ar)
		{
			// get LumpNum if necessary
			if (S_sfx[sound_id].LumpNum < 0)
			{
				GCon->Logf(NAME_Dev, "Sound %s lump not found",
					*S_sfx[sound_id].TagName);
				return false;
			}
			Ar = W_CreateLumpReader(S_sfx[sound_id].LumpNum);
		}

		for (VSampleLoader* Ldr = VSampleLoader::List;
			!S_sfx[sound_id].Data; Ldr = Ldr->Next)
		{
			Ldr->Load(S_sfx[sound_id], *Ar);
		}
		delete Ar;
	}
	Z_ChangeTag(S_sfx[sound_id].Data, PU_SOUND);
	S_sfx[sound_id].UseCount++;
	return true;
	unguard;
}

//==========================================================================
//
//	S_DoneWithLump
//
//==========================================================================

void S_DoneWithLump(int sound_id)
{
	guard(S_DoneWithLump);
	sfxinfo_t &sfx = S_sfx[sound_id];
	if (!sfx.Data || !sfx.UseCount)
	{
		Sys_Error("S_DoneWithLump: Empty lump");
	}

	sfx.UseCount--;
	if (sfx.UseCount)
	{
		//	still used
		return;
	}

	//	Make data cachable
	Z_ChangeTag(sfx.Data, PU_CACHE);
	unguard;
}

//==========================================================================
//
//	VRawSampleLoader::Load
//
//==========================================================================

void VRawSampleLoader::Load(sfxinfo_t& Sfx, FArchive& Ar)
{
	guard(VRawSampleLoader::Load);
	//	Read header and see if it's a valid raw sample.
	FRawSoundHeader Hdr;
	Ar.Seek(0);
	Ar.Serialise(&Hdr, 8);
	int Rate = LittleShort(Hdr.SampleRate);
	if ((Rate != 11025 && Rate != 22050 && Rate != 44100) ||
		LittleLong(Hdr.DataSize) != Ar.TotalSize() - 8)
	{
		return;
	}

	Sfx.SampleBits = 8;
	Sfx.SampleRate = LittleShort(Hdr.SampleRate);
	Sfx.DataSize = LittleLong(Hdr.DataSize);
	Sfx.Data = Z_Malloc(Sfx.DataSize, PU_SOUND, &Sfx.Data);
	Ar.Serialise(Sfx.Data, Sfx.DataSize);
	unguard;
}

#endif
#ifndef CLIENT
//==========================================================================
//
//	S_Init
//
//==========================================================================

void S_Init(void)
{
	S_InitScript();
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.17  2005/11/17 18:53:21  dj_jl
//	Implemented support for sndinfo extensions.
//
//	Revision 1.16  2005/11/07 22:57:09  dj_jl
//	Some M$VC fixes.
//	
//	Revision 1.15  2005/11/06 15:27:09  dj_jl
//	Added support for 16 bit sounds.
//	
//	Revision 1.14  2005/11/05 15:50:07  dj_jl
//	Voices played as normal sounds.
//	
//	Revision 1.13  2005/10/20 22:31:27  dj_jl
//	Removed Hexen's devsnd support.
//	
//	Revision 1.12  2004/11/30 07:17:17  dj_jl
//	Made string pointers const.
//	
//	Revision 1.11  2004/11/23 12:43:10  dj_jl
//	Wad file lump namespaces.
//	
//	Revision 1.10  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.9  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.8  2002/07/20 14:50:24  dj_jl
//	Missing sound data will not crash game anymore.
//	
//	Revision 1.7  2002/01/12 18:05:00  dj_jl
//	Beautification
//	
//	Revision 1.6  2002/01/11 08:11:05  dj_jl
//	Changes in sound list
//	Added guard macros
//	
//	Revision 1.5  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
