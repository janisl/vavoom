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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
#define PROG_VERSION	10

// TYPES -------------------------------------------------------------------

enum
{
#define DECLARE_OPC(name, argcount)		OPC_##name
#endif

	DECLARE_OPC(DONE, 0),
	DECLARE_OPC(RETURN, 0),
	DECLARE_OPC(PUSHNUMBER, 1),
	DECLARE_OPC(PUSHPOINTED, 0),
	DECLARE_OPC(LOCALADDRESS, 1),
	DECLARE_OPC(GLOBALADDRESS, 1),
	DECLARE_OPC(ADD, 0),
	DECLARE_OPC(SUBTRACT, 0),
	DECLARE_OPC(MULTIPLY, 0),
	DECLARE_OPC(DIVIDE, 0),

	DECLARE_OPC(MODULUS, 0),
	DECLARE_OPC(UDIVIDE, 0),
	DECLARE_OPC(UMODULUS, 0),
	DECLARE_OPC(EQ, 0),
	DECLARE_OPC(NE, 0),
	DECLARE_OPC(LT, 0),
	DECLARE_OPC(GT, 0),
	DECLARE_OPC(LE, 0),
	DECLARE_OPC(GE, 0),
	DECLARE_OPC(ULT, 0),

	DECLARE_OPC(UGT, 0),
	DECLARE_OPC(ULE, 0),
	DECLARE_OPC(UGE, 0),
	DECLARE_OPC(ANDLOGICAL, 0),
	DECLARE_OPC(ORLOGICAL, 0),
	DECLARE_OPC(NEGATELOGICAL, 0),
	DECLARE_OPC(ANDBITWISE, 0),
	DECLARE_OPC(ORBITWISE, 0),
	DECLARE_OPC(XORBITWISE, 0),
	DECLARE_OPC(LSHIFT, 0),

	DECLARE_OPC(RSHIFT, 0),
	DECLARE_OPC(URSHIFT, 0),
	DECLARE_OPC(UNARYMINUS, 0),
	DECLARE_OPC(BITINVERSE, 0),
	DECLARE_OPC(CALL, 1),
	DECLARE_OPC(GOTO, 1),
	DECLARE_OPC(IFGOTO, 1),
	DECLARE_OPC(IFNOTGOTO, 1),
	DECLARE_OPC(CASEGOTO, 2),
	DECLARE_OPC(DROP, 0),

	DECLARE_OPC(ASSIGN, 0),
	DECLARE_OPC(ADDVAR, 0),
	DECLARE_OPC(SUBVAR, 0),
	DECLARE_OPC(MULVAR, 0),
	DECLARE_OPC(DIVVAR, 0),
	DECLARE_OPC(MODVAR, 0),
	DECLARE_OPC(UDIVVAR, 0),
	DECLARE_OPC(UMODVAR, 0),
	DECLARE_OPC(ANDVAR, 0),
	DECLARE_OPC(ORVAR, 0),

	DECLARE_OPC(XORVAR, 0),
	DECLARE_OPC(LSHIFTVAR, 0),
	DECLARE_OPC(RSHIFTVAR, 0),
	DECLARE_OPC(URSHIFTVAR, 0),
	DECLARE_OPC(PREINC, 0),
	DECLARE_OPC(PREDEC, 0),
	DECLARE_OPC(POSTINC, 0),
	DECLARE_OPC(POSTDEC, 0),
	DECLARE_OPC(IFTOPGOTO, 1),
	DECLARE_OPC(IFNOTTOPGOTO, 1),

	DECLARE_OPC(ASSIGN_DROP, 0),
	DECLARE_OPC(ADDVAR_DROP, 0),
	DECLARE_OPC(SUBVAR_DROP, 0),
	DECLARE_OPC(MULVAR_DROP, 0),
	DECLARE_OPC(DIVVAR_DROP, 0),
	DECLARE_OPC(MODVAR_DROP, 0),
	DECLARE_OPC(UDIVVAR_DROP, 0),
	DECLARE_OPC(UMODVAR_DROP, 0),
	DECLARE_OPC(ANDVAR_DROP, 0),
	DECLARE_OPC(ORVAR_DROP, 0),

	DECLARE_OPC(XORVAR_DROP, 0),
	DECLARE_OPC(LSHIFTVAR_DROP, 0),
	DECLARE_OPC(RSHIFTVAR_DROP, 0),
	DECLARE_OPC(URSHIFTVAR_DROP, 0),
	DECLARE_OPC(INC_DROP, 0),
	DECLARE_OPC(DEC_DROP, 0),
	DECLARE_OPC(FADD, 0),
	DECLARE_OPC(FSUBTRACT, 0),
	DECLARE_OPC(FMULTIPLY, 0),
	DECLARE_OPC(FDIVIDE, 0),

	DECLARE_OPC(FEQ, 0),
	DECLARE_OPC(FNE, 0),
	DECLARE_OPC(FLT, 0),
	DECLARE_OPC(FGT, 0),
	DECLARE_OPC(FLE, 0),
	DECLARE_OPC(FGE, 0),
	DECLARE_OPC(FUNARYMINUS, 0),
	DECLARE_OPC(FADDVAR, 0),
	DECLARE_OPC(FSUBVAR, 0),
	DECLARE_OPC(FMULVAR, 0),

	DECLARE_OPC(FDIVVAR, 0),
	DECLARE_OPC(FADDVAR_DROP, 0),
	DECLARE_OPC(FSUBVAR_DROP, 0),
	DECLARE_OPC(FMULVAR_DROP, 0),
	DECLARE_OPC(FDIVVAR_DROP, 0),
	DECLARE_OPC(SWAP, 0),
	DECLARE_OPC(ICALL, 0),
	DECLARE_OPC(VPUSHPOINTED, 0),
	DECLARE_OPC(VADD, 0),
	DECLARE_OPC(VSUBTRACT, 0),

	DECLARE_OPC(VPRESCALE, 0),
	DECLARE_OPC(VPOSTSCALE, 0),
	DECLARE_OPC(VISCALE, 0),
	DECLARE_OPC(VEQ, 0),
	DECLARE_OPC(VNE, 0),
	DECLARE_OPC(VUNARYMINUS, 0),
	DECLARE_OPC(VDROP, 0),
	DECLARE_OPC(VASSIGN, 0),
	DECLARE_OPC(VADDVAR, 0),
	DECLARE_OPC(VSUBVAR, 0),

	DECLARE_OPC(VSCALEVAR, 0),
	DECLARE_OPC(VISCALEVAR, 0),
	DECLARE_OPC(VASSIGN_DROP, 0),
	DECLARE_OPC(VADDVAR_DROP, 0),
	DECLARE_OPC(VSUBVAR_DROP, 0),
	DECLARE_OPC(VSCALEVAR_DROP, 0),
	DECLARE_OPC(VISCALEVAR_DROP, 0),
	DECLARE_OPC(RETURNL, 0),
	DECLARE_OPC(RETURNV, 0),
	DECLARE_OPC(PUSHSTRING, 1),

	DECLARE_OPC(COPY, 0),
	DECLARE_OPC(SWAP3, 0),
	DECLARE_OPC(PUSHFUNCTION, 1),
	DECLARE_OPC(PUSHCLASSID, 1),
	DECLARE_OPC(DYNAMIC_CAST, 1),
	DECLARE_OPC(CASE_GOTO_CLASSID, 2),

#undef DECLARE_OPC
#ifndef OPCODE_INFO
	NUM_OPCODES
};

struct dprograms_t
{
	char	magic[4];		//	"VPRG"
	int		version;

	int		ofs_strings;    //	First string is empty
	int		num_strings;

	int		ofs_statements;
	int		num_statements;	//	Instruction 0 is error

	int		ofs_globals;
	int		num_globals;

	int		ofs_functions;
	int		num_functions;	//	Function 0 is empty
	
	int		ofs_globaldefs;
	int		num_globaldefs;

	int		ofs_globalinfo;
    int		reserved1;

	int		ofs_classinfo;
	int		num_classinfo;
};

enum
{
	FUNC_Native		= 0x0001	// Native function
};

struct dfunction_t
{
	union
	{
		int		s_name;
		char*	name;
	};
	int		first_statement;	//	NegatØvi skaitõi ir iebÝvñtÆs funkcijas
	short	num_parms;
	short	num_locals;
    short	type;
	short	flags;
};

struct globaldef_t
{
	unsigned short	type;
	unsigned short	ofs;
	union
	{
		int		s_name;
		char*	name;
	};
};

struct dclassinfo_t
{
	union
	{
		int		s_name;
		char*	name;
	};
	int		vtable;
	int		size;
	int		parent;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif

//**************************************************************************
//
//	$Log$
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
