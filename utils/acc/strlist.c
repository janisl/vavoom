
//**************************************************************************
//**
//** strlist.c
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <string.h>
#include "common.h"
#include "strlist.h"
#include "error.h"
#include "misc.h"
#include "pcode.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef struct
{
	char *name;
	int address;
} stringInfo_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int str_StringCount;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static stringInfo_t StringInfo[MAX_STRINGS];

// CODE --------------------------------------------------------------------

//==========================================================================
//
// STR_Init
//
//==========================================================================

void STR_Init(void)
{
	str_StringCount = 0;
}

//==========================================================================
//
// STR_Find
//
//==========================================================================

int STR_Find(char *name)
{
	int i;

	for(i = 0; i < str_StringCount; i++)
	{
		if(strcmp(StringInfo[i].name, name) == 0)
		{
			return i;
		}
	}
	// Add to list
	if(str_StringCount == MAX_STRINGS)
	{
		ERR_Exit(ERR_TOO_MANY_STRINGS, YES, "Current maximum: %d",
			MAX_STRINGS);
	}
	MS_Message(MSG_DEBUG, "Adding string %d:\n  \"%s\"\n",
		str_StringCount, name);
	StringInfo[str_StringCount].name = MS_Alloc(strlen(name)+1,
		ERR_OUT_OF_MEMORY);
	strcpy(StringInfo[str_StringCount].name, name);
	str_StringCount++;
	return str_StringCount-1;
}

//==========================================================================
//
// STR_WriteStrings
//
// Writes all the strings to the p-code buffer.
//
//==========================================================================

void STR_WriteStrings(void)
{
	int i;
	U_LONG pad;

	MS_Message(MSG_DEBUG, "---- STR_WriteStrings ----\n");
	for(i = 0; i < str_StringCount; i++)
	{
		StringInfo[i].address = pc_Address;
		PC_AppendString(StringInfo[i].name);
	}
	if(pc_Address%4 != 0)
	{ // Need to align
		pad = 0;
		PC_Append((void *)&pad, 4-(pc_Address%4));
	}
}

//==========================================================================
//
// STR_WriteList
//
//==========================================================================

void STR_WriteList(void)
{
	int i;

	MS_Message(MSG_DEBUG, "---- STR_WriteList ----\n");
	PC_AppendLong((U_LONG)str_StringCount);
	for(i = 0; i < str_StringCount; i++)
	{
		PC_AppendLong((U_LONG)StringInfo[i].address);
	}
}
