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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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

#ifndef OPCODE_INFO

#define PROG_MAGIC		"VPRG"
#define PROG_VERSION	19

#define	MAX_PARAMS		16

enum EType
{
	ev_void,
	ev_int,
	ev_float,
	ev_name,
	ev_string,
	ev_pointer,
	ev_reference,
	ev_array,
	ev_struct,
	ev_vector,
	ev_method,
	ev_classid,
	ev_bool,
	ev_delegate,
	ev_state,
	ev_unknown,

	NUM_BASIC_TYPES
};

enum
{
	FIELD_Native	= 0x0001,	//	Native serialisation
	FIELD_Transient	= 0x0002,	//	Not to be saved
	FIELD_Private	= 0x0004,	//	Private field
	FIELD_ReadOnly	= 0x0008,	//	Read-only field
};

enum
{
	FUNC_Native		= 0x0001,	// Native method
	FUNC_Static		= 0x0002,	// Static method
	FUNC_VarArgs	= 0x0004,	// Variable argument count
	FUNC_Final		= 0x0008,	// Final version of a method
};

enum
{
	MEMBER_Package,
	MEMBER_Field,
	MEMBER_Method,
	MEMBER_State,
	MEMBER_Const,
	MEMBER_Struct,
	MEMBER_Class,
};

enum
{
	OPCARGS_None,
	OPCARGS_Member,
	OPCARGS_BranchTarget,
	OPCARGS_IntBranchTarget,
	OPCARGS_Int,
	OPCARGS_Name,
	OPCARGS_String,
};

enum
{
#define DECLARE_OPC(name, args, argcount)		OPC_##name
#endif

	DECLARE_OPC(Done, None, 0),

	//	Call / return
	DECLARE_OPC(Call, Member, 1),
	DECLARE_OPC(PushVFunc, Int, 1),
	DECLARE_OPC(ICall, None, 0),
	DECLARE_OPC(Return, None, 0),
	DECLARE_OPC(ReturnL, None, 0),
	DECLARE_OPC(ReturnV, None, 0),

	//	Branching
	DECLARE_OPC(Goto, BranchTarget, 1),
	DECLARE_OPC(IfGoto, BranchTarget, 1),
	DECLARE_OPC(IfNotGoto, BranchTarget, 1),
	DECLARE_OPC(IfTopGoto, BranchTarget, 1),
	DECLARE_OPC(IfNotTopGoto, BranchTarget, 1),
	DECLARE_OPC(CaseGoto, IntBranchTarget, 2),

	//	Push constants.
	DECLARE_OPC(PushNumber, Int, 1),
	DECLARE_OPC(PushName, Name, 1),
	DECLARE_OPC(PushString, String, 1),
	DECLARE_OPC(PushClassId, Member, 1),
	DECLARE_OPC(PushState, Member, 1),

	//	Loading of variables
	DECLARE_OPC(LocalAddress, Int, 1),
	DECLARE_OPC(Offset, Int, 1),
	DECLARE_OPC(ArrayElement, Int, 1),
	DECLARE_OPC(PushPointed, None, 0),
	DECLARE_OPC(VPushPointed, None, 0),
	DECLARE_OPC(PushBool, Int, 1),
	DECLARE_OPC(PushPointedDelegate, None, 0),

	//	Integer opeartors
	DECLARE_OPC(Add, None, 0),
	DECLARE_OPC(Subtract, None, 0),
	DECLARE_OPC(Multiply, None, 0),
	DECLARE_OPC(Divide, None, 0),
	DECLARE_OPC(Modulus, None, 0),
	DECLARE_OPC(Equals, None, 0),
	DECLARE_OPC(NotEquals, None, 0),
	DECLARE_OPC(Less, None, 0),
	DECLARE_OPC(Greater, None, 0),
	DECLARE_OPC(LessEquals, None, 0),
	DECLARE_OPC(GreaterEquals, None, 0),
	DECLARE_OPC(AndLogical, None, 0),
	DECLARE_OPC(OrLogical, None, 0),
	DECLARE_OPC(NegateLogical, None, 0),
	DECLARE_OPC(AndBitwise, None, 0),
	DECLARE_OPC(OrBitwise, None, 0),
	DECLARE_OPC(XOrBitwise, None, 0),
	DECLARE_OPC(LShift, None, 0),
	DECLARE_OPC(RShift, None, 0),
	DECLARE_OPC(UnaryMinus, None, 0),
	DECLARE_OPC(BitInverse, None, 0),

	//	Increment / decrement
	DECLARE_OPC(PreInc, None, 0),
	DECLARE_OPC(PreDec, None, 0),
	DECLARE_OPC(PostInc, None, 0),
	DECLARE_OPC(PostDec, None, 0),
	DECLARE_OPC(IncDrop, None, 0),
	DECLARE_OPC(DecDrop, None, 0),

	//	Integer assignment operators
	DECLARE_OPC(AssignDrop, None, 0),
	DECLARE_OPC(AddVarDrop, None, 0),
	DECLARE_OPC(SubVarDrop, None, 0),
	DECLARE_OPC(MulVarDrop, None, 0),
	DECLARE_OPC(DivVarDrop, None, 0),
	DECLARE_OPC(ModVarDrop, None, 0),
	DECLARE_OPC(AndVarDrop, None, 0),
	DECLARE_OPC(OrVarDrop, None, 0),
	DECLARE_OPC(XOrVarDrop, None, 0),
	DECLARE_OPC(LShiftVarDrop, None, 0),
	DECLARE_OPC(RShiftVarDrop, None, 0),

	//	Floating point operators
	DECLARE_OPC(FAdd, None, 0),
	DECLARE_OPC(FSubtract, None, 0),
	DECLARE_OPC(FMultiply, None, 0),
	DECLARE_OPC(FDivide, None, 0),
	DECLARE_OPC(FEquals, None, 0),
	DECLARE_OPC(FNotEquals, None, 0),
	DECLARE_OPC(FLess, None, 0),
	DECLARE_OPC(FGreater, None, 0),
	DECLARE_OPC(FLessEquals, None, 0),
	DECLARE_OPC(FGreaterEquals, None, 0),
	DECLARE_OPC(FUnaryMinus, None, 0),

	//	Floating point assignment operators
	DECLARE_OPC(FAddVarDrop, None, 0),
	DECLARE_OPC(FSubVarDrop, None, 0),
	DECLARE_OPC(FMulVarDrop, None, 0),
	DECLARE_OPC(FDivVarDrop, None, 0),

	//	Vector operators
	DECLARE_OPC(VAdd, None, 0),
	DECLARE_OPC(VSubtract, None, 0),
	DECLARE_OPC(VPreScale, None, 0),
	DECLARE_OPC(VPostScale, None, 0),
	DECLARE_OPC(VIScale, None, 0),
	DECLARE_OPC(VEquals, None, 0),
	DECLARE_OPC(VNotEquals, None, 0),
	DECLARE_OPC(VUnaryMinus, None, 0),

	//	Vector assignment operators
	DECLARE_OPC(VAssignDrop, None, 0),
	DECLARE_OPC(VAddVarDrop, None, 0),
	DECLARE_OPC(VSubVarDrop, None, 0),
	DECLARE_OPC(VScaleVarDrop, None, 0),
	DECLARE_OPC(VIScaleVarDrop, None, 0),

	//	Pointer opeartors
	DECLARE_OPC(PtrEquals, None, 0),
	DECLARE_OPC(PtrNotEquals, None, 0),

	//	Drop result
	DECLARE_OPC(Drop, None, 0),
	DECLARE_OPC(VDrop, None, 0),

	//	Swap stack elements.
	DECLARE_OPC(Swap, None, 0),
	DECLARE_OPC(Swap3, None, 0),

	//	Special assignment operators
	DECLARE_OPC(AssignPtrDrop, None, 0),
	DECLARE_OPC(AssignBool, Int, 1),
	DECLARE_OPC(AssignDelegate, None, 0),


	//	Dynamic cast
	DECLARE_OPC(DynamicCast, Member, 1),

#undef DECLARE_OPC
#ifndef OPCODE_INFO
	NUM_OPCODES
};

struct dprograms_t
{
	char	magic[4];		//	"VPRG"
	int		version;

	int		ofs_names;
	int		num_names;

	int		num_strings;
	int		ofs_strings;

	int		ofs_mobjinfo;
	int		num_mobjinfo;

	int		ofs_scriptids;
	int		num_scriptids;

	int		ofs_exportinfo;
	int		ofs_exportdata;
	int		num_exports;

	int		ofs_imports;
	int		num_imports;
};

#endif
