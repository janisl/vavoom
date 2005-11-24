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

#include "vcc.h"

// MACROS ------------------------------------------------------------------

#define CODE_BUFFER_SIZE		(256 * 1024)
#define	MAX_GLOBALS				(256 * 1024)
#define MAX_STRINGS				8192
#define	MAX_STRINGS_BUF			500000

#define OPCODE_STATS

// TYPES -------------------------------------------------------------------

struct TStringInfo
{
	int offs;
	int next;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int*			CodeBuffer;
int				CodeBufferSize;

int*			globals;
byte*			globalinfo;
int				numglobals;

TGlobalDef*		globaldefs;
int				numglobaldefs;

TFunction*		functions;
int				numfunctions;
int				numbuiltins;

constant_t		Constants[MAX_CONSTANTS];
int				numconstants;
constant_t		*ConstantsHash[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			undoOpcode;
static int			undoSize;

static char*		strings;
static int			strofs;
static TStringInfo	StringInfo[MAX_STRINGS];
static int 			StringCount;
static int			StringLookup[256];

static struct
{
	char*	name;
	int		params;
	int		usecount;
} StatementInfo[NUM_OPCODES] =
{
#define DECLARE_OPC(name, argcount)		{ #name, argcount, 0}
#define OPCODE_INFO
#include "../../source/progdefs.h"
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
	memset(CodeBuffer, 0, CODE_BUFFER_SIZE * 4);
	CodeBufferSize = 1;
	globals = new int[MAX_GLOBALS];
	memset(globals, 0, MAX_GLOBALS * 4);
	globalinfo = new byte[MAX_GLOBALS];
	memset(globalinfo, 0, MAX_GLOBALS);
	numglobals = 1;
	globaldefs = new TGlobalDef[MAX_GLOBALS];
	memset(globaldefs, 0, MAX_GLOBALS * sizeof(TGlobalDef));
	numglobaldefs = 1;
	globaldefs[0].type = &type_void;

	//	Functions.
	functions = new TFunction[MAX_FUNCTIONS];
	memset(functions, 0, MAX_FUNCTIONS * sizeof(TFunction));
	numfunctions = 1;
	numbuiltins = 1;
	functions[0].ReturnType = &type_void;

	strings = new char[MAX_STRINGS_BUF];
	memset(strings, 0, MAX_STRINGS_BUF);
	memset(StringLookup, 0, 256 * 4);
	//	1. simbolu virkne ir tukýa
	StringCount = 1;
	strofs = 4;
}

//==========================================================================
//
//	StringHashFunc
//
//==========================================================================

static int StringHashFunc(const char *str)
{
	return (*str ^ (str[1] << 4)) & 0xff;
}

//==========================================================================
//
//  FindString
//
//  Atrie÷ nobØdi simbolu virk·u masØvÆ
//
//==========================================================================

int FindString(const char *str)
{
	if (!*str)
	{
		return 0;
	}
	int hash = StringHashFunc(str);
	for (int i = StringLookup[hash]; i; i = StringInfo[i].next)
	{
		if (!strcmp(strings + StringInfo[i].offs, str))
		{
			return StringInfo[i].offs;
		}
	}

	//  Pievieno jaunu
	if (StringCount == MAX_STRINGS)
	{
		ERR_Exit(ERR_TOO_MANY_STRINGS, true,
				 "Current maximum: %d", MAX_STRINGS);
	}

	StringInfo[StringCount].offs = strofs;
	StringInfo[StringCount].next = StringLookup[hash];
	StringLookup[hash] = StringCount;
	strcpy(strings + strofs, str);
	strofs += (strlen(str) + 4) & ~3;
	StringCount++;
	return StringInfo[StringCount - 1].offs;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int* AddStatement(int statement)
{
	if (CurrentPass == 1)
	{
		dprintf("AddStatement in pass 1\n");
	}

	if (CodeBufferSize >= CODE_BUFFER_SIZE)
	{
		ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
	}
	if (StatementInfo[statement].params != 0)
	{
		ERR_Exit(ERR_NONE, false, "Opcode doesn't have 0 params");
	}

	if (statement == OPC_Drop)
	{
		switch (undoOpcode)
		{
		case OPC_Assign:
			statement = OPC_AssignDrop;
			break;

		case OPC_AddVar:
			statement = OPC_AddVarDrop;
			break;

		case OPC_SubVar:
			statement = OPC_SubVarDrop;
			break;

		case OPC_MulVar:
			statement = OPC_MulVarDrop;
			break;

		case OPC_DivVar:
			statement = OPC_DivVarDrop;
			break;

		case OPC_ModVar:
			statement = OPC_ModVarDrop;
			break;

		case OPC_AndVar:
			statement = OPC_AndVarDrop;
			break;

		case OPC_OrVar:
			statement = OPC_OrVarDrop;
			break;

		case OPC_XOrVar:
			statement = OPC_XOrVarDrop;
			break;

		case OPC_LShiftVar:
			statement = OPC_LShiftVarDrop;
			break;

		case OPC_RShiftVar:
			statement = OPC_RShiftVarDrop;
			break;

		case OPC_PreInc:
		case OPC_PostInc:
			statement = OPC_IncDrop;
			break;

		case OPC_PreDec:
		case OPC_PostDec:
			statement = OPC_DecDrop;
			break;

		case OPC_FAddVar:
			statement = OPC_FAddVarDrop;
			break;

		case OPC_FSubVar:
			statement = OPC_FSubVarDrop;
			break;

		case OPC_FMulVar:
			statement = OPC_FMulVarDrop;
			break;

		case OPC_FDivVar:
			statement = OPC_FDivVarDrop;
			break;

		case OPC_AssignBool:
//FIXME
		 	return &CodeBuffer[CodeBufferSize - 1];

		default:
			break;
		}

		if (statement != OPC_Drop)
		{
			UndoStatement();
#ifdef OPCODE_STATS
			StatementInfo[undoOpcode].usecount--;
#endif
		}
	}

	if (statement == OPC_VDrop)
	{
		switch (undoOpcode)
		{
		case OPC_VAssign:
			statement = OPC_VAssignDrop;
			break;

		case OPC_VAddVar:
			statement = OPC_VAddVarDrop;
			break;

		case OPC_VSubVar:
			statement = OPC_VSubVarDrop;
			break;

		case OPC_VScaleVar:
			statement = OPC_VScaleVarDrop;
			break;

		case OPC_VIScaleVar:
			statement = OPC_VIScaleVarDrop;
			break;

		default:
			break;
		}

		if (statement != OPC_VDrop)
		{
			UndoStatement();
#ifdef OPCODE_STATS
			StatementInfo[undoOpcode].usecount--;
#endif
		}
	}

	undoOpcode = statement;
	undoSize = CodeBufferSize;

	CodeBuffer[CodeBufferSize++] = statement;
#ifdef OPCODE_STATS
	StatementInfo[statement].usecount++;
#endif

	return &CodeBuffer[CodeBufferSize - 1];
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int* AddStatement(int statement, int parm1)
{
	if (CurrentPass == 1)
	{
		dprintf("AddStatement in pass 1\n");
	}

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
#ifdef OPCODE_STATS
	StatementInfo[statement].usecount++;
#endif

	return &CodeBuffer[CodeBufferSize - 1];
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int* AddStatement(int statement, int parm1, int parm2)
{
	if (CurrentPass == 1)
	{
		dprintf("AddStatement in pass 1\n");
	}

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
#ifdef OPCODE_STATS
	StatementInfo[statement].usecount++;
#endif

	return &CodeBuffer[CodeBufferSize - 1];
}

//==========================================================================
//
//  UndoStatement
//
//==========================================================================

int UndoStatement(void)
{
	if (CurrentPass == 1)
	{
		dprintf("UndoStatement in pass 1\n");
	}

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

	//	Chack buffers
	if (StringCount >= MAX_STRINGS)
	{
		ERR_Exit(ERR_TOO_MANY_STRINGS, true,
				 "Current maximum: %d", MAX_STRINGS);
	}
	if (CodeBufferSize >= CODE_BUFFER_SIZE)
	{
		ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
	}
	if (numglobals >= MAX_GLOBALS)
	{
		ERR_Exit(ERR_NONE, false, "Globals overflow");
	}
	if (numfunctions >= MAX_FUNCTIONS)
	{
		ERR_Exit(ERR_NONE, false, "Functions overflow");
	}
	if (strofs >= MAX_STRINGS_BUF)
	{
		ERR_Exit(ERR_NONE, false, "Strings buffer overflow");
	}

	f = fopen(name, "wb");
	if (!f)
	{
		ERR_Exit(ERR_CANT_OPEN_FILE, false, "File: \"%s\".", name);
	}

	memset(&progs, 0, sizeof(progs));
	fwrite(&progs, 1, sizeof(progs), f);

	progs.ofs_names = ftell(f);
	progs.num_names = FName::GetMaxNames();
	for (i = 0; i < FName::GetMaxNames(); i++)
	{
		FNameEntry *E = FName::GetEntry(i);
		int len = strlen(E->Name);
		fwrite(E->Name, 1, (len + 4) & ~3, f);
	}

	progs.ofs_strings = ftell(f);
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

	progs.ofs_globalinfo = ftell(f);
	fwrite(globalinfo, 1, (numglobals + 3) & ~3, f);

	progs.ofs_functions = ftell(f);
	progs.num_functions = numfunctions;
	for (i = 0; i < numfunctions; i++)
	{
		dfunction_t func;
		func.name = LittleShort(functions[i].Name.GetIndex());
		func.outer_class = LittleShort(functions[i].OuterClass ?
			functions[i].OuterClass->classid : -1);
		func.first_statement = LittleLong(functions[i].first_statement);
		func.num_parms = LittleShort(functions[i].ParamsSize);
		func.num_locals = LittleShort(functions[i].num_locals);
		func.type = LittleShort(functions[i].ReturnType->type);
		func.flags = LittleShort(functions[i].flags);
		fwrite(&func, 1, sizeof(dfunction_t), f);
	}	

	progs.ofs_globaldefs = ftell(f);
	progs.num_globaldefs = numglobaldefs;
	for (i = 0; i < numglobaldefs; i++)
	{
		dglobaldef_t gdef;
		gdef.name = LittleShort(globaldefs[i].Name.GetIndex());
		gdef.ofs = LittleShort(globaldefs[i].ofs);
		fwrite(&gdef, 1, sizeof(dglobaldef_t), f);
	}

	progs.ofs_classinfo = ftell(f);
	progs.num_classinfo = numclasses;
	for (i = 0; i < numclasses; i++)
	{
		dclassinfo_t ci;
		TType *ct = classtypes[i];

		ci.name = LittleLong(ct->Name.GetIndex());
		ci.vtable = LittleLong(ct->vtable);
		ci.size = LittleShort(ct->size);
		ci.num_methods = LittleShort(ct->num_methods);
		ci.parent = LittleLong(ct->aux_type ? ct->aux_type->classid : 0);
		ci.num_properties = LittleLong(ct->num_properties);
		ci.ofs_properties = LittleLong(ct->ofs_properties);
		fwrite(&ci, 1, sizeof(ci), f);
	}

	dprintf("            count   size\n");
	dprintf("Header     %6d %6ld\n", 1, sizeof(progs));
	dprintf("Names      %6d %6d\n", FName::GetMaxNames(), progs.ofs_strings - progs.ofs_names);
	dprintf("Strings    %6d %6d\n", StringCount, strofs);
	dprintf("Statements %6d %6d\n", CodeBufferSize, CodeBufferSize * 4);
	dprintf("Globals    %6d %6d\n", numglobals, numglobals * 4);
	dprintf("Global info       %6d\n", (numglobals + 3) & ~3);
	dprintf("Functions  %6d %6ld\n", numfunctions, numfunctions * sizeof(dfunction_t));
	dprintf("Builtins   %6d\n", numbuiltins);
	dprintf("Globaldefs %6d %6ld\n", numglobaldefs, numglobaldefs * sizeof(dglobaldef_t));
	dprintf("Class info %6d %6ld\n", numclasses, numclasses * sizeof(dclassinfo_t));
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

#ifdef OPCODE_STATS
	dprintf("\n-----------------------------------------------\n\n");
	for (i = 0; i < NUM_OPCODES; i++)
	{
		dprintf("%-16s %d\n", StatementInfo[i].name, StatementInfo[i].usecount);
	}
#endif
}

//==========================================================================
//
//	DumpAsmFunction
//
//	Dizasamblñ funkciju
//
//==========================================================================

void DumpAsmFunction(int num)
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
			if (st == OPC_Call)
			{
				//	IzsauktÆs funkcijas vÆrds
				dprintf("(%s::%s)", functions[CodeBuffer[s]].OuterClass ?
					*functions[CodeBuffer[s]].OuterClass->Name : "none",
					*functions[CodeBuffer[s]].Name);
			}
			else if (st == OPC_PushString)
			{
				//  Sibolu virkne
				dprintf("(%s)", strings + CodeBuffer[s]);
			}
			else if (st == OPC_PushBool || st == OPC_AssignBool)
			{
				dprintf("(%x)", CodeBuffer[s]);
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
	char	buf[1024];
	char	*cname;
	char	*fname;

	strcpy(buf, name);
	if (strstr(buf, "::"))
	{
		cname = buf;
		fname = strstr(buf, "::") + 2;
		fname[-2] = 0;
	}
	else
	{
		cname = NULL;
		fname = buf;
	}
	dprintf("--------------------------------------------\n");
	dprintf("Dump ASM function %s\n\n", name);
	for (i = 0; i < numfunctions; i++)
	{
		if (((!cname && !functions[i].OuterClass) ||
			(cname && functions[i].OuterClass &&
			!strcmp(cname, *functions[i].OuterClass->Name))) &&
			!strcmp(fname, *functions[i].Name))
		{
			DumpAsmFunction(i);
			return;
		}
	}
	dprintf("Not found!\n");
}

//**************************************************************************
//
//	$Log$
//	Revision 1.20  2005/11/24 20:42:05  dj_jl
//	Renamed opcodes, cleanup and improvements.
//
//	Revision 1.19  2003/09/24 16:44:26  dj_jl
//	Fixed asm dump of class members
//	
//	Revision 1.18  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//	
//	Revision 1.17  2002/02/26 17:52:20  dj_jl
//	Exporting special property info into progs.
//	
//	Revision 1.16  2002/02/16 16:28:36  dj_jl
//	Added support for bool variables
//	
//	Revision 1.15  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//	
//	Revision 1.14  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//	
//	Revision 1.13  2001/12/27 17:44:02  dj_jl
//	Removed support for C++ style constructors and destructors, some fixes
//	
//	Revision 1.12  2001/12/18 19:09:41  dj_jl
//	Some extra info in progs and other small changes
//	
//	Revision 1.11  2001/12/12 19:22:22  dj_jl
//	Support for method usage as state functions, dynamic cast
//	Added dynamic arrays
//	
//	Revision 1.10  2001/12/03 19:25:44  dj_jl
//	Fixed calling of parent function
//	Added defaultproperties
//	Fixed vectors as arguments to methods
//	
//	Revision 1.9  2001/12/01 18:17:09  dj_jl
//	Fixed calling of parent method, speedup
//	
//	Revision 1.8  2001/11/09 14:42:28  dj_jl
//	References, beautification
//	
//	Revision 1.7  2001/10/27 07:54:59  dj_jl
//	Added support for constructors and destructors
//	
//	Revision 1.6  2001/10/02 17:44:52  dj_jl
//	Some optimizations
//	
//	Revision 1.5  2001/09/27 17:05:24  dj_jl
//	Increased strings limit
//	
//	Revision 1.4  2001/09/20 16:09:55  dj_jl
//	Added basic object-oriented support
//	
//	Revision 1.3  2001/08/21 17:52:54  dj_jl
//	Added support for real string pointers, beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
