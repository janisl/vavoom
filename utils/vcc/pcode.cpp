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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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

TArray<mobjinfo_t>	mobj_info;
TArray<mobjinfo_t>	script_ids;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<const char*>	PackagePath;
static TArray<VPackage*>	LoadedPackages;

static TArray<char>			strings;
static TArray<TStringInfo>	StringInfo;
static int					StringLookup[256];

static VMethod*				CurrentFunc;

static struct
{
	char*	name;
	int		Args;
	int		usecount;
} StatementInfo[NUM_OPCODES] =
{
#define DECLARE_OPC(name, args)		{ #name, OPCARGS_##args, 0}
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
	StringInfo.Alloc();
	StringInfo[0].offs = 0;
	StringInfo[0].next = 0;
	strings.SetNum(4);
	memset(strings.Ptr(), 0, 4);
}

//==========================================================================
//
//	AddPackagePath
//
//==========================================================================

void AddPackagePath(const char* Path)
{
	PackagePath.Append(Path);
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
	TStringInfo& SI = StringInfo.Alloc();
	int AddLen = (strlen(str) + 4) & ~3;
	int Ofs = strings.Num();
	strings.SetNum(strings.Num() + AddLen);
	memset(&strings[Ofs], 0, AddLen);
	SI.offs = Ofs;
	SI.next = StringLookup[hash];
	StringLookup[hash] = StringInfo.Num() - 1;
	strcpy(&strings[Ofs], str);
	return SI.offs;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement)
{
	if (StatementInfo[statement].Args != OPCARGS_None)
	{
		ERR_Exit(ERR_NONE, false, "Opcode doesn't take 0 params");
	}

	if (statement == OPC_Drop)
	{
		switch (CurrentFunc->Instructions[CurrentFunc->Instructions.Num() - 1].Opcode)
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

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = 0;
	I.Arg2 = 0;

	return CurrentFunc->Instructions.Num() - 1;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement, int parm1)
{
	if (StatementInfo[statement].Args != OPCARGS_BranchTarget &&
		StatementInfo[statement].Args != OPCARGS_Byte &&
		StatementInfo[statement].Args != OPCARGS_Short &&
		StatementInfo[statement].Args != OPCARGS_Int &&
		StatementInfo[statement].Args != OPCARGS_String)
	{
		ERR_Exit(ERR_NONE, false, "Opcode does\'t take 1 params");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = parm1;
	I.Arg2 = 0;

	return CurrentFunc->Instructions.Num() - 1;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement, float FloatArg)
{
	if (StatementInfo[statement].Args != OPCARGS_Int)
	{
		ERR_Exit(ERR_NONE, false, "Opcode does\'t take float argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = *(vint32*)&FloatArg;

	return CurrentFunc->Instructions.Num() - 1;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement, VName NameArg)
{
	if (StatementInfo[statement].Args != OPCARGS_Name)
	{
		ERR_Exit(ERR_NONE, false, "Opcode does\'t take name argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.NameArg = NameArg;

	return CurrentFunc->Instructions.Num() - 1;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement, VMemberBase* Member)
{
	if (StatementInfo[statement].Args != OPCARGS_Member &&
		StatementInfo[statement].Args != OPCARGS_FieldOffset &&
		StatementInfo[statement].Args != OPCARGS_VTableIndex)
	{
		ParseError("Opcode does\'t take member as argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Member = Member;

	return CurrentFunc->Instructions.Num() - 1;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement, const TType& TypeArg)
{
	if (StatementInfo[statement].Args != OPCARGS_TypeSize)
	{
		ParseError("Opcode does\'t take type as argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.TypeArg = TypeArg;

	return CurrentFunc->Instructions.Num() - 1;
}

//==========================================================================
//
//  AddStatement
//
//==========================================================================

int AddStatement(int statement, int parm1, int parm2)
{
	if (StatementInfo[statement].Args != OPCARGS_ByteBranchTarget &&
		StatementInfo[statement].Args != OPCARGS_ShortBranchTarget &&
		StatementInfo[statement].Args != OPCARGS_IntBranchTarget)
	{
		ERR_Exit(ERR_NONE, false, "Opcode does\'t take 2 params");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = parm1;
	I.Arg2 = parm2;

	return CurrentFunc->Instructions.Num() - 1;
}

//==========================================================================
//
//	EmitPushNumber
//
//==========================================================================

void EmitPushNumber(int Val)
{
	if (Val == 0)
		AddStatement(OPC_PushNumber0);
	else if (Val == 1)
		AddStatement(OPC_PushNumber1);
	else if (Val >= 0 && Val < 256)
		AddStatement(OPC_PushNumberB, Val);
	else if (Val >= MIN_VINT16 && Val <= MAX_VINT16)
		AddStatement(OPC_PushNumberS, Val);
	else
		AddStatement(OPC_PushNumber, Val);
}

//==========================================================================
//
//	EmitLocalAddress
//
//==========================================================================

void EmitLocalAddress(int Ofs)
{
	if (Ofs == 0)
		AddStatement(OPC_LocalAddress0);
	else if (Ofs == 1)
		AddStatement(OPC_LocalAddress1);
	else if (Ofs == 2)
		AddStatement(OPC_LocalAddress2);
	else if (Ofs == 3)
		AddStatement(OPC_LocalAddress3);
	else if (Ofs == 4)
		AddStatement(OPC_LocalAddress4);
	else if (Ofs == 5)
		AddStatement(OPC_LocalAddress5);
	else if (Ofs == 6)
		AddStatement(OPC_LocalAddress6);
	else if (Ofs == 7)
		AddStatement(OPC_LocalAddress7);
	else if (Ofs < 256)
		AddStatement(OPC_LocalAddressB, Ofs);
	else if (Ofs < MAX_VINT16)
		AddStatement(OPC_LocalAddressS, Ofs);
	else
		AddStatement(OPC_LocalAddress, Ofs);
}

//==========================================================================
//
//  FixupJump
//
//==========================================================================

void FixupJump(int Pos, int JmpPos)
{
	CurrentFunc->Instructions[Pos].Arg1 = JmpPos;
}

//==========================================================================
//
//  FixupJump
//
//==========================================================================

void FixupJump(int Pos)
{
	CurrentFunc->Instructions[Pos].Arg1 = CurrentFunc->Instructions.Num();
}

//==========================================================================
//
//  GetNumInstructions
//
//==========================================================================

int GetNumInstructions()
{
	return CurrentFunc->Instructions.Num();
}

//==========================================================================
//
//  UndoStatement
//
//==========================================================================

int UndoStatement()
{
	int Ret = CurrentFunc->Instructions[CurrentFunc->Instructions.Num() - 1].Opcode;
	CurrentFunc->Instructions.RemoveIndex(CurrentFunc->Instructions.Num() - 1);
	return Ret;
}

//==========================================================================
//
// WriteBreaks
//
//==========================================================================

void WriteBreaks()
{
	BreakLevel--;
	while (BreakInfo.Num() && BreakInfo[BreakInfo.Num() - 1].level > BreakLevel)
	{
		FixupJump(BreakInfo[BreakInfo.Num() - 1].addressPtr);
		BreakInfo.SetNum(BreakInfo.Num() - 1);
	}
}

//==========================================================================
//
// WriteContinues
//
//==========================================================================

void WriteContinues(int address)
{
	ContinueLevel--;
	while (ContinueInfo.Num() && ContinueInfo[ContinueInfo.Num() - 1].level > ContinueLevel)
	{
		FixupJump(ContinueInfo[ContinueInfo.Num() - 1].addressPtr, address);
		ContinueInfo.SetNum(ContinueInfo.Num() - 1);
	}
}

//==========================================================================
//
//	EmitClearStrings
//
//==========================================================================

void EmitClearStrings(int Start, int End)
{
	for (int i = Start; i < End; i++)
	{
		if (localdefs[i].Cleared)
		{
			continue;
		}
		if (localdefs[i].type.type == ev_string)
		{
			EmitLocalAddress(localdefs[i].ofs);
			AddStatement(OPC_ClearPointedStr);
		}
		if (localdefs[i].type.type == ev_struct &&
			localdefs[i].type.Struct->NeedsDestructor())
		{
			EmitLocalAddress(localdefs[i].ofs);
			AddStatement(OPC_ClearPointedStruct, localdefs[i].type.Struct);
		}
		if (localdefs[i].type.type == ev_array)
		{
			if (localdefs[i].type.ArrayInnerType == ev_string)
			{
				for (int j = 0; j < localdefs[i].type.array_dim; j++)
				{
					EmitLocalAddress(localdefs[i].ofs);
					EmitPushNumber(j);
					AddStatement(OPC_ArrayElement, localdefs[i].type.GetArrayInnerType());
					AddStatement(OPC_ClearPointedStr);
				}
			}
			else if (localdefs[i].type.ArrayInnerType == ev_struct &&
				localdefs[i].type.Struct->NeedsDestructor())
			{
				for (int j = 0; j < localdefs[i].type.array_dim; j++)
				{
					EmitLocalAddress(localdefs[i].ofs);
					EmitPushNumber(j);
					AddStatement(OPC_ArrayElement, localdefs[i].type.GetArrayInnerType());
					AddStatement(OPC_ClearPointedStruct, localdefs[i].type.Struct);
				}
			}
		}
	}
}

//==========================================================================
//
//	BeginCode
//
//==========================================================================

void BeginCode(VMethod* Func)
{
	CurrentFunc = Func;
	CurrentFunc->Instructions.Clear();
	CurrentFunc->Instructions.Resize(1024);
}

//==========================================================================
//
//	EndCode
//
//==========================================================================

void EndCode(VMethod* Func)
{
#ifdef OPCODE_STATS
	for (int i = 0; i < Func->Instructions.Num(); i++)
	{
		StatementInfo[Func->Instructions[i].Opcode].usecount++;
	}
#endif
	FInstruction& Dummy = Func->Instructions.Alloc();
	Dummy.Opcode = OPC_Done;
	DumpAsmFunction(Func);
}

//==========================================================================
//
//	VProgsImport
//
//==========================================================================

struct VProgsImport
{
	vuint8			Type;
	VName			Name;
	vint32			OuterIndex;

	VMemberBase*	Obj;

	VProgsImport()
	: Type(0)
	, OuterIndex(0)
	, Obj(0)
	{}
	VProgsImport(VMemberBase* InObj, vint32 InOuterIndex)
	: Type(InObj->MemberType)
	, Name(InObj->Name)
	, OuterIndex(InOuterIndex)
	, Obj(InObj)
	{}

	friend VStream& operator<<(VStream& Strm, VProgsImport& I)
	{
		return Strm << I.Type << I.Name << STRM_INDEX(I.OuterIndex);
	}
};

//==========================================================================
//
//	VProgsExport
//
//==========================================================================

struct VProgsExport
{
	vuint8			Type;
	VName			Name;

	VMemberBase*	Obj;

	VProgsExport()
	: Type(0)
	, Obj(0)
	{}
	VProgsExport(VMemberBase* InObj)
	: Type(InObj->MemberType)
	, Name(InObj->Name)
	, Obj(InObj)
	{}

	friend VStream& operator<<(VStream& Strm, VProgsExport& E)
	{
		return Strm << E.Type << E.Name;
	}
};

//==========================================================================
//
//	VProgsReader
//
//==========================================================================

class VProgsReader : public VStream
{
private:
	FILE*				File;

public:
	VName*				NameRemap;
	int					NumImports;
	VProgsImport*		Imports;
	int					NumExports;
	VProgsExport*		Exports;

	VProgsReader(FILE* InFile)
	: File(InFile)
	, NameRemap(0)
	, NumExports(0)
	, Exports(0)
	{
		bLoading = true;
	}
	~VProgsReader()
	{
		fclose(File);
	}

	//	Stream interface.
	void Serialise(void* V, int Length)
	{
		if (fread(V, Length, 1, File) != 1)
		{
			bError = true;
		}
	}
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
	void Flush()
	{
		if (fflush(File))
		{
			bError = true;
		}
	}
	bool Close()
	{
		return !bError;
	}

	VStream& operator<<(VName& Name)
	{
		int NameIndex;
		*this << STRM_INDEX(NameIndex);
		Name = NameRemap[NameIndex];
		return *this;
	}
	VStream& operator<<(VMemberBase*& Ref)
	{
		int ObjIndex;
		*this << STRM_INDEX(ObjIndex);
		if (ObjIndex > 0)
		{
			Ref = Exports[ObjIndex - 1].Obj;
		}
		else if (ObjIndex < 0)
		{
			Ref = Imports[-ObjIndex - 1].Obj;
		}
		else
		{
			Ref = NULL;
		}
		return *this;
	}

	VMemberBase* GetImport(int Index)
	{
		VProgsImport& I = Imports[Index];
		if (!I.Obj)
		{
			if (I.Type == MEMBER_Package)
				I.Obj = LoadPackage(I.Name);
			else
				I.Obj = VMemberBase::StaticFindMember(I.Name,
					GetImport(-I.OuterIndex - 1), I.Type);
		}
		return I.Obj;
	}
	void ResolveImports()
	{
		for (int i = 0; i < NumImports; i++)
			GetImport(i);
	}
};

//==========================================================================
//
//	VProgsWriter
//
//==========================================================================

class VProgsWriter : public VStream
{
private:
	FILE*		File;

public:
	vint32*					NamesMap;
	vint32*					MembersMap;
	TArray<VName>			Names;
	TArray<VProgsImport>	Imports;
	TArray<VProgsExport>	Exports;

	VProgsWriter(FILE* InFile)
	: File(InFile)
	{
		bLoading = false;
		NamesMap = new vint32[VName::GetNumNames()];
		for (int i = 0; i < VName::GetNumNames(); i++)
			NamesMap[i] = -1;
		MembersMap = new vint32[VMemberBase::GMembers.Num()];
		memset(MembersMap, 0, VMemberBase::GMembers.Num() * sizeof(vint32));
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
		int TmpIdx = NamesMap[Name.GetIndex()];
		*this << STRM_INDEX(TmpIdx);
		return *this;
	}
	VStream& operator<<(VMemberBase*& Ref)
	{
		int TmpIdx = Ref ? MembersMap[Ref->MemberIndex] : 0;
		*this << STRM_INDEX(TmpIdx);
		return *this;
	}
	int GetMemberIndex(VMemberBase* Obj)
	{
		if (!Obj)
			return 0;
		if (!MembersMap[Obj->MemberIndex])
		{
			MembersMap[Obj->MemberIndex] = -Imports.Append(VProgsImport(Obj,
				GetMemberIndex(Obj->Outer))) - 1;
		}
		return MembersMap[Obj->MemberIndex];
	}

	void AddExport(VMemberBase* Obj)
	{
		MembersMap[Obj->MemberIndex] = Exports.Append(VProgsExport(Obj)) + 1;
	}
};

//==========================================================================
//
//	VProgsWriter
//
//==========================================================================

class VImportsCollector : public VStream
{
	VProgsWriter		&Writer;

public:
	VImportsCollector(VProgsWriter& InWriter)
	: Writer(InWriter)
	{
		bLoading = false;
	}
	VStream& operator<<(VName& Name)
	{
		if (Writer.NamesMap[Name.GetIndex()] == -1)
			Writer.NamesMap[Name.GetIndex()] = Writer.Names.Append(Name);
		return *this;
	}
	VStream& operator<<(VMemberBase*& Ref)
	{
		if (Ref != CurrentPackage)
			Writer.GetMemberIndex(Ref);
		return *this;
	}
};

//==========================================================================
//
//	LoadPackage
//
//==========================================================================

VPackage* LoadPackage(VName InName)
{
	int				i;
	VName*			NameRemap;
	dprograms_t		Progs;
	VProgsReader*	Reader;

	//	Check if already loaded.
	for (i = 0; i < LoadedPackages.Num(); i++)
		if (LoadedPackages[i]->Name == InName)
			return LoadedPackages[i];

	//	Load PROGS from a specified file
	FILE* f = fopen(va("%s.dat", *InName), "rb");
	if (!f)
	{
		for (i = 0; i < PackagePath.Num(); i++)
		{
			f = fopen(va("%s/%s.dat", PackagePath[i], *InName), "rb");
			if (f)
				break;
		}
	}
	if (!f)
	{
		ParseError("Can't find package");
		return NULL;
	}
	Reader = new VProgsReader(f);

	// byte swap the header
	Reader->Seek(0);
	Reader->Serialise(Progs.magic, 4);
	for (i = 1; i < (int)sizeof(Progs) / 4; i++)
	{
		*Reader << ((int*)&Progs)[i];
	}

	if (strncmp(Progs.magic, PROG_MAGIC, 4))
	{
		ParseError("Progs has wrong file ID, possibly older version");
		BailOut();
	}
	if (Progs.version != PROG_VERSION)
	{
		ParseError("Progs has wrong version number (%i should be %i)",
			Progs.version, PROG_VERSION);
		BailOut();
	}

	// Read names
	NameRemap = new VName[Progs.num_names];
	Reader->Seek(Progs.ofs_names);
	for (i = 0; i < Progs.num_names; i++)
	{
		VNameEntry E;
		*Reader << E;
		NameRemap[i] = E.Name;
	}
	Reader->NameRemap = NameRemap;

	Reader->Imports = new VProgsImport[Progs.num_imports];
	Reader->NumImports = Progs.num_imports;
	Reader->Seek(Progs.ofs_imports);
	for (i = 0; i < Progs.num_imports; i++)
	{
		*Reader << Reader->Imports[i];
	}
	Reader->ResolveImports();

	VProgsExport* Exports = new VProgsExport[Progs.num_exports];
	Reader->Exports = Exports;
	Reader->NumExports = Progs.num_exports;

	VPackage* Pkg = new VPackage(InName);
	LoadedPackages.Append(Pkg);

	//	Create objects
	Reader->Seek(Progs.ofs_exportinfo);
	for (i = 0; i < Progs.num_exports; i++)
	{
		*Reader << Exports[i];
		switch (Exports[i].Type)
		{
		case MEMBER_Package:
			Exports[i].Obj = new VPackage(Exports[i].Name);
			break;
		case MEMBER_Field:
			Exports[i].Obj = new VField(Exports[i].Name, NULL, TLocation());
			break;
		case MEMBER_Method:
			Exports[i].Obj = new VMethod(Exports[i].Name, NULL, TLocation());
			break;
		case MEMBER_State:
			Exports[i].Obj = new VState(Exports[i].Name, NULL, TLocation());
			break;
		case MEMBER_Const:
			Exports[i].Obj = new VConstant(Exports[i].Name, NULL, TLocation());
			break;
		case MEMBER_Struct:
			Exports[i].Obj = new VStruct(Exports[i].Name, NULL, TLocation());
			break;
		case MEMBER_Class:
			Exports[i].Obj = new VClass(Exports[i].Name, NULL, TLocation());
			break;
		}
	}

	//	Serialise objects.
	Reader->Seek(Progs.ofs_exportdata);
	for (i = 0; i < Progs.num_exports; i++)
	{
		Exports[i].Obj->Serialise(*Reader);
		if (!Exports[i].Obj->Outer)
			Exports[i].Obj->Outer = Pkg;
	}

	delete[] NameRemap;
	delete[] Exports;
	delete Reader;
	return Pkg;
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

	VProgsWriter Writer(f);

	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->IsIn(CurrentPackage))
			Writer.AddExport(VMemberBase::GMembers[i]);
	}

	//
	//	Collect list of imported objects and used names.
	//
	VImportsCollector Collector(Writer);
	for (i = 0; i < Writer.Exports.Num(); i++)
	{
		Collector << Writer.Exports[i];
	}
	for (i = 0; i < Writer.Exports.Num(); i++)
	{
		Writer.Exports[i].Obj->Serialise(Collector);
	}
	for (i = 0; i < Writer.Imports.Num(); i++)
	{
		Collector << Writer.Imports[i];
	}

	//
	//	Now write the object file.
	//
	memset(&progs, 0, sizeof(progs));
	Writer.Serialise(&progs, sizeof(progs));

	//	Serialise names.
	progs.ofs_names = Writer.Tell();
	progs.num_names = Writer.Names.Num();
	for (i = 0; i < Writer.Names.Num(); i++)
	{
		Writer << *VName::GetEntry(Writer.Names[i].GetIndex());
	}

	progs.ofs_strings = Writer.Tell();
	progs.num_strings = strings.Num();
	Writer.Serialise(&strings[0], strings.Num());

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

	//	Serialise imports.
	progs.num_imports = Writer.Imports.Num();
	progs.ofs_imports = Writer.Tell();
	for (i = 0; i < Writer.Imports.Num(); i++)
	{
		Writer << Writer.Imports[i];
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
	dprintf("Names      %6d %6d\n", Writer.Names.Num(), progs.ofs_strings - progs.ofs_names);
	dprintf("Strings    %6d %6d\n", StringInfo.Num(), strings.Num());
	dprintf("Builtins   %6d\n", numbuiltins);
	dprintf("Mobj info  %6d %6d\n", mobj_info.Num(), progs.ofs_scriptids - progs.ofs_mobjinfo);
	dprintf("Script Ids %6d %6d\n", script_ids.Num(), progs.ofs_imports - progs.ofs_scriptids);
	dprintf("Imports    %6d %6d\n", Writer.Imports.Num(), progs.ofs_exportinfo - progs.ofs_imports);
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
	dprintf("%d opcodes\n", NUM_OPCODES);
#endif
}

//==========================================================================
//
//	DumpAsmFunction
//
//	Disassembles a function.
//
//==========================================================================

void DumpAsmFunction(VMethod* Func)
{
	dprintf("--------------------------------------------\n");
	dprintf("Dump ASM function %s.%s\n\n", *Func->Outer->Name,
		*Func->Name);
	if (Func->Flags & FUNC_Native)
	{
		//	Builtin function
		dprintf("Builtin function.\n");
		return;
	}
	for (int s = 0; s < Func->Instructions.Num(); s++)
	{
		//	Opcode
		int st = Func->Instructions[s].Opcode;
		dprintf("%6d: %s", s, StatementInfo[st].name);
		switch (StatementInfo[st].Args)
		{
		case OPCARGS_None:
			break;
		case OPCARGS_Member:
			//	Name of the object
			dprintf(" %s.%s", *Func->Instructions[s].Member->Outer->Name,
				*Func->Instructions[s].Member->Name);
			break;
		case OPCARGS_BranchTarget:
			dprintf(" %6d", Func->Instructions[s].Arg1);
			break;
		case OPCARGS_ByteBranchTarget:
		case OPCARGS_ShortBranchTarget:
		case OPCARGS_IntBranchTarget:
			dprintf(" %6d, %6d", Func->Instructions[s].Arg1, Func->Instructions[s].Arg2);
			break;
		case OPCARGS_Byte:
		case OPCARGS_Short:
		case OPCARGS_Int:
			dprintf(" %6d (%x)", Func->Instructions[s].Arg1, Func->Instructions[s].Arg1);
			break;
		case OPCARGS_Name:
			//  Name
			dprintf("\'%s\'", *Func->Instructions[s].NameArg);
			break;
		case OPCARGS_String:
			//  String
			dprintf("\"%s\"", &strings[Func->Instructions[s].Arg1]);
			break;
		case OPCARGS_FieldOffset:
			dprintf(" %s", *Func->Instructions[s].Member->Name);
			break;
		case OPCARGS_VTableIndex:
			dprintf(" %s", *Func->Instructions[s].Member->Name);
			break;
		case OPCARGS_TypeSize:
			{
				char Tmp[256];
				Func->Instructions[s].TypeArg.GetName(Tmp);
				dprintf(" %s", Tmp);
			}
			break;
		}
		dprintf("\n");
	}
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
	vuint8 RealType = T.type;
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
	Strm << STRM_INDEX(NumLocals)
		<< STRM_INDEX(Flags)
		<< ReturnType
		<< STRM_INDEX(NumParams)
		<< STRM_INDEX(ParamsSize);
	for (int i = 0; i < NumParams; i++)
		Strm << ParamTypes[i];

	if (Strm.IsLoading())
	{
		int NumInstructions;
		Strm << STRM_INDEX(NumInstructions);
		Instructions.SetNum(NumInstructions);
	}
	else
	{
		int NumInstructions = Instructions.Num();
		Strm << STRM_INDEX(NumInstructions);
	}
	for (int i = 0; i < Instructions.Num(); i++)
	{
		vuint8 Opc;
		if (Strm.IsLoading())
		{
			Strm << Opc;
			Instructions[i].Opcode = Opc;
		}
		else
		{
			Opc = Instructions[i].Opcode;
			Strm << Opc;
		}
		switch (StatementInfo[Opc].Args)
		{
		case OPCARGS_None:
			break;
		case OPCARGS_Member:
		case OPCARGS_FieldOffset:
		case OPCARGS_VTableIndex:
			Strm << Instructions[i].Member;
			break;
		case OPCARGS_BranchTarget:
			Strm << Instructions[i].Arg1;
			break;
		case OPCARGS_ByteBranchTarget:
		case OPCARGS_ShortBranchTarget:
		case OPCARGS_IntBranchTarget:
			Strm << STRM_INDEX(Instructions[i].Arg1);
			Strm << Instructions[i].Arg2;
			break;
		case OPCARGS_Byte:
		case OPCARGS_Short:
		case OPCARGS_Int:
			Strm << STRM_INDEX(Instructions[i].Arg1);
			break;
		case OPCARGS_Name:
			Strm << Instructions[i].NameArg;
			break;
		case OPCARGS_String:
			Strm << Instructions[i].Arg1;
			break;
		case OPCARGS_TypeSize:
			Strm << Instructions[i].TypeArg;
			break;
		}
	}
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
		<< STRM_INDEX(StackSize)
		<< Fields;
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
		<< DefaultProperties;
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
		<< STRM_INDEX(Frame)
		<< ModelName
		<< STRM_INDEX(ModelFrame)
		<< Time
		<< NextState
		<< Function
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
		Strm << FloatValue;
		break;

	case ev_name:
		Strm << *(VName*)&Value;
		break;

	default:
		Strm << STRM_INDEX(Value);
		break;
	}
}
