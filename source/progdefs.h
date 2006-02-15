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

// MACROS ------------------------------------------------------------------

#define PROG_MAGIC		"VPRG"
#define PROG_VERSION	19

// TYPES -------------------------------------------------------------------

enum
{
#define DECLARE_OPC(name, argcount)		OPC_##name
#endif

	DECLARE_OPC(Done, 0),
	DECLARE_OPC(Return, 0),
	DECLARE_OPC(PushNumber, 1),
	DECLARE_OPC(PushPointed, 0),
	DECLARE_OPC(LocalAddress, 1),
	DECLARE_OPC(GlobalAddress, 1),
	DECLARE_OPC(Add, 0),
	DECLARE_OPC(Subtract, 0),
	DECLARE_OPC(Multiply, 0),
	DECLARE_OPC(Divide, 0),

	DECLARE_OPC(Modulus, 0),
	DECLARE_OPC(Equals, 0),
	DECLARE_OPC(NotEquals, 0),
	DECLARE_OPC(Less, 0),
	DECLARE_OPC(Greater, 0),
	DECLARE_OPC(LessEquals, 0),
	DECLARE_OPC(GreaterEquals, 0),
	DECLARE_OPC(AndLogical, 0),
	DECLARE_OPC(OrLogical, 0),
	DECLARE_OPC(NegateLogical, 0),

	DECLARE_OPC(AndBitwise, 0),
	DECLARE_OPC(OrBitwise, 0),
	DECLARE_OPC(XOrBitwise, 0),
	DECLARE_OPC(LShift, 0),
	DECLARE_OPC(RShift, 0),
	DECLARE_OPC(UnaryMinus, 0),
	DECLARE_OPC(BitInverse, 0),
	DECLARE_OPC(Call, 1),
	DECLARE_OPC(Goto, 1),
	DECLARE_OPC(IfGoto, 1),

	DECLARE_OPC(IfNotGoto, 1),
	DECLARE_OPC(CaseGoto, 2),
	DECLARE_OPC(Drop, 0),
	DECLARE_OPC(Assign, 0),
	DECLARE_OPC(AddVar, 0),
	DECLARE_OPC(SubVar, 0),
	DECLARE_OPC(MulVar, 0),
	DECLARE_OPC(DivVar, 0),
	DECLARE_OPC(ModVar, 0),
	DECLARE_OPC(AndVar, 0),

	DECLARE_OPC(OrVar, 0),
	DECLARE_OPC(XOrVar, 0),
	DECLARE_OPC(LShiftVar, 0),
	DECLARE_OPC(RShiftVar, 0),
	DECLARE_OPC(PreInc, 0),
	DECLARE_OPC(PreDec, 0),
	DECLARE_OPC(PostInc, 0),
	DECLARE_OPC(PostDec, 0),
	DECLARE_OPC(IfTopGoto, 1),
	DECLARE_OPC(IfNotTopGoto, 1),

	DECLARE_OPC(AssignDrop, 0),
	DECLARE_OPC(AddVarDrop, 0),
	DECLARE_OPC(SubVarDrop, 0),
	DECLARE_OPC(MulVarDrop, 0),
	DECLARE_OPC(DivVarDrop, 0),
	DECLARE_OPC(ModVarDrop, 0),
	DECLARE_OPC(AndVarDrop, 0),
	DECLARE_OPC(OrVarDrop, 0),
	DECLARE_OPC(XOrVarDrop, 0),
	DECLARE_OPC(LShiftVarDrop, 0),

	DECLARE_OPC(RShiftVarDrop, 0),
	DECLARE_OPC(IncDrop, 0),
	DECLARE_OPC(DecDrop, 0),
	DECLARE_OPC(FAdd, 0),
	DECLARE_OPC(FSubtract, 0),
	DECLARE_OPC(FMultiply, 0),
	DECLARE_OPC(FDivide, 0),
	DECLARE_OPC(FEquals, 0),
	DECLARE_OPC(FNotEquals, 0),
	DECLARE_OPC(FLess, 0),

	DECLARE_OPC(FGreater, 0),
	DECLARE_OPC(FLessEquals, 0),
	DECLARE_OPC(FGreaterEquals, 0),
	DECLARE_OPC(FUnaryMinus, 0),
	DECLARE_OPC(FAddVar, 0),
	DECLARE_OPC(FSubVar, 0),
	DECLARE_OPC(FMulVar, 0),
	DECLARE_OPC(FDivVar, 0),
	DECLARE_OPC(FAddVarDrop, 0),
	DECLARE_OPC(FSubVarDrop, 0),

	DECLARE_OPC(FMulVarDrop, 0),
	DECLARE_OPC(FDivVarDrop, 0),
	DECLARE_OPC(Swap, 0),
	DECLARE_OPC(ICall, 0),
	DECLARE_OPC(VPushPointed, 0),
	DECLARE_OPC(VAdd, 0),
	DECLARE_OPC(VSubtract, 0),
	DECLARE_OPC(VPreScale, 0),
	DECLARE_OPC(VPostScale, 0),
	DECLARE_OPC(VIScale, 0),

	DECLARE_OPC(VEquals, 0),
	DECLARE_OPC(VNotEquals, 0),
	DECLARE_OPC(VUnaryMinus, 0),
	DECLARE_OPC(VDrop, 0),
	DECLARE_OPC(VAssign, 0),
	DECLARE_OPC(VAddVar, 0),
	DECLARE_OPC(VSubVar, 0),
	DECLARE_OPC(VScaleVar, 0),
	DECLARE_OPC(VIScaleVar, 0),
	DECLARE_OPC(VAssignDrop, 0),

	DECLARE_OPC(VAddVarDrop, 0),
	DECLARE_OPC(VSubVarDrop, 0),
	DECLARE_OPC(VScaleVarDrop, 0),
	DECLARE_OPC(VIScaleVarDrop, 0),
	DECLARE_OPC(ReturnL, 0),
	DECLARE_OPC(ReturnV, 0),
	DECLARE_OPC(PushString, 1),
	DECLARE_OPC(Copy, 0),
	DECLARE_OPC(Swap3, 0),
	DECLARE_OPC(PushFunction, 1),

	DECLARE_OPC(PushClassId, 1),
	DECLARE_OPC(DynamicCast, 1),
	DECLARE_OPC(CaseGotoClassId, 2),
	DECLARE_OPC(PushName, 1),
	DECLARE_OPC(CaseGotoName, 2),
	DECLARE_OPC(PushBool, 1),
	DECLARE_OPC(AssignBool, 1),
	DECLARE_OPC(PushVFunc, 1),

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

	int		ofs_strings;    //	First string is empty

	int		ofs_statements;
	int		num_statements;	//	Instruction 0 is error

	int		ofs_globals;
	int		num_globals;

	int		ofs_functions;
	int		num_functions;	//	Function 0 is empty
	
	int		ofs_globaldefs;
	int		num_globaldefs;

	int		ofs_globalinfo;

	int		ofs_classinfo;
	int		num_classinfo;

	int		ofs_vtables;
	int		num_vtables;

	int		ofs_propinfo;
	int		num_propinfo;

	int		ofs_sprnames;
	int		num_sprnames;

	int		ofs_mdlnames;
	int		num_mdlnames;

	int		ofs_states;
	int		num_states;

	int		ofs_mobjinfo;
	int		num_mobjinfo;

	int		ofs_scriptids;
	int		num_scriptids;
};

enum
{
	FUNC_Native		= 0x0001	// Native function
};

enum
{
	GLOBALTYPE_String = 1,
	GLOBALTYPE_Class = 2,
	GLOBALTYPE_Name = 3
};

struct dfunction_t
{
	short	name;
	short	outer_class;
	int		first_statement;	//	Negative numbers are builtin functions
	short	num_parms;
	short	num_locals;
    short	type;
	short	flags;
};

struct dglobaldef_t
{
	unsigned short	name;
	unsigned short	ofs;
};

struct dclassinfo_t
{
	int		name;
	int		vtable;
	short	size;
	short	num_methods;
	int		parent;
	int		num_properties;
	int		ofs_properties;
};

struct dfield_t
{
	short	type;
	short	ofs;
};

struct dstate_t
{
	short			sprite;
	short			model_index;
	unsigned char	frame;
	unsigned char	model_frame;
	short			nextstate;
	float			time;
	short			function;
	short			statename;
};

struct dmobjinfo_t
{
	short	doomednum;
	short	class_id;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.23  2006/02/15 23:27:41  dj_jl
//	Added script ID class attribute.
//
//	Revision 1.22  2005/12/22 19:04:58  dj_jl
//	Changed version number.
//	
//	Revision 1.21  2005/12/07 22:53:26  dj_jl
//	Moved compiler generated data out of globals.
//	
//	Revision 1.20  2005/11/24 20:06:47  dj_jl
//	Renamed opcodes.
//	
//	Revision 1.19  2005/03/28 07:49:56  dj_jl
//	Changed version number.
//	
//	Revision 1.18  2004/12/22 07:37:21  dj_jl
//	Increased argument count limit.
//	
//	Revision 1.17  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.16  2002/06/14 15:36:35  dj_jl
//	Changed version number.
//	
//	Revision 1.15  2002/03/28 18:00:41  dj_jl
//	Updated version number.
//	
//	Revision 1.14  2002/03/16 17:53:12  dj_jl
//	Added opcode for pushing virtual function.
//	
//	Revision 1.13  2002/02/26 17:54:26  dj_jl
//	Importing special property info from progs and using it in saving.
//	
//	Revision 1.12  2002/02/16 16:29:26  dj_jl
//	Added support for bool variables
//	
//	Revision 1.11  2002/01/11 08:07:17  dj_jl
//	Added names to progs
//	
//	Revision 1.10  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.9  2001/12/27 17:40:06  dj_jl
//	Added method count to VClass, changed version number
//	
//	Revision 1.8  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.7  2001/12/12 19:27:46  dj_jl
//	Added dynamic cast
//	
//	Revision 1.6  2001/12/03 19:21:45  dj_jl
//	Added swaping with vector
//	
//	Revision 1.5  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.4  2001/08/21 17:39:22  dj_jl
//	Real string pointers in progs
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
