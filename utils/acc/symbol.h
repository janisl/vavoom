
//**************************************************************************
//**
//** symbol.h
//**
//**************************************************************************

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

// HEADER FILES ------------------------------------------------------------

#include "common.h"
#include "pcode.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef enum
{
	SY_LABEL,
	SY_SCRIPTVAR,
	SY_MAPVAR,
	SY_WORLDVAR,
	SY_SPECIAL,
	SY_CONSTANT,
	SY_INTERNFUNC
} symbolType_t;

typedef struct
{
	int index;
} symVar_t;

typedef struct
{
	int address;
} symLabel_t;

typedef struct
{
	int value;
	int argCount;
} symSpecial_t;

typedef struct
{
	int value;
} symConstant_t;

typedef struct
{
	pcd_t directCommand;
	pcd_t stackCommand;
	int argCount;
	boolean hasReturnValue;
} symInternFunc_t;

typedef struct symbolNode_s
{
	struct symbolNode_s *left;
	struct symbolNode_s *right;
	char *name;
	symbolType_t type;
	union
	{
		symVar_t var;
		symLabel_t label;
		symSpecial_t special;
		symConstant_t constant;
		symInternFunc_t internFunc;
	} info;
} symbolNode_t;

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SY_Init(void);
symbolNode_t *SY_Find(char *name);
symbolNode_t *SY_FindLocal(char *name);
symbolNode_t *SY_FindGlobal(char *name);
symbolNode_t *SY_InsertLocal(char *name, symbolType_t type);
symbolNode_t *SY_InsertGlobal(char *name, symbolType_t type);
void SY_FreeLocals(void);
void SY_FreeGlobals(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
