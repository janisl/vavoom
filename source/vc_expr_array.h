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
//	VArrayElement
//
//==========================================================================

class VArrayElement : public VExpression
{
public:
	VExpression*		op;
	VExpression*		ind;
	bool				AddressRequested;
	bool				IsAssign;

	VArrayElement(VExpression*, VExpression*, const TLocation&);
	~VArrayElement();
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveAssignmentTarget(VEmitContext&);
	void RequestAddressOf();
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDynArrayGetNum
//
//==========================================================================

class VDynArrayGetNum : public VExpression
{
public:
	VExpression*		ArrayExpr;

	VDynArrayGetNum(VExpression*, const TLocation&);
	~VDynArrayGetNum();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDynArraySetNum
//
//==========================================================================

class VDynArraySetNum : public VExpression
{
public:
	VExpression*		ArrayExpr;
	VExpression*		NumExpr;

	VDynArraySetNum(VExpression*, VExpression*, const TLocation&);
	~VDynArraySetNum();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsDynArraySetNum() const;
};

//==========================================================================
//
//	VDynArrayInsert
//
//==========================================================================

class VDynArrayInsert : public VExpression
{
public:
	VExpression*		ArrayExpr;
	VExpression*		IndexExpr;
	VExpression*		CountExpr;

	VDynArrayInsert(VExpression*, VExpression*, VExpression*,
		const TLocation&);
	~VDynArrayInsert();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDynArrayRemove
//
//==========================================================================

class VDynArrayRemove : public VExpression
{
public:
	VExpression*		ArrayExpr;
	VExpression*		IndexExpr;
	VExpression*		CountExpr;

	VDynArrayRemove(VExpression*, VExpression*, VExpression*,
		const TLocation&);
	~VDynArrayRemove();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};
