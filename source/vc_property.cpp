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
//	VProperty::VProperty
//
//==========================================================================

VProperty::VProperty(VName AName, VMemberBase* AOuter, TLocation ALoc)
: VMemberBase(MEMBER_Property, AName, AOuter, ALoc)
, Type(TYPE_Void)
, GetFunc(NULL)
, SetFunc(NULL)
, DefaultField(NULL)
, Flags(0)
, TypeExpr(NULL)
, DefaultFieldName(NAME_None)
{
}

//==========================================================================
//
//	VProperty::~VProperty
//
//==========================================================================

VProperty::~VProperty()
{
	if (TypeExpr)
	{
		delete TypeExpr;
		TypeExpr = NULL;
	}
}

//==========================================================================
//
//	VProperty::Serialise
//
//==========================================================================

void VProperty::Serialise(VStream& Strm)
{
	guard(VProperty::Serialise);
	VMemberBase::Serialise(Strm);
	Strm << Type << GetFunc << SetFunc << DefaultField << Flags;
	unguard;
}

//==========================================================================
//
//	VProperty::Define
//
//==========================================================================

bool VProperty::Define()
{
	if (TypeExpr)
	{
		VEmitContext ec(this);
		TypeExpr = TypeExpr->ResolveAsType(ec);
	}
	if (!TypeExpr)
	{
		return false;
	}

	if (TypeExpr->Type.Type == TYPE_Void)
	{
		ParseError(TypeExpr->Loc, "Property cannot have void type.");
		return false;
	}
	Type = TypeExpr->Type;

	if (DefaultFieldName != NAME_None)
	{
		DefaultField = ((VClass*)Outer)->FindField(DefaultFieldName, Loc,
			(VClass*)Outer);
		if (!DefaultField)
		{
			ParseError(Loc, "No such field %s", *DefaultFieldName);
			return false;
		}
	}

	VProperty* BaseProp = NULL;
	if (((VClass*)Outer)->ParentClass)
	{
		BaseProp = ((VClass*)Outer)->ParentClass->FindProperty(Name);
	}
	if (BaseProp)
	{
		if (BaseProp->Flags & PROP_Final)
		{
			ParseError(Loc, "Property alaready has been declared final and cannot be overriden");
		}
		if (!Type.Equals(BaseProp->Type))
		{
			ParseError(Loc, "Property redeclared with a different type");
		}
	}
	return true;
}
