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

#define NUM_AMBIENT_SOUNDS		256

// TYPES -------------------------------------------------------------------

#ifdef CLIENT
class VRawSampleLoader : public VSampleLoader
{
public:
	void Load(sfxinfo_t&, VStream&);
};
#endif

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
	VName		Sound;		// Logical name of sound to play
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

#ifdef CLIENT
static VRawSampleLoader		RawSampleLoader;
#endif

static TArray<VName>		PlayerClasses;
static TArray<VName>		PlayerGenders;
static TArray<FPlayerSound>	PlayerSounds;
static int					NumPlayerReserves;
static float				CurrentChangePitch = 7.0 / 255.0;
static FAmbientSound*		AmbientSounds[NUM_AMBIENT_SOUNDS];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	AddSoundLump
//
//==========================================================================

static int AddSoundLump(VName TagName, int Lump)
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

static int FindSound(VName TagName)
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

static int AddSound(VName TagName, int Lump)
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
			delete[] sfx->Sounds;
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

static int FindOrAddSound(VName TagName)
{
	int id = FindSound(TagName);
	return id ? id : AddSoundLump(TagName, -1);
}

//==========================================================================
//
//	FindPlayerClass
//
//==========================================================================

static int FindPlayerClass(VName CName)
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

static int AddPlayerClass(VName CName)
{
	int idx = FindPlayerClass(CName);
	return idx == -1 ? PlayerClasses.AddItem(CName) : idx;
}

//==========================================================================
//
//	FindPlayerGender
//
//==========================================================================

static int FindPlayerGender(VName GName)
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

static int AddPlayerGender(VName GName)
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
		sprintf(SndName, "svox/%s", *W_LumpName(Lump));

		int id = AddSoundLump(SndName, Lump);
		S_sfx[id].ChangePitch = 0;
	}

	//	Load script SNDINFO
	for (Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_sndinfo)
		{
			SC_OpenLumpNum(Lump);
			S_ParseSndinfo();
		}
	}

	//	Optionally parse script file.
	VStr filename = FL_FindFile("scripts/sndinfo.txt");
	if (fl_devmode && filename)
	{
		SC_OpenFile(*filename);
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
					S_sfx[id].Sounds = new int[list.Num()];
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

				id = AddSoundLump(FakeName, W_CheckNumForName(
					VName(sc_String, VName::AddLower8)));
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
				if (sc_Number < 0 || sc_Number >= NUM_AMBIENT_SOUNDS)
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
			VName TagName = sc_String;
			SC_MustGetString();
			AddSound(TagName, W_CheckNumForName(VName(sc_String, VName::AddLower8)));
		}
	}
	SC_Close();
}

//==========================================================================
//
//	S_GetSoundID
//
//==========================================================================

int S_GetSoundID(VName Name)
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

int S_ResolveEntitySound(VName ClassName, VName GenderName, VName SoundName)
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
		VStream* Strm = FL_OpenFileRead(va("sound/%s.flac", *W_LumpName(S_sfx[sound_id].LumpNum)));
		if (!Strm)
			Strm = FL_OpenFileRead(va("sound/%s.wav", *W_LumpName(S_sfx[sound_id].LumpNum)));
		if (!Strm)
			Strm = FL_OpenFileRead(va("sound/%s.raw", *W_LumpName(S_sfx[sound_id].LumpNum)));
		if (!Strm)
		{
			// get LumpNum if necessary
			if (S_sfx[sound_id].LumpNum < 0)
			{
				GCon->Logf(NAME_Dev, "Sound %s lump not found",
					*S_sfx[sound_id].TagName);
				return false;
			}
			Strm = W_CreateLumpReaderNum(S_sfx[sound_id].LumpNum);
		}

		for (VSampleLoader* Ldr = VSampleLoader::List;
			Ldr && !S_sfx[sound_id].Data; Ldr = Ldr->Next)
		{
			Ldr->Load(S_sfx[sound_id], *Strm);
		}
		delete Strm;
		if (!S_sfx[sound_id].Data)
		{
			GCon->Logf(NAME_Dev, "Failed to load sound %s",
				*S_sfx[sound_id].TagName);
			return false;
		}
	}
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
	unguard;
}

//==========================================================================
//
//	VRawSampleLoader::Load
//
//==========================================================================

void VRawSampleLoader::Load(sfxinfo_t& Sfx, VStream& Strm)
{
	guard(VRawSampleLoader::Load);
	//	Read header and see if it's a valid raw sample.
	vuint16		Unknown;
	vuint16		SampleRate;
	vuint32		DataSize;

	Strm.Seek(0);
	Strm << Unknown
		<< SampleRate
		<< DataSize;
	if ((SampleRate != 11025 && SampleRate != 22050 && SampleRate != 44100) ||
		(vint32)DataSize != Strm.TotalSize() - 8)
	{
		return;
	}

	Sfx.SampleBits = 8;
	Sfx.SampleRate = SampleRate;
	Sfx.DataSize = DataSize;
	Sfx.Data = Z_Malloc(Sfx.DataSize, PU_STATIC, 0);
	Strm.Serialise(Sfx.Data, Sfx.DataSize);
	unguard;
}

#endif

//==========================================================================
//
//	S_ShutdownData
//
//==========================================================================

void S_ShutdownData()
{
	guard(S_ShutdownData);
	SN_FreeSequenceData();

	for (int i = 0; i < S_sfx.Num(); i++)
	{
		if (S_sfx[i].Data)
		{
			Z_Free(S_sfx[i].Data);
		}
		if (S_sfx[i].Sounds)
		{
			delete[] S_sfx[i].Sounds;
		}
	}

	S_sfx.Empty();
	PlayerClasses.Empty();
	PlayerGenders.Empty();
	PlayerSounds.Empty();

	for (int i = 0; i < NUM_AMBIENT_SOUNDS; i++)
	{
		if (AmbientSounds[i])
		{
			delete AmbientSounds[i];
		}
	}
	unguard;
}
