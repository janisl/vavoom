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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:56  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "vcc.h"

// MACROS ------------------------------------------------------------------

#define CODE_BUFFER_SIZE		(256 * 1024)
#define	MAX_GLOBALS				(256 * 1024)
#define MAX_STRINGS				2048
#define	MAX_STRINGS_BUF			500000

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int*			CodeBuffer;
int				CodeBufferSize;

int*			globals;
int				numglobals;

TGlobalDef*		globaldefs;
int				numglobaldefs;

TFunction*		functions;
int				numfunctions;
int				numbuiltins;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		undoOpcode;
static int		undoSize;

static int		StringInfo[MAX_STRINGS];
static int 		StringCount;

static int		udiv = 0;
static int		umod = 0;
static int		ult = 0;
static int		ule = 0;
static int		ugt = 0;
static int		uge = 0;
static int		urshift = 0;
static int		udiva = 0;
static int		umoda = 0;
static int		urshifta = 0;

static struct
{
	char*	name;
    int		params;
} StatementInfo[NUM_OPCODES] =
{
	{"DONE", 0},
	{"RETURN", 0},
	{"PUSHNUMBER", 1},
	{"PUSHPOINTED", 0},
    {"LOCALADDRESS", 1},
    {"GLOBALADDRESS", 1},
	{"ADD", 0},
	{"SUBTRACT", 0},
	{"MULTIPLY", 0},
	{"DIVIDE", 0},

	{"MODULUS", 0},
	{"UDIVIDE", 0},
	{"UMODULUS", 0},
	{"EQ", 0},
	{"NE", 0},
	{"LT", 0},
	{"GT", 0},
	{"LE", 0},
	{"GE", 0},
	{"ULT", 0},

	{"UGT", 0},
	{"ULE", 0},
	{"UGE", 0},
	{"ANDLOGICAL", 0},
	{"ORLOGICAL", 0},
	{"NEGATELOGICAL", 0},
	{"ANDBITWISE", 0},
	{"ORBITWISE", 0},
	{"XORBITWISE", 0},
	{"LSHIFT", 0},

	{"RSHIFT", 0},
	{"URSHIFT", 0},
	{"UNARYMINUS", 0},
    {"BITINVERSE", 0},
	{"CALL", 1},
	{"GOTO", 1},
	{"IFGOTO", 1},
	{"IFNOTGOTO", 1},
	{"CASEGOTO", 2},
	{"DROP", 0},

	{"ASSIGN", 0},
	{"ADDVAR", 0},
	{"SUBVAR", 0},
	{"MULVAR", 0},
	{"DIVVAR", 0},
	{"MODVAR", 0},
	{"UDIVVAR", 0},
	{"UMODVAR", 0},
    {"ANDVAR", 0},
    {"ORVAR", 0},

    {"XORVAR", 0},
    {"LSHIFTVAR", 0},
    {"RSHIFTVAR", 0},
    {"URSHIFTVAR", 0},
	{"PREINC", 0},
	{"PREDEC", 0},
    {"POSTINC", 0},
    {"POSTDEC", 0},
	{"IFTOPGOTO", 1},
    {"IFNOTTOPGOTO", 1},

	{"ASSIGN_DROP", 0},
	{"ADDVAR_DROP", 0},
	{"SUBVAR_DROP", 0},
	{"MULVAR_DROP", 0},
	{"DIVVAR_DROP", 0},
	{"MODVAR_DROP", 0},
	{"UDIVVAR_DROP", 0},
	{"UMODVAR_DROP", 0},
    {"ANDVAR_DROP", 0},
    {"ORVAR_DROP", 0},

    {"XORVAR_DROP", 0},
    {"LSHIFTVAR_DROP", 0},
    {"RSHIFTVAR_DROP", 0},
    {"URSHIFTVAR_DROP", 0},
	{"INC_DROP", 0},
	{"DEC_DROP", 0},
	{"FADD", 0},
	{"FSUBTRACT", 0},
	{"FMULTIPLY", 0},
	{"FDIVIDE", 0},

	{"FEQ", 0},
	{"FNE", 0},
	{"FLT", 0},
	{"FGT", 0},
	{"FLE", 0},
	{"FGE", 0},
	{"FUNARYMINUS", 0},
	{"FADDVAR", 0},
	{"FSUBVAR", 0},
	{"FMULVAR", 0},

	{"FDIVVAR", 0},
	{"FADDVAR_DROP", 0},
	{"FSUBVAR_DROP", 0},
	{"FMULVAR_DROP", 0},
	{"FDIVVAR_DROP", 0},
	{"SWAP", 0},
	{"ICALL", 0},
	{"VPUSHPOINTED", 0},
	{"VADD", 0},
	{"VSUBTRACT", 0},

	{"VPRESCALE", 0},
	{"VPOSTSCALE", 0},
	{"VISCALE", 0},
	{"VEQ", 0},
	{"VNE", 0},
	{"VUNARYMINUS", 0},
	{"VDROP", 0},
	{"VASSIGN", 0},
	{"VADDVAR", 0},
	{"VSUBVAR", 0},

	{"VSCALEVAR", 0},
	{"VISCALEVAR", 0},
	{"VASSIGN_DROP", 0},
	{"VADDVAR_DROP", 0},
	{"VSUBVAR_DROP", 0},
	{"VSCALEVAR_DROP", 0},
	{"VISCALEVAR_DROP", 0},
	{"RETURNL", 0},
	{"RETURNV", 0},
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	PC_Init
//
//==========================================================================

void PC_Init(void)
{
	CodeBuffer = new int[CODE_BUFFER_SIZE];
	CodeBufferSize = 1;
	globals = new int[MAX_GLOBALS];
	numglobals = 1;
	globaldefs = new TGlobalDef[MAX_GLOBALS];
	numglobaldefs = 1;
	globaldefs[0].type = &type_void;
	functions = new TFunction[MAX_FUNCTIONS];
	numfunctions = 1;
    numbuiltins = 1;
	functions[0].type = &type_function;
    strings = new char[MAX_STRINGS_BUF];
	//	1. simbolu virkne ir tukýa
	StringCount = 1;
	strofs = 4;
	InitInfoTables();
}

//==========================================================================
//
//  FindString
//
//  Atrie÷ nobØdi simbolu virk·u masØvÆ
//
//==========================================================================

int FindString(char *str)
{
	int i;

	for (i = 0; i < StringCount; i++)
	{
		if (!strcmp(strings + StringInfo[i], str))
		{
			return StringInfo[i];
		}
	}

	//  Pievieno jaunu
	if (StringCount == MAX_STRINGS)
	{
		ERR_Exit(ERR_TOO_MANY_STRINGS, true,
				 "Current maximum: %d", MAX_STRINGS);
	}

	StringInfo[StringCount] = strofs;
	strcpy(strings + strofs, str);
	strofs += (strlen(str) + 4) & ~3;
	StringCount++;
	return StringInfo[StringCount - 1];
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int* AddStatement(int statement)
{
	if (CodeBufferSize >= CODE_BUFFER_SIZE)
    {
    	ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
    }
    if (StatementInfo[statement].params != 0)
    {
    	ERR_Exit(ERR_NONE, false, "Opcode doesn't have 0 params");
    }

	if (statement == OPC_DROP)
    {
    	switch (undoOpcode)
        {
		 case OPC_ASSIGN:
			statement = OPC_ASSIGN_DROP;
			break;

		 case OPC_ADDVAR:
			statement = OPC_ADDVAR_DROP;
			break;

		 case OPC_SUBVAR:
			statement = OPC_SUBVAR_DROP;
			break;

		 case OPC_MULVAR:
			statement = OPC_MULVAR_DROP;
			break;

		 case OPC_DIVVAR:
			statement = OPC_DIVVAR_DROP;
			break;

		 case OPC_MODVAR:
			statement = OPC_MODVAR_DROP;
			break;

		 case OPC_UDIVVAR:
			statement = OPC_UDIVVAR_DROP;
			break;

		 case OPC_UMODVAR:
			statement = OPC_UMODVAR_DROP;
			break;

		 case OPC_ANDVAR:
			statement = OPC_ANDVAR_DROP;
			break;

		 case OPC_ORVAR:
			statement = OPC_ORVAR_DROP;
			break;

		 case OPC_XORVAR:
			statement = OPC_XORVAR_DROP;
			break;

		 case OPC_LSHIFTVAR:
			statement = OPC_LSHIFTVAR_DROP;
			break;

		 case OPC_RSHIFTVAR:
			statement = OPC_RSHIFTVAR_DROP;
			break;

		 case OPC_URSHIFTVAR:
			statement = OPC_URSHIFTVAR_DROP;
			break;

		 case OPC_PREINC:
		 case OPC_POSTINC:
			statement = OPC_INC_DROP;
			break;

		 case OPC_PREDEC:
		 case OPC_POSTDEC:
			statement = OPC_DEC_DROP;
			break;

		 case OPC_FADDVAR:
			statement = OPC_FADDVAR_DROP;
			break;

		 case OPC_FSUBVAR:
			statement = OPC_FSUBVAR_DROP;
			break;

		 case OPC_FMULVAR:
			statement = OPC_FMULVAR_DROP;
			break;

		 case OPC_FDIVVAR:
			statement = OPC_FDIVVAR_DROP;
			break;

		 default:
			break;
        }

        if (statement != OPC_DROP)
        {
        	UndoStatement();
        }
    }

	if (statement == OPC_VDROP)
    {
    	switch (undoOpcode)
        {
		 case OPC_VASSIGN:
			statement = OPC_VASSIGN_DROP;
			break;

		 case OPC_VADDVAR:
			statement = OPC_VADDVAR_DROP;
			break;

		 case OPC_VSUBVAR:
			statement = OPC_VSUBVAR_DROP;
			break;

		 case OPC_VSCALEVAR:
			statement = OPC_VSCALEVAR_DROP;
			break;

		 case OPC_VISCALEVAR:
			statement = OPC_VISCALEVAR_DROP;
			break;

		 default:
			break;
        }

        if (statement != OPC_VDROP)
        {
        	UndoStatement();
        }
    }

	undoOpcode = statement;
    undoSize = CodeBufferSize;

	CodeBuffer[CodeBufferSize++] = statement;

    if (statement == OPC_UDIVIDE) udiv++;
    if (statement == OPC_UMODULUS) umod++;
    if (statement == OPC_ULT) ult++;
    if (statement == OPC_ULE) ule++;
    if (statement == OPC_UGT) ugt++;
    if (statement == OPC_UGE) uge++;
    if (statement == OPC_URSHIFT) urshift++;
    if (statement == OPC_UDIVVAR) udiva++;
    if (statement == OPC_UMODVAR) umoda++;
    if (statement == OPC_URSHIFTVAR) urshifta++;
	return &CodeBuffer[CodeBufferSize - 1];
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int* AddStatement(int statement, int parm1)
{
	if (CodeBufferSize >= CODE_BUFFER_SIZE)
    {
    	ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
    }
    if (StatementInfo[statement].params != 1)
    {
    	ERR_Exit(ERR_NONE, false, "Opcode does.t have 1 params");
    }

	undoOpcode = statement;
    undoSize = CodeBufferSize;

	CodeBuffer[CodeBufferSize++] = statement;
   	CodeBuffer[CodeBufferSize++] = parm1;

	return &CodeBuffer[CodeBufferSize - 1];
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int* AddStatement(int statement, int parm1, int parm2)
{
	if (CodeBufferSize >= CODE_BUFFER_SIZE)
    {
    	ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
    }
    if (StatementInfo[statement].params != 2)
    {
    	ERR_Exit(ERR_NONE, false, "Opcode does.t have 2 params");
    }

	undoOpcode = statement;
    undoSize = CodeBufferSize;

	CodeBuffer[CodeBufferSize++] = statement;
   	CodeBuffer[CodeBufferSize++] = parm1;
   	CodeBuffer[CodeBufferSize++] = parm2;

	return &CodeBuffer[CodeBufferSize - 1];
}

//==========================================================================
//
//  UndoStatement
//
//==========================================================================

int UndoStatement(void)
{
	CodeBufferSize = undoSize;
    return undoOpcode;
}

//==========================================================================
//
//	PC_WriteObject
//
//==========================================================================

void PC_WriteObject(char *name)
{
	FILE*			f;
	int				i;
	dprograms_t		progs;

	dprintf("Writing object\n");
	f = fopen(name, "wb");
	if (!f)
	{
        ERR_Exit(ERR_CANT_OPEN_FILE, false, "File: \"%s\".", name);
	}

	memset(&progs, 0, sizeof(progs));
	fwrite(&progs, 1, sizeof(progs), f);

	progs.ofs_strings = ftell(f);
	progs.num_strings = strofs;
	fwrite(strings, 1, strofs, f);

	progs.ofs_statements = ftell(f);
	progs.num_statements = CodeBufferSize;
	for (i = 0; i < CodeBufferSize; i++)
	{
    	int opc;
    	opc = LittleLong(CodeBuffer[i]);
		fwrite(&opc, 1, 4, f);
	}

	progs.ofs_globals = ftell(f);
	progs.num_globals = numglobals;
	for (i = 0; i < numglobals; i++)
    {
    	int gv;
    	gv = LittleLong(globals[i]);
		fwrite(&gv, 1, 4, f);
	}

	progs.ofs_functions = ftell(f);
	progs.num_functions = numfunctions;
	progs.num_builtins = numbuiltins;
	for (i = 0; i < numfunctions; i++)
	{
		dfunction_t func;
		func.s_name = LittleLong(functions[i].s_name);
		func.first_statement = LittleLong(functions[i].first_statement);
		func.num_parms = LittleShort(functions[i].type->params_size);
		func.num_locals = LittleShort(functions[i].num_locals);
		func.type = LittleLong(functions[i].type->aux_type->type);
		fwrite(&func, 1, sizeof(dfunction_t), f);
	}	

	progs.ofs_globaldefs = ftell(f);
	progs.num_globaldefs = numglobaldefs;
	for (i = 0; i < numglobaldefs; i++)
	{
		globaldef_t gdef;
		gdef.type = LittleShort(globaldefs[i].type->type);
		gdef.ofs = LittleShort(globaldefs[i].ofs);
		gdef.s_name = LittleLong(globaldefs[i].s_name);
		fwrite(&gdef, 1, sizeof(globaldef_t), f);
	}

	dprintf("            count   size\n");
	dprintf("Header     %6d %6ld\n", 1, sizeof(progs));
	dprintf("Strings    %6d %6d\n", StringCount, strofs);
	dprintf("Statements %6d %6d\n", CodeBufferSize, CodeBufferSize * 4);
	dprintf("Globals    %6d %6d\n", numglobals, numglobals * 4);
	dprintf("Functions  %6d %6ld\n", numfunctions, numfunctions * sizeof(dfunction_t));
    dprintf("Builtins   %6d\n", numbuiltins);
	dprintf("Globaldefs %6d %6ld\n", numglobaldefs, numglobaldefs * sizeof(globaldef_t));
	dprintf("TOTAL SIZE        %6d\n", (int)ftell(f));

	memcpy(progs.magic, PROG_MAGIC, 4);
	progs.version = PROG_VERSION;
	for (i = 0; i < (int)sizeof(progs) / 4; i++)
    {
		((int *)&progs)[i] = LittleLong(((int *)&progs)[i]);
	}
	fseek(f, 0, SEEK_SET);
	fwrite(&progs, 1, sizeof(progs), f);

	fclose(f);
	dprintf("%d %d %d %d %d %d %d %d %d %d\n", udiv, umod, ult, ule,
		ugt, uge, urshift, udiva, umoda, urshifta);
}

//==========================================================================
//
//	DumpAsmFunction
//
//	Dizasamblñ funkciju
//
//==========================================================================

static void DumpAsmFunction(int num)
{
	int		s;
    int		st;
    int		i;

	s = functions[num].first_statement;
    if (s < 0)
    {
    	//	IebÝvñtÆ funkcija
    	dprintf("Builtin Nr. %d\n", -s);
        return;
    }
	do
    {
    	//	OperÆcijas kods
    	st = CodeBuffer[s];
    	dprintf("%6d: %s ", s, StatementInfo[st].name);
        s++;
        if (StatementInfo[st].params >= 1)
        {
        	//	1. arguments
        	dprintf("%6d ", CodeBuffer[s]);
	        if (st == OPC_CALL)
    	    {
            	//	IzsauktÆs funkcijas vÆrds
        		dprintf("(%s)", strings + functions[CodeBuffer[s]].s_name);
	        }
	        s++;
        }
        if (StatementInfo[st].params >= 2)
        {
        	//	2. arguments
        	dprintf("%6d ", CodeBuffer[s]);
	        s++;
        }
        dprintf("\n");
		for (i=0; i<numfunctions; i++)
        {
        	//	Ja nÆkoýÆ komanda ir kÆdas citas funkcijas pirmÆ komanda,
            // tad ýØ funkcija ir beigusies
        	if (s == functions[i].first_statement)
            {
            	s = CodeBufferSize;
            }
        }
	} while (s < CodeBufferSize);
}

//==========================================================================
//
//  PC_DumpAsm
//
//==========================================================================

void PC_DumpAsm(char* name)
{
	int		i;

	dprintf("--------------------------------------------\n");
    dprintf("Dump ASM function %s\n\n", name);
	for (i=0; i<numfunctions; i++)
    {
    	if (!strcmp(name, strings + functions[i].s_name))
        {
        	DumpAsmFunction(i);
            return;
		}
    }
	dprintf("Not found!\n");
}

