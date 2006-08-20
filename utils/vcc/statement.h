//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: parse.cpp 1670 2006-08-20 12:39:46Z dj_jl $
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

class VStatement
{
public:
	TLocation		Loc;

	VStatement(const TLocation&);
	virtual ~VStatement();
	virtual bool Resolve() = 0;
	virtual void DoEmit() = 0;
	void Emit();
};

class VEmptyStatement : public VStatement
{
public:
	VEmptyStatement(const TLocation&);
	bool Resolve();
	void DoEmit();
};

class VIf : public VStatement
{
public:
	VExpression*	Expr;
	VStatement*		TrueStatement;
	VStatement*		FalseStatement;

	VIf(VExpression*, VStatement*, const TLocation&);
	VIf(VExpression*, VStatement*, VStatement*, const TLocation&);
	~VIf();
	bool Resolve();
	void DoEmit();
};

class VWhile : public VStatement
{
public:
	VExpression*		Expr;
	VStatement*			Statement;
	int					NumLocalsOnStart;

	VWhile(VExpression*, VStatement*, const TLocation&);
	~VWhile();
	bool Resolve();
	void DoEmit();
};

class VDo : public VStatement
{
public:
	VExpression*		Expr;
	VStatement*			Statement;
	int					NumLocalsOnStart;

	VDo(VExpression*, VStatement*, const TLocation&);
	~VDo();
	bool Resolve();
	void DoEmit();
};

class VFor : public VStatement
{
public:
	TArray<VExpression*>	InitExpr;
	VExpression*			CondExpr;
	TArray<VExpression*>	LoopExpr;
	VStatement*				Statement;
	int						NumLocalsOnStart;

	VFor(const TLocation&);
	~VFor();
	bool Resolve();
	void DoEmit();
};

class VSwitch : public VStatement
{
public:
	struct VCaseInfo
	{
		int		value;
		int		address;
	};

	VExpression*		Expr;
	TArray<VCaseInfo>	CaseInfo;
	int					defaultAddress;
	TArray<VStatement*>	Statements;
	int					NumLocalsOnStart;

	VSwitch(const TLocation&);
	~VSwitch();
	bool Resolve();
	void DoEmit();
};

class VSwitchCase : public VStatement
{
public:
	VSwitch*		Switch;
	VExpression*	Expr;
	vint32			Value;

	VSwitchCase(VSwitch*, VExpression*, const TLocation&);
	bool Resolve();
	void DoEmit();
};

class VSwitchDefault : public VStatement
{
public:
	VSwitch*		Switch;

	VSwitchDefault(VSwitch*, const TLocation&);
	bool Resolve();
	void DoEmit();
};

class VBreak : public VStatement
{
public:
	int				NumLocalsEnd;

	VBreak(const TLocation&);
	bool Resolve();
	void DoEmit();
};

class VContinue : public VStatement
{
public:
	int				NumLocalsEnd;

	VContinue(const TLocation&);
	bool Resolve();
	void DoEmit();
};

class VReturn : public VStatement
{
public:
	VExpression*		Expr;
	int					NumLocalsToClear;

	VReturn(VExpression*, const TLocation&);
	~VReturn();
	bool Resolve();
	void DoEmit();
};

class VExpressionStatement : public VStatement
{
public:
	VExpression*		Expr;

	VExpressionStatement(VExpression*);
	~VExpressionStatement();
	bool Resolve();
	void DoEmit();
};

class VLocalVarStatement : public VStatement
{
public:
	VLocalDecl*		Decl;

	VLocalVarStatement(VLocalDecl*);
	~VLocalVarStatement();
	bool Resolve();
	void DoEmit();
};

class VCompound : public VStatement
{
public:
	TArray<VStatement*>		Statements;
	int						NumLocalsOnStart;
	int						NumLocalsOnEnd;

	VCompound(const TLocation&);
	~VCompound();
	bool Resolve();
	void DoEmit();
};
