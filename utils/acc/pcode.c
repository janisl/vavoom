
//**************************************************************************
//**
//** pcode.c
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <string.h>
#include <stddef.h>
#include "pcode.h"
#include "common.h"
#include "error.h"
#include "misc.h"
#include "strlist.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef struct scriptInfo_s
{
	int number;
	int address;
	int argCount;
} scriptInfo_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void Append(void *buffer, size_t size);
static void Write(void *buffer, size_t size, int address);
static void Skip(size_t size);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int pc_Address;
byte *pc_Buffer;
byte *pc_BufferPtr;
int pc_ScriptCount;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int BufferSize;
static boolean ObjectOpened = NO;
static scriptInfo_t ScriptInfo[MAX_SCRIPT_COUNT];
static char ObjectName[MAX_FILE_NAME_LENGTH];
static int ObjectFlags;

static char *PCDNames[PCODE_COMMAND_COUNT] =
{
	"PCD_NOP",
	"PCD_TERMINATE",
	"PCD_SUSPEND",
	"PCD_PUSHNUMBER",
	"PCD_LSPEC1",
	"PCD_LSPEC2",
	"PCD_LSPEC3",
	"PCD_LSPEC4",
	"PCD_LSPEC5",
	"PCD_LSPEC1DIRECT",
	"PCD_LSPEC2DIRECT",
	"PCD_LSPEC3DIRECT",
	"PCD_LSPEC4DIRECT",
	"PCD_LSPEC5DIRECT",
	"PCD_ADD",
	"PCD_SUBTRACT",
	"PCD_MULTIPLY",
	"PCD_DIVIDE",
	"PCD_MODULUS",
	"PCD_EQ",
	"PCD_NE",
	"PCD_LT",
	"PCD_GT",
	"PCD_LE",
	"PCD_GE",
	"PCD_ASSIGNSCRIPTVAR",
	"PCD_ASSIGNMAPVAR",
	"PCD_ASSIGNWORLDVAR",
	"PCD_PUSHSCRIPTVAR",
	"PCD_PUSHMAPVAR",
	"PCD_PUSHWORLDVAR",
	"PCD_ADDSCRIPTVAR",
	"PCD_ADDMAPVAR",
	"PCD_ADDWORLDVAR",
	"PCD_SUBSCRIPTVAR",
	"PCD_SUBMAPVAR",
	"PCD_SUBWORLDVAR",
	"PCD_MULSCRIPTVAR",
	"PCD_MULMAPVAR",
	"PCD_MULWORLDVAR",
	"PCD_DIVSCRIPTVAR",
	"PCD_DIVMAPVAR",
	"PCD_DIVWORLDVAR",
	"PCD_MODSCRIPTVAR",
	"PCD_MODMAPVAR",
	"PCD_MODWORLDVAR",
	"PCD_INCSCRIPTVAR",
	"PCD_INCMAPVAR",
	"PCD_INCWORLDVAR",
	"PCD_DECSCRIPTVAR",
	"PCD_DECMAPVAR",
	"PCD_DECWORLDVAR",
	"PCD_GOTO",
	"PCD_IFGOTO",
	"PCD_DROP",
	"PCD_DELAY",
	"PCD_DELAYDIRECT",
	"PCD_RANDOM",
	"PCD_RANDOMDIRECT",
	"PCD_THINGCOUNT",
	"PCD_THINGCOUNTDIRECT",
	"PCD_TAGWAIT",
	"PCD_TAGWAITDIRECT",
	"PCD_POLYWAIT",
	"PCD_POLYWAITDIRECT",
	"PCD_CHANGEFLOOR",
	"PCD_CHANGEFLOORDIRECT",
	"PCD_CHANGECEILING",
	"PCD_CHANGECEILINGDIRECT",
	"PCD_RESTART",
	"PCD_ANDLOGICAL",
	"PCD_ORLOGICAL",
	"PCD_ANDBITWISE",
	"PCD_ORBITWISE",
	"PCD_EORBITWISE",
	"PCD_NEGATELOGICAL",
	"PCD_LSHIFT",
	"PCD_RSHIFT",
	"PCD_UNARYMINUS",
	"PCD_IFNOTGOTO",
	"PCD_LINESIDE",
	"PCD_SCRIPTWAIT",
	"PCD_SCRIPTWAITDIRECT",
	"PCD_CLEARLINESPECIAL",
	"PCD_CASEGOTO",
	"PCD_BEGINPRINT",
	"PCD_ENDPRINT",
	"PCD_PRINTSTRING",
	"PCD_PRINTNUMBER",
	"PCD_PRINTCHARACTER",
	"PCD_PLAYERCOUNT",
	"PCD_GAMETYPE",
	"PCD_GAMESKILL",
	"PCD_TIMER",
	"PCD_SECTORSOUND",
	"PCD_AMBIENTSOUND",
	"PCD_SOUNDSEQUENCE",
	"PCD_SETLINETEXTURE",
	"PCD_SETLINEBLOCKING",
	"PCD_SETLINESPECIAL",
	"PCD_THINGSOUND",
	"PCD_ENDPRINTBOLD"
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// PC_OpenObject
//
//==========================================================================

void PC_OpenObject(char *name, size_t size, int flags)
{
	if(ObjectOpened == YES)
	{
		PC_CloseObject();
	}
	if(strlen(name) >= MAX_FILE_NAME_LENGTH)
	{
		ERR_Exit(ERR_FILE_NAME_TOO_LONG, NO, "File: \"%s\".", name);
	}
	strcpy(ObjectName, name);
	pc_Buffer = MS_Alloc(size, ERR_ALLOC_PCODE_BUFFER);
	pc_BufferPtr = pc_Buffer;
	pc_Address = 0;
	ObjectFlags = flags;
	BufferSize = size;
	pc_ScriptCount = 0;
	ObjectOpened = YES;
	PC_AppendString("ACS");
	PC_SkipLong(); // Script table offset
}

//==========================================================================
//
// PC_CloseObject
//
//==========================================================================

void PC_CloseObject(void)
{
	int i;
	scriptInfo_t *info;

	MS_Message(MSG_DEBUG, "---- PC_CloseObject ----\n");
	STR_WriteStrings();
	PC_WriteLong((U_LONG)pc_Address, 4);
	PC_AppendLong((U_LONG)pc_ScriptCount);
	for(i = 0; i < pc_ScriptCount; i++)
	{
		info = &ScriptInfo[i];
		MS_Message(MSG_DEBUG, "Script %d, address = %d, arg count = %d\n",
			info->number, info->address, info->argCount);
		PC_AppendLong((U_LONG)info->number);
		PC_AppendLong((U_LONG)info->address);
		PC_AppendLong((U_LONG)info->argCount);
	}
	STR_WriteList();
	if(MS_SaveFile(ObjectName, pc_Buffer, pc_Address) == FALSE)
	{
		ERR_Exit(ERR_SAVE_OBJECT_FAILED, NO, NULL);
	}
}

//==========================================================================
//
// PC_Append functions
//
//==========================================================================

static void Append(void *buffer, size_t size)
{
	if(pc_Address+size > BufferSize)
	{
		ERR_Exit(ERR_PCODE_BUFFER_OVERFLOW, NO, NULL);
	}
	memcpy(pc_BufferPtr, buffer, size);
	pc_BufferPtr += size;
	pc_Address += size;
}

void PC_Append(void *buffer, size_t size)
{
	MS_Message(MSG_DEBUG, "AD> %06d = (%d bytes)\n", pc_Address, size);
	Append(buffer, size);
}

/*
void PC_AppendByte(U_BYTE val)
{
	MS_Message(MSG_DEBUG, "AB> %06d = %d\n", pc_Address, val);
	Append(&val, sizeof(U_BYTE));
}
*/

/*
void PC_AppendWord(U_WORD val)
{
	MS_Message(MSG_DEBUG, "AW> %06d = %d\n", pc_Address, val);
	val = MS_LittleUWORD(val);
	Append(&val, sizeof(U_WORD));
}
*/

void PC_AppendLong(U_LONG val)
{
	MS_Message(MSG_DEBUG, "AL> %06d = %d\n", pc_Address, val);
	val = MS_LittleULONG(val);
	Append(&val, sizeof(U_LONG));
}

void PC_AppendString(char *string)
{
	int length;

	length = strlen(string)+1;
	MS_Message(MSG_DEBUG, "AS> %06d = \"%s\" (%d bytes)\n",
		pc_Address, string, length);
	Append(string, length);
}

void PC_AppendCmd(pcd_t command)
{
	MS_Message(MSG_DEBUG, "AC> %06d = #%d:%s\n", pc_Address,
		command, PCDNames[command]);
	command = MS_LittleULONG(command);
	Append(&command, sizeof(U_LONG));
}

//==========================================================================
//
// PC_Write functions
//
//==========================================================================

static void Write(void *buffer, size_t size, int address)
{
	if(address+size > BufferSize)
	{
		ERR_Exit(ERR_PCODE_BUFFER_OVERFLOW, NO, NULL);
	}
	memcpy(pc_Buffer+address, buffer, size);
}

void PC_Write(void *buffer, size_t size, int address)
{
	MS_Message(MSG_DEBUG, "WD> %06d = (%d bytes)\n", address, size);
	Write(buffer, size, address);
}

/*
void PC_WriteByte(U_BYTE val, int address)
{
	MS_Message(MSG_DEBUG, "WB> %06d = %d\n", address, val);
	Write(&val, sizeof(U_BYTE), address);
}
*/

/*
void PC_WriteWord(U_WORD val, int address)
{
	MS_Message(MSG_DEBUG, "WW> %06d = %d\n", address, val);
	val = MS_LittleUWORD(val);
	Write(&val, sizeof(U_WORD), address);
}
*/

void PC_WriteLong(U_LONG val, int address)
{
	MS_Message(MSG_DEBUG, "WL> %06d = %d\n", address, val);
	val = MS_LittleULONG(val);
	Write(&val, sizeof(U_LONG), address);
}

void PC_WriteString(char *string, int address)
{
	int length;

	length = strlen(string)+1;
	MS_Message(MSG_DEBUG, "WS> %06d = \"%s\" (%d bytes)\n",
		address, string, length);
	Write(string, length, address);
}

void PC_WriteCmd(pcd_t command, int address)
{
	MS_Message(MSG_DEBUG, "WC> %06d = #%d:%s\n", address,
		command, PCDNames[command]);
	command = MS_LittleULONG(command);
	Write(&command, sizeof(U_LONG), address);
}

//==========================================================================
//
// PC_Skip functions
//
//==========================================================================

static void Skip(size_t size)
{
	if(pc_Address+size > BufferSize)
	{
		ERR_Exit(ERR_PCODE_BUFFER_OVERFLOW, NO, NULL);
	}
	pc_BufferPtr += size;
	pc_Address += size;
}

void PC_Skip(size_t size)
{
	MS_Message(MSG_DEBUG, "SD> %06d (skip %d bytes)\n",
		pc_Address, size);
	Skip(size);
}

/*
void PC_SkipByte(void)
{
	MS_Message(MSG_DEBUG, "SB> %06d (skip byte)\n", pc_Address);
	Skip(sizeof(U_BYTE));
}
*/

/*
void PC_SkipWord(void)
{
	MS_Message(MSG_DEBUG, "SW> %06d (skip word)\n", pc_Address);
	Skip(sizeof(U_WORD));
}
*/

void PC_SkipLong(void)
{
	MS_Message(MSG_DEBUG, "SL> %06d (skip long)\n", pc_Address);
	Skip(sizeof(U_LONG));
}

//==========================================================================
//
// PC_AddScript
//
//==========================================================================

void PC_AddScript(int number, int argCount)
{
	scriptInfo_t *script;

	if(pc_ScriptCount == MAX_SCRIPT_COUNT)
	{
		ERR_Exit(ERR_TOO_MANY_SCRIPTS, YES, NULL);
	}
	script = &ScriptInfo[pc_ScriptCount];
	script->number = number;
	script->address = pc_Address;
	script->argCount = argCount;
	pc_ScriptCount++;
}
