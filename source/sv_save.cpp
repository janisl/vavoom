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
#include "archive.h"

// MACROS ------------------------------------------------------------------

#define USE_STDIO

#define MAX_TARGET_PLAYERS 512
#define MAX_MAPS	99
#define BASE_SLOT	8
#define REBORN_SLOT	9

#define SAVEGAMENAME			"vavm"
#define EMPTYSTRING				"empty slot"
#define MOBJ_NULL 				-1
#define SAVE_NAME(_name, _slot) \
   	sprintf(_name, "%s/saves/savegame.vs%d", fl_gamedir, _slot)
#define SAVE_MAP_NAME(_name, _slot, _map) \
   	sprintf(_name, "%s/saves/%s.vs%d", fl_gamedir, _map, _slot)

#define SAVE_DESCRIPTION_LENGTH		24
#define SAVE_VERSION_TEXT			"Version 1.11"
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

class FSaver:public FArchive
{
public:
	FSaver()
	{}

	// FArchive intervace
	void Serialize(void *V, int Length);

	// Public functions
	void OpenRead(const char *FileName);
	void OpenWrite(const char *FileName);
	void Close(void);
private:
#ifdef USE_STDIO
	FILE *Handle;
#else
	int Handle;
#endif
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_SpawnServer(char *mapname, boolean spawn_thinkers);
void SV_ShutdownServer(boolean);
void CL_Disconnect(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern VMapObject		*sv_mobjs[MAX_MOBJS];
extern mobj_base_t	sv_mo_base[MAX_MOBJS];
extern bool			sv_loading;
extern TMessage		sv_signon;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean 	SavingPlayers;
static FSaver	Saver;

inline byte _GET_BYTE(void) { byte B; Saver.Serialize(&B, 1); return B; }
inline word _GET_WORD(void) { word W; Saver.Serialize(&W, 2); return W; }
inline int _GET_LONG(void) { dword L; Saver.Serialize(&L, 4); return L; }
inline float _GET_FLOAT(void) { float F; Saver.Serialize(&F, 4); return F; }
#define GET_BYTE	_GET_BYTE()
#define GET_WORD	_GET_WORD()
#define GET_LONG	_GET_LONG()
#define GET_FLOAT	_GET_FLOAT()

static FName		*NameRemap;

// CODE --------------------------------------------------------------------

void FSaver::OpenRead(const char *FileName)
{
#ifdef USE_STDIO
	Handle = fopen(FileName, "rb");
#else
	Handle = Sys_FileOpenRead(FileName);
#endif
	ArIsLoading = true;
	ArIsSaving = false;
	ArIsPersistent = true;
}

void FSaver::OpenWrite(const char *FileName)
{
#ifdef USE_STDIO
	Handle = fopen(FileName, "wb");
#else
	Handle = Sys_FileOpenWrite(FileName);
#endif
	ArIsLoading = false;
	ArIsSaving = true;
	ArIsPersistent = true;
}

void FSaver::Close(void)
{
#ifdef USE_STDIO
	if (Handle)
	{
		fclose(Handle);
		Handle = NULL;
	}
#else
	if (Handle >= 0)
	{
		Sys_FileClose(Handle);
		Handle = -1;
	}
#endif
}

void FSaver::Serialize(void *V, int Length)
{
	if (ArIsLoading)
	{
#ifdef USE_STDIO
		fread(V, 1, Length, Handle);
#else
		Sys_FileRead(Handle, V, Length);
#endif
	}
	else
	{
#ifdef USE_STDIO
		fwrite(V, 1, Length, Handle);
#else
		Sys_FileWrite(Handle, V, Length);
#endif
	}
}

//==========================================================================
//
//	SV_GetSaveString
//
//==========================================================================

boolean	SV_GetSaveString(int slot, char* buf)
{
	char		fileName[MAX_OSPATH];
	FILE*		f;

	SAVE_NAME(fileName, slot);
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
}

//==========================================================================
//
//	OpenStreamOut
//
//==========================================================================

static void OpenStreamOut(char *fileName)
{
	Saver.OpenWrite(fileName);
}

//==========================================================================
//
//	StreamOutByte
//
//==========================================================================

static void StreamOutByte(byte val)
{
	Saver.Serialize(&val, sizeof(byte));
}

//==========================================================================
//
//	StreamOutWord
//
//==========================================================================

static void StreamOutWord(word val)
{
	Saver.Serialize(&val, sizeof(word));
}

//==========================================================================
//
//	StreamOutLong
//
//==========================================================================

static void StreamOutLong(int val)
{
	Saver.Serialize(&val, sizeof(int));
}

//==========================================================================
//
//	StreamOutFloat
//
//==========================================================================

static void StreamOutFloat(float val)
{
	Saver.Serialize(&val, sizeof(float));
}

//==========================================================================
//
//	StreamOutBuffer
//
//==========================================================================

static void StreamOutBuffer(const void *buffer, int size)
{
	Saver.Serialize(const_cast<void *>(buffer), size);
}

//==========================================================================
//
//	CloseStreamOut
//
//==========================================================================

static void CloseStreamOut(void)
{
	Saver.Close();
}

//==========================================================================
//
// CreateSavePath
//
//==========================================================================

void CreateSavePath(void)
{
	Sys_CreateDirectory(va("%s/saves", fl_gamedir));
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
	char slotExt[4];
	const char *curName;
	char fileName[MAX_OSPATH];

	sprintf(slotExt, "vs%d", slot);
	if (!Sys_OpenDir(va("%s/saves", fl_gamedir)))
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
			sprintf(fileName, "%s/saves/%s", fl_gamedir, curName);
			remove(fileName);
		}
	}
	Sys_CloseDir();
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
	char srcExt[4];
	char dstExt[4];
	const char *curName;
	char sourceName[MAX_OSPATH];
	char destName[MAX_OSPATH];

	sprintf(srcExt, "vs%d", sourceSlot);
	sprintf(dstExt, "vs%d", destSlot);
	if (!Sys_OpenDir(va("%s/saves", fl_gamedir)))
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
			sprintf(sourceName, "%s/saves/%s", fl_gamedir, curName);
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
}

//==========================================================================
//
//	AssertSegment
//
//==========================================================================

static void AssertSegment(gameArchiveSegment_t segType)
{
	if (GET_LONG != (int)segType)
	{
		Host_Error("Corrupt save game: Segment [%d] failed alignment check",
			segType);
	}
}

//==========================================================================
//
//	GetMobjNum
//
//==========================================================================

int GetMobjNum(VMapObject *mobj)
{
	try
	{
		if (!mobj || (mobj->bIsPlayer && !SavingPlayers))
		{
			return MOBJ_NULL;
		}
		return mobj->NetID;
	}
	catch (...)
	{
		dprintf("- GetMobjNum %p\n", mobj);
		throw;
	}
}

//==========================================================================
//
//	SetMobjPtr
//
//==========================================================================

VMapObject* SetMobjPtr(int id)
{
	if (id == MOBJ_NULL)
	{
		return NULL;
	}
	return sv_mobjs[id];
}

//==========================================================================
//
//	ArchiveNames
//
//==========================================================================

static void ArchiveNames(FArchive &Ar)
{
	StreamOutLong(FName::GetMaxNames());
	for (int i = 0; i < FName::GetMaxNames(); i++)
	{
		FNameEntry *E = FName::GetEntry(i);
		StreamOutByte(strlen(E->Name) + 1);
		Ar.Serialize(E->Name, strlen(E->Name) + 1);
	}
}

//==========================================================================
//
//	UnarchiveNames
//
//==========================================================================

static void UnarchiveNames(FArchive &Ar)
{
	int total = GET_LONG;
	NameRemap = (FName *)Z_StrMalloc(total * 4);
	for (int i = 0; i < total; i++)
	{
		int len = GET_BYTE;
		char buf[NAME_SIZE];
		Ar.Serialize(buf, len);
		NameRemap[i] = FName(buf);
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
	try
	{
		//  Get params
		int NameIndex = GET_LONG;
		VClass *Class = SV_GetClass(NameIndex);
		if (!Class)
		{
			Sys_Error("No such class %s", *NameRemap[NameIndex]);
		}

		//  Allocate object and copy data
		VObject *o = VObject::StaticSpawnObject(Class, NULL, tag);
		Saver.Serialize((byte*)o + sizeof(VObject), Class->ClassSize - sizeof(VObject));
		return o;
	}
	catch (...)
	{
		dprintf("- ReadVObject\n");
		throw;
	}
}

//==========================================================================
//
// ArchivePlayers
//
//==========================================================================

static void ArchivePlayers(void)
{
	int			i;
	player_t 	tempPlayer;
	FFunction *pf_archive_player;

	pf_archive_player = svpr.FuncForName("ArchivePlayer");
	StreamOutLong(ASEG_PLAYERS);
	for (i = 0; i < MAXPLAYERS; i++)
	{
		StreamOutByte((byte)players[i].bActive);
	}
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!players[i].bActive)
		{
			continue;
		}

		tempPlayer = players[i];
		svpr.Exec(pf_archive_player, (int)&tempPlayer);
		StreamOutBuffer(&tempPlayer, sizeof(player_t));

		for (int pi = 0; pi < NUMPSPRITES; pi++)
		{
			if (!tempPlayer.ViewEnts[pi])
			{
				continue;
			}
			WriteVObject(tempPlayer.ViewEnts[pi]);
		}
	}
}

//==========================================================================
//
// UnarchivePlayers
//
//==========================================================================

static void UnarchivePlayers(void)
{
	int		i;
	FFunction *pf_unarchive_player;

	pf_unarchive_player = svpr.FuncForName("UnarchivePlayer");
	AssertSegment(ASEG_PLAYERS);
	for (i = 0; i < MAXPLAYERS; i++)
	{
		players[i].bActive = GET_BYTE;
	}
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!players[i].bActive)
		{
			continue;
		}
		Saver.Serialize(&players[i], sizeof(player_t));
		players[i].MO = NULL; // Will be set when unarc thinker
		svpr.Exec(pf_unarchive_player, (int)&players[i]);
		players[i].bActive = false;

		for (int pi = 0; pi < NUMPSPRITES; pi++)
		{
			if (!players[i].ViewEnts[pi])
			{
				continue;
			}
			players[i].ViewEnts[pi] = (VViewEntity *)ReadVObject(PU_STRING);
			players[i].ViewEnts[pi]->Player = &players[i];
		}
	}
}

//==========================================================================
//
//	CalcSecMinMaxs
//
//==========================================================================

static void CalcSecMinMaxs(sector_t *sector)
{
	float	minz;
	float	maxz;
	int		i;

	if (sector->floor.normal.z == 1.0)
	{
		//	Horisontal floor
		sector->floor.minz = sector->floor.dist;
		sector->floor.maxz = sector->floor.dist;
	}
	else
	{
		//	Sloped floor
		minz = 99999.0;
		maxz = -99999.0;
		for (i = 0; i < sector->linecount; i++)
		{
			float z;
			z = sector->floor.GetPointZ(*sector->lines[i]->v1);
			if (minz > z)
				minz = z;
			if (maxz < z)
				maxz = z;
		}
		sector->floor.minz = minz;
		sector->floor.maxz = maxz;
	}

	if (sector->ceiling.normal.z == -1.0)
	{
		//	Horisontal ceiling
		sector->ceiling.minz = -sector->ceiling.dist;
		sector->ceiling.maxz = -sector->ceiling.dist;
	}
	else
	{
		//	Sloped ceiling
		minz = 99999.0;
		maxz = -99999.0;
		for (i = 0; i < sector->linecount; i++)
		{
			float z;
			z = sector->ceiling.GetPointZ(*sector->lines[i]->v1);
			if (minz > z)
				minz = z;
			if (maxz < z)
				maxz = z;
		}
		sector->ceiling.minz = minz;
		sector->ceiling.maxz = maxz;
	}

	sector->floorheight = sector->floor.minz;
	sector->ceilingheight = sector->ceiling.maxz;
}

//==========================================================================
//
//	Level__Serialize
//
//==========================================================================

static void Level__Serialize(FArchive &Ar)
{
	int i;
	int j;
	sector_t* sec;
	line_t* li;
	side_t* si;

	//
	//	Sectors
	//
	for (i = 0, sec = level.sectors; i < level.numsectors; i++, sec++)
	{
		Ar << sec->floor.dist
			<< sec->ceiling.dist;
		if (Ar.IsSaving())
		{
			StreamOutWord((word)sec->floor.pic);
			StreamOutWord((word)sec->ceiling.pic);
			StreamOutWord((word)sec->params.lightlevel);
			StreamOutWord((word)sec->special);
			StreamOutWord((word)sec->tag);
			StreamOutWord((word)sec->seqType);
		}
		else
		{
			sec->floor.pic = (word)GET_WORD;
			sec->ceiling.pic = GET_WORD;
			sec->params.lightlevel = GET_WORD;
			sec->special = GET_WORD;
			sec->tag = GET_WORD;
			sec->seqType = GET_WORD;
			CalcSecMinMaxs(sec);
		}
	}

	//
	//	Lines
	//
	for (i = 0, li = level.lines; i < level.numlines; i++, li++)
	{
		if (Ar.IsSaving())
		{
			StreamOutWord((word)li->flags);
			StreamOutByte((byte)li->special);
			StreamOutWord((word)li->arg1);
			StreamOutByte((byte)li->arg2);
			StreamOutByte((byte)li->arg3);
			StreamOutByte((byte)li->arg4);
			StreamOutByte((byte)li->arg5);
		}
		else
		{
			li->flags = GET_WORD;
			li->special = GET_BYTE;
			li->arg1 = GET_WORD;
			li->arg2 = GET_BYTE;
			li->arg3 = GET_BYTE;
			li->arg4 = GET_BYTE;
			li->arg5 = GET_BYTE;
		}
		for (j = 0; j < 2; j++)
		{
			if (li->sidenum[j] == -1)
			{
				continue;
			}
			si = &level.sides[li->sidenum[j]];
			if (Ar.IsSaving())
			{
				StreamOutWord((word)si->textureoffset);
				StreamOutWord((word)si->rowoffset);
				StreamOutWord((word)si->toptexture);
				StreamOutWord((word)si->bottomtexture);
				StreamOutWord((word)si->midtexture);
			}
			else
			{
				si->textureoffset = GET_WORD;
				si->rowoffset = GET_WORD;
				si->toptexture = GET_WORD;
				si->bottomtexture = GET_WORD;
				si->midtexture = GET_WORD;
			}
		}
	}

	//
	//	Polyobjs
	//
	for (i = 0; i < level.numpolyobjs; i++)
	{
		if (Ar.IsSaving())
		{
			Ar << level.polyobjs[i].angle
				<< level.polyobjs[i].startSpot.x
				<< level.polyobjs[i].startSpot.y;
		}
		else
		{
			float angle, polyX, polyY;

			Ar << angle
				<< polyX
				<< polyY;
			PO_RotatePolyobj(level.polyobjs[i].tag, angle);
			PO_MovePolyobj(level.polyobjs[i].tag, 
				polyX - level.polyobjs[i].startSpot.x, 
				polyY - level.polyobjs[i].startSpot.y);
		}
	}
}

//==========================================================================
//
//	ArchiveWorld
//
//==========================================================================

static void ArchiveWorld(void)
{
	StreamOutLong(ASEG_WORLD);

	Level__Serialize(Saver);
}

//==========================================================================
//
// UnarchiveWorld
//
//==========================================================================

static void UnarchiveWorld(void)
{
	AssertSegment(ASEG_WORLD);

	Level__Serialize(Saver);
}

//==========================================================================
//
// ArchiveThinkers
//
//==========================================================================

static void ArchiveThinkers(void)
{
	FFunction *pf_archive_thinker = svpr.FuncForName("ArchiveThinker");

	StreamOutLong(ASEG_THINKERS);

	for (TObjectIterator<VThinker> It; It; ++It)
	{
		int size = It->GetClass()->ClassSize;

		VThinker *th = (VThinker*)Z_Malloc(size);
		memcpy(th, *It, size);

		VMapObject *mobj = Cast<VMapObject>(th);
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
				mobj->Player = (player_t *)((mobj->Player - players) + 1);
			}
		}

		svpr.Exec(pf_archive_thinker, (int)th);

		StreamOutByte(1);
		WriteVObject(th);
		Z_Free(th);
	}

	//
	//  End marker
	//
	StreamOutByte(0);
}

//==========================================================================
//
//  UnarchiveThinkers
//
//==========================================================================

static void UnarchiveThinkers(void)
{
	VThinker	*thinker;

	AssertSegment(ASEG_THINKERS);

	while (GET_BYTE)
	{
		thinker = (VThinker *)ReadVObject(PU_LEVSPEC);

		//  Handle mobjs
		VMapObject *mobj = Cast<VMapObject>(thinker);
		if (mobj)
		{
			if (mobj->bIsPlayer)
			{
				mobj->Player = &players[(int)mobj->Player - 1];
				mobj->Player->MO = mobj;
			}
			mobj->SubSector = NULL;	//	Must mark as not linked
			SV_LinkToWorld(mobj);
			sv_mobjs[mobj->NetID] = mobj;
		}
	}

	//  Call unarchive function for each thinker.
	FFunction *pf_unarchive_thinker = svpr.FuncForName("UnarchiveThinker");

	for (TObjectIterator<VThinker> It; It; ++It)
	{
		svpr.Exec(pf_unarchive_thinker, (int)*It);
	}

	svpr.Exec("AfterUnarchiveThinkers");
}

//==========================================================================
//
// ArchiveScripts
//
//==========================================================================

static void ArchiveScripts(void)
{
	int i;

	StreamOutLong(ASEG_SCRIPTS);
	for (i = 0; i < ACScriptCount; i++)
	{
		StreamOutWord((word)ACSInfo[i].state);
		StreamOutWord((word)ACSInfo[i].waitValue);
	}
	StreamOutBuffer(MapVars, sizeof(MapVars));
}

//==========================================================================
//
// UnarchiveScripts
//
//==========================================================================

static void UnarchiveScripts(void)
{
	int i;

	AssertSegment(ASEG_SCRIPTS);
	for(i = 0; i < ACScriptCount; i++)
	{
		ACSInfo[i].state = (aste_t)GET_WORD;
		ACSInfo[i].waitValue = GET_WORD;
	}
	Saver.Serialize(MapVars, sizeof(MapVars));
}

//==========================================================================
//
// ArchiveSounds
//
//==========================================================================

static void ArchiveSounds(void)
{
#ifdef CLIENT
	seqnode_t *node;

	StreamOutLong(ASEG_SOUNDS);

	// Save the sound sequences
	StreamOutLong(ActiveSequences);
	for (node = SequenceListHead; node; node = node->next)
	{
		StreamOutLong(node->sequence);
		StreamOutLong(node->delayTics);
		StreamOutLong(node->volume);
		StreamOutLong(SN_GetSequenceOffset(node->sequence,
			node->sequencePtr));
		StreamOutLong(node->currentSoundID);
		StreamOutLong(node->origin_id);
		StreamOutFloat(node->origin.x);
		StreamOutFloat(node->origin.y);
		StreamOutFloat(node->origin.z);
	}
#endif
}

//==========================================================================
//
// UnarchiveSounds
//
//==========================================================================

static void UnarchiveSounds(void)
{
#ifdef CLIENT
	int i;
	int numSequences;
	int sequence;
	int delayTics;
	int volume;
	int seqOffset;
	int soundID;
	int objectNum;
	float x;
	float y;
	float z;

	AssertSegment(ASEG_SOUNDS);

	// Reload and restart all sound sequences
	numSequences = GET_LONG;
	for (i = 0; i < numSequences; i++)
	{
		sequence = GET_LONG;
		delayTics = GET_LONG;
		volume = GET_LONG;
		seqOffset = GET_LONG;
		soundID = GET_LONG;
		objectNum = GET_LONG;
		x = GET_FLOAT;
		y = GET_FLOAT;
		z = GET_FLOAT;
		SN_StartSequence(objectNum, TVec(x, y, z), sequence);
		SN_ChangeNodeData(i, seqOffset, delayTics, volume, soundID);
	}
#endif
}

//==========================================================================
//
// SV_SaveMap
//
//==========================================================================

static void SV_SaveMap(int slot, boolean savePlayers)
{
	char fileName[100];

	// Make sure we don't have any garbage
	VObject::CollectGarbage();

	SavingPlayers = savePlayers;

	CreateSavePath();

	// Open the output file
	SAVE_MAP_NAME(fileName, slot, level.mapname);
	OpenStreamOut(fileName);

	StreamOutLong(ASEG_NAMES);
	ArchiveNames(Saver);

	// Place a header marker
	StreamOutLong(ASEG_MAP_HEADER);

	// Write the level timer
	StreamOutLong(level.tictime);

	//	Write totals, because when thinkers are not spawned, they are not
	// counted
	StreamOutLong(level.totalkills);
	StreamOutLong(level.totalitems);
	StreamOutLong(level.totalsecret);

	//	Save baseline
	StreamOutLong(ASEG_BASELINE);
	StreamOutLong(sv_signon.CurSize);
	StreamOutBuffer(sv_signon.Data, sv_signon.CurSize);
	StreamOutBuffer(sv_mo_base, sizeof(sv_mo_base));

	ArchiveWorld();
	ArchiveThinkers();
	ArchiveScripts();
	ArchiveSounds();

	// Place a termination marker
	StreamOutLong(ASEG_END);

	// Close the output file
	CloseStreamOut();
}

//==========================================================================
//
//	SV_LoadMap
//
//==========================================================================

static void SV_LoadMap(char *mapname, int slot)
{
	char fileName[100];

	// Load a base level
	SV_SpawnServer(mapname, false);

	// Create the name
	SAVE_MAP_NAME(fileName, slot, mapname);

	// Load the file
	Saver.OpenRead(fileName);

	// Load names
	AssertSegment(ASEG_NAMES);
	UnarchiveNames(Saver);

	AssertSegment(ASEG_MAP_HEADER);

	// Read the level timer
	level.tictime = GET_LONG;

	level.totalkills = GET_LONG;
	level.totalitems = GET_LONG;
	level.totalsecret = GET_LONG;

	AssertSegment(ASEG_BASELINE);
	int len = GET_LONG;
	sv_signon.Clear();
	void *tmp = Z_StrMalloc(len);
	Saver.Serialize(tmp, len);
	sv_signon.Write(tmp, len);
	Saver.Serialize(sv_mo_base, sizeof(sv_mo_base));

	UnarchiveWorld();
	UnarchiveThinkers();
	UnarchiveScripts();
	UnarchiveSounds();

	AssertSegment(ASEG_END);

	// Free save buffer
	Saver.Close();

	Z_Free(NameRemap);
}

//==========================================================================
//
//	SV_SaveGame
//
//==========================================================================

void SV_SaveGame(int slot, char* description)
{
	char versionText[SAVE_VERSION_TEXT_LENGTH];
	char fileName[100];

	CreateSavePath();

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
	ArchiveNames(Saver);

	// Place a header marker
	StreamOutLong(ASEG_GAME_HEADER);

	// Write current map and difficulty
	StreamOutByte((byte)gameskill);
	StreamOutBuffer(level.mapname, 8);
 
	// Write global script info
	StreamOutBuffer(WorldVars, sizeof(WorldVars));
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
}

//==========================================================================
//
//	SV_LoadGame
//
//==========================================================================

void SV_LoadGame(int slot)
{
	char		fileName[100];
	char		mapname[12];

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
	Saver.OpenRead(fileName);

	// Set the save pointer and skip the description field
	char desc[SAVE_DESCRIPTION_LENGTH];
	Saver.Serialize(desc, SAVE_DESCRIPTION_LENGTH);

	// Check the version text
	char versionText[SAVE_VERSION_TEXT_LENGTH];
	Saver.Serialize(versionText, SAVE_VERSION_TEXT_LENGTH);
	if (strcmp(versionText, SAVE_VERSION_TEXT))
	{
		// Bad version
		Saver.Close();
		return;
	}

	// Load names
	AssertSegment(ASEG_NAMES);
	UnarchiveNames(Saver);

	AssertSegment(ASEG_GAME_HEADER);

	gameskill = (skill_t)GET_BYTE;
	Saver.Serialize(mapname, 8);
	mapname[8] = 0;

	//	Init skill hacks
	svpr.Exec("G_InitNew", gameskill);

	// Read global script info
	Saver.Serialize(WorldVars, sizeof(WorldVars));
	Saver.Serialize(ACSStore, sizeof(ACSStore));

	// Read the player structures
	UnarchivePlayers();

	AssertSegment(ASEG_END);

	Saver.Close();

	Z_Free(NameRemap);

	sv_loading = true;

	// Load the current map
	SV_LoadMap(mapname, BASE_SLOT);

#ifdef CLIENT
	if (cls.state != ca_dedicated)
		CmdBuf << "Connect local\n";
#endif
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

	SAVE_NAME(fileName, REBORN_SLOT);
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

	SAVE_MAP_NAME(fileName, BASE_SLOT, mapname);
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
	if (Argc() != 3)
	{
		return;
	}

	if (netgame)
	{
		con << "Can't save in net game\n";
		return;
	}

	if (!sv.active)
	{
		con << "you can't save if you aren't playing!";
		return;
	}

	if (sv.intermission)
	{
		return;
	}

	if (strlen(Argv(2)) >= 32)
	{
		con << "Description too long\n";
		return;
	}

	Draw_SaveIcon();

	SV_SaveGame(atoi(Argv(1)), Argv(2));

	con << "GAME SAVED\n";
} 
 
//==========================================================================
//
//	COMMAND Load
//
//==========================================================================

COMMAND(Load)
{
	if (Argc() != 2)
	{
		return;
	}
	if (netgame)
	{
		con << "Can't load in net game\n";
		return;
	}

	int slot = atoi(Argv(1));
	char	desc[32];
	if (!SV_GetSaveString(slot, desc))
	{
		con << "Empty slot\n";
		return;
	}
	con << "Loading \"" << desc << "\"\n";

	Draw_LoadIcon();
	SV_LoadGame(slot);
	if (!netgame)
	{
		// Copy the base slot to the reborn slot
		SV_UpdateRebornSlot();
	}
}

#endif

//**************************************************************************
//
//	$Log$
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
