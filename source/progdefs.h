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

#ifndef __PROGDEFS_H
#define __PROGDEFS_H

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define PROG_MAGIC		"VPRG"
#define PROG_VERSION	10

// TYPES -------------------------------------------------------------------

enum
{
	OPC_DONE,
	OPC_RETURN,
	OPC_PUSHNUMBER,
	OPC_PUSHPOINTED,
    OPC_LOCALADDRESS,
    OPC_GLOBALADDRESS,
	OPC_ADD,
	OPC_SUBTRACT,
	OPC_MULTIPLY,
	OPC_DIVIDE,

	OPC_MODULUS,
	OPC_UDIVIDE,
	OPC_UMODULUS,
	OPC_EQ,
	OPC_NE,
	OPC_LT,
	OPC_GT,
	OPC_LE,
	OPC_GE,
	OPC_ULT,

	OPC_UGT,
	OPC_ULE,
	OPC_UGE,
	OPC_ANDLOGICAL,
	OPC_ORLOGICAL,
	OPC_NEGATELOGICAL,
	OPC_ANDBITWISE,
	OPC_ORBITWISE,
	OPC_XORBITWISE,
	OPC_LSHIFT,

	OPC_RSHIFT,
	OPC_URSHIFT,
	OPC_UNARYMINUS,
    OPC_BITINVERSE,
	OPC_CALL,
	OPC_GOTO,
	OPC_IFGOTO,
	OPC_IFNOTGOTO,
	OPC_CASEGOTO,
	OPC_DROP,

	OPC_ASSIGN,
	OPC_ADDVAR,
	OPC_SUBVAR,
	OPC_MULVAR,
	OPC_DIVVAR,
	OPC_MODVAR,
	OPC_UDIVVAR,
	OPC_UMODVAR,
    OPC_ANDVAR,
    OPC_ORVAR,

    OPC_XORVAR,
    OPC_LSHIFTVAR,
    OPC_RSHIFTVAR,
    OPC_URSHIFTVAR,
	OPC_PREINC,
	OPC_PREDEC,
    OPC_POSTINC,
    OPC_POSTDEC,
	OPC_IFTOPGOTO,
    OPC_IFNOTTOPGOTO,

	OPC_ASSIGN_DROP,
	OPC_ADDVAR_DROP,
	OPC_SUBVAR_DROP,
	OPC_MULVAR_DROP,
	OPC_DIVVAR_DROP,
	OPC_MODVAR_DROP,
	OPC_UDIVVAR_DROP,
	OPC_UMODVAR_DROP,
    OPC_ANDVAR_DROP,
    OPC_ORVAR_DROP,

    OPC_XORVAR_DROP,
    OPC_LSHIFTVAR_DROP,
    OPC_RSHIFTVAR_DROP,
    OPC_URSHIFTVAR_DROP,
	OPC_INC_DROP,
	OPC_DEC_DROP,
	OPC_FADD,
	OPC_FSUBTRACT,
	OPC_FMULTIPLY,
	OPC_FDIVIDE,

	OPC_FEQ,
	OPC_FNE,
	OPC_FLT,
	OPC_FGT,
	OPC_FLE,
	OPC_FGE,
	OPC_FUNARYMINUS,
	OPC_FADDVAR,
	OPC_FSUBVAR,
	OPC_FMULVAR,

	OPC_FDIVVAR,
	OPC_FADDVAR_DROP,
	OPC_FSUBVAR_DROP,
	OPC_FMULVAR_DROP,
	OPC_FDIVVAR_DROP,
	OPC_SWAP,
	OPC_ICALL,
	OPC_VPUSHPOINTED,
	OPC_VADD,
	OPC_VSUBTRACT,

	OPC_VPRESCALE,
	OPC_VPOSTSCALE,
	OPC_VISCALE,
	OPC_VEQ,
	OPC_VNE,
	OPC_VUNARYMINUS,
	OPC_VDROP,
	OPC_VASSIGN,
	OPC_VADDVAR,
	OPC_VSUBVAR,

	OPC_VSCALEVAR,
	OPC_VISCALEVAR,
	OPC_VASSIGN_DROP,
	OPC_VADDVAR_DROP,
	OPC_VSUBVAR_DROP,
	OPC_VSCALEVAR_DROP,
	OPC_VISCALEVAR_DROP,
	OPC_RETURNL,
	OPC_RETURNV,
	OPC_PUSHSTRING,

	OPC_COPY,

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
    int		num_builtins;

	int		ofs_classinfo;
	int		num_classinfo;
};

struct dfunction_t
{
	int		s_name;
	int		first_statement;	//	NegatØvi skaitõi ir iebÝvñtÆs funkcijas
	short	num_parms;
	short	num_locals;
    int		type;
};

struct globaldef_t
{
	unsigned short	type;
	unsigned short	ofs;
	int				s_name;
};

struct dclassinfo_t
{
	int		s_name;
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
