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
#define SAVE_NAME(_name, _slot) \
	sprintf(_name, "saves/savegame.vs%d", _slot)
#define SAVE_MAP_NAME(_name, _slot, _map) \
	sprintf(_name, "saves/%s.vs%d", _map, _slot)
#define SAVE_NAME_ABS(_name, _slot) \
	sprintf(_name, "%s/savegame.vs%d", SV_GetSavesDir(), _slot)
#define SAVE_MAP_NAME_ABS(_name, _slot, _map) \
	sprintf(_name, "%s/%s.vs%d", SV_GetSavesDir(), _map, _slot)

#define SAVE_DESCRIPTION_LENGTH		24
#define SAVE_VERSION_TEXT			"Version 1.19"
#define SAVE_VERSION_TEXT_LENGTH	16

// TYPES -------------------------------------------------------------------

typedef enum
{
	ASEG_GAME_HEADER = 101,
	ASEG_NAMES,
	ASEG_MAP_HEADER,
	ASEG_BASELINE,
	ASEG_WORLD,
	ASEG_THINKERS,
	ASEG_SCRIPTS,
	ASEG_PLAYERS,
	ASEG_SOUNDS,
	ASEG_END
} gameArchiveSegment_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_SpawnServer(char *mapname, boolean spawn_thinkers);
void SV_SendServerInfoToClients();
void SV_ShutdownServer(boolean);
void CL_Disconnect(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern VEntity		**sv_mobjs;
extern mobj_base_t	*sv_mo_base;
extern bool			sv_loading;
extern int			sv_load_num_players;
extern TMessage		sv_signon;

extern boolean		in_secret;
extern char			mapaftersecret[12];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char		SavesDir[MAX_OSPATH];
static boolean 	SavingPlayers;
static VStream	*Saver;
static VStream	*Loader;

#define GET_BYTE	Streamer<byte>(*Loader)
#define GET_WORD	Streamer<word>(*Loader)
#define GET_LONG	Streamer<int>(*Loader)
#define GET_FLOAT	Streamer<float>(*Loader)

static FName		*NameRemap;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SV_GetSavesDir
//
//==========================================================================

static const char* SV_GetSavesDir()
{
	if (!SavesDir[0])
	{
		if (fl_savedir[0])
			sprintf(SavesDir, "%s/%s/saves", fl_savedir, fl_gamedir);
		else
			sprintf(SavesDir, "%s/%s/saves", fl_basedir, fl_gamedir);
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
	char		fileName[MAX_OSPATH];
	FILE*		f;

	SAVE_NAME_ABS(fileName, slot);
	f = fopen(fileName, "rb");
	if (f)
	{
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
//	OpenStreamOut
//
//==========================================================================

static void OpenStreamOut(char *fileName)
{
	Saver = FL_OpenFileWrite(fileName);
}

//==========================================================================
//
//	StreamOutByte
//
//==========================================================================

static void StreamOutByte(byte val)
{
	*Saver << val;
}

//==========================================================================
//
//	StreamOutLong
//
//==========================================================================

static void StreamOutLong(int val)
{
	*Saver << val;
}

//==========================================================================
//
//	StreamOutFloat
//
//==========================================================================

static void StreamOutFloat(float val)
{
	*Saver << val;
}

//==========================================================================
//
//	StreamOutBuffer
//
//==========================================================================

static void StreamOutBuffer(const void *buffer, int size)
{
	Saver->Serialise(const_cast<void *>(buffer), size);
}

//==========================================================================
//
//	CloseStreamOut
//
//==========================================================================

static void CloseStreamOut(void)
{
	Saver->Close();
	delete Saver;
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
	const char *curName;
	char fileName[MAX_OSPATH];

	sprintf(slotExt, "vs%d", slot);
	if (!Sys_OpenDir(SV_GetSavesDir()))
	{
		//  Directory doesn't exist ... yet
		return;
	}
	while ((curName = Sys_ReadDir()) != NULL)
	{
		char ext[8];

		FL_ExtractFileExtension(curName, ext);
		if (!strcmp(ext, slotExt))
		{
			sprintf(fileName, "%s/%s", SV_GetSavesDir(), curName);
			remove(fileName);
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
	char srcExt[4];
	char dstExt[4];
	const char *curName;
	char sourceName[MAX_OSPATH];
	char destName[MAX_OSPATH];

	sprintf(srcExt, "vs%d", sourceSlot);
	sprintf(dstExt, "vs%d", destSlot);
	if (!Sys_OpenDir(SV_GetSavesDir()))
	{
		//  Directory doesn't exist ... yet
		return;
	}
	while ((curName = Sys_ReadDir()) != NULL)
	{
		char ext[8];

		FL_ExtractFileExtension(curName, ext);
		if (!strcmp(ext, srcExt))
		{
			sprintf(sourceName, "%s/%s", SV_GetSavesDir(), curName);
			strcpy(destName, sourceName);
			FL_StripExtension(destName);
			strcat(destName, ".");
			strcat(destName, dstExt);

			int length;
			byte *buffer;

			length = M_ReadFile(sourceName, &buffer);
			M_WriteFile(destName, buffer, length);
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
	if (GET_LONG != (int)segType)
	{
		Host_Error("Corrupt save game: Segment [%d] failed alignment check",
			segType);
	}
	unguard;
}

//==========================================================================
//
//	GetMobjNum
//
//==========================================================================

int GetMobjNum(VEntity *mobj)
{
	guard(GetMobjNum);
	if (mobj && !mobj->IsA(VEntity::StaticClass()))
	{
		return MOBJ_NULL;
	}
	if (!mobj || (mobj->bIsPlayer && !SavingPlayers))
	{
		return MOBJ_NULL;
	}
	return mobj->NetID;
	unguard;
}

//==========================================================================
//
//	SetMobjPtr
//
//==========================================================================

VEntity* SetMobjPtr(int id)
{
	guard(SetMobjPtr);
	if (id == MOBJ_NULL)
	{
		return NULL;
	}
	return sv_mobjs[id];
	unguard;
}

//==========================================================================
//
//	ArchiveNames
//
//==========================================================================

static void ArchiveNames(VStream &Strm)
{
	int Count = FName::GetMaxNames();
	Strm << Count;
	for (int i = 0; i < Count; i++)
	{
		Strm << *FName::GetEntry(i);
	}
}

//==========================================================================
//
//	UnarchiveNames
//
//==========================================================================

static void UnarchiveNames(VStream &Strm)
{
	int Count;
	Strm << Count;
	NameRemap = (FName*)Z_StrMalloc(Count * 4);
	for (int i = 0; i < Count; i++)
	{
		FNameEntry E;
		Strm << E;
		NameRemap[i] = FName(E.Name);
	}
}

//==========================================================================
//
//	UnarchiveName
//
//==========================================================================

FName UnarchiveName(int Index)
{
	return NameRemap[Index];
}

//==========================================================================
//
//	SV_GetClass
//
//==========================================================================

VClass *SV_GetClass(int NameIndex)
{
	return VClass::FindClass(*NameRemap[NameIndex]);
}

//==========================================================================
//
//	WriteVObject
//
//==========================================================================

void WriteVObject(VObject *Obj)
{
	VClass *Class = Obj->GetClass();

	StreamOutLong(Class->GetFName().GetIndex());
	StreamOutBuffer((byte *)Obj + sizeof(VObject),
		Class->ClassSize - sizeof(VObject));
}

//==========================================================================
//
//	ReadVObject
//
//==========================================================================

VObject *ReadVObject(int tag)
{
	guard(ReadVObject);
	//  Get params
	int NameIndex = GET_LONG;
	VClass *Class = SV_GetClass(NameIndex);
	if (!Class)
	{
		Sys_Error("No such class %s", *NameRemap[NameIndex]);
	}

	//  Allocate object and copy data
	VObject *o = VObject::StaticSpawnObject(Class, tag);
	Loader->Serialise((byte*)o + sizeof(VObject), Class->ClassSize - sizeof(VObject));
	return o;
	unguard;
}

//==========================================================================
//
// ArchivePlayers
//
//==========================================================================

static void ArchivePlayers(void)
{
	guard(ArchivePlayers);
	int			i;
	VBasePlayer	*tempPlayer;

	StreamOutLong(ASEG_PLAYERS);
	for (i = 0; i < MAXPLAYERS; i++)
	{
		StreamOutByte((byte)!!GGameInfo->Players[i]);
	}
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!GGameInfo->Players[i])
		{
			continue;
		}

		tempPlayer = (VBasePlayer*)Z_Malloc(GGameInfo->Players[i]->GetClass()->ClassSize);
		memcpy(tempPlayer, GGameInfo->Players[i], GGameInfo->Players[i]->GetClass()->ClassSize);
		tempPlayer->eventArchivePlayer();
		StreamOutBuffer((byte*)tempPlayer + sizeof(VObject),
			GGameInfo->Players[i]->GetClass()->ClassSize - sizeof(VObject));

		for (int pi = 0; pi < NUMPSPRITES; pi++)
		{
			if (!tempPlayer->ViewEnts[pi])
			{
				continue;
			}
			WriteVObject(tempPlayer->ViewEnts[pi]);
		}
		Z_Free(tempPlayer);
	}
	unguard;
}

//==========================================================================
//
// UnarchivePlayers
//
//==========================================================================

static void UnarchivePlayers(void)
{
	guard(UnarchivePlayers);
	int		i;

	AssertSegment(ASEG_PLAYERS);
	sv_load_num_players = 0;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		byte Active = GET_BYTE;
		GPlayersBase[i]->bActive = Active;
		if (Active)
		{
			GGameInfo->Players[i] = GPlayersBase[i];
			sv_load_num_players++;
		}
		else
		{
			GGameInfo->Players[i] = NULL;
		}
	}
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!GGameInfo->Players[i])
		{
			continue;
		}
		//	Save old stats pointer
		int* OldStats = GGameInfo->Players[i]->OldStats;
		Loader->Serialise((byte*)GGameInfo->Players[i] + sizeof(VObject),
			GGameInfo->Players[i]->GetClass()->ClassSize - sizeof(VObject));
		GGameInfo->Players[i]->MO = NULL; // Will be set when unarc thinker
		GGameInfo->Players[i]->eventUnarchivePlayer();
		GGameInfo->Players[i]->bActive = false;
		//	Restore pointer
		GGameInfo->Players[i]->OldStats = OldStats;

		for (int pi = 0; pi < NUMPSPRITES; pi++)
		{
			if (!GGameInfo->Players[i]->ViewEnts[pi])
			{
				continue;
			}
			GGameInfo->Players[i]->ViewEnts[pi] = (VViewEntity *)ReadVObject(PU_STRING);
			GGameInfo->Players[i]->ViewEnts[pi]->Player = GGameInfo->Players[i];
		}
		GGameInfo->Players[i] = NULL;
	}
	unguard;
}

//==========================================================================
//
//	Level__Serialise
//
//==========================================================================

static void Level__Serialise(VStream& Strm)
{
	guard(Level__Serialise);
	int i;
	int j;
	sector_t* sec;
	line_t* li;
	side_t* si;

	//
	//	Sectors
	//
	for (i = 0, sec = GLevel->Sectors; i < GLevel->NumSectors; i++, sec++)
	{
		Strm << sec->floor.dist
			<< sec->ceiling.dist
			<< sec->floor.pic
			<< sec->ceiling.pic
			<< sec->params.lightlevel
			<< sec->special
			<< sec->tag
			<< sec->seqType;
		if (Strm.IsLoading())
		{
			CalcSecMinMaxs(sec);
		}
	}

	//
	//	Lines
	//
	for (i = 0, li = GLevel->Lines; i < GLevel->NumLines; i++, li++)
	{
		//	Temporary hack to save seen on automap flags.
#ifdef CLIENT
		if (cls.state == ca_connected)
		{
			li->flags |= GClLevel->Lines[i].flags & ML_MAPPED;
		}
#endif
		Strm << li->flags
			<< li->special
			<< li->arg1
			<< li->arg2
			<< li->arg3
			<< li->arg4
			<< li->arg5;
		for (j = 0; j < 2; j++)
		{
			if (li->sidenum[j] == -1)
			{
				continue;
			}
			si = &GLevel->Sides[li->sidenum[j]];
			Strm << si->textureoffset 
				<< si->rowoffset
				<< si->toptexture 
				<< si->bottomtexture 
				<< si->midtexture;
		}
	}

	//
	//	Polyobjs
	//
	for (i = 0; i < GLevel->NumPolyObjs; i++)
	{
		if (Strm.IsLoading())
		{
			float angle, polyX, polyY;

			Strm << angle 
				<< polyX 
				<< polyY;
			PO_RotatePolyobj(GLevel->PolyObjs[i].tag, angle);
			PO_MovePolyobj(GLevel->PolyObjs[i].tag, 
				polyX - GLevel->PolyObjs[i].startSpot.x, 
				polyY - GLevel->PolyObjs[i].startSpot.y);
		}
		else
		{
			Strm << GLevel->PolyObjs[i].angle
				<< GLevel->PolyObjs[i].startSpot.x
				<< GLevel->PolyObjs[i].startSpot.y;
		}
	}
	unguard;
}

//==========================================================================
//
//	ArchiveWorld
//
//==========================================================================

static void ArchiveWorld(void)
{
	StreamOutLong(ASEG_WORLD);

	Level__Serialise(*Saver);
}

//==========================================================================
//
// UnarchiveWorld
//
//==========================================================================

static void UnarchiveWorld(void)
{
	AssertSegment(ASEG_WORLD);

	Level__Serialise(*Loader);
}

//==========================================================================
//
//	MangleVObject
//
//==========================================================================

void MangleVObject(VObject *Obj, VClass *InClass)
{
	guard(MangleVObject);
	if (InClass->GetFlags() & CLASSOF_Native)
	{
		return;
	}
	if (InClass->GetSuperClass())
	{
		MangleVObject(Obj, InClass->GetSuperClass());
	}
	for (int i = 0; i < InClass->NumPropertyInfo; i++)
	{
		int *p = (int *)((byte *)Obj + InClass->PropertyInfo[i].Offset);
		switch (InClass->PropertyInfo[i].Type)
		{
		case PROPTYPE_Reference:
			*p = GetMobjNum((VEntity *)*p);
			break;

		case PROPTYPE_ClassID:
			if (*p)
			{
				*p = ((VClass *)*p)->GetFName().GetIndex();
			}
			else
			{
				*p = -1;
			}
			break;

		case PROPTYPE_Name:
			break;

		case PROPTYPE_String:
			if (*p)
			{
				*p = svpr.GetStringOffs((char *)*p);
			}
			break;
		}
	}
	unguard;
}

//==========================================================================
//
//	UnMangleVObject
//
//==========================================================================

void UnMangleVObject(VObject *Obj, VClass *InClass)
{
	guard(UnMangleVObject);
	if (InClass->GetFlags() & CLASSOF_Native)
	{
		return;
	}
	if (InClass->GetSuperClass())
	{
		UnMangleVObject(Obj, InClass->GetSuperClass());
	}
	for (int i = 0; i < InClass->NumPropertyInfo; i++)
	{
		int *p = (int *)((byte *)Obj + InClass->PropertyInfo[i].Offset);
		switch (InClass->PropertyInfo[i].Type)
		{
		case PROPTYPE_Reference:
			*p = (int)SetMobjPtr(*p);
			break;

		case PROPTYPE_ClassID:
			if (*p == -1)
			{
				*p = 0;
			}
			else
			{
				*p = (int)SV_GetClass(*p);
			}
			break;

		case PROPTYPE_Name:
			*p = UnarchiveName(*p).GetIndex();
			break;

		case PROPTYPE_String:
			if (*p)
			{
				*p = (int)svpr.StrAtOffs(*p);
			}
			break;
		}
	}
	unguard;
}

//==========================================================================
//
// ArchiveThinkers
//
//==========================================================================

static void ArchiveThinkers(void)
{
	guard(ArchiveThinkers);
	StreamOutLong(ASEG_THINKERS);

	for (TObjectIterator<VThinker> It; It; ++It)
	{
		int size = It->GetClass()->ClassSize;

		VThinker *th = (VThinker*)Z_Malloc(size);
		memcpy(th, *It, size);

		VEntity *mobj = Cast<VEntity>(th);
		if (mobj)
		{
			if (mobj->bIsPlayer)
			{
				if (!SavingPlayers)
				{
					// Skipping player mobjs
					Z_Free(th);
					continue;
				}
				mobj->Player = (VBasePlayer *)(SV_GetPlayerNum(mobj->Player) + 1);
			}
		}

		th->eventArchive();
		MangleVObject(th, th->GetClass());

		StreamOutByte(1);
		WriteVObject(th);
		Z_Free(th);
	}

	//
	//  End marker
	//
	StreamOutByte(0);
	unguard;
}

//==========================================================================
//
//  UnarchiveThinkers
//
//==========================================================================

static void UnarchiveThinkers(void)
{
	guard(UnarchiveThinkers);
	VThinker	*thinker;

	AssertSegment(ASEG_THINKERS);

	while (GET_BYTE)
	{
		thinker = (VThinker *)ReadVObject(PU_LEVSPEC);
		thinker->XLevel = GLevel;

		//  Handle level info
		if (thinker->IsA(VLevelInfo::StaticClass()))
		{
			GLevelInfo = (VLevelInfo*)thinker;
		}

		//  Handle entities
		VEntity *Ent = Cast<VEntity>(thinker);
		if (Ent)
		{
			if (Ent->bIsPlayer)
			{
				Ent->Player = GPlayersBase[(int)Ent->Player - 1];
				Ent->Player->MO = Ent;
			}
			Ent->SubSector = NULL;	//	Must mark as not linked
			Ent->LinkToWorld();
			sv_mobjs[Ent->NetID] = Ent;
		}
	}

	//  Call unarchive function for each thinker.
	GLevelInfo->Game = GGameInfo;

	for (TObjectIterator<VThinker> It; It; ++It)
	{
		It->Level = GLevelInfo;
		(*It)->eventUnarchive();
		UnMangleVObject(*It, It->GetClass());
	}

	GLevelInfo->eventAfterUnarchiveThinkers();
	unguard;
}

//==========================================================================
//
// ArchiveScripts
//
//==========================================================================

static void ArchiveScripts(void)
{
	StreamOutLong(ASEG_SCRIPTS);
	P_SerialiseScripts(*Saver);
}

//==========================================================================
//
// UnarchiveScripts
//
//==========================================================================

static void UnarchiveScripts(void)
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
	StreamOutLong(ASEG_SOUNDS);
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
	char fileName[100];

	// Make sure we don't have any garbage
	VObject::CollectGarbage();

	SavingPlayers = savePlayers;

	// Open the output file
	SAVE_MAP_NAME(fileName, slot, level.mapname);
	OpenStreamOut(fileName);

	StreamOutLong(ASEG_NAMES);
	ArchiveNames(*Saver);

	// Place a header marker
	StreamOutLong(ASEG_MAP_HEADER);

	// Write the level timer
	StreamOutFloat(level.time);
	StreamOutLong(level.tictime);

	//	Write totals, because when thinkers are not spawned, they are not
	// counted
	StreamOutLong(level.totalkills);
	StreamOutLong(level.totalitems);
	StreamOutLong(level.totalsecret);
	StreamOutLong(level.currentkills);
	StreamOutLong(level.currentitems);
	StreamOutLong(level.currentsecret);

	StreamOutLong(level.sky1Texture);
	StreamOutLong(level.sky2Texture);
	StreamOutFloat(level.sky1ScrollDelta);
	StreamOutFloat(level.sky2ScrollDelta);
	StreamOutByte(level.doubleSky);
	StreamOutByte(level.lightning);
	StreamOutBuffer(level.skybox, sizeof(level.skybox));

	StreamOutBuffer(level.songLump, sizeof(level.songLump));
	StreamOutLong(level.cdTrack);

	//	Save baseline
	StreamOutLong(ASEG_BASELINE);
	StreamOutLong(sv_signon.CurSize);
	StreamOutBuffer(sv_signon.Data, sv_signon.CurSize);
	StreamOutBuffer(sv_mo_base, sizeof(mobj_base_t) * GMaxEntities);

	ArchiveWorld();
	ArchiveThinkers();
	ArchiveScripts();
	ArchiveSounds();

	// Place a termination marker
	StreamOutLong(ASEG_END);

	// Close the output file
	CloseStreamOut();
	unguard;
}

//==========================================================================
//
//	SV_LoadMap
//
//==========================================================================

static void SV_LoadMap(char *mapname, int slot)
{
	guard(SV_LoadMap);
	char fileName[100];

	// Load a base level
	SV_SpawnServer(mapname, false);

	// Create the name
	SAVE_MAP_NAME(fileName, slot, mapname);

	// Load the file
	Loader = FL_OpenFileRead(fileName);

	// Load names
	AssertSegment(ASEG_NAMES);
	UnarchiveNames(*Loader);

	AssertSegment(ASEG_MAP_HEADER);

	// Read the level timer
	level.time = GET_FLOAT;
	level.tictime = GET_LONG;

	level.totalkills = GET_LONG;
	level.totalitems = GET_LONG;
	level.totalsecret = GET_LONG;
	level.currentkills = GET_LONG;
	level.currentitems = GET_LONG;
	level.currentsecret = GET_LONG;

	level.sky1Texture = GET_LONG;
	level.sky2Texture = GET_LONG;
	level.sky1ScrollDelta = GET_FLOAT;
	level.sky2ScrollDelta = GET_FLOAT;
	level.doubleSky = GET_BYTE;
	level.lightning = GET_BYTE;
	Loader->Serialise(level.skybox, sizeof(level.skybox));

	Loader->Serialise(level.songLump, sizeof(level.songLump));
	level.cdTrack = GET_LONG;

	AssertSegment(ASEG_BASELINE);
	int len = GET_LONG;
	sv_signon.Clear();
	void *tmp = Z_StrMalloc(len);
	Loader->Serialise(tmp, len);
	sv_signon.Write(tmp, len);
	Loader->Serialise(sv_mo_base, sizeof(mobj_base_t) * GMaxEntities);

	UnarchiveWorld();
	UnarchiveThinkers();
	UnarchiveScripts();
	UnarchiveSounds();

	AssertSegment(ASEG_END);

	// Free save buffer
	Loader->Close();
	delete Loader;

	Z_Free(NameRemap);

	//	Do this here so that clients have loaded info, not initial one.
	SV_SendServerInfoToClients();
	unguard;
}

//==========================================================================
//
//	SV_SaveGame
//
//==========================================================================

void SV_SaveGame(int slot, char* description)
{
	guard(SV_SaveGame);
	char versionText[SAVE_VERSION_TEXT_LENGTH];
	char fileName[100];
	int i;

	// Open the output file
	SAVE_NAME(fileName, BASE_SLOT);
	OpenStreamOut(fileName);

	// Write game save description
	StreamOutBuffer(description, SAVE_DESCRIPTION_LENGTH);

	// Write version info
	memset(versionText, 0, SAVE_VERSION_TEXT_LENGTH);
	strcpy(versionText, SAVE_VERSION_TEXT);
	StreamOutBuffer(versionText, SAVE_VERSION_TEXT_LENGTH);

	// Write names
	StreamOutLong(ASEG_NAMES);
	ArchiveNames(*Saver);

	// Place a header marker
	StreamOutLong(ASEG_GAME_HEADER);

	// Write current map and difficulty
	StreamOutByte((byte)gameskill);
	StreamOutBuffer(level.mapname, 8);

	// Write secret level info
	StreamOutByte(in_secret);
	StreamOutBuffer(mapaftersecret, 8);

	// Write global script info
	StreamOutBuffer(WorldVars, sizeof(WorldVars));
	StreamOutBuffer(GlobalVars, sizeof(GlobalVars));
	for (i = 0; i < MAX_ACS_WORLD_VARS; i++)
	{
		WorldArrays[i].Serialise(*Saver);
	}
	for (i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
	{
		GlobalArrays[i].Serialise(*Saver);
	}
	StreamOutBuffer(ACSStore, sizeof(ACSStore));

	ArchivePlayers();

	// Place a termination marker
	StreamOutLong(ASEG_END);

	// Close the output file
	CloseStreamOut();

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
	char		fileName[100];
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

	// Create the name
	SAVE_NAME(fileName, BASE_SLOT);

	// Load the file
	Loader = FL_OpenFileRead(fileName);

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

	// Load names
	AssertSegment(ASEG_NAMES);
	UnarchiveNames(*Loader);

	AssertSegment(ASEG_GAME_HEADER);

	gameskill = (skill_t)GET_BYTE;
	Loader->Serialise(mapname, 8);
	mapname[8] = 0;

	//	Init skill hacks
	GGameInfo->eventInitNewGame(gameskill);

	// Read secret level info
	in_secret = GET_BYTE;
	Loader->Serialise(mapaftersecret, 8);
	mapaftersecret[8] = 0;

	// Read global script info
	Loader->Serialise(WorldVars, sizeof(WorldVars));
	Loader->Serialise(GlobalVars, sizeof(GlobalVars));
	for (i = 0; i < MAX_ACS_WORLD_VARS; i++)
	{
		WorldArrays[i].Serialise(*Loader);
	}
	for (i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
	{
		GlobalArrays[i].Serialise(*Loader);
	}
	Loader->Serialise(ACSStore, sizeof(ACSStore));

	// Read the player structures
	UnarchivePlayers();

	AssertSegment(ASEG_END);

	Loader->Close();
	delete Loader;

	Z_Free(NameRemap);

	sv_loading = true;

	// Load the current map
	SV_LoadMap(mapname, BASE_SLOT);

#ifdef CLIENT
	if (cls.state != ca_dedicated)
		CmdBuf << "Connect local\n";
#endif
	unguard;
}

//==========================================================================
//
//	SV_InitBaseSlot
//
//==========================================================================

void SV_InitBaseSlot(void)
{
	ClearSaveSlot(BASE_SLOT);
}

//==========================================================================
//
// SV_GetRebornSlot
//
//==========================================================================

int SV_GetRebornSlot(void)
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

boolean SV_RebornSlotAvailable(void)
{
	char fileName[100];

	SAVE_NAME_ABS(fileName, REBORN_SLOT);
	return Sys_FileExists(fileName);
}

//==========================================================================
//
// SV_UpdateRebornSlot
//
// Copies the base slot to the reborn slot.
//
//==========================================================================

void SV_UpdateRebornSlot(void)
{
	ClearSaveSlot(REBORN_SLOT);
	CopySaveSlot(BASE_SLOT, REBORN_SLOT);
}

//==========================================================================
//
// SV_ClearRebornSlot
//
//==========================================================================

void SV_ClearRebornSlot(void)
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
	char		fileName[100];

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

	SAVE_MAP_NAME_ABS(fileName, BASE_SLOT, mapname);
	if (!deathmatch && Sys_FileExists(fileName))
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

void Draw_SaveIcon(void);
void Draw_LoadIcon(void);

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
	if (Argc() != 3)
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

	if (strlen(Argv(2)) >= 32)
	{
		GCon->Log("Description too long");
		return;
	}

	Draw_SaveIcon();

	SV_SaveGame(atoi(Argv(1)), Argv(2));

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
	if (Argc() != 2)
	{
		return;
	}
	if (deathmatch)
	{
		GCon->Log("Can't load in deathmatch game");
		return;
	}

	int slot = atoi(Argv(1));
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
