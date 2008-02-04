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

class VTypeExpr;

//==========================================================================
//
//	VExpression
//
//==========================================================================

class VExpression
{
public:
	VFieldType		Type;
	VFieldType		RealType;
	int			Flags;
	TLocation	Loc;

	VExpression(const TLocation&);
	virtual ~VExpression();
	virtual VExpression* DoResolve(VEmitContext&) = 0;
	VExpression* Resolve(VEmitContext&);
	VExpression* ResolveBoolean(VEmitContext&);
	virtual VTypeExpr* ResolveAsType(VEmitContext&);
	virtual VExpression* ResolveAssignmentTarget(VEmitContext&);
	virtual VExpression* ResolveIterator(VEmitContext&);
	virtual void RequestAddressOf();
	virtual void Emit(VEmitContext&) = 0;
	virtual void EmitBranchable(VEmitContext&, VLabel, bool);
	void EmitPushPointedCode(VFieldType, VEmitContext&);
	virtual bool IsValidTypeExpression();
	virtual bool IsIntConst() const;
	virtual bool IsFloatConst() const;
	virtual vint32 GetIntConst() const;
	virtual float GetFloatConst() const;
	virtual bool IsDefaultObject() const;
	virtual bool IsPropertyAssign() const;
	virtual bool IsDynArraySetNum() const;
	virtual VExpression* CreateTypeExprCopy();
	virtual bool AddDropResult();
};
