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
#define OPEN_SCRIPTS_BASE 1000
#define PRINT_BUFFER_SIZE 256
#define GAME_SINGLE_PLAYER 0
#define GAME_NET_COOPERATIVE 1
#define GAME_NET_DEATHMATCH 2
#define TEXTURE_TOP 0
#define TEXTURE_MIDDLE 1
#define TEXTURE_BOTTOM 2

// TYPES -------------------------------------------------------------------

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
	PCD_LSpec2Direct,
	PCD_LSpec3Direct,
	PCD_LSpec4Direct,
	PCD_LSpec5Direct,
	PCD_Add,
	PCD_Subtract,
	PCD_Multiply,
	PCD_Divide,
	PCD_Modulus,
	PCD_EQ,
	PCD_NE,
	PCD_LT,
	PCD_GT,
	PCD_LE,
	PCD_GE,
	PCD_AssignScriptVar,
	PCD_AssignMapVar,
	PCD_AssignWorldVar,
	PCD_PushScriptVar,
	PCD_PushMapVar,
	PCD_PushWorldVar,
	PCD_AddScriptVar,
	PCD_AddMapVar,
	PCD_AddWorldVar,
	PCD_SubScriptVar,
	PCD_SubMapVar,
	PCD_SubWorldVar,
	PCD_MulScriptVar,
	PCD_MulMapVar,
	PCD_MulWorldVar,
	PCD_DivScriptVar,
	PCD_DivMapVar,
	PCD_DivWorldVar,
	PCD_ModScriptVar,
	PCD_ModMapVar,
	PCD_ModWorldVar,
	PCD_IncScriptVar,
	PCD_IncMapVar,
	PCD_IncWorldVar,
	PCD_DecScriptVar,
	PCD_DecMapVar,
	PCD_DecWorldVar,
	PCD_Goto,
	PCD_IfGoto,
	PCD_Drop,
	PCD_Delay,
	PCD_DelayDirect,
	PCD_Random,
	PCD_RandomDirect,
	PCD_ThingCount,
	PCD_ThingCountDirect,
	PCD_TagWait,
	PCD_TagWaitDirect,
	PCD_PolyWait,
	PCD_PolyWaitDirect,
	PCD_ChangeFloor,
	PCD_ChangeFloorDirect,
	PCD_ChangeCeiling,
	PCD_ChangeCeilingDirect,
	PCD_Restart,
	PCD_AndLogical,
	PCD_OrLogical,
	PCD_AndBitwise,
	PCD_OrBitwise,
	PCD_EorBitwise,
	PCD_NegateLogical,
	PCD_LShift,
	PCD_RShift,
	PCD_UnaryMinus,
	PCD_IfNotGoto,
	PCD_LineSide,
	PCD_ScriptWait,
	PCD_ScriptWaitDirect,
	PCD_ClearLineSpecial,
	PCD_CaseGoto,
	PCD_BeginPrint,
	PCD_EndPrint,
	PCD_PrintString,
	PCD_PrintNumber,
	PCD_PrintCharacter,
	PCD_PlayerCount,
	PCD_GameType,
	PCD_GameSkill,
	PCD_Timer,
	PCD_SectorSound,
	PCD_AmbientSound,
	PCD_SoundSequence,
	PCD_SetLineTexture,
	PCD_SetLineBlocking,
	PCD_SetLineSpecial,
	PCD_ThingSound,
	PCD_EndPrintBold,

	PCODE_COMMAND_COUNT
};

struct acsHeader_t
{
	int marker;
	int infoOffset;
	int code;
};

class VACS : public VThinker
{
	DECLARE_CLASS(VACS, VThinker, 0)
	NO_DEFAULT_CONSTRUCTOR(VACS)

	VEntity 	*Activator;
	line_t 		*line;
	int 		side;
	int 		number;
	int 		infoIndex;
	float		DelayTime;
	int 		stack[ACS_STACK_DEPTH];
	int			stackPtr;
	int 		vars[MAX_ACS_SCRIPT_VARS];
	int 		*ip;

	void Tick(float DeltaTime);

	DECLARE_FUNCTION(Archive)
	DECLARE_FUNCTION(Unarchive)
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void StartOpenACS(int number, int infoIndex, int *address);
static void ScriptFinished(int number);
static boolean TagBusy(int tag);
static boolean AddToACSStore(const char *map, int number, int *args);
static int GetACSIndex(int number);
static void Push(int value);
static int Pop(void);
static int Top(void);
static void Drop(void);

static int FindSectorFromTag(int tag, int start);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int ACScriptCount;
acsInfo_t *ACSInfo;
int MapVars[MAX_ACS_MAP_VARS];
int WorldVars[MAX_ACS_WORLD_VARS];
acsstore_t ACSStore[MAX_ACS_STORE+1]; // +1 for termination marker

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, ACS)

static byte *ActionCodeBase;
static VACS *ACScript;
static int *PCodePtr;
static int SpecArgs[8];
static int ACStringCount;
static char **ACStrings;
static char PrintBuffer[PRINT_BUFFER_SIZE];
static VACS *NewScript;

static FFunction *pf_TagBusy;

// CODE --------------------------------------------------------------------

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

//==========================================================================
//
// P_LoadACScripts
//
//==========================================================================

void P_LoadACScripts(boolean spawn_thinkers)
{
	guard(P_LoadACScripts);
	int i;
	int *buffer;
	acsHeader_t *header;
	acsInfo_t *info;

	pf_TagBusy = svpr.FuncForName("TagBusy");

	if (!GLevel->Behavior)
    {
		ACScriptCount = 0;
		return;
    }
	if (GLevel->BehaviorSize < (int)sizeof(acsHeader_t))
    {
		GCon->Log("Behavior lump too small");
		ACScriptCount = 0;
		return;
    }
	header = (acsHeader_t *)GLevel->Behavior;
	ActionCodeBase = (byte *)header;
	buffer = (int *)((byte *)header+header->infoOffset);
	ACScriptCount = *buffer++;
	if (ACScriptCount == 0)
	{
		// Empty behavior lump
		return;
	}
	ACSInfo = (acsInfo_t*)Z_Malloc(ACScriptCount*sizeof(acsInfo_t), PU_LEVEL, 0);
	memset(ACSInfo, 0, ACScriptCount*sizeof(acsInfo_t));
	for(i = 0, info = ACSInfo; i < ACScriptCount; i++, info++)
	{
		info->number = *buffer++;
		info->address = (int *)((byte *)ActionCodeBase+*buffer++);
		info->argCount = *buffer++;
		if(info->number >= OPEN_SCRIPTS_BASE)
		{ // Auto-activate
			info->number -= OPEN_SCRIPTS_BASE;
            if (spawn_thinkers)
            {
				StartOpenACS(info->number, i, info->address);
			}
			info->state = ASTE_RUNNING;
		}
		else
		{
			info->state = ASTE_INACTIVE;
		}
	}
	ACStringCount = *buffer++;
	ACStrings = (char **)buffer;
	for(i = 0; i < ACStringCount; i++)
	{
		ACStrings[i] += (int)ActionCodeBase;
	}
	memset(MapVars, 0, sizeof(MapVars));
	unguard;
}

//==========================================================================
//
// StartOpenACS
//
//==========================================================================

static void StartOpenACS(int number, int infoIndex, int *address)
{
	VACS *script;

	script = (VACS *)VObject::StaticSpawnObject(VACS::StaticClass(), NULL, PU_LEVSPEC);
	script->number = number;

	// World objects are allotted 1 second for initialization
	script->DelayTime = 1.0;

	script->infoIndex = infoIndex;
	script->ip = address;
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
			P_StartACS(store->script, 0, store->args, NULL, NULL, 0);
			if (NewScript)
			{
				NewScript->DelayTime = 1.0;
			}
			strcpy(store->map, "-");
		}
	}
	unguard;
}

//==========================================================================
//
// P_StartACS
//
//==========================================================================

boolean P_StartACS(int number, int map_num, int *args, VEntity *activator,
	line_t *line, int side)
{
	guard(P_StartACS);
	int i;
	VACS *script;
	int infoIndex;
	aste_t *statePtr;
	char map[12] = "";

	if (map_num)
	{
		strcpy(map, SV_GetMapName(map_num));
	}

	NewScript = NULL;
	if (map[0] && strcmp(map, level.mapname))
	{ // Add to the script store
		return AddToACSStore(map, number, args);
	}
	infoIndex = GetACSIndex(number);
	if(infoIndex == -1)
	{
		// Script not found
		GCon->Logf(NAME_Dev, "P_StartACS ERROR: Unknown script %d", number);
		return false;
	}
	statePtr = &ACSInfo[infoIndex].state;
	if(*statePtr == ASTE_SUSPENDED)
	{ // Resume a suspended script
		*statePtr = ASTE_RUNNING;
		return true;
	}
	if(*statePtr != ASTE_INACTIVE)
	{ // Script is already executing
		return false;
	}
	script = (VACS *)VObject::StaticSpawnObject(VACS::StaticClass(), NULL, PU_LEVSPEC);
	script->number = number;
	script->infoIndex = infoIndex;
	script->Activator = (VEntity *)activator;
	script->line = line;
	script->side = side;
	script->ip = ACSInfo[infoIndex].address;
	for(i = 0; i < ACSInfo[infoIndex].argCount; i++)
	{
		script->vars[i] = args[i];
	}
	*statePtr = ASTE_RUNNING;
	NewScript = script;
	return true;
	unguard;
}

//==========================================================================
//
// AddToACSStore
//
//==========================================================================

static boolean AddToACSStore(const char *map, int number, int *args)
{
	int i;
	int index;

	index = -1;
	for (i = 0; ACSStore[i].map[0]; i++)
	{
		if (ACSStore[i].script == number && !strcmp(ACSStore[i].map, map))
		{ // Don't allow duplicates
			return false;
		}
		if (index == -1 && ACSStore[i].map[0] == '-')
		{ // Remember first empty slot
			index = i;
		}
	}
	if (index == -1)
	{ // Append required
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
	ACSStore[index].args[0] = args[0];
	ACSStore[index].args[1] = args[1];
	ACSStore[index].args[2] = args[2];
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
	int infoIndex;

	infoIndex = GetACSIndex(number);
	if (infoIndex == -1)
	{ // Script not found
		return false;
	}
	if (ACSInfo[infoIndex].state == ASTE_INACTIVE ||
		ACSInfo[infoIndex].state == ASTE_TERMINATING)
	{ // States that disallow termination
		return false;
	}
	ACSInfo[infoIndex].state = ASTE_TERMINATING;
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
	int infoIndex;

	infoIndex = GetACSIndex(number);
	if (infoIndex == -1)
	{ // Script not found
		return false;
	}
	if (ACSInfo[infoIndex].state == ASTE_INACTIVE ||
		ACSInfo[infoIndex].state == ASTE_SUSPENDED ||
		ACSInfo[infoIndex].state == ASTE_TERMINATING)
	{ // States that disallow suspension
		return false;
	}
	ACSInfo[infoIndex].state = ASTE_SUSPENDED;
	return true;
	unguard;
}

//==========================================================================
//
// P_Init
//
//==========================================================================

void P_ACSInitNewGame(void)
{
	guard(P_ACSInitNewGame);
	memset(WorldVars, 0, sizeof(WorldVars));
	memset(ACSStore, 0, sizeof(ACSStore));
	unguard;
}

//==========================================================================
//
//	VAcs::Tick
//
//==========================================================================

void VACS::Tick(float DeltaTime)
{
	int cmd;
	int action;

	if (ACSInfo[infoIndex].state == ASTE_TERMINATING)
	{
		ACSInfo[infoIndex].state = ASTE_INACTIVE;
		ScriptFinished(number);
		ConditionalDestroy();
		return;
	}
	if (ACSInfo[infoIndex].state != ASTE_RUNNING)
	{
		return;
	}
	if (DelayTime)
	{
		DelayTime -= DeltaTime;
		if (DelayTime < 0)
			DelayTime = 0;
		return;
	}
	ACScript = this;
	PCodePtr = ip;
	action = SCRIPT_CONTINUE;
	do
	{
		cmd = *PCodePtr++;
		switch (cmd)
		{
		//	Standard P-Code commands.
		case PCD_Nop:
			break;

		case PCD_Terminate:
			action = SCRIPT_TERMINATE;
			break;

		case PCD_Suspend:
			ACSInfo[infoIndex].state = ASTE_SUSPENDED;
			action = SCRIPT_STOP;
			break;

		case PCD_PushNumber:
			Push(*PCodePtr++);
			break;

		case PCD_LSpec1:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[0] = Pop();
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec2:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[1] = Pop();
				SpecArgs[0] = Pop();
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec3:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[2] = Pop();
				SpecArgs[1] = Pop();
				SpecArgs[0] = Pop();
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec4:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[3] = Pop();
				SpecArgs[2] = Pop();
				SpecArgs[1] = Pop();
				SpecArgs[0] = Pop();
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec5:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[4] = Pop();
				SpecArgs[3] = Pop();
				SpecArgs[2] = Pop();
				SpecArgs[1] = Pop();
				SpecArgs[0] = Pop();
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec1Direct:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[0] = *PCodePtr++;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec2Direct:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[0] = *PCodePtr++;
				SpecArgs[1] = *PCodePtr++;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec3Direct:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[0] = *PCodePtr++;
				SpecArgs[1] = *PCodePtr++;
				SpecArgs[2] = *PCodePtr++;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec4Direct:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[0] = *PCodePtr++;
				SpecArgs[1] = *PCodePtr++;
				SpecArgs[2] = *PCodePtr++;
				SpecArgs[3] = *PCodePtr++;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_LSpec5Direct:
			{
				int special;
			
				special = *PCodePtr++;
				SpecArgs[0] = *PCodePtr++;
				SpecArgs[1] = *PCodePtr++;
				SpecArgs[2] = *PCodePtr++;
				SpecArgs[3] = *PCodePtr++;
				SpecArgs[4] = *PCodePtr++;
				P_ExecuteLineSpecial(special, SpecArgs, line, side, Activator);
			}
			break;

		case PCD_Add:
			Push(Pop() + Pop());
			break;

		case PCD_Subtract:
			{
				int operand2;
			
				operand2 = Pop();
				Push(Pop() - operand2);
			}
			break;

		case PCD_Multiply:
			Push(Pop() * Pop());
			break;

		case PCD_Divide:
			{
				int operand2;
			
				operand2 = Pop();
				Push(Pop() / operand2);
			}
			break;

		case PCD_Modulus:
			{
				int operand2;
			
				operand2 = Pop();
				Push(Pop() % operand2);
			}
			break;

		case PCD_EQ:
			Push(Pop() == Pop());
			break;

		case PCD_NE:
			Push(Pop() != Pop());
			break;

		case PCD_LT:
			{
				int operand2;
			
				operand2 = Pop();
				Push(Pop() < operand2);
			}
			break;

		case PCD_GT:
			{
				int operand2;
			
				operand2 = Pop();
				Push(Pop() > operand2);
			}
			break;

		case PCD_LE:
			{
				int operand2;
			
				operand2 = Pop();
				Push(Pop() <= operand2);
			}
			break;

		case PCD_GE:
			{
				int operand2;
			
				operand2 = Pop();
				Push(Pop() >= operand2);
			}
			break;

		case PCD_AssignScriptVar:
			vars[*PCodePtr++] = Pop();
			break;

		case PCD_AssignMapVar:
			MapVars[*PCodePtr++] = Pop();
			break;

		case PCD_AssignWorldVar:
			WorldVars[*PCodePtr++] = Pop();
			break;

		case PCD_PushScriptVar:
			Push(vars[*PCodePtr++]);
			break;

		case PCD_PushMapVar:
			Push(MapVars[*PCodePtr++]);
			break;

		case PCD_PushWorldVar:
			Push(WorldVars[*PCodePtr++]);
			break;

		case PCD_AddScriptVar:
			vars[*PCodePtr++] += Pop();
			break;

		case PCD_AddMapVar:
			MapVars[*PCodePtr++] += Pop();
			break;

		case PCD_AddWorldVar:
			WorldVars[*PCodePtr++] += Pop();
			break;

		case PCD_SubScriptVar:
			vars[*PCodePtr++] -= Pop();
			break;

		case PCD_SubMapVar:
			MapVars[*PCodePtr++] -= Pop();
			break;

		case PCD_SubWorldVar:
			WorldVars[*PCodePtr++] -= Pop();
			break;

		case PCD_MulScriptVar:
			vars[*PCodePtr++] *= Pop();
			break;

		case PCD_MulMapVar:
			MapVars[*PCodePtr++] *= Pop();
			break;

		case PCD_MulWorldVar:
			WorldVars[*PCodePtr++] *= Pop();
			break;

		case PCD_DivScriptVar:
			vars[*PCodePtr++] /= Pop();
			break;

		case PCD_DivMapVar:
			MapVars[*PCodePtr++] /= Pop();
			break;

		case PCD_DivWorldVar:
			WorldVars[*PCodePtr++] /= Pop();
			break;

		case PCD_ModScriptVar:
			vars[*PCodePtr++] %= Pop();
			break;

		case PCD_ModMapVar:
			MapVars[*PCodePtr++] %= Pop();
			break;

		case PCD_ModWorldVar:
			WorldVars[*PCodePtr++] %= Pop();
			break;

		case PCD_IncScriptVar:
			vars[*PCodePtr++]++;
			break;

		case PCD_IncMapVar:
			MapVars[*PCodePtr++]++;
			break;

		case PCD_IncWorldVar:
			WorldVars[*PCodePtr++]++;
			break;

		case PCD_DecScriptVar:
			vars[*PCodePtr++]--;
			break;

		case PCD_DecMapVar:
			MapVars[*PCodePtr++]--;
			break;

		case PCD_DecWorldVar:
			WorldVars[*PCodePtr++]--;
			break;

		case PCD_Goto:
			PCodePtr = (int *)(ActionCodeBase+*PCodePtr);
			break;

		case PCD_IfGoto:
			if (Pop())
			{
				PCodePtr = (int*)(ActionCodeBase + *PCodePtr);
			}
			else
			{
				PCodePtr++;
			}
			break;

		case PCD_Drop:
			Drop();
			break;

		case PCD_Delay:
			DelayTime = float(Pop()) / 35.0;
			action = SCRIPT_STOP;
			break;

		case PCD_DelayDirect:
			DelayTime = float(*PCodePtr++) / 35.0;
			action = SCRIPT_STOP;
			break;

		case PCD_Random:
			{
				int low;
				int high;
			
				high = Pop();
				low = Pop();
				Push(low + (int)(Random() * (high - low + 1)));
			}
			break;

		case PCD_RandomDirect:
			{
				int low;
				int high;
			
				low = *PCodePtr++;
				high = *PCodePtr++;
				Push(low + (int)(Random() * (high - low + 1)));
			}
			break;

		case PCD_ThingCount:
			{
				int tid;
			
				tid = Pop();
				Push(ThingCount(Pop(), tid));
			}
			break;

		case PCD_ThingCountDirect:
			{
				int type;
			
				type = *PCodePtr++;
				Push(ThingCount(type, *PCodePtr++));
			}
			break;

		case PCD_TagWait:
			ACSInfo[infoIndex].waitValue = Pop();
			ACSInfo[infoIndex].state = ASTE_WAITINGFORTAG;
			action = SCRIPT_STOP;
			break;

		case PCD_TagWaitDirect:
			ACSInfo[infoIndex].waitValue = *PCodePtr++;
			ACSInfo[infoIndex].state = ASTE_WAITINGFORTAG;
			action = SCRIPT_STOP;
			break;

		case PCD_PolyWait:
			ACSInfo[infoIndex].waitValue = Pop();
			ACSInfo[infoIndex].state = ASTE_WAITINGFORPOLY;
			action = SCRIPT_STOP;
			break;

		case PCD_PolyWaitDirect:
			ACSInfo[infoIndex].waitValue = *PCodePtr++;
			ACSInfo[infoIndex].state = ASTE_WAITINGFORPOLY;
			action = SCRIPT_STOP;
			break;

		case PCD_ChangeFloor:
			{
				int tag;
				int flat;
				int sectorIndex;
			
				flat = R_FlatNumForName(ACStrings[Pop()]);
				tag = Pop();
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
			
				tag = *PCodePtr++;
				flat = R_FlatNumForName(ACStrings[*PCodePtr++]);
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
			
				flat = R_FlatNumForName(ACStrings[Pop()]);
				tag = Pop();
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
			
				tag = *PCodePtr++;
				flat = R_FlatNumForName(ACStrings[*PCodePtr++]);
				sectorIndex = -1;
				while ((sectorIndex = FindSectorFromTag(tag, sectorIndex)) >= 0)
				{
					SV_SetCeilPic(sectorIndex, flat);
				}
			}
			break;

		case PCD_Restart:
			PCodePtr = ACSInfo[infoIndex].address;
			break;

		case PCD_AndLogical:
			Push(Pop() && Pop());
			break;

		case PCD_OrLogical:
			Push(Pop() || Pop());
			break;

		case PCD_AndBitwise:
			Push(Pop() & Pop());
			break;

		case PCD_OrBitwise:
			Push(Pop() | Pop());
			break;

		case PCD_EorBitwise:
			Push(Pop() ^ Pop());
			break;

		case PCD_NegateLogical:
			Push(!Pop());
			break;

		case PCD_LShift:
			{
				int operand2;

				operand2 = Pop();
				Push(Pop() << operand2);
			}
			break;

		case PCD_RShift:
			{
				int operand2;
			
				operand2 = Pop();
				Push(Pop() >> operand2);
			}
			break;

		case PCD_UnaryMinus:
			Push(-Pop());
			break;

		case PCD_IfNotGoto:
			if (Pop())
			{
				PCodePtr++;
			}
			else
			{
				PCodePtr = (int*)(ActionCodeBase + *PCodePtr);
			}
			break;

		case PCD_LineSide:
			Push(side);
			break;

		case PCD_ScriptWait:
			ACSInfo[infoIndex].waitValue = Pop();
			ACSInfo[infoIndex].state = ASTE_WAITINGFORSCRIPT;
			action = SCRIPT_STOP;
			break;

		case PCD_ScriptWaitDirect:
			ACSInfo[infoIndex].waitValue = *PCodePtr++;
			ACSInfo[infoIndex].state = ASTE_WAITINGFORSCRIPT;
			action = SCRIPT_STOP;
			break;

		case PCD_ClearLineSpecial:
			if (line)
			{
				line->special = 0;
			}
			break;

		case PCD_CaseGoto:
			if (Top() == *PCodePtr++)
			{
				PCodePtr = (int*)(ActionCodeBase + *PCodePtr);
				Drop();
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
			{
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
			}
			break;

		case PCD_PrintString:
			strcat(PrintBuffer, ACStrings[Pop()]);
			break;

		case PCD_PrintNumber:
			{
				char tempStr[16];
			
				sprintf(tempStr, "%d", Pop());
				strcat(PrintBuffer, tempStr);
			}
			break;

		case PCD_PrintCharacter:
			{
				char *bufferEnd;
			
				bufferEnd = PrintBuffer + strlen(PrintBuffer);
				*bufferEnd++ = Pop();
				*bufferEnd = 0;
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
				Push(count);
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
				Push(gametype);
			}
			break;

		case PCD_GameSkill:
			Push(gameskill);
			break;

		case PCD_Timer:
			Push(level.tictime);
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
				volume = Pop();
				SV_SectorStartSound(sector, S_GetSoundID(ACStrings[Pop()]), 0, volume);
			}
			break;

		case PCD_AmbientSound:
			{
				int volume;
			
				volume = Pop();
				SV_StartSound(NULL, S_GetSoundID(ACStrings[Pop()]), 0, volume);
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
				SV_SectorStartSequence(sec, ACStrings[Pop()]);
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
			
				texture = R_TextureNumForName(ACStrings[Pop()]);
				position = Pop();
				side = Pop();
				lineTag = Pop();
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
			
				blocking = Pop() ? ML_BLOCKING : 0;
				lineTag = Pop();
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
			
				arg5 = Pop();
				arg4 = Pop();
				arg3 = Pop();
				arg2 = Pop();
				arg1 = Pop();
				special = Pop();
				lineTag = Pop();
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
			
				volume = Pop();
				sound = S_GetSoundID(ACStrings[Pop()]);
				tid = Pop();
				searcher = -1;
				while ((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
				{
					SV_StartSound(mobj, sound, 0, volume);
				}
			}
			break;

		case PCD_EndPrintBold:
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

		default:
			Host_Error("Illegal ACS opcode %d", cmd);
			break;
		}
	} while  (action == SCRIPT_CONTINUE);
	ip = PCodePtr;
	if (action == SCRIPT_TERMINATE)
	{
		ACSInfo[infoIndex].state = ASTE_INACTIVE;
		ScriptFinished(number);
		ConditionalDestroy();
	}
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
	acs->ip = (int *)((int)(acs->ip) - (int)ActionCodeBase);
	acs->line = acs->line ? (line_t *)(acs->line - acs->XLevel->Lines) : (line_t *)-1;
	acs->Activator = (VEntity *)GetMobjNum(acs->Activator);
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
	acs->ip = (int *)(ActionCodeBase + (int)acs->ip);
	if ((int)acs->line == -1)
	{
		acs->line = NULL;
	}
	else
	{
		acs->line = &acs->XLevel->Lines[(int)acs->line];
	}
	acs->Activator = (VEntity *)SetMobjPtr((int)acs->Activator);
	unguard;
}

//==========================================================================
//
// P_TagFinished
//
//==========================================================================

void P_TagFinished(int tag)
{
	guard(P_TagFinished);
	int i;

	if (TagBusy(tag))
	{
		return;
	}
	for (i = 0; i < ACScriptCount; i++)
	{
		if (ACSInfo[i].state == ASTE_WAITINGFORTAG
			&& ACSInfo[i].waitValue == tag)
		{
			ACSInfo[i].state = ASTE_RUNNING;
		}
	}
	unguard;
}

//==========================================================================
//
// P_PolyobjFinished
//
//==========================================================================

void P_PolyobjFinished(int po)
{
	guard(P_PolyobjFinished);
	int i;

	if (PO_Busy(po))
	{
		return;
	}
	for (i = 0; i < ACScriptCount; i++)
	{
		if (ACSInfo[i].state == ASTE_WAITINGFORPOLY
			&& ACSInfo[i].waitValue == po)
		{
			ACSInfo[i].state = ASTE_RUNNING;
		}
	}
	unguard;
}

//==========================================================================
//
// ScriptFinished
//
//==========================================================================

static void ScriptFinished(int number)
{
	int i;

	for(i = 0; i < ACScriptCount; i++)
	{
		if(ACSInfo[i].state == ASTE_WAITINGFORSCRIPT
			&& ACSInfo[i].waitValue == number)
		{
			ACSInfo[i].state = ASTE_RUNNING;
		}
	}
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

static boolean TagBusy(int tag)
{
	return svpr.Exec(pf_TagBusy, tag);
}

//==========================================================================
//
// GetACSIndex
//
// Returns the index of a script number.  Returns -1 if the script number
// is not found.
//
//==========================================================================

static int GetACSIndex(int number)
{
	int i;

	for(i = 0; i < ACScriptCount; i++)
	{
		if(ACSInfo[i].number == number)
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
// Push
//
//==========================================================================

static void Push(int value)
{
	ACScript->stack[ACScript->stackPtr++] = value;
}

//==========================================================================
//
// Pop
//
//==========================================================================

static int Pop(void)
{
	return ACScript->stack[--ACScript->stackPtr];
}

//==========================================================================
//
// Top
//
//==========================================================================

static int Top(void)
{
	return ACScript->stack[ACScript->stackPtr-1];
}

//==========================================================================
//
// Drop
//
//==========================================================================

static void Drop(void)
{
	ACScript->stackPtr--;
}

//**************************************************************************
//
//	$Log$
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
