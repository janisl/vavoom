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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

#ifndef IN_VCC

#define DECLARE_OPC(name, args)		{ OPCARGS_##args }
#define OPCODE_INFO
static struct
{
	int		Args;
} OpcodeInfo[NUM_OPCODES] =
{
#include "progdefs.h"
};

#endif

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VMethod::VMethod
//
//==========================================================================

VMethod::VMethod(VName AName, VMemberBase* AOuter, TLocation ALoc)
: VMemberBase(MEMBER_Method, AName, AOuter, ALoc)
, NumLocals(0)
, Flags(0)
, NumParams(0)
, ParamsSize(0)
#ifndef IN_VCC
, NumInstructions(0)
, Instructions(0)
#endif
, SuperMethod(NULL)
, ReplCond(NULL)
#ifndef IN_VCC
, Profile1(0)
, Profile2(0)
, NativeFunc(0)
, VTableIndex(0)
, NetIndex(0)
, NextNetMethod(0)
#else
, ReturnType(TYPE_Void)
, Modifiers(0)
, ReturnTypeExpr(NULL)
, Statement(NULL)
#endif
{
	memset(ParamFlags, 0, sizeof(ParamFlags));
}

//==========================================================================
//
//  VMethod::~VMethod
//
//==========================================================================

VMethod::~VMethod()
{
	guard(VMethod::~VMethod);
#ifndef IN_VCC
	if (Instructions)
	{
		delete[] Instructions;
	}
#else
	if (ReturnTypeExpr)
	{
		delete ReturnTypeExpr;
	}
	if (Statement)
	{
		delete Statement;
	}
#endif
	unguard;
}

//==========================================================================
//
//  PF_Fixme
//
//==========================================================================

#ifndef IN_VCC
static void PF_Fixme()
{
	Sys_Error("unimplemented bulitin");
}
#endif

//==========================================================================
//
//	VMethod::Serialise
//
//==========================================================================

void VMethod::Serialise(VStream& Strm)
{
	guard(VMethod::Serialise);
	VMemberBase::Serialise(Strm);

	Strm << SuperMethod
		<< STRM_INDEX(NumLocals)
		<< STRM_INDEX(Flags)
		<< ReturnType
		<< STRM_INDEX(NumParams)
		<< STRM_INDEX(ParamsSize);
	for (int i = 0; i < NumParams; i++)
	{
		Strm << ParamTypes[i]
			<< ParamFlags[i];
	}
	Strm << ReplCond;

#ifndef IN_VCC
	//	Set up builtins
	if (NumParams > 16)
		Sys_Error("Function has more than 16 params");
	for (FBuiltinInfo* B = FBuiltinInfo::Builtins; B; B = B->Next)
	{
		if (Outer == B->OuterClass && !VStr::Cmp(*Name, B->Name))
		{
			if (Flags & FUNC_Native)
			{
				NativeFunc = B->Func;
				break;
			}
			else
			{
				Sys_Error("PR_LoadProgs: Builtin %s redefined", B->Name);
			}
		}
	}
	if (!NativeFunc && Flags & FUNC_Native)
	{
		//	Default builtin
		NativeFunc = PF_Fixme;
#if defined CLIENT && defined SERVER
		//	Don't abort with error, because it will be done, when this
		// function will be called (if it will be called).
		GCon->Logf(NAME_Dev, "WARNING: Builtin %s not found!",
			*GetFullName());
#endif
	}
#endif

	//
	//	Instructions
	//
#ifndef IN_VCC
	Strm << STRM_INDEX(NumInstructions);
	Instructions = new FInstruction[NumInstructions];
	for (int i = 0; i < NumInstructions; i++)
#else
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
#endif
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
#ifndef IN_VCC
		switch (OpcodeInfo[Opc].Args)
#else
		switch (StatementInfo[Opc].Args)
#endif
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
		case OPCARGS_Type:
			Strm << Instructions[i].TypeArg;
			break;
		}
	}
	unguard;
}

#ifdef IN_VCC

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
		if (!(Flags & FUNC_Final))
		{
			ParseError(Loc, "Currently static methods must be final.");
			Ret = false;
		}
	}

	if ((Flags & FUNC_VarArgs) && !(Flags & FUNC_Native))
	{
		ParseError(Loc, "Only native methods can have varargs");
		Ret = false;
	}

	if ((Flags & FUNC_Iterator) && !(Flags & FUNC_Native))
	{
		ParseError(Loc, "Iterators can only be native");
		Ret = false;
	}

	VEmitContext ec(this);

	if (ReturnTypeExpr)
	{
		ReturnTypeExpr = ReturnTypeExpr->ResolveAsType(ec);
	}
	if (ReturnTypeExpr)
	{
		VFieldType t = ReturnTypeExpr->Type;
		if (t.Type != TYPE_Void)
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
	ParamsSize = Flags & FUNC_Static ? 0 : 1;
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
		VFieldType type = P.TypeExpr->Type;

		if (type.Type == TYPE_Void)
		{
			ParseError(P.TypeExpr->Loc, "Bad variable type");
			Ret = false;
			continue;
		}

		TModifiers::Check(P.Modifiers, AllowedParmModifiers, P.Loc);

		ParamTypes[i] = type;
		ParamFlags[i] = TModifiers::ParmAttr(P.Modifiers);
		if ((ParamFlags[i] & FPARM_Optional) && (ParamFlags[i] & FPARM_Out))
		{
			ParseError(P.Loc, "Modifiers optional and out are mutually exclusive");
		}
		if (ParamFlags[i] & FPARM_Out)
		{
			ParamsSize++;
		}
		else
		{
			type.CheckPassable(P.TypeExpr->Loc);
			ParamsSize += type.GetSize() / 4;
		}
		if (ParamFlags[i] & FPARM_Optional)
		{
			ParamsSize++;
		}
	}

	//	If this is a overriden method, verify that return type and argument
	// types match.
	SuperMethod = NULL;
	if (Outer->MemberType == MEMBER_Class && Name != NAME_None &&
		((VClass*)Outer)->ParentClass)
	{
		SuperMethod = ((VClass*)Outer)->ParentClass->CheckForMethod(Name);
	}
	if (SuperMethod)
	{
		if (SuperMethod->Flags & FUNC_Final)
		{
			ParseError(Loc, "Method already has been declared as final and cannot be overriden.");
			Ret = false;
		}
		if (!SuperMethod->ReturnType.Equals(ReturnType))
		{
			ParseError(Loc, "Method redefined with different return type");
			Ret = false;
		}
		else if (SuperMethod->NumParams != NumParams)
		{
			ParseError(Loc, "Method redefined with different number of arguments");
			Ret = false;
		}
		else
		{
			for (int i = 0; i < NumParams; i++)
			{
				if (!SuperMethod->ParamTypes[i].Equals(ParamTypes[i]))
				{
					ParseError(Loc, "Type of argument %d differs from base class", i + 1);
					Ret = false;
				}
				if ((SuperMethod->ParamFlags[i] ^ ParamFlags[i]) &
					(FPARM_Optional | FPARM_Out))
				{
					ParseError(Loc, "Modifiers of argument %d differs from base class", i + 1);
					Ret = false;
				}
			}
		}

		//	Inherit network flags
		Flags |= SuperMethod->Flags & FUNC_NetFlags;
	}

	if (Flags & FUNC_Spawner)
	{
		//	Verify that it's a valid spawner method
		if (NumParams < 1)
		{
			ParseError(Loc, "Spawner method must have at least 1 argument");
		}
		else if (ParamTypes[0].Type != TYPE_Class)
		{
			ParseError(Loc, "Spawner method must have class as it's first argument");
		}
		else if (ReturnType.Type != TYPE_Reference)
		{
			ParseError(Loc, "Spawner method must return an object reference");
		}
		else if (ReturnType.Class != ParamTypes[0].Class)
		{
			ParseError(Loc, "Spawner method must return an object of the same type as class");
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
	ec.localsofs = Flags & FUNC_Static ? 0 : 1;
	if (Outer->MemberType == MEMBER_Class &&
		this == ((VClass*)Outer)->DefaultProperties)
	{
		ec.InDefaultProperties = true;
	}

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
			L.Type = ParamTypes[i];
			L.Offset = ec.localsofs;
			L.Visible = true;
			L.ParamFlags = ParamFlags[i];
		}
		if (ParamFlags[i] & FPARM_Out)
		{
			ec.localsofs++;
		}
		else
		{
			ec.localsofs += ParamTypes[i].GetSize() / 4;
		}
		if (ParamFlags[i] & FPARM_Optional)
		{
			if (P.Name != NAME_None)
			{
				VLocalVarDef& L = ec.LocalDefs.Alloc();
				L.Name = va("specified_%s", *P.Name);
				L.Type = TYPE_Int;
				L.Offset = ec.localsofs;
				L.Visible = true;
				L.ParamFlags = 0;
			}
			ec.localsofs++;
		}
	}

	for (int i = 0; i < ec.LocalDefs.Num(); i++)
	{
		if (ec.LocalDefs[i].Type.Type == TYPE_Vector &&
			!(ParamFlags[i] & FPARM_Out))
		{
			ec.AddStatement(OPC_VFixParam, ec.LocalDefs[i].Offset);
		}
	}

	if (!Statement->Resolve(ec))
	{
		return;
	}

	Statement->Emit(ec);

	if (ReturnType.Type == TYPE_Void)
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
		case OPCARGS_Type:
			{
				dprintf(" %s", *Instructions[s].TypeArg.GetName());
			}
			break;
		}
		dprintf("\n");
	}
}

#else

//==========================================================================
//
//	VMethod::PostLoad
//
//==========================================================================

void VMethod::PostLoad()
{
	guard(VMethod::PostLoad);
	//FIXME It should be called only once so it's safe for now.
	//if (ObjectFlags & CLASSOF_PostLoaded)
	//{
	//	return;
	//}

	CompileCode();

	//ObjectFlags |= CLASSOF_PostLoaded;
	unguard;
}

//==========================================================================
//
//	VMethod::CompileCode
//
//==========================================================================

#define WriteUInt8(p)	Statements.Append(p)
#define WriteInt16(p)	Statements.SetNum(Statements.Num() + 2); \
	*(vint16*)&Statements[Statements.Num() - 2] = (p)
#define WriteInt32(p)	Statements.SetNum(Statements.Num() + 4); \
	*(vint32*)&Statements[Statements.Num() - 4] = (p)
#define WritePtr(p)		Statements.SetNum(Statements.Num() + sizeof(void*)); \
	*(void**)&Statements[Statements.Num() - sizeof(void*)] = (p)
#define WriteType(T) \
	WriteUInt8(T.Type); \
	WriteUInt8(T.ArrayInnerType); \
	WriteUInt8(T.InnerType); \
	WriteUInt8(T.PtrLevel); \
	WriteInt32(T.ArrayDim); \
	WritePtr(T.Class);

void VMethod::CompileCode()
{
	guard(VMethod::CompileCode);
	Statements.Clear();
	if (!NumInstructions)
	{
		return;
	}

	OptimiseInstructions();

	for (int i = 0; i < NumInstructions - 1; i++)
	{
		Instructions[i].Address = Statements.Num();
		Statements.Append(Instructions[i].Opcode);
		switch (OpcodeInfo[Instructions[i].Opcode].Args)
		{
		case OPCARGS_None:
			break;
		case OPCARGS_Member:
			WritePtr(Instructions[i].Member);
			break;
		case OPCARGS_BranchTargetB:
			WriteUInt8(0);
			break;
		case OPCARGS_BranchTargetNB:
			WriteUInt8(0);
			break;
		case OPCARGS_BranchTargetS:
			WriteInt16(0);
			break;
		case OPCARGS_BranchTarget:
			WriteInt32(0);
			break;
		case OPCARGS_ByteBranchTarget:
			WriteUInt8(Instructions[i].Arg1);
			WriteInt16(0);
			break;
		case OPCARGS_ShortBranchTarget:
			WriteInt16(Instructions[i].Arg1);
			WriteInt16(0);
			break;
		case OPCARGS_IntBranchTarget:
			WriteInt32(Instructions[i].Arg1);
			WriteInt16(0);
			break;
		case OPCARGS_Byte:
			WriteUInt8(Instructions[i].Arg1);
			break;
		case OPCARGS_Short:
			WriteInt16(Instructions[i].Arg1);
			break;
		case OPCARGS_Int:
			WriteInt32(Instructions[i].Arg1);
			break;
		case OPCARGS_Name:
			WriteInt32(Instructions[i].NameArg.GetIndex());
			break;
		case OPCARGS_NameS:
			WriteInt16(Instructions[i].NameArg.GetIndex());
			break;
		case OPCARGS_NameB:
			WriteUInt8(Instructions[i].NameArg.GetIndex());
			break;
		case OPCARGS_String:
			WritePtr(GetPackage()->Strings + Instructions[i].Arg1);
			break;
		case OPCARGS_FieldOffset:
			//	Make sure struct / class field offsets have been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteInt32(((VField*)Instructions[i].Member)->Ofs);
			break;
		case OPCARGS_FieldOffsetS:
			//	Make sure struct / class field offsets have been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteInt16(((VField*)Instructions[i].Member)->Ofs);
			break;
		case OPCARGS_FieldOffsetB:
			//	Make sure struct / class field offsets have been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteUInt8(((VField*)Instructions[i].Member)->Ofs);
			break;
		case OPCARGS_VTableIndex:
			//	Make sure class virtual table has been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteInt16(((VMethod*)Instructions[i].Member)->VTableIndex);
			break;
		case OPCARGS_VTableIndexB:
			//	Make sure class virtual table has been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteUInt8(((VMethod*)Instructions[i].Member)->VTableIndex);
			break;
		case OPCARGS_VTableIndex_Byte:
			//	Make sure class virtual table has been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteInt16(((VMethod*)Instructions[i].Member)->VTableIndex);
			WriteUInt8(Instructions[i].Arg2);
			break;
		case OPCARGS_VTableIndexB_Byte:
			//	Make sure class virtual table has been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteUInt8(((VMethod*)Instructions[i].Member)->VTableIndex);
			WriteUInt8(Instructions[i].Arg2);
			break;
		case OPCARGS_FieldOffset_Byte:
			//	Make sure struct / class field offsets have been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteInt32(((VField*)Instructions[i].Member)->Ofs);
			WriteUInt8(Instructions[i].Arg2);
			break;
		case OPCARGS_FieldOffsetS_Byte:
			//	Make sure struct / class field offsets have been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteInt16(((VField*)Instructions[i].Member)->Ofs);
			WriteUInt8(Instructions[i].Arg2);
			break;
		case OPCARGS_FieldOffsetB_Byte:
			//	Make sure struct / class field offsets have been calculated.
			Instructions[i].Member->Outer->PostLoad();
			WriteUInt8(((VField*)Instructions[i].Member)->Ofs);
			WriteUInt8(Instructions[i].Arg2);
			break;
		case OPCARGS_TypeSize:
			WriteInt32(Instructions[i].TypeArg.GetSize());
			break;
		case OPCARGS_TypeSizeS:
			WriteInt16(Instructions[i].TypeArg.GetSize());
			break;
		case OPCARGS_TypeSizeB:
			WriteUInt8(Instructions[i].TypeArg.GetSize());
			break;
		case OPCARGS_Type:
			WriteType(Instructions[i].TypeArg);
			break;
		}
	}
	Instructions[NumInstructions - 1].Address = Statements.Num();

	for (int i = 0; i < NumInstructions - 1; i++)
	{
		switch (OpcodeInfo[Instructions[i].Opcode].Args)
		{
		case OPCARGS_BranchTargetB:
			Statements[Instructions[i].Address + 1] =
				Instructions[Instructions[i].Arg1].Address -
				Instructions[i].Address;
			break;
		case OPCARGS_BranchTargetNB:
			Statements[Instructions[i].Address + 1] =
				Instructions[i].Address -
				Instructions[Instructions[i].Arg1].Address;
			break;
		case OPCARGS_BranchTargetS:
			*(vint16*)&Statements[Instructions[i].Address + 1] =
				Instructions[Instructions[i].Arg1].Address -
				Instructions[i].Address;
			break;
		case OPCARGS_BranchTarget:
			*(vint32*)&Statements[Instructions[i].Address + 1] =
				Instructions[Instructions[i].Arg1].Address -
				Instructions[i].Address;
			break;
		case OPCARGS_ByteBranchTarget:
			*(vint16*)&Statements[Instructions[i].Address + 2] =
				Instructions[Instructions[i].Arg2].Address -
				Instructions[i].Address;
			break;
		case OPCARGS_ShortBranchTarget:
			*(vint16*)&Statements[Instructions[i].Address + 3] =
				Instructions[Instructions[i].Arg2].Address -
				Instructions[i].Address;
			break;
		case OPCARGS_IntBranchTarget:
			*(vint16*)&Statements[Instructions[i].Address + 5] =
				Instructions[Instructions[i].Arg2].Address -
				Instructions[i].Address;
			break;
		}
	}

	//	We don't need instructions anymore.
	delete[] Instructions;
	Instructions = NULL;
	unguard;
}

//==========================================================================
//
//	VMethod::OptimiseInstructions
//
//==========================================================================

void VMethod::OptimiseInstructions()
{
	guard(VMethod::OptimiseInstructions);
	int Addr = 0;
	for (int i = 0; i < NumInstructions - 1; i++)
	{
		switch (Instructions[i].Opcode)
		{
		case OPC_PushVFunc:
			//	Make sure class virtual table has been calculated.
			Instructions[i].Member->Outer->PostLoad();
			if (((VMethod*)Instructions[i].Member)->VTableIndex < 256)
			{
				Instructions[i].Opcode = OPC_PushVFuncB;
			}
			break;

		case OPC_VCall:
			//	Make sure class virtual table has been calculated.
			Instructions[i].Member->Outer->PostLoad();
			if (((VMethod*)Instructions[i].Member)->VTableIndex < 256)
			{
				Instructions[i].Opcode = OPC_VCallB;
			}
			break;

		case OPC_DelegateCall:
			//	Make sure struct / class field offsets have been calculated.
			Instructions[i].Member->Outer->PostLoad();
			if (((VField*)Instructions[i].Member)->Ofs < 256)
			{
				Instructions[i].Opcode = OPC_DelegateCallB;
			}
			else if (((VField*)Instructions[i].Member)->Ofs <= MAX_VINT16)
			{
				Instructions[i].Opcode = OPC_DelegateCallS;
			}
			break;

		case OPC_Offset:
		case OPC_FieldValue:
		case OPC_VFieldValue:
		case OPC_PtrFieldValue:
		case OPC_StrFieldValue:
		case OPC_ByteFieldValue:
		case OPC_Bool0FieldValue:
		case OPC_Bool1FieldValue:
		case OPC_Bool2FieldValue:
		case OPC_Bool3FieldValue:
			//	Make sure struct / class field offsets have been calculated.
			Instructions[i].Member->Outer->PostLoad();
			if (((VField*)Instructions[i].Member)->Ofs < 256)
			{
				Instructions[i].Opcode += 2;
			}
			else if (((VField*)Instructions[i].Member)->Ofs <= MAX_VINT16)
			{
				Instructions[i].Opcode++;
			}
			break;

		case OPC_ArrayElement:
			if (Instructions[i].TypeArg.GetSize() < 256)
			{
				Instructions[i].Opcode = OPC_ArrayElementB;
			}
			else if (Instructions[i].TypeArg.GetSize() < MAX_VINT16)
			{
				Instructions[i].Opcode = OPC_ArrayElementS;
			}
			break;

		case OPC_PushName:
			if (Instructions[i].NameArg.GetIndex() < 256)
			{
				Instructions[i].Opcode = OPC_PushNameB;
			}
			else if (Instructions[i].NameArg.GetIndex() < MAX_VINT16)
			{
				Instructions[i].Opcode = OPC_PushNameS;
			}
			break;
		}

		//	Calculate approximate addresses for jump instructions.
		Instructions[i].Address = Addr;
		switch (OpcodeInfo[Instructions[i].Opcode].Args)
		{
		case OPCARGS_None:
			Addr++;
			break;
		case OPCARGS_Member:
		case OPCARGS_String:
			Addr += 1 + sizeof(void*);
			break;
		case OPCARGS_BranchTargetB:
		case OPCARGS_BranchTargetNB:
		case OPCARGS_Byte:
		case OPCARGS_NameB:
		case OPCARGS_FieldOffsetB:
		case OPCARGS_VTableIndexB:
		case OPCARGS_TypeSizeB:
			Addr += 2;
			break;
		case OPCARGS_BranchTargetS:
		case OPCARGS_Short:
		case OPCARGS_NameS:
		case OPCARGS_FieldOffsetS:
		case OPCARGS_VTableIndex:
		case OPCARGS_VTableIndexB_Byte:
		case OPCARGS_FieldOffsetB_Byte:
		case OPCARGS_TypeSizeS:
			Addr += 3;
			break;
		case OPCARGS_ByteBranchTarget:
		case OPCARGS_VTableIndex_Byte:
		case OPCARGS_FieldOffsetS_Byte:
			Addr += 4;
			break;
		case OPCARGS_BranchTarget:
		case OPCARGS_ShortBranchTarget:
		case OPCARGS_Int:
		case OPCARGS_Name:
		case OPCARGS_FieldOffset:
		case OPCARGS_TypeSize:
			Addr += 5;
			break;
		case OPCARGS_FieldOffset_Byte:
			Addr += 6;
			break;
		case OPCARGS_IntBranchTarget:
			Addr += 7;
			break;
		case OPCARGS_Type:
			Addr += 9 + sizeof(void*);
			break;
		}
	}

	//	Now do jump instructions.
	vint32 Offs;
	for (int i = 0; i < NumInstructions - 1; i++)
	{
		switch (OpcodeInfo[Instructions[i].Opcode].Args)
		{
		case OPCARGS_BranchTarget:
			Offs = Instructions[Instructions[i].Arg1].Address -
				Instructions[i].Address;
			if (Offs >= 0 && Offs < 256)
			{
				Instructions[i].Opcode -= 3;
			}
			else if (Offs < 0 && Offs > -256)
			{
				Instructions[i].Opcode -= 2;
			}
			else if (Offs >= MIN_VINT16 && Offs <= MAX_VINT16)
			{
				Instructions[i].Opcode -= 1;
			}
			break;
		}
	}
	Instructions[NumInstructions - 1].Address = Addr;
	unguard;
}

#endif
