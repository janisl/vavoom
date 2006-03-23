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

void DumpAsmFunction(VMethod*);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TArray<VMemberBase*>	VMemberBase::GMembers;
VMemberBase*			VMemberBase::GMembersHash[4096];

VPackage*			CurrentPackage;
int					numbuiltins;

TArray<VMethod*>	vtables;
TArray<mobjinfo_t>	mobj_info;
TArray<mobjinfo_t>	script_ids;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<const char*>	PackagePath;

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

VStream& operator<<(VStream& Strm, VField*& Obj)
{ return Strm << *(VMemberBase**)&Obj; }
VStream& operator<<(VStream& Strm, VMethod*& Obj)
{ return Strm << *(VMemberBase**)&Obj; }
VStream& operator<<(VStream& Strm, VState*& Obj)
{ return Strm << *(VMemberBase**)&Obj; }
VStream& operator<<(VStream& Strm, VStruct*& Obj)
{ return Strm << *(VMemberBase**)&Obj; }
VStream& operator<<(VStream& Strm, VClass*& Obj)
{ return Strm << *(VMemberBase**)&Obj; }

//==========================================================================
//
//	PC_Init
//
//==========================================================================

void PC_Init()
{
	CurrentPackage = new VPackage();

	//	Strings
	memset(StringLookup, 0, 256 * 4);
	//	1-st string is empty
	StringInfo.Add();
	StringInfo[0].offs = 0;
	StringInfo[0].next = 0;
	strings.AddZeroed(4);
}

//==========================================================================
//
//	AddPackagePath
//
//==========================================================================

void AddPackagePath(const char* Path)
{
	PackagePath.AddItem(Path);
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
//  Return offset in strings array.
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

void BeginCode(VMethod*)
{
	Instructions.Empty(1024);
}

//==========================================================================
//
//	EndCode
//
//==========================================================================

void EndCode(VMethod* Func)
{
	int i;
	Func->FirstStatement = CodeBuffer.Num();

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
//	VProgsWriter
//
//==========================================================================

struct TExport
{
	VMemberBase*	Obj;
	vuint8			Type;

	TExport(VMemberBase* InObj, vuint8 InType)
	: Obj(InObj)
	, Type(InType)
	{}

	friend VStream& operator<<(VStream& Strm, TExport& E)
	{
		return Strm << E.Type << E.Obj->Name;
	}
};

class VProgsWriter : public VStream
{
private:
	FILE*		File;

public:
	TArray<TExport>		Exports;

	VProgsWriter(FILE* InFile)
	: File(InFile)
	{
		bLoading = false;
	}

	//	VStream interface.
	void Seek(int InPos)
	{
		if (fseek(File, InPos, SEEK_SET))
		{
			bError = true;
		}
	}
	int Tell()
	{
		return ftell(File);
	}
	int TotalSize()
	{
		int CurPos = ftell(File);
		fseek(File, 0, SEEK_END);
		int Size = ftell(File);
		fseek(File, CurPos, SEEK_SET);
		return Size;
	}
	bool AtEnd()
	{
		return !!feof(File);
	}
	bool Close()
	{
		return !bError;
	}
	void Serialise(void* V, int Length)
	{
		if (fwrite(V, Length, 1, File) != 1)
		{
			bError = true;
		}
	}
	void Flush()
	{
		if (fflush(File))
		{
			bError = true;
		}
	}

	VStream& operator<<(VName& Name)
	{
		int TmpIdx = Name.GetIndex();
		*this << STRM_INDEX(TmpIdx);
		return *this;
	}
	VStream& operator<<(VMemberBase*& Ref)
	{
		int TmpIdx = Ref ? Ref->ExportIndex : 0;
		*this << STRM_INDEX(TmpIdx);
		return *this;
	}

	void AddExport(VMemberBase* Obj)
	{
		TExport* E = new(Exports) TExport(Obj, Obj->MemberType);
		E->Obj->ExportIndex = Exports.Num();
	}
};

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

	VProgsWriter Writer(f);

	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->IsIn(CurrentPackage))
			Writer.AddExport(VMemberBase::GMembers[i]);
	}

	memset(&progs, 0, sizeof(progs));
	Writer.Serialise(&progs, sizeof(progs));

	//	Serialise names.
	progs.ofs_names = Writer.Tell();
	progs.num_names = VName::GetNumNames();
	for (i = 0; i < VName::GetNumNames(); i++)
	{
		Writer << *VName::GetEntry(i);
	}

	progs.ofs_strings = Writer.Tell();
	progs.num_strings = strings.Num();
	Writer.Serialise(&strings[0], strings.Num());

	progs.ofs_statements = Writer.Tell();
	progs.num_statements = CodeBuffer.Num();
	for (i = 0; i < CodeBuffer.Num(); i++)
	{
		vuint8 Tmp = CodeBuffer[i];
		Writer << Tmp;
		if (StatementInfo[CodeBuffer[i]].params >= 1)
		{
			switch (CodeBuffer[i])
			{
			case OPC_PushName:
			case OPC_CaseGotoName:
				Writer << *(VName*)&CodeBuffer[i + 1];
				break;
			case OPC_PushFunction:
			case OPC_Call:
			case OPC_PushClassId:
			case OPC_DynamicCast:
			case OPC_CaseGotoClassId:
			case OPC_PushState:
				Writer << VMemberBase::GMembers[CodeBuffer[i + 1]];
				break;
			default:
				Writer << CodeBuffer[i + 1];
				break;
			}
		}
		if (StatementInfo[CodeBuffer[i]].params >= 2)
		{
			Writer << CodeBuffer[i + 2];
		}
		i += StatementInfo[CodeBuffer[i]].params;
	}

	progs.ofs_vtables = Writer.Tell();
	progs.num_vtables = vtables.Num();
	for (i = 0; i < vtables.Num(); i++)
	{
		Writer << vtables[i];
	}

	progs.ofs_mobjinfo = Writer.Tell();
	progs.num_mobjinfo = mobj_info.Num();
	for (i = 0; i < mobj_info.Num(); i++)
	{
		Writer << STRM_INDEX(mobj_info[i].doomednum)
			<< mobj_info[i].class_id;
	}

	progs.ofs_scriptids = Writer.Tell();
	progs.num_scriptids = script_ids.Num();
	for (i = 0; i < script_ids.Num(); i++)
	{
		Writer << STRM_INDEX(script_ids[i].doomednum)
			<< script_ids[i].class_id;
	}

	progs.num_exports = Writer.Exports.Num();

	//	Serialise object infos.
	progs.ofs_exportinfo = Writer.Tell();
	for (i = 0; i < Writer.Exports.Num(); i++)
	{
		Writer << Writer.Exports[i];
	}

	//	Serialise objects.
	progs.ofs_exportdata = Writer.Tell();
	for (i = 0; i < Writer.Exports.Num(); i++)
	{
		Writer.Exports[i].Obj->Serialise(Writer);
	}

	//	Print statistics.
	dprintf("            count   size\n");
	dprintf("Header     %6d %6ld\n", 1, sizeof(progs));
	dprintf("Names      %6d %6d\n", VName::GetNumNames(), progs.ofs_strings - progs.ofs_names);
	dprintf("Strings    %6d %6d\n", StringInfo.Num(), strings.Num());
	dprintf("Statements %6d %6d\n", CodeBuffer.Num(), progs.ofs_vtables - progs.ofs_statements);
	dprintf("Builtins   %6d\n", numbuiltins);
	dprintf("VTables    %6d %6d\n", vtables.Num(), progs.ofs_mobjinfo - progs.ofs_vtables);
	dprintf("Mobj info  %6d %6d\n", mobj_info.Num(), progs.ofs_scriptids - progs.ofs_mobjinfo);
	dprintf("Script Ids %6d %6d\n", script_ids.Num(), progs.ofs_exportinfo - progs.ofs_scriptids);
	dprintf("Exports    %6d %6d\n", Writer.Exports.Num(), progs.ofs_exportdata - progs.ofs_exportinfo);
	dprintf("Type data  %6d %6d\n", Writer.Exports.Num(), Writer.Tell() - progs.ofs_exportdata);
	dprintf("TOTAL SIZE       %7d\n", Writer.Tell());

	//	Write header.
	memcpy(progs.magic, PROG_MAGIC, 4);
	progs.version = PROG_VERSION;
	Writer.Seek(0);
	Writer.Serialise(progs.magic, 4);
	for (i = 1; i < (int)sizeof(progs) / 4; i++)
	{
		Writer << ((int*)&progs)[i];
	}

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

void DumpAsmFunction(VMethod* Func)
{
	int		s;
	int		st;
	int		i;

	dprintf("--------------------------------------------\n");
	dprintf("Dump ASM function %s.%s\n\n", *Func->Outer->Name,
		*Func->Name);
	s = Func->FirstStatement;
	if (Func->Flags & FUNC_Native)
	{
		//	Builtin function
		dprintf("Builtin function.\n");
		return;
	}
	do
	{
		//	Opcode
		st = CodeBuffer[s];
		dprintf("%6d (%4d): %s ", s, s - Func->FirstStatement, StatementInfo[st].name);
		s++;
		if (StatementInfo[st].params >= 1)
		{
			//	1-st argument
			dprintf("%6d ", CodeBuffer[s]);
			if (st == OPC_Call)
			{
				//	Name of the function called
				dprintf("(%s.%s)", *VMemberBase::GMembers[CodeBuffer[s]]->Outer->Name,
					*VMemberBase::GMembers[CodeBuffer[s]]->Name);
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
		for (i = 0; i < VMemberBase::GMembers.Num(); i++)
		{
			//	if next command is first statement of another function,
			// then this function has ended.
			if (VMemberBase::GMembers[i]->MemberType == MEMBER_Method &&
				s == ((VMethod*)VMemberBase::GMembers[i])->FirstStatement)
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
		dprintf("Dump ASM: Bad name %s\n", name);
		return;
	}
	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->MemberType == MEMBER_Method &&
			!strcmp(cname, *VMemberBase::GMembers[i]->Outer->Name) &&
			!strcmp(fname, *VMemberBase::GMembers[i]->Name))
		{
			DumpAsmFunction((VMethod*)VMemberBase::GMembers[i]);
			return;
		}
	}
	dprintf("Dump ASM: %s not found!\n", name);
}

//==========================================================================
//
//	operator VStream << TType
//
//==========================================================================

VStream& operator<<(VStream& Strm, TType& T)
{
	Strm << T.type;
	byte RealType = T.type;
	if (RealType == ev_array)
	{
		Strm << T.ArrayInnerType
			<< STRM_INDEX(T.array_dim);
		RealType = T.ArrayInnerType;
	}
	if (RealType == ev_pointer)
	{
		Strm << T.InnerType
			<< T.PtrLevel;
		RealType = T.InnerType;
	}
	if (RealType == ev_reference)
		Strm << T.Class;
	else if (RealType == ev_struct || RealType == ev_vector)
		Strm << T.Struct;
	else if (RealType == ev_delegate)
		Strm << T.Function;
	else if (RealType == ev_bool)
		Strm << T.bit_mask;
	return Strm;
}

//==========================================================================
//
//	VMemberBase::Serialise
//
//==========================================================================

void VMemberBase::Serialise(VStream& Strm)
{
	Strm << Outer;
}

//==========================================================================
//
//	VField::Serialise
//
//==========================================================================

void VField::Serialise(VStream& Strm)
{
	VMemberBase::Serialise(Strm);
	Strm << Next
		<< STRM_INDEX(ofs)
		<< type
		<< func
		<< STRM_INDEX(flags);
}

//==========================================================================
//
//	VMethod::Serialise
//
//==========================================================================

void VMethod::Serialise(VStream& Strm)
{
	VMemberBase::Serialise(Strm);
	Strm << STRM_INDEX(FirstStatement)
		<< STRM_INDEX(NumLocals)
		<< STRM_INDEX(Flags)
		<< ReturnType
		<< STRM_INDEX(NumParams)
		<< STRM_INDEX(ParamsSize);
	for (int i = 0; i < NumParams; i++)
		Strm << ParamTypes[i];
}

//==========================================================================
//
//	VStruct::Serialise
//
//==========================================================================

void VStruct::Serialise(VStream& Strm)
{
	VMemberBase::Serialise(Strm);
	Strm << ParentStruct
		<< IsVector
		<< STRM_INDEX(Size)
		<< Fields
		<< STRM_INDEX(AvailableSize)
		<< STRM_INDEX(AvailableOfs);
}

//==========================================================================
//
//	VClass::Serialise
//
//==========================================================================

void VClass::Serialise(VStream& Strm)
{
	VMemberBase::Serialise(Strm);
	Strm << ParentClass
		<< Fields
		<< States
		<< STRM_INDEX(VTable)
		<< STRM_INDEX(NumMethods)
		<< STRM_INDEX(Size);
}

//==========================================================================
//
//	VState::Serialise
//
//==========================================================================

void VState::Serialise(VStream& Strm)
{
	VMemberBase::Serialise(Strm);
	Strm << SpriteName
		<< STRM_INDEX(frame)
		<< ModelName
		<< STRM_INDEX(model_frame)
		<< time
		<< nextstate
		<< function
		<< Next;
}

//==========================================================================
//
//	VConstant::Serialise
//
//==========================================================================

void VConstant::Serialise(VStream& Strm)
{
	VMemberBase::Serialise(Strm);
	Strm << Type;
	switch (Type)
	{
	case ev_float:
		Strm << *(float*)&value;
		break;

	case ev_name:
		Strm << *(VName*)&value;
		break;

	default:
		Strm << STRM_INDEX(value);
		break;
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.37  2006/03/23 22:22:02  dj_jl
//	Hashing of members for faster search.
//
//	Revision 1.36  2006/03/23 18:30:54  dj_jl
//	Use single list of all members, members tree.
//	
//	Revision 1.35  2006/03/18 16:52:21  dj_jl
//	Better code serialisation.
//	
//	Revision 1.34  2006/03/12 20:04:50  dj_jl
//	States as objects, added state variable type.
//	
//	Revision 1.33  2006/03/10 19:31:55  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.32  2006/02/28 19:17:20  dj_jl
//	Added support for constants.
//	
//	Revision 1.31  2006/02/27 21:23:55  dj_jl
//	Rewrote names class.
//	
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
