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
//**	
//**	Archiving: SaveGame I/O.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "network.h"
#include "sv_local.h"
#include "zipstream.h"

// MACROS ------------------------------------------------------------------

#define MAX_TARGET_PLAYERS 512
#define MAX_MAPS	99
#define BASE_SLOT	8
#define REBORN_SLOT	9

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
#define SAVE_VERSION_TEXT			"Version 1.30"
#define SAVE_VERSION_TEXT_LENGTH	16

// TYPES -------------------------------------------------------------------

enum gameArchiveSegment_t
{
	ASEG_GAME_HEADER = 101,
	ASEG_MAP_HEADER,
	ASEG_WORLD,
	ASEG_SCRIPTS,
	ASEG_SOUNDS,
	ASEG_END
};

class VSavedMap
{
public:
	TArray<vuint8>		Data;
	VName				Name;
};

class VSaveSlot
{
public:
	TArray<VSavedMap*>	Maps;
	TArray<vuint8>		GeneralData;

	~VSaveSlot()
	{
		Clear();
	}
	void Clear();
	void LoadSlot(int Slot);
	void SaveToSlot(int Slot);
	VSavedMap* FindMap(VName Name);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VSaveSlot		BaseSlot;

// CODE --------------------------------------------------------------------

class VSaveLoaderStream : public VStream
{
private:
	VStream*			Stream;

public:
	TArray<VName>		NameRemap;
	TArray<VObject*>	Exports;

	VSaveLoaderStream(VStream* InStream)
	: Stream(InStream)
	{
		bLoading = true;
	}
	~VSaveLoaderStream()
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
		int NameIndex;
		*this << STRM_INDEX(NameIndex);
		Name = NameRemap[NameIndex];
		return *this;
	}
	VStream& operator<<(VObject*& Ref)
	{
		guard(Loader::operator<<VObject*&);
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
		return *this;
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
			Ptr = NULL;
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
	TArray<vint32>		NamesMap;
	TArray<vint32>		ObjectsMap;

	VSaveWriterStream(VStream* InStream)
	: Stream(InStream)
	{
		bLoading = false;
		NamesMap.SetNum(VName::GetNumNames());
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
			NamesMap[Name.GetIndex()] = Names.Append(Name);
		}
		*this << STRM_INDEX(NamesMap[Name.GetIndex()]);
		return *this;
	}
	VStream& operator<<(VObject*& Ref)
	{
		guard(Saver::operator<<VObject*&);
		int TmpIdx;
		if (!Ref)
		{
			TmpIdx = 0;
		}
		else
		{
			TmpIdx = ObjectsMap[Ref->GetIndex()];
		}
		return *this << STRM_INDEX(TmpIdx);
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
			TmpIdx = -1;
		}
		*this << STRM_INDEX(TmpIdx);
	}
};

//==========================================================================
//
//	SV_GetSavesDir
//
//==========================================================================

static VStr SV_GetSavesDir()
{
	if (fl_savedir)
		return fl_savedir + "/" + fl_gamedir + "/saves";
	else
		return fl_basedir + "/" + fl_gamedir + "/saves";
}

//==========================================================================
//
//	VSaveSlot::Clear
//
//==========================================================================

void VSaveSlot::Clear()
{
	guard(VSaveSlot::Clear);
	for (int i = 0; i < Maps.Num(); i++)
		delete Maps[i];
	Maps.Clear();
	GeneralData.Clear();
	unguard;
}

//==========================================================================
//
//	VSaveSlot::LoadSlot
//
//==========================================================================

void VSaveSlot::LoadSlot(int Slot)
{
	guard(VSaveSlot::LoadSlot);
	Clear();
	char slotExt[4];
	VStr curName;

	sprintf(slotExt, "vs%d", Slot);
	if (!Sys_OpenDir(SV_GetSavesDir()))
	{
		//  Directory doesn't exist ... yet
		return;
	}
	while ((curName = Sys_ReadDir()))
	{
		VStr ext = curName.ExtractFileExtension();
		if (ext == slotExt && curName != VStr("savegame.") + VStr(slotExt))
		{
			GCon->Logf("Loading %s", *curName);
			VSavedMap* Map = new VSavedMap();
			Map->Name = *curName.ExtractFileBase();
			VStream* Strm = FL_OpenFileRead(SAVE_MAP_NAME(Slot, *Map->Name));
			Map->Data.SetNum(Strm->TotalSize());
			Strm->Serialise(Map->Data.Ptr(), Map->Data.Num());
			delete Strm;
			Maps.Append(Map);
		}
	}
	Sys_CloseDir();

	VStream* Strm = FL_OpenFileRead(SAVE_NAME(Slot));
	if (Strm)
	{
		GeneralData.SetNum(Strm->TotalSize());
		Strm->Serialise(GeneralData.Ptr(), GeneralData.Num());
		delete Strm;
	}
	unguard;
}

//==========================================================================
//
//	VSaveSlot::SaveToSlot
//
//==========================================================================

void VSaveSlot::SaveToSlot(int Slot)
{
	guard(VSaveSlot::SaveToSlot);
	for (int i = 0; i < Maps.Num(); i++)
	{
		VStream* Strm = FL_OpenFileWrite(SAVE_MAP_NAME(Slot, *Maps[i]->Name));
		Strm->Serialise(Maps[i]->Data.Ptr(), Maps[i]->Data.Num());
		Strm->Close();
		delete Strm;
	}

	VStream* Strm = FL_OpenFileWrite(*SAVE_NAME(Slot));
	Strm->Serialise(GeneralData.Ptr(), GeneralData.Num());
	Strm->Close();
	delete Strm;
	unguard;
}

//==========================================================================
//
//	VSaveSlot::FindMap
//
//==========================================================================

VSavedMap* VSaveSlot::FindMap(VName Name)
{
	guard(VSaveSlot::FindMap);
	for (int i = 0; i < Maps.Num(); i++)
	{
		if (Maps[i]->Name == Name)
		{
			return Maps[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	SV_GetSaveString
//
//==========================================================================

bool SV_GetSaveString(int slot, VStr* buf)
{
	guard(SV_GetSaveString);
	FILE*		f;
	char		Desc[SAVE_DESCRIPTION_LENGTH + 1];

	f = fopen(*SAVE_NAME_ABS(slot), "rb");
	if (f)
	{
		fseek(f, 4, SEEK_SET);
		fread(Desc, 1, SAVE_DESCRIPTION_LENGTH, f);
		Desc[SAVE_DESCRIPTION_LENGTH] = 0;
		*buf = Desc;
		fclose(f);
		return true;
	}
	else
	{
		*buf = EMPTYSTRING;
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

static void AssertSegment(VStream& Strm, gameArchiveSegment_t segType)
{
	guard(AssertSegment);
	if (Streamer<int>(Strm) != (int)segType)
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

static void ArchiveNames(VSaveWriterStream* Saver)
{
	//	Write offset to the names in the beginning of the file.
	vint32 NamesOffset = Saver->Tell();
	Saver->Seek(0);
	*Saver << NamesOffset;
	Saver->Seek(NamesOffset);

	//	Serialise names.
	vint32 Count = Saver->Names.Num();
	*Saver << STRM_INDEX(Count);
	for (int i = 0; i < Count; i++)
	{
		*Saver << *VName::GetEntry(Saver->Names[i].GetIndex());
	}
}

//==========================================================================
//
//	UnarchiveNames
//
//==========================================================================

static void UnarchiveNames(VSaveLoaderStream* Loader)
{
	vint32 NamesOffset;
	*Loader << NamesOffset;

	vint32 TmpOffset = Loader->Tell();
	Loader->Seek(NamesOffset);
	vint32 Count;
	*Loader << STRM_INDEX(Count);
	Loader->NameRemap.SetNum(Count);
	for (int i = 0; i < Count; i++)
	{
		VNameEntry E;
		*Loader << E;
		Loader->NameRemap[i] = VName(E.Name);
	}
	Loader->Seek(TmpOffset);
}

//==========================================================================
//
// ArchiveThinkers
//
//==========================================================================

static void ArchiveThinkers(VSaveWriterStream* Saver, bool SavingPlayers)
{
	guard(ArchiveThinkers);
	vint32 Seg = ASEG_WORLD;
	*Saver << Seg;

	Saver->ObjectsMap.SetNum(VObject::GetObjectsCount());
	for (int i = 0; i < VObject::GetObjectsCount(); i++)
	{
		Saver->ObjectsMap[i] = 0;
	}

	//	Add level
	Saver->Exports.Append(GLevel);
	Saver->ObjectsMap[GLevel->GetIndex()] = Saver->Exports.Num();

	//	Add world info
	vuint8 WorldInfoSaved = (byte)SavingPlayers;
	*Saver << WorldInfoSaved;
	if (WorldInfoSaved)
	{
		Saver->Exports.Append(GGameInfo->WorldInfo);
		Saver->ObjectsMap[GGameInfo->WorldInfo->GetIndex()] = Saver->Exports.Num();
	}

	//	Add players.
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		byte Active = (byte)(SavingPlayers && GGameInfo->Players[i]);
		*Saver << Active;
		if (!Active)
		{
			continue;
		}

		Saver->Exports.Append(GGameInfo->Players[i]);
		Saver->ObjectsMap[GGameInfo->Players[i]->GetIndex()] = Saver->Exports.Num();
	}

	//	Add thinkers.
	int ThinkersStart = Saver->Exports.Num();
	for (TThinkerIterator<VThinker> Th(GLevel); Th; ++Th)
	{
		VEntity *mobj = Cast<VEntity>(*Th);
		if (mobj && mobj->EntityFlags & VEntity::EF_IsPlayer && !SavingPlayers)
		{
			// Skipping player mobjs
			continue;
		}

		Saver->Exports.Append(*Th);
		Saver->ObjectsMap[Th->GetIndex()] = Saver->Exports.Num();
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

static void UnarchiveThinkers(VSaveLoaderStream* Loader)
{
	guard(UnarchiveThinkers);
	VObject*			Obj;

	AssertSegment(*Loader, ASEG_WORLD);

	//	Add level.
	Loader->Exports.Append(GLevel);

	//	Add world info
	vuint8 WorldInfoSaved;
	*Loader << WorldInfoSaved;
	if (WorldInfoSaved)
	{
		Loader->Exports.Append(GGameInfo->WorldInfo);
	}

	//	Add players.
	sv_load_num_players = 0;
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		byte Active;
		*Loader << Active;
		if (Active)
		{
			sv_load_num_players++;
			Loader->Exports.Append(GPlayersBase[i]);
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
		Obj = VObject::StaticSpawnObject(Class);

		//  Handle level info
		if (Obj->IsA(VLevelInfo::StaticClass()))
		{
			GLevelInfo = (VLevelInfo*)Obj;
			GLevelInfo->Game = GGameInfo;
			GLevelInfo->World = GGameInfo->WorldInfo;
			GLevel->LevelInfo = GLevelInfo;
		}

		Loader->Exports.Append(Obj);
	}

	GLevelInfo->Game = GGameInfo;
	GLevelInfo->World = GGameInfo->WorldInfo;

	for (int i = 0; i < Loader->Exports.Num(); i++)
	{
		Loader->Exports[i]->Serialise(*Loader);
	}

	GLevelInfo->eventAfterUnarchiveThinkers();
	unguard;
}

//==========================================================================
//
// ArchiveSounds
//
//==========================================================================

static void ArchiveSounds(VStream& Strm)
{
	vint32 Seg = ASEG_SOUNDS;
	Strm << Seg;
#ifdef CLIENT
	GAudio->SerialiseSounds(Strm);
#else
	vint32 Dummy = 0;
	Strm << Dummy;
#endif
}

//==========================================================================
//
// UnarchiveSounds
//
//==========================================================================

static void UnarchiveSounds(VStream& Strm)
{
	AssertSegment(Strm, ASEG_SOUNDS);
#ifdef CLIENT
	GAudio->SerialiseSounds(Strm);
#else
	vint32 Dummy = 0;
	Strm << Dummy;
	Strm.Seek(Strm.Tell() + Dummy * 36);
#endif
}

//==========================================================================
//
// SV_SaveMap
//
//==========================================================================

static void SV_SaveMap(bool savePlayers)
{
	guard(SV_SaveMap);
	// Make sure we don't have any garbage
	VObject::CollectGarbage();

	// Open the output file
	VMemoryStream* InStrm = new VMemoryStream();
	VSaveWriterStream* Saver = new VSaveWriterStream(InStrm);

	int NamesOffset = 0;
	*Saver << NamesOffset;

	// Place a header marker
	vint32 Seg = ASEG_MAP_HEADER;
	*Saver << Seg;

	// Write the level timer
	*Saver << GLevel->Time
		<< GLevel->TicTime;

	ArchiveThinkers(Saver, savePlayers);
	ArchiveSounds(*Saver);

	// Place a termination marker
	Seg = ASEG_END;
	*Saver << Seg;

	ArchiveNames(Saver);

	// Close the output file
	Saver->Close();

	TArray<vuint8>& Buf = InStrm->GetArray();

	VSavedMap* Map = BaseSlot.FindMap(GLevel->MapName);
	if (!Map)
	{
		Map = new VSavedMap();
		BaseSlot.Maps.Append(Map);
		Map->Name = GLevel->MapName;
	}
	Map->Data = InStrm->GetArray();

	delete Saver;
	unguard;
}

//==========================================================================
//
//	SV_LoadMap
//
//==========================================================================

static void SV_LoadMap(VName MapName)
{
	guard(SV_LoadMap);
	// Load a base level
	SV_SpawnServer(*MapName, false, false);

	// Load the file
	VSavedMap* Map = BaseSlot.FindMap(MapName);
	check(Map);
	VSaveLoaderStream* Loader = new VSaveLoaderStream(
		new VArrayStream(Map->Data));

	// Load names
	UnarchiveNames(Loader);

	AssertSegment(*Loader, ASEG_MAP_HEADER);

	// Read the level timer
	*Loader << GLevel->Time
		<< GLevel->TicTime;

	UnarchiveThinkers(Loader);
	UnarchiveSounds(*Loader);

	AssertSegment(*Loader, ASEG_END);

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

	BaseSlot.LoadSlot(BASE_SLOT);

	// Open the output file
	VMemoryStream* InStrm = new VMemoryStream();
	VSaveWriterStream* Saver = new VSaveWriterStream(InStrm);

	int NamesOffset = 0;
	*Saver << NamesOffset;

	// Write game save description
	char desc[SAVE_DESCRIPTION_LENGTH];
	memset(desc, 0, sizeof(desc));
	VStr::NCpy(desc, description, SAVE_DESCRIPTION_LENGTH - 1);
	Saver->Serialise(desc, SAVE_DESCRIPTION_LENGTH);

	// Write version info
	memset(versionText, 0, SAVE_VERSION_TEXT_LENGTH);
	VStr::Cpy(versionText, SAVE_VERSION_TEXT);
	Saver->Serialise(versionText, SAVE_VERSION_TEXT_LENGTH);

	// Place a header marker
	vint32 Seg = ASEG_GAME_HEADER;
	*Saver << Seg;

	// Write current map
	*Saver << GLevel->MapName;

	// Place a termination marker
	Seg = ASEG_END;
	*Saver << Seg;

	// Write names
	ArchiveNames(Saver);

	BaseSlot.GeneralData = InStrm->GetArray();

	// Close the output file
	Saver->Close();
	delete Saver;

	// Save out the current map
	SV_SaveMap(true); // true = save player info

	BaseSlot.SaveToSlot(BASE_SLOT);

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
	VName		mapname;

	SV_ShutdownGame();

	// Copy all needed save files to the base slot
	if (slot != BASE_SLOT)
	{
		ClearSaveSlot(BASE_SLOT);
		CopySaveSlot(slot, BASE_SLOT);
	}
	BaseSlot.LoadSlot(BASE_SLOT);

	// Load the file
	VSaveLoaderStream* Loader = new VSaveLoaderStream(
		new VArrayStream(BaseSlot.GeneralData));

	// Load names
	UnarchiveNames(Loader);

	// Set the save pointer and skip the description field
	char desc[SAVE_DESCRIPTION_LENGTH];
	Loader->Serialise(desc, SAVE_DESCRIPTION_LENGTH);

	// Check the version text
	char versionText[SAVE_VERSION_TEXT_LENGTH];
	Loader->Serialise(versionText, SAVE_VERSION_TEXT_LENGTH);
	if (VStr::Cmp(versionText, SAVE_VERSION_TEXT))
	{
		// Bad version
		Loader->Close();
		delete Loader;
		GCon->Log("Savegame is from incompatible version");
		return;
	}

	AssertSegment(*Loader, ASEG_GAME_HEADER);

	*Loader << mapname;

	AssertSegment(*Loader, ASEG_END);

	Loader->Close();
	delete Loader;

	sv_loading = true;

	// Load the current map
	SV_LoadMap(mapname);

#ifdef CLIENT
	if (GGameInfo->NetMode != NM_DedicatedServer)
	{
		CL_SetUpLocalPlayer();
	}
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

bool SV_RebornSlotAvailable()
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

void SV_MapTeleport(VName mapname)
{
	guard(SV_MapTeleport);
	TArray<VThinker*>		TravelObjs;

	//	Call PreTravel event
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!GGameInfo->Players[i])
		{
			continue;
		}
		GGameInfo->Players[i]->eventPreTravel();
	}

	//	Coolect list of thinkers that will go to the new level.
	for (VThinker* Th = GLevel->ThinkerHead; Th; Th = Th->Next)
	{
		VEntity *vent = Cast<VEntity>(Th);
		if (vent && (//(vent->EntityFlags & VEntity::EF_IsPlayer) ||
			(vent->Owner && (vent->Owner->EntityFlags & VEntity::EF_IsPlayer))))
		{
			TravelObjs.Append(vent);
			GLevel->RemoveThinker(vent);
			vent->UnlinkFromWorld();
			GLevel->DelSectorList();
			vent->StopSound(0);
		}
		if (Th->IsA(VPlayerReplicationInfo::StaticClass()))
		{
			TravelObjs.Append(Th);
			GLevel->RemoveThinker(Th);
		}
	}

	BaseSlot.LoadSlot(BASE_SLOT);
	if (!deathmatch)
	{
		const mapInfo_t& old_info = P_GetMapInfo(GLevel->MapName);
		const mapInfo_t& new_info = P_GetMapInfo(mapname);
		//	All maps in cluster 0 are treated as in different clusters.
		if (old_info.Cluster && old_info.Cluster == new_info.Cluster &&
			(P_GetClusterDef(old_info.Cluster)->Flags & CLUSTERF_Hub))
		{
			// Same cluster - save map without saving player mobjs
			SV_SaveMap(false);
			BaseSlot.SaveToSlot(BASE_SLOT);
		}
		else
		{
			// Entering new cluster - clear base slot
			ClearSaveSlot(BASE_SLOT);
			BaseSlot.Clear();
		}
	}

	sv_map_travel = true;
	if (!deathmatch && Sys_FileExists(SAVE_MAP_NAME_ABS(BASE_SLOT, *mapname)))
	{
		// Unarchive map
		SV_LoadMap(mapname);
	}
	else
	{
		// New map
		SV_SpawnServer(*mapname, true, false);
	}

	//	Add traveling thinkers to the new level.
	for (int i = 0; i < TravelObjs.Num(); i++)
	{
		GLevel->AddThinker(TravelObjs[i]);
		VEntity* Ent = Cast<VEntity>(TravelObjs[i]);
		if (Ent)
		{
			Ent->LinkToWorld();
		}
	}

#ifdef CLIENT
	if (GGameInfo->NetMode == NM_TitleMap ||
		GGameInfo->NetMode == NM_Standalone ||
		GGameInfo->NetMode == NM_ListenServer)
	{
		CL_SetUpStandaloneClient();
	}
#endif

	// Launch waiting scripts
	if (!deathmatch)
	{
		GLevel->Acs->CheckAcsStore();
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

	if (GGameInfo->NetMode == NM_None || GGameInfo->NetMode == NM_TitleMap ||
		GGameInfo->NetMode == NM_Client)
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
	VStr desc;
	if (!SV_GetSaveString(slot, &desc))
	{
		GCon->Log("Empty slot");
		return;
	}
	GCon->Logf("Loading \"%s\"", *desc);

	Draw_LoadIcon();
	SV_LoadGame(slot);
	if (GGameInfo->NetMode == NM_Standalone)
	{
		// Copy the base slot to the reborn slot
		SV_UpdateRebornSlot();
	}
	unguard;
}

#endif
