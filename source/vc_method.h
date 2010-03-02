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
//	Method flags
//
//==========================================================================

enum
{
	FUNC_Native			= 0x0001,	// Native method
	FUNC_Static			= 0x0002,	// Static method
	FUNC_VarArgs		= 0x0004,	// Variable argument count
	FUNC_Final			= 0x0008,	// Final version of a method
	FUNC_Spawner		= 0x0010,	// Automatic cast of return value
	FUNC_Net			= 0x0020,	// Method is network-replicated
	FUNC_NetReliable	= 0x0040,	// Sent reliably over the network
	FUNC_Iterator		= 0x0080,	// Can be used in foreach statements

	FUNC_NetFlags = FUNC_Net | FUNC_NetReliable,
};

//==========================================================================
//
//	Parameter flags
//
//==========================================================================

enum
{
	FPARM_Optional	= 0x01,
	FPARM_Out		= 0x02,
};

//==========================================================================
//
//	builtin_t
//
//==========================================================================

typedef void (*builtin_t)();

//==========================================================================
//
//	FBuiltinInfo
//
//==========================================================================

class FBuiltinInfo
{
	const char		*Name;
	VClass			*OuterClass;
    builtin_t		Func;
	FBuiltinInfo	*Next;

	static FBuiltinInfo *Builtins;

	friend class VMethod;

public:
	FBuiltinInfo(const char *InName, VClass *InClass, builtin_t InFunc)
		: Name(InName), OuterClass(InClass), Func(InFunc)
	{
		Next = Builtins;
		Builtins = this;
	}
};

//==========================================================================
//
//	FInstruction
//
//==========================================================================

struct FInstruction
{
	vint32			Address;
	vint32			Opcode;
	vint32			Arg1;
	vint32			Arg2;
	VMemberBase*	Member;
	VName			NameArg;
	VFieldType		TypeArg;

	friend VStream& operator << (VStream&, FInstruction&);
};

//==========================================================================
//
//	VMethodParam
//
//==========================================================================

class VMethodParam
{
public:
	VExpression*	TypeExpr;
	VName			Name;
	TLocation		Loc;

	VMethodParam();
	~VMethodParam();
};

//==========================================================================
//
//	VMethod
//
//==========================================================================

class VMethod : public VMemberBase
{
public:
	enum { MAX_PARAMS		= 16 };

	//	Persistent fields
	vint32					NumLocals;
	vint32					Flags;
	VFieldType				ReturnType;
	vint32					NumParams;
	vint32					ParamsSize;
	VFieldType				ParamTypes[MAX_PARAMS];
	vuint8					ParamFlags[MAX_PARAMS];
	TArray<FInstruction>	Instructions;
	VMethod*				SuperMethod;
	VMethod*				ReplCond;

	//	Compiler fields
	VExpression*			ReturnTypeExpr;
	VMethodParam			Params[MAX_PARAMS];
	VStatement*				Statement;
	VName					SelfTypeName;

	//	Run-time fields
	vuint32					Profile1;
	vuint32					Profile2;
	TArray<vuint8>			Statements;
	builtin_t				NativeFunc;
	vint16					VTableIndex;
	vint32					NetIndex;
	VMethod*				NextNetMethod;

	VMethod(VName, VMemberBase*, TLocation);
	~VMethod();

	void Serialise(VStream&);
	bool Define();
	void Emit();
	void DumpAsm();
	void PostLoad();

	friend inline VStream& operator<<(VStream& Strm, VMethod*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }

private:
	void CompileCode();
	void OptimiseInstructions();
};
