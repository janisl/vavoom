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
#define DECLARE_OPC(name, argcount)		OPC_##name
#endif

	DECLARE_OPC(Done, 0),
	DECLARE_OPC(Return, 0),
	DECLARE_OPC(PushNumber, 1),
	DECLARE_OPC(PushPointed, 0),
	DECLARE_OPC(LocalAddress, 1),
	DECLARE_OPC(Offset, 1),
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
	DECLARE_OPC(PushPointedDelegate, 0),
	DECLARE_OPC(AssignDelegate, 0),
	DECLARE_OPC(PushState, 1),

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

	int		ofs_statements;
	int		num_statements;

	int		ofs_vtables;
	int		num_vtables;

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

//**************************************************************************
//
//	$Log$
//	Revision 1.33  2006/03/26 13:06:18  dj_jl
//	Implemented support for modular progs.
//
//	Revision 1.32  2006/03/23 23:10:55  dj_jl
//	Added support for final methods.
//	
//	Revision 1.31  2006/03/18 16:51:15  dj_jl
//	Renamed type class names, better code serialisation.
//	
//	Revision 1.30  2006/03/13 21:22:20  dj_jl
//	Added support for read-only, private and transient fields.
//	
//	Revision 1.29  2006/03/12 20:06:02  dj_jl
//	States as objects, added state variable type.
//	
//	Revision 1.28  2006/03/10 19:31:25  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.27  2006/02/28 22:50:20  dj_jl
//	Added support for constants.
//	
//	Revision 1.26  2006/02/25 17:09:35  dj_jl
//	Import all progs type info.
//	
//	Revision 1.25  2006/02/19 20:36:03  dj_jl
//	Implemented support for delegates.
//	
//	Revision 1.24  2006/02/17 19:23:47  dj_jl
//	Removed support for progs global variables.
//	
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
