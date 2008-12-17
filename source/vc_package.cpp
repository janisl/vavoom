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

#include "vc_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

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
//	VImportsCollector
//
//==========================================================================

class VImportsCollector : public VStream
{
	VProgsWriter		&Writer;
	VPackage*			Package;

public:
	VImportsCollector(VProgsWriter& AWriter, VPackage* APackage)
	: Writer(AWriter)
	, Package(APackage)
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
		if (Ref != Package)
			Writer.GetMemberIndex(Ref);
		return *this;
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
	VStream*			Stream;

public:
	VName*				NameRemap;
	int					NumImports;
	VProgsImport*		Imports;
	int					NumExports;
	VProgsExport*		Exports;

	VProgsReader(VStream* InStream)
	: Stream(InStream)
	, NameRemap(0)
	, NumImports(0)
	, NumExports(0)
	, Exports(0)
	{
		bLoading = true;
	}
	~VProgsReader()
	{
		delete[] NameRemap;
		delete[] Imports;
		delete[] Exports;
		delete Stream;
	}

	//	Stream interface.
	void Serialise(void* Data, int Len)
	{
		Stream->Serialise(Data, Len);
	}
	void Seek(int Pos)
	{
		Stream->Seek(Pos);
	}
	int Tell()
	{
		return Stream->Tell();
	}
	int TotalSize()
	{
		return Stream->TotalSize();
	}
	bool AtEnd()
	{
		return Stream->AtEnd();
	}
	void Flush()
	{
		Stream->Flush();
	}
	bool Close()
	{
		return Stream->Close();
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
			check(ObjIndex <= NumExports);
			Ref = Exports[ObjIndex - 1].Obj;
		}
		else if (ObjIndex < 0)
		{
			check(-ObjIndex <= NumImports);
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
			{
				I.Obj = VMemberBase::StaticLoadPackage(I.Name, TLocation());
			}
			else if (I.Type == MEMBER_DecorateClass)
			{
				for (int i = 0; i < VMemberBase::GDecorateClassImports.Num(); i++)
				{
					if (VMemberBase::GDecorateClassImports[i]->Name == I.Name)
					{
						I.Obj = VMemberBase::GDecorateClassImports[i];
						break;
					}
				}
				if (!I.Obj)
				{
					I.Obj = VClass::FindClass(*I.Name);
				}
				if (!I.Obj)
				{
					VClass* Tmp = new VClass(I.Name, NULL, TLocation());
					Tmp->MemberType = MEMBER_DecorateClass;
					Tmp->ParentClassName = I.ParentClassName;
					VMemberBase::GDecorateClassImports.Append(Tmp);
					I.Obj = Tmp;
				}
			}
			else
			{
				I.Obj = VMemberBase::StaticFindMember(I.Name,
					GetImport(-I.OuterIndex - 1), I.Type);
			}
		}
		return I.Obj;
	}
	void ResolveImports()
	{
		for (int i = 0; i < NumImports; i++)
			GetImport(i);
	}
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	operator VStream << mobjinfo_t
//
//==========================================================================

VStream& operator<<(VStream& Strm, mobjinfo_t& MI)
{
	return Strm << STRM_INDEX(MI.DoomEdNum)
		<< STRM_INDEX(MI.GameFilter)
		<< MI.Class;
}

//==========================================================================
//
//	VPackage::VPackage
//
//==========================================================================

VPackage::VPackage()
: VMemberBase(MEMBER_Package, NAME_None, NULL, TLocation())
, NumBuiltins(0)
, Checksum(0)
, Reader(NULL)
{
	//	Strings
	memset(StringLookup, 0, 256 * 4);
	//	1-st string is empty
	StringInfo.Alloc();
	StringInfo[0].Offs = 0;
	StringInfo[0].Next = 0;
	Strings.SetNum(4);
	memset(Strings.Ptr(), 0, 4);
}

//==========================================================================
//
//	VPackage::VPackage
//
//==========================================================================

VPackage::VPackage(VName AName)
: VMemberBase(MEMBER_Package, AName, NULL, TLocation())
, NumBuiltins(0)
, Checksum(0)
, Reader(NULL)
{
	//	Strings
	memset(StringLookup, 0, 256 * 4);
	//	1-st string is empty
	StringInfo.Alloc();
	StringInfo[0].Offs = 0;
	StringInfo[0].Next = 0;
	Strings.SetNum(4);
	memset(Strings.Ptr(), 0, 4);
}

//==========================================================================
//
//	VPackage::~VPackage
//
//==========================================================================

VPackage::~VPackage()
{
}

//==========================================================================
//
//	VPackage::Serialise
//
//==========================================================================

void VPackage::Serialise(VStream& Strm)
{
	guard(VPackage::Serialise);
	VMemberBase::Serialise(Strm);
	unguard;
}

//==========================================================================
//
//	VPackage::StringHashFunc
//
//==========================================================================

int VPackage::StringHashFunc(const char *str)
{
	return (*str ^ (str[1] << 4)) & 0xff;
}

//==========================================================================
//
//  VPackage::FindString
//
//  Return offset in strings array.
//
//==========================================================================

int VPackage::FindString(const char *str)
{
	guard(VPackage::FindString);
	if (!*str)
	{
		return 0;
	}
	int hash = StringHashFunc(str);
	for (int i = StringLookup[hash]; i; i = StringInfo[i].Next)
	{
		if (!VStr::Cmp(&Strings[StringInfo[i].Offs], str))
		{
			return StringInfo[i].Offs;
		}
	}

	//  Add new string
	TStringInfo& SI = StringInfo.Alloc();
	int AddLen = (VStr::Length(str) + 4) & ~3;
	int Ofs = Strings.Num();
	Strings.SetNum(Strings.Num() + AddLen);
	memset(&Strings[Ofs], 0, AddLen);
	SI.Offs = Ofs;
	SI.Next = StringLookup[hash];
	StringLookup[hash] = StringInfo.Num() - 1;
	VStr::Cpy(&Strings[Ofs], str);
	return SI.Offs;
	unguard;
}

//==========================================================================
//
//	VPackage::FindConstant
//
//==========================================================================

VConstant* VPackage::FindConstant(VName Name)
{
	guard(VPackage::FindConstant);
	VMemberBase* m = StaticFindMember(Name, this, MEMBER_Const);
	if (m)
	{
		return (VConstant*)m;
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VPackage::FindDecorateImportClass
//
//==========================================================================

VClass* VPackage::FindDecorateImportClass(VName AName) const
{
	guard(VPackage::FindDecorateImportClass);
	for (int i = 0; i < ParsedDecorateImportClasses.Num(); i++)
	{
		if (ParsedDecorateImportClasses[i]->Name == AName)
		{
			return ParsedDecorateImportClasses[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VPackage::Emit
//
//==========================================================================

void VPackage::Emit()
{
	guard(VPackage::Emit);
	dprintf("Importing packages\n");

	for (int i = 0; i < PackagesToLoad.Num(); i++)
	{
		PackagesToLoad[i].Pkg = StaticLoadPackage(PackagesToLoad[i].Name,
			PackagesToLoad[i].Loc);
	}

	if (NumErrors)
	{
		BailOut();
	}

	dprintf("Defining constants\n");

	for (int i = 0; i < ParsedConstants.Num(); i++)
	{
		ParsedConstants[i]->Define();
	}

	dprintf("Defining structs\n");

	for (int i = 0; i < ParsedStructs.Num(); i++)
	{
		ParsedStructs[i]->Define();
	}

	dprintf("Defining classes\n");

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->Define();
	}

	for (int i = 0; i < ParsedDecorateImportClasses.Num(); i++)
	{
		ParsedDecorateImportClasses[i]->Define();
	}

	if (NumErrors)
	{
		BailOut();
	}

	dprintf("Defining struct members\n");

	for (int i = 0; i < ParsedStructs.Num(); i++)
	{
		ParsedStructs[i]->DefineMembers();
	}

	dprintf("Defining class members\n");

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->DefineMembers();
	}

	if (NumErrors)
	{
		BailOut();
	}

	dprintf("Emiting classes\n");

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->Emit();
	}

	if (NumErrors)
	{
		BailOut();
	}
	unguard;
}

//==========================================================================
//
//	VPackage::WriteObject
//
//==========================================================================

void VPackage::WriteObject(const VStr& name)
{
	guard(VPackage::WriteObject);
	FILE*			f;
	int				i;
	dprograms_t		progs;

	dprintf("Writing object\n");

	f = fopen(*name, "wb");
	if (!f)
	{
		FatalError("Can't open file \"%s\".", *name);
	}

	VProgsWriter Writer(f);

	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->IsIn(this))
			Writer.AddExport(VMemberBase::GMembers[i]);
	}

	//	Add decorate class imports.
	for (i = 0; i < ParsedDecorateImportClasses.Num(); i++)
	{
		VProgsImport I(ParsedDecorateImportClasses[i], 0);
		I.Type = MEMBER_DecorateClass;
		I.ParentClassName = ParsedDecorateImportClasses[i]->ParentClassName;
		Writer.MembersMap[ParsedDecorateImportClasses[i]->MemberIndex] =
			-Writer.Imports.Append(I) - 1;
	}

	//
	//	Collect list of imported objects and used names.
	//
	VImportsCollector Collector(Writer, this);
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
	progs.num_strings = Strings.Num();
	Writer.Serialise(&Strings[0], Strings.Num());

	progs.ofs_mobjinfo = Writer.Tell();
	progs.num_mobjinfo = MobjInfo.Num();
	for (i = 0; i < MobjInfo.Num(); i++)
	{
		Writer << MobjInfo[i];
	}

	progs.ofs_scriptids = Writer.Tell();
	progs.num_scriptids = ScriptIds.Num();
	for (i = 0; i < ScriptIds.Num(); i++)
	{
		Writer << ScriptIds[i];
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
	dprintf("Strings    %6d %6d\n", StringInfo.Num(), Strings.Num());
	dprintf("Builtins   %6d\n", NumBuiltins);
	dprintf("Mobj info  %6d %6d\n", MobjInfo.Num(), progs.ofs_scriptids - progs.ofs_mobjinfo);
	dprintf("Script Ids %6d %6d\n", ScriptIds.Num(), progs.ofs_imports - progs.ofs_scriptids);
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
	unguard;
}

//==========================================================================
//
//	VPackage::LoadObject
//
//==========================================================================

void VPackage::LoadObject(TLocation l)
{
	guard(VPackage::LoadObject);
#ifdef IN_VCC
	dprintf("Loading package %s\n", *Name);

	//	Load PROGS from a specified file
	VStream* f = OpenFile(va("%s.dat", *Name));
	if (!f)
	{
		for (int i = 0; i < GPackagePath.Num(); i++)
		{
			f = OpenFile(GPackagePath[i] + "/" + Name + ".dat");
			if (f)
			{
				break;
			}
		}
	}
	if (!f)
	{
		ParseError(l, "Can't find package %s", *Name);
		return;
	}
	VProgsReader* Reader = new VProgsReader(f);
#else
	//	Load PROGS from a specified file
	VStream* Strm = FL_OpenFileRead(va("progs/%s.dat", *Name));
	if (!Strm)
	{
		if (FL_FileExists(va("progs/%s/classes.vc", *Name)))
		{
			//	Compile package
			VLexer Lex;
			Lex.OpenSource(va("progs/%s/classes.vc", *Name));
			VParser Parser(Lex, this);
			Parser.Parse();
			Emit();

			//	Copy mobj infos and spawn IDs.
			for (int i = 0; i < MobjInfo.Num(); i++)
			{
				VClass::GMobjInfos.Alloc() = MobjInfo[i];
			}
			for (int i = 0; i < ScriptIds.Num(); i++)
			{
				VClass::GScriptIds.Alloc() = ScriptIds[i];
			}

			for (int i = 0; i < GMembers.Num(); i++)
			{
				if (GMembers[i]->IsIn(this))
				{
					GMembers[i]->PostLoad();
				}
			}

			//	Create default objects.
			for (int i = 0; i < ParsedClasses.Num(); i++)
			{
				ParsedClasses[i]->CreateDefaults();
			}

			if (Name == NAME_engine)
			{
				for (VClass* Cls = GClasses; Cls; Cls = Cls->LinkNext)
				{
					if (!Cls->Outer && Cls->MemberType == MEMBER_Class)
					{
						Cls->PostLoad();
						Cls->CreateDefaults();
						Cls->Outer = this;
					}
				}
			}

			return;
		}
		Sys_Error("Progs package %s not found", *Name);
	}
	VProgsReader* Reader = new VProgsReader(Strm);

	//	Calcutate CRC
	TCRC crc;
	crc.Init();
	for (int i = 0; i < Reader->TotalSize(); i++)
	{
		crc + Streamer<vuint8>(*Reader);
	}
#endif

	// Read the header
	dprograms_t Progs;
	Reader->Seek(0);
	Reader->Serialise(Progs.magic, 4);
	for (int i = 1; i < (int)sizeof(Progs) / 4; i++)
	{
		*Reader << ((int*)&Progs)[i];
	}

	if (VStr::NCmp(Progs.magic, PROG_MAGIC, 4))
	{
		ParseError(l, "Package %s has wrong file ID", *Name);
		BailOut();
	}
	if (Progs.version != PROG_VERSION)
	{
		ParseError(l, "Package %s has wrong version number (%i should be %i)",
			*Name, Progs.version, PROG_VERSION);
		BailOut();
	}

	// Read names
	VName* NameRemap = new VName[Progs.num_names];
	Reader->Seek(Progs.ofs_names);
	for (int i = 0; i < Progs.num_names; i++)
	{
		VNameEntry E;
		*Reader << E;
		NameRemap[i] = E.Name;
	}
	Reader->NameRemap = NameRemap;

	Reader->Imports = new VProgsImport[Progs.num_imports];
	Reader->NumImports = Progs.num_imports;
	Reader->Seek(Progs.ofs_imports);
	for (int i = 0; i < Progs.num_imports; i++)
	{
		*Reader << Reader->Imports[i];
	}
	Reader->ResolveImports();

	VProgsExport* Exports = new VProgsExport[Progs.num_exports];
	Reader->Exports = Exports;
	Reader->NumExports = Progs.num_exports;

#ifndef IN_VCC
	Checksum = crc;
	this->Reader = Reader;
#endif

	//	Create objects
	Reader->Seek(Progs.ofs_exportinfo);
	for (int i = 0; i < Progs.num_exports; i++)
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
		case MEMBER_Property:
			Exports[i].Obj = new VProperty(Exports[i].Name, NULL, TLocation());
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
#ifndef IN_VCC
			Exports[i].Obj = VClass::FindClass(*Exports[i].Name);
			if (!Exports[i].Obj)
#endif
			{
				Exports[i].Obj = new VClass(Exports[i].Name, NULL, TLocation());
			}
			break;
		}
	}

	//	Read strings.
	Strings.SetNum(Progs.num_strings);
	Reader->Seek(Progs.ofs_strings);
	Reader->Serialise(Strings.Ptr(), Progs.num_strings);

	//	Serialise objects.
	Reader->Seek(Progs.ofs_exportdata);
	for (int i = 0; i < Progs.num_exports; i++)
	{
		Exports[i].Obj->Serialise(*Reader);
		if (!Exports[i].Obj->Outer)
		{
			Exports[i].Obj->Outer = this;
		}
	}

#ifndef IN_VCC
	//	Set up info tables.
	Reader->Seek(Progs.ofs_mobjinfo);
	for (int i = 0; i < Progs.num_mobjinfo; i++)
	{
		*Reader << VClass::GMobjInfos.Alloc();
	}
	Reader->Seek(Progs.ofs_scriptids);
	for (int i = 0; i < Progs.num_scriptids; i++)
	{
		*Reader << VClass::GScriptIds.Alloc();
	}

	for (int i = 0; i < Progs.num_exports; i++)
	{
		Exports[i].Obj->PostLoad();
	}

	//	Create default objects.
	for (int i = 0; i < Progs.num_exports; i++)
	{
		if (Exports[i].Obj->MemberType == MEMBER_Class)
		{
			((VClass*)Exports[i].Obj)->CreateDefaults();
		}
	}

	if (Name == NAME_engine)
	{
		for (VClass* Cls = GClasses; Cls; Cls = Cls->LinkNext)
		{
			if (!Cls->Outer && Cls->MemberType == MEMBER_Class)
			{
				Cls->PostLoad();
				Cls->CreateDefaults();
				Cls->Outer = this;
			}
		}
	}
#endif

	delete Reader;
	this->Reader = NULL;
	unguard;
}
