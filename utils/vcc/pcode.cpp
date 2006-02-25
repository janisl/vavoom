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

TArray<TFunction>	functions;
int					numbuiltins;

TArray<constant_t>	Constants;
int					ConstantsHash[256];

TArray<TClass*>		classtypes;
TArray<int>			vtables;

TArray<TStruct*>	structtypes;

TArray<field_t*>	FieldList;

TArray<FName>		sprite_names;
TArray<FName>		models;
TArray<state_t>		states;
TArray<mobjinfo_t>	mobj_info;
TArray<mobjinfo_t>	script_ids;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<FInstruction>	Instructions;
static TArray<int>			CodeBuffer;

static TArray<char>			strings;
static TArray<TStringInfo>	StringInfo;
static int					StringLookup[256];

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

void PC_Init()
{
	memset(ConstantsHash, -1, sizeof(ConstantsHash));

	//	Strings
	memset(StringLookup, 0, 256 * 4);
	//	1-st string is empty
	StringInfo.Add();
	StringInfo[0].offs = 0;
	StringInfo[0].next = 0;
	strings.AddZeroed(4);

	//	Classes
	classtypes.AddItem(new TClass);
	classtypes[0]->Name = NAME_Object;
	classtypes[0]->Size = -1;
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
		if (!strcmp(&strings[StringInfo[i].offs], str))
		{
			return StringInfo[i].offs;
		}
	}

	//  Add new string
	int Idx = StringInfo.Add();
	int Ofs = strings.AddZeroed((strlen(str) + 4) & ~3);
	StringInfo[Idx].offs = Ofs;
	StringInfo[Idx].next = StringLookup[hash];
	StringLookup[hash] = Idx;
	strcpy(&strings[Ofs], str);
	return StringInfo[Idx].offs;
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

	if (StatementInfo[statement].params != 0)
	{
		ERR_Exit(ERR_NONE, false, "Opcode doesn't have 0 params");
	}

	if (statement == OPC_Drop)
	{
		switch (Instructions[Instructions.Num() - 1].Opcode)
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

	int i = Instructions.Add();
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

	if (StatementInfo[statement].params != 1)
	{
		ERR_Exit(ERR_NONE, false, "Opcode does.t have 1 params");
	}

	int i = Instructions.Add();
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

	if (StatementInfo[statement].params != 2)
	{
		ERR_Exit(ERR_NONE, false, "Opcode does.t have 2 params");
	}

	int i = Instructions.Add();
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
	Instructions[Pos].Arg1 = Instructions.Num();
}

//==========================================================================
//
//  GetNumInstructions
//
//==========================================================================

int GetNumInstructions()
{
	return Instructions.Num();
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

	int Ret = Instructions.Last().Opcode;
	Instructions.Pop();
	return Ret;
}

//==========================================================================
//
//	BeginCode
//
//==========================================================================

void BeginCode(int)
{
	Instructions.Empty(1024);
}

//==========================================================================
//
//	EndCode
//
//==========================================================================

void EndCode(int FuncNum)
{
	int i;
	functions[FuncNum].FirstStatement = CodeBuffer.Num();

	for (i = 0; i < Instructions.Num(); i++)
	{
		Instructions[i].Address = CodeBuffer.Num();
		CodeBuffer.AddItem(Instructions[i].Opcode);
		if (StatementInfo[Instructions[i].Opcode].params > 0)
			CodeBuffer.AddItem(Instructions[i].Arg1);
		if (StatementInfo[Instructions[i].Opcode].params > 1)
			CodeBuffer.AddItem(Instructions[i].Arg2);
#ifdef OPCODE_STATS
		StatementInfo[Instructions[i].Opcode].usecount++;
#endif
	}
	Instructions[Instructions.Add()].Address = CodeBuffer.Num();

	for (i = 0; i < Instructions.Num() - 1; i++)
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
//	ConvType
//
//==========================================================================

static void ConvType(const TType& T, dtype_t& dt)
{
	dt.Type = T.type;
	dt.InnerType = T.InnerType;
	dt.ArrayInnerType = T.ArrayInnerType;
	dt.PtrLevel = T.PtrLevel;
	dt.ArrayDim = LittleLong(T.array_dim);
	byte RealType = T.type;
	if (RealType == ev_array)
		RealType = T.ArrayInnerType;
	if (RealType == ev_pointer)
		RealType = T.InnerType;
	if (RealType == ev_reference)
		dt.Extra = LittleLong(T.Class->Index);
	else if (RealType == ev_struct)
		dt.Extra = LittleLong(T.Struct->Index);
	else
		dt.Extra = LittleLong(T.bit_mask);
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
	fwrite(&strings[0], 1, strings.Num(), f);

	progs.ofs_statements = ftell(f);
	progs.num_statements = CodeBuffer.Num();
	for (i = 0; i < CodeBuffer.Num(); i++)
	{
		int opc;
		opc = LittleLong(CodeBuffer[i]);
		fwrite(&opc, 1, 4, f);
	}

	progs.ofs_functions = ftell(f);
	progs.num_functions = functions.Num();
	for (i = 0; i < functions.Num(); i++)
	{
		dfunction_t func;
		func.name = LittleShort(functions[i].Name.GetIndex());
		func.outer_class = LittleShort(functions[i].OuterClass ?
			functions[i].OuterClass->Index : -1);
		func.first_statement = LittleLong(functions[i].FirstStatement);
		func.num_parms = LittleShort(functions[i].NumParams);
		func.ParamsSize = LittleShort(functions[i].ParamsSize);
		func.num_locals = LittleShort(functions[i].NumLocals);
		func.flags = LittleShort(functions[i].Flags);
		ConvType(functions[i].ReturnType, func.ReturnType);
		for (int j = 0; j < MAX_PARAMS; j++)
			ConvType(functions[i].ParamTypes[j], func.ParamTypes[j]);
		fwrite(&func, 1, sizeof(dfunction_t), f);
	}	

	progs.ofs_classinfo = ftell(f);
	progs.num_classinfo = classtypes.Num();
	for (i = 0; i < classtypes.Num(); i++)
	{
		dclassinfo_t ci;
		TClass& ct = *classtypes[i];

		ci.name = LittleShort(ct.Name.GetIndex());
		ci.fields = LittleShort(ct.Fields ? ct.Fields->Index : -1);
		ci.vtable = LittleLong(ct.VTable);
		ci.size = LittleShort(ct.Size);
		ci.num_methods = LittleShort(ct.NumMethods);
		ci.parent = LittleLong(ct.ParentClass ? ct.ParentClass->Index : 0);
		fwrite(&ci, 1, sizeof(ci), f);
	}

	progs.ofs_vtables = ftell(f);
	progs.num_vtables = vtables.Num();
	for (i = 0; i < vtables.Num(); i++)
	{
		short gv;
		gv = LittleShort(vtables[i]);
		fwrite(&gv, 1, 2, f);
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
		s.outer_class = LittleShort(states[i].OuterClass->Index);
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

	progs.ofs_scriptids = ftell(f);
	progs.num_scriptids = script_ids.Num();
	for (i = 0; i < script_ids.Num(); i++)
	{
		dmobjinfo_t m;
		m.doomednum = LittleShort(script_ids[i].doomednum);
		m.class_id = LittleShort(script_ids[i].class_id);
		fwrite(&m, 1, sizeof(dmobjinfo_t), f);
	}

	progs.ofs_fields = ftell(f);
	progs.num_fields = FieldList.Num();
	for (i = 0; i < FieldList.Num(); i++)
	{
		dfield_t df;
		df.name = LittleShort(FieldList[i]->Name.GetIndex());
		df.next = LittleShort(FieldList[i]->Next ? FieldList[i]->Next->Index : -1);
		df.ofs = LittleShort(FieldList[i]->ofs);
		df.func_num = LittleShort(FieldList[i]->func_num);
		df.flags = LittleShort(FieldList[i]->flags);
		ConvType(FieldList[i]->type, df.type);
		fwrite(&df, 1, sizeof(dfield_t), f);
	}

	progs.ofs_structs = ftell(f);
	progs.num_structs = structtypes.Num();
	for (i = 0; i < structtypes.Num(); i++)
	{
		TStruct* S = structtypes[i];
		dstruct_t ds;
		ds.Name = LittleShort(S->Name.GetIndex());
		ds.OuterClass = LittleShort(S->OuterClass ? S->OuterClass->Index : -1);
		ds.ParentStruct = LittleShort(S->ParentStruct ? S->ParentStruct->Index : -1);
		ds.Size = LittleShort(S->Size);
		ds.Fields = LittleShort(S->Fields ? S->Fields->Index : -1);
		ds.AvailableSize = LittleShort(S->AvailableSize);
		ds.AvailableOfs = LittleShort(S->AvailableOfs);
		ds.IsVector = LittleShort(S->IsVector);
		fwrite(&ds, 1, sizeof(dstruct_t), f);
	}

	progs.ofs_constants = ftell(f);
	progs.num_constants = Constants.Num();
	for (i = 0; i < Constants.Num(); i++)
	{
		dconstant_t dc;
		dc.Name = LittleShort(Constants[i].Name.GetIndex());
		dc.OuterClass = LittleShort(Constants[i].OuterClass ? Constants[i].OuterClass->Index : -1);
		dc.Value = LittleLong(Constants[i].value);
		fwrite(&dc, 1, sizeof(dconstant_t), f);
	}

	dprintf("            count   size\n");
	dprintf("Header     %6d %6ld\n", 1, sizeof(progs));
	dprintf("Names      %6d %6d\n", FName::GetMaxNames(), progs.ofs_strings - progs.ofs_names);
	dprintf("Strings    %6d %6d\n", StringInfo.Num(), strings.Num());
	dprintf("Statements %6d %6d\n", CodeBuffer.Num(), CodeBuffer.Num() * 4);
	dprintf("Functions  %6d %6ld\n", functions.Num(), functions.Num() * sizeof(dfunction_t));
	dprintf("Builtins   %6d\n", numbuiltins);
	dprintf("Class info %6d %6ld\n", classtypes.Num(), classtypes.Num() * sizeof(dclassinfo_t));
	dprintf("VTables    %6d %6d\n", vtables.Num(), vtables.Num() * 2);
	dprintf("Spr names  %6d %6d\n", sprite_names.Num(), sprite_names.Num() * 2);
	dprintf("Mdl names  %6d %6d\n", models.Num(), models.Num() * 2);
	dprintf("States     %6d %6d\n", states.Num(), states.Num() * sizeof(dstate_t));
	dprintf("Mobj info  %6d %6d\n", mobj_info.Num(), mobj_info.Num() * sizeof(dmobjinfo_t));
	dprintf("Script Ids %6d %6d\n", script_ids.Num(), script_ids.Num() * sizeof(dmobjinfo_t));
	dprintf("Fields     %6d %6d\n", FieldList.Num(), FieldList.Num() * sizeof(dfield_t));
	dprintf("Structs    %6d %6d\n", structtypes.Num(), structtypes.Num() * sizeof(dstruct_t));
	dprintf("Constants  %6d %6d\n", Constants.Num(), Constants.Num() * sizeof(dconstant_t));
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
	if (functions[num].Flags & FUNC_Native)
	{
		//	Builtin function
		dprintf("Builtin function.\n");
		return;
	}
	do
	{
		//	Opcode
		st = CodeBuffer[s];
		dprintf("%6d (%4d): %s ", s, s - functions[num].FirstStatement, StatementInfo[st].name);
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
				dprintf("(%s)", &strings[CodeBuffer[s]]);
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
		for (i = 0; i < functions.Num(); i++)
		{
			//	if next command is first statement of another function,
			// then this function has ended.
			if (s == functions[i].FirstStatement)
			{
				s = CodeBuffer.Num();
			}
		}
	} while (s < CodeBuffer.Num());
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
	for (i = 0; i < functions.Num(); i++)
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
//	Revision 1.30  2006/02/25 17:07:57  dj_jl
//	Linked list of fields, export all type info.
//
//	Revision 1.29  2006/02/19 20:37:02  dj_jl
//	Implemented support for delegates.
//	
//	Revision 1.28  2006/02/19 14:37:36  dj_jl
//	Changed type handling.
//	
//	Revision 1.27  2006/02/17 19:25:00  dj_jl
//	Removed support for progs global variables and functions.
//	
//	Revision 1.26  2006/02/15 23:27:06  dj_jl
//	Added script ID class attribute.
//	
//	Revision 1.25  2005/12/12 20:58:47  dj_jl
//	Removed compiler limitations.
//	
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
