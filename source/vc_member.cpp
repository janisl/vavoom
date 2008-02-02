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

#ifdef IN_VCC
#include "../utils/vcc/vcc.h"
#else
#include "gamedefs.h"
#include "progdefs.h"
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

//==========================================================================
//
//	VProgsReader
//
//==========================================================================

#ifdef IN_VCC
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
				I.Obj = VMemberBase::LoadPackage(I.Name, TLocation());
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
#else
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
				I.Obj = VMemberBase::StaticLoadPackage(I.Name);
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
#endif

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#ifdef IN_VCC
TArray<VMemberBase*>	VMemberBase::GMembers;
VMemberBase*			VMemberBase::GMembersHash[4096];

TArray<const char*>		VMemberBase::PackagePath;
TArray<VPackage*>		VMemberBase::LoadedPackages;
#else
bool					VMemberBase::GObjInitialised;
VClass*					VMemberBase::GClasses;
TArray<VMemberBase*>	VMemberBase::GMembers;
TArray<VPackage*>		VMemberBase::GLoadedPackages;
TArray<VClass*>			VMemberBase::GNetClassLookup;
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

#ifdef IN_VCC

//==========================================================================
//
//	VProgsImport::VProgsImport
//
//==========================================================================

VProgsImport::VProgsImport(VMemberBase* InObj, vint32 InOuterIndex)
: Type(InObj->MemberType)
, Name(InObj->Name)
, OuterIndex(InOuterIndex)
, Obj(InObj)
{
}

//==========================================================================
//
//	VProgsExport::VProgsExport
//
//==========================================================================

VProgsExport::VProgsExport(VMemberBase* InObj)
: Type(InObj->MemberType)
, Name(InObj->Name)
, Obj(InObj)
{
}

#else

//==========================================================================
//
//	operator VStream << mobjinfo_t
//
//==========================================================================

VStream& operator<<(VStream& Strm, mobjinfo_t& MI)
{
	return Strm << STRM_INDEX(MI.doomednum)
		<< STRM_INDEX(MI.GameFilter)
		<< MI.class_id;
}

#endif

//==========================================================================
//
//	VMemberBase::VMemberBase
//
//==========================================================================

#ifndef IN_VCC
VMemberBase::VMemberBase(vuint8 AMemberType, VName AName)
: MemberType(AMemberType)
, Outer(0)
, Name(AName)
{
	if (GObjInitialised)
	{
		GMembers.Append(this);
	}
}
#else
VMemberBase::VMemberBase(vuint8 AType, VName AName, VMemberBase* AOuter,
	TLocation ALoc)
: MemberType(AType)
, Name(AName)
, Outer(AOuter)
, Loc(ALoc)
{
	MemberIndex = GMembers.Append(this);
	int HashIndex = Name.GetIndex() & 4095;
	HashNext = GMembersHash[HashIndex];
	GMembersHash[HashIndex] = this;
}
#endif

//==========================================================================
//
//	VMemberBase::~VMemberBase
//
//==========================================================================

VMemberBase::~VMemberBase()
{
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

#ifdef IN_VCC

//==========================================================================
//
//	VMemberBase::IsIn
//
//==========================================================================

bool VMemberBase::IsIn(VMemberBase* SomeOuter) const
{
	for (VMemberBase* Tst = Outer; Tst; Tst = Tst->Outer)
		if (Tst == SomeOuter)
			return true;
	return !SomeOuter;
}

//==========================================================================
//
//	VMemberBase::AddPackagePath
//
//==========================================================================

void VMemberBase::AddPackagePath(const char* Path)
{
	PackagePath.Append(Path);
}

//==========================================================================
//
//	VMemberBase::LoadPackage
//
//==========================================================================

VPackage* VMemberBase::LoadPackage(VName InName, TLocation l)
{
	int				i;
	VName*			NameRemap;
	dprograms_t		Progs;
	VProgsReader*	Reader;

	//	Check if already loaded.
	for (i = 0; i < LoadedPackages.Num(); i++)
		if (LoadedPackages[i]->Name == InName)
			return LoadedPackages[i];

	dprintf("Loading package %s\n", *InName);

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
		ParseError(l, "Can't find package %s", *InName);
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

	if (VStr::NCmp(Progs.magic, PROG_MAGIC, 4))
	{
		ParseError(l, "Package %s has wrong file ID", *InName);
		BailOut();
	}
	if (Progs.version != PROG_VERSION)
	{
		ParseError(l, "Package %s has wrong version number (%i should be %i)",
			*InName, Progs.version, PROG_VERSION);
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
//	VMemberBase::StaticFindMember
//
//==========================================================================

VMemberBase* VMemberBase::StaticFindMember(VName InName,
	VMemberBase* InOuter, vuint8 InType)
{
	int HashIndex = InName.GetIndex() & 4095;
	for (VMemberBase* m = GMembersHash[HashIndex]; m; m = m->HashNext)
	{
		if (m->Name == InName && (m->Outer == InOuter ||
			(InOuter == ANY_PACKAGE && m->Outer->MemberType == MEMBER_Package)) &&
			(InType == ANY_MEMBER || m->MemberType == InType))
		{
			return m;
		}
	}
	return NULL;
}

//==========================================================================
//
//	VMemberBase::CheckForType
//
//==========================================================================

VFieldType VMemberBase::CheckForType(VClass* InClass, VName Name)
{
	if (Name == NAME_None)
	{
		return VFieldType(TYPE_Unknown);
	}

	VMemberBase* m = StaticFindMember(Name, ANY_PACKAGE, MEMBER_Class);
	if (m)
	{
		return VFieldType((VClass*)m);
	}
	m = StaticFindMember(Name, InClass ? (VMemberBase*)InClass :
		(VMemberBase*)ANY_PACKAGE, MEMBER_Struct);
	if (m)
	{
		return VFieldType((VStruct*)m);
	}
	if (InClass)
	{
		return CheckForType(InClass->ParentClass, Name);
	}
	return VFieldType(TYPE_Unknown);
}

//==========================================================================
//
//	VMemberBase::CheckForClass
//
//==========================================================================

VClass* VMemberBase::CheckForClass(VName Name)
{
	VMemberBase* m = StaticFindMember(Name, ANY_PACKAGE, MEMBER_Class);
	if (m)
	{
		return (VClass*)m;
	}
	return NULL;
}

#else

//==========================================================================
//
//	VMemberBase::GetFullName
//
//==========================================================================

VStr VMemberBase::GetFullName() const
{
	if (Outer)
		return Outer->GetFullName() + "." + Name;
	return VStr(Name);
}

//==========================================================================
//
//	VMemberBase::GetPackage
//
//==========================================================================

VPackage* VMemberBase::GetPackage() const
{
	guard(VMemberBase::GetPackage);
	for (const VMemberBase* p = this; p; p = p->Outer)
		if (p->MemberType == MEMBER_Package)
			return (VPackage*)p;
	Sys_Error("Member object %s not in a package", *GetFullName());
	return NULL;
	unguard;
}

//==========================================================================
//
//	VMemberBase::PostLoad
//
//==========================================================================

void VMemberBase::PostLoad()
{
}

//==========================================================================
//
//	VMemberBase::Shutdown
//
//==========================================================================

void VMemberBase::Shutdown()
{
}

//==========================================================================
//
//	VMemberBase::StaticInit
//
//==========================================================================

void VMemberBase::StaticInit()
{
	guard(VMemberBase::StaticInit);
	for (VClass* C = GClasses; C; C = C->LinkNext)
		GMembers.Append(C);
	GObjInitialised = true;
	unguard;
}

//==========================================================================
//
//	VMemberBase::StaticExit
//
//==========================================================================

void VMemberBase::StaticExit()
{
	for (int i = 0; i < GMembers.Num(); i++)
	{
		if (GMembers[i]->MemberType != MEMBER_Class ||
			!(((VClass*)GMembers[i])->ObjectFlags & CLASSOF_Native))
		{
			delete GMembers[i];
		}
		else
		{
			GMembers[i]->Shutdown();
		}
	}
	GMembers.Clear();
	GLoadedPackages.Clear();
	GNetClassLookup.Clear();
	VClass::GMobjInfos.Clear();
	VClass::GScriptIds.Clear();
	VClass::GSpriteNames.Clear();
	GObjInitialised = false;
}

//==========================================================================
//
//	VMemberBase::StaticLoadPackage
//
//==========================================================================

VPackage* VMemberBase::StaticLoadPackage(VName InName)
{
	guard(VMemberBase::StaticLoadPackage);
	int				i;
	VName*			NameRemap;
	dprograms_t		Progs;
	TCRC			crc;
	VProgsReader*	Reader;

	//	Check if already loaded.
	for (i = 0; i < GLoadedPackages.Num(); i++)
		if (GLoadedPackages[i]->Name == InName)
			return GLoadedPackages[i];

	if (fl_devmode && FL_FileExists(va("progs/%s.dat", *InName)))
	{
		//	Load PROGS from a specified file
		Reader = new VProgsReader(FL_OpenFileRead(va("progs/%s.dat", *InName)));
	}
	else
	{
		//	Load PROGS from wad file
		Reader = new VProgsReader(W_CreateLumpReaderName(InName, WADNS_Progs));
	}

	//	Calcutate CRC
	crc.Init();
	for (i = 0; i < Reader->TotalSize(); i++)
	{
		crc + Streamer<vuint8>(*Reader);
	}

	// Read the header
	Reader->Seek(0);
	Reader->Serialise(Progs.magic, 4);
	for (i = 1; i < (int)sizeof(Progs) / 4; i++)
	{
		*Reader << ((int*)&Progs)[i];
	}

	if (VStr::NCmp(Progs.magic, PROG_MAGIC, 4))
		Sys_Error("Progs has wrong file ID, possibly older version");
	if (Progs.version != PROG_VERSION)
		Sys_Error("Progs has wrong version number (%i should be %i)",
			Progs.version, PROG_VERSION);

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
	GLoadedPackages.Append(Pkg);
	Pkg->Checksum = crc;
	Pkg->Reader = Reader;

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
			Exports[i].Obj = new VField(Exports[i].Name);
			break;
		case MEMBER_Property:
			Exports[i].Obj = new VProperty(Exports[i].Name);
			break;
		case MEMBER_Method:
			Exports[i].Obj = new VMethod(Exports[i].Name);
			break;
		case MEMBER_State:
			Exports[i].Obj = new VState(Exports[i].Name);
			break;
		case MEMBER_Const:
			Exports[i].Obj = new VConstant(Exports[i].Name);
			break;
		case MEMBER_Struct:
			Exports[i].Obj = new VStruct(Exports[i].Name);
			break;
		case MEMBER_Class:
			Exports[i].Obj = VClass::FindClass(*Exports[i].Name);
			if (!Exports[i].Obj)
			{
				Exports[i].Obj = new VClass(Exports[i].Name);
			}
			break;
		}
	}

	//	Read strings.
	Pkg->Strings = new char[Progs.num_strings];
	Reader->Seek(Progs.ofs_strings);
	Reader->Serialise(Pkg->Strings, Progs.num_strings);

	//	Serialise objects.
	Reader->Seek(Progs.ofs_exportdata);
	for (i = 0; i < Progs.num_exports; i++)
	{
		Exports[i].Obj->Serialise(*Reader);
		if (!Exports[i].Obj->Outer)
			Exports[i].Obj->Outer = Pkg;
	}

	//	Set up info tables.
	Reader->Seek(Progs.ofs_mobjinfo);
	for (i = 0; i < Progs.num_mobjinfo; i++)
	{
		*Reader << VClass::GMobjInfos.Alloc();
	}
	Reader->Seek(Progs.ofs_scriptids);
	for (i = 0; i < Progs.num_scriptids; i++)
	{
		*Reader << VClass::GScriptIds.Alloc();
	}

	for (i = 0; i < Progs.num_exports; i++)
	{
		Exports[i].Obj->PostLoad();
	}

	//	Create default objects.
	for (i = 0; i < Progs.num_exports; i++)
	{
		if (Exports[i].Obj->MemberType == MEMBER_Class)
		{
			((VClass*)Exports[i].Obj)->CreateDefaults();
		}
	}

	if (InName == "engine")
	{
		for (VClass* Cls = GClasses; Cls; Cls = Cls->LinkNext)
		{
			if (!Cls->Outer)
			{
				Cls->PostLoad();
				Cls->CreateDefaults();
				Cls->Outer = Pkg;
			}
		}
	}

	delete Reader;
	Pkg->Reader = NULL;
	return Pkg;
	unguard;
}

//==========================================================================
//
//	VMemberBase::StaticFindMember
//
//==========================================================================

VMemberBase* VMemberBase::StaticFindMember(VName InName,
	VMemberBase* InOuter, vuint8 InType)
{
	guard(VMemberBase::StaticFindMember);
	for (int i = 0; i < GMembers.Num(); i++)
		if (GMembers[i]->MemberType == InType &&
			GMembers[i]->Name == InName && GMembers[i]->Outer == InOuter)
			return GMembers[i];
	return NULL;
	unguard;
}

//==========================================================================
//
//	VMemberBase::SetUpNetClasses
//
//==========================================================================

void VMemberBase::SetUpNetClasses()
{
	guard(VMemberBase::SetUpNetClasses);
	GNetClassLookup.Clear();
	GNetClassLookup.Append(NULL);
	for (int i = 0; i < GMembers.Num(); i++)
	{
		if (GMembers[i]->MemberType == MEMBER_Class)
		{
			VClass* C = static_cast<VClass*>(GMembers[i]);
			if (C->IsChildOf(VThinker::StaticClass()))
			{
				C->NetId = GNetClassLookup.Num();
				GNetClassLookup.Append(C);
			}
		}
	}
	unguard;
}

#endif
