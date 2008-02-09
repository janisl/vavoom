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
//	VAssignment
//
//==========================================================================

class VAssignment : public VExpression
{
public:
	enum EAssignOper
	{
		Assign,
		AddAssign,
		MinusAssign,
		MultiplyAssign,
		DivideAssign,
		ModAssign,
		AndAssign,
		OrAssign,
		XOrAssign,
		LShiftAssign,
		RShiftAssign,
	};
	EAssignOper		Oper;
	VExpression*	op1;
	VExpression*	op2;

	VAssignment(EAssignOper, VExpression*, VExpression*, const TLocation&);
	~VAssignment();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VPropertyAssign
//
//==========================================================================

class VPropertyAssign : public VInvocation
{
public:
	VPropertyAssign(VExpression* ASelfExpr, VMethod* AFunc, bool AHaveSelf,
		const TLocation& ALoc);
	bool IsPropertyAssign() const;
};
