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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

#ifdef CLIENT
class VRawSampleLoader : public VSampleLoader
{
public:
	void Load(sfxinfo_t&, VStream&);
};
#endif

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VSampleLoader*		VSampleLoader::List;

VSoundManager*		GSoundManager;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

#ifdef CLIENT
static VRawSampleLoader		RawSampleLoader;
#endif

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSoundManager::VSoundManager
//
//==========================================================================

VSoundManager::VSoundManager()
: NumPlayerReserves(0)
, CurrentChangePitch(7.0 / 255.0)
{
	memset(AmbientSounds, 0, sizeof(AmbientSounds));
}

//==========================================================================
//
//	VSoundManager::~VSoundManager
//
//==========================================================================

VSoundManager::~VSoundManager()
{
	guard(VSoundManager::~VSoundManager);
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

	for (int i = 0; i < NUM_AMBIENT_SOUNDS; i++)
	{
		if (AmbientSounds[i])
		{
			delete AmbientSounds[i];
		}
	}

	for (int i = 0; i < SeqInfo.Num(); i++)
	{
		if (SeqInfo[i].Data)
		{
			delete[] SeqInfo[i].Data;
		}
	}
	unguard;
}

//==========================================================================
//
//	VSoundManager::Init
//
//	Loads sound script lump or file, if param -devsnd was specified
//
//==========================================================================

void VSoundManager::Init()
{
	guard(VSoundManager::Init);
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
			ParseSndinfo(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)));
		}
	}

	//	Optionally parse script file.
	if (fl_devmode && FL_FileExists("scripts/sndinfo.txt"))
	{
		ParseSndinfo(new VScriptParser("scripts/sndinfo.txt",
			FL_OpenFileRead("scripts/sndinfo.txt")));
	}

	S_sfx.Condense();

	//	Load script SNDSEQ
	memset(SeqTrans, -1, sizeof(SeqTrans));
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_sndseq)
		{
			ParseSequenceScript(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)));
		}
	}

	//	Optionally parse script file.
	if (fl_devmode && FL_FileExists("scripts/sndseq.txt"))
	{
		ParseSequenceScript(new VScriptParser("scripts/sndseq.txt",
			FL_OpenFileRead("scripts/sndseq.txt")));
	}
	unguard;
}

//==========================================================================
//
//	VSoundManager::ParseSndinfo
//
//==========================================================================

void VSoundManager::ParseSndinfo(VScriptParser* sc)
{
	guard(VSoundManager::ParseSndinfo);
	TArray<int>		list;

	while (!sc->AtEnd())
	{
		if (sc->Check("$archivepath"))
		{
			// $archivepath <directory>
			//	Ignored.
			sc->ExpectString();
		}
		else if (sc->Check("$map"))
		{
			// $map <map number> <song name>
			sc->ExpectNumber();
			sc->ExpectName8();
			if (sc->Number)
			{
				P_PutMapSongLump(sc->Number, sc->Name8);
			}
		}
		else if (sc->Check("$registered"))
		{
			// $registered
			//	Unused.
		}
		else if (sc->Check("$limit"))
		{
			// $limit <logical name> <max channels>
			sc->ExpectString();
			int sfx = FindOrAddSound(*sc->String);
			sc->ExpectNumber();
			S_sfx[sfx].NumChannels = MID(0, sc->Number, 255);
		}
		else if (sc->Check("$pitchshift"))
		{
			// $pitchshift <logical name> <pitch shift amount>
			sc->ExpectString();
			int sfx = FindOrAddSound(*sc->String);
			sc->ExpectNumber();
			S_sfx[sfx].ChangePitch = ((1 << MID(0, sc->Number, 7)) - 1) / 255.0;
		}
		else if (sc->Check("$pitchshiftrange"))
		{
			// $pitchshiftrange <pitch shift amount>
			sc->ExpectNumber();
			CurrentChangePitch = ((1 << MID(0, sc->Number, 7)) - 1) / 255.0;
		}
		else if (sc->Check("$alias"))
		{
			// $alias <name of alias> <name of real sound>
			sc->ExpectString();
			int sfxfrom = AddSound(*sc->String, -1);
			sc->ExpectString();
			//if (S_sfx[sfxfrom].bPlayerCompat)
			//{
			//	sfxfrom = S_sfx[sfxfrom].link;
			//}
			S_sfx[sfxfrom].Link = FindOrAddSound(*sc->String);
		}
		else if (sc->Check("$random"))
		{
			// $random <logical name> { <logical name> ... }
			list.Clear();
			sc->ExpectString();
			int id = AddSound(*sc->String, -1);
			sc->Expect("{");
			while (!sc->Check("}"))
			{
				sc->ExpectString();
				int sfxto = FindOrAddSound(*sc->String);
				list.Append(sfxto);
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
		else if (sc->Check("$playerreserve"))
		{
			// $playerreserve <logical name>
			sc->ExpectString();
			int id = AddSound(*sc->String, -1);
			S_sfx[id].Link = NumPlayerReserves++;
			S_sfx[id].bPlayerReserve = true;
		}
		else if (sc->Check("$playersound"))
		{
			// $playersound <player class> <gender> <logical name> <lump name>
			int PClass, Gender, RefId;
			char FakeName[NAME_SIZE];
			size_t len;
			int id;

			ParsePlayerSoundCommon(sc, PClass, Gender, RefId);
			len = VStr::Length(*PlayerClasses[PClass]);
			memcpy(FakeName, *PlayerClasses[PClass], len);
			FakeName[len] = '|';
			FakeName[len + 1] = Gender + '0';
			VStr::Cpy(&FakeName[len + 2], *S_sfx[RefId].TagName);

			id = AddSoundLump(FakeName, W_CheckNumForName(
				VName(*sc->String, VName::AddLower8)));
			FPlayerSound& PlrSnd = PlayerSounds.Alloc();
			PlrSnd.ClassId = PClass;
			PlrSnd.GenderId = Gender;
			PlrSnd.RefId = RefId;
			PlrSnd.SoundId = id;
		}
		else if (sc->Check("$playersounddup"))
		{
			// $playersounddup <player class> <gender> <logical name> <target sound name>
			int PClass, Gender, RefId, TargId;

			ParsePlayerSoundCommon(sc, PClass, Gender, RefId);
			TargId = FindSound(*sc->String);
			if (!S_sfx[TargId].bPlayerReserve)
			{
				sc->Error(va("%s is not a player sound", *sc->String));
			}
			int AliasTo = FindPlayerSound(PClass, Gender, TargId);
			FPlayerSound& PlrSnd = PlayerSounds.Alloc();
			PlrSnd.ClassId = PClass;
			PlrSnd.GenderId = Gender;
			PlrSnd.RefId = RefId;
			PlrSnd.SoundId = AliasTo;
		}
		else if (sc->Check("$playeralias"))
		{
			// $playeralias <player class> <gender> <logical name> <logical name of existing sound>
			int PClass, Gender, RefId;

			ParsePlayerSoundCommon(sc, PClass, Gender, RefId);
			int AliasTo = FindOrAddSound(*sc->String);
			FPlayerSound& PlrSnd = PlayerSounds.Alloc();
			PlrSnd.ClassId = PClass;
			PlrSnd.GenderId = Gender;
			PlrSnd.RefId = RefId;
			PlrSnd.SoundId = AliasTo;
		}
		else if (sc->Check("$singular"))
		{
			// $singular <logical name>
			sc->ExpectString();
			int sfx = FindOrAddSound(*sc->String);
			S_sfx[sfx].bSingular = true;
		}
		else if (sc->Check("$ambient"))
		{
			// $ambient <num> <logical name> [point [atten] | surround | [world]]
			//			<continuous | random <minsecs> <maxsecs> | periodic <secs>>
			//			<volume>
			FAmbientSound* ambient, dummy;

			sc->ExpectNumber();
			if (sc->Number < 0 || sc->Number >= NUM_AMBIENT_SOUNDS)
			{
				GCon->Logf("Bad ambient index (%d)", sc->Number);
				ambient = &dummy;
			}
			else if (AmbientSounds[sc->Number])
			{
				ambient = AmbientSounds[sc->Number];
			}
			else
			{
				ambient = new FAmbientSound;
				AmbientSounds[sc->Number] = ambient;
			}
			memset(ambient, 0, sizeof(FAmbientSound));

			sc->ExpectString();
			ambient->Sound = *sc->String;
			ambient->Attenuation = 0;

			if (sc->Check("point"))
			{
				ambient->Type = SNDTYPE_Point;
				if (sc->CheckFloat())
				{
					if (sc->Float > 0)
					{
						ambient->Attenuation = sc->Float;
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
			}
			else if (sc->Check("surround"))
			{
				ambient->Type = SNDTYPE_Surround;
				ambient->Attenuation = -1;
			}
			else if (sc->Check("world"))
			{
				// World is an optional keyword
			}

			if (sc->Check("continuous"))
			{
				ambient->Type |= SNDTYPE_Continuous;
			}
			else if (sc->Check("random"))
			{
				ambient->Type |= SNDTYPE_Random;
				sc->ExpectFloat();
				ambient->PeriodMin = sc->Float;
				sc->ExpectFloat();
				ambient->PeriodMax = sc->Float;
			}
			else if (sc->Check("periodic"))
			{
				ambient->Type |= SNDTYPE_Periodic;
				sc->ExpectFloat();
				ambient->PeriodMin = sc->Float;
			}
			else
			{
				sc->ExpectString();
				GCon->Logf("Unknown ambient type (%s)", *sc->String);
			}

			sc->ExpectFloat();
			ambient->Volume = sc->Float;
			if (ambient->Volume > 1)
				ambient->Volume = 1;
			else if (ambient->Volume < 0)
				ambient->Volume = 0;
		}
		else
		{
			sc->ExpectString();
			if (**sc->String == '$')
			{
				sc->Error("Unknown command");
			}
			VName TagName = *sc->String;
			sc->ExpectName8();
			AddSound(TagName, W_CheckNumForName(sc->Name8));
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	VSoundManager::AddSoundLump
//
//==========================================================================

int VSoundManager::AddSoundLump(VName TagName, int Lump)
{
	guard(VSoundManager::AddSoundLump);
	sfxinfo_t S;
	memset(&S, 0, sizeof(S));
	S.TagName = TagName;
	S.Data = NULL;
	S.Priority = 127;
	S.NumChannels = 2;
	S.ChangePitch = CurrentChangePitch;
	S.LumpNum = Lump;
	S.Link = -1;
	return S_sfx.Append(S);
	unguard;
}

//==========================================================================
//
//	VSoundManager::AddSound
//
//==========================================================================

int VSoundManager::AddSound(VName TagName, int Lump)
{
	guard(VSoundManager::AddSound);
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
	unguard;
}

//==========================================================================
//
//	VSoundManager::FindSound
//
//==========================================================================

int VSoundManager::FindSound(VName TagName)
{
	guard(VSoundManager::FindSound);
	for (int i = 0; i < S_sfx.Num(); i++)
	{
		if (S_sfx[i].TagName == TagName)
		{
			return i;
		}
	}
	return 0;
	unguard;
}

//==========================================================================
//
//	VSoundManager::FindOrAddSound
//
//==========================================================================

int VSoundManager::FindOrAddSound(VName TagName)
{
	guard(VSoundManager::FindOrAddSound);
	int id = FindSound(TagName);
	return id ? id : AddSoundLump(TagName, -1);
	unguard;
}

//==========================================================================
//
//	VSoundManager::ParsePlayerSoundCommon
//
//	Parses the common part of playersound commands in SNDINFO
// (player class, gender, and ref id)
//
//==========================================================================

void VSoundManager::ParsePlayerSoundCommon(VScriptParser* sc, int& PClass,
	int& Gender, int& RefId)
{
	guard(VSoundManager::ParsePlayerSoundCommon);
	sc->ExpectString();
	PClass = AddPlayerClass(*sc->String);
	sc->ExpectString();
	Gender = AddPlayerGender(*sc->String);
	sc->ExpectString();
	RefId = FindSound(*sc->String);
	if (!S_sfx[RefId].bPlayerReserve)
	{
		sc->Error(va("%s has not been reserved for a player sound",
			*sc->String));
	}
	sc->ExpectString();
	unguard;
}

//==========================================================================
//
//	VSoundManager::AddPlayerClass
//
//==========================================================================

int VSoundManager::AddPlayerClass(VName CName)
{
	guard(VSoundManager::AddPlayerClass);
	int idx = FindPlayerClass(CName);
	return idx == -1 ? PlayerClasses.Append(CName) : idx;
	unguard;
}

//==========================================================================
//
//	VSoundManager::FindPlayerClass
//
//==========================================================================

int VSoundManager::FindPlayerClass(VName CName)
{
	guard(VSoundManager::FindPlayerClass);
	for (int i = 0; i < PlayerClasses.Num(); i++)
		if (PlayerClasses[i] == CName)
			return i;
	return -1;
	unguard;
}

//==========================================================================
//
//	VSoundManager::AddPlayerGender
//
//==========================================================================

int VSoundManager::AddPlayerGender(VName GName)
{
	guard(VSoundManager::AddPlayerGender);
	int idx = FindPlayerGender(GName);
	return idx == -1 ? PlayerGenders.Append(GName) : idx;
	unguard;
}

//==========================================================================
//
//	VSoundManager::FindPlayerGender
//
//==========================================================================

int VSoundManager::FindPlayerGender(VName GName)
{
	guard(VSoundManager::FindPlayerGender);
	for (int i = 0; i < PlayerGenders.Num(); i++)
		if (PlayerGenders[i] == GName)
			return i;
	return -1;
	unguard;
}

//==========================================================================
//
//	VSoundManager::FindPlayerSound
//
//==========================================================================

int VSoundManager::FindPlayerSound(int PClass, int Gender, int RefId)
{
	guard(VSoundManager::FindPlayerSound);
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
	unguard;
}

//==========================================================================
//
//	VSoundManager::LookupPlayerSound
//
//==========================================================================

int VSoundManager::LookupPlayerSound(int ClassId, int GenderId, int RefId)
{
	guard(VSoundManager::LookupPlayerSound);
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
	unguard;
}

//==========================================================================
//
//	VSoundManager::GetSoundID
//
//==========================================================================

int VSoundManager::GetSoundID(VName Name)
{
	guard(VSoundManager::GetSoundID);
	for (int i = 0; i < S_sfx.Num(); i++)
	{
		if (S_sfx[i].TagName == Name)
		{
			return i;
		}
	}
	GCon->Logf("WARNING! Can't find sound %s", *Name);
	return 0;
	unguard;
}

//==========================================================================
//
//	VSoundManager::GetSoundID
//
//==========================================================================

int VSoundManager::GetSoundID(const char *name)
{
	guard(VSoundManager::GetSoundID);
	for (int i = 0; i < S_sfx.Num(); i++)
	{
		if (S_sfx[i].TagName == name)
		{
			return i;
		}
	}
	GCon->Logf("WARNING! Can't find sound named %s", name);
	return 0;
	unguard;
}

//==========================================================================
//
//	VSoundManager::ResolveSound
//
//==========================================================================

int VSoundManager::ResolveSound(int InSoundId)
{
	guard(VSoundManager::ResolveSound);
	return ResolveSound(0, 0, InSoundId);
	unguard;
}

//==========================================================================
//
//	VSoundManager::ResolveEntitySound
//
//==========================================================================

int VSoundManager::ResolveEntitySound(VName ClassName, VName GenderName,
	VName SoundName)
{
	guard(VSoundManager::ResolveEntitySound);
	int ClassId = FindPlayerClass(ClassName);
	if (ClassId == -1)
		ClassId = 0;
	int GenderId = FindPlayerGender(GenderName);
	if (GenderId == -1)
		GenderId = 0;
	int SoundId = GetSoundID(SoundName);
	return ResolveSound(ClassId, GenderId, SoundId);
	unguard;
}

//==========================================================================
//
//	VSoundManager::ResolveSound
//
//==========================================================================

int VSoundManager::ResolveSound(int ClassID, int GenderID, int InSoundId)
{
	guard(VSoundManager::ResolveSound);
	int sound_id = InSoundId;
	while (S_sfx[sound_id].Link != -1)
	{
		if (S_sfx[sound_id].bPlayerReserve)
		{
			sound_id = LookupPlayerSound(ClassID, GenderID, sound_id);
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
//	VSoundManager::LoadSound
//
//==========================================================================

bool VSoundManager::LoadSound(int sound_id)
{
	guard(VSoundManager::LoadSound);
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
//	VSoundManager::DoneWithLump
//
//==========================================================================

void VSoundManager::DoneWithLump(int sound_id)
{
	guard(VSoundManager::DoneWithLump);
	sfxinfo_t &sfx = S_sfx[sound_id];
	if (!sfx.Data || !sfx.UseCount)
	{
		Sys_Error("Empty lump");
	}

	sfx.UseCount--;
	if (sfx.UseCount)
	{
		//	still used
		return;
	}
	Z_Free(sfx.Data);
	sfx.Data = NULL;
	unguard;
}

//==========================================================================
//
//	VSoundManager::ParseSequenceScript
//
//==========================================================================

void VSoundManager::ParseSequenceScript(VScriptParser* sc)
{
	guard(VSoundManager::ParseSequenceScript);
	TArray<vint32>	TempData;
	bool			inSequence = false;
	int				SeqId = 0;

	while (!sc->AtEnd())
	{
		sc->ExpectString();
		if (**sc->String == ':')
		{
			if (inSequence)
			{
				sc->Error("SN_InitSequenceScript:  Nested Script Error");
			}
			for (SeqId = 0; SeqId < SeqInfo.Num(); SeqId++)
			{
				if (SeqInfo[SeqId].Name == *sc->String + 1)
				{
					Z_Free(SeqInfo[SeqId].Data);
					break;
				}
			}
			if (SeqId == SeqInfo.Num())
			{
				SeqInfo.Alloc();
			}
			TempData.Clear();
			inSequence = true;
			SeqInfo[SeqId].Name = *sc->String + 1;
			SeqInfo[SeqId].Data = NULL;
			SeqInfo[SeqId].StopSound = 0;
			continue; // parse the next command
		}
		if (!inSequence)
		{
			sc->Error("String outside sequence");
			continue;
		}
		sc->UnGet();
		if (sc->Check("playuntildone"))
		{
			sc->ExpectString();
			TempData.Append(SSCMD_Play);
			TempData.Append(GetSoundID(*sc->String));
			TempData.Append(SSCMD_WaitUntilDone);
		}
		else if (sc->Check("play"))
		{
			sc->ExpectString();
			TempData.Append(SSCMD_Play);
			TempData.Append(GetSoundID(*sc->String));
		}
		else if (sc->Check("playtime"))
		{
			sc->ExpectString();
			TempData.Append(SSCMD_Play);
			TempData.Append(GetSoundID(*sc->String));
			sc->ExpectNumber();
			TempData.Append(SSCMD_Delay);
			TempData.Append(sc->Number);
		}
		else if (sc->Check("playrepeat"))
		{
			sc->ExpectString();
			TempData.Append(SSCMD_PlayRepeat);
			TempData.Append(GetSoundID(*sc->String));
		}
		else if (sc->Check("playloop"))
		{
			sc->ExpectString();
			TempData.Append(SSCMD_PlayLoop);
			TempData.Append(GetSoundID(*sc->String));
			sc->ExpectNumber();
			TempData.Append(sc->Number);
		}
		else if (sc->Check("delay"))
		{
			TempData.Append(SSCMD_Delay);
			sc->ExpectNumber();
			TempData.Append(sc->Number);
		}
		else if (sc->Check("delayrand"))
		{
			TempData.Append(SSCMD_DelayRand);
			sc->ExpectNumber();
			TempData.Append(sc->Number);
			sc->ExpectNumber();
			TempData.Append(sc->Number);
		}
		else if (sc->Check("volume"))
		{
			TempData.Append(SSCMD_Volume);
			sc->ExpectNumber();
			TempData.Append(sc->Number);
		}
		else if (sc->Check("attenuation"))
		{
			TempData.Append(SSCMD_Attenuation);
			sc->ExpectString();
			vint32 Atten = 0;
			if (sc->Check("none"))
				Atten = 0;
			else if (sc->Check("normal"))
				Atten = 1;
			else if (sc->Check("idle"))
				Atten = 2;
			else if (sc->Check("static"))
				Atten = 3;
			else if (sc->Check("surround"))
				Atten = 4;
			else
				sc->Error("Bad attenuation");
			TempData.Append(Atten);
		}
		else if (sc->Check("stopsound"))
		{
			sc->ExpectString();
			SeqInfo[SeqId].StopSound = GetSoundID(*sc->String);
			TempData.Append(SSCMD_StopSound);
		}
		else if (sc->Check("nostopcutoff"))
		{
			SeqInfo[SeqId].StopSound = -1;
			TempData.Append(SSCMD_StopSound);
		}
		else if (sc->Check("door"))
		{
			AssignSeqTranslations(sc, SeqId, SEQ_Door);
		}
		else if (sc->Check("platform"))
		{
			AssignSeqTranslations(sc, SeqId, SEQ_Platform);
		}
		else if (sc->Check("environment"))
		{
			AssignSeqTranslations(sc, SeqId, SEQ_Environment);
		}
		else if (sc->Check("end"))
		{
			TempData.Append(SSCMD_End);
			SeqInfo[SeqId].Data = new vint32[TempData.Num()];
			memcpy(SeqInfo[SeqId].Data, TempData.Ptr(), TempData.Num() *
				sizeof(vint32));
			inSequence = false;
		}
		else
		{
			sc->Error("Unknown commmand.");
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	VSoundManager::AssignSeqTranslations
//
//==========================================================================

void VSoundManager::AssignSeqTranslations(VScriptParser* sc, int SeqId,
	seqtype_t SeqType)
{
	guard(VSoundManager::AssignSeqTranslations);
	sc->Crossed = false;

	while (sc->GetString() && !sc->Crossed)
	{
		char* Stopper;
		int Num = strtol(*sc->String, &Stopper, 0);
		if (*Stopper == 0)
		{
			SeqTrans[(Num & 63) + SeqType * 64] = SeqId;
		}
	}

	sc->UnGet();
	unguard;
}

//==========================================================================
//
//  VSoundManager::SetSeqTrans
//
//==========================================================================

void VSoundManager::SetSeqTrans(VName Name, int Num, int SeqType)
{
	guard(VSoundManager::SetSeqTrans);
	for (int i = 0; i < SeqInfo.Num(); i++)
	{
		if (SeqInfo[i].Name == Name)
		{
			SeqTrans[(Num & 63) + SeqType * 64] = i;
			return;
		}
	}
	unguard;
}

//==========================================================================
//
//  VSoundManager::GetSeqTrans
//
//==========================================================================

VName VSoundManager::GetSeqTrans(int Num, int SeqType)
{
	guard(VSoundManager::GetSeqTrans);
	if (Num < 0)
	{
		//	If not assigned, use 0 as default.
		Num = 0;
	}
	if (SeqTrans[(Num & 63) + SeqType * 64] < 0)
	{
		return NAME_None;
	}
	return SeqInfo[SeqTrans[(Num & 63) + SeqType * 64]].Name;
	unguard;
}

#ifdef CLIENT

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
	Sfx.Data = Z_Malloc(Sfx.DataSize);
	Strm.Serialise(Sfx.Data, Sfx.DataSize);
	unguard;
}

#endif
