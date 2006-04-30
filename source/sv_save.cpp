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
//**	Copyright (C) 1999-2002 J�nis Legzdi��
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
//**	Archiving: SaveGame I/O.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define MAX_TARGET_PLAYERS 512
#define MAX_MAPS	99
#define BASE_SLOT	8
#define REBORN_SLOT	9

#define SAVEGAMENAME			"vavm"
#define EMPTYSTRING				"empty slot"
#define MOBJ_NULL 				-1
#define SAVE_NAME(_slot) \
	(VStr("saves/savegame.vs") + _slot)
#define SAVE_MAP_NAME(_slot, _map) \
	(VStr("saves/") + _map + ".vs" + _slot)
#define SAVE_NAME_ABS(_slot) \
	(SV_GetSavesDir() + "/savegame.vs" + _slot)
#define SAVE_MAP_NAME_ABS(_slot, _map) \
	(SV_GetSavesDir() + "/" + _map + ".vs" + _slot)

#define SAVE_DESCRIPTION_LENGTH		24
#define SAVE_VERSION_TEXT			"Version 1.19"
#define SAVE_VERSION_TEXT_LENGTH	16

// TYPES -------------------------------------------------------------------

enum gameArchiveSegment_t
{
	ASEG_GAME_HEADER = 101,
	ASEG_MAP_HEADER,
	ASEG_BASELINE,
	ASEG_WORLD,
	ASEG_SCRIPTS,
	ASEG_SOUNDS,
	ASEG_END
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_SpawnServer(char *mapname, boolean spawn_thinkers);
void SV_SendServerInfoToClients();
void SV_ShutdownServer(boolean);
void CL_Disconnect();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern VEntity		**sv_mobjs;
extern mobj_base_t	*sv_mo_base;
extern bool			sv_loading;
extern int			sv_load_num_players;
extern VMessage		sv_signon;

extern boolean		in_secret;
extern char			mapaftersecret[12];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VStr			SavesDir;
static boolean 		SavingPlayers;

// CODE --------------------------------------------------------------------

class VSaveLoaderStream : public VStream
{
private:
	VStream*			Stream;

public:
	VName*				NameRemap;
	TArray<VObject*>	Exports;

	VSaveLoaderStream(VStream* InStream)
	: Stream(InStream)
	, NameRemap(0)
	{
		bLoading = true;
	}
	~VSaveLoaderStream()
	{
		Z_Free(NameRemap);
		delete Stream;
	}

	//	Stream interface.
	void Serialise(void* Data, int Len)
	{
		Stream->Serialise(Data, Len);
	}
	void Seek(int Pos)
	{
		Stream->Seek(Pos);
	}
	int Tell()
	{
		return Stream->Tell();
	}
	int TotalSize()
	{
		return Stream->TotalSize();
	}
	bool AtEnd()
	{
		return Stream->AtEnd();
	}
	void Flush()
	{
		Stream->Flush();
	}
	bool Close()
	{
		return Stream->Close();
	}

	VStream& operator<<(VName& Name)
	{
		int NameIndex;
		*this << STRM_INDEX(NameIndex);
		Name = NameRemap[NameIndex];
		return *this;
	}
	void SerialiseReference(VObject*& Ref, VClass*)
	{
		guard(Loader::SerialiseReference);
		int TmpIdx;
		*this << STRM_INDEX(TmpIdx);
		if (TmpIdx == 0)
		{
			Ref = NULL;
		}
		else if (TmpIdx > 0)
		{
			if (TmpIdx > Exports.Num())
				Sys_Error("Bad index %d", TmpIdx);
			Ref = Exports[TmpIdx - 1];
		}
		else
		{
			Ref = GPlayersBase[-TmpIdx - 1];
		}
		unguard;
	}
	void SerialiseStructPointer(void*& Ptr, VStruct* Struct)
	{
		int TmpIdx;
		*this << STRM_INDEX(TmpIdx);
		if (Struct->Name == "sector_t")
		{
			Ptr = TmpIdx >= 0 ? &GLevel->Sectors[TmpIdx] : NULL;
		}
		else if (Struct->Name == "line_t")
		{
			Ptr = TmpIdx >= 0 ? &GLevel->Lines[TmpIdx] : NULL;
		}
		else
		{
			dprintf("Don't know how to handle pointer to %s\n", *Struct->Name);
			Ptr = (void*)TmpIdx;
		}
	}
};

class VSaveWriterStream : public VStream
{
private:
	VStream*			Stream;

public:
	TArray<VName>		Names;
	TArray<VObject*>	Exports;
	vint32*				NamesMap;
	vint32*				ObjectsMap;

	VSaveWriterStream(VStream* InStream)
	: Stream(InStream)
	{
		bLoading = false;
		NamesMap = (vint32*)Z_Malloc(sizeof(vint32) * VName::GetNumNames(), PU_STATIC, 0);
		for (int i = 0; i < VName::GetNumNames(); i++)
		{
			NamesMap[i] = -1;
		}
	}
	~VSaveWriterStream()
	{
		delete Stream;
	}

	//	Stream interface.
	void Serialise(void* Data, int Len)
	{
		Stream->Serialise(Data, Len);
	}
	void Seek(int Pos)
	{
		Stream->Seek(Pos);
	}
	int Tell()
	{
		return Stream->Tell();
	}
	int TotalSize()
	{
		return Stream->TotalSize();
	}
	bool AtEnd()
	{
		return Stream->AtEnd();
	}
	void Flush()
	{
		Stream->Flush();
	}
	bool Close()
	{
		return Stream->Close();
	}

	VStream& operator<<(VName& Name)
	{
		if (NamesMap[Name.GetIndex()] == -1)
		{
			NamesMap[Name.GetIndex()] = Names.AddItem(Name);
		}
		*this << STRM_INDEX(NamesMap[Name.GetIndex()]);
		return *this;
	}
	void SerialiseReference(VObject*& Ref, VClass*)
	{
		guard(Saver::SerialiseReference);
		int TmpIdx;
		if (!Ref)
		{
			TmpIdx = 0;
		}
		else
		{
			TmpIdx = ObjectsMap[Ref->GetIndex()];
		}
		*this << STRM_INDEX(TmpIdx);
		unguard;
	}
	void SerialiseStructPointer(void*& Ptr, VStruct* Struct)
	{
		int TmpIdx;
		if (Struct->Name == "sector_t")
		{
			if (Ptr)
				TmpIdx = (sector_t*)Ptr - GLevel->Sectors;
			else
    			TmpIdx = -1;
		}
		else if (Struct->Name == "line_t")
		{
			if (Ptr)
				TmpIdx = (line_t*)Ptr - GLevel->Lines;
			else
    			TmpIdx = -1;
		}
		else
		{
			dprintf("Don't know how to handle pointer to %s\n", *Struct->Name);
			TmpIdx = (int)Ptr;
		}
		*this << STRM_INDEX(TmpIdx);
	}
};

static VSaveWriterStream*	Saver;
static VSaveLoaderStream*	Loader;

//==========================================================================
//
//	SV_GetSavesDir
//
//==========================================================================

static VStr SV_GetSavesDir()
{
	if (!SavesDir)
	{
		if (fl_savedir)
			SavesDir = fl_savedir + "/" + fl_gamedir + "/saves";
		else
			SavesDir = fl_basedir + "/" + fl_gamedir + "/saves";
	}
	return SavesDir;
}

//==========================================================================
//
//	SV_GetSaveString
//
//==========================================================================

boolean	SV_GetSaveString(int slot, char* buf)
{
	guard(SV_GetSaveString);
	FILE*		f;

	f = fopen(*SAVE_NAME_ABS(slot), "rb");
	if (f)
	{
		fseek(f, 4, SEEK_SET);
		fread(buf, 1, SAVE_DESCRIPTION_LENGTH, f);
		fclose(f);
		return true;
	}
	else
	{
		strcpy(buf, EMPTYSTRING);
		return false;
	}
	unguard;
}

//==========================================================================
//
//	ClearSaveSlot
//
//	Deletes all save game files associated with a slot number.
//
//==========================================================================

static void ClearSaveSlot(int slot)
{
	guard(ClearSaveSlot);
	char slotExt[4];
	VStr curName;

	sprintf(slotExt, "vs%d", slot);
	if (!Sys_OpenDir(SV_GetSavesDir()))
	{
		//  Directory doesn't exist ... yet
		return;
	}
	while ((curName = Sys_ReadDir()))
	{
		VStr ext = curName.ExtractFileExtension();
		if (ext == slotExt)
		{
			remove(*(SV_GetSavesDir() + "/" + curName));
		}
	}
	Sys_CloseDir();
	unguard;
}

//==========================================================================
//
// CopySaveSlot
//
// Copies all the save game files from one slot to another.
//
//==========================================================================

static void CopySaveSlot(int sourceSlot, int destSlot)
{
	guard(CopySaveSlot);
	VStr curName;

	VStr srcExt = VStr("vs") + sourceSlot;
	VStr dstExt = VStr("vs") + destSlot;
	if (!Sys_OpenDir(SV_GetSavesDir()))
	{
		//  Directory doesn't exist ... yet
		return;
	}
	while ((curName = Sys_ReadDir()))
	{
		VStr ext = VStr(curName).ExtractFileExtension();
		if (ext == srcExt)
		{
			VStr sourceName = SV_GetSavesDir() + "/" + curName;
			VStr destName = sourceName.StripExtension() + "." + dstExt;

			int length;
			byte *buffer;

			length = M_ReadFile(*sourceName, &buffer);
			M_WriteFile(*destName, buffer, length);
			Z_Free(buffer);
		}
	}
	Sys_CloseDir();
	unguard;
}

//==========================================================================
//
//	AssertSegment
//
//==========================================================================

static void AssertSegment(gameArchiveSegment_t segType)
{
	guard(AssertSegment);
	if (Streamer<int>(*Loader) != (int)segType)
	{
		Host_Error("Corrupt save game: Segment [%d] failed alignment check",
			segType);
	}
	unguard;
}

//==========================================================================
//
//	ArchiveNames
//
//==========================================================================

static void ArchiveNames(VStream &Strm)
{
	//	Write offset to the names in the beginning of the file.
	vint32 NamesOffset = Strm.Tell();
	Strm.Seek(0);
	Strm << NamesOffset;
	Strm.Seek(NamesOffset);

	//	Serialise names.
	vint32 Count = Saver->Names.Num();
	Strm << STRM_INDEX(Count);
	for (int i = 0; i < Count; i++)
	{
		Strm << *VName::GetEntry(Saver->Names[i].GetIndex());
	}
}

//==========================================================================
//
//	UnarchiveNames
//
//==========================================================================

static void UnarchiveNames(VStream &Strm)
{
	vint32 NamesOffset;
	*Loader << NamesOffset;

	vint32 TmpOffset = Strm.Tell();
	Strm.Seek(NamesOffset);
	vint32 Count;
	Strm << STRM_INDEX(Count);
	Loader->NameRemap = (VName*)Z_Malloc(Count * 4, PU_STATIC, 0);
	for (int i = 0; i < Count; i++)
	{
		VNameEntry E;
		Strm << E;
		Loader->NameRemap[i] = VName(E.Name);
	}
	Strm.Seek(TmpOffset);
}

//==========================================================================
//
// ArchiveThinkers
//
//==========================================================================

static void ArchiveThinkers()
{
	guard(ArchiveThinkers);
	vint32 Seg = ASEG_WORLD;
	*Saver << Seg;

	Saver->ObjectsMap = (int*)Z_Calloc(VObject::GetObjectsCount() * 4, PU_STATIC, 0);

	//	Add level
	Saver->Exports.AddItem(GLevel);
	Saver->ObjectsMap[GLevel->GetIndex()] = Saver->Exports.Num();

	//	Add players.
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		byte Active = (byte)(SavingPlayers && GGameInfo->Players[i]);
		*Saver << Active;
		if (!Active)
		{
			continue;
		}

		Saver->Exports.AddItem(GGameInfo->Players[i]);
		Saver->ObjectsMap[GGameInfo->Players[i]->GetIndex()] = Saver->Exports.Num();
	}

	//	Add thinkers.
	int ThinkersStart = Saver->Exports.Num();
	for (VThinker* Th = GLevel->ThinkerHead; Th; Th = Th->Next)
	{
		VEntity *mobj = Cast<VEntity>(Th);
		if (mobj && mobj->EntityFlags & VEntity::EF_IsPlayer && !SavingPlayers)
		{
			// Skipping player mobjs
			continue;
		}

		Saver->Exports.AddItem(Th);
		Saver->ObjectsMap[Th->GetIndex()] = Saver->Exports.Num();
	}

	//	Add player weapon objects.
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!SavingPlayers || !GGameInfo->Players[i])
		{
			continue;
		}

		for (int pi = 0; pi < NUMPSPRITES; pi++)
		{
			if (GGameInfo->Players[i]->ViewEnts[pi])
			{
				Saver->Exports.AddItem(GGameInfo->Players[i]->ViewEnts[pi]);
				Saver->ObjectsMap[GGameInfo->Players[i]->ViewEnts[pi]->GetIndex()] = Saver->Exports.Num();
			}
		}
	}

	vint32 NumObjects = Saver->Exports.Num() - ThinkersStart;
	*Saver << STRM_INDEX(NumObjects);
	for (int i = ThinkersStart; i < Saver->Exports.Num(); i++)
	{
		VName CName = Saver->Exports[i]->GetClass()->GetVName();
		*Saver << CName;
	}

	//	Serialise objects.
	for (int i = 0; i < Saver->Exports.Num(); i++)
	{
		Saver->Exports[i]->Serialise(*Saver);
	}
	unguard;
}

//==========================================================================
//
//  UnarchiveThinkers
//
//==========================================================================

static void UnarchiveThinkers()
{
	guard(UnarchiveThinkers);
	VObject*			Obj;

	AssertSegment(ASEG_WORLD);

	//	Add level.
	Loader->Exports.AddItem(GLevel);

	//	Add players.
	sv_load_num_players = 0;
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		byte Active;
		*Loader << Active;
		if (Active)
		{
			sv_load_num_players++;
			Loader->Exports.AddItem(GPlayersBase[i]);
		}
	}

	vint32 NumObjects;
	*Loader << STRM_INDEX(NumObjects);
	for (int i = 0; i < NumObjects; i++)
	{
		//  Get params
		VName CName;
		*Loader << CName;
		VClass *Class = VClass::FindClass(*CName);
		if (!Class)
		{
			Sys_Error("No such class %s", *CName);
		}
	
		//  Allocate object and copy data
		Obj = VObject::StaticSpawnObject(Class, PU_LEVSPEC);

		//  Handle level info
		if (Obj->IsA(VLevelInfo::StaticClass()))
		{
			GLevelInfo = (VLevelInfo*)Obj;
		}

		Loader->Exports.AddItem(Obj);
	}

	GLevelInfo->Game = GGameInfo;

	for (int i = 0; i < Loader->Exports.Num(); i++)
	{
		Loader->Exports[i]->Serialise(*Loader);
	}

	GLevelInfo->eventAfterUnarchiveThinkers();
	unguard;
}

//==========================================================================
//
// ArchiveScripts
//
//==========================================================================

static void ArchiveScripts()
{
	vint32 Seg = ASEG_SCRIPTS;
	*Saver << Seg;
	P_SerialiseScripts(*Saver);
}

//==========================================================================
//
// UnarchiveScripts
//
//==========================================================================

static void UnarchiveScripts()
{
	AssertSegment(ASEG_SCRIPTS);
	P_SerialiseScripts(*Loader);
}

//==========================================================================
//
// ArchiveSounds
//
//==========================================================================

static void ArchiveSounds()
{
	vint32 Seg = ASEG_SOUNDS;
	*Saver << Seg;
	SN_SerialiseSounds(*Saver);
}

//==========================================================================
//
// UnarchiveSounds
//
//==========================================================================

static void UnarchiveSounds()
{
	AssertSegment(ASEG_SOUNDS);
	SN_SerialiseSounds(*Loader);
}

//==========================================================================
//
// SV_SaveMap
//
//==========================================================================

static void SV_SaveMap(int slot, boolean savePlayers)
{
	guard(SV_SaveMap);
	// Make sure we don't have any garbage
	VObject::CollectGarbage();

	SavingPlayers = savePlayers;

	// Open the output file
	Saver = new VSaveWriterStream(FL_OpenFileWrite(*SAVE_MAP_NAME(slot,
		(const char*)level.mapname)));

	int NamesOffset = 0;
	*Saver << NamesOffset;

	// Place a header marker
	vint32 Seg = ASEG_MAP_HEADER;
	*Saver << Seg;

	// Write the level timer
	*Saver << level.time
		<< level.tictime;

	//	Write totals, because when thinkers are not spawned, they are not
	// counted
	*Saver << level.totalkills
		<< level.totalitems
		<< level.totalsecret
		<< level.currentkills
		<< level.currentitems
		<< level.currentsecret;

	*Saver << level.sky1Texture
		<< level.sky2Texture
		<< level.sky1ScrollDelta
		<< level.sky2ScrollDelta
		<< level.doubleSky
		<< level.lightning;
	Saver->Serialise(level.skybox, sizeof(level.skybox));

	Saver->Serialise(level.songLump, sizeof(level.songLump));
	*Saver << level.cdTrack;

	//	Save baseline
	Seg = ASEG_BASELINE;
	*Saver << Seg;
	*Saver << STRM_INDEX(sv_signon.CurSize);
	Saver->Serialise(sv_signon.Data, sv_signon.CurSize);
	for (int i = 0; i < GMaxEntities; i++)
	{
		if (sv_mo_base[i].Origin.x ||
			sv_mo_base[i].Origin.y ||
			sv_mo_base[i].Origin.z ||
			sv_mo_base[i].Angles.pitch ||
			sv_mo_base[i].Angles.yaw ||
			sv_mo_base[i].Angles.roll ||
			sv_mo_base[i].SpriteType ||
			sv_mo_base[i].SpriteIndex ||
			sv_mo_base[i].SpriteFrame ||
			sv_mo_base[i].ModelIndex ||
			sv_mo_base[i].ModelFrame ||
			sv_mo_base[i].Translucency ||
			sv_mo_base[i].Translation ||
			sv_mo_base[i].Effects)
		{
			*Saver << STRM_INDEX(i)
				<< sv_mo_base[i].Origin
				<< sv_mo_base[i].Angles
				<< STRM_INDEX(sv_mo_base[i].SpriteType)
				<< STRM_INDEX(sv_mo_base[i].SpriteIndex)
				<< STRM_INDEX(sv_mo_base[i].SpriteFrame)
				<< STRM_INDEX(sv_mo_base[i].ModelIndex)
				<< STRM_INDEX(sv_mo_base[i].ModelFrame)
				<< STRM_INDEX(sv_mo_base[i].Translucency)
				<< STRM_INDEX(sv_mo_base[i].Translation)
				<< STRM_INDEX(sv_mo_base[i].Effects);
		}
	}
	int Term = -1;
	*Saver << STRM_INDEX(Term);

	ArchiveThinkers();
	ArchiveScripts();
	ArchiveSounds();

	// Place a termination marker
	Seg = ASEG_END;
	*Saver << Seg;

	ArchiveNames(*Saver);

	// Close the output file
	Saver->Close();
	delete Saver;
	unguard;
}

//==========================================================================
//
//	SV_LoadMap
//
//==========================================================================

static void SV_LoadMap(const char *mapname, int slot)
{
	guard(SV_LoadMap);
	// Load a base level
	SV_SpawnServer(const_cast<char*>(mapname), false);

	// Load the file
	Loader = new VSaveLoaderStream(FL_OpenFileRead(SAVE_MAP_NAME(slot, mapname)));

	// Load names
	UnarchiveNames(*Loader);

	AssertSegment(ASEG_MAP_HEADER);

	// Read the level timer
	*Loader << level.time
		<< level.tictime;

	*Loader << level.totalkills
		<< level.totalitems
		<< level.totalsecret
		<< level.currentkills
		<< level.currentitems
		<< level.currentsecret;

	*Loader << level.sky1Texture
		<< level.sky2Texture
		<< level.sky1ScrollDelta
		<< level.sky2ScrollDelta
		<< level.doubleSky
		<< level.lightning;
	Loader->Serialise(level.skybox, sizeof(level.skybox));

	Loader->Serialise(level.songLump, sizeof(level.songLump));
	*Loader << level.cdTrack;

	AssertSegment(ASEG_BASELINE);
	int len;
	*Loader << STRM_INDEX(len);
	sv_signon.Clear();
	void *tmp = Z_Malloc(len, PU_STATIC, 0);
	Loader->Serialise(tmp, len);
	sv_signon.Write(tmp, len);
	memset(sv_mo_base, 0, sizeof(mobj_base_t) * GMaxEntities);
	int Idx;
	*Loader << STRM_INDEX(Idx);
	while (Idx != -1)
	{
		*Loader << sv_mo_base[Idx].Origin
			<< sv_mo_base[Idx].Angles
			<< STRM_INDEX(sv_mo_base[Idx].SpriteType)
			<< STRM_INDEX(sv_mo_base[Idx].SpriteIndex)
			<< STRM_INDEX(sv_mo_base[Idx].SpriteFrame)
			<< STRM_INDEX(sv_mo_base[Idx].ModelIndex)
			<< STRM_INDEX(sv_mo_base[Idx].ModelFrame)
			<< STRM_INDEX(sv_mo_base[Idx].Translucency)
			<< STRM_INDEX(sv_mo_base[Idx].Translation)
			<< STRM_INDEX(sv_mo_base[Idx].Effects)
			<< STRM_INDEX(Idx);
	}

	UnarchiveThinkers();
	UnarchiveScripts();
	UnarchiveSounds();

	AssertSegment(ASEG_END);

	// Free save buffer
	Loader->Close();
	delete Loader;

	//	Do this here so that clients have loaded info, not initial one.
	SV_SendServerInfoToClients();
	unguard;
}

//==========================================================================
//
//	SV_SaveGame
//
//==========================================================================

void SV_SaveGame(int slot, const char* description)
{
	guard(SV_SaveGame);
	char versionText[SAVE_VERSION_TEXT_LENGTH];
	int i;

	// Open the output file
	Saver = new VSaveWriterStream(FL_OpenFileWrite(*SAVE_NAME(BASE_SLOT)));

	int NamesOffset = 0;
	*Saver << NamesOffset;

	// Write game save description
	char desc[SAVE_DESCRIPTION_LENGTH];
	memset(desc, 0, sizeof(desc));
	strncpy(desc, description, SAVE_DESCRIPTION_LENGTH - 1);
	Saver->Serialise(desc, SAVE_DESCRIPTION_LENGTH);

	// Write version info
	memset(versionText, 0, SAVE_VERSION_TEXT_LENGTH);
	strcpy(versionText, SAVE_VERSION_TEXT);
	Saver->Serialise(versionText, SAVE_VERSION_TEXT_LENGTH);

	// Place a header marker
	vint32 Seg = ASEG_GAME_HEADER;
	*Saver << Seg;

	// Write current map and difficulty
	byte Skill = (byte)gameskill;
	*Saver << Skill;
	Saver->Serialise(level.mapname, 8);

	// Write secret level info
	byte InSec = in_secret;
	*Saver << InSec;
	Saver->Serialise(mapaftersecret, 8);

	// Write global script info
	for (i = 0; i < MAX_ACS_WORLD_VARS; i++)
	{
		*Saver << STRM_INDEX(WorldVars[i]);
	}
	for (i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
	{
		*Saver << STRM_INDEX(GlobalVars[i]);
	}
	for (i = 0; i < MAX_ACS_WORLD_VARS; i++)
	{
		WorldArrays[i].Serialise(*Saver);
	}
	for (i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
	{
		GlobalArrays[i].Serialise(*Saver);
	}
	vint32 NumAcsStore = 0;
	for (acsstore_t* store = ACSStore; store->map[0] != 0; store++)
		if (store->map[0] != '-')
			NumAcsStore++;
	*Saver << STRM_INDEX(NumAcsStore);
	for (acsstore_t* store = ACSStore; store->map[0] != 0; store++)
		if (store->map[0] != '-')
		{
			Saver->Serialise(ACSStore[i].map, 9);
			*Saver << STRM_INDEX(ACSStore[i].script)
				<< STRM_INDEX(ACSStore[i].args[0])
				<< STRM_INDEX(ACSStore[i].args[1])
				<< STRM_INDEX(ACSStore[i].args[2]);
		}

	// Place a termination marker
	Seg = ASEG_END;
	*Saver << Seg;

	// Write names
	ArchiveNames(*Saver);

	// Close the output file
	Saver->Close();
	delete Saver;

	// Save out the current map
	SV_SaveMap(BASE_SLOT, true); // true = save player info

	if (slot != BASE_SLOT)
	{
		// Clear all save files at destination slot
		ClearSaveSlot(slot);

		// Copy base slot to destination slot
		CopySaveSlot(BASE_SLOT, slot);
	}
	unguard;
}

//==========================================================================
//
//	SV_LoadGame
//
//==========================================================================

void SV_LoadGame(int slot)
{
	guard(SV_LoadGame);
	char		mapname[12];
	int			i;

	SV_ShutdownServer(false);
#ifdef CLIENT
	CL_Disconnect();
#endif

	// Copy all needed save files to the base slot
	if (slot != BASE_SLOT)
	{
		ClearSaveSlot(BASE_SLOT);
		CopySaveSlot(slot, BASE_SLOT);
	}

	// Load the file
	Loader = new VSaveLoaderStream(FL_OpenFileRead(SAVE_NAME(BASE_SLOT)));

	// Load names
	UnarchiveNames(*Loader);

	// Set the save pointer and skip the description field
	char desc[SAVE_DESCRIPTION_LENGTH];
	Loader->Serialise(desc, SAVE_DESCRIPTION_LENGTH);

	// Check the version text
	char versionText[SAVE_VERSION_TEXT_LENGTH];
	Loader->Serialise(versionText, SAVE_VERSION_TEXT_LENGTH);
	if (strcmp(versionText, SAVE_VERSION_TEXT))
	{
		// Bad version
		Loader->Close();
		delete Loader;
		GCon->Log("Savegame is from incompatible version");
		return;
	}

	AssertSegment(ASEG_GAME_HEADER);

	gameskill = (skill_t)Streamer<byte>(*Loader);
	Loader->Serialise(mapname, 8);
	mapname[8] = 0;

	//	Init skill hacks
	GGameInfo->eventInitNewGame(gameskill);

	// Read secret level info
	in_secret = Streamer<byte>(*Loader);
	Loader->Serialise(mapaftersecret, 8);
	mapaftersecret[8] = 0;

	// Read global script info
	for (i = 0; i < MAX_ACS_WORLD_VARS; i++)
	{
		*Loader << STRM_INDEX(WorldVars[i]);
	}
	for (i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
	{
		*Loader << STRM_INDEX(GlobalVars[i]);
	}
	for (i = 0; i < MAX_ACS_WORLD_VARS; i++)
	{
		WorldArrays[i].Serialise(*Loader);
	}
	for (i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
	{
		GlobalArrays[i].Serialise(*Loader);
	}
	memset(ACSStore, 0, sizeof(ACSStore));
	vint32 NumAcsStore = 0;
	*Loader << STRM_INDEX(NumAcsStore);
	for (i = 0; i < NumAcsStore; i++)
	{
		Loader->Serialise(ACSStore[i].map, 9);
		*Loader << STRM_INDEX(ACSStore[i].script)
			<< STRM_INDEX(ACSStore[i].args[0])
			<< STRM_INDEX(ACSStore[i].args[1])
			<< STRM_INDEX(ACSStore[i].args[2]);
	}

	AssertSegment(ASEG_END);

	Loader->Close();
	delete Loader;

	sv_loading = true;

	// Load the current map
	SV_LoadMap(mapname, BASE_SLOT);

#ifdef CLIENT
	if (cls.state != ca_dedicated)
		GCmdBuf << "Connect local\n";
#endif
	unguard;
}

//==========================================================================
//
//	SV_InitBaseSlot
//
//==========================================================================

void SV_InitBaseSlot()
{
	ClearSaveSlot(BASE_SLOT);
}

//==========================================================================
//
// SV_GetRebornSlot
//
//==========================================================================

int SV_GetRebornSlot()
{
	return REBORN_SLOT;
}

//==========================================================================
//
// SV_RebornSlotAvailable
//
// Returns true if the reborn slot is available.
//
//==========================================================================

boolean SV_RebornSlotAvailable()
{
	return Sys_FileExists(SAVE_NAME_ABS(REBORN_SLOT));
}

//==========================================================================
//
// SV_UpdateRebornSlot
//
// Copies the base slot to the reborn slot.
//
//==========================================================================

void SV_UpdateRebornSlot()
{
	ClearSaveSlot(REBORN_SLOT);
	CopySaveSlot(BASE_SLOT, REBORN_SLOT);
}

//==========================================================================
//
// SV_ClearRebornSlot
//
//==========================================================================

void SV_ClearRebornSlot()
{
	ClearSaveSlot(REBORN_SLOT);
}

//==========================================================================
//
// SV_MapTeleport
//
//==========================================================================

void SV_MapTeleport(char *map)
{
	guard(SV_MapTeleport);
	char		mapname[12];

	//	Make a copy because SV_SpawnServer can modify it
	strcpy(mapname, map);
	if (!deathmatch)
	{
		mapInfo_t	old_info;
		mapInfo_t	new_info;

		P_GetMapInfo(level.mapname, old_info);
		P_GetMapInfo(mapname, new_info);
		//	All maps in cluster 0 are treated as in fifferent clusters
		if (old_info.cluster && old_info.cluster == new_info.cluster)
		{
			// Same cluster - save map without saving player mobjs
			SV_SaveMap(BASE_SLOT, false);
		}
		else
		{
			// Entering new cluster - clear base slot
			ClearSaveSlot(BASE_SLOT);
		}
	}

	if (!deathmatch && Sys_FileExists(SAVE_MAP_NAME_ABS(BASE_SLOT, (const char*)mapname)))
	{
		// Unarchive map
		SV_LoadMap(mapname, BASE_SLOT);
	}
	else
	{
		// New map
		SV_SpawnServer(mapname, true);
	}

	// Launch waiting scripts
	if (!deathmatch)
	{
		P_CheckACSStore();
	}
	unguard;
}

#ifdef CLIENT

void Draw_SaveIcon();
void Draw_LoadIcon();

//==========================================================================
//
//	COMMAND Save
//
//	Called by the menu task. Description is a 24 byte text string
//
//==========================================================================

COMMAND(Save)
{
	guard(COMMAND Save)
	if (Args.Num() != 3)
	{
		return;
	}

	if (deathmatch)
	{
		GCon->Log("Can't save in deathmatch game");
		return;
	}

	if (!sv.active)
	{
		GCon->Log("you can't save if you aren't playing!");
		return;
	}

	if (sv.intermission)
	{
		GCon->Log("You can't save while in intermission!");
		return;
	}

	if (Args[2].Length() >= 32)
	{
		GCon->Log("Description too long");
		return;
	}

	Draw_SaveIcon();

	SV_SaveGame(atoi(*Args[1]), *Args[2]);

	GCon->Log("Game saved");
	unguard;
} 
 
//==========================================================================
//
//	COMMAND Load
//
//==========================================================================

COMMAND(Load)
{
	guard(COMMAND Load);
	if (Args.Num() != 2)
	{
		return;
	}
	if (deathmatch)
	{
		GCon->Log("Can't load in deathmatch game");
		return;
	}

	int slot = atoi(*Args[1]);
	char	desc[32];
	if (!SV_GetSaveString(slot, desc))
	{
		GCon->Log("Empty slot");
		return;
	}
	GCon->Logf("Loading \"%s\"", desc);

	Draw_LoadIcon();
	SV_LoadGame(slot);
	if (!netgame)
	{
		// Copy the base slot to the reborn slot
		SV_UpdateRebornSlot();
	}
	unguard;
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.62  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.61  2006/03/16 00:37:55  dj_jl
//	Savegame size optimisations.
//	
//	Revision 1.60  2006/03/12 12:54:49  dj_jl
//	Removed use of bitfields for portability reasons.
//	
//	Revision 1.59  2006/03/06 13:05:51  dj_jl
//	Thunbker list in level, client now uses entity class.
//	
//	Revision 1.58  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//	
//	Revision 1.57  2006/02/28 18:06:28  dj_jl
//	Put thinkers back in linked list.
//	
//	Revision 1.56  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.55  2006/02/27 18:44:25  dj_jl
//	Serialisation of indexes in a compact way.
//	
//	Revision 1.54  2006/02/26 20:52:48  dj_jl
//	Proper serialisation of level and players.
//	
//	Revision 1.53  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//	
//	Revision 1.52  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.51  2006/02/21 17:54:13  dj_jl
//	Save pointer to old stats.
//	
//	Revision 1.50  2006/02/15 23:28:18  dj_jl
//	Moved all server progs global variables to classes.
//	
//	Revision 1.49  2006/02/13 18:34:34  dj_jl
//	Moved all server progs global functions to classes.
//	
//	Revision 1.48  2006/02/05 18:52:44  dj_jl
//	Moved common utils to level info class or built-in.
//	
//	Revision 1.47  2006/01/29 20:41:30  dj_jl
//	On Unix systems use ~/.vavoom for generated files.
//	
//	Revision 1.46  2005/12/29 19:50:24  dj_jl
//	Fixed loading.
//	
//	Revision 1.45  2005/12/29 17:26:01  dj_jl
//	Changed version number.
//	
//	Revision 1.44  2005/12/27 22:24:00  dj_jl
//	Created level info class, moved action special handling to it.
//	
//	Revision 1.43  2005/11/24 20:09:23  dj_jl
//	Removed unused fields from Object class.
//	
//	Revision 1.42  2005/11/20 15:50:40  dj_jl
//	Some fixes.
//	
//	Revision 1.41  2005/11/20 12:38:50  dj_jl
//	Implemented support for sound sequence extensions.
//	
//	Revision 1.40  2005/04/04 07:48:13  dj_jl
//	Fix for loading level variables.
//	
//	Revision 1.39  2005/03/28 07:24:36  dj_jl
//	Saving a net game.
//	
//	Revision 1.38  2005/01/24 12:56:58  dj_jl
//	Saving of level time.
//	
//	Revision 1.37  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.36  2004/12/22 07:50:51  dj_jl
//	Fixed loading of ACS arrays.
//	
//	Revision 1.35  2004/12/03 16:15:47  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.34  2004/01/30 17:32:59  dj_jl
//	Fixed loading
//	
//	Revision 1.33  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//	
//	Revision 1.32  2003/10/22 06:16:53  dj_jl
//	Secret level info saved in savegame
//	
//	Revision 1.31  2003/07/11 16:45:20  dj_jl
//	Made array of players with pointers
//	
//	Revision 1.30  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.29  2002/08/28 16:41:09  dj_jl
//	Merged VMapObject with VEntity, some natives.
//	
//	Revision 1.28  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.27  2002/07/13 07:50:58  dj_jl
//	Added guarding.
//	
//	Revision 1.26  2002/06/14 15:36:35  dj_jl
//	Changed version number.
//	
//	Revision 1.25  2002/05/18 16:56:35  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.24  2002/05/03 17:06:23  dj_jl
//	Mangling of string pointers.
//	
//	Revision 1.23  2002/02/26 17:54:26  dj_jl
//	Importing special property info from progs and using it in saving.
//	
//	Revision 1.22  2002/02/15 19:12:04  dj_jl
//	Property namig style change
//	
//	Revision 1.21  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.20  2002/01/12 18:04:01  dj_jl
//	Added unarchieving of names
//	
//	Revision 1.19  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.18  2001/12/28 16:25:32  dj_jl
//	Fixed loading of ViewEnts
//	
//	Revision 1.17  2001/12/27 17:33:29  dj_jl
//	Removed thinker list
//	
//	Revision 1.16  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.15  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.14  2001/12/04 18:14:46  dj_jl
//	Renamed thinker_t to VThinker
//	
//	Revision 1.13  2001/11/09 18:16:10  dj_jl
//	Fixed copying and deleting when save directory doesn't exist
//	
//	Revision 1.12  2001/11/09 14:32:00  dj_jl
//	Copy and delete using directory listing
//	
//	Revision 1.11  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.10  2001/10/02 17:43:50  dj_jl
//	Added addfields to lines, sectors and polyobjs
//	
//	Revision 1.9  2001/09/24 17:35:24  dj_jl
//	Support for thinker classes
//	
//	Revision 1.8  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.7  2001/08/30 17:46:21  dj_jl
//	Removed game dependency
//	
//	Revision 1.6  2001/08/23 17:48:43  dj_jl
//	NULL pointer is valid thinker, so no warnings
//	
//	Revision 1.5  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.4  2001/08/04 17:32:39  dj_jl
//	Beautification
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
