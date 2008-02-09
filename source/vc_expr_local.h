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
//	VLocalEntry
//
//==========================================================================

class VLocalEntry
{
public:
	VExpression*	TypeExpr;
	VName			Name;
	TLocation		Loc;
	VExpression*	Value;

	VLocalEntry()
	: TypeExpr(NULL)
	, Name(NAME_None)
	, Value(NULL)
	{}
};

//==========================================================================
//
//	VLocalDecl
//
//==========================================================================

class VLocalDecl : public VExpression
{
public:
	TArray<VLocalEntry>	Vars;

	VLocalDecl(const TLocation&);
	~VLocalDecl();

	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);

	void Declare(VEmitContext&);
	void EmitInitialisations(VEmitContext&);
};

//==========================================================================
//
//	VLocalVar
//
//==========================================================================

class VLocalVar : public VExpression
{
public:
	int				num;
	bool			AddressRequested;
	bool			PushOutParam;

	VLocalVar(int ANum, const TLocation& ALoc);
	VExpression* DoResolve(VEmitContext&);
	void RequestAddressOf();
	void Emit(VEmitContext&);
};
