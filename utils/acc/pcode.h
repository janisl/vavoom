
//**************************************************************************
//**
//** pcode.h
//**
//**************************************************************************

#ifndef __PCODE_H__
#define __PCODE_H__

// HEADER FILES ------------------------------------------------------------

#include <stddef.h>
#include "common.h"

// MACROS ------------------------------------------------------------------

#define OPEN_SCRIPTS_BASE 1000

// TYPES -------------------------------------------------------------------

typedef enum
{
	PCD_NOP,
	PCD_TERMINATE,
	PCD_SUSPEND,
	PCD_PUSHNUMBER,
	PCD_LSPEC1,
	PCD_LSPEC2,
	PCD_LSPEC3,
	PCD_LSPEC4,
	PCD_LSPEC5,
	PCD_LSPEC1DIRECT,
	PCD_LSPEC2DIRECT,
	PCD_LSPEC3DIRECT,
	PCD_LSPEC4DIRECT,
	PCD_LSPEC5DIRECT,
	PCD_ADD,
	PCD_SUBTRACT,
	PCD_MULTIPLY,
	PCD_DIVIDE,
	PCD_MODULUS,
	PCD_EQ,
	PCD_NE,
	PCD_LT,
	PCD_GT,
	PCD_LE,
	PCD_GE,
	PCD_ASSIGNSCRIPTVAR,
	PCD_ASSIGNMAPVAR,
	PCD_ASSIGNWORLDVAR,
	PCD_PUSHSCRIPTVAR,
	PCD_PUSHMAPVAR,
	PCD_PUSHWORLDVAR,
	PCD_ADDSCRIPTVAR,
	PCD_ADDMAPVAR,
	PCD_ADDWORLDVAR,
	PCD_SUBSCRIPTVAR,
	PCD_SUBMAPVAR,
	PCD_SUBWORLDVAR,
	PCD_MULSCRIPTVAR,
	PCD_MULMAPVAR,
	PCD_MULWORLDVAR,
	PCD_DIVSCRIPTVAR,
	PCD_DIVMAPVAR,
	PCD_DIVWORLDVAR,
	PCD_MODSCRIPTVAR,
	PCD_MODMAPVAR,
	PCD_MODWORLDVAR,
	PCD_INCSCRIPTVAR,
	PCD_INCMAPVAR,
	PCD_INCWORLDVAR,
	PCD_DECSCRIPTVAR,
	PCD_DECMAPVAR,
	PCD_DECWORLDVAR,
	PCD_GOTO,
	PCD_IFGOTO,
	PCD_DROP,
	PCD_DELAY,
	PCD_DELAYDIRECT,
	PCD_RANDOM,
	PCD_RANDOMDIRECT,
	PCD_THINGCOUNT,
	PCD_THINGCOUNTDIRECT,
	PCD_TAGWAIT,
	PCD_TAGWAITDIRECT,
	PCD_POLYWAIT,
	PCD_POLYWAITDIRECT,
	PCD_CHANGEFLOOR,
	PCD_CHANGEFLOORDIRECT,
	PCD_CHANGECEILING,
	PCD_CHANGECEILINGDIRECT,
	PCD_RESTART,
	PCD_ANDLOGICAL,
	PCD_ORLOGICAL,
	PCD_ANDBITWISE,
	PCD_ORBITWISE,
	PCD_EORBITWISE,
	PCD_NEGATELOGICAL,
	PCD_LSHIFT,
	PCD_RSHIFT,
	PCD_UNARYMINUS,
	PCD_IFNOTGOTO,
	PCD_LINESIDE,
	PCD_SCRIPTWAIT,
	PCD_SCRIPTWAITDIRECT,
	PCD_CLEARLINESPECIAL,
	PCD_CASEGOTO,
	PCD_BEGINPRINT,
	PCD_ENDPRINT,
	PCD_PRINTSTRING,
	PCD_PRINTNUMBER,
	PCD_PRINTCHARACTER,
	PCD_PLAYERCOUNT,
	PCD_GAMETYPE,
	PCD_GAMESKILL,
	PCD_TIMER,
	PCD_SECTORSOUND,
	PCD_AMBIENTSOUND,
	PCD_SOUNDSEQUENCE,
	PCD_SETLINETEXTURE,
	PCD_SETLINEBLOCKING,
	PCD_SETLINESPECIAL,
	PCD_THINGSOUND,
	PCD_ENDPRINTBOLD,
	PCODE_COMMAND_COUNT
} pcd_t;

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void PC_OpenObject(char *name, size_t size, int flags);
void PC_CloseObject(void);
void PC_Append(void *buffer, size_t size);
//void PC_AppendByte(U_BYTE val);
//void PC_AppendWord(U_WORD val);
void PC_AppendLong(U_LONG val);
void PC_AppendString(char *string);
void PC_AppendCmd(pcd_t command);
void PC_Write(void *buffer, size_t size, int address);
//void PC_WriteByte(U_BYTE val, int address);
//void PC_WriteWord(U_WORD val, int address);
void PC_WriteLong(U_LONG val, int address);
void PC_WriteString(char *string, int address);
void PC_WriteCmd(pcd_t command, int address);
void PC_Skip(size_t size);
//void PC_SkipByte(void);
//void PC_SkipWord(void);
void PC_SkipLong(void);
void PC_AddScript(int number, int argCount);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int pc_Address;
extern byte *pc_Buffer;
extern byte *pc_BufferPtr;
extern int pc_ScriptCount;

#endif
