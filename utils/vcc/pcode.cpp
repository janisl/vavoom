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

VStatementInfo StatementInfo[NUM_OPCODES] =
{
#define DECLARE_OPC(name, args)		{ #name, OPCARGS_##args, 0}
#define OPCODE_INFO
#include "../../source/progdefs.h"
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VEmitContext::VEmitContext
//
//==========================================================================

VEmitContext::VEmitContext(VMemberBase* Member)
: CurrentFunc(NULL)
, FuncRetType(TYPE_Unknown)
, localsofs(0)
, InDefaultProperties(false)
{
	//	Find the class.
	VMemberBase* CM = Member;
	while (CM && CM->MemberType != MEMBER_Class)
	{
		CM = CM->Outer;
	}
	SelfClass = (VClass*)CM;

	VMemberBase* PM = Member;
	while (PM && PM->MemberType != MEMBER_Package)
	{
		PM = PM->Outer;
	}
	Package = (VPackage*)PM;

	if (Member->MemberType == MEMBER_Method)
	{
		CurrentFunc = (VMethod*)Member;
		CurrentFunc->Instructions.Clear();
		CurrentFunc->Instructions.Resize(1024);
		FuncRetType = CurrentFunc->ReturnType;
	}
}

//==========================================================================
//
//	VEmitContext::EndCode
//
//==========================================================================

void VEmitContext::EndCode()
{
	//	Fix-up labels.
	for (int i = 0; i < Fixups.Num(); i++)
	{
		if (Labels[Fixups[i].LabelIdx] < 0)
		{
			FatalError("Label was not marked");
		}
		if (Fixups[i].Arg == 1)
		{
			CurrentFunc->Instructions[Fixups[i].Pos].Arg1 =
				Labels[Fixups[i].LabelIdx];
		}
		else
		{
			CurrentFunc->Instructions[Fixups[i].Pos].Arg2 =
				Labels[Fixups[i].LabelIdx];
		}
	}

#ifdef OPCODE_STATS
	for (int i = 0; i < CurrentFunc->Instructions.Num(); i++)
	{
		StatementInfo[CurrentFunc->Instructions[i].Opcode].usecount++;
	}
#endif
	FInstruction& Dummy = CurrentFunc->Instructions.Alloc();
	Dummy.Opcode = OPC_Done;
//	CurrentFunc->DumpAsm();
}

//==========================================================================
//
//	VEmitContext::CheckForLocalVar
//
//==========================================================================

int VEmitContext::CheckForLocalVar(VName Name)
{
	if (Name == NAME_None)
	{
		return -1;
	}
	for (int i = 0; i < LocalDefs.Num(); i++)
	{
		if (!LocalDefs[i].Visible)
		{
			continue;
		}
		if (LocalDefs[i].Name == Name)
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
//  VEmitContext::DefineLabel
//
//==========================================================================

VLabel VEmitContext::DefineLabel()
{
	Labels.Append(-1);
	return VLabel(Labels.Num() - 1);
}

//==========================================================================
//
//  VEmitContext::MarkLabel
//
//==========================================================================

void VEmitContext::MarkLabel(VLabel l)
{
	if (l.Index < 0 || l.Index >= Labels.Num())
	{
		FatalError("Bad label index %d", l.Index);
	}
	if (Labels[l.Index] >= 0)
	{
		FatalError("Label has already been marked");
	}
	Labels[l.Index] = CurrentFunc->Instructions.Num();
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement)
{
	if (StatementInfo[statement].Args != OPCARGS_None)
	{
		FatalError("Opcode doesn't take 0 params");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = 0;
	I.Arg2 = 0;
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement, int parm1)
{
	if (StatementInfo[statement].Args != OPCARGS_Byte &&
		StatementInfo[statement].Args != OPCARGS_Short &&
		StatementInfo[statement].Args != OPCARGS_Int &&
		StatementInfo[statement].Args != OPCARGS_String)
	{
		FatalError("Opcode does\'t take 1 params");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = parm1;
	I.Arg2 = 0;
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement, float FloatArg)
{
	if (StatementInfo[statement].Args != OPCARGS_Int)
	{
		FatalError("Opcode does\'t take float argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = *(vint32*)&FloatArg;
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement, VName NameArg)
{
	if (StatementInfo[statement].Args != OPCARGS_Name)
	{
		FatalError("Opcode does\'t take name argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.NameArg = NameArg;
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement, VMemberBase* Member)
{
	if (StatementInfo[statement].Args != OPCARGS_Member &&
		StatementInfo[statement].Args != OPCARGS_FieldOffset &&
		StatementInfo[statement].Args != OPCARGS_VTableIndex)
	{
		FatalError("Opcode does\'t take member as argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Member = Member;
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement, VMemberBase* Member, int Arg)
{
	if (StatementInfo[statement].Args != OPCARGS_VTableIndex_Byte &&
		StatementInfo[statement].Args != OPCARGS_FieldOffset_Byte)
	{
		FatalError("Opcode does\'t take member and byte as argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Member = Member;
	I.Arg2 = Arg;
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement, const TType& TypeArg)
{
	if (StatementInfo[statement].Args != OPCARGS_TypeSize &&
		StatementInfo[statement].Args != OPCARGS_Type)
	{
		FatalError("Opcode doesn\'t take type as argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.TypeArg = TypeArg;
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement, VLabel Lbl)
{
	if (StatementInfo[statement].Args != OPCARGS_BranchTarget)
	{
		FatalError("Opcode does\'t take label as argument");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = 0;
	I.Arg2 = 0;

	VLabelFixup& Fix = Fixups.Alloc();
	Fix.Pos = CurrentFunc->Instructions.Num() - 1;
	Fix.Arg = 1;
	Fix.LabelIdx = Lbl.Index;
}

//==========================================================================
//
//  VEmitContext::AddStatement
//
//==========================================================================

void VEmitContext::AddStatement(int statement, int parm1, VLabel Lbl)
{
	if (StatementInfo[statement].Args != OPCARGS_ByteBranchTarget &&
		StatementInfo[statement].Args != OPCARGS_ShortBranchTarget &&
		StatementInfo[statement].Args != OPCARGS_IntBranchTarget)
	{
		FatalError("Opcode does\'t take 2 params");
	}

	FInstruction& I = CurrentFunc->Instructions.Alloc();
	I.Opcode = statement;
	I.Arg1 = parm1;
	I.Arg2 = 0;

	VLabelFixup& Fix = Fixups.Alloc();
	Fix.Pos = CurrentFunc->Instructions.Num() - 1;
	Fix.Arg = 2;
	Fix.LabelIdx = Lbl.Index;
}

//==========================================================================
//
//	VEmitContext::EmitPushNumber
//
//==========================================================================

void VEmitContext::EmitPushNumber(int Val)
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
//	VEmitContext::EmitLocalAddress
//
//==========================================================================

void VEmitContext::EmitLocalAddress(int Ofs)
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
//	VEmitContext::EmitClearStrings
//
//==========================================================================

void VEmitContext::EmitClearStrings(int Start, int End)
{
	for (int i = Start; i < End; i++)
	{
		if (LocalDefs[i].Type.Type == TYPE_String)
		{
			EmitLocalAddress(LocalDefs[i].Offset);
			AddStatement(OPC_ClearPointedStr);
		}
		if (LocalDefs[i].Type.Type == TYPE_Struct &&
			LocalDefs[i].Type.Struct->NeedsDestructor())
		{
			EmitLocalAddress(LocalDefs[i].Offset);
			AddStatement(OPC_ClearPointedStruct, LocalDefs[i].Type.Struct);
		}
		if (LocalDefs[i].Type.Type == TYPE_Array)
		{
			if (LocalDefs[i].Type.ArrayInnerType == TYPE_String)
			{
				for (int j = 0; j < LocalDefs[i].Type.ArrayDim; j++)
				{
					EmitLocalAddress(LocalDefs[i].Offset);
					EmitPushNumber(j);
					AddStatement(OPC_ArrayElement, LocalDefs[i].Type.GetArrayInnerType());
					AddStatement(OPC_ClearPointedStr);
				}
			}
			else if (LocalDefs[i].Type.ArrayInnerType == TYPE_Struct &&
				LocalDefs[i].Type.Struct->NeedsDestructor())
			{
				for (int j = 0; j < LocalDefs[i].Type.ArrayDim; j++)
				{
					EmitLocalAddress(LocalDefs[i].Offset);
					EmitPushNumber(j);
					AddStatement(OPC_ArrayElement, LocalDefs[i].Type.GetArrayInnerType());
					AddStatement(OPC_ClearPointedStruct, LocalDefs[i].Type.Struct);
				}
			}
		}
	}
}
