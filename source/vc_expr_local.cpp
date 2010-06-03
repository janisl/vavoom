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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLocalDecl::VLocalDecl
//
//==========================================================================

VLocalDecl::VLocalDecl(const TLocation& ALoc)
: VExpression(ALoc)
{
}

//==========================================================================
//
//	VLocalDecl::~VLocalDecl
//
//==========================================================================

VLocalDecl::~VLocalDecl()
{
	for (int i = 0; i < Vars.Num(); i++)
	{
		if (Vars[i].TypeExpr)
		{
			delete Vars[i].TypeExpr;
			Vars[i].TypeExpr = NULL;
		}
		if (Vars[i].Value)
		{
			delete Vars[i].Value;
			Vars[i].Value = NULL;
		}
	}
}

//==========================================================================
//
//	VLocalDecl::DoResolve
//
//==========================================================================

VExpression* VLocalDecl::DoResolve(VEmitContext& ec)
{
	Declare(ec);
	return this;
}

//==========================================================================
//
//	VLocalDecl::Emit
//
//==========================================================================

void VLocalDecl::Emit(VEmitContext& ec)
{
	EmitInitialisations(ec);
}

//==========================================================================
//
//	VLocalDecl::Declare
//
//==========================================================================

void VLocalDecl::Declare(VEmitContext& ec)
{
	for (int i = 0; i < Vars.Num(); i++)
	{
		VLocalEntry& e = Vars[i];

		if (ec.CheckForLocalVar(e.Name) != -1)
		{
			ParseError(e.Loc, "Redefined identifier %s", *e.Name);
		}

		e.TypeExpr = e.TypeExpr->ResolveAsType(ec);
		if (!e.TypeExpr)
		{
			continue;
		}
		VFieldType Type = e.TypeExpr->Type;
		if (Type.Type == TYPE_Void)
		{
			ParseError(e.TypeExpr->Loc, "Bad variable type");
		}

		VLocalVarDef& L = ec.LocalDefs.Alloc();
		L.Name = e.Name;
		L.Type = Type;
		L.Offset = ec.localsofs;
		L.Visible = false;
		L.ParamFlags = 0;

		//  Initialisation
		if (e.Value)
		{
			VExpression* op1 = new VLocalVar(ec.LocalDefs.Num() - 1, e.Loc);
			e.Value = new VAssignment(VAssignment::Assign, op1, e.Value, e.Loc);
			e.Value = e.Value->Resolve(ec);
		}

		L.Visible = true;

		ec.localsofs += Type.GetStackSize() / 4;
		if (ec.localsofs > 1024)
		{
			ParseWarning(e.Loc, "Local vars > 1k");
		}
	}
}

//==========================================================================
//
//	VLocalDecl::EmitInitialisations
//
//==========================================================================

void VLocalDecl::EmitInitialisations(VEmitContext& ec)
{
	for (int i = 0; i < Vars.Num(); i++)
	{
		if (Vars[i].Value)
		{
			Vars[i].Value->Emit(ec);
		}
	}
}

//==========================================================================
//
//	VLocalVar::VLocalVar
//
//==========================================================================

VLocalVar::VLocalVar(int ANum, const TLocation& ALoc)
: VExpression(ALoc)
, num(ANum)
, AddressRequested(false)
, PushOutParam(false)
{
}

//==========================================================================
//
//	VLocalVar::DoResolve
//
//==========================================================================

VExpression* VLocalVar::DoResolve(VEmitContext& ec)
{
	Type = ec.LocalDefs[num].Type;
	RealType = ec.LocalDefs[num].Type;
	if (Type.Type == TYPE_Byte || Type.Type == TYPE_Bool)
	{
		Type = VFieldType(TYPE_Int);
	}
	PushOutParam = !!(ec.LocalDefs[num].ParamFlags & FPARM_Out);
	return this;
}

//==========================================================================
//
//	VLocalVar::RequestAddressOf
//
//==========================================================================

void VLocalVar::RequestAddressOf()
{
	if (PushOutParam)
	{
		PushOutParam = false;
		return;
	}
	if (AddressRequested)
		ParseError(Loc, "Multiple address of");
	AddressRequested = true;
}

//==========================================================================
//
//	VLocalVar::Emit
//
//==========================================================================

void VLocalVar::Emit(VEmitContext& ec)
{
	if (AddressRequested)
	{
		ec.EmitLocalAddress(ec.LocalDefs[num].Offset);
	}
	else if (ec.LocalDefs[num].ParamFlags & FPARM_Out)
	{
		if (ec.LocalDefs[num].Offset < 256)
		{
			int Ofs = ec.LocalDefs[num].Offset;
			if (Ofs == 0)
				ec.AddStatement(OPC_LocalValue0);
			else if (Ofs == 1)
				ec.AddStatement(OPC_LocalValue1);
			else if (Ofs == 2)
				ec.AddStatement(OPC_LocalValue2);
			else if (Ofs == 3)
				ec.AddStatement(OPC_LocalValue3);
			else if (Ofs == 4)
				ec.AddStatement(OPC_LocalValue4);
			else if (Ofs == 5)
				ec.AddStatement(OPC_LocalValue5);
			else if (Ofs == 6)
				ec.AddStatement(OPC_LocalValue6);
			else if (Ofs == 7)
				ec.AddStatement(OPC_LocalValue7);
			else
				ec.AddStatement(OPC_LocalValueB, Ofs);
		}
		else
		{
			ec.EmitLocalAddress(ec.LocalDefs[num].Offset);
			ec.AddStatement(OPC_PushPointedPtr);
		}
		if (PushOutParam)
		{
			EmitPushPointedCode(ec.LocalDefs[num].Type, ec);
		}
	}
	else if (ec.LocalDefs[num].Offset < 256)
	{
		int Ofs = ec.LocalDefs[num].Offset;
		if (ec.LocalDefs[num].Type.Type == TYPE_Bool &&
			ec.LocalDefs[num].Type.BitMask != 1)
		{
			ParseError(Loc, "Strange local bool mask");
		}
		switch (ec.LocalDefs[num].Type.Type)
		{
		case TYPE_Int:
		case TYPE_Byte:
		case TYPE_Bool:
		case TYPE_Float:
		case TYPE_Name:
		case TYPE_Pointer:
		case TYPE_Reference:
		case TYPE_Class:
		case TYPE_State:
			if (Ofs == 0)
				ec.AddStatement(OPC_LocalValue0);
			else if (Ofs == 1)
				ec.AddStatement(OPC_LocalValue1);
			else if (Ofs == 2)
				ec.AddStatement(OPC_LocalValue2);
			else if (Ofs == 3)
				ec.AddStatement(OPC_LocalValue3);
			else if (Ofs == 4)
				ec.AddStatement(OPC_LocalValue4);
			else if (Ofs == 5)
				ec.AddStatement(OPC_LocalValue5);
			else if (Ofs == 6)
				ec.AddStatement(OPC_LocalValue6);
			else if (Ofs == 7)
				ec.AddStatement(OPC_LocalValue7);
			else
				ec.AddStatement(OPC_LocalValueB, Ofs);
			break;

		case TYPE_Vector:
			ec.AddStatement(OPC_VLocalValueB, Ofs);
			break;

		case TYPE_String:
			ec.AddStatement(OPC_StrLocalValueB, Ofs);
			break;

		default:
			ParseError(Loc, "Invalid operation of this variable type");
		}
	}
	else
	{
		ec.EmitLocalAddress(ec.LocalDefs[num].Offset);
		EmitPushPointedCode(ec.LocalDefs[num].Type, ec);
	}
}
