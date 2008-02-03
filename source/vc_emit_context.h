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

class VEmitContext
{
private:
	struct VLabelFixup
	{
		int					Pos;
		int					LabelIdx;
		int					Arg;
	};

	TArray<int>				Labels;
	TArray<VLabelFixup>		Fixups;

public:
	VMethod*				CurrentFunc;
	VClass*					SelfClass;
	VPackage*				Package;

	VFieldType				FuncRetType;

	TArray<VLocalVarDef>	LocalDefs;
	int						localsofs;

	VLabel					LoopStart;
	VLabel					LoopEnd;

	bool					InDefaultProperties;

	VEmitContext(VMemberBase*);
	void EndCode();

	int CheckForLocalVar(VName);

	VLabel DefineLabel();
	void MarkLabel(VLabel);

	void AddStatement(int);
	void AddStatement(int, int);
	void AddStatement(int, float);
	void AddStatement(int, VName);
	void AddStatement(int, VMemberBase*);
	void AddStatement(int, VMemberBase*, int);
	void AddStatement(int, const VFieldType&);
	void AddStatement(int, VLabel);
	void AddStatement(int, int, VLabel);
	void EmitPushNumber(int);
	void EmitLocalAddress(int);
	void EmitClearStrings(int, int);
};

struct VStatementInfo
{
	const char*		name;
	int				Args;
	int				usecount;
};

extern VStatementInfo			StatementInfo[NUM_OPCODES];
