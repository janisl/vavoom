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
//	VBaseInvocation
//
//==========================================================================

class VBaseInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[VMethod::MAX_PARAMS + 1];

	VBaseInvocation(VName, int, VExpression**, const TLocation&);
	~VBaseInvocation();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VCastOrInvocation
//
//==========================================================================

class VCastOrInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[VMethod::MAX_PARAMS + 1];

	VCastOrInvocation(VName, const TLocation&, int, VExpression**);
	~VCastOrInvocation();
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveIterator(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDotInvocation
//
//==========================================================================

class VDotInvocation : public VExpression
{
public:
	VExpression*	SelfExpr;
	VName			MethodName;
	int				NumArgs;
	VExpression*	Args[VMethod::MAX_PARAMS + 1];

	VDotInvocation(VExpression*, VName, const TLocation&, int, VExpression**);
	~VDotInvocation();
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveIterator(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VInvocation
//
//==========================================================================

class VInvocation : public VExpression
{
public:
	VExpression*	SelfExpr;
	VMethod*		Func;
	VField*			DelegateField;
	bool			HaveSelf;
	bool			BaseCall;
	int				NumArgs;
	VExpression*	Args[VMethod::MAX_PARAMS + 1];

	VInvocation(VExpression* ASelfExpr, VMethod* AFunc, VField* ADelegateField,
		bool AHaveSelf, bool ABaseCall, const TLocation& ALoc, int ANumArgs,
		VExpression** AArgs);
	~VInvocation();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	void CheckParams();
};
