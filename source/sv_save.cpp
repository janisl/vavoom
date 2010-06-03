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

#define REBORN_SLOT				9

#define EMPTYSTRING				"empty slot"
#define MOBJ_NULL 				-1
#define SAVE_NAME(_slot) \
	(VStr("saves/save") + _slot + ".vsg")
#define SAVE_NAME_ABS(_slot) \
	(SV_GetSavesDir() + "/save" + _slot + ".vsg")

#define SAVE_DESCRIPTION_LENGTH		24
#define SAVE_VERSION_TEXT			"Version 1.31"
#define SAVE_VERSION_TEXT_LENGTH	16

// TYPES -------------------------------------------------------------------

enum gameArchiveSegment_t
{
	ASEG_MAP_HEADER = 101,
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
	vint32				DecompressedSize;
};

class VSaveSlot
{
public:
	VStr				Description;
	VName				CurrentMap;
	TArray<VSavedMap*>	Maps;

	~VSaveSlot()
	{
		Clear();
	}
	void Clear();
	bool LoadSlot(int Slot);
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
		Stream = NULL;
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
		Stream = NULL;
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
	if (fl_savedir.IsNotEmpty())
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
	Description.Clean();
	CurrentMap = NAME_None;
	for (int i = 0; i < Maps.Num(); i++)
	{
		delete Maps[i];
		Maps[i] = NULL;
	}
	Maps.Clear();
	unguard;
}

//==========================================================================
//
//	VSaveSlot::LoadSlot
//
//==========================================================================

bool VSaveSlot::LoadSlot(int Slot)
{
	guard(VSaveSlot::LoadSlot);
	Clear();
	VStream* Strm = FL_OpenFileRead(SAVE_NAME(Slot));
	if (!Strm)
	{
		GCon->Log("Savegame file doesn't exist");
		return false;
	}

	// Check the version text
	char VersionText[SAVE_VERSION_TEXT_LENGTH];
	Strm->Serialise(VersionText, SAVE_VERSION_TEXT_LENGTH);
	if (VStr::Cmp(VersionText, SAVE_VERSION_TEXT))
	{
		// Bad version
		Strm->Close();
		delete Strm;
		Strm = NULL;
		GCon->Log("Savegame is from incompatible version");
		return false;
	}

	*Strm << Description;

	VStr TmpName;
	*Strm << TmpName;
	CurrentMap = *TmpName;

	int NumMaps;
	*Strm << STRM_INDEX(NumMaps);
	for (int i = 0; i < NumMaps; i++)
	{
		VSavedMap* Map = new VSavedMap();
		Maps.Append(Map);
		vint32 DataLen;
		*Strm << TmpName
			<< Map->DecompressedSize
			<< STRM_INDEX(DataLen);
		Map->Name = *TmpName;
		Map->Data.SetNum(DataLen);
		Strm->Serialise(Map->Data.Ptr(), Map->Data.Num());
	}

	Strm->Close();
	delete Strm;
	Strm = NULL;
	return true;
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
	VStream* Strm = FL_OpenFileWrite(*SAVE_NAME(Slot));

	// Write version info
	char VersionText[SAVE_VERSION_TEXT_LENGTH];
	memset(VersionText, 0, SAVE_VERSION_TEXT_LENGTH);
	VStr::Cpy(VersionText, SAVE_VERSION_TEXT);
	Strm->Serialise(VersionText, SAVE_VERSION_TEXT_LENGTH);

	// Write game save description
	*Strm << Description;

	// Write current map
	VStr TmpName(CurrentMap);
	*Strm << TmpName;

	int NumMaps = Maps.Num();
	*Strm << STRM_INDEX(NumMaps);
	for (int i = 0; i < Maps.Num(); i++)
	{
		VStr TmpName(Maps[i]->Name);
		vint32 DataLen = Maps[i]->Data.Num();
		*Strm << TmpName
			<< Maps[i]->DecompressedSize
			<< STRM_INDEX(DataLen);
		Strm->Serialise(Maps[i]->Data.Ptr(), Maps[i]->Data.Num());
	}

	Strm->Close();
	delete Strm;
	Strm = NULL;
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

bool SV_GetSaveString(int Slot, VStr& Desc)
{
	guard(SV_GetSaveString);
	VStream* Strm = FL_OpenFileRead(SAVE_NAME(Slot));
	if (Strm)
	{
		char VersionText[SAVE_VERSION_TEXT_LENGTH];
		Strm->Serialise(VersionText, SAVE_VERSION_TEXT_LENGTH);
		*Strm << Desc;
		if (VStr::Cmp(VersionText, SAVE_VERSION_TEXT))
		{
			//	Bad version, put an asterisk in front of the description.
			Desc = "*" + Desc;
		}
		delete Strm;
		Strm = NULL;
		return true;
	}
	else
	{
		Desc = EMPTYSTRING;
		return false;
	}
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

	//	Compress map data.
	Map->DecompressedSize = Buf.Num();
	Map->Data.Clear();
	VArrayStream* ArrStrm = new VArrayStream(Map->Data);
	ArrStrm->BeginWrite();
	VZipStreamWriter* ZipStrm = new VZipStreamWriter(ArrStrm);
	ZipStrm->Serialise(Buf.Ptr(), Buf.Num());
	delete ZipStrm;
	ZipStrm = NULL;
	delete ArrStrm;
	ArrStrm = NULL;

	delete Saver;
	Saver = NULL;
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

	VSavedMap* Map = BaseSlot.FindMap(MapName);
	check(Map);

	//	Decompress map data
	VArrayStream* ArrStrm = new VArrayStream(Map->Data);
	VZipStreamReader* ZipStrm = new VZipStreamReader(ArrStrm);
	TArray<vuint8> DecompressedData;
	DecompressedData.SetNum(Map->DecompressedSize);
	ZipStrm->Serialise(DecompressedData.Ptr(), DecompressedData.Num());
	delete ZipStrm;
	ZipStrm = NULL;
	delete ArrStrm;
	ArrStrm = NULL;

	VSaveLoaderStream* Loader = new VSaveLoaderStream(
		new VArrayStream(DecompressedData));

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
	Loader = NULL;

	//	Do this here so that clients have loaded info, not initial one.
	SV_SendServerInfoToClients();
	unguard;
}

//==========================================================================
//
//	SV_SaveGame
//
//==========================================================================

void SV_SaveGame(int slot, const VStr& Description)
{
	guard(SV_SaveGame);
	BaseSlot.Description = Description;
	BaseSlot.CurrentMap = GLevel->MapName;

	// Save out the current map
	SV_SaveMap(true); // true = save player info

	// Write data to destination slot
	BaseSlot.SaveToSlot(slot);
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
	SV_ShutdownGame();

	if (!BaseSlot.LoadSlot(slot))
	{
		return;
	}

	sv_loading = true;

	// Load the current map
	SV_LoadMap(BaseSlot.CurrentMap);

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
	BaseSlot.Clear();
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
	BaseSlot.SaveToSlot(REBORN_SLOT);
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
		}
		else
		{
			// Entering new cluster - clear base slot
			BaseSlot.Clear();
		}
	}

	sv_map_travel = true;
	if (!deathmatch && BaseSlot.FindMap(mapname))
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

	SV_SaveGame(atoi(*Args[1]), Args[2]);

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
	if (!SV_GetSaveString(slot, desc))
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
