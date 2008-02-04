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
//	VIntLiteral
//
//==========================================================================

class VIntLiteral : public VExpression
{
public:
	vint32		Value;

	VIntLiteral(vint32, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsIntConst() const;
	vint32 GetIntConst() const;
};

//==========================================================================
//
//	VFloatLiteral
//
//==========================================================================

class VFloatLiteral : public VExpression
{
public:
	float		Value;

	VFloatLiteral(float, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsFloatConst() const;
	float GetFloatConst() const;
};

//==========================================================================
//
//	VNameLiteral
//
//==========================================================================

class VNameLiteral : public VExpression
{
public:
	VName		Value;

	VNameLiteral(VName, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VStringLiteral
//
//==========================================================================

class VStringLiteral : public VExpression
{
public:
	vint32		Value;

	VStringLiteral(vint32, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VSelf
//
//==========================================================================

class VSelf : public VExpression
{
public:
	VSelf(const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VNoneLiteral
//
//==========================================================================

class VNoneLiteral : public VExpression
{
public:
	VNoneLiteral(const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VNullLiteral
//
//==========================================================================

class VNullLiteral : public VExpression
{
public:
	VNullLiteral(const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};
