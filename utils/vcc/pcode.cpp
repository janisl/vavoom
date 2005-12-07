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
#define MAX_INSTRUCTIONS		(256 * 1024)
#define	MAX_GLOBALS				(256 * 1024)
#define MAX_STRINGS				8192
#define	MAX_STRINGS_BUF			500000
#define MAX_CLASSES				1024
#define MAX_STRUCTS				1024
#define	MAX_VTABLES				(128 * 1024)
#define	MAX_PROPINFOS			1024

#define OPCODE_STATS

// TYPES -------------------------------------------------------------------

struct TStringInfo
{
	int offs;
	int next;
};

struct FInstruction
{
	int			Address;
	int			Opcode;
	int			Arg1;
	int			Arg2;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

void DumpAsmFunction(int num);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

FInstruction*	Instructions;
int				NumInstructions;

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

TClass*			classtypes;
int				numclasses;

TStruct*		structtypes;
int				numstructs;

int*			vtables;
int				numvtables;

dfield_t*		propinfos;
int				numpropinfos;

TArray<FName>		sprite_names;

TArray<FName>		models;

TArray<state_t>		states;
TArray<compstate_t>	compstates;

TArray<mobjinfo_t>	mobj_info;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int*			CodeBuffer;
static int			CodeBufferSize;

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
	//	Code buffer
	CodeBuffer = new int[CODE_BUFFER_SIZE];
	memset(CodeBuffer, 0, CODE_BUFFER_SIZE * 4);
	CodeBufferSize = 1;

	Instructions = new FInstruction[MAX_INSTRUCTIONS];
	memset(Instructions, 0, MAX_INSTRUCTIONS * sizeof(FInstruction));
	NumInstructions = 1;

	//	Globals
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

	//	Strings
	strings = new char[MAX_STRINGS_BUF];
	memset(strings, 0, MAX_STRINGS_BUF);
	memset(StringLookup, 0, 256 * 4);
	//	1-st string is empty
	StringCount = 1;
	strofs = 4;

	//	Structs
	structtypes = new TStruct[MAX_STRUCTS];
	numstructs = 2;
	structtypes[0].Name = NAME_state_t;
	structtypes[0].Type = &type_state;
	structtypes[0].Size = -1;
	type_state.Struct = &structtypes[0];
	structtypes[1].Name = NAME_mobjinfo_t;
	structtypes[1].Type = &type_mobjinfo;
	structtypes[1].Size = -1;
	type_mobjinfo.Struct = &structtypes[1];

	//	Classes
	classtypes = new TClass[MAX_CLASSES];
	numclasses = 1;
	classtypes[0].Name = NAME_Object;
	classtypes[0].Size = -1;

	//	Virtual tables
	vtables = new int[MAX_VTABLES];
	memset(vtables, 0, MAX_VTABLES * 4);

	//	Property info
	propinfos = new dfield_t[MAX_PROPINFOS];
	memset(propinfos, 0, MAX_PROPINFOS * sizeof(dfield_t));
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

int AddStatement(int statement)
{
	if (CurrentPass == 1)
	{
		dprintf("AddStatement in pass 1\n");
	}

	if (CodeBufferSize >= CODE_BUFFER_SIZE)
	{
		ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
	}
	if (NumInstructions >= MAX_INSTRUCTIONS)
	{
		ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
	}
	if (StatementInfo[statement].params != 0)
	{
		ERR_Exit(ERR_NONE, false, "Opcode doesn't have 0 params");
	}

	if (statement == OPC_Drop)
	{
		switch (Instructions[NumInstructions - 1].Opcode)
		{
		case OPC_PreInc:
		case OPC_PostInc:
			statement = OPC_IncDrop;
			break;

		case OPC_PreDec:
		case OPC_PostDec:
			statement = OPC_DecDrop;
			break;

		default:
			break;
		}

		if (statement != OPC_Drop)
		{
			UndoStatement();
		}
	}

	int i = NumInstructions++;
	Instructions[i].Opcode = statement;
	Instructions[i].Arg1 = 0;
	Instructions[i].Arg2 = 0;

	return i;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement, int parm1)
{
	if (CurrentPass == 1)
	{
		dprintf("AddStatement in pass 1\n");
	}

	if (CodeBufferSize >= CODE_BUFFER_SIZE)
	{
		ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
	}
	if (NumInstructions >= MAX_INSTRUCTIONS)
	{
		ERR_Exit(ERR_NONE, false, "Code buffer overflow.");
	}
	if (StatementInfo[statement].params != 1)
	{
		ERR_Exit(ERR_NONE, false, "Opcode does.t have 1 params");
	}

	int i = NumInstructions++;
	Instructions[i].Opcode = statement;
	Instructions[i].Arg1 = parm1;
	Instructions[i].Arg2 = 0;

	return i;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement, int parm1, int parm2)
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

	int i = NumInstructions++;
	Instructions[i].Opcode = statement;
	Instructions[i].Arg1 = parm1;
	Instructions[i].Arg2 = parm2;

	return i;
}

//==========================================================================
//
//  FixupJump
//
//==========================================================================

void FixupJump(int Pos, int JmpPos)
{
	Instructions[Pos].Arg1 = JmpPos;
}

//==========================================================================
//
//  FixupJump
//
//==========================================================================

void FixupJump(int Pos)
{
	Instructions[Pos].Arg1 = NumInstructions;
}

//==========================================================================
//
//  UndoStatement
//
//==========================================================================

int UndoStatement()
{
	if (CurrentPass == 1)
	{
		dprintf("UndoStatement in pass 1\n");
	}

	NumInstructions--;
	return Instructions[NumInstructions].Opcode;
}

//==========================================================================
//
//	BeginCode
//
//==========================================================================

void BeginCode(int)
{
	NumInstructions = 0;
}

//==========================================================================
//
//	EndCode
//
//==========================================================================

void EndCode(int FuncNum)
{
	int i;
	functions[FuncNum].FirstStatement = CodeBufferSize;

	for (i = 0; i < NumInstructions; i++)
	{
		Instructions[i].Address = CodeBufferSize;
		CodeBuffer[CodeBufferSize++] = Instructions[i].Opcode;
		if (StatementInfo[Instructions[i].Opcode].params > 0)
			CodeBuffer[CodeBufferSize++] = Instructions[i].Arg1;
		if (StatementInfo[Instructions[i].Opcode].params > 1)
			CodeBuffer[CodeBufferSize++] = Instructions[i].Arg2;
#ifdef OPCODE_STATS
		StatementInfo[Instructions[i].Opcode].usecount++;
#endif
	}
	Instructions[NumInstructions].Address = CodeBufferSize;

	for (i = 0; i < NumInstructions; i++)
	{
		switch (Instructions[i].Opcode)
		{
		case OPC_Goto:
		case OPC_IfGoto:
		case OPC_IfNotGoto:
		case OPC_IfTopGoto:
		case OPC_IfNotTopGoto:
			CodeBuffer[Instructions[i].Address + 1] =
				Instructions[Instructions[i].Arg1].Address;
			break;
		case OPC_CaseGoto:
		case OPC_CaseGotoName:
		case OPC_CaseGotoClassId:
			CodeBuffer[Instructions[i].Address + 2] =
				Instructions[Instructions[i].Arg2].Address;
			break;
		}
	}
}

//==========================================================================
//
//	WriteCode
//
//==========================================================================

static void WriteCode()
{
//	for (int i = 0; i < numfunctions; i++)
//	{
//		DumpAsmFunction(i);
//	}
}

//==========================================================================
//
//	LittleFloat
//
//==========================================================================

float LittleFloat(float f)
{
	union
	{
		int i;
		float f;
	} fi;
	fi.f = f;
	fi.i = LittleLong(fi.i);
	return fi.f;
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

	WriteCode();

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
	if (NumInstructions >= MAX_INSTRUCTIONS)
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
	if (numvtables >= MAX_VTABLES)
	{
		ERR_Exit(ERR_NONE, false, "VTables overflow");
	}
	if (numpropinfos >= MAX_PROPINFOS)
	{
		ERR_Exit(ERR_NONE, false, "Property infos overflow");
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
			functions[i].OuterClass->Index : -1);
		func.first_statement = LittleLong(functions[i].FirstStatement);
		func.num_parms = LittleShort(functions[i].ParamsSize);
		func.num_locals = LittleShort(functions[i].NumLocals);
		func.type = LittleShort(functions[i].ReturnType->type);
		func.flags = LittleShort(functions[i].Flags);
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
		TClass& ct = classtypes[i];

		ci.name = LittleLong(ct.Name.GetIndex());
		ci.vtable = LittleLong(ct.VTable);
		ci.size = LittleShort(ct.Size);
		ci.num_methods = LittleShort(ct.NumMethods);
		ci.parent = LittleLong(ct.ParentClass ? ct.ParentClass->Index : 0);
		ci.num_properties = LittleLong(ct.NumProperties);
		ci.ofs_properties = LittleLong(ct.OfsProperties);
		fwrite(&ci, 1, sizeof(ci), f);
	}

	progs.ofs_vtables = ftell(f);
	progs.num_vtables = numvtables;
	for (i = 0; i < numvtables; i++)
	{
		short gv;
		gv = LittleShort(vtables[i]);
		fwrite(&gv, 1, 2, f);
	}

	progs.ofs_propinfo = ftell(f);
	progs.num_propinfo = numpropinfos;
	for (i = 0; i < numpropinfos; i++)
	{
		dfield_t pi;
		pi.type = LittleShort(propinfos[i].type);
		pi.ofs = LittleShort(propinfos[i].ofs);
		fwrite(&pi, 1, sizeof(dfield_t), f);
	}

	progs.ofs_sprnames = ftell(f);
	progs.num_sprnames = sprite_names.Num();
	for (i = 0; i < sprite_names.Num(); i++)
	{
		short n;
		n = LittleShort(sprite_names[i].GetIndex());
		fwrite(&n, 1, 2, f);
	}

	progs.ofs_mdlnames = ftell(f);
	progs.num_mdlnames = models.Num();
	for (i = 0; i < models.Num(); i++)
	{
		short n;
		n = LittleShort(models[i].GetIndex());
		fwrite(&n, 1, 2, f);
	}

	progs.ofs_states = ftell(f);
	progs.num_states = states.Num();
	for (i = 0; i < states.Num(); i++)
	{
		dstate_t s;
		s.sprite = LittleShort(states[i].sprite);
		s.frame = (byte)states[i].frame;
		s.model_index = LittleShort(states[i].model_index);
		s.model_frame = (byte)states[i].model_frame;
		s.time = LittleFloat(states[i].time);
		s.nextstate = LittleShort(states[i].nextstate);
		s.function = LittleShort(states[i].function);
		s.statename = LittleShort(states[i].statename.GetIndex());
		fwrite(&s, 1, sizeof(dstate_t), f);
	}

	progs.ofs_mobjinfo = ftell(f);
	progs.num_mobjinfo = mobj_info.Num();
	for (i = 0; i < mobj_info.Num(); i++)
	{
		dmobjinfo_t m;
		m.doomednum = LittleShort(mobj_info[i].doomednum);
		m.class_id = LittleShort(mobj_info[i].class_id);
		fwrite(&m, 1, sizeof(dmobjinfo_t), f);
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
	dprintf("VTables    %6d %6d\n", numvtables, numvtables * 2);
	dprintf("Prop info  %6d %6d\n", numpropinfos, numpropinfos * sizeof(dfield_t));
	dprintf("Spr names  %6d %6d\n", sprite_names.Num(), sprite_names.Num() * 2);
	dprintf("Mdl names  %6d %6d\n", models.Num(), models.Num() * 2);
	dprintf("States     %6d %6d\n", states.Num(), states.Num() * sizeof(dstate_t));
	dprintf("Mobj info  %6d %6d\n", mobj_info.Num(), mobj_info.Num() * sizeof(dmobjinfo_t));
	dprintf("TOTAL SIZE       %7d\n", (int)ftell(f));

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

	dprintf("--------------------------------------------\n");
	dprintf("Dump ASM function %s.%s\n\n", functions[num].OuterClass ?
		*functions[num].OuterClass->Name : "None", *functions[num].Name);
	s = functions[num].FirstStatement;
	if (s < 0)
	{
		//	Builtin function
		dprintf("Builtin Nr. %d\n", -s);
		return;
	}
	do
	{
		//	Opcode
		st = CodeBuffer[s];
		dprintf("%6d: %s ", s, StatementInfo[st].name);
		s++;
		if (StatementInfo[st].params >= 1)
		{
			//	1-st argument
			dprintf("%6d ", CodeBuffer[s]);
			if (st == OPC_Call)
			{
				//	Name of the function called
				dprintf("(%s.%s)", functions[CodeBuffer[s]].OuterClass ?
					*functions[CodeBuffer[s]].OuterClass->Name : "none",
					*functions[CodeBuffer[s]].Name);
			}
			else if (st == OPC_PushString)
			{
				//  String
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
			//	2-nd argument
			dprintf("%6d ", CodeBuffer[s]);
			s++;
		}
		dprintf("\n");
		for (i = 0; i < numfunctions; i++)
		{
			//	if next command is first statement of another function,
			// then this function has ended.
			if (s == functions[i].FirstStatement)
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
	if (strstr(buf, "."))
	{
		cname = buf;
		fname = strstr(buf, ".") + 1;
		fname[-1] = 0;
	}
	else
	{
		cname = NULL;
		fname = buf;
	}
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
	dprintf("Dump ASM: %s not found!\n", name);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.24  2005/12/07 22:52:55  dj_jl
//	Moved compiler generated data out of globals.
//
//	Revision 1.23  2005/11/30 23:55:05  dj_jl
//	Directly use with-drop statements.
//	
//	Revision 1.22  2005/11/30 13:14:53  dj_jl
//	Implemented instruction buffer.
//	
//	Revision 1.21  2005/11/29 19:31:43  dj_jl
//	Class and struct classes, removed namespaces, beautification.
//	
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
