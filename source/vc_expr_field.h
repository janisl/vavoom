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

//==========================================================================
//
//	VPointerField
//
//==========================================================================

class VPointerField : public VExpression
{
public:
	VExpression*		op;
	VName				FieldName;

	VPointerField(VExpression*, VName, const TLocation&);
	~VPointerField();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDotField
//
//==========================================================================

class VDotField : public VExpression
{
public:
	VExpression*		op;
	VName				FieldName;

	VDotField(VExpression*, VName, const TLocation&);
	~VDotField();
	VExpression* IntResolve(VEmitContext&, bool);
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveAssignmentTarget(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VFieldAccess
//
//==========================================================================

class VFieldAccess : public VExpression
{
public:
	VExpression*		op;
	VField*				field;
	bool				AddressRequested;

	VFieldAccess(VExpression* AOp, VField* AField, const TLocation& ALoc, int ExtraFlags);
	~VFieldAccess();
	VExpression* DoResolve(VEmitContext&);
	void RequestAddressOf();
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDelegateVal
//
//==========================================================================

class VDelegateVal : public VExpression
{
public:
	VExpression*		op;
	VMethod*			M;

	VDelegateVal(VExpression* AOp, VMethod* AM, const TLocation& ALoc);
	~VDelegateVal();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};
