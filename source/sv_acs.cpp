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

struct acsHeader_t
{
	int marker;
	int infoOffset;
	int code;
};

class VACS:public VThinker
{
	DECLARE_CLASS(VACS, VThinker, 0)
	NO_DEFAULT_CONSTRUCTOR(VACS)

	VMapObject 	*activator;
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

static int CmdNOP(void);
static int CmdTerminate(void);
static int CmdSuspend(void);
static int CmdPushNumber(void);
static int CmdLSpec1(void);
static int CmdLSpec2(void);
static int CmdLSpec3(void);
static int CmdLSpec4(void);
static int CmdLSpec5(void);
static int CmdLSpec1Direct(void);
static int CmdLSpec2Direct(void);
static int CmdLSpec3Direct(void);
static int CmdLSpec4Direct(void);
static int CmdLSpec5Direct(void);
static int CmdAdd(void);
static int CmdSubtract(void);
static int CmdMultiply(void);
static int CmdDivide(void);
static int CmdModulus(void);
static int CmdEQ(void);
static int CmdNE(void);
static int CmdLT(void);
static int CmdGT(void);
static int CmdLE(void);
static int CmdGE(void);
static int CmdAssignScriptVar(void);
static int CmdAssignMapVar(void);
static int CmdAssignWorldVar(void);
static int CmdPushScriptVar(void);
static int CmdPushMapVar(void);
static int CmdPushWorldVar(void);
static int CmdAddScriptVar(void);
static int CmdAddMapVar(void);
static int CmdAddWorldVar(void);
static int CmdSubScriptVar(void);
static int CmdSubMapVar(void);
static int CmdSubWorldVar(void);
static int CmdMulScriptVar(void);
static int CmdMulMapVar(void);
static int CmdMulWorldVar(void);
static int CmdDivScriptVar(void);
static int CmdDivMapVar(void);
static int CmdDivWorldVar(void);
static int CmdModScriptVar(void);
static int CmdModMapVar(void);
static int CmdModWorldVar(void);
static int CmdIncScriptVar(void);
static int CmdIncMapVar(void);
static int CmdIncWorldVar(void);
static int CmdDecScriptVar(void);
static int CmdDecMapVar(void);
static int CmdDecWorldVar(void);
static int CmdGoto(void);
static int CmdIfGoto(void);
static int CmdDrop(void);
static int CmdDelay(void);
static int CmdDelayDirect(void);
static int CmdRandom(void);
static int CmdRandomDirect(void);
static int CmdThingCount(void);
static int CmdThingCountDirect(void);
static int CmdTagWait(void);
static int CmdTagWaitDirect(void);
static int CmdPolyWait(void);
static int CmdPolyWaitDirect(void);
static int CmdChangeFloor(void);
static int CmdChangeFloorDirect(void);
static int CmdChangeCeiling(void);
static int CmdChangeCeilingDirect(void);
static int CmdRestart(void);
static int CmdAndLogical(void);
static int CmdOrLogical(void);
static int CmdAndBitwise(void);
static int CmdOrBitwise(void);
static int CmdEorBitwise(void);
static int CmdNegateLogical(void);
static int CmdLShift(void);
static int CmdRShift(void);
static int CmdUnaryMinus(void);
static int CmdIfNotGoto(void);
static int CmdLineSide(void);
static int CmdScriptWait(void);
static int CmdScriptWaitDirect(void);
static int CmdClearLineSpecial(void);
static int CmdCaseGoto(void);
static int CmdBeginPrint(void);
static int CmdEndPrint(void);
static int CmdPrintString(void);
static int CmdPrintNumber(void);
static int CmdPrintCharacter(void);
static int CmdPlayerCount(void);
static int CmdGameType(void);
static int CmdGameSkill(void);
static int CmdTimer(void);
static int CmdSectorSound(void);
static int CmdAmbientSound(void);
static int CmdSoundSequence(void);
static int CmdSetLineTexture(void);
static int CmdSetLineBlocking(void);
static int CmdSetLineSpecial(void);
static int CmdThingSound(void);
static int CmdEndPrintBold(void);

static void ThingCount(int type, int tid);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int ACScriptCount;
acsInfo_t *ACSInfo;
int MapVars[MAX_ACS_MAP_VARS];
int WorldVars[MAX_ACS_WORLD_VARS];
acsstore_t ACSStore[MAX_ACS_STORE+1]; // +1 for termination marker

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(VACS)

static byte *ActionCodeBase;
static VACS *ACScript;
static int *PCodePtr;
static int SpecArgs[8];
static int ACStringCount;
static char **ACStrings;
static char PrintBuffer[PRINT_BUFFER_SIZE];
static VACS *NewScript;

static FFunction *pf_TagBusy;

static int (*PCodeCmds[])(void) =
{
	CmdNOP,
	CmdTerminate,
	CmdSuspend,
	CmdPushNumber,
	CmdLSpec1,
	CmdLSpec2,
	CmdLSpec3,
	CmdLSpec4,
	CmdLSpec5,
	CmdLSpec1Direct,
	CmdLSpec2Direct,
	CmdLSpec3Direct,
	CmdLSpec4Direct,
	CmdLSpec5Direct,
	CmdAdd,
	CmdSubtract,
	CmdMultiply,
	CmdDivide,
	CmdModulus,
	CmdEQ,
	CmdNE,
	CmdLT,
	CmdGT,
	CmdLE,
	CmdGE,
	CmdAssignScriptVar,
	CmdAssignMapVar,
	CmdAssignWorldVar,
	CmdPushScriptVar,
	CmdPushMapVar,
	CmdPushWorldVar,
	CmdAddScriptVar,
	CmdAddMapVar,
	CmdAddWorldVar,
	CmdSubScriptVar,
	CmdSubMapVar,
	CmdSubWorldVar,
	CmdMulScriptVar,
	CmdMulMapVar,
	CmdMulWorldVar,
	CmdDivScriptVar,
	CmdDivMapVar,
	CmdDivWorldVar,
	CmdModScriptVar,
	CmdModMapVar,
	CmdModWorldVar,
	CmdIncScriptVar,
	CmdIncMapVar,
	CmdIncWorldVar,
	CmdDecScriptVar,
	CmdDecMapVar,
	CmdDecWorldVar,
	CmdGoto,
	CmdIfGoto,
	CmdDrop,
	CmdDelay,
	CmdDelayDirect,
	CmdRandom,
	CmdRandomDirect,
	CmdThingCount,
	CmdThingCountDirect,
	CmdTagWait,
	CmdTagWaitDirect,
	CmdPolyWait,
	CmdPolyWaitDirect,
	CmdChangeFloor,
	CmdChangeFloorDirect,
	CmdChangeCeiling,
	CmdChangeCeilingDirect,
	CmdRestart,
	CmdAndLogical,
	CmdOrLogical,
	CmdAndBitwise,
	CmdOrBitwise,
	CmdEorBitwise,
	CmdNegateLogical,
	CmdLShift,
	CmdRShift,
	CmdUnaryMinus,
	CmdIfNotGoto,
	CmdLineSide,
	CmdScriptWait,
	CmdScriptWaitDirect,
	CmdClearLineSpecial,
	CmdCaseGoto,
	CmdBeginPrint,
	CmdEndPrint,
	CmdPrintString,
	CmdPrintNumber,
	CmdPrintCharacter,
	CmdPlayerCount,
	CmdGameType,
	CmdGameSkill,
	CmdTimer,
	CmdSectorSound,
	CmdAmbientSound,
	CmdSoundSequence,
	CmdSetLineTexture,
	CmdSetLineBlocking,
	CmdSetLineSpecial,
	CmdThingSound,
	CmdEndPrintBold
};

// CODE --------------------------------------------------------------------

static boolean P_ExecuteLineSpecial(int special, int *args, line_t *line, int side,
	VMapObject *mo)
{
   	return svpr.Exec("ExecuteLineSpecial",
   		special, (int)args, (int)line, side, (int)mo);
}

static line_t *P_FindLine(int lineTag, int *searchPosition)
{
	return (line_t*)svpr.Exec("P_FindLine", lineTag, (int)searchPosition);
}

static VMapObject *P_FindMobjFromTID(int tid, int *searchPosition)
{
	return (VMapObject*)svpr.Exec("FindMobjFromTID", tid, (int)searchPosition);
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

	if (!level.behavior)
    {
		ACScriptCount = 0;
		return;
    }
	header = (acsHeader_t *)level.behavior;
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

boolean P_StartACS(int number, int map_num, int *args, VMapObject *activator,
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
	{ // Script not found
		//I_Error("P_StartACS: Unknown script number %d", number);
		con << "P_STARTACS ERROR: UNKNOWN SCRIPT " << number << endl;
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
	script->activator = activator;
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
		Destroy();
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
	do
	{
		cmd = *PCodePtr++;
		action = PCodeCmds[cmd]();
	} while  (action == SCRIPT_CONTINUE);
	ip = PCodePtr;
	if (action == SCRIPT_TERMINATE)
	{
		ACSInfo[infoIndex].state = ASTE_INACTIVE;
		ScriptFinished(number);
		Destroy();
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
	acs->line = acs->line ? (line_t *)(acs->line - level.lines) : (line_t *)-1;
	acs->activator = (VMapObject *)GetMobjNum(acs->activator);
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
		acs->line = &level.lines[(int)acs->line];
	}
	acs->activator = SetMobjPtr((int)acs->activator);
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
	
    for (i = start + 1; i < level.numsectors; i++)
		if (level.sectors[i].tag == tag)
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

//==========================================================================
//
// P-Code Commands
//
//==========================================================================

static int CmdNOP(void)
{
	return SCRIPT_CONTINUE;
}

static int CmdTerminate(void)
{
	return SCRIPT_TERMINATE;
}

static int CmdSuspend(void)
{
	ACSInfo[ACScript->infoIndex].state = ASTE_SUSPENDED;
	return SCRIPT_STOP;
}

static int CmdPushNumber(void)
{
	Push(*PCodePtr++);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec1(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[0] = Pop();
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec2(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec3(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[2] = Pop();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec4(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[3] = Pop();
	SpecArgs[2] = Pop();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec5(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[4] = Pop();
	SpecArgs[3] = Pop();
	SpecArgs[2] = Pop();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec1Direct(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[0] = *PCodePtr++;
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec2Direct(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[0] = *PCodePtr++;
	SpecArgs[1] = *PCodePtr++;
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec3Direct(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[0] = *PCodePtr++;
	SpecArgs[1] = *PCodePtr++;
	SpecArgs[2] = *PCodePtr++;
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec4Direct(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[0] = *PCodePtr++;
	SpecArgs[1] = *PCodePtr++;
	SpecArgs[2] = *PCodePtr++;
	SpecArgs[3] = *PCodePtr++;
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec5Direct(void)
{
	int special;

	special = *PCodePtr++;
	SpecArgs[0] = *PCodePtr++;
	SpecArgs[1] = *PCodePtr++;
	SpecArgs[2] = *PCodePtr++;
	SpecArgs[3] = *PCodePtr++;
	SpecArgs[4] = *PCodePtr++;
	P_ExecuteLineSpecial(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdAdd(void)
{
	Push(Pop()+Pop());
	return SCRIPT_CONTINUE;
}

static int CmdSubtract(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop()-operand2);
	return SCRIPT_CONTINUE;
}

static int CmdMultiply(void)
{
	Push(Pop()*Pop());
	return SCRIPT_CONTINUE;
}

static int CmdDivide(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop()/operand2);
	return SCRIPT_CONTINUE;
}

static int CmdModulus(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop()%operand2);
	return SCRIPT_CONTINUE;
}

static int CmdEQ(void)
{
	Push(Pop() == Pop());
	return SCRIPT_CONTINUE;
}

static int CmdNE(void)
{
	Push(Pop() != Pop());
	return SCRIPT_CONTINUE;
}

static int CmdLT(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop() < operand2);
	return SCRIPT_CONTINUE;
}

static int CmdGT(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop() > operand2);
	return SCRIPT_CONTINUE;
}

static int CmdLE(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop() <= operand2);
	return SCRIPT_CONTINUE;
}

static int CmdGE(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop() >= operand2);
	return SCRIPT_CONTINUE;
}

static int CmdAssignScriptVar(void)
{
	ACScript->vars[*PCodePtr++] = Pop();
	return SCRIPT_CONTINUE;
}

static int CmdAssignMapVar(void)
{
	MapVars[*PCodePtr++] = Pop();
	return SCRIPT_CONTINUE;
}

static int CmdAssignWorldVar(void)
{
	WorldVars[*PCodePtr++] = Pop();
	return SCRIPT_CONTINUE;
}

static int CmdPushScriptVar(void)
{
	Push(ACScript->vars[*PCodePtr++]);
	return SCRIPT_CONTINUE;
}

static int CmdPushMapVar(void)
{
	Push(MapVars[*PCodePtr++]);
	return SCRIPT_CONTINUE;
}

static int CmdPushWorldVar(void)
{
	Push(WorldVars[*PCodePtr++]);
	return SCRIPT_CONTINUE;
}

static int CmdAddScriptVar(void)
{
	ACScript->vars[*PCodePtr++] += Pop();
	return SCRIPT_CONTINUE;
}

static int CmdAddMapVar(void)
{
	MapVars[*PCodePtr++] += Pop();
	return SCRIPT_CONTINUE;
}

static int CmdAddWorldVar(void)
{
	WorldVars[*PCodePtr++] += Pop();
	return SCRIPT_CONTINUE;
}

static int CmdSubScriptVar(void)
{
	ACScript->vars[*PCodePtr++] -= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdSubMapVar(void)
{
	MapVars[*PCodePtr++] -= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdSubWorldVar(void)
{
	WorldVars[*PCodePtr++] -= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdMulScriptVar(void)
{
	ACScript->vars[*PCodePtr++] *= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdMulMapVar(void)
{
	MapVars[*PCodePtr++] *= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdMulWorldVar(void)
{
	WorldVars[*PCodePtr++] *= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdDivScriptVar(void)
{
	ACScript->vars[*PCodePtr++] /= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdDivMapVar(void)
{
	MapVars[*PCodePtr++] /= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdDivWorldVar(void)
{
	WorldVars[*PCodePtr++] /= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdModScriptVar(void)
{
	ACScript->vars[*PCodePtr++] %= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdModMapVar(void)
{
	MapVars[*PCodePtr++] %= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdModWorldVar(void)
{
	WorldVars[*PCodePtr++] %= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdIncScriptVar(void)
{
	ACScript->vars[*PCodePtr++]++;
	return SCRIPT_CONTINUE;
}

static int CmdIncMapVar(void)
{
	MapVars[*PCodePtr++]++;
	return SCRIPT_CONTINUE;
}

static int CmdIncWorldVar(void)
{
	WorldVars[*PCodePtr++]++;
	return SCRIPT_CONTINUE;
}

static int CmdDecScriptVar(void)
{
	ACScript->vars[*PCodePtr++]--;
	return SCRIPT_CONTINUE;
}

static int CmdDecMapVar(void)
{
	MapVars[*PCodePtr++]--;
	return SCRIPT_CONTINUE;
}

static int CmdDecWorldVar(void)
{
	WorldVars[*PCodePtr++]--;
	return SCRIPT_CONTINUE;
}

static int CmdGoto(void)
{
	PCodePtr = (int *)(ActionCodeBase+*PCodePtr);
	return SCRIPT_CONTINUE;
}

static int CmdIfGoto(void)
{
	if(Pop())
	{
		PCodePtr = (int *)(ActionCodeBase+*PCodePtr);
	}
	else
	{
		PCodePtr++;
	}
	return SCRIPT_CONTINUE;
}

static int CmdDrop(void)
{
	Drop();
	return SCRIPT_CONTINUE;
}

static int CmdDelay(void)
{
	ACScript->DelayTime = float(Pop()) / 35.0;
	return SCRIPT_STOP;
}

static int CmdDelayDirect(void)
{
	ACScript->DelayTime = float(*PCodePtr++) / 35.0;
	return SCRIPT_STOP;
}

static int CmdRandom(void)
{
	int low;
	int high;

	high = Pop();
	low = Pop();
	Push(low + (int)(Random() * (high - low + 1)));
	return SCRIPT_CONTINUE;
}

static int CmdRandomDirect(void)
{
	int low;
	int high;

	low = *PCodePtr++;
	high = *PCodePtr++;
	Push(low + (int)(Random() * (high - low + 1)));
	return SCRIPT_CONTINUE;
}

static int CmdThingCount(void)
{
	int tid;

	tid = Pop();
	ThingCount(Pop(), tid);
	return SCRIPT_CONTINUE;
}

static int CmdThingCountDirect(void)
{
	int type;

	type = *PCodePtr++;
	ThingCount(type, *PCodePtr++);
	return SCRIPT_CONTINUE;
}

static void ThingCount(int type, int tid)
{
	Push(svpr.Exec("ThingCount", type, tid));
}

static int CmdTagWait(void)
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = ASTE_WAITINGFORTAG;
	return SCRIPT_STOP;
}

static int CmdTagWaitDirect(void)
{
	ACSInfo[ACScript->infoIndex].waitValue = *PCodePtr++;
	ACSInfo[ACScript->infoIndex].state = ASTE_WAITINGFORTAG;
	return SCRIPT_STOP;
}

static int CmdPolyWait(void)
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = ASTE_WAITINGFORPOLY;
	return SCRIPT_STOP;
}

static int CmdPolyWaitDirect(void)
{
	ACSInfo[ACScript->infoIndex].waitValue = *PCodePtr++;
	ACSInfo[ACScript->infoIndex].state = ASTE_WAITINGFORPOLY;
	return SCRIPT_STOP;
}

static int CmdChangeFloor(void)
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
	return SCRIPT_CONTINUE;
}

static int CmdChangeFloorDirect(void)
{
	int tag;
	int flat;
	int sectorIndex;

	tag = *PCodePtr++;
	flat = R_FlatNumForName(ACStrings[*PCodePtr++]);
	sectorIndex = -1;
	while((sectorIndex = FindSectorFromTag(tag, sectorIndex)) >= 0)
	{
		SV_SetFloorPic(sectorIndex, flat);
	}
	return SCRIPT_CONTINUE;
}

static int CmdChangeCeiling(void)
{
	int tag;
	int flat;
	int sectorIndex;

	flat = R_FlatNumForName(ACStrings[Pop()]);
	tag = Pop();
	sectorIndex = -1;
	while((sectorIndex = FindSectorFromTag(tag, sectorIndex)) >= 0)
	{
		SV_SetCeilPic(sectorIndex, flat);
	}
	return SCRIPT_CONTINUE;
}

static int CmdChangeCeilingDirect(void)
{
	int tag;
	int flat;
	int sectorIndex;

	tag = *PCodePtr++;
	flat = R_FlatNumForName(ACStrings[*PCodePtr++]);
	sectorIndex = -1;
	while((sectorIndex = FindSectorFromTag(tag, sectorIndex)) >= 0)
	{
		SV_SetCeilPic(sectorIndex, flat);
	}
	return SCRIPT_CONTINUE;
}

static int CmdRestart(void)
{
	PCodePtr = ACSInfo[ACScript->infoIndex].address;
	return SCRIPT_CONTINUE;
}

static int CmdAndLogical(void)
{
	Push(Pop() && Pop());
	return SCRIPT_CONTINUE;
}

static int CmdOrLogical(void)
{
	Push(Pop() || Pop());
	return SCRIPT_CONTINUE;
}

static int CmdAndBitwise(void)
{
	Push(Pop()&Pop());
	return SCRIPT_CONTINUE;
}

static int CmdOrBitwise(void)
{
	Push(Pop()|Pop());
	return SCRIPT_CONTINUE;
}

static int CmdEorBitwise(void)
{
	Push(Pop()^Pop());
	return SCRIPT_CONTINUE;
}

static int CmdNegateLogical(void)
{
	Push(!Pop());
	return SCRIPT_CONTINUE;
}

static int CmdLShift(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop()<<operand2);
	return SCRIPT_CONTINUE;
}

static int CmdRShift(void)
{
	int operand2;

	operand2 = Pop();
	Push(Pop()>>operand2);
	return SCRIPT_CONTINUE;
}

static int CmdUnaryMinus(void)
{
	Push(-Pop());
	return SCRIPT_CONTINUE;
}

static int CmdIfNotGoto(void)
{
	if(Pop())
	{
		PCodePtr++;
	}
	else
	{
		PCodePtr = (int *)(ActionCodeBase+*PCodePtr);
	}
	return SCRIPT_CONTINUE;
}

static int CmdLineSide(void)
{
	Push(ACScript->side);
	return SCRIPT_CONTINUE;
}

static int CmdScriptWait(void)
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = ASTE_WAITINGFORSCRIPT;
	return SCRIPT_STOP;
}

static int CmdScriptWaitDirect(void)
{
	ACSInfo[ACScript->infoIndex].waitValue = *PCodePtr++;
	ACSInfo[ACScript->infoIndex].state = ASTE_WAITINGFORSCRIPT;
	return SCRIPT_STOP;
}

static int CmdClearLineSpecial(void)
{
	if(ACScript->line)
	{
		ACScript->line->special = 0;
	}
	return SCRIPT_CONTINUE;
}

static int CmdCaseGoto(void)
{
	if(Top() == *PCodePtr++)
	{
		PCodePtr = (int *)(ActionCodeBase+*PCodePtr);
		Drop();
	}
	else
	{
		PCodePtr++;
	}
	return SCRIPT_CONTINUE;
}

static int CmdBeginPrint(void)
{
	*PrintBuffer = 0;
	return SCRIPT_CONTINUE;
}

static int CmdEndPrint(void)
{
	if (ACScript->activator && ACScript->activator->bIsPlayer)
	{
		SV_ClientCenterPrintf(ACScript->activator->Player, "%s\n", PrintBuffer);
	}
	else
	{
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (players[i].bActive)
			{
				SV_ClientCenterPrintf(&players[i], "%s\n", PrintBuffer);
			}
		}
	}
	return SCRIPT_CONTINUE;
}

static int CmdEndPrintBold(void)
{//FIXME yellow message
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (players[i].bActive)
		{
			SV_ClientCenterPrintf(&players[i], "%s\n", PrintBuffer);
		}
	}
	return SCRIPT_CONTINUE;
}

static int CmdPrintString(void)
{
	strcat(PrintBuffer, ACStrings[Pop()]);
	return SCRIPT_CONTINUE;
}

static int CmdPrintNumber(void)
{
	char tempStr[16];

	sprintf(tempStr, "%d", Pop());
	strcat(PrintBuffer, tempStr);
	return SCRIPT_CONTINUE;
}

static int CmdPrintCharacter(void)
{
	char *bufferEnd;

	bufferEnd = PrintBuffer+strlen(PrintBuffer);
	*bufferEnd++ = Pop();
	*bufferEnd = 0;
	return SCRIPT_CONTINUE;
}

static int CmdPlayerCount(void)
{
	int i;
	int count;

	count = 0;
	for(i = 0; i < MAXPLAYERS; i++)
	{
		count += players[i].bActive;
	}
	Push(count);
	return SCRIPT_CONTINUE;
}

static int CmdGameType(void)
{
	int gametype;

	if(netgame == false)
	{
		gametype = GAME_SINGLE_PLAYER;
	}
	else if(deathmatch)
	{
		gametype = GAME_NET_DEATHMATCH;
	}
	else
	{
		gametype = GAME_NET_COOPERATIVE;
	}
	Push(gametype);
	return SCRIPT_CONTINUE;
}

static int CmdGameSkill(void)
{
	Push(gameskill);
	return SCRIPT_CONTINUE;
}

static int CmdTimer(void)
{
	Push(level.tictime);
	return SCRIPT_CONTINUE;
}

static int CmdSectorSound(void)
{
	int volume;
	sector_t *sector;

	sector = NULL;
	if (ACScript->line)
	{
		sector = ACScript->line->frontsector;
	}
	volume = Pop();
	SV_SectorStartSound(sector, S_GetSoundID(ACStrings[Pop()]), 0, volume);
	return SCRIPT_CONTINUE;
}

static int CmdThingSound(void)
{
	int tid;
	int sound;
	int volume;
	VMapObject *mobj;
	int searcher;

	volume = Pop();
	sound = S_GetSoundID(ACStrings[Pop()]);
	tid = Pop();
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
	{
		SV_StartSound(mobj, sound, 0, volume);
	}
	return SCRIPT_CONTINUE;
}

static int CmdAmbientSound(void)
{
	int volume;

	volume = Pop();
	SV_StartSound(NULL, S_GetSoundID(ACStrings[Pop()]), 0, volume);
	return SCRIPT_CONTINUE;
}

static int CmdSoundSequence(void)
{
	sector_t *sec;

	sec = NULL;
	if (ACScript->line)
	{
		sec = ACScript->line->frontsector;
	}
	SV_SectorStartSequence(sec, ACStrings[Pop()]);
	return SCRIPT_CONTINUE;
}

static int CmdSetLineTexture(void)
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
	while((line = P_FindLine(lineTag, &searcher)) != NULL)
	{
		SV_SetLineTexture(line->sidenum[side], position, texture);
	}
	return SCRIPT_CONTINUE;
}

static int CmdSetLineBlocking(void)
{
	line_t *line;
	int lineTag;
	boolean blocking;
	int searcher;

	blocking = Pop() ? ML_BLOCKING : 0;
	lineTag = Pop();
	searcher = -1;
	while((line = P_FindLine(lineTag, &searcher)) != NULL)
	{
		line->flags = (line->flags&~ML_BLOCKING)|blocking;
	}
	return SCRIPT_CONTINUE;
}

static int CmdSetLineSpecial(void)
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
	while((line = P_FindLine(lineTag, &searcher)) != NULL)
	{
		line->special = special;
		line->arg1 = arg1;
		line->arg2 = arg2;
		line->arg3 = arg3;
		line->arg4 = arg4;
		line->arg5 = arg5;
	}
	return SCRIPT_CONTINUE;
}

//**************************************************************************
//
//	$Log$
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
