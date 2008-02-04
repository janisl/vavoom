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
//	VDelegateToBool
//
//==========================================================================

class VDelegateToBool : public VExpression
{
public:
	VExpression*		op;

	VDelegateToBool(VExpression* AOp);
	~VDelegateToBool();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VStringToBool
//
//==========================================================================

class VStringToBool : public VExpression
{
public:
	VExpression*		op;

	VStringToBool(VExpression* AOp);
	~VStringToBool();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VPointerToBool
//
//==========================================================================

class VPointerToBool : public VExpression
{
public:
	VExpression*		op;

	VPointerToBool(VExpression* AOp);
	~VPointerToBool();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDynamicCast
//
//==========================================================================

class VDynamicCast : public VExpression
{
public:
	VClass*				Class;
	VExpression*		op;

	VDynamicCast(VClass* AClass, VExpression* AOp, const TLocation& ALoc);
	~VDynamicCast();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDynamicClassCast
//
//==========================================================================

class VDynamicClassCast : public VExpression
{
public:
	VName				ClassName;
	VExpression*		op;

	VDynamicClassCast(VName, VExpression*, const TLocation&);
	~VDynamicClassCast();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};
