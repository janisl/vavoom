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

#include "gamedefs.h"
#include "progdefs.h"

bool					VMemberBase::GObjInitialized;
VClass*					VMemberBase::GClasses;
TArray<VMemberBase*>	VMemberBase::GMembers;
TArray<VPackage*>		VMemberBase::GLoadedPackages;

TArray<mobjinfo_t>		VClass::GMobjInfos;
TArray<mobjinfo_t>		VClass::GScriptIds;
TArray<VName>			VClass::GSpriteNames;
TArray<VName>			VClass::GModelNames;

#define DECLARE_OPC(name, argcount)		argcount
#define OPCODE_INFO
static int OpcodeArgCount[NUM_OPCODES] =
{
#include "progdefs.h"
};

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
		Z_Free(NameRemap);
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

//==========================================================================
//
//	VMemberBase::VMemberBase
//
//==========================================================================

VMemberBase::VMemberBase(vuint8 InMemberType, VName AName)
: MemberType(InMemberType)
, Outer(0)
, Name(AName)
{
	if (GObjInitialized)
	{
		GMembers.AddItem(this);
	}
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
//	VMemberBase::Serialise
//
//==========================================================================

void VMemberBase::Serialise(VStream& Strm)
{
	Strm << Outer;
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
//	VMemberBase::StaticInit
//
//==========================================================================

void VMemberBase::StaticInit()
{
	guard(VMemberBase::StaticInit);
	VClass::GModelNames.AddItem(NAME_None);
	for (VClass* C = GClasses; C; C = C->LinkNext)
		GMembers.AddItem(C);
	GObjInitialized = true;
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
	}
	GMembers.Empty();
	GLoadedPackages.Empty();
	VClass::GMobjInfos.Empty();
	VClass::GScriptIds.Empty();
	VClass::GSpriteNames.Empty();
	VClass::GModelNames.Empty();
	GObjInitialized = false;
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

	if (fl_devmode && FL_FindFile(va("progs/%s.dat", *InName)))
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
		crc + Streamer<byte>(*Reader);
	}

	// byte swap the header
	Reader->Seek(0);
	Reader->Serialise(Progs.magic, 4);
	for (i = 1; i < (int)sizeof(Progs) / 4; i++)
	{
		*Reader << ((int*)&Progs)[i];
	}

	if (strncmp(Progs.magic, PROG_MAGIC, 4))
		Sys_Error("Progs has wrong file ID, possibly older version");
	if (Progs.version != PROG_VERSION)
		Sys_Error("Progs has wrong version number (%i should be %i)",
			Progs.version, PROG_VERSION);

	// Read names
	NameRemap = Z_New(VName, Progs.num_names, PU_STATIC, 0);
	Reader->Seek(Progs.ofs_names);
	for (i = 0; i < Progs.num_names; i++)
	{
		VNameEntry E;
		*Reader << E;
		NameRemap[i] = E.Name;
	}
	Reader->NameRemap = NameRemap;

	Reader->Imports = Z_CNew(VProgsImport, Progs.num_imports, PU_STATIC, 0);
	Reader->NumImports = Progs.num_imports;
	Reader->Seek(Progs.ofs_imports);
	for (i = 0; i < Progs.num_imports; i++)
	{
		*Reader << Reader->Imports[i];
	}
	Reader->ResolveImports();

	VProgsExport* Exports = Z_CNew(VProgsExport, Progs.num_exports, PU_STATIC, 0);
	Reader->Exports = Exports;
	Reader->NumExports = Progs.num_exports;

	VPackage* Pkg = new VPackage(InName);
	GLoadedPackages.AddItem(Pkg);
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
	Pkg->Strings = Z_CNew(char, Progs.num_strings, PU_STATIC, 0);
	Reader->Seek(Progs.ofs_strings);
	Reader->Serialise(Pkg->Strings, Progs.num_strings);

	Pkg->Statements = Z_CNew(int, Progs.num_statements, PU_STATIC, 0);
	Pkg->VTables = Z_CNew(VMethod*, Progs.num_vtables, PU_STATIC, 0);

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
		mobjinfo_t mi;
		*Reader << mi;
		VClass::GMobjInfos.AddItem(mi);
	}
	Reader->Seek(Progs.ofs_scriptids);
	for (i = 0; i < Progs.num_scriptids; i++)
	{
		mobjinfo_t si;
		*Reader << si;
		VClass::GScriptIds.AddItem(si);
	}

	//	Set up function pointers in vitual tables
	Reader->Seek(Progs.ofs_vtables);
	for (i = 0; i < Progs.num_vtables; i++)
	{
		*(VStream*)Reader << Pkg->VTables[i];
	}

	//
	//	Read statements and patch code
	//
	Reader->Seek(Progs.ofs_statements);
	for (i = 0; i < Progs.num_statements; i++)
	{
		vuint8 Tmp;
		*Reader << Tmp;
		Pkg->Statements[i] = Tmp;
		if (OpcodeArgCount[Pkg->Statements[i]] >= 1)
		{
			switch (Pkg->Statements[i])
			{
			case OPC_PushString:
				*Reader << Pkg->Statements[i + 1];
				Pkg->Statements[i + 1] += (int)Pkg->Strings;
				break;
			case OPC_PushName:
			case OPC_CaseGotoName:
				*Reader << *(VName*)&Pkg->Statements[i + 1];
				break;
			case OPC_PushFunction:
			case OPC_Call:
			case OPC_PushClassId:
			case OPC_DynamicCast:
			case OPC_CaseGotoClassId:
			case OPC_PushState:
				*Reader << *(VMemberBase**)&Pkg->Statements[i + 1];
				break;
			case OPC_Goto:
			case OPC_IfGoto:
			case OPC_IfNotGoto:
			case OPC_IfTopGoto:
			case OPC_IfNotTopGoto:
				*Reader << Pkg->Statements[i + 1];
				Pkg->Statements[i + 1] = (int)(Pkg->Statements + Pkg->Statements[i + 1]);
				break;
			default:
				*Reader << Pkg->Statements[i + 1];
				break;
			}
		}
		if (OpcodeArgCount[Pkg->Statements[i]] >= 2)
		{
			*Reader << Pkg->Statements[i + 2];
			switch (Pkg->Statements[i])
			{
			case OPC_CaseGoto:
			case OPC_CaseGotoClassId:
			case OPC_CaseGotoName:
				Pkg->Statements[i + 2] = (int)(Pkg->Statements + Pkg->Statements[i + 2]);
				break;
			}
		}
		i += OpcodeArgCount[Pkg->Statements[i]];
	}

	for (i = 0; i < Progs.num_exports; i++)
	{
		Exports[i].Obj->PostLoad();
	}

	Z_Free(NameRemap);
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
//	VPackage::VPackage
//
//==========================================================================

VPackage::VPackage(VName AName)
: VMemberBase(MEMBER_Package, AName)
, Checksum(0)
, Strings(0)
, Statements(0)
, VTables(0)
, Reader(0)
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
//	VField::VField
//
//==========================================================================

VField::VField(VName AName)
: VMemberBase(MEMBER_Field, AName)
, Next(0)
, NextReference(0)
, Ofs(0)
, Flags(0)
{
	memset(&Type, 0, sizeof(Type));
}

//==========================================================================
//
//	VField::Serialise
//
//==========================================================================

void VField::Serialise(VStream& Strm)
{
	guard(VField::Serialise);
	VMemberBase::Serialise(Strm);
	VMethod* func;
	Strm << Next
		<< STRM_INDEX(Ofs)
		<< Type
		<< func
		<< STRM_INDEX(Flags);
	unguard;
}

//==========================================================================
//
//	operator VStream << FType
//
//==========================================================================

VStream& operator<<(VStream& Strm, VField::FType& T)
{
	guard(operator VStream << FType);
	Strm << T.Type;
	byte RealType = T.Type;
	if (RealType == ev_array)
	{
		Strm << T.ArrayInnerType
			<< STRM_INDEX(T.ArrayDim);
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
		Strm << T.BitMask;
	return Strm;
	unguard;
}

//==========================================================================
//
//	VField::SerialiseFieldValue
//
//==========================================================================

void VField::SerialiseFieldValue(VStream& Strm, byte* Data, const VField::FType& Type)
{
	guard(SerialiseFieldValue);
	VField::FType IntType;
	int InnerSize;
	switch (Type.Type)
	{
	case ev_int:
		Strm << *(int*)Data;
		break;

	case ev_float:
		Strm << *(float*)Data;
		break;

	case ev_bool:
		if (Type.BitMask == 1)
			Strm << *(int*)Data;
		break;

	case ev_vector:
		Strm << *(TVec*)Data;
		break;

	case ev_name:
		Strm << *(VName*)Data;
		break;

	case ev_string:
		if (Strm.IsLoading())
		{
			int TmpIdx;
			Strm << TmpIdx;
			if (TmpIdx)
			{
				*(int*)Data = (int)svpr.StrAtOffs(TmpIdx);
			}
			else
			{
				*(int*)Data = 0;
			}
		}
		else
		{
			int TmpIdx = 0;
			if (*(int*)Data)
			{
				TmpIdx = svpr.GetStringOffs(*(char**)Data);
			}
			Strm << TmpIdx;
		}
		break;

	case ev_pointer:
		if (Type.InnerType == ev_struct)
			Strm.SerialiseStructPointer(*(void**)Data, Type.Struct);
		else
		{
			dprintf("Don't know how to serialise pointer type %d\n", Type.InnerType);
			Strm << *(int*)Data;
		}
		break;

	case ev_reference:
		Strm.SerialiseReference(*(VObject**)Data, Type.Class);
		break;

	case ev_classid:
		if (Strm.IsLoading())
		{
			VName CName;
			Strm << CName;
			if (CName != NAME_None)
			{
				*(VClass**)Data = VClass::FindClass(*CName);
			}
			else
			{
				*(VClass**)Data = NULL;
			}
		}
		else
		{
			VName CName = NAME_None;
			if (*(VClass**)Data)
			{
				CName = (*(VClass**)Data)->GetVName();
			}
			Strm << CName;
		}
		break;

	case ev_state:
		if (Strm.IsLoading())
		{
			VName CName;
			VName SName;
			Strm << CName << SName;
			if (SName != NAME_None)
			{
				*(VState**)Data = VClass::FindClass(*CName)->FindStateChecked(SName);
			}
			else
			{
				*(VState**)Data = NULL;
			}
		}
		else
		{
			VName CName = NAME_None;
			VName SName = NAME_None;
			if (*(VState**)Data)
			{
				CName = (*(VState**)Data)->Outer->GetVName();
				SName = (*(VState**)Data)->Name;
			}
			Strm << CName << SName;
		}
		break;

	case ev_delegate:
		Strm.SerialiseReference(*(VObject**)Data, Type.Class);
		if (Strm.IsLoading())
		{
			VName FuncName;
			Strm << FuncName;
			if (*(VObject**)Data)
				((VMethod**)Data)[1] = (*(VObject**)Data)->GetVFunction(FuncName);
		}
		else
		{
			VName FuncName = NAME_None;
			if (*(VObject**)Data)
				FuncName = ((VMethod**)Data)[1]->Name;
			Strm << FuncName;
		}
		break;

	case ev_struct:
		Type.Struct->SerialiseObject(Strm, Data);
		break;

	case ev_array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.Type == ev_vector ? 12 : IntType.Type == ev_struct ? IntType.Struct->Size : 4;
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			SerialiseFieldValue(Strm, Data + i * InnerSize, IntType);
		}
		break;
	}
	unguard;
}

//==========================================================================
//
//	VField::CleanField
//
//==========================================================================

void VField::CleanField(byte* Data, const VField::FType& Type)
{
	guard(CleanField);
	VField::FType IntType;
	int InnerSize;
	switch (Type.Type)
	{
	case ev_reference:
		if (*(VObject**)Data && (*(VObject**)Data)->GetFlags() & _OF_CleanupRef)
		{
			*(VObject**)Data = NULL;
		}
		break;

	case ev_delegate:
		if (*(VObject**)Data && (*(VObject**)Data)->GetFlags() & _OF_CleanupRef)
		{
			*(VObject**)Data = NULL;
			((VMethod**)Data)[1] = NULL;
		}
		break;

	case ev_struct:
		Type.Struct->CleanObject(Data);
		break;

	case ev_array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.Type == ev_struct ? IntType.Struct->Size : 4;
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			CleanField(Data + i * InnerSize, IntType);
		}
		break;
	}
	unguard;
}

//==========================================================================
//
//	VMethod::VMethod
//
//==========================================================================

VMethod::VMethod(VName AName)
: VMemberBase(MEMBER_Method, AName)
, FirstStatement(0)
, NumParms(0)
, NumLocals(0)
, Type(0)
, Flags(0)
, Profile1(0)
, Profile2(0)
{
}

//==========================================================================
//
//  PF_Fixme
//
//==========================================================================

static void PF_Fixme()
{
	Sys_Error("unimplemented bulitin");
}

//==========================================================================
//
//	VMethod::Serialise
//
//==========================================================================

void VMethod::Serialise(VStream& Strm)
{
	guard(VMethod::Serialise);
	VMemberBase::Serialise(Strm);
	VField::FType TmpType;
	vint32 TmpNumParams;
	vint32 TmpNumLocals;
	vint32 TmpFlags;
	vint32 ParamsSize;
	Strm << STRM_INDEX(FirstStatement)
		<< STRM_INDEX(TmpNumLocals)
		<< STRM_INDEX(TmpFlags)
		<< TmpType
		<< STRM_INDEX(TmpNumParams)
		<< STRM_INDEX(ParamsSize);
	Type = TmpType.Type;
	NumParms = ParamsSize;
	NumLocals = TmpNumLocals;
	Flags = TmpFlags;
	for (int i = 0; i < TmpNumParams; i++)
		Strm << TmpType;

	//	Set up builtins
	if (NumParms > 16)
		Sys_Error("Function has more than 16 params");
	for (FBuiltinInfo* B = FBuiltinInfo::Builtins; B; B = B->Next)
	{
		if (Outer == B->OuterClass && !strcmp(*Name, B->Name))
		{
			if (Flags & FUNC_Native)
			{
				FirstStatement = (int)B->Func;
				break;
			}
			else
			{
				Sys_Error("PR_LoadProgs: Builtin %s redefined", B->Name);
			}
		}
	}
	if (!FirstStatement && Flags & FUNC_Native)
	{
		//	Default builtin
		FirstStatement = (int)PF_Fixme;
#if defined CLIENT && defined SERVER
		//	Don't abort with error, because it will be done, when this
		// function will be called (if it will be called).
		GCon->Logf(NAME_Dev, "WARNING: Builtin %s not found!",
			*GetFullName());
#endif
	}
	unguard;
}

//==========================================================================
//
//	VMethod::PostLoad
//
//==========================================================================

void VMethod::PostLoad()
{
	if (!(Flags & FUNC_Native))
	{
		FirstStatement = (int)(GetPackage()->Statements + FirstStatement);
	}
}

//==========================================================================
//
//	VState::VState
//
//==========================================================================

VState::VState(VName AName)
: VMemberBase(MEMBER_State, AName)
, SpriteName(NAME_None)
, SpriteIndex(0)
, frame(0)
, ModelName(NAME_None)
, ModelIndex(0)
, model_frame(0)
, time(0)
, nextstate(0)
, function(0)
, Next(0)
{
}

//==========================================================================
//
//	VState::Serialise
//
//==========================================================================

void VState::Serialise(VStream& Strm)
{
	guard(VState::Serialise);
	VMemberBase::Serialise(Strm);
	Strm << SpriteName
		<< STRM_INDEX(frame)
		<< ModelName
		<< STRM_INDEX(model_frame)
		<< time
		<< nextstate
		<< function
		<< Next;
	if (Strm.IsLoading())
	{
		SpriteIndex = VClass::FindSprite(SpriteName);
		ModelIndex = VClass::FindModel(ModelName);
	}
	unguard;
}

//==========================================================================
//
//	VState::IsInRange
//
//==========================================================================

bool VState::IsInRange(VState* Start, VState* End, int MaxDepth)
{
	guard(VState::IsInRange);
	int Depth = 0;
	VState* check = Start;
	do
	{
		if (check == this)
			return true;
		if (check)
			check = check->Next;
		Depth++;
	}
	while (Depth < MaxDepth && check != End);
	return false;
	unguard;
}

//==========================================================================
//
//	VConstant::VConstant
//
//==========================================================================

VConstant::VConstant(VName AName)
: VMemberBase(MEMBER_Const, AName)
, Type(0)
, Value(0)
{
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
		Strm << *(float*)&Value;
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
//	VStruct::VStruct
//
//==========================================================================

VStruct::VStruct(VName AName)
: VMemberBase(MEMBER_Struct, AName)
, ObjectFlags(0)
, ParentStruct(0)
, Size(0)
, Fields(0)
, ReferenceFields(0)
{
}

//==========================================================================
//
//	VStruct::Serialise
//
//==========================================================================

void VStruct::Serialise(VStream& Strm)
{
	guard(VStruct::Serialise);
	VMemberBase::Serialise(Strm);
	vuint8 IsVector;
	vint32 AvailableSize;
	vint32 AvailableOfs;
	Strm << ParentStruct
		<< IsVector
		<< STRM_INDEX(Size)
		<< Fields
		<< STRM_INDEX(AvailableSize)
		<< STRM_INDEX(AvailableOfs);
	unguard;
}

//==========================================================================
//
//	VStruct::PostLoad
//
//==========================================================================

void VStruct::PostLoad()
{
	InitReferences();
}

//==========================================================================
//
//	VStruct::InitReferences
//
//==========================================================================

void VStruct::InitReferences()
{
	guard(VStruct::InitReferences);
	if (ObjectFlags & CLASSOF_RefsInitialised)
	{
		return;
	}

	ReferenceFields = NULL;
	if (ParentStruct)
	{
		ParentStruct->InitReferences();
		ReferenceFields = ParentStruct->ReferenceFields;
	}

	for (VField* F = Fields; F; F = F->Next)
	{
		switch (F->Type.Type)
		{
		case ev_reference:
		case ev_delegate:
			F->NextReference = ReferenceFields;
			ReferenceFields = F;
			break;
		
		case ev_struct:
			F->Type.Struct->InitReferences();
			if (F->Type.Struct->ReferenceFields)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			break;
		
		case ev_array:
			if (F->Type.ArrayInnerType == ev_reference)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			else if (F->Type.ArrayInnerType == ev_struct)
			{
				F->Type.Struct->InitReferences();
				if (F->Type.Struct->ReferenceFields)
				{
					F->NextReference = ReferenceFields;
					ReferenceFields = F;
				}
			}
			break;
		}
	}

	ObjectFlags |= CLASSOF_RefsInitialised;
	unguard;
}

//==========================================================================
//
//	VStruct::SerialiseObject
//
//==========================================================================

void VStruct::SerialiseObject(VStream& Strm, byte* Data)
{
	guard(VStruct::SerialiseObject);
	//	Serialise parent struct's fields.
	if (ParentStruct)
	{
		ParentStruct->SerialiseObject(Strm, Data);
	}
	//	Serialise fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		//	Skip native and transient fields.
		if (F->Flags & (FIELD_Native | FIELD_Transient))
		{
			continue;
		}
		VField::SerialiseFieldValue(Strm, Data + F->Ofs, F->Type);
	}
	unguardf(("(%s)", *Name));
}

//==========================================================================
//
//	VStruct::CleanObject
//
//==========================================================================

void VStruct::CleanObject(byte* Data)
{
	guard(VStruct::CleanObject);
	for (VField* F = ReferenceFields; F; F = F->NextReference)
	{
		VField::CleanField(Data + F->Ofs, F->Type);
	}
	unguardf(("(%s)", *Name));
}

//==========================================================================
//
//	operator VStream << mobjinfo_t
//
//==========================================================================

VStream& operator<<(VStream& Strm, mobjinfo_t& MI)
{
	return Strm << STRM_INDEX(MI.doomednum)
		<< MI.class_id;
}

//==========================================================================
//
//	VClass::VClass
//
//==========================================================================

VClass::VClass(VName AName)
: VMemberBase(MEMBER_Class, AName)
, ObjectFlags(0)
, LinkNext(0)
, ParentClass(0)
, ClassSize(0)
, ClassFlags(0)
, ClassVTable(0)
, ClassConstructor(0)
, ClassNumMethods(0)
, VTableOffset(0)
, Fields(0)
, ReferenceFields(0)
, States(0)
{
	guard(VClass::VClass);
	LinkNext = GClasses;
	GClasses = this;
	unguard;
}

//==========================================================================
//
//	VClass::VClass
//
//==========================================================================

VClass::VClass(ENativeConstructor, size_t ASize, dword AClassFlags, 
	VClass *AParent, EName AName, void(*ACtor)())
: VMemberBase(MEMBER_Class, AName)
, ObjectFlags(CLASSOF_Native)
, LinkNext(0)
, ParentClass(AParent)
, ClassSize(ASize)
, ClassFlags(AClassFlags)
, ClassVTable(0)
, ClassConstructor(ACtor)
, ClassNumMethods(0)
, VTableOffset(0)
, Fields(0)
, ReferenceFields(0)
, States(0)
{
	guard(native VClass::VClass);
	LinkNext = GClasses;
	GClasses = this;
	unguard;
}

//==========================================================================
//
//	VClass::~VClass
//
//==========================================================================

VClass::~VClass()
{
	guard(VClass::~VClass);
	if (!GObjInitialized)
	{
		return;
	}
	if (GClasses == this)
	{
		GClasses = LinkNext;
	}
	else
	{
		VClass* Prev = GClasses;
		while (Prev && Prev->LinkNext != this)
		{
			Prev = Prev->LinkNext;
		}
		if (Prev)
		{
			Prev->LinkNext = LinkNext;
		}
		else
		{
			GCon->Log(NAME_Dev, "VClass Unlink: Class not in list");
		}
	}
	unguard;
}

//==========================================================================
//
//	VClass::Serialise
//
//==========================================================================

void VClass::Serialise(VStream& Strm)
{
	guard(VClass::Serialise);
	VMemberBase::Serialise(Strm);
	int PrevSize = ClassSize;
	VClass* PrevParent = ParentClass;
	Strm << ParentClass
		<< Fields
		<< States
		<< STRM_INDEX(VTableOffset)
		<< STRM_INDEX(ClassNumMethods)
		<< STRM_INDEX(ClassSize);
	if ((ObjectFlags & CLASSOF_Native) && ClassSize != PrevSize)
	{
		Sys_Error("Bad class size, class %s, C++ %d, VavoomC %d)",
			GetName(), PrevSize, ClassSize);
	}
	if ((ObjectFlags & CLASSOF_Native) && ParentClass != PrevParent)
	{
		Sys_Error("Bad parent class, class %s, C++ %s, VavoomC %s)",
			GetName(), PrevParent ? PrevParent->GetName() : "(none)",
			ParentClass ? ParentClass->GetName() : "(none)");
	}
	unguard;
}

//==========================================================================
//
//	VClass::FindClass
//
//==========================================================================

VClass *VClass::FindClass(const char *AName)
{
	VName TempName(AName, VName::Find);
	if (TempName == NAME_None)
	{
		// No such name, no chance to find a class
		return NULL;
	}
	for (VClass* Cls = GClasses; Cls; Cls = Cls->LinkNext)
	{
		if (Cls->GetVName() == TempName)
		{
			return Cls;
		}
	}
	return NULL;
}

//==========================================================================
//
//	VClass::FindSprite
//
//==========================================================================

int VClass::FindSprite(VName Name)
{
	guard(VClass::FindSprite);
	for (int i = 0; i < GSpriteNames.Num(); i++)
		if (GSpriteNames[i] == Name)
			return i;
	return GSpriteNames.AddItem(Name);
	unguard;
}

//==========================================================================
//
//	VClass::FindModel
//
//==========================================================================

int VClass::FindModel(VName Name)
{
	guard(VClass::FindModel);
	for (int i = 0; i < GModelNames.Num(); i++)
		if (GModelNames[i] == Name)
			return i;
	return GModelNames.AddItem(Name);
	unguard;
}

//==========================================================================
//
//	VClass::FindFunction
//
//==========================================================================

VMethod *VClass::FindFunction(VName AName)
{
	guard(VClass::FindFunction);
	for (int i = 0; i < ClassNumMethods; i++)
	{
		if (ClassVTable[i]->Name == AName)
		{
			return ClassVTable[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindFunctionChecked
//
//==========================================================================

VMethod *VClass::FindFunctionChecked(VName AName)
{
	guard(VClass::FindFunctionChecked);
	VMethod *func = FindFunction(AName);
	if (!func)
	{
		Sys_Error("Function %s not found", *AName);
	}
	return func;
	unguard;
}

//==========================================================================
//
//	VClass::GetFunctionIndex
//
//==========================================================================

int VClass::GetFunctionIndex(VName AName)
{
	guard(VClass::GetFunctionIndex);
	for (int i = 0; i < ClassNumMethods; i++)
	{
		if (ClassVTable[i]->Name == AName)
		{
			return i;
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VClass::FindState
//
//==========================================================================

VState* VClass::FindState(VName AName)
{
	guard(VClass::FindState);
	for (VState* s = States; s; s = s->Next)
	{
		if (s->Name == AName)
		{
			return s;
		}
	}
	if (ParentClass)
	{
		return ParentClass->FindState(AName);
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindStateChecked
//
//==========================================================================

VState* VClass::FindStateChecked(VName AName)
{
	guard(VClass::FindStateChecked);
	VState* s = FindState(AName);
	if (!s)
	{
		Sys_Error("State %s not found", *AName);
	}
	return s;
	unguard;
}

//==========================================================================
//
//	VClass::PostLoad
//
//==========================================================================

void VClass::PostLoad()
{
	if (!ClassVTable)
	{
		ClassVTable = GetPackage()->VTables + VTableOffset;
	}
	InitReferences();
}

//==========================================================================
//
//	VClass::InitReferences
//
//==========================================================================

void VClass::InitReferences()
{
	guard(VClass::InitReferences);
	if (ObjectFlags & CLASSOF_RefsInitialised)
	{
		return;
	}

	ReferenceFields = NULL;
	if (GetSuperClass())
	{
		GetSuperClass()->InitReferences();
		ReferenceFields = GetSuperClass()->ReferenceFields;
	}

	for (VField* F = Fields; F; F = F->Next)
	{
		switch (F->Type.Type)
		{
		case ev_reference:
		case ev_delegate:
			F->NextReference = ReferenceFields;
			ReferenceFields = F;
			break;
		
		case ev_struct:
			F->Type.Struct->InitReferences();
			if (F->Type.Struct->ReferenceFields)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			break;
		
		case ev_array:
			if (F->Type.ArrayInnerType == ev_reference)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			else if (F->Type.ArrayInnerType == ev_struct)
			{
				F->Type.Struct->InitReferences();
				if (F->Type.Struct->ReferenceFields)
				{
					F->NextReference = ReferenceFields;
					ReferenceFields = F;
				}
			}
			break;
		}
	}

	ObjectFlags |= CLASSOF_RefsInitialised;
	unguard;
}

//==========================================================================
//
//	VClass::SerialiseObject
//
//==========================================================================

void VClass::SerialiseObject(VStream& Strm, VObject* Obj)
{
	guard(SerialiseObject);
	//	Serialise parent class fields.
	if (GetSuperClass())
	{
		GetSuperClass()->SerialiseObject(Strm, Obj);
	}
	//	Serialise fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		//	Skip native and transient fields.
		if (F->Flags & (FIELD_Native | FIELD_Transient))
		{
			continue;
		}
		VField::SerialiseFieldValue(Strm, (byte*)Obj + F->Ofs, F->Type);
	}
	unguardf(("(%s)", GetName()));
}

//==========================================================================
//
//	VClass::CleanObject
//
//==========================================================================

void VClass::CleanObject(VObject* Obj)
{
	guard(CleanObject);
	for (VField* F = ReferenceFields; F; F = F->NextReference)
	{
		VField::CleanField((byte*)Obj + F->Ofs, F->Type);
	}
	unguardf(("(%s)", GetName()));
}
