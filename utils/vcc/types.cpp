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

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

TArray<VMemberBase*>	VMemberBase::GMembers;
VMemberBase*			VMemberBase::GMembersHash[4096];

TArray<const char*>		VMemberBase::PackagePath;
TArray<VPackage*>		VMemberBase::LoadedPackages;

// CODE --------------------------------------------------------------------

//BEGIN TType

//==========================================================================
//
//	TType::TType
//
//==========================================================================

TType::TType(VClass* InClass) :
	type(ev_reference), InnerType(ev_void), ArrayInnerType(ev_void),
	PtrLevel(0), array_dim(0), Class(InClass)
{
}

//==========================================================================
//
//	TType::TType
//
//==========================================================================

TType::TType(VStruct* InStruct) :
	type(InStruct->IsVector ? ev_vector : ev_struct), InnerType(ev_void),
	ArrayInnerType(ev_void), PtrLevel(0), array_dim(0), Struct(InStruct)
{
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
//	TType::Equals
//
//==========================================================================

bool TType::Equals(const TType& Other) const
{
	if (type != Other.type ||
		InnerType != Other.InnerType ||
		ArrayInnerType != Other.ArrayInnerType ||
		PtrLevel != Other.PtrLevel ||
		array_dim != Other.array_dim ||
		bit_mask != Other.bit_mask)
		return false;
	return true;
}

//==========================================================================
//
//	TType::MakePointerType
//
//==========================================================================

TType TType::MakePointerType() const
{
	TType pointer = *this;
	if (pointer.type == ev_pointer)
	{
		pointer.PtrLevel++;
	}
	else
	{
		pointer.InnerType = pointer.type;
		pointer.type = ev_pointer;
		pointer.PtrLevel = 1;
	}
	return pointer;
}

//==========================================================================
//
//	TType::GetPointerInnerType
//
//==========================================================================

TType TType::GetPointerInnerType() const
{
	if (type != ev_pointer)
	{
		FatalError("Not a pointer type");
		return *this;
	}
	TType ret = *this;
	ret.PtrLevel--;
	if (ret.PtrLevel <= 0)
	{
		ret.type = InnerType;
		ret.InnerType = ev_void;
	}
	return ret;
}

//==========================================================================
//
//	TType::MakeArrayType
//
//==========================================================================

TType TType::MakeArrayType(int elcount, TLocation l) const
{
	if (type == ev_array)
	{
		ParseError(l, "Can't have multi-dimensional arrays");
	}
	TType array = *this;
	array.ArrayInnerType = type;
	array.type = ev_array;
	array.array_dim = elcount;
	return array;
}

//==========================================================================
//
//	TType::GetArrayInnerType
//
//==========================================================================

TType TType::GetArrayInnerType() const
{
	if (type != ev_array)
	{
		FatalError("Not an array type");
		return *this;
	}
	TType ret = *this;
	ret.type = ArrayInnerType;
	ret.ArrayInnerType = ev_void;
	ret.array_dim = 0;
	return ret;
}

//==========================================================================
//
//	TType::GetSize
//
//==========================================================================

int TType::GetSize() const
{
	switch (type)
	{
	case ev_int:		return 4;
	case ev_float:		return 4;
	case ev_name:		return 4;
	case ev_string:		return 4;
	case ev_pointer:	return 4;
	case ev_reference:	return 4;
	case ev_array:		return array_dim * GetArrayInnerType().GetSize();
	case ev_struct:		return Struct->StackSize * 4;
	case ev_vector:		return 12;
	case ev_classid:	return 4;
	case ev_state:		return 4;
	case ev_bool:		return 4;
	case ev_delegate:	return 8;
	}
	return 0;
}

//==========================================================================
//
//	TType::CheckPassable
//
//	Check, if type can be pushed into the stack
//
//==========================================================================

void TType::CheckPassable(TLocation l) const
{
	if (GetSize() != 4 && type != ev_vector && type != ev_delegate)
	{
		char Name[256];
		GetName(Name);
		ParseError(l, "Type %s is not passable", Name);
	}
}

//==========================================================================
//
//	TType::CheckMatch
//
//	Check, if types are compatible
//
//	t1 - current type
//	t2 - needed type
//
//==========================================================================

void TType::CheckMatch(TLocation l, const TType& Other) const
{
	CheckPassable(l);
	Other.CheckPassable(l);
	if (Equals(Other))
	{
		return;
	}
	if (type == ev_vector && Other.type == ev_vector)
	{
		return;
	}
	if (type == ev_pointer && Other.type == ev_pointer)
	{
		TType it1 = GetPointerInnerType();
		TType it2 = Other.GetPointerInnerType();
		if (it1.Equals(it2))
		{
			return;
		}
		if ((it1.type == ev_void) || (it2.type == ev_void))
		{
			return;
		}
		if (it1.type == ev_struct && it2.type == ev_struct)
		{
			VStruct* s1 = it1.Struct;
			VStruct* s2 = it2.Struct;
			for (VStruct* st1 = s1->ParentStruct; st1; st1 = st1->ParentStruct)
			{
				if (st1 == s2)
				{
					return;
				}
			}
		}
	}
	if (type == ev_reference && Other.type == ev_reference)
	{
		VClass* c1 = Class;
		VClass* c2 = Other.Class;
		if (!c1 || !c2)
		{
			//	none reference can be assigned to any reference.
			return;
		}
		if (c1 == c2)
		{
			return;
		}
		for (VClass* pc1 = c1->ParentClass; pc1; pc1 = pc1->ParentClass)
		{
			if (pc1 == c2)
			{
				return;
			}
		}
	}
	if (type == ev_int && Other.type == ev_bool)
	{
		return;
	}
	//	Allow assigning none to states, classes and delegates
	if (type == ev_reference && Class == NULL && (Other.type == ev_classid ||
		Other.type == ev_state || Other.type == ev_delegate))
	{
		return;
	}
	if (type == ev_delegate && Other.type == ev_delegate)
	{
		VMethod& F1 = *Function;
		VMethod& F2 = *Other.Function;
		if (F1.Flags & FUNC_Static || F2.Flags & FUNC_Static)
		{
			ParseError(l, "Can't assign a static function to delegate");
		}
		if (!F1.ReturnType.Equals(F2.ReturnType))
		{
			ParseError(l, "Delegate has different return type");
		}
		else if (F1.NumParams != F2.NumParams)
		{
			ParseError(l, "Delegate has different number of arguments");
		}
		else for (int i = 0; i < F1.NumParams; i++)
			if (!F1.ParamTypes[i].Equals(F2.ParamTypes[i]))
			{
				ParseError(l, "Delegate argument %d differs", i + 1);
			}
		return;
	}
	char Name1[256];
	char Name2[256];
	GetName(Name1);
	Other.GetName(Name2);
	ParseError(l, "Type mistmatch, types %s and %s are not compatible %d %d",
		Name1, Name2, type, Other.type);
}

//==========================================================================
//
//	TType::GetName
//
//==========================================================================

void TType::GetName(char* Dest) const
{
	switch (type)
	{
	case ev_int:		strcpy(Dest, "int"); break;
	case ev_float:		strcpy(Dest, "float"); break;
	case ev_name:		strcpy(Dest, "name"); break;
	case ev_string:		strcpy(Dest, "string"); break;
	case ev_pointer:	GetPointerInnerType().GetName(Dest); 
		for (int i = 0; i < PtrLevel; i++) strcat(Dest, "*"); break;
	case ev_reference:	strcpy(Dest, Class ? *Class->Name : "none"); break;
	case ev_array:		GetArrayInnerType().GetName(Dest); strcat(Dest, "[]"); break;
	case ev_struct:		strcpy(Dest, *Struct->Name); break;
	case ev_vector:		strcpy(Dest, "vector"); break;
	case ev_classid:	strcpy(Dest, "classid"); break;
	case ev_state:		strcpy(Dest, "state"); break;
	case ev_bool:		strcpy(Dest, "bool"); break;
	default:			strcpy(Dest, "unknown"); break;
	}
}

//END

//BEGIN Import / export helper classes

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

//END

//BEGIN VMemberBase

//==========================================================================
//
//	VMemberBase::VMemberBase
//
//==========================================================================

VMemberBase::VMemberBase(vuint8 InType, VName InName, VMemberBase* InOuter,
	TLocation InLoc)
: MemberType(InType)
, MemberIndex(GMembers.Append(this))
, Name(InName)
, Outer(InOuter)
, Loc(InLoc)
{
	int HashIndex = Name.GetIndex() & 4095;
	HashNext = GMembersHash[HashIndex];
	GMembersHash[HashIndex] = this;
}

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
//	VMemberBase::Serialise
//
//==========================================================================

void VMemberBase::Serialise(VStream& Strm)
{
	Strm << Outer;
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

	if (strncmp(Progs.magic, PROG_MAGIC, 4))
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

TType VMemberBase::CheckForType(VClass* InClass, VName Name)
{
	if (Name == NAME_None)
	{
		return TType(ev_unknown);
	}

	VMemberBase* m = StaticFindMember(Name, ANY_PACKAGE, MEMBER_Class);
	if (m)
	{
		return TType((VClass*)m);
	}
	m = StaticFindMember(Name, InClass ? (VMemberBase*)InClass :
		(VMemberBase*)ANY_PACKAGE, MEMBER_Struct);
	if (m)
	{
		return TType((VStruct*)m);
	}
	if (InClass)
	{
		return CheckForType(InClass->ParentClass, Name);
	}
	return TType(ev_unknown);
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

//END

//BEGIN VField

//==========================================================================
//
//	VField::VField
//
//==========================================================================

VField::VField(VName InName, VMemberBase* InOuter, TLocation InLoc)
: VMemberBase(MEMBER_Field, InName, InOuter, InLoc)
, Next(NULL)
, type(ev_void)
, TypeExpr(NULL)
, func(NULL)
, Modifiers(0)
, flags(0)
{
}

//==========================================================================
//
//	VField::~VField
//
//==========================================================================

VField::~VField()
{
	if (TypeExpr)
		delete TypeExpr;
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
//	VField::NeedsDestructor
//
//==========================================================================

bool VField::NeedsDestructor() const
{
	if (type.type == ev_string)
		return true;
	if (type.type == ev_array)
	{
		if (type.ArrayInnerType == ev_string)
			return true;
		if (type.ArrayInnerType == ev_struct)
			return type.Struct->NeedsDestructor();
	}
	if (type.type == ev_struct)
		return type.Struct->NeedsDestructor();
	return false;
}

//==========================================================================
//
//	VField::Define
//
//==========================================================================

bool VField::Define()
{
	if (type.type == ev_delegate)
	{
		return func->Define();
	}

	if (TypeExpr)
	{
		VEmitContext ec(this);
		TypeExpr = TypeExpr->ResolveAsType(ec);
	}
	if (!TypeExpr)
	{
		return false;
	}

	if (TypeExpr->Type.type == ev_void)
	{
		ParseError(TypeExpr->Loc, "Field cannot have void type.");
		return false;
	}
	type = TypeExpr->Type;

	Modifiers = TModifiers::Check(Modifiers, AllowedModifiers, Loc);
	flags = TModifiers::FieldAttr(Modifiers);
	return true;
}

//END

//BEGIN VMethod

//==========================================================================
//
//	VMethod::VMethod
//
//==========================================================================

VMethod::VMethod(VName InName, VMemberBase* InOuter, TLocation InLoc)
: VMemberBase(MEMBER_Method, InName, InOuter, InLoc)
, NumLocals(0)
, Flags(0)
, ReturnType(ev_void)
, NumParams(0)
, ParamsSize(0)
, Modifiers(0)
, ReturnTypeExpr(NULL)
, Statement(NULL)
{
	memset(ParamFlags, 0, sizeof(ParamFlags));
}

//==========================================================================
//
//	VMethod::~VMethod
//
//==========================================================================

VMethod::~VMethod()
{
	if (ReturnTypeExpr)
		delete ReturnTypeExpr;
	if (Statement)
		delete Statement;
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
		Strm << ParamTypes[i]
			<< ParamFlags[i];

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
		case OPCARGS_VTableIndex_Byte:
		case OPCARGS_FieldOffset_Byte:
			Strm << Instructions[i].Member;
			Strm << STRM_INDEX(Instructions[i].Arg2);
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
//	VMethod::Define
//
//==========================================================================

bool VMethod::Define()
{
	bool Ret = true;

	Modifiers = TModifiers::Check(Modifiers, AllowedModifiers, Loc);
	Flags |= TModifiers::MethodAttr(Modifiers);

	if (Flags & FUNC_Static)
	{
		if (!(Flags & FUNC_Native))
		{
			ParseError(Loc, "Currently only native methods can be static");
			Ret = false;
		}
		if (!(Flags & FUNC_Final))
		{
			ParseError(Loc, "Currently static methods must be final.");
			Ret = false;
		}
	}

	if ((Flags & FUNC_VarArgs) && !(Flags & FUNC_Native))
	{
		ParseError(Loc, "Only native methods can have varargs");
	}

	VEmitContext ec(this);

	if (ReturnTypeExpr)
	{
		ReturnTypeExpr = ReturnTypeExpr->ResolveAsType(ec);
	}
	if (ReturnTypeExpr)
	{
		TType t = ReturnTypeExpr->Type;
		if (t.type != ev_void)
		{
			//	Function's return type must be void, vector or with size 4
			t.CheckPassable(ReturnTypeExpr->Loc);
		}
		ReturnType = t;
	}
	else
	{
		Ret = false;
	}

	//	Resolve parameters types.
	ParamsSize = 1;
	for (int i = 0; i < NumParams; i++)
	{
		VMethodParam& P = Params[i];

		if (P.TypeExpr)
		{
			P.TypeExpr = P.TypeExpr->ResolveAsType(ec);
		}
		if (!P.TypeExpr)
		{
			Ret = false;
			continue;
		}
		TType type = P.TypeExpr->Type;

		if (type.type == ev_void)
		{
			ParseError(P.TypeExpr->Loc, "Bad variable type");
			Ret = false;
			continue;
		}
		type.CheckPassable(P.TypeExpr->Loc);

		TModifiers::Check(P.Modifiers, AllowedParmModifiers, P.Loc);

		ParamTypes[i] = type;
		ParamFlags[i] = TModifiers::ParmAttr(P.Modifiers);
		if (ParamFlags[i] & FPARM_Optional && ParamFlags[i] & FPARM_Out)
		{
			ParseError(P.Loc, "Modifiers optional and out are mutually exclusive");
		}
		ParamsSize += type.GetSize() / 4;
		if (ParamFlags[i] & FPARM_Optional)
		{
			ParamsSize++;
		}
	}

	//	If this is a overriden method, verify that return type and argument
	// types match.
	VMethod* BaseMethod = NULL;
	if (Outer->MemberType == MEMBER_Class && Name != NAME_None &&
		((VClass*)Outer)->ParentClass)
	{
		BaseMethod = ((VClass*)Outer)->ParentClass->CheckForMethod(Name);
	}
	if (BaseMethod)
	{
		if (BaseMethod->Flags & FUNC_Final)
		{
			ParseError(Loc, "Method already has been declared as final and cannot be overriden.");
			Ret = false;
		}
		if (!BaseMethod->ReturnType.Equals(ReturnType))
		{
			ParseError(Loc, "Method redefined with different return type");
			Ret = false;
		}
		else if (BaseMethod->NumParams != NumParams)
		{
			ParseError(Loc, "Method redefined with different number of arguments");
			Ret = false;
		}
		else for (int i = 0; i < NumParams; i++)
			if (!BaseMethod->ParamTypes[i].Equals(ParamTypes[i]))
			{
				ParseError(Loc, "Type of argument %d differs from base class", i + 1);
				Ret = false;
			}
	}

	return Ret;
}

//==========================================================================
//
//	VMethod::Emit
//
//==========================================================================

void VMethod::Emit()
{
	if (Flags & FUNC_Native)
	{
		if (Statement)
		{
			ParseError(Loc, "Native methods can't have a body");
		}
		return;
	}

	if (Outer->MemberType == MEMBER_Field)
	{
		//	Delegate
		return;
	}

	if (!Statement)
	{
		ParseError(Loc, "Method body missing");
		return;
	}

	VEmitContext ec(this);

	ec.LocalDefs.Clear();
	ec.localsofs = 1;

	for (int i = 0; i < NumParams; i++)
	{
		VMethodParam& P = Params[i];
		if (P.Name != NAME_None)
		{
			if (ec.CheckForLocalVar(P.Name) != -1)
			{
				ParseError(P.Loc, "Redefined identifier %s", *P.Name);
			}
			VLocalVarDef& L = ec.LocalDefs.Alloc();
			L.Name = P.Name;
			L.type = ParamTypes[i];
			L.ofs = ec.localsofs;
			L.Visible = true;
			L.ParamFlags = ParamFlags[i];
		}
		ec.localsofs += ParamTypes[i].GetSize() / 4;
		if (ParamFlags[i] & FPARM_Optional)
		{
			if (P.Name != NAME_None)
			{
				VLocalVarDef& L = ec.LocalDefs.Alloc();
				L.Name = va("specified_%s", *P.Name);
				L.type = ev_int;
				L.ofs = ec.localsofs;
				L.Visible = true;
				L.ParamFlags = 0;
			}
			ec.localsofs++;
		}
	}

	for (int i = 0; i < ec.LocalDefs.Num(); i++)
	{
		if (ec.LocalDefs[i].type.type == ev_vector)
		{
			ec.AddStatement(OPC_VFixParam, ec.LocalDefs[i].ofs);
		}
	}

	if (!Statement->Resolve(ec))
	{
		return;
	}

	//  Call parent constructor
	if (this == ec.SelfClass->DefaultProperties && ec.SelfClass->ParentClass)
	{
		ec.AddStatement(OPC_LocalValue0);
		ec.AddStatement(OPC_Call, ec.SelfClass->ParentClass->DefaultProperties);
	}

	Statement->Emit(ec);

	if (ReturnType.type == ev_void)
	{
		ec.EmitClearStrings(0, ec.LocalDefs.Num());
		ec.AddStatement(OPC_Return);
	}
	NumLocals = ec.localsofs;
	ec.EndCode();
}

//==========================================================================
//
//	VMethod::DumpAsm
//
//	Disassembles a method.
//
//==========================================================================

void VMethod::DumpAsm()
{
	VMemberBase* PM = Outer;
	while (PM->MemberType != MEMBER_Package)
	{
		PM = PM->Outer;
	}
	VPackage* Package = (VPackage*)PM;

	dprintf("--------------------------------------------\n");
	dprintf("Dump ASM function %s.%s\n\n", *Outer->Name, *Name);
	if (Flags & FUNC_Native)
	{
		//	Builtin function
		dprintf("Builtin function.\n");
		return;
	}
	for (int s = 0; s < Instructions.Num(); s++)
	{
		//	Opcode
		int st = Instructions[s].Opcode;
		dprintf("%6d: %s", s, StatementInfo[st].name);
		switch (StatementInfo[st].Args)
		{
		case OPCARGS_None:
			break;
		case OPCARGS_Member:
			//	Name of the object
			dprintf(" %s.%s", *Instructions[s].Member->Outer->Name,
				*Instructions[s].Member->Name);
			break;
		case OPCARGS_BranchTarget:
			dprintf(" %6d", Instructions[s].Arg1);
			break;
		case OPCARGS_ByteBranchTarget:
		case OPCARGS_ShortBranchTarget:
		case OPCARGS_IntBranchTarget:
			dprintf(" %6d, %6d", Instructions[s].Arg1, Instructions[s].Arg2);
			break;
		case OPCARGS_Byte:
		case OPCARGS_Short:
		case OPCARGS_Int:
			dprintf(" %6d (%x)", Instructions[s].Arg1, Instructions[s].Arg1);
			break;
		case OPCARGS_Name:
			//  Name
			dprintf("\'%s\'", *Instructions[s].NameArg);
			break;
		case OPCARGS_String:
			//  String
			dprintf("\"%s\"", &Package->Strings[Instructions[s].Arg1]);
			break;
		case OPCARGS_FieldOffset:
			dprintf(" %s", *Instructions[s].Member->Name);
			break;
		case OPCARGS_VTableIndex:
			dprintf(" %s", *Instructions[s].Member->Name);
			break;
		case OPCARGS_VTableIndex_Byte:
		case OPCARGS_FieldOffset_Byte:
			dprintf(" %s %d", *Instructions[s].Member->Name, Instructions[s].Arg2);
			break;
		case OPCARGS_TypeSize:
			{
				char Tmp[256];
				Instructions[s].TypeArg.GetName(Tmp);
				dprintf(" %s", Tmp);
			}
			break;
		}
		dprintf("\n");
	}
}

//END

//BEGIN VConstant

//==========================================================================
//
//	VConstant::VConstant
//
//==========================================================================

VConstant::VConstant(VName InName, VMemberBase* InOuter, TLocation InLoc)
: VMemberBase(MEMBER_Const, InName, InOuter, InLoc)
, Type(ev_unknown)
, Value(0)
, ValueExpr(NULL)
, PrevEnumValue(NULL)
{
}

//==========================================================================
//
//	VConstant::~VConstant
//
//==========================================================================

VConstant::~VConstant()
{
	if (ValueExpr)
		delete ValueExpr;
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

//==========================================================================
//
//	VConstant::Define
//
//==========================================================================

bool VConstant::Define()
{
	if (PrevEnumValue)
	{
		Value = PrevEnumValue->Value + 1;
		return true;
	}

	if (ValueExpr)
	{
		VEmitContext ec(this);
		ValueExpr = ValueExpr->Resolve(ec);
	}
	if (!ValueExpr)
	{
		return false;
	}

	switch (Type)
	{
	case ev_int:
		if (!ValueExpr->IsIntConst())
		{
			ParseError(ValueExpr->Loc, "Integer constant expected");
			return false;
		}
		Value = ValueExpr->GetIntConst();
		break;

	case ev_float:
		if (!ValueExpr->IsFloatConst())
		{
			ParseError(ValueExpr->Loc, "Float constant expected");
			return false;
		}
		FloatValue = ValueExpr->GetFloatConst();
		break;

	default:
		ParseError(Loc, "Unsupported type of constant");
		return false;
	}
	return true;
}

//END

//BEGIN VStruct

//==========================================================================
//
//	VStruct::VStruct
//
//==========================================================================

VStruct::VStruct(VName InName, VMemberBase* InOuter, TLocation InLoc)
: VMemberBase(MEMBER_Struct, InName, InOuter, InLoc)
, ParentStruct(0)
, IsVector(false)
, StackSize(0)
, Fields(0)
, ParentStructName(NAME_None)
, Defined(true)
{
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
//	VStruct::AddField
//
//==========================================================================

void VStruct::AddField(VField* f)
{
	for (VField* Check = Fields; Check; Check = Check->Next)
	{
		if (f->Name == Check->Name)
		{
			ParseError(f->Loc, "Redeclared field");
			ParseError(Check->Loc, "Previous declaration here");
		}
	}

	if (!Fields)
		Fields = f;
	else
	{
		VField* Prev = Fields;
		while (Prev->Next)
			Prev = Prev->Next;
		Prev->Next = f;
	}
	f->Next = NULL;
}

//==========================================================================
//
//	VStruct::CheckForField
//
//==========================================================================

VField* VStruct::CheckForField(VName FieldName)
{
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (fi->Name == FieldName)
		{
			return fi;
		}
	}
	if (ParentStruct)
	{
		return ParentStruct->CheckForField(FieldName);
	}
	return NULL;
}

//==========================================================================
//
//	VStruct::NeedsDestructor
//
//==========================================================================

bool VStruct::NeedsDestructor() const
{
	for (VField* F = Fields; F; F = F->Next)
		if (F->NeedsDestructor())
			return true;
	if (ParentStruct)
		return ParentStruct->NeedsDestructor();
	return false;
}

//==========================================================================
//
//	VStruct::Define
//
//==========================================================================

bool VStruct::Define()
{
	if (ParentStructName != NAME_None)
	{
		TType type = CheckForType(Outer->MemberType == MEMBER_Class ?
			(VClass*)Outer : NULL, ParentStructName);
		if (type.type != ev_struct)
		{
			ParseError(ParentStructLoc, "%s is not a struct type",
				*ParentStructName);
		}
		else
		{
			ParentStruct = type.Struct;
		}
	}

	if (ParentStruct && !ParentStruct->Defined)
	{
		ParseError(ParentStructLoc, "Parent struct must be declared before");
		return false;
	}

	Defined = true;
	return true;
}

//==========================================================================
//
//	VStruct::DefineMembers
//
//==========================================================================

bool VStruct::DefineMembers()
{
	bool Ret = true;

	//	Define fields.
	vint32 size = 0;
	if (ParentStruct)
	{
		size = ParentStruct->StackSize * 4;
	}
	VField* PrevBool = NULL;
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (!fi->Define())
		{
			Ret = false;
		}
		if (fi->type.type == ev_bool && PrevBool && PrevBool->type.bit_mask != 0x80000000)
		{
			fi->type.bit_mask = PrevBool->type.bit_mask << 1;
		}
		else
		{
			size += fi->type.GetSize();
		}
		PrevBool = fi->type.type == ev_bool ? fi : NULL;
	}

	//	Validate vector type.
	if (IsVector)
	{
		int fc = 0;
		for (VField* f = Fields; f; f = f->Next)
		{
			if (f->type.type != ev_float)
			{
				ParseError(f->Loc, "Vector can have only float fields");
				Ret = false;
			}
			fc++;
		}
		if (fc != 3)
		{
			ParseError(Loc, "Vector must have exactly 3 float fields");
			Ret = false;
		}
	}

	StackSize = (size + 3) / 4;
	return Ret;
}

//END

//BEGIN VState

//==========================================================================
//
//	VState::VState
//
//==========================================================================

VState::VState(VName InName, VMemberBase* InOuter, TLocation InLoc)
: VMemberBase(MEMBER_State, InName, InOuter, InLoc)
, SpriteName(NAME_None)
, Frame(0)
, ModelName(NAME_None)
, ModelFrame(0)
, Time(0)
, NextState(0)
, Function(0)
, Next(0)
, FrameExpr(NULL)
, ModelFrameExpr(NULL)
, TimeExpr(NULL)
, NextStateName(NAME_None)
{
}

//==========================================================================
//
//	VState::~VState
//
//==========================================================================

VState::~VState()
{
	if (FrameExpr)
		delete FrameExpr;
	if (ModelFrameExpr)
		delete ModelFrameExpr;
	if (TimeExpr)
		delete TimeExpr;
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
//	VState::Define
//
//==========================================================================

bool VState::Define()
{
	bool Ret = true;

	if (!Function->Define())
	{
		Ret = false;
	}

	return Ret;
}

//==========================================================================
//
//	VState::Emit
//
//==========================================================================

void VState::Emit()
{
	VEmitContext ec(this);
	if (FrameExpr)
		FrameExpr = FrameExpr->Resolve(ec);
	if (ModelFrameExpr)
		ModelFrameExpr = ModelFrameExpr->Resolve(ec);
	if (TimeExpr)
		TimeExpr = TimeExpr->Resolve(ec);

	if (!FrameExpr || !TimeExpr)
		return;

	if (!FrameExpr->IsIntConst())
	{
		ParseError(FrameExpr->Loc, "Integer constant expected");
		return;
	}
	if (ModelFrameExpr && !ModelFrameExpr->IsIntConst())
	{
		ParseError(ModelFrameExpr->Loc, "Integer constant expected");
		return;
	}
	if (!TimeExpr->IsFloatConst())
	{
		ParseError(TimeExpr->Loc, "Float constant expected");
		return;
	}

	Frame = FrameExpr->GetIntConst();
	if (ModelFrameExpr)
		ModelFrame = ModelFrameExpr->GetIntConst();
	Time = TimeExpr->GetFloatConst();

	if (NextStateName != NAME_None)
	{
		NextState = ((VClass*)Outer)->CheckForState(NextStateName);
		if (!NextState)
		{
			ParseError(Loc, "No such state %s", *NextStateName);
		}
	}

	Function->Emit();
}

//END

//BEGIN VClass

//==========================================================================
//
//	VClass::VClass
//
//==========================================================================

VClass::VClass(VName InName, VMemberBase* InOuter, TLocation InLoc)
: VMemberBase(MEMBER_Class, InName, InOuter, InLoc)
, ParentClass(NULL)
, Fields(NULL)
, States(NULL)
, DefaultProperties(NULL)
, ParentClassName(NAME_None)
, Modifiers(0)
, Defined(true)
{
}

//==========================================================================
//
//	VClass::~VClass
//
//==========================================================================

VClass::~VClass()
{
	for (int i = 0; i < MobjInfoExpressions.Num(); i++)
		if (MobjInfoExpressions[i])
			delete MobjInfoExpressions[i];
	for (int i = 0; i < ScriptIdExpressions.Num(); i++)
		if (ScriptIdExpressions[i])
			delete ScriptIdExpressions[i];
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
//	VClass::AddConstant
//
//==========================================================================

void VClass::AddConstant(VConstant* c)
{
	Constants.Append(c);
}

//==========================================================================
//
//	VClass::AddField
//
//==========================================================================

void VClass::AddField(VField* f)
{
	if (!Fields)
		Fields = f;
	else
	{
		VField* Prev = Fields;
		while (Prev->Next)
			Prev = Prev->Next;
		Prev->Next = f;
	}
	f->Next = NULL;
}

//==========================================================================
//
//	VClass::AddState
//
//==========================================================================

void VClass::AddState(VState* s)
{
	if (!States)
		States = s;
	else
	{
		VState* Prev = States;
		while (Prev->Next)
			Prev = Prev->Next;
		Prev->Next = s;
	}
	s->Next = NULL;
}

//==========================================================================
//
//	VClass::AddMethod
//
//==========================================================================

void VClass::AddMethod(VMethod* m)
{
	Methods.Append(m);
}

//==========================================================================
//
//	CheckForFunction
//
//==========================================================================

VMethod* VClass::CheckForFunction(VName Name)
{
	if (Name == NAME_None)
	{
		return NULL;
	}
	return (VMethod*)StaticFindMember(Name, this, MEMBER_Method);
}

//==========================================================================
//
//	VClass::CheckForMethod
//
//==========================================================================

VMethod* VClass::CheckForMethod(VName Name)
{
	if (Name == NAME_None)
	{
		return NULL;
	}
	VMethod* M = (VMethod*)StaticFindMember(Name, this, MEMBER_Method);
	if (M)
	{
		return M;
	}
	if (ParentClass)
	{
		return ParentClass->CheckForMethod(Name);
	}
	return NULL;
}

//==========================================================================
//
//	VClass::CheckForConstant
//
//==========================================================================

VConstant* VClass::CheckForConstant(VName Name)
{
	VMemberBase* m = StaticFindMember(Name, this, MEMBER_Const);
	if (m)
	{
		return (VConstant*)m;
	}
	if (ParentClass)
	{
		return ParentClass->CheckForConstant(Name);
	}
	return NULL;
}

//==========================================================================
//
//	VClass::CheckForField
//
//==========================================================================

VField* VClass::CheckForField(TLocation l, VName Name, VClass* SelfClass, bool CheckPrivate)
{
	if (Name == NAME_None)
	{
		return NULL;
	}
	for (VField *fi = Fields; fi; fi = fi->Next)
	{
		if (Name == fi->Name)
		{
			if (CheckPrivate && fi->flags & FIELD_Private &&
				this != SelfClass)
			{
				ParseError(l, "Field %s is private", *fi->Name);
			}
			return fi;
		}
	}
	if (ParentClass)
	{
		return ParentClass->CheckForField(l, Name, SelfClass, CheckPrivate);
	}
	return NULL;
}

//==========================================================================
//
//	VClass::CheckForState
//
//==========================================================================

VState* VClass::CheckForState(VName StateName)
{
	VMemberBase* m = StaticFindMember(StateName, this, MEMBER_State);
	if (m)
	{
		return (VState*)m;
	}
	if (ParentClass)
	{
		return ParentClass->CheckForState(StateName);
	}
	return NULL;
}

//==========================================================================
//
//	VClass::Define
//
//==========================================================================

bool VClass::Define()
{
	Modifiers = TModifiers::Check(Modifiers, AllowedModifiers, Loc);
	int ClassAttr = TModifiers::ClassAttr(Modifiers);

	if (ParentClassName != NAME_None)
	{
		ParentClass = CheckForClass(ParentClassName);
		if (!ParentClass)
		{
			ParseError(ParentClassLoc, "No such class %s", *ParentClassName);
		}
		else if (!ParentClass->Defined)
		{
			ParseError(ParentClassLoc, "Parent class must be defined before");
		}
	}

	for (int i = 0; i < Structs.Num(); i++)
	{
		if (!Structs[i]->Define())
		{
			return false;
		}
	}

	for (int i = 0; i < MobjInfoExpressions.Num(); i++)
	{
		VEmitContext ec(this);
		MobjInfoExpressions[i] = MobjInfoExpressions[i]->Resolve(ec);
		if (!MobjInfoExpressions[i])
		{
			return false;
		}
		if (!MobjInfoExpressions[i]->IsIntConst())
		{
			ParseError(MobjInfoExpressions[i]->Loc, "Integer constant expected");
			return false;
		}
		mobjinfo_t& mi = ec.Package->mobj_info.Alloc();
		mi.doomednum = MobjInfoExpressions[i]->GetIntConst();
		mi.class_id = this;
	}

	for (int i = 0; i < ScriptIdExpressions.Num(); i++)
	{
		VEmitContext ec(this);
		ScriptIdExpressions[i] = ScriptIdExpressions[i]->Resolve(ec);
		if (!ScriptIdExpressions[i])
		{
			return false;
		}
		if (!ScriptIdExpressions[i]->IsIntConst())
		{
			ParseError(ScriptIdExpressions[i]->Loc, "Integer constant expected");
			return false;
		}
		mobjinfo_t& mi = ec.Package->script_ids.Alloc();
		mi.doomednum = ScriptIdExpressions[i]->GetIntConst();
		mi.class_id = this;
	}

	Defined = true;
	return true;
}

//==========================================================================
//
//	VClass::DefineMembers
//
//==========================================================================

bool VClass::DefineMembers()
{
	bool Ret = true;

	for (int i = 0; i < Constants.Num(); i++)
	{
		if (!Constants[i]->Define())
		{
			Ret = false;
		}
	}

	for (int i = 0; i < Structs.Num(); i++)
	{
		Structs[i]->DefineMembers();
	}

	VField* PrevBool = NULL;
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (!fi->Define())
		{
			Ret = false;
		}
		if (fi->type.type == ev_bool && PrevBool && PrevBool->type.bit_mask != 0x80000000)
		{
			fi->type.bit_mask = PrevBool->type.bit_mask << 1;
		}
		PrevBool = fi->type.type == ev_bool ? fi : NULL;
	}

	for (int i = 0; i < Methods.Num(); i++)
	{
		if (!Methods[i]->Define())
		{
			Ret = false;
		}
	}

	if (!DefaultProperties->Define())
	{
		Ret = false;
	}

	for (VState* s = States; s; s = s->Next)
	{
		if (!s->Define())
		{
			Ret = false;
		}
	}

	return Ret;
}

//==========================================================================
//
//	VClass::Emit
//
//==========================================================================

void VClass::Emit()
{
	//	Emit method code.
	for (int i = 0; i < Methods.Num(); i++)
	{
		Methods[i]->Emit();
	}

	//	Emit code of the state methods.
	for (VState* s = States; s; s = s->Next)
	{
		s->Emit();
	}

	DefaultProperties->Emit();
}

//END

//BEGIN VPackage

//==========================================================================
//
//	VPackage::VPackage
//
//==========================================================================

VPackage::VPackage()
: VMemberBase(MEMBER_Package, NAME_None, NULL, TLocation())
{
	//	Strings
	memset(StringLookup, 0, 256 * 4);
	//	1-st string is empty
	StringInfo.Alloc();
	StringInfo[0].offs = 0;
	StringInfo[0].next = 0;
	Strings.SetNum(4);
	memset(Strings.Ptr(), 0, 4);
}

//==========================================================================
//
//	VPackage::VPackage
//
//==========================================================================

VPackage::VPackage(VName InName)
: VMemberBase(MEMBER_Package, InName, NULL, TLocation())
{
	//	Strings
	memset(StringLookup, 0, 256 * 4);
	//	1-st string is empty
	StringInfo.Alloc();
	StringInfo[0].offs = 0;
	StringInfo[0].next = 0;
	Strings.SetNum(4);
	memset(Strings.Ptr(), 0, 4);
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
	if (!*str)
	{
		return 0;
	}
	int hash = StringHashFunc(str);
	for (int i = StringLookup[hash]; i; i = StringInfo[i].next)
	{
		if (!strcmp(&Strings[StringInfo[i].offs], str))
		{
			return StringInfo[i].offs;
		}
	}

	//  Add new string
	TStringInfo& SI = StringInfo.Alloc();
	int AddLen = (strlen(str) + 4) & ~3;
	int Ofs = Strings.Num();
	Strings.SetNum(Strings.Num() + AddLen);
	memset(&Strings[Ofs], 0, AddLen);
	SI.offs = Ofs;
	SI.next = StringLookup[hash];
	StringLookup[hash] = StringInfo.Num() - 1;
	strcpy(&Strings[Ofs], str);
	return SI.offs;
}

//==========================================================================
//
//	VPackage::CheckForConstant
//
//==========================================================================

VConstant* VPackage::CheckForConstant(VName Name)
{
	VMemberBase* m = StaticFindMember(Name, this, MEMBER_Const);
	if (m)
	{
		return (VConstant*)m;
	}
	return NULL;
}

//==========================================================================
//
//	VPackage::Emit
//
//==========================================================================

void VPackage::Emit()
{
	for (int i = 0; i < PackagesToLoad.Num(); i++)
	{
		PackagesToLoad[i].Pkg = LoadPackage(PackagesToLoad[i].Name,
			PackagesToLoad[i].Loc);
	}

	if (NumErrors)
	{
		BailOut();
	}

	for (int i = 0; i < ParsedConstants.Num(); i++)
	{
		ParsedConstants[i]->Define();
	}

	for (int i = 0; i < ParsedStructs.Num(); i++)
	{
		ParsedStructs[i]->Define();
	}

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->Define();
	}

	if (NumErrors)
	{
		BailOut();
	}

	for (int i = 0; i < ParsedStructs.Num(); i++)
	{
		ParsedStructs[i]->DefineMembers();
	}

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->DefineMembers();
	}

	if (NumErrors)
	{
		BailOut();
	}

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->Emit();
	}

	if (NumErrors)
	{
		BailOut();
	}
}

//==========================================================================
//
//	VPackage::WriteObject
//
//==========================================================================

void VPackage::WriteObject(const char *name)
{
	FILE*			f;
	int				i;
	dprograms_t		progs;

	dprintf("Writing object\n");

	f = fopen(name, "wb");
	if (!f)
	{
		FatalError("Can't open file \"%s\".", name);
	}

	VProgsWriter Writer(f);

	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->IsIn(this))
			Writer.AddExport(VMemberBase::GMembers[i]);
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
	dprintf("Strings    %6d %6d\n", StringInfo.Num(), Strings.Num());
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

//END
