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
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define SCRIPT_CONTINUE 0
#define SCRIPT_STOP 1
#define SCRIPT_TERMINATE 2
#define PRINT_BUFFER_SIZE 256
#define GAME_SINGLE_PLAYER 0
#define GAME_NET_COOPERATIVE 1
#define GAME_NET_DEATHMATCH 2
#define TEXTURE_TOP 0
#define TEXTURE_MIDDLE 1
#define TEXTURE_BOTTOM 2
#define ACS_STACK_DEPTH		4096

// TYPES -------------------------------------------------------------------

enum EACSFormat
{
	ACS_Old,
	ACS_Enhanced,
	ACS_LittleEnhanced,
	ACS_Unknown
};

//	Script flags.
enum
{
	SCRIPTF_Net = 0x0001	//	Safe to "puke" in multiplayer.
};

enum EPCD
{
	PCD_Nop,
	PCD_Terminate,
	PCD_Suspend,
	PCD_PushNumber,
	PCD_LSpec1,
	PCD_LSpec2,
	PCD_LSpec3,
	PCD_LSpec4,
	PCD_LSpec5,
	PCD_LSpec1Direct,
	PCD_LSpec2Direct,//10
	PCD_LSpec3Direct,
	PCD_LSpec4Direct,
	PCD_LSpec5Direct,
	PCD_Add,
	PCD_Subtract,
	PCD_Multiply,
	PCD_Divide,
	PCD_Modulus,
	PCD_EQ,
	PCD_NE,//20
	PCD_LT,
	PCD_GT,
	PCD_LE,
	PCD_GE,
	PCD_AssignScriptVar,
	PCD_AssignMapVar,
	PCD_AssignWorldVar,
	PCD_PushScriptVar,
	PCD_PushMapVar,
	PCD_PushWorldVar,//30
	PCD_AddScriptVar,
	PCD_AddMapVar,
	PCD_AddWorldVar,
	PCD_SubScriptVar,
	PCD_SubMapVar,
	PCD_SubWorldVar,
	PCD_MulScriptVar,
	PCD_MulMapVar,
	PCD_MulWorldVar,
	PCD_DivScriptVar,//40
	PCD_DivMapVar,
	PCD_DivWorldVar,
	PCD_ModScriptVar,
	PCD_ModMapVar,
	PCD_ModWorldVar,
	PCD_IncScriptVar,
	PCD_IncMapVar,
	PCD_IncWorldVar,
	PCD_DecScriptVar,
	PCD_DecMapVar,//50
	PCD_DecWorldVar,
	PCD_Goto,
	PCD_IfGoto,
	PCD_Drop,
	PCD_Delay,
	PCD_DelayDirect,
	PCD_Random,
	PCD_RandomDirect,
	PCD_ThingCount,
	PCD_ThingCountDirect,//60
	PCD_TagWait,
	PCD_TagWaitDirect,
	PCD_PolyWait,
	PCD_PolyWaitDirect,
	PCD_ChangeFloor,
	PCD_ChangeFloorDirect,
	PCD_ChangeCeiling,
	PCD_ChangeCeilingDirect,
	PCD_Restart,
	PCD_AndLogical,//70
	PCD_OrLogical,
	PCD_AndBitwise,
	PCD_OrBitwise,
	PCD_EorBitwise,
	PCD_NegateLogical,
	PCD_LShift,
	PCD_RShift,
	PCD_UnaryMinus,
	PCD_IfNotGoto,
	PCD_LineSide,//80
	PCD_ScriptWait,
	PCD_ScriptWaitDirect,
	PCD_ClearLineSpecial,
	PCD_CaseGoto,
	PCD_BeginPrint,
	PCD_EndPrint,
	PCD_PrintString,
	PCD_PrintNumber,
	PCD_PrintCharacter,
	PCD_PlayerCount,//90
	PCD_GameType,
	PCD_GameSkill,
	PCD_Timer,
	PCD_SectorSound,
	PCD_AmbientSound,
	PCD_SoundSequence,
	PCD_SetLineTexture,
	PCD_SetLineBlocking,
	PCD_SetLineSpecial,
	PCD_ThingSound,//100
	PCD_EndPrintBold,
	PCD_ActivatorSound,// Start of the extended opcodes.
	PCD_LocalAmbientSound,
	PCD_SetLineMonsterBlocking,
	PCD_PlayerBlueSkull,	// Start of new [Skull Tag] pcodes
	PCD_PlayerRedSkull,
	PCD_PlayerYellowSkull,
	PCD_PlayerMasterSkull,
	PCD_PlayerBlueCard,
	PCD_PlayerRedCard,//110
	PCD_PlayerYellowCard,
	PCD_PlayerMasterCard,
	PCD_PlayerBlackSkull,
	PCD_PlayerSilverSkull,
	PCD_PlayerGoldSkull,
	PCD_PlayerBlavkCard,
	PCD_PlayerSilverCard,
	PCD_PlayerOnTeam,
	PCD_PlayerTeam,
	PCD_PlayerHealth,//120
	PCD_PlayerArmorPoints,
	PCD_PlayerFrags,
	PCD_PlayerExpert,
	PCD_BlueTeamCount,
	PCD_RedTeamCount,
	PCD_BlueTeamScore,
	PCD_RedTeamScore,
	PCD_IsOneFlagCTF,
	PCD_LSpec6,				// These are never used. They should probably
	PCD_LSpec6Direct,//130	// be given names like PCD_Dummy.
	PCD_PrintName,
	PCD_MusicChange,
	PCD_Team2FragPoints,
	PCD_ConsoleCommand,
	PCD_SinglePlayer,		// [RH] End of Skull Tag p-codes
	PCD_FixedMul,
	PCD_FixedDiv,
	PCD_SetGravity,
	PCD_SetGravityDirect,
	PCD_SetAirControl,//140
	PCD_SetAirControlDirect,
	PCD_ClearInventory,
	PCD_GiveInventory,
	PCD_GiveInventoryDirect,
	PCD_TakeInventory,
	PCD_TakeInventoryDirect,
	PCD_CheckInventory,
	PCD_CheckInventoryDirect,
	PCD_Spawn,
	PCD_SpawnDirect,//150
	PCD_SpawnSpot,
	PCD_SpawnSpotDirect,
	PCD_SetMusic,
	PCD_SetMusicDirect,
	PCD_LocalSetMusic,
	PCD_LocalSetMusicDirect,
	PCD_PrintFixed,
	PCD_PrintLocalized,
	PCD_MoreHudMessage,
	PCD_OptHudMessage,//160
	PCD_EndHudMessage,
	PCD_EndHudMessageBold,
	PCD_SetStyle,
	PCD_SetStyleDirect,
	PCD_SetFont,
	PCD_SetFontDirect,
	PCD_PushByte,
	PCD_LSpec1DirectB,
	PCD_LSpec2DirectB,
	PCD_LSpec3DirectB,//170
	PCD_LSpec4DirectB,
	PCD_LSpec5DirectB,
	PCD_DelayDirectB,
	PCD_RandomDirectB,
	PCD_PushBytes,
	PCD_Push2Bytes,
	PCD_Push3Bytes,
	PCD_Push4Bytes,
	PCD_Push5Bytes,
	PCD_SetThingSpecial,//180
	PCD_AssignGlobalVar,
	PCD_PushGlobalVar,
	PCD_AddGlobalVar,
	PCD_SubGlobalVar,
	PCD_MulGlobalVar,
	PCD_DivGlobalVar,
	PCD_ModGlobalVar,
	PCD_IncGlobalVar,
	PCD_DecGlobalVar,
	PCD_FadeTo,//190
	PCD_FadeRange,
	PCD_CancelFade,
	PCD_PlayMovie,
	PCD_SetFloorTrigger,
	PCD_SetCeilingTrigger,
	PCD_GetActorX,
	PCD_GetActorY,
	PCD_GetActorZ,
	PCD_StartTranslation,
	PCD_TranslationRange1,//200
	PCD_TranslationRange2,
	PCD_EndTranslation,
	PCD_Call,
	PCD_CallDiscard,
	PCD_ReturnVoid,
	PCD_ReturnVal,
	PCD_PushMapArray,
	PCD_AssignMapArray,
	PCD_AddMapArray,
	PCD_SubMapArray,//210
	PCD_MulMapArray,
	PCD_DivMapArray,
	PCD_ModMapArray,
	PCD_IncMapArray,
	PCD_DecMapArray,
	PCD_Dup,
	PCD_Swap,
	PCD_WriteToIni,
	PCD_GetFromIni,
	PCD_Sin,//220
	PCD_Cos,
	PCD_VectorAngle,
	PCD_CheckWeapon,
	PCD_SetWeapon,
	PCD_TagString,
	PCD_PushWorldArray,
	PCD_AssignWorldArray,
	PCD_AddWorldArray,
	PCD_SubWorldArray,
	PCD_MulWorldArray,//230
	PCD_DivWorldArray,
	PCD_ModWorldArray,
	PCD_IncWorldArray,
	PCD_DecWorldArray,
	PCD_PushGlobalArray,
	PCD_AssignGlobalArray,
	PCD_AddGlobalArray,
	PCD_SubGlobalArray,
	PCD_MulGlobalArray,
	PCD_DivGlobalArray,//240
	PCD_ModGlobalArray,
	PCD_IncGlobalArray,
	PCD_DecGlobalArray,
	PCD_SetMarineWeapon,
	PCD_SetActorProperty,
	PCD_GetActorProperty,
	PCD_PlayerNumber,
	PCD_ActivatorTID,
	PCD_SetMarineSprite,
	PCD_GetScreenWidth,//250
	PCD_GetScreenHeight,
	PCD_ThingProjectile2,
	PCD_StrLen,
	PCD_SetHudSize,
	PCD_GetCvar,
	PCD_CaseGotoSorted,
	PCD_SetResultValue,
	PCD_GetLineRowOffset,
	PCD_GetActorFloorZ,
	PCD_GetActorAngle,//260
	PCD_GetSectorFloorZ,
	PCD_GetSectorCeilingZ,
	PCD_LSpec5Result,
	PCD_GetSigilPieces,

	PCODE_COMMAND_COUNT
};

enum aste_t
{
	ASTE_INACTIVE,
	ASTE_RUNNING,
	ASTE_SUSPENDED,
	ASTE_WAITINGFORTAG,
	ASTE_WAITINGFORPOLY,
	ASTE_WAITINGFORSCRIPT,
	ASTE_TERMINATING
};

struct acsHeader_t
{
	char	marker[4];
	int		infoOffset;
	int		code;
};

struct acsInfo_t
{
	word	number;
	byte	type;
	byte	argCount;
	int 	*address;
	word	Flags;
	word	VarCount;
	aste_t 	state;
	int 	waitValue;
};

struct FACScriptFunction
{
	byte	ArgCount;
	byte	LocalCount;
	byte	HasReturnValue;
	byte	ImportNum;
	dword	Address;
};

//
//	A action code scripts object module - level's BEHAVIOR lump or library.
//
class FACScriptsObject
{
private:
	struct FArrayInfo
	{
		int		Size;
		int*	Data;
	};

	EACSFormat			Format;

	int					LumpNum;
	int					LibraryID;

	int					DataSize;
	byte*				Data;

	byte*				Chunks;

	int					NumScripts;
	acsInfo_t*			Scripts;

	FACScriptFunction*	Functions;
	int					NumFunctions;

	int					NumStrings;
	char**				Strings;

	int					MapVarStore[MAX_ACS_MAP_VARS];

	int					NumArrays;
	FArrayInfo*			ArrayStore;
	int					NumTotalArrays;
	FArrayInfo**		Arrays;

	TArray<FACScriptsObject*>	Imports;

	static TArray<FACScriptsObject*>	LoadedObjects;

	void LoadOldObject();
	void LoadEnhancedObject();
	void UnencryptStrings();
	int FindFunctionName(const char* Name) const;
	int FindMapVarName(const char* Name) const;
	int FindMapArray(const char* Name) const;
	int FindStringInChunk(byte* Chunk, const char* Name) const;
	byte* FindChunk(const char* id) const;
	byte* NextChunk(byte* prev) const;
	void Serialise(FArchive& Ar);
	void StartTypedACScripts(int Type);
	void TagFinished(int tag);
	void PolyobjFinished(int po);
	void ScriptFinished(int number);

public:
	int*				MapVars[MAX_ACS_MAP_VARS];

	FACScriptsObject(int Lump);

	int* OffsetToPtr(int Offs);
	int PtrToOffset(int* Ptr);
	EACSFormat GetFormat() const
	{
		return Format;
	}
	int GetNumScripts() const
	{
		return NumScripts;
	}
	acsInfo_t& GetScriptInfo(int i)
	{
		return Scripts[i];
	}
	const char* GetString(int i) const
	{
		return Strings[i];
	}
	int GetLibraryID() const
	{
		return LibraryID;
	}
	acsInfo_t* FindScript(int Number) const;
	FACScriptFunction* GetFunction(int funcnum, FACScriptsObject*& Object);
	int GetArrayVal(int ArrayIdx, int Index);
	void SetArrayVal(int ArrayIdx, int Index, int Value);

	static FACScriptsObject* StaticLoadObject(int Lump);
	static void StaticUnloadObjects();
	static acsInfo_t* StaticFindScript(int Number, FACScriptsObject*& Object);
	static const char* StaticGetString(int Index);
	static FACScriptsObject* StaticGetObject(int Index);
	static void StaticStartTypedACScripts(int Type);
	static void StaticSerialise(FArchive& Ar);
	static void StaticTagFinished(int tag);
	static void StaticPolyobjFinished(int po);
	static void StaticScriptFinished(int number);
};

struct CallReturn
{
	int					ReturnAddress;
	FACScriptFunction*	ReturnFunction;
	FACScriptsObject*	ReturnObject;
	byte				bDiscardResult;
	byte				Pad[3];
};

class VACS : public VThinker
{
	DECLARE_CLASS(VACS, VThinker, 0)
	NO_DEFAULT_CONSTRUCTOR(VACS)

	VEntity*			Activator;
	line_t*				line;
	int 				side;
	int 				number;
	acsInfo_t*			info;
	float				DelayTime;
	int*				LocalVars;
	int*				ip;
	FACScriptsObject*	activeObject;

	int RunScript(float DeltaTime);
	void Tick(float DeltaTime);

	DECLARE_FUNCTION(Archive)
	DECLARE_FUNCTION(Unarchive)
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static bool TagBusy(int tag);
static bool AddToACSStore(const char* map, int number, int arg1, int arg2,
	int arg3);

static int FindSectorFromTag(int tag, int start);
static void GiveInventory(VEntity* Activator, const char* Type, int Amount);
static void TakeInventory(VEntity* Activator, const char* Type, int Amount);
static int CheckInventory(VEntity* Activator, const char* Type);
static void strbin(char *str);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int WorldVars[MAX_ACS_WORLD_VARS];
int GlobalVars[MAX_ACS_GLOBAL_VARS];
FACSGrowingArray WorldArrays[MAX_ACS_WORLD_VARS];
FACSGrowingArray GlobalArrays[MAX_ACS_GLOBAL_VARS];
acsstore_t ACSStore[MAX_ACS_STORE+1]; // +1 for termination marker

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, ACS)

TArray<FACScriptsObject*>	FACScriptsObject::LoadedObjects;
static int 					stack[ACS_STACK_DEPTH];
static int					stackPtr;

static FFunction*			pf_TagBusy;

// CODE --------------------------------------------------------------------

static VACS* SpawnScript(acsInfo_t* Info, FACScriptsObject* Object,
	VEntity* Activator, line_t* Line, int Side, int Arg1, int Arg2, int Arg3,
	bool Delayed)
{
	VACS* script = (VACS*)VObject::StaticSpawnObject(VACS::StaticClass(),
		NULL, PU_LEVSPEC);
	script->info = Info;
	script->number = Info->number;
	script->ip = Info->address;
	script->activeObject = Object;
	script->Activator = Activator;
	script->line = Line;
	script->side = Side;
	script->LocalVars = (int*)Z_Malloc(Info->VarCount * 4, PU_LEVSPEC, 0);
	script->LocalVars[0] = Arg1;
	script->LocalVars[1] = Arg2;
	script->LocalVars[2] = Arg3;
	memset(script->LocalVars + Info->argCount, 0,
		(Info->VarCount - Info->argCount) * 4);
	if (Delayed)
	{
		//	World objects are allotted 1 second for initialization.
		script->DelayTime = 1.0;
	}
	Info->state = ASTE_RUNNING;
	return script;
}

static boolean P_ExecuteLineSpecial(int special, int *args, line_t *line, int side,
	VEntity *mo)
{
   	return svpr.Exec("ExecuteLineSpecial",
   		special, (int)args, (int)line, side, (int)mo);
}

static line_t *P_FindLine(int lineTag, int *searchPosition)
{
	return (line_t*)svpr.Exec("P_FindLine", lineTag, (int)searchPosition);
}

static VEntity *P_FindMobjFromTID(int tid, int *searchPosition)
{
	return (VEntity*)svpr.Exec("FindMobjFromTID", tid, (int)searchPosition);
}

static int ThingCount(int type, int tid)
{
	return svpr.Exec("ThingCount", type, tid);
}

static VEntity* EntityFromTID(int TID, VEntity* Default)
{
	if (!TID)
	{
		return Default;
	}
	else
	{
		int search = -1;
		return P_FindMobjFromTID(TID, &search);
	}
}

//==========================================================================
//
//	FACScriptsObject::FACScriptsObject
//
//==========================================================================

FACScriptsObject::FACScriptsObject(int Lump)
{
	guard(FACScriptsObject::FACScriptsObject);
	Format = ACS_Unknown;
	LumpNum = Lump;
	LibraryID = 0;
	DataSize = 0;
	Data = NULL;
	Chunks = NULL;
	NumScripts = 0;
	Scripts = NULL;
	NumFunctions = 0;
	Functions = NULL;
	NumStrings = 0;
	Strings = NULL;
	NumArrays = 0;
	ArrayStore = NULL;
	NumTotalArrays = 0;
	Arrays = NULL;
	memset(MapVarStore, 0, sizeof(MapVarStore));

	if (Lump < 0)
    {
		return;
    }
	if (W_LumpLength(Lump) < (int)sizeof(acsHeader_t))
    {
		GCon->Log("Behavior lump too small");
		return;
    }

	Data = (byte*)W_CacheLumpNum(Lump, PU_LEVEL);
	acsHeader_t* header = (acsHeader_t*)Data;

	//	Check header.
	if (header->marker[0] != 'A' || header->marker[1] != 'C' ||
		header->marker[2] != 'S')
	{
		return;
	}
	//	Determine format.
	switch (header->marker[3])
	{
	case 0:
		Format = ACS_Old;
		break;
	case 'E':
		Format = ACS_Enhanced;
		break;
	case 'e':
		Format = ACS_LittleEnhanced;
		break;
	default:
		return;
	}

	DataSize = W_LumpLength(Lump);

	if (Format == ACS_Old)
	{
		dword dirofs = LittleLong(header->infoOffset);
		byte* pretag = Data + dirofs - 4;

		Chunks = Data + DataSize;
		//	Check for redesigned ACSE/ACSe
		if (dirofs >= 6 * 4 && pretag[0] == 'A' &&
			pretag[1] == 'C' && pretag[2] == 'S' &&
			(pretag[3] == 'e' || pretag[3] == 'E'))
		{
			Format = (pretag[3] == 'e') ? ACS_LittleEnhanced : ACS_Enhanced;
			Chunks = Data + LittleLong(*(int*)(Data + dirofs - 8));
			//	Forget about the compatibility cruft at the end of the lump
			DataSize = dirofs - 8;
		}
	}
	else
	{
		Chunks = Data + LittleLong(header->infoOffset);
	}

	if (Format == ACS_Old)
	{
		LoadOldObject();
	}
	else
	{
		LoadEnhancedObject();
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::LoadOldObject
//
//==========================================================================

void FACScriptsObject::LoadOldObject()
{
	guard(FACScriptsObject::LoadOldObject);
	int i;
	int *buffer;
	acsInfo_t *info;
	acsHeader_t *header;

	header = (acsHeader_t*)Data;

	//	Load script info.
	buffer = (int*)(Data + LittleLong(header->infoOffset));
	NumScripts = LittleLong(*buffer++);
	if (NumScripts == 0)
	{
		//	Empty behavior lump
		return;
	}
	Scripts = (acsInfo_t*)Z_Malloc(NumScripts * sizeof(acsInfo_t), PU_LEVEL, 0);
	memset(Scripts, 0, NumScripts * sizeof(acsInfo_t));
	for (i = 0, info = Scripts; i < NumScripts; i++, info++)
	{
		info->number = LittleLong(*buffer) % 1000;
		info->type = LittleLong(*buffer) / 1000;
		buffer++;
		info->address = OffsetToPtr(LittleLong(*buffer++));
		info->argCount = LittleLong(*buffer++);
		info->Flags = 0;
		info->VarCount = MAX_ACS_SCRIPT_VARS;
		info->state = ASTE_INACTIVE;
	}

	//	Load strings.
	NumStrings = LittleLong(*buffer++);
	Strings = (char**)buffer;
	for (i = 0; i < NumStrings; i++)
	{
		Strings[i] = (char*)Data + LittleLong((int)Strings[i]);
	}

	//	Set up map vars.
	for (i = 0; i < MAX_ACS_MAP_VARS; i++)
	{
		MapVars[i] = &MapVarStore[i];
	}

	//	Add to loaded objects.
	LibraryID = LoadedObjects.AddItem(this) << 16;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::LoadEnhancedObject
//
//==========================================================================

void FACScriptsObject::LoadEnhancedObject()
{
	guard(FACScriptsObject::LoadEnhancedObject);
	int i;
	int *buffer;
	acsInfo_t *info;

	//	Load scripts.
	buffer = (int*)FindChunk("SPTR");
	if (Data[3] != 0)
	{
		NumScripts = LittleLong(buffer[1]) / 12;
		Scripts = (acsInfo_t*)Z_Malloc(NumScripts * sizeof(acsInfo_t), PU_LEVEL, 0);
		memset(Scripts, 0, NumScripts * sizeof(acsInfo_t));
		buffer += 2;

		for (i = 0, info = Scripts; i < NumScripts; i++, info++)
		{
			info->number = LittleShort(*(short*)buffer);
			info->type = LittleShort(((short*)buffer)[1]);
			buffer++;
			info->address = OffsetToPtr(LittleLong(*buffer++));
			info->argCount = LittleLong(*buffer++);
			info->Flags = 0;
			info->VarCount = MAX_ACS_SCRIPT_VARS;
			info->state = ASTE_INACTIVE;
		}
	}
	else
	{
		NumScripts = LittleLong(buffer[1]) / 8;
		Scripts = (acsInfo_t*)Z_Malloc(NumScripts * sizeof(acsInfo_t), PU_LEVEL, 0);
		memset(Scripts, 0, NumScripts * sizeof(acsInfo_t));
		buffer += 2;

		for (i = 0, info = Scripts; i < NumScripts; i++, info++)
		{
			info->number = LittleShort(*(short*)buffer);
			info->type = ((byte*)buffer)[2];
			info->argCount = ((byte*)buffer)[3];
			buffer++;
			info->address = OffsetToPtr(LittleLong(*buffer++));
			info->Flags = 0;
			info->VarCount = MAX_ACS_SCRIPT_VARS;
			info->state = ASTE_INACTIVE;
		}
	}

	//	Load script flags.
	buffer = (int*)FindChunk("SFLG");
	if (buffer)
	{
		int count = LittleLong(buffer[1]) / 4;
		buffer += 2;
		for (int i = 0; i < count; i++, buffer++)
		{
			acsInfo_t* info = FindScript(LittleShort(((word*)buffer)[0]));
			if (info)
			{
				info->Flags = LittleShort(((word*)buffer)[1]);
			}
		}
	}

	//	Load script var counts
	buffer = (int*)FindChunk("SVCT");
	if (buffer)
	{
		int count = LittleLong(buffer[1]) / 4;
		buffer += 2;
		for (i = 0; i < count; i++, buffer++)
		{
			acsInfo_t* info = FindScript(LittleShort(((word*)buffer)[0]));
			if (info)
			{
				info->VarCount = LittleShort(((word*)buffer)[1]);
				//	Make sure it's at least 3 so in SpawnScript we can safely
				// assign args to first 3 variables.
				if (info->VarCount < 3)
					info->VarCount = 3;
			}
		}
	}

	//	Load functions.
	buffer = (int*)FindChunk("FUNC");
	if (buffer)
	{
		NumFunctions = LittleLong(buffer[1]) / 8;
		Functions = (FACScriptFunction*)(buffer + 2);
		for (i = 0; i < NumFunctions; i++)
			Functions[i].Address = LittleLong(Functions[i].Address);
	}

	//	Unencrypt strings.
	UnencryptStrings();

	//	A temporary hack.
	buffer = (int*)FindChunk("STRL");
	if (buffer)
	{
		buffer += 2;
		NumStrings = LittleLong(buffer[1]);
		Strings = (char**)(buffer + 3);
		for(i = 0; i < NumStrings; i++)
		{
			Strings[i] = (char*)buffer + LittleLong((int)Strings[i]);
		}
	}

	//	Initialize this object's map variable pointers to defaults. They can
	// be changed later once the imported modules are loaded.
	for (i = 0; i < MAX_ACS_MAP_VARS; i++)
	{
		MapVars[i] = &MapVarStore[i];
	}

	//	Initialize this object's map variables.
	memset(MapVarStore, 0, sizeof(MapVarStore));
	buffer = (int*)FindChunk("MINI");
	while (buffer)
	{
		int numvars = LittleLong(buffer[1]) / 4 - 1;
		int firstvar = LittleLong(buffer[2]);
		for (i = 0; i < numvars; i++)
		{
			MapVarStore[firstvar + i] = LittleLong(buffer[3 + i]);
		}
		buffer = (int*)NextChunk((byte*)buffer);
	}

	//	Create arrays.
	buffer = (int*)FindChunk("ARAY");
	if (buffer)
	{
		NumArrays = LittleLong(buffer[1]) / 8;
		ArrayStore = Z_New<FArrayInfo>(NumArrays);
		memset(ArrayStore, 0, sizeof(*ArrayStore) * NumArrays);
		for (i = 0; i < NumArrays; ++i)
		{
			MapVarStore[LittleLong(buffer[2 + i * 2])] = i;
			ArrayStore[i].Size = LittleLong(buffer[3 + i * 2]);
			ArrayStore[i].Data = Z_New<int>(ArrayStore[i].Size);
			memset(ArrayStore[i].Data, 0, ArrayStore[i].Size * sizeof(int));
		}
	}

	//	Initialize arrays.
	buffer = (int*)FindChunk("AINI");
	while (buffer)
	{
		int arraynum = MapVarStore[LittleLong(buffer[2])];
		if ((unsigned)arraynum < (unsigned)NumArrays)
		{
			int initsize = (LittleLong(buffer[1]) - 4) / 4;
			if (initsize > ArrayStore[arraynum].Size)
				initsize = ArrayStore[arraynum].Size;
			int *elems = ArrayStore[arraynum].Data;
			for (i = 0; i < initsize; i++)
			{
				elems[i] = LittleLong(buffer[3 + i]);
			}
		}
		buffer = (int*)NextChunk((byte*)buffer);
	}

	//	Start setting up array pointers.
	NumTotalArrays = NumArrays;
	buffer = (int*)FindChunk("AIMP");
	if (buffer)
	{
		NumTotalArrays += LittleLong(buffer[2]);
	}
	if (NumTotalArrays)
	{
		Arrays = Z_New<FArrayInfo*>(NumTotalArrays);
		for (i = 0; i < NumArrays; ++i)
		{
			Arrays[i] = &ArrayStore[i];
		}
	}

	//	Now that everything is set up, record this object as being among
	// the loaded objects. We need to do this before resolving any imports,
	// because an import might (indirectly) need to resolve exports in this
	// module. The only things that can be exported are functions and map
	// variables, which must already be present if they're exported, so this
	// is okay.
	LibraryID = LoadedObjects.AddItem(this) << 16;

	//	Tag the library ID to any map variables that are initialized with
	// strings.
	if (LibraryID)
	{
		buffer = (int*)FindChunk("MSTR");
		if (buffer)
		{
			for (i = 0; i < LittleLong(buffer[1]) / 4; i++)
			{
				MapVarStore[LittleLong(buffer[i + 2])] |= LibraryID;
			}
		}

		buffer = (int*)FindChunk("ASTR");
		if (buffer)
		{
			for (i = 0; i < LittleLong(buffer[1]) / 4; i++)
			{
				int arraynum = MapVarStore[LittleLong(buffer[i + 2])];
				if ((unsigned)arraynum < (unsigned)NumArrays)
				{
					int *elems = ArrayStore[arraynum].Data;
					for (int j = ArrayStore[arraynum].Size; j > 0; j--, elems++)
					{
						*elems |= LibraryID;
					}
				}
			}
		}
	}

	//	Library loading.
	buffer = (int*)FindChunk("LOAD");
	if (buffer)
	{
		const char* const parse = (char*)&buffer[2];
		for (i = 0; i < LittleLong(buffer[1]); i++)
		{
			if (parse[i])
			{
				FACScriptsObject* Object = NULL;
				int Lump = W_CheckNumForName(&parse[i], WADNS_ACSLibrary);
				if (Lump < 0)
				{
					GCon->Logf("Could not find ACS library %s.", &parse[i]);
				}
				else
				{
					Object = StaticLoadObject(Lump);
				}
				Imports.AddItem(Object);
				do ; while (parse[++i]);
			}
		}

		//	Go through each imported object in order and resolve all
		// imported functions and map variables.
		for (i = 0; i < Imports.Num(); i++)
		{
			FACScriptsObject* lib = Imports[i];
			int j;

			if (!lib)
				continue;

			// Resolve functions
			buffer = (int*)FindChunk("FNAM");
			for (j = 0; j < NumFunctions; j++)
			{
				FACScriptFunction *func = &Functions[j];
				if (func->Address != 0 || func->ImportNum != 0)
					continue;

				int libfunc = lib->FindFunctionName((char*)(buffer + 2) +
					LittleLong(buffer[3 + j]));
				if (libfunc < 0)
					continue;

				FACScriptFunction* realfunc = &lib->Functions[libfunc];
				//	Make sure that the library really defines this
				// function. It might simply be importing it itself.
				if (realfunc->Address == 0 || realfunc->ImportNum != 0)
					continue;

				func->Address = libfunc;
				func->ImportNum = i + 1;
				if (realfunc->ArgCount != func->ArgCount)
				{
					GCon->Logf("Function %s in %s has %d arguments. "
						"%s expects it to have %d.",
						(char *)(buffer + 2) + LittleLong(buffer[3 + j]),
						W_LumpName(lib->LumpNum), realfunc->ArgCount,
						W_LumpName(LumpNum), func->ArgCount);
					Format = ACS_Unknown;
				}
				//	The next two properties do not effect code compatibility,
				// so it is okay for them to be different in the imported
				// module than they are in this one, as long as we make sure
				// to use the real values.
				func->LocalCount = realfunc->LocalCount;
				func->HasReturnValue = realfunc->HasReturnValue;
			}

			//	Resolve map variables.
			buffer = (int*)FindChunk("MIMP");
			if (buffer)
			{
				char* parse = (char*)&buffer[2];
				for (j = 0; j < LittleLong(buffer[1]); j++)
				{
					int varNum = LittleLong(*(int*)&parse[j]);
					j += 4;
					int impNum = lib->FindMapVarName(&parse[j]);
					if (impNum >= 0)
					{
						MapVars[varNum] = &lib->MapVarStore[impNum];
					}
					do ; while (parse[++j]);
				}
			}

			// Resolve arrays
			if (NumTotalArrays > NumArrays)
			{
				buffer = (int*)FindChunk("AIMP");
				char* parse = (char*)&buffer[3];
				for (j = 0; j < LittleLong(buffer[2]); j++)
				{
					int varNum = LittleLong(*(int*)parse);
					parse += 4;
					int expectedSize = LittleLong(*(int*)parse);
					parse += 4;
					int impNum = lib->FindMapArray(parse);
					if (impNum >= 0)
					{
						Arrays[NumArrays + j] = &lib->ArrayStore[impNum];
						MapVarStore[varNum] = NumArrays + j;
						if (lib->ArrayStore[impNum].Size != expectedSize)
						{
							Format = ACS_Unknown;
							GCon->Logf("The array %s in %s has %ld elements, "
								"but %s expects it to only have %ld.",
								parse, W_LumpName(lib->LumpNum),
								lib->ArrayStore[impNum].Size,
								W_LumpName(LumpNum), expectedSize);
						}
					}
					do ; while (*++parse);
					++parse;
				}
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::UnencryptStrings
//
//==========================================================================

void FACScriptsObject::UnencryptStrings()
{
	guard(FACScriptsObject::UnencryptStrings);
	byte *prevchunk = NULL;
	dword *chunk = (dword*)FindChunk("STRE");
	while (chunk)
	{
		for (int strnum = 0; strnum < LittleLong(chunk[3]); strnum++)
		{
			int ofs = LittleLong(chunk[5 + strnum]);
			byte* data = (byte*)chunk + ofs + 8;
			byte last;
			int p = (byte)(ofs * 157135);
			int i = 0;
			do
			{
				last = (data[i] ^= (byte)(p + (i >> 1)));
				i++;
			} while (last != 0);
		}
		prevchunk = (byte*)chunk;
		chunk = (dword*)NextChunk((byte*)chunk);
		prevchunk[3] = 'L';
	}
	if (prevchunk)
	{
		prevchunk[3] = 'L';
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::FindFunctionName
//
//==========================================================================

int FACScriptsObject::FindFunctionName(const char* Name) const
{
	guard(FACScriptsObject::FindFunctionName);
	return FindStringInChunk(FindChunk("FNAM"), Name);
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::FindMapVarName
//
//==========================================================================

int FACScriptsObject::FindMapVarName(const char* Name) const
{
	guard(FACScriptsObject::FindMapVarName);
	return FindStringInChunk(FindChunk("MEXP"), Name);
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::FindMapArray
//
//==========================================================================

int FACScriptsObject::FindMapArray(const char* Name) const
{
	guard(FACScriptsObject::FindMapArray);
	int var = FindMapVarName(Name);
	if (var >= 0)
	{
		return MapVarStore[var];
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::FindStringInChunk
//
//==========================================================================

int FACScriptsObject::FindStringInChunk(byte* Chunk, const char* Name) const
{
	guard(FACScriptsObject::FindStringInChunk);
	if (Chunk)
	{
		int count = LittleLong(((int*)Chunk)[2]);
		for (int i = 0; i < count; ++i)
		{
			if (!stricmp(Name, (char*)(Chunk + 8) +
				LittleLong(((int*)Chunk)[3 + i])))
			{
				return i;
			}
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::FindChunk
//
//==========================================================================

byte* FACScriptsObject::FindChunk(const char* id) const
{
	guard(FACScriptsObject::FindChunk);
	byte* chunk = Chunks;
	while (chunk && chunk < Data + DataSize)
	{
		if (*(int*)chunk == *(int*)id)
		{
			return chunk;
		}
		chunk = chunk + LittleLong(((int*)chunk)[1]) + 8;
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::NextChunk
//
//==========================================================================

byte* FACScriptsObject::NextChunk(byte* prev) const
{
	guard(FACScriptsObject::NextChunk);
	int id = *(int*)prev;
	byte* chunk = prev + LittleLong(((int*)prev)[1]) + 8;
	while (chunk && chunk < Data + DataSize)
	{
		if (*(int*)chunk == id)
		{
			return chunk;
		}
		chunk = chunk + LittleLong(((int*)chunk)[1]) + 8;
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::Serialise
//
//==========================================================================

void FACScriptsObject::Serialise(FArchive& Ar)
{
	guard(FACScriptsObject::Serialise);
	for (int i = 0; i < NumScripts; i++)
	{
		Ar << *(int*)&Scripts[i].state;
		Ar << Scripts[i].waitValue;
	}
	Ar.Serialise(MapVarStore, sizeof(MapVarStore));
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::OffsetToPtr
//
//==========================================================================

int* FACScriptsObject::OffsetToPtr(int Offs)
{
	if (Offs < 0 || Offs >= DataSize)
		Host_Error("Bad offset in ACS file");
	return (int*)(Data + Offs);
}

//==========================================================================
//
//	FACScriptsObject::PtrToOffset
//
//==========================================================================

int FACScriptsObject::PtrToOffset(int* Ptr)
{
	return (byte*)Ptr - Data;
}

//==========================================================================
//
//	FACScriptsObject::FindScript
//
//==========================================================================

acsInfo_t* FACScriptsObject::FindScript(int Number) const
{
	guard(FACScriptsObject::FindScript);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].number == Number)
		{
			return Scripts + i;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::GetFunction
//
//==========================================================================

FACScriptFunction* FACScriptsObject::GetFunction(int funcnum,
	FACScriptsObject*& Object)
{
	guard(FACScriptsObject::GetFunction);
	if ((unsigned)funcnum >= (unsigned)NumFunctions)
	{
		return NULL;
	}
	FACScriptFunction* Func = Functions + funcnum;
	if (Func->ImportNum)
	{
		return Imports[Func->ImportNum - 1]->GetFunction(Func->Address,
			Object);
	}
	Object = this;
	return Func;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::GetArrayVal
//
//==========================================================================

int FACScriptsObject::GetArrayVal(int ArrayIdx, int Index)
{
	guard(FACScriptsObject::GetArrayVal);
	if ((unsigned)ArrayIdx >= (unsigned)NumTotalArrays)
		return 0;
	if ((unsigned)Index >= (unsigned)Arrays[ArrayIdx]->Size)
		return 0;
	return Arrays[ArrayIdx]->Data[Index];
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::SetArrayVal
//
//==========================================================================

void FACScriptsObject::SetArrayVal(int ArrayIdx, int Index, int Value)
{
	guard(FACScriptsObject::SetArrayVal);
	if ((unsigned)ArrayIdx >= (unsigned)NumTotalArrays)
		return;
	if ((unsigned)Index >= (unsigned)Arrays[ArrayIdx]->Size)
		return;
	Arrays[ArrayIdx]->Data[Index] = Value;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StartTypedACScripts
//
//==========================================================================

void FACScriptsObject::StartTypedACScripts(int Type)
{
	guard(FACScriptsObject::StartTypedACScripts);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].type == Type)
		{
			// Auto-activate
			VACS* script = SpawnScript(&Scripts[i], this, NULL, NULL, 0,
				0, 0, 0, true);
		}
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticLoadObject
//
//==========================================================================

FACScriptsObject* FACScriptsObject::StaticLoadObject(int Lump)
{
	guard(FACScriptsObject::StaticLoadObject);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		if (LoadedObjects[i]->LumpNum == Lump)
		{
			return LoadedObjects[i];
		}
	}
	return new FACScriptsObject(Lump);
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticUnloadObjects
//
//==========================================================================

void FACScriptsObject::StaticUnloadObjects()
{
	guard(FACScriptsObject::StaticUnloadObjects);
	for (int i = 0; i < LoadedObjects.Num(); i++)
		delete LoadedObjects[i];
	LoadedObjects.Empty();
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticFindScript
//
//==========================================================================

acsInfo_t* FACScriptsObject::StaticFindScript(int Number, FACScriptsObject*& Object)
{
	guard(FACScriptsObject::StaticFindScript);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		acsInfo_t* Found = LoadedObjects[i]->FindScript(Number);
		if (Found)
		{
			Object = LoadedObjects[i];
			return Found;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticGetString
//
//==========================================================================

const char* FACScriptsObject::StaticGetString(int Index)
{
	guard(FACScriptsObject::StaticGetString);
	int ObjIdx = Index >> 16;
	if (ObjIdx >= LoadedObjects.Num())
	{
		return NULL;
	}
	return LoadedObjects[ObjIdx]->GetString(Index & 0xffff);
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticGetObject
//
//==========================================================================

FACScriptsObject* FACScriptsObject::StaticGetObject(int Index)
{
	guard(FACScriptsObject::StaticGetObject);
	if ((unsigned)Index >= (unsigned)LoadedObjects.Num())
	{
		return NULL;
	}
	return LoadedObjects[Index];
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticStartTypedACScripts
//
//==========================================================================

void FACScriptsObject::StaticStartTypedACScripts(int Type)
{
	guard(FACScriptsObject::StaticStartTypedACScripts);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->StartTypedACScripts(Type);
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticSerialise
//
//==========================================================================

void FACScriptsObject::StaticSerialise(FArchive& Ar)
{
	guard(FACScriptsObject::StaticSerialise);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->Serialise(Ar);
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::TagFinished
//
//==========================================================================

void FACScriptsObject::TagFinished(int tag)
{
	guard(FACScriptsObject::TagFinished);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].state == ASTE_WAITINGFORTAG &&
			Scripts[i].waitValue == tag)
		{
			Scripts[i].state = ASTE_RUNNING;
		}
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticTagFinished
//
//==========================================================================

void FACScriptsObject::StaticTagFinished(int tag)
{
	guard(FACScriptsObject::StaticTagFinished);
	if (TagBusy(tag))
	{
		return;
	}
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->TagFinished(tag);
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::PolyobjFinished
//
//==========================================================================

void FACScriptsObject::PolyobjFinished(int po)
{
	guard(FACScriptsObject::PolyobjFinished);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].state == ASTE_WAITINGFORPOLY &&
			Scripts[i].waitValue == po)
		{
			Scripts[i].state = ASTE_RUNNING;
		}
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticPolyobjFinished
//
//==========================================================================

void FACScriptsObject::StaticPolyobjFinished(int po)
{
	guard(FACScriptsObject::StaticPolyobjFinished);
	if (PO_Busy(po))
	{
		return;
	}
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->PolyobjFinished(po);
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::ScriptFinished
//
//==========================================================================

void FACScriptsObject::ScriptFinished(int number)
{
	guard(FACScriptsObject::ScriptFinished);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].state == ASTE_WAITINGFORSCRIPT &&
			Scripts[i].waitValue == number)
		{
			Scripts[i].state = ASTE_RUNNING;
		}
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticScriptFinished
//
//==========================================================================

void FACScriptsObject::StaticScriptFinished(int number)
{
	guard(FACScriptsObject::StaticScriptFinished);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->ScriptFinished(number);
	}
	unguard;
}

//==========================================================================
//
//	FACSGrowingArray::Redim
//
//==========================================================================

void FACSGrowingArray::Redim(int NewSize)
{
	guard(FACSGrowingArray::Redim);
	if (!NewSize && Data)
	{
		Z_Free(Data);
		Data = NULL;
	}
	else if (NewSize && !Data)
	{
		Data = Z_New<int>(NewSize);
	}
	else if (NewSize && Data)
	{
		Z_Resize((void**)&Data, NewSize * sizeof(int));
	}
	//	Clear newly allocated elements.
	if (NewSize > Size)
	{
		memset(Data + Size, 0, (NewSize - Size) * sizeof(int));
	}
	Size = NewSize;
	unguard;
}

//==========================================================================
//
//	FACSGrowingArray::SetElemVal
//
//==========================================================================

void FACSGrowingArray::SetElemVal(int Index, int Value)
{
	guard(FACSGrowingArray::SetElemVal);
	if (Index >= Size)
	{
		Redim(Index + 1);
	}
	Data[Index] = Value;
	unguard;
}

//==========================================================================
//
//	FACSGrowingArray::GetElemVal
//
//==========================================================================

int FACSGrowingArray::GetElemVal(int Index)
{
	guard(FACSGrowingArray::GetElemVal);
	if ((unsigned)Index >= (unsigned)Size)
		return 0;
	return Data[Index];
	unguard;
}

//==========================================================================
//
//	FACSGrowingArray::Serialise
//
//==========================================================================

void FACSGrowingArray::Serialise(FArchive& Ar)
{
	guard(FACSGrowingArray::Serialise);
	if (Ar.IsSaving())
	{
		Ar << Size;
	}
	else
	{
		int NewSize;
		Ar << NewSize;
		Redim(NewSize);
	}
	for (int i = 0; i < Size; i++)
	{
		Ar << Data[i];
	}
	unguard;
}

//==========================================================================
//
// P_LoadACScripts
//
//==========================================================================

void P_LoadACScripts(int Lump)
{
	guard(P_LoadACScripts);
	pf_TagBusy = svpr.FuncForName("TagBusy");

	FACScriptsObject::StaticUnloadObjects();

	if (Lump < 0)
	{
		return;
	}

	FACScriptsObject::StaticLoadObject(Lump);
	unguard;
}

//==========================================================================
//
//	P_StartTypedACScripts
//
//==========================================================================

void P_StartTypedACScripts(int Type)
{
	FACScriptsObject::StaticStartTypedACScripts(Type);
}

//==========================================================================
//
// P_CheckACSStore
//
// Scans the ACS store and executes all scripts belonging to the current
// map.
//
//==========================================================================

void P_CheckACSStore(void)
{
	guard(P_CheckACSStore);
	acsstore_t *store;

	for (store = ACSStore; store->map[0] != 0; store++)
	{
		if (!strcmp(store->map, level.mapname))
		{
			FACScriptsObject* Object;
			acsInfo_t* info = FACScriptsObject::StaticFindScript(store->script, Object);
			if (info)
			{
				if (info->state == ASTE_SUSPENDED)
				{
					//	Resume a suspended script
					info->state = ASTE_RUNNING;
				}
				else if (info->state == ASTE_INACTIVE)
				{
					SpawnScript(info, Object, NULL, NULL, 0, store->args[0],
						store->args[1], store->args[2], true);
				}
			}
			else
			{
				//	Script not found.
				GCon->Logf(NAME_Dev, "P_CheckACSStore: Unknown script %d",
					store->script);
			}
			strcpy(store->map, "-");
		}
	}
	unguard;
}

//==========================================================================
//
//	P_StartACS
//
//==========================================================================

bool P_StartACS(int number, int map_num, int arg1, int arg2, int arg3,
	VEntity *activator, line_t *line, int side, bool Always, bool WantResult)
{
	guard(P_StartACS);
	char map[12] = "";
	FACScriptsObject* Object;

	if (map_num)
	{
		strcpy(map, SV_GetMapName(map_num));
	}

	if (map[0] && strcmp(map, level.mapname))
	{
		// Add to the script store
		return AddToACSStore(map, number, arg1, arg2, arg3);
	}
	acsInfo_t* info = FACScriptsObject::StaticFindScript(number, Object);
	if (!info)
	{
		//	Script not found
		GCon->Logf(NAME_Dev, "P_StartACS ERROR: Unknown script %d", number);
		return false;
	}
	if (!Always)
	{
		if (info->state == ASTE_SUSPENDED)
		{
			//	Resume a suspended script
			info->state = ASTE_RUNNING;
			return true;
		}
		if (info->state != ASTE_INACTIVE)
		{
			//	Script is already executing
			return false;
		}
	}
	VACS* script = SpawnScript(info, Object, activator, line, side, arg1,
		arg2, arg3, false);
	if (WantResult)
	{
		return script->RunScript(host_frametime);
	}
	return true;
	unguard;
}

//==========================================================================
//
// AddToACSStore
//
//==========================================================================

static bool AddToACSStore(const char *map, int number, int arg1, int arg2,
	int arg3)
{
	int i;
	int index;

	index = -1;
	for (i = 0; ACSStore[i].map[0]; i++)
	{
		if (ACSStore[i].script == number && !strcmp(ACSStore[i].map, map))
		{
			// Don't allow duplicates
			return false;
		}
		if (index == -1 && ACSStore[i].map[0] == '-')
		{
			// Remember first empty slot
			index = i;
		}
	}
	if (index == -1)
	{
		// Append required
		if (i == MAX_ACS_STORE)
		{
			Sys_Error("AddToACSStore: MAX_ACS_STORE (%d) exceeded.",
				MAX_ACS_STORE);
		}
		index = i;
		ACSStore[index + 1].map[0] = 0;
	}
	strcpy(ACSStore[index].map, map);
	ACSStore[index].script = number;
	ACSStore[index].args[0] = arg1;
	ACSStore[index].args[1] = arg2;
	ACSStore[index].args[2] = arg3;
	return true;
}

//==========================================================================
//
// P_TerminateACS
//
//==========================================================================

boolean P_TerminateACS(int number, int)
{
	guard(P_TerminateACS);
	acsInfo_t* info;
	FACScriptsObject* Object;

	info = FACScriptsObject::StaticFindScript(number, Object);
	if (!info)
	{
		//	Script not found
		return false;
	}
	if (info->state == ASTE_INACTIVE || info->state == ASTE_TERMINATING)
	{
		//	States that disallow termination
		return false;
	}
	info->state = ASTE_TERMINATING;
	return true;
	unguard;
}

//==========================================================================
//
// P_SuspendACS
//
//==========================================================================

boolean P_SuspendACS(int number, int)
{
	guard(P_SuspendACS);
	acsInfo_t* info;
	FACScriptsObject* Object;

	info = FACScriptsObject::StaticFindScript(number, Object);
	if (!info)
	{
		//	Script not found.
		return false;
	}
	if (info->state == ASTE_INACTIVE || info->state == ASTE_SUSPENDED ||
		info->state == ASTE_TERMINATING)
	{
		// States that disallow suspension
		return false;
	}
	info->state = ASTE_SUSPENDED;
	return true;
	unguard;
}

//==========================================================================
//
//	P_ACSInitNewGame
//
//==========================================================================

void P_ACSInitNewGame()
{
	guard(P_ACSInitNewGame);
	memset(WorldVars, 0, sizeof(WorldVars));
	memset(GlobalVars, 0, sizeof(GlobalVars));
	memset(ACSStore, 0, sizeof(ACSStore));
	unguard;
}

//==========================================================================
//
//	P_SerialiseScripts
//
//==========================================================================

void P_SerialiseScripts(FArchive& Ar)
{
	FACScriptsObject::StaticSerialise(Ar);
}

//==========================================================================
//
//	VAcs::Tick
//
//==========================================================================

void VACS::Tick(float DeltaTime)
{
	RunScript(DeltaTime);
}

//==========================================================================
//
//	VACS::RunScript
//
//==========================================================================

inline int getbyte(int*& pc)
{
	int res = *(byte*)pc;
	pc = (int*)((byte*)pc + 1);
	return res;
}

int VACS::RunScript(float DeltaTime)
{
	int cmd;
	int action;
	int SpecArgs[8];
	char PrintBuffer[PRINT_BUFFER_SIZE];
	int resultValue = 1;

	if (info->state == ASTE_TERMINATING)
	{
		info->state = ASTE_INACTIVE;
		FACScriptsObject::StaticScriptFinished(number);
		ConditionalDestroy();
		Z_Free(LocalVars);
		return resultValue;
	}
	if (info->state != ASTE_RUNNING)
	{
		return resultValue;
	}
	if (DelayTime)
	{
		DelayTime -= DeltaTime;
		if (DelayTime < 0)
			DelayTime = 0;
		return resultValue;
	}
	int optstart = -1;
	int* locals = LocalVars;
	FACScriptFunction* activeFunction = NULL;
	EACSFormat fmt = activeObject->GetFormat();
	int* PCodePtr = ip;
	action = SCRIPT_CONTINUE;
	do
	{
		//	Push	stack[stackPtr++] = 
		//	Pop		stack[--stackPtr]
		//	Top		stack[stackPtr - 1]
		//	Drop	stackPtr--

#define PC_GET_INT	(*PCodePtr++)
#define NEXTBYTE	(fmt == ACS_LittleEnhanced ? getbyte(PCodePtr) : PC_GET_INT)

		if (fmt == ACS_LittleEnhanced)
		{
			cmd = getbyte(PCodePtr);
			if (cmd >= 256 - 16)
			{
				cmd = (256 - 16) + ((cmd - (256 - 16)) << 8) + getbyte(PCodePtr);
			}
		}
		else
		{
			cmd = PC_GET_INT;
		}
		switch (cmd)
		{
		//	Standard P-Code commands.
		case PCD_Nop:
			break;

		case PCD_Terminate:
			action = SCRIPT_TERMINATE;
			break;

		case PCD_Suspend:
			info->state = ASTE_SUSPENDED;
			action = SCRIPT_STOP;
			break;

		case PCD_PushNumber:
			stack[stackPtr++] = PC_GET_INT;
			break;

		case PCD_LSpec1:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[0] = stack[--stackPtr];
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec2:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[1] = stack[stackPtr - 1];
				SpecArgs[0] = stack[stackPtr - 2];
				stackPtr -= 2;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec3:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[2] = stack[stackPtr - 1];
				SpecArgs[1] = stack[stackPtr - 2];
				SpecArgs[0] = stack[stackPtr - 3];
				stackPtr -= 3;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec4:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[3] = stack[stackPtr - 1];
				SpecArgs[2] = stack[stackPtr - 2];
				SpecArgs[1] = stack[stackPtr - 3];
				SpecArgs[0] = stack[stackPtr - 4];
				stackPtr -= 4;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec5:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[4] = stack[stackPtr - 1];
				SpecArgs[3] = stack[stackPtr - 2];
				SpecArgs[2] = stack[stackPtr - 3];
				SpecArgs[1] = stack[stackPtr - 4];
				SpecArgs[0] = stack[stackPtr - 5];
				stackPtr -= 5;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec1Direct:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[0] = PC_GET_INT;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec2Direct:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[0] = PC_GET_INT;
				SpecArgs[1] = PC_GET_INT;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec3Direct:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[0] = PC_GET_INT;
				SpecArgs[1] = PC_GET_INT;
				SpecArgs[2] = PC_GET_INT;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec4Direct:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[0] = PC_GET_INT;
				SpecArgs[1] = PC_GET_INT;
				SpecArgs[2] = PC_GET_INT;
				SpecArgs[3] = PC_GET_INT;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec5Direct:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[0] = PC_GET_INT;
				SpecArgs[1] = PC_GET_INT;
				SpecArgs[2] = PC_GET_INT;
				SpecArgs[3] = PC_GET_INT;
				SpecArgs[4] = PC_GET_INT;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_Add:
			stack[stackPtr - 2] = stack[stackPtr - 2] + stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_Subtract:
			stack[stackPtr - 2] = stack[stackPtr - 2] - stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_Multiply:
			stack[stackPtr - 2] = stack[stackPtr - 2] * stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_Divide:
			stack[stackPtr - 2] = stack[stackPtr - 2] / stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_Modulus:
			stack[stackPtr - 2] = stack[stackPtr - 2] % stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_EQ:
			stack[stackPtr - 2] = stack[stackPtr - 2] == stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_NE:
			stack[stackPtr - 2] = stack[stackPtr - 2] != stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_LT:
			stack[stackPtr - 2] = stack[stackPtr - 2] < stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_GT:
			stack[stackPtr - 2] = stack[stackPtr - 2] > stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_LE:
			stack[stackPtr - 2] = stack[stackPtr - 2] <= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_GE:
			stack[stackPtr - 2] = stack[stackPtr - 2] >= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_AssignScriptVar:
			locals[NEXTBYTE] = stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_AssignMapVar:
			*activeObject->MapVars[NEXTBYTE] = stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_AssignWorldVar:
			WorldVars[NEXTBYTE] = stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_PushScriptVar:
			stack[stackPtr++] = locals[NEXTBYTE];
			break;

		case PCD_PushMapVar:
			stack[stackPtr++] = *activeObject->MapVars[NEXTBYTE];
			break;

		case PCD_PushWorldVar:
			stack[stackPtr++] = WorldVars[NEXTBYTE];
			break;

		case PCD_AddScriptVar:
			locals[NEXTBYTE] += stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_AddMapVar:
			*activeObject->MapVars[NEXTBYTE] += stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_AddWorldVar:
			WorldVars[NEXTBYTE] += stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_SubScriptVar:
			locals[NEXTBYTE] -= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_SubMapVar:
			*activeObject->MapVars[NEXTBYTE] -= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_SubWorldVar:
			WorldVars[NEXTBYTE] -= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_MulScriptVar:
			locals[NEXTBYTE] *= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_MulMapVar:
			*activeObject->MapVars[NEXTBYTE] *= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_MulWorldVar:
			WorldVars[NEXTBYTE] *= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_DivScriptVar:
			locals[NEXTBYTE] /= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_DivMapVar:
			*activeObject->MapVars[NEXTBYTE] /= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_DivWorldVar:
			WorldVars[NEXTBYTE] /= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_ModScriptVar:
			locals[NEXTBYTE] %= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_ModMapVar:
			*activeObject->MapVars[NEXTBYTE] %= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_ModWorldVar:
			WorldVars[NEXTBYTE] %= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_IncScriptVar:
			locals[NEXTBYTE]++;
			break;

		case PCD_IncMapVar:
			*activeObject->MapVars[NEXTBYTE]++;
			break;

		case PCD_IncWorldVar:
			WorldVars[NEXTBYTE]++;
			break;

		case PCD_DecScriptVar:
			locals[NEXTBYTE]--;
			break;

		case PCD_DecMapVar:
			*activeObject->MapVars[NEXTBYTE]--;
			break;

		case PCD_DecWorldVar:
			WorldVars[NEXTBYTE]--;
			break;

		case PCD_Goto:
			PCodePtr = activeObject->OffsetToPtr(*PCodePtr);
			break;

		case PCD_IfGoto:
			if (stack[--stackPtr])
			{
				PCodePtr = activeObject->OffsetToPtr(*PCodePtr);
			}
			else
			{
				PCodePtr++;
			}
			break;

		case PCD_Drop:
			stackPtr--;
			break;

		case PCD_Delay:
			DelayTime = float(stack[stackPtr - 1]) / 35.0;
			stackPtr--;
			action = SCRIPT_STOP;
			break;

		case PCD_DelayDirect:
			DelayTime = float(PC_GET_INT) / 35.0;
			action = SCRIPT_STOP;
			break;

		case PCD_Random:
			{
				int low;
				int high;
			
				high = stack[stackPtr - 1];
				low = stack[stackPtr - 2];
				stack[stackPtr - 2] = low + (int)(Random() * (high - low + 1));
				stackPtr--;
			}
			break;

		case PCD_RandomDirect:
			{
				int low;
				int high;
			
				low = PC_GET_INT;
				high = PC_GET_INT;
				stack[stackPtr++] = low + (int)(Random() * (high - low + 1));
			}
			break;

		case PCD_ThingCount:
			stack[stackPtr - 2] = ThingCount(stack[stackPtr - 2], stack[stackPtr - 1]);
			stackPtr--;
			break;

		case PCD_ThingCountDirect:
			{
				int type;
			
				type = PC_GET_INT;
				stack[stackPtr++] = ThingCount(type, PC_GET_INT);
			}
			break;

		case PCD_TagWait:
			info->waitValue = stack[stackPtr - 1];
			info->state = ASTE_WAITINGFORTAG;
			stackPtr--;
			action = SCRIPT_STOP;
			break;

		case PCD_TagWaitDirect:
			info->waitValue = PC_GET_INT;
			info->state = ASTE_WAITINGFORTAG;
			action = SCRIPT_STOP;
			break;

		case PCD_PolyWait:
			info->waitValue = stack[stackPtr - 1];
			info->state = ASTE_WAITINGFORPOLY;
			stackPtr--;
			action = SCRIPT_STOP;
			break;

		case PCD_PolyWaitDirect:
			info->waitValue = PC_GET_INT;
			info->state = ASTE_WAITINGFORPOLY;
			action = SCRIPT_STOP;
			break;

		case PCD_ChangeFloor:
			{
				int tag;
				int flat;
				int sectorIndex;

				flat = R_FlatNumForName(FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
				tag = stack[stackPtr - 2];
				stackPtr -= 2;
				sectorIndex = -1;
				while ((sectorIndex = FindSectorFromTag(tag, sectorIndex)) >= 0)
				{
					SV_SetFloorPic(sectorIndex, flat);
				}
			}
			break;

		case PCD_ChangeFloorDirect:
			{
				int tag;
				int flat;
				int sectorIndex;
			
				tag = PC_GET_INT;
				flat = R_FlatNumForName(FACScriptsObject::StaticGetString(PC_GET_INT));
				sectorIndex = -1;
				while ((sectorIndex = FindSectorFromTag(tag, sectorIndex)) >= 0)
				{
					SV_SetFloorPic(sectorIndex, flat);
				}
			}
			break;

		case PCD_ChangeCeiling:
			{
				int tag;
				int flat;
				int sectorIndex;
			
				flat = R_FlatNumForName(FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
				tag = stack[stackPtr - 2];
				stackPtr -= 2;
				sectorIndex = -1;
				while ((sectorIndex = FindSectorFromTag(tag, sectorIndex)) >= 0)
				{
					SV_SetCeilPic(sectorIndex, flat);
				}
			}
			break;

		case PCD_ChangeCeilingDirect:
			{
				int tag;
				int flat;
				int sectorIndex;
			
				tag = PC_GET_INT;
				flat = R_FlatNumForName(FACScriptsObject::StaticGetString(PC_GET_INT));
				sectorIndex = -1;
				while ((sectorIndex = FindSectorFromTag(tag, sectorIndex)) >= 0)
				{
					SV_SetCeilPic(sectorIndex, flat);
				}
			}
			break;

		case PCD_Restart:
			PCodePtr = info->address;
			break;

		case PCD_AndLogical:
			stack[stackPtr - 2] = stack[stackPtr - 2] && stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_OrLogical:
			stack[stackPtr - 2] = stack[stackPtr - 2] || stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_AndBitwise:
			stack[stackPtr - 2] = stack[stackPtr - 2] & stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_OrBitwise:
			stack[stackPtr - 2] = stack[stackPtr - 2] | stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_EorBitwise:
			stack[stackPtr - 2] = stack[stackPtr - 2] ^ stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_NegateLogical:
			stack[stackPtr - 1] = !stack[stackPtr - 1];
			break;

		case PCD_LShift:
			stack[stackPtr - 2] = stack[stackPtr - 2] << stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_RShift:
			stack[stackPtr - 2] = stack[stackPtr - 2] >> stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_UnaryMinus:
			stack[stackPtr - 1] = -stack[stackPtr - 1];
			break;

		case PCD_IfNotGoto:
			if (stack[stackPtr - 1])
			{
				PCodePtr++;
			}
			else
			{
				PCodePtr = activeObject->OffsetToPtr(*PCodePtr);
			}
			stackPtr--;
			break;

		case PCD_LineSide:
			stack[stackPtr++] = side;
			break;

		case PCD_ScriptWait:
			info->waitValue = stack[stackPtr - 1];
			info->state = ASTE_WAITINGFORSCRIPT;
			stackPtr--;
			action = SCRIPT_STOP;
			break;

		case PCD_ScriptWaitDirect:
			info->waitValue = PC_GET_INT;
			info->state = ASTE_WAITINGFORSCRIPT;
			action = SCRIPT_STOP;
			break;

		case PCD_ClearLineSpecial:
			if (line)
			{
				line->special = 0;
			}
			break;

		case PCD_CaseGoto:
			if (stack[stackPtr - 1] == PC_GET_INT)
			{
				PCodePtr = activeObject->OffsetToPtr(*PCodePtr);
				stackPtr--;
			}
			else
			{
				PCodePtr++;
			}
			break;

		case PCD_BeginPrint:
			*PrintBuffer = 0;
			break;

		case PCD_EndPrint:
			strbin(PrintBuffer);
			if (Activator && Activator->bIsPlayer)
			{
				SV_ClientCenterPrintf(Activator->Player, "%s\n", PrintBuffer);
			}
			else
			{
				for (int i = 0; i < MAXPLAYERS; i++)
				{
					if (svvars.Players[i])
					{
						SV_ClientCenterPrintf(svvars.Players[i], "%s\n", PrintBuffer);
					}
				}
			}
			break;

		case PCD_PrintString:
			strcat(PrintBuffer, FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
			stackPtr--;
			break;

		case PCD_PrintNumber:
			strcat(PrintBuffer, va("%d", stack[stackPtr - 1]));
			stackPtr--;
			break;

		case PCD_PrintCharacter:
			{
				char *bufferEnd;
			
				bufferEnd = PrintBuffer + strlen(PrintBuffer);
				*bufferEnd++ = stack[stackPtr - 1];
				*bufferEnd = 0;
				stackPtr--;
			}
			break;

		case PCD_PlayerCount:
			{
				int i;
				int count;
			
				count = 0;
				for(i = 0; i < MAXPLAYERS; i++)
				{
					if (svvars.Players[i])
						count++;
				}
				stack[stackPtr++] = count;
			}
			break;

		case PCD_GameType:
			{
				int gametype;
			
				if (netgame == false)
				{
					gametype = GAME_SINGLE_PLAYER;
				}
				else if (deathmatch)
				{
					gametype = GAME_NET_DEATHMATCH;
				}
				else
				{
					gametype = GAME_NET_COOPERATIVE;
				}
				stack[stackPtr++] = gametype;
			}
			break;

		case PCD_GameSkill:
			stack[stackPtr++] = gameskill;
			break;

		case PCD_Timer:
			stack[stackPtr++] = level.tictime;
			break;

		case PCD_SectorSound:
			{
				int volume;
				sector_t *sector;
			
				sector = NULL;
				if (line)
				{
					sector = line->frontsector;
				}
				volume = stack[stackPtr - 1];
				stackPtr--;
				SV_SectorStartSound(sector,
					S_GetSoundID(FACScriptsObject::StaticGetString(stack[--stackPtr])), 0, volume);
			}
			break;

		case PCD_AmbientSound:
			{
				int volume;
			
				volume = stack[stackPtr - 1];
				stackPtr--;
				SV_StartSound(NULL, S_GetSoundID(
					FACScriptsObject::StaticGetString(stack[--stackPtr])), 0, volume);
			}
			break;

		case PCD_SoundSequence:
			{
				sector_t *sec;
			
				sec = NULL;
				if (line)
				{
					sec = line->frontsector;
				}
				SV_SectorStartSequence(sec, FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
				stackPtr--;
			}
			break;

		case PCD_SetLineTexture:
			{
				line_t *line;
				int lineTag;
				int side;
				int position;
				int texture;
				int searcher;
			
				texture = R_TextureNumForName(FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
				position = stack[stackPtr - 2];
				side = stack[stackPtr - 3];
				lineTag = stack[stackPtr - 4];
				stackPtr -= 4;
				searcher = -1;
				while ((line = P_FindLine(lineTag, &searcher)) != NULL)
				{
					SV_SetLineTexture(line->sidenum[side], position, texture);
				}
			}
			break;

		case PCD_SetLineBlocking:
			{
				line_t *line;
				int lineTag;
				int blocking;
				int searcher;
			
				blocking = stack[stackPtr - 1] ? ML_BLOCKING : 0;
				lineTag = stack[stackPtr - 2];
				stackPtr -= 2;
				searcher = -1;
				while ((line = P_FindLine(lineTag, &searcher)) != NULL)
				{
					line->flags = (line->flags & ~ML_BLOCKING) | blocking;
				}
			}
			break;

		case PCD_SetLineSpecial:
			{
				line_t *line;
				int lineTag;
				int special, arg1, arg2, arg3, arg4, arg5;
				int searcher;
			
				arg5 = stack[stackPtr - 1];
				arg4 = stack[stackPtr - 2];
				arg3 = stack[stackPtr - 3];
				arg2 = stack[stackPtr - 4];
				arg1 = stack[stackPtr - 5];
				special = stack[stackPtr - 6];
				lineTag = stack[stackPtr - 7];
				stackPtr -= 7;
				searcher = -1;
				while ((line = P_FindLine(lineTag, &searcher)) != NULL)
				{
					line->special = special;
					line->arg1 = arg1;
					line->arg2 = arg2;
					line->arg3 = arg3;
					line->arg4 = arg4;
					line->arg5 = arg5;
				}
			}
			break;

		case PCD_ThingSound:
			{
				int tid;
				int sound;
				int volume;
				VEntity *mobj;
				int searcher;
			
				volume = stack[stackPtr - 1];
				sound = S_GetSoundID(FACScriptsObject::StaticGetString(stack[stackPtr - 2]));
				tid = stack[stackPtr - 3];
				stackPtr -= 3;
				searcher = -1;
				while ((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
				{
					SV_StartSound(mobj, sound, 0, volume);
				}
			}
			break;

		case PCD_EndPrintBold:
			strbin(PrintBuffer);
			{
				//FIXME yellow message
				for (int i = 0; i < MAXPLAYERS; i++)
				{
					if (svvars.Players[i])
					{
						SV_ClientCenterPrintf(svvars.Players[i], "%s\n", PrintBuffer);
					}
				}
			}
			break;

		case PCD_ActivatorSound:
			{
				int sound;

				sound = S_GetSoundID(FACScriptsObject::StaticGetString(stack[stackPtr - 2]));
				SV_StartSound(Activator, sound, 0, stack[stackPtr - 1]);
				stackPtr -= 2;
			}
			break;

		case PCD_LocalAmbientSound:
			{
				int sound;

				sound = S_GetSoundID(FACScriptsObject::StaticGetString(stack[stackPtr - 2]));
				SV_StartLocalSound(Activator, sound, 0, stack[stackPtr - 1]);
				stackPtr -= 2;
			}
			break;

		case PCD_SetLineMonsterBlocking:
			{
				line_t *line;
				int lineTag;
				int blocking;
				int searcher;
			
				blocking = stack[stackPtr - 1] ? ML_BLOCKMONSTERS : 0;
				lineTag = stack[stackPtr - 2];
				stackPtr -= 2;
				searcher = -1;
				while ((line = P_FindLine(lineTag, &searcher)) != NULL)
				{
					line->flags = (line->flags & ~ML_BLOCKING) | blocking;
				}
			}
			break;

		case PCD_PlayerHealth:
			if (Activator)
				stack[stackPtr++] = Activator->Health;
			else
				stack[stackPtr++] = 0;
			break;

		case PCD_PlayerArmorPoints:
			if (Activator && Activator->Player)
				//FIXME
				stack[stackPtr++] = 0;
			else
				stack[stackPtr++] = 0;
			break;

		case PCD_PlayerFrags:
			if (Activator && Activator->Player)
				stack[stackPtr++] = Activator->Player->Frags;
			else
				stack[stackPtr++] = 0;
			break;

		case PCD_PrintName:
			{
				VBasePlayer* player = NULL;

				if (stack[stackPtr - 1] == 0 ||
					(unsigned)stack[stackPtr - 1] > MAXPLAYERS)
				{
					if (Activator)
					{
						player = Activator->Player;
					}
				}
				else
				{
					player = svvars.Players[stack[stackPtr - 1] - 1];
				}
				if (player && player->bSpawned)
				{
					strcat(PrintBuffer, player->PlayerName);
				}
				else if (player && !player->bSpawned)
				{
					strcat(PrintBuffer, va("Player %d", stack[stackPtr - 1]));
				}
				else if (Activator)
				{
					strcat(PrintBuffer, Activator->GetClass()->GetName());
				}
				else
				{
					strcat(PrintBuffer, " ");
				}
				stackPtr--;
			}
			break;

		case PCD_MusicChange:
			//FIXME send over network. Also needs to be saved in savegame.
			S_StartSong(FACScriptsObject::StaticGetString(stack[stackPtr - 2]), stack[stackPtr - 1], true);
			stackPtr -= 2;
			break;

		case PCD_SinglePlayer:
			stack[stackPtr++] = !netgame;
			break;

		case PCD_FixedMul:
			stack[stackPtr - 2] = int((double(stack[stackPtr - 2]) /
				double(0x10000)) * double(stack[stackPtr - 1]));
			stackPtr--;
			break;

		case PCD_FixedDiv:
			stack[stackPtr - 2] = int((double(stack[stackPtr - 2]) /
				double(stack[stackPtr - 1])) * double(0x10000));
			stackPtr--;
			break;

		case PCD_SetGravity:
			//FIXME
			{float grav = float(stack[stackPtr - 1]) / float(0x10000);}
			stackPtr--;
			break;

		case PCD_SetGravityDirect:
			//FIXME
			{float grav = float(PC_GET_INT) / float(0x10000);}
			stackPtr--;
			break;

		case PCD_SetAirControl:
			//FIXME
			{float airc = float(stack[stackPtr - 1]) / float(0x10000);}
			stackPtr--;
			break;

		case PCD_SetAirControlDirect:
			//FIXME
			{float airc = float(PC_GET_INT) / float(0x10000);}
			stackPtr--;
			break;

		case PCD_ClearInventory:
			//FIXME
			if (Activator)
			{
				//DoClearInv(Activator);
			}
			else
			{
				for (int i = 0; i < MAXPLAYERS; ++i)
				{
					//if (svvars.Players[i] && svvars.Players[i]->bSpawned)
						//DoClearInv(svvars.Players[i]->MO);
				}
			}
			break;

		case PCD_GiveInventory:
			//FIXME
			GiveInventory(Activator, FACScriptsObject::StaticGetString(stack[stackPtr - 2]),
				stack[stackPtr - 1]);
			stackPtr -= 2;
			break;

		case PCD_GiveInventoryDirect:
			//FIXME
			{
				const char* ItemName = FACScriptsObject::StaticGetString(PC_GET_INT);
				GiveInventory(Activator, ItemName, PC_GET_INT);
			}
			break;

		case PCD_TakeInventory:
			//FIXME
			TakeInventory(Activator, FACScriptsObject::StaticGetString(stack[stackPtr - 2]),
				stack[stackPtr - 1]);
			stackPtr -= 2;
			break;

		case PCD_TakeInventoryDirect:
			{
				const char* ItemName = FACScriptsObject::StaticGetString(PC_GET_INT);
				TakeInventory(Activator, ItemName, PC_GET_INT);
			}
			break;

		case PCD_CheckInventory:
			stack[stackPtr - 1] = CheckInventory(Activator,
				FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
			break;

		case PCD_CheckInventoryDirect:
			stack[stackPtr++] = CheckInventory(Activator,
				FACScriptsObject::StaticGetString(stack[PC_GET_INT]));
			break;

		case PCD_Spawn:
			//FIXME
//			stack[stackPtr - 6] = DoSpawn(FACScriptsObject::StaticGetString(stack[stackPtr - 6]),
//				float(stack[stackPtr - 5]) / float(0x10000),
//				float(stack[stackPtr - 4]) / float(0x10000),
//				float(stack[stackPtr - 3]) / float(0x10000),
//				stack[stackPtr - 2],
//				float(stack[stackPtr - 1]) * 360.0 / 256.0);
			stackPtr -= 5;
			break;

		case PCD_SpawnDirect:
			{
				//FIXME
				const char* TypeName = FACScriptsObject::StaticGetString(PC_GET_INT);
				float x = float(PC_GET_INT) / float(0x10000);
				float y = float(PC_GET_INT) / float(0x10000);
				float z = float(PC_GET_INT) / float(0x10000);
				int tid = PC_GET_INT;
				float ang = float(PC_GET_INT) * 360.0 / 256.0;
//				stack[stackPtr] = DoSpawn(TypeName, x, y, z, tid, ang);
				stackPtr++;
			}
			break;

		case PCD_SpawnSpot:
			//FIXME
			{
				const char* TypeName = FACScriptsObject::StaticGetString(stack[stackPtr - 4]);
				int spot = stack[stackPtr - 3];
				int tid = stack[stackPtr -2];
				float ang = float(stack[stackPtr - 1]) * 360.0 / 256.0;
				//stack[stackPtr - 4] = DoSpawnSpot(TypeName, spot, tid, ang);
				stackPtr -= 3;
			}
			break;

		case PCD_SpawnSpotDirect:
			//FIXME
			{
				const char* TypeName = FACScriptsObject::StaticGetString(PC_GET_INT);
				int spot = PC_GET_INT;
				int tid = PC_GET_INT;
				float ang = float(PC_GET_INT) * 360.0 / 256.0;
				//stack[stackPtr] = DoSpawnSpot(TypeName, spot, tid, ang);
				stackPtr++;
			}
			break;

		case PCD_SetMusic:
			//FIXME send over network. Also needs to be saved in savegame.
			//What's the third argument?
			S_StartSong(FACScriptsObject::StaticGetString(stack[stackPtr - 3]), stack[stackPtr - 2], true);
			stackPtr -= 3;
			break;

		case PCD_SetMusicDirect:
			//FIXME send over network. Also needs to be saved in savegame.
			//What's the third argument?
			{
				const char* SongName = FACScriptsObject::StaticGetString(PC_GET_INT);
				int cdtrack = PC_GET_INT;
				PC_GET_INT;
				S_StartSong(SongName, cdtrack, true);
			}
			break;

		case PCD_LocalSetMusic:
			//FIXME send over network. Also needs to be saved in savegame.
			//What's the third argument?
			//Only for Activator.
			S_StartSong(FACScriptsObject::StaticGetString(stack[stackPtr - 3]), stack[stackPtr - 2], true);
			stackPtr -= 3;
			break;

		case PCD_LocalSetMusicDirect:
			//FIXME send over network. Also needs to be saved in savegame.
			//What's the third argument?
			//Only for Activator.
			{
				const char* SongName = FACScriptsObject::StaticGetString(PC_GET_INT);
				int cdtrack = PC_GET_INT;
				PC_GET_INT;
				S_StartSong(SongName, cdtrack, true);
			}
			break;

		case PCD_PrintFixed:
			strcat(PrintBuffer, va("%f", float(stack[stackPtr - 1]) / float(0x10000)));
			stackPtr--;
			break;

		case PCD_PrintLocalized:
			//FIXME print localized string.
			strcat(PrintBuffer, FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
			stackPtr--;
			break;

		case PCD_MoreHudMessage:
			strbin(PrintBuffer);
			optstart = -1;
			break;

		case PCD_OptHudMessage:
			optstart = stackPtr;
			break;

		case PCD_EndHudMessage:
		case PCD_EndHudMessageBold:
			{
				if (optstart == -1)
				{
					optstart = stackPtr;
				}
				int type = stack[optstart - 6];
				int id = stack[optstart - 5];
				int color = stack[optstart - 4];
				float x = float(stack[optstart - 3]) / float(0x10000);
				float y = float(stack[optstart - 2]) / float(0x10000);
				float holdTime = float(stack[optstart - 1]) / float(0x10000);

				/*FIXME
				if (screen == NULL ||
					players[consoleplayer].mo == screen)
				{
					DHUDMessage *msg;

					switch (type & ~HUDMSG_LOG)
					{
					default:	// normal
						msg = new DHUDMessage (work, x, y, hudwidth, hudheight, color, holdTime);
						break;
					case 1:		// fade out
						{
							float fadeTime = (optstart < sp) ?
								FIXED2FLOAT(Stack[optstart]) : 0.5f;
							msg = new DHUDMessageFadeOut (work, x, y, hudwidth, hudheight, color, holdTime, fadeTime);
						}
						break;
					case 2:		// type on, then fade out
						{
							float typeTime = (optstart < sp) ?
								FIXED2FLOAT(Stack[optstart]) : 0.05f;
							float fadeTime = (optstart < sp-1) ?
								FIXED2FLOAT(Stack[optstart+1]) : 0.5f;
							msg = new DHUDMessageTypeOnFadeOut (work, x, y, hudwidth, hudheight, color, typeTime, holdTime, fadeTime);
						}
						break;
					}
					StatusBar->AttachMessage (msg, id ? 0xff000000|id : 0);
					if (type & HUDMSG_LOG)
					{
						static char bar[] = TEXTCOLOR_ORANGE "\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36"
					"\36\36\36\36\36\36\36\36\36\36\36\36\37" TEXTCOLOR_NORMAL "\n";

						workreal[0] = '\x1c';
						workreal[1] = color >= CR_BRICK && color <= CR_YELLOW ? 'A' + color : '-';
						AddToConsole (-1, bar);
						AddToConsole (-1, workreal);
						AddToConsole (-1, bar);
					}
				}*/
				if (cmd != PCD_EndHudMessageBold &&
					Activator && Activator->bIsPlayer)
				{
					SV_ClientPrintf(Activator->Player, "%s\n", PrintBuffer);
				}
				else
				{
					for (int i = 0; i < MAXPLAYERS; i++)
					{
						if (svvars.Players[i])
						{
							SV_ClientPrintf(svvars.Players[i], "%s\n", PrintBuffer);
						}
					}
				}
			}
			stackPtr = optstart - 6;
			break;

		case PCD_SetFont:
			{
				const char* FontName = FACScriptsObject::StaticGetString(stack[stackPtr - 1]);
				//FIXME set the font.
				stackPtr--;
			}
			break;

		case PCD_SetFontDirect:
			{
				const char* FontName = FACScriptsObject::StaticGetString(PC_GET_INT);
				//FIXME set the font.
			}
			break;

		case PCD_PushByte:
			stack[stackPtr++] = ((byte*)PCodePtr)[0];
			PCodePtr = (int*)((byte*)PCodePtr + 1);
			break;

		case PCD_LSpec1DirectB:
			SpecArgs[0] = ((byte*)PCodePtr)[1];
			P_ExecuteLineSpecial(((byte*)PCodePtr)[0], SpecArgs, line, side, Activator);
			PCodePtr = (int*)((byte*)PCodePtr + 2);
			break;

		case PCD_LSpec2DirectB:
			{
				int special;
			
				special = ((byte*)PCodePtr)[0];
				SpecArgs[0] = ((byte*)PCodePtr)[1];
				SpecArgs[1] = ((byte*)PCodePtr)[2];
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
				PCodePtr = (int*)((byte*)PCodePtr + 3);
			}
			break;

		case PCD_LSpec3DirectB:
			{
				int special;
			
				special = ((byte*)PCodePtr)[0];
				SpecArgs[0] = ((byte*)PCodePtr)[1];
				SpecArgs[1] = ((byte*)PCodePtr)[2];
				SpecArgs[2] = ((byte*)PCodePtr)[3];
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
				PCodePtr = (int*)((byte*)PCodePtr + 4);
			}
			break;

		case PCD_LSpec4DirectB:
			{
				int special;
			
				special = ((byte*)PCodePtr)[0];
				SpecArgs[0] = ((byte*)PCodePtr)[1];
				SpecArgs[1] = ((byte*)PCodePtr)[2];
				SpecArgs[2] = ((byte*)PCodePtr)[3];
				SpecArgs[3] = ((byte*)PCodePtr)[4];
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
				PCodePtr = (int*)((byte*)PCodePtr + 5);
			}
			break;

		case PCD_LSpec5DirectB:
			{
				int special;
			
				special = ((byte*)PCodePtr)[0];
				SpecArgs[0] = ((byte*)PCodePtr)[1];
				SpecArgs[1] = ((byte*)PCodePtr)[2];
				SpecArgs[2] = ((byte*)PCodePtr)[3];
				SpecArgs[3] = ((byte*)PCodePtr)[4];
				SpecArgs[4] = ((byte*)PCodePtr)[5];
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
				PCodePtr = (int*)((byte*)PCodePtr + 6);
			}
			break;

		case PCD_DelayDirectB:
			DelayTime = float(((byte*)PCodePtr)[0]) / 35.0;
			PCodePtr = (int*)((byte*)PCodePtr + 1);
			action = SCRIPT_STOP;
			break;

		case PCD_RandomDirectB:
			{
				int low;
				int high;
			
				low = ((byte*)PCodePtr)[0];
				high = ((byte*)PCodePtr)[1];
				stack[stackPtr++] = low + (int)(Random() * (high - low + 1));
				PCodePtr = (int*)((byte*)PCodePtr + 2);
			}
			break;

		case PCD_PushBytes:
			{
				int count = ((byte*)PCodePtr)[0];
				for (int i = 0; i < count; i++)
					stack[stackPtr + i] = ((byte*)PCodePtr)[i + 1];
				stackPtr += count;
				PCodePtr = (int*)((byte*)PCodePtr + count + 1);
			}
			break;

		case PCD_Push2Bytes:
			stack[stackPtr] = ((byte*)PCodePtr)[0];
			stack[stackPtr + 1] = ((byte*)PCodePtr)[1];
			stackPtr += 2;
			PCodePtr = (int*)((byte*)PCodePtr + 2);
			break;

		case PCD_Push3Bytes:
			stack[stackPtr] = ((byte*)PCodePtr)[0];
			stack[stackPtr + 1] = ((byte*)PCodePtr)[1];
			stack[stackPtr + 2] = ((byte*)PCodePtr)[2];
			stackPtr += 3;
			PCodePtr = (int*)((byte*)PCodePtr + 3);
			break;

		case PCD_Push4Bytes:
			stack[stackPtr] = ((byte*)PCodePtr)[0];
			stack[stackPtr + 1] = ((byte*)PCodePtr)[1];
			stack[stackPtr + 2] = ((byte*)PCodePtr)[2];
			stack[stackPtr + 3] = ((byte*)PCodePtr)[3];
			stackPtr += 4;
			PCodePtr = (int*)((byte*)PCodePtr + 4);
			break;

		case PCD_Push5Bytes:
			stack[stackPtr] = ((byte*)PCodePtr)[0];
			stack[stackPtr + 1] = ((byte*)PCodePtr)[1];
			stack[stackPtr + 2] = ((byte*)PCodePtr)[2];
			stack[stackPtr + 3] = ((byte*)PCodePtr)[3];
			stack[stackPtr + 4] = ((byte*)PCodePtr)[4];
			stackPtr += 5;
			PCodePtr = (int*)((byte*)PCodePtr + 5);
			break;

		case PCD_SetThingSpecial:
			{
				int tid = stack[stackPtr - 7];
				int special = stack[stackPtr - 6];
				int arg1 = stack[stackPtr - 5];
				int arg2 = stack[stackPtr - 4];
				int arg3 = stack[stackPtr - 3];
				int arg4 = stack[stackPtr - 2];
				int arg5 = stack[stackPtr - 1];
				//FIXME implement.
				stackPtr -= 7;
			}
			break;

		case PCD_AssignGlobalVar:
			GlobalVars[NEXTBYTE] = stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_PushGlobalVar:
			stack[stackPtr++] = GlobalVars[NEXTBYTE];
			break;

		case PCD_AddGlobalVar:
			GlobalVars[NEXTBYTE] += stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_SubGlobalVar:
			GlobalVars[NEXTBYTE] -= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_MulGlobalVar:
			GlobalVars[NEXTBYTE] *= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_DivGlobalVar:
			GlobalVars[NEXTBYTE] /= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_ModGlobalVar:
			GlobalVars[NEXTBYTE] %= stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_IncGlobalVar:
			GlobalVars[NEXTBYTE]++;
			break;

		case PCD_DecGlobalVar:
			GlobalVars[NEXTBYTE]--;
			break;

		case PCD_FadeTo:
			{
				int r = stack[stackPtr - 5];
				int g = stack[stackPtr - 4];
				int b = stack[stackPtr - 3];
				int a = stack[stackPtr - 2];
				float time = float(stack[stackPtr - 1]) / float(0x10000);
				//FIXME implement
				stackPtr -= 5;
			}
			break;

		case PCD_FadeRange:
			{
				int r1 = stack[stackPtr - 9];
				int g1 = stack[stackPtr - 8];
				int b1 = stack[stackPtr - 7];
				int a1 = stack[stackPtr - 6];
				int r2 = stack[stackPtr - 5];
				int g2 = stack[stackPtr - 4];
				int b2 = stack[stackPtr - 3];
				int a2 = stack[stackPtr - 2];
				float time = float(stack[stackPtr - 1]) / float(0x10000);
				//FIXME implement
				stackPtr -= 9;
			}
			break;

		case PCD_CancelFade:
			//FIXME implement.
			break;

		case PCD_PlayMovie:
			{
				const char* MovieName = FACScriptsObject::StaticGetString(stack[stackPtr - 1]);
				//FIXME implement
				stack[stackPtr - 1] = 0;
			}
			break;

		case PCD_SetFloorTrigger:
			{
				int tag = stack[stackPtr - 8];
				int height = stack[stackPtr - 7];
				int special = stack[stackPtr - 6];
				int arg0 = stack[stackPtr - 5];
				int arg1 = stack[stackPtr - 4];
				int arg2 = stack[stackPtr - 3];
				int arg3 = stack[stackPtr - 2];
				int arg4 = stack[stackPtr - 1];
				//FIXME implement.
				stackPtr -= 8;
			}
			break;

		case PCD_SetCeilingTrigger:
			{
				int tag = stack[stackPtr - 8];
				int height = stack[stackPtr - 7];
				int special = stack[stackPtr - 6];
				int arg0 = stack[stackPtr - 5];
				int arg1 = stack[stackPtr - 4];
				int arg2 = stack[stackPtr - 3];
				int arg3 = stack[stackPtr - 2];
				int arg4 = stack[stackPtr - 1];
				//FIXME implement.
				stackPtr -= 8;
			}
			break;

		case PCD_GetActorX:
			{
				VEntity* Ent = EntityFromTID(stack[stackPtr - 1], Activator);
				if (!Ent)
				{
					stack[stackPtr - 1] = 0;
				}
				else
				{
					stack[stackPtr - 1] = int(Ent->Origin.x * 0x10000);
				}
			}
			break;

		case PCD_GetActorY:
			{
				VEntity* Ent = EntityFromTID(stack[stackPtr - 1], Activator);
				if (!Ent)
				{
					stack[stackPtr - 1] = 0;
				}
				else
				{
					stack[stackPtr - 1] = int(Ent->Origin.y * 0x10000);
				}
			}
			break;

		case PCD_GetActorZ:
			{
				VEntity* Ent = EntityFromTID(stack[stackPtr - 1], Activator);
				if (!Ent)
				{
					stack[stackPtr - 1] = 0;
				}
				else
				{
					stack[stackPtr - 1] = int(Ent->Origin.z * 0x10000);
				}
			}
			break;

		case PCD_StartTranslation:
			{
				int Index = stack[stackPtr - 1];
				stackPtr--;
				//FIXME
			}
			break;

		case PCD_TranslationRange1:
			{
				int start = stack[stackPtr - 4];
				int end = stack[stackPtr - 3];
				int pal1 = stack[stackPtr - 2];
				int pal2 = stack[stackPtr - 1];
				stackPtr -= 4;
				//FIXME
			}
			break;

		case PCD_TranslationRange2:
			{
				int start = stack[stackPtr - 8];
				int end = stack[stackPtr - 7];
				int r1 = stack[stackPtr - 6];
				int g1 = stack[stackPtr - 5];
				int b1 = stack[stackPtr - 4];
				int r2 = stack[stackPtr - 3];
				int g2 = stack[stackPtr - 2];
				int b2 = stack[stackPtr - 1];
				stackPtr -= 8;
				//FIXME
			}
			break;

		case PCD_EndTranslation:
			//FIXME
			break;

		case PCD_Call:
		case PCD_CallDiscard:
			{
				int funcnum;
				int i;
				FACScriptFunction *func;
				FACScriptsObject* object = activeObject;

				funcnum = NEXTBYTE;
				func = activeObject->GetFunction(funcnum, object);
				if (!func)
				{
					GCon->Logf("Function %d in script %d out of range", funcnum, number);
					action = SCRIPT_TERMINATE;
					break;
				}
				if (stackPtr + func->LocalCount + 64 > ACS_STACK_DEPTH)
				{
					// 64 is the margin for the function's working space
					GCon->Logf("Out of stack space in script %d", number);
					action = SCRIPT_TERMINATE;
					break;
				}
				// The function's first argument is also its first local variable.
				locals = &stack[stackPtr - func->ArgCount];
				// Make space on the stack for any other variables the function uses.
				for (i = 0; i < func->LocalCount; i++)
				{
					stack[stackPtr + i] = 0;
				}
				stackPtr += i;
				((CallReturn*)&stack[stackPtr])->ReturnAddress = activeObject->PtrToOffset(PCodePtr);
				((CallReturn*)&stack[stackPtr])->ReturnFunction = activeFunction;
				((CallReturn*)&stack[stackPtr])->ReturnObject = activeObject;
				((CallReturn*)&stack[stackPtr])->bDiscardResult = (cmd == PCD_CallDiscard);
				stackPtr += sizeof(CallReturn) / sizeof(int);
				PCodePtr = activeObject->OffsetToPtr(func->Address);
				activeObject = object;
				activeFunction = func;
			}
			break;

		case PCD_ReturnVoid:
		case PCD_ReturnVal:
			{
				int value;
				CallReturn *retState;

				if (cmd == PCD_ReturnVal)
				{
					value = stack[--stackPtr];
				}
				else
				{
					value = 0;
				}
				stackPtr -= sizeof(CallReturn) / sizeof(int);
				retState = (CallReturn*)&stack[stackPtr];
				PCodePtr = activeObject->OffsetToPtr(retState->ReturnAddress);
				stackPtr -= activeFunction->ArgCount + activeFunction->LocalCount;
				activeFunction = retState->ReturnFunction;
				activeObject = retState->ReturnObject;
				fmt = activeObject->GetFormat();
				if (!activeFunction)
				{
					locals = LocalVars;
				}
				else
				{
					locals = &stack[stackPtr - activeFunction->ArgCount -
						activeFunction->LocalCount - sizeof(CallReturn) / sizeof(int)];
				}
				if (!retState->bDiscardResult)
				{
					stack[stackPtr++] = value;
				}
			}
			break;

		case PCD_PushMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 1];
				stack[stackPtr - 1] = activeObject->GetArrayVal(ANum, Idx);
			}
			break;

		case PCD_AssignMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				activeObject->SetArrayVal(ANum, Idx, Val);
				stackPtr -= 2;
			}
			break;

		case PCD_AddMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				activeObject->SetArrayVal(ANum, Idx,
					activeObject->GetArrayVal(ANum, Idx) + Val);
				stackPtr -= 2;
			}
			break;

		case PCD_SubMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				activeObject->SetArrayVal(ANum, Idx,
					activeObject->GetArrayVal(ANum, Idx) - Val);
				stackPtr -= 2;
			}
			break;

		case PCD_MulMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				activeObject->SetArrayVal(ANum, Idx,
					activeObject->GetArrayVal(ANum, Idx) * Val);
				stackPtr -= 2;
			}
			break;

		case PCD_DivMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				activeObject->SetArrayVal(ANum, Idx,
					activeObject->GetArrayVal(ANum, Idx) / Val);
				stackPtr -= 2;
			}
			break;

		case PCD_ModMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				activeObject->SetArrayVal(ANum, Idx,
					activeObject->GetArrayVal(ANum, Idx) % Val);
				stackPtr -= 2;
			}
			break;

		case PCD_IncMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 1];
				activeObject->SetArrayVal(ANum, Idx,
					activeObject->GetArrayVal(ANum, Idx) + 1);
				stackPtr--;
			}
			break;

		case PCD_DecMapArray:
			{
				int ANum = *activeObject->MapVars[NEXTBYTE];
				int Idx = stack[stackPtr - 1];
				activeObject->SetArrayVal(ANum, Idx,
					activeObject->GetArrayVal(ANum, Idx) - 1);
				stackPtr--;
			}
			break;

		case PCD_Dup:
			stack[stackPtr] = stack[stackPtr - 1];
			stackPtr++;
			break;

		case PCD_Swap:
			{
				int tmp = stack[stackPtr - 2];
				stack[stackPtr - 2] = stack[stackPtr - 1];
				stack[stackPtr - 1] = tmp;
			}
			break;

		case PCD_Sin:
			stack[stackPtr - 1] = int(msin(float(stack[stackPtr - 1]) *
				360.0 / 0x10000) * 0x10000);
			break;

		case PCD_Cos:
			stack[stackPtr - 1] = int(mcos(float(stack[stackPtr - 1]) *
				360.0 / 0x10000) * 0x10000);
			break;

		case PCD_VectorAngle:
			stack[stackPtr - 2] = int(matan(float(stack[stackPtr - 1]) / float(0x10000),
				float(stack[stackPtr - 2]) / float(0x10000)) / 360.0 * 0x10000);
			stackPtr--;
			break;

		case PCD_CheckWeapon:
			{
				const char* WpnName = FACScriptsObject::StaticGetString(stack[stackPtr - 1]);
				//FIXME implement.
				stack[stackPtr - 1] = 0;
			}
			break;

		case PCD_SetWeapon:
			{
				const char* WpnName = FACScriptsObject::StaticGetString(stack[stackPtr - 1]);
				//FIXME implement.
				stack[stackPtr - 1] = 0;
			}
			break;

		case PCD_TagString:
			stack[stackPtr - 1] |= activeObject->GetLibraryID();
			break;

		case PCD_PushWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 1];
				stack[stackPtr - 1] = WorldArrays[ANum].GetElemVal(Idx);
			}
			break;

		case PCD_AssignWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				WorldArrays[ANum].SetElemVal(Idx, Val);
				stackPtr -= 2;
			}
			break;

		case PCD_AddWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				WorldArrays[ANum].SetElemVal(Idx,
					WorldArrays[ANum].GetElemVal(Idx) + Val);
				stackPtr -= 2;
			}
			break;

		case PCD_SubWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				WorldArrays[ANum].SetElemVal(Idx,
					WorldArrays[ANum].GetElemVal(Idx) - Val);
				stackPtr -= 2;
			}
			break;

		case PCD_MulWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				WorldArrays[ANum].SetElemVal(Idx,
					WorldArrays[ANum].GetElemVal(Idx) * Val);
				stackPtr -= 2;
			}
			break;

		case PCD_DivWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				WorldArrays[ANum].SetElemVal(Idx,
					WorldArrays[ANum].GetElemVal(Idx) / Val);
				stackPtr -= 2;
			}
			break;

		case PCD_ModWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				WorldArrays[ANum].SetElemVal(Idx,
					WorldArrays[ANum].GetElemVal(Idx) % Val);
				stackPtr -= 2;
			}
			break;

		case PCD_IncWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 1];
				WorldArrays[ANum].SetElemVal(Idx,
					WorldArrays[ANum].GetElemVal(Idx) + 1);
				stackPtr--;
			}
			break;

		case PCD_DecWorldArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 1];
				WorldArrays[ANum].SetElemVal(Idx,
					WorldArrays[ANum].GetElemVal(Idx) - 1);
				stackPtr--;
			}
			break;

		case PCD_PushGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 1];
				stack[stackPtr - 1] = GlobalArrays[ANum].GetElemVal(Idx);
			}
			break;

		case PCD_AssignGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				GlobalArrays[ANum].SetElemVal(Idx, Val);
				stackPtr -= 2;
			}
			break;

		case PCD_AddGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				GlobalArrays[ANum].SetElemVal(Idx,
					GlobalArrays[ANum].GetElemVal(Idx) + Val);
				stackPtr -= 2;
			}
			break;

		case PCD_SubGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				GlobalArrays[ANum].SetElemVal(Idx,
					GlobalArrays[ANum].GetElemVal(Idx) - Val);
				stackPtr -= 2;
			}
			break;

		case PCD_MulGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				GlobalArrays[ANum].SetElemVal(Idx,
					GlobalArrays[ANum].GetElemVal(Idx) * Val);
				stackPtr -= 2;
			}
			break;

		case PCD_DivGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				GlobalArrays[ANum].SetElemVal(Idx,
					GlobalArrays[ANum].GetElemVal(Idx) / Val);
				stackPtr -= 2;
			}
			break;

		case PCD_ModGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 2];
				int Val = stack[stackPtr - 1];
				GlobalArrays[ANum].SetElemVal(Idx,
					GlobalArrays[ANum].GetElemVal(Idx) % Val);
				stackPtr -= 2;
			}
			break;

		case PCD_IncGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 1];
				GlobalArrays[ANum].SetElemVal(Idx,
					GlobalArrays[ANum].GetElemVal(Idx) + 1);
				stackPtr--;
			}
			break;

		case PCD_DecGlobalArray:
			{
				int ANum = NEXTBYTE;
				int Idx = stack[stackPtr - 1];
				GlobalArrays[ANum].SetElemVal(Idx,
					GlobalArrays[ANum].GetElemVal(Idx) - 1);
				stackPtr--;
			}
			break;

		case PCD_SetMarineWeapon:
			{
				int TID = stack[stackPtr - 2];
				const char* WeapName = FACScriptsObject::StaticGetString(stack[stackPtr - 1]);
				//FIXME implement.
				stackPtr -= 2;
			}
			break;

		case PCD_SetActorProperty:
			{
				int TID = stack[stackPtr - 3];
				int property = stack[stackPtr - 2];
				int value = stack[stackPtr - 1];
				//FIXME implement.
				stackPtr -= 3;
			}
			break;

		case PCD_GetActorProperty:
			{
				int TID = stack[stackPtr - 2];
				int property = stack[stackPtr - 1];
				//FIXME implement.
				stack[stackPtr - 2] = 0;
				stackPtr -= 1;
			}
			break;

		case PCD_PlayerNumber:
			if (!Activator || !Activator->bIsPlayer)
			{
				stack[stackPtr++] = -1;
			}
			else
			{
				stack[stackPtr++] = SV_GetPlayerNum(Activator->Player);
			}
			break;

		case PCD_ActivatorTID:
			if (!Activator)
			{
				stack[stackPtr++] = 0;
			}
			else
			{
				stack[stackPtr++] = Activator->TID;
			}
			break;

		case PCD_SetMarineSprite:
			{
				int TID = stack[stackPtr - 2];
				const char* ClassName = FACScriptsObject::StaticGetString(stack[stackPtr - 1]);
				//FIXME implement.
				stackPtr -= 2;
			}
			break;

		case PCD_GetScreenWidth:
			//FIXME server knows nothing about screens.
			stack[stackPtr++] = 320;
			break;

		case PCD_GetScreenHeight:
			//FIXME server knows nothing about screens.
			stack[stackPtr++] = 200;
			break;

		case PCD_ThingProjectile2:
			// Like Thing_Projectile(Gravity) specials, but you can give the
			// projectile a TID.
			// Thing_Projectile2(tid, type, angle, speed, vspeed, gravity, newtid);
			{
				int tid = stack[stackPtr - 7];
				int type = stack[stackPtr - 6];
				int angle = stack[stackPtr - 5];
				int speed = stack[stackPtr - 4];
				int vspeed = stack[stackPtr - 3];
				int gravity = stack[stackPtr - 2];
				int newtid = stack[stackPtr - 1];
				//FIXME implement.
			}
			stackPtr -= 7;
			break;

		case PCD_StrLen:
			stack[stackPtr - 1] = strlen(FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
			break;

		case PCD_SetHudSize:
			{
				int hudwidth = abs(stack[stackPtr - 3]);
				int hudheight = abs(stack[stackPtr - 2]);
				if (stack[stackPtr - 1])
				{
					//	Negative height means to cover the status bar.
					hudheight = -hudheight;
				}
				//FIXME implement.
				stackPtr -= 3;
			}
			break;

		case PCD_GetCvar:
			stack[stackPtr - 1] = TCvar::Value(FACScriptsObject::StaticGetString(stack[stackPtr - 1]));
			break;

		case PCD_CaseGotoSorted:
			//	The count and jump table are 4-byte aligned.
			PCodePtr = (int*)(((int)PCodePtr + 3) & ~3);
			{
				int numcases = PC_GET_INT;
				int min = 0, max = numcases - 1;
				while (min <= max)
				{
					int mid = (min + max) / 2;
					int caseval = PCodePtr[mid * 2];
					if (caseval == stack[stackPtr - 1])
					{
						PCodePtr = activeObject->OffsetToPtr(PCodePtr[mid * 2 + 1]);
						stackPtr--;
						break;
					}
					else if (caseval < stack[stackPtr - 1])
					{
						min = mid + 1;
					}
					else
					{
						max = mid - 1;
					}
				}
				if (min > max)
				{
					// The case was not found, so go to the next instruction.
					PCodePtr += numcases * 2;
				}
			}
			break;

		case PCD_SetResultValue:
			resultValue = stack[stackPtr - 1];
			stackPtr--;
			break;

		case PCD_GetLineRowOffset:
			if (line)
			{
				stack[stackPtr++] = (int)GLevel->Sides[line->sidenum[0]].rowoffset;
			}
			else
			{
				stack[stackPtr++] = 0;
			}
			break;

		case PCD_GetActorFloorZ:
			{
				VEntity* Ent = EntityFromTID(stack[stackPtr - 1], Activator);
				if (!Ent)
				{
					stack[stackPtr - 1] = 0;
				}
				else
				{
					stack[stackPtr - 1] = int(Ent->FloorZ * 0x10000);
				}
			}
			break;

		case PCD_GetActorAngle:
			{
				VEntity* Ent = EntityFromTID(stack[stackPtr - 1], Activator);
				if (!Ent)
				{
					stack[stackPtr - 1] = 0;
				}
				else
				{
					stack[stackPtr - 1] = int(Ent->Angles.yaw * 0x10000 / 360) & 0xffff;
				}
			}
			break;

		case PCD_GetSectorFloorZ:
			// Arguments are (tag, x, y). If you don't use slopes, then (x, y) don't
			// really matter and can be left as (0, 0) if you like.
			{
				int secnum = FindSectorFromTag(stack[stackPtr - 3], -1);
				float z = 0;

				if (secnum >= 0)
				{
					z = GLevel->Sectors[secnum].floor.GetPointZ(
						stack[stackPtr - 2], stack[stackPtr - 1]);
				}
				stack[stackPtr - 3] = int(z * 0x10000);
				stackPtr -= 2;
			}
			break;

		case PCD_GetSectorCeilingZ:
			// Arguments are (tag, x, y). If you don't use slopes, then (x, y) don't
			// really matter and can be left as (0, 0) if you like.
			{
				int secnum = FindSectorFromTag(stack[stackPtr - 3], -1);
				float z = 0;

				if (secnum >= 0)
				{
					z = GLevel->Sectors[secnum].ceiling.GetPointZ(
						stack[stackPtr - 2], stack[stackPtr - 1]);
				}
				stack[stackPtr - 3] = int(z * 0x10000);
				stackPtr -= 2;
			}
			break;

		case PCD_LSpec5Result:
			{
				int special;
			
				special = NEXTBYTE;
				SpecArgs[4] = stack[stackPtr - 1];
				SpecArgs[3] = stack[stackPtr - 2];
				SpecArgs[2] = stack[stackPtr - 3];
				SpecArgs[1] = stack[stackPtr - 4];
				SpecArgs[0] = stack[stackPtr - 5];
				stack[stackPtr - 5] = P_ExecuteLineSpecial(special,
					SpecArgs, line, side, Activator);
				stackPtr -= 4;
			}
			break;

		case PCD_GetSigilPieces:
			if (Activator)
				stack[stackPtr++] = Activator->eventGetSigilPieces();
			else
				stack[stackPtr++] = 0;
			break;

			//	These opcodes are not supported. They will terminate script.
		case PCD_PlayerBlueSkull:
		case PCD_PlayerRedSkull:
		case PCD_PlayerYellowSkull:
		case PCD_PlayerMasterSkull:
		case PCD_PlayerBlueCard:
		case PCD_PlayerRedCard:
		case PCD_PlayerYellowCard:
		case PCD_PlayerMasterCard:
		case PCD_PlayerBlackSkull:
		case PCD_PlayerSilverSkull:
		case PCD_PlayerGoldSkull:
		case PCD_PlayerBlavkCard:
		case PCD_PlayerSilverCard:
		case PCD_PlayerOnTeam:
		case PCD_PlayerTeam:
		case PCD_PlayerExpert:
		case PCD_BlueTeamCount:
		case PCD_RedTeamCount:
		case PCD_BlueTeamScore:
		case PCD_RedTeamScore:
		case PCD_IsOneFlagCTF:
		case PCD_LSpec6:
		case PCD_LSpec6Direct:
		case PCD_Team2FragPoints:
		case PCD_ConsoleCommand:
		case PCD_SetStyle:
		case PCD_SetStyleDirect:
		case PCD_WriteToIni:
		case PCD_GetFromIni:
			GCon->Logf(NAME_Dev, "Unsupported ACS opcode %d", cmd);
			action = SCRIPT_TERMINATE;
			break;

		default:
			Host_Error("Illegal ACS opcode %d", cmd);
			break;
		}
	} while  (action == SCRIPT_CONTINUE);
	ip = PCodePtr;
	if (action == SCRIPT_TERMINATE)
	{
		info->state = ASTE_INACTIVE;
		FACScriptsObject::StaticScriptFinished(number);
		ConditionalDestroy();
		Z_Free(LocalVars);
	}
	return resultValue;
}

//==========================================================================
//
//	ACS.Archive
//
//==========================================================================

IMPLEMENT_FUNCTION(VACS, Archive)
{
	guard(VACS.Archive);
	VACS	*acs;

	acs = (VACS *)PR_Pop();
	acs->ip = (int*)acs->activeObject->PtrToOffset(acs->ip);
	acs->line = acs->line ? (line_t *)(acs->line - acs->XLevel->Lines) : (line_t *)-1;
	acs->Activator = (VEntity *)GetMobjNum(acs->Activator);
	acs->activeObject = (FACScriptsObject*)(acs->activeObject->GetLibraryID() >> 16);
	unguard;
}

//==========================================================================
//
//	ACS.Unarchive
//
//==========================================================================

IMPLEMENT_FUNCTION(VACS, Unarchive)
{
	guard(VACS.Unarchive);
	VACS	*acs;

	acs = (VACS *)PR_Pop();
	acs->activeObject = FACScriptsObject::StaticGetObject((int)acs->activeObject);
	acs->ip = acs->activeObject->OffsetToPtr((int)acs->ip);
	if ((int)acs->line == -1)
	{
		acs->line = NULL;
	}
	else
	{
		acs->line = &acs->XLevel->Lines[(int)acs->line];
	}
	acs->Activator = (VEntity *)SetMobjPtr((int)acs->Activator);
	acs->info = acs->activeObject->FindScript(acs->number);
	unguard;
}

//==========================================================================
//
// P_TagFinished
//
//==========================================================================

void P_TagFinished(int tag)
{
	FACScriptsObject::StaticTagFinished(tag);
}

//==========================================================================
//
// P_PolyobjFinished
//
//==========================================================================

void P_PolyobjFinished(int po)
{
	FACScriptsObject::StaticPolyobjFinished(po);
}

//==========================================================================
//
//  FindSectorFromTag
//
//	RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
//==========================================================================

static int FindSectorFromTag(int tag, int start)
{
    int	i;
	
    for (i = start + 1; i < GLevel->NumSectors; i++)
		if (GLevel->Sectors[i].tag == tag)
		    return i;
    
    return -1;
}

//==========================================================================
//
//	TagBusy
//
//==========================================================================

static bool TagBusy(int tag)
{
	return !!svpr.Exec(pf_TagBusy, tag);
}

//============================================================================
//
//	GiveInventory
//
//============================================================================

static void GiveInventory(VEntity* Activator, const char* Type, int Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	if (strcmp(Type, "Armor") == 0)
	{
		Type = "BasicArmor";
	}
	else if (Activator)
	{
		Activator->eventGiveInventory(Type, Amount);
	}
	else
	{
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (svvars.Players[i] && svvars.Players[i]->bSpawned)
				svvars.Players[i]->MO->eventGiveInventory(Type, Amount);
		}
	}
}

//============================================================================
//
//	GiveInventory
//
//============================================================================

static void TakeInventory(VEntity* Activator, const char* Type, int Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	if (strcmp(Type, "Armor") == 0)
	{
		Type = "BasicArmor";
	}
	if (Activator)
	{
		Activator->eventTakeInventory(Type, Amount);
	}
	else
	{
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (svvars.Players[i] && svvars.Players[i]->bSpawned)
				svvars.Players[i]->MO->eventTakeInventory(Type, Amount);
		}
	}
}

//============================================================================
//
//	CheckInventory
//
//============================================================================

static int CheckInventory(VEntity* Activator, const char* Type)
{
	if (!Activator)
		return 0;

	if (strcmp(Type, "Armor") == 0)
	{
		Type = "BasicArmor";
	}
	else if (!strcmp(Type, "Health"))
	{
		return Activator->Health;
	}
	return Activator->eventCheckInventory(Type);
}

//============================================================================
//
//	strbin
//
//============================================================================

static void strbin(char *str)
{
	char *p = str, c;
	int i;

	while ((c = *p++))
	{
		if (c != '\\')
		{
			*str++ = c;
		}
		else
		{
			switch (*p)
			{
//FIXME
//			case 'c':
//				*str++ = TEXTCOLOR_ESCAPE;
//				break;
			case 'n':
				*str++ = '\n';
				break;
			case 't':
				*str++ = '\t';
				break;
			case 'r':
				*str++ = '\r';
				break;
			case '\n':
				break;
			case 'x':
			case 'X':
				c = 0;
				p++;
				for (i = 0; i < 2; i++)
				{
					c <<= 4;
					if (*p >= '0' && *p <= '9')
						c += *p-'0';
					else if (*p >= 'a' && *p <= 'f')
						c += 10 + *p-'a';
					else if (*p >= 'A' && *p <= 'F')
						c += 10 + *p-'A';
					else
						break;
					p++;
				}
				*str++ = c;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				c = 0;
				for (i = 0; i < 3; i++)
				{
					c <<= 3;
					if (*p >= '0' && *p <= '7')
						c += *p-'0';
					else
						break;
					p++;
				}
				*str++ = c;
				break;
			default:
				*str++ = *p;
				break;
			}
			p++;
		}
	}
	*str = 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.30  2004/12/22 07:49:13  dj_jl
//	More extended ACS support, more linedef flags.
//
//	Revision 1.29  2004/12/03 16:15:47  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.28  2004/11/01 07:31:15  dj_jl
//	Replaced function pointer array with big swutch statement.
//	
//	Revision 1.27  2004/10/07 06:47:11  dj_jl
//	Behavior lump size check.
//	
//	Revision 1.26  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.25  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//	
//	Revision 1.24  2003/07/11 16:45:20  dj_jl
//	Made array of players with pointers
//	
//	Revision 1.23  2003/03/08 12:10:13  dj_jl
//	API fixes.
//	
//	Revision 1.22  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.21  2002/08/28 16:41:09  dj_jl
//	Merged VMapObject with VEntity, some natives.
//	
//	Revision 1.20  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.19  2002/07/23 13:10:37  dj_jl
//	Some fixes for switching to floating-point time.
//	
//	Revision 1.18  2002/07/13 07:50:58  dj_jl
//	Added guarding.
//	
//	Revision 1.17  2002/04/11 16:42:09  dj_jl
//	Renamed Think to Tick.
//	
//	Revision 1.16  2002/03/16 17:55:11  dj_jl
//	Some small changes.
//	
//	Revision 1.15  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.14  2002/02/15 19:12:04  dj_jl
//	Property namig style change
//	
//	Revision 1.13  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.12  2002/01/11 08:13:35  dj_jl
//	Fixed sector sound
//	
//	Revision 1.11  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.10  2001/12/27 17:33:29  dj_jl
//	Removed thinker list
//	
//	Revision 1.9  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.8  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.7  2001/10/09 17:28:41  dj_jl
//	Moved thing counting to progs
//	
//	Revision 1.6  2001/10/02 17:43:50  dj_jl
//	Added addfields to lines, sectors and polyobjs
//	
//	Revision 1.5  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.4  2001/08/29 17:55:42  dj_jl
//	Added sound channels
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
