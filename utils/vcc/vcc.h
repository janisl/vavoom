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
//**	Copyright (C) 1999-2001 J∆nis Legzdi∑˝
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

#ifndef __VCC_H__
#define __VCC_H__

// HEADER FILES ------------------------------------------------------------

#include "cmdlib.h"
#include "../../source/progdefs.h"

using namespace VavoomUtils;

// MACROS ------------------------------------------------------------------

#define MAX_FILE_NAME_LENGTH	512
#define MAX_QUOTED_LENGTH		256
#define MAX_IDENTIFIER_LENGTH	64
#define	MAX_PARAMS				8
#define MAX_LOCAL_DEFS			64
#define	MAX_FUNCTIONS			8192
#define MAX_CONSTANTS			10000

#define PF_VARARGS				0x8000
#define PF_COUNT_MASK			0x7fff

// TYPES -------------------------------------------------------------------

enum
{
	ev_void,
	ev_int,
	ev_uint,
	ev_float,
	ev_string,
	ev_function,
	ev_pointer,
	ev_array,
	ev_struct,
	ev_vector,
	ev_class,
	ev_method,
	ev_classid,

	NUM_BASIC_TYPES
};

class TType;

struct field_t
{
	char		name[MAX_IDENTIFIER_LENGTH];
	int			ofs;
	TType		*type;
	int			func_num;	// Method's function
};

class TType
{
 public:
	TType(void) {}
	TType(const char *Aname, int Atype, TType *Anext, TType *Aaux_type, int Asize) :
		type(Atype), next(Anext), aux_type(Aaux_type), size(Asize)
	{
		strcpy(name, Aname);
	}

	char		name[MAX_IDENTIFIER_LENGTH];
	int			s_name;
	int			type;
	TType		*next;
	TType		*aux_type;
	int			size;

	//	Structure fields
	int			numfields;
	field_t		*fields;
	//	Addfield info
	int			available_size;
	int			available_ofs;

	//  Class stuff
	int			classid;
	int			num_methods;
	int			vtable;

	//	Function params
	int			num_params;
	int			params_size;
	TType		*param_types[MAX_PARAMS];
};

enum error_t
{
	ERR_NONE,
	//  Atmi∑as kı›das
	ERR_TOO_MANY_STRINGS,
	ERR_BREAK_OVERFLOW,
	ERR_CONTINUE_OVERFLOW,
	ERR_CASE_OVERFLOW,
	ERR_PARAMS_OVERFLOW,
    ERR_LOCALS_OVERFLOW,
	ERR_STATEMENTS_OVERFLOW,
	//  Failu kı›das
    ERR_CANT_OPEN_FILE,
	ERR_CANT_OPEN_DBGFILE,
	//  Sadalÿ˝anas kı›das
	ERR_INCL_NESTING_TOO_DEEP,
	ERR_BAD_RADIX_CONSTANT,
	ERR_STRING_TOO_LONG,
	ERR_EOF_IN_STRING,
    ERR_NEW_LINE_INSIDE_QUOTE,
	ERR_UNKNOWN_ESC_CHAR,
	ERR_IDENTIFIER_TOO_LONG,
	ERR_BAD_CHARACTER,
	ERR_UNTERM_COMMENT,
	ERR_TOO_MENY_CONSTANTS,
	//  Sintakses kı›das
	ERR_BAD_CONST_EXPR,
	ERR_BAD_EXPR,
	ERR_MISSING_LPAREN,
	ERR_MISSING_RPAREN,
	ERR_MISSING_LBRACE,
	ERR_MISSING_RBRACE,
	ERR_MISSING_COLON,
	ERR_MISSING_SEMICOLON,
	ERR_BAD_ARG_COUNT,
    ERR_VOID_VALUE,
    ERR_PARAM_TYPE,
	ERR_ILLEGAL_EXPR_IDENT,
	ERR_BAD_ASSIGNEMENT,
	ERR_MISPLACED_BREAK,
	ERR_MISPLACED_CONTINUE,
	ERR_UNEXPECTED_EOF,
	ERR_BAD_DO_STATEMENT,
	ERR_NO_RET_VALUE,
	ERR_VOID_RET,
	ERR_MULTIPLE_DEFAULT,
	ERR_INVALID_STATEMENT,
	ERR_INVALID_IDENTIFIER,
	ERR_VOID_VAR,
	ERR_REDEFINED_IDENTIFIER,
    ERR_TYPE_MISTMATCH,
	ERR_BAD_VAR_TYPE,
	ERR_FUNCTION_REDECLARED,
	ERR_INVALID_DECLARATOR,
	ERR_INVALID_DIRECTIVE,
	ERR_STRING_LIT_NOT_FOUND,
	ERR_UNDEFINED_FUNCTIONS,
	ERR_BAD_INCDEC,
	ERR_END_OF_NON_VOID_FUNCTION,
    ERR_NOT_A_STRUCT,
    ERR_NOT_A_FIELD,
	ERR_MISSING_RFIGURESCOPE,
	ERR_BAD_ARRAY,
    ERR_EXPR_TYPE_MISTMATCH,
    ERR_POINTER_TO_POINTER,

    NUM_ERRORS
};

enum tokenType_t
{
	TK_NONE,
	TK_EOF,				//	Sasniegtas faila biegas
	TK_IDENTIFIER, 		//	Identifik∆tors, vÒrtÿba: tk_String
	TK_PUNCT, 			//	speci∆lais simbols, vÒrtÿba: tk_String
	TK_KEYWORD,			//	atslÒgv∆rds, vÒrtÿba: tk_String
	TK_STRING,			//	simbolu virkne, vÒrtÿba: tk_String
	TK_INTEGER,			//	vesels skaitlis, vÒrtÿba: tk_Number
	TK_FLOAT,			//	peldo˝∆ komata skaitlis, vÒrtÿba: tk_Float
};

class TFunction
{
 public:
	int			s_name;
	int			first_statement;	//	Negatÿvi skaitıi ir ieb›vÒt∆s funkcijas
	int			num_locals;
	TType		*type;
};

class TGlobalDef
{
 public:
	TType		*type;
	int			ofs;
	int			s_name;
};

struct localvardef_t
{
	char	name[MAX_IDENTIFIER_LENGTH];
	TType	*type;
	int		ofs;
};

struct constant_t
{
	char	name[MAX_IDENTIFIER_LENGTH];
	int		value;
	int		next;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void PA_Parse(void);

void ERR_Exit(error_t error, boolean info, char *text, ...);
void ParseError(error_t error);
void ParseError(error_t error, const char *text, ...) __attribute__ ((format(printf, 2, 3)));
void ParseError(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
void ParseWarning(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
void BailOut(void) __attribute__((noreturn));
void ERR_RemoveErrorFile(void);

int dprintf(const char *text, ...);

void TK_Init(void);
void TK_OpenSource(char *fileName);
void TK_CloseSource(void);
void TK_NextToken(void);
boolean TK_Check(char *string);
void TK_Expect(char *string, error_t error);
void TK_AddConstant(char* name, int value);

void PC_Init(void);
int FindString(char *str);
int *AddStatement(int statement);
int *AddStatement(int statement, int parm1);
int *AddStatement(int statement, int parm1, int parm2);
int UndoStatement(void);
void PC_WriteObject(char *name);
void PC_DumpAsm(char* name);

TType *CheckForType(void);
TType *FindType(TType *type);
TType *MakePointerType(TType *type);
TType *MakeArrayType(TType *type, int elcount);
int TypeSize(TType *t);
void TypeCheckPassable(TType *type);
void TypeCheck1(TType *t);
void TypeCheck2(TType *t);
void TypeCheck3(TType *t1, TType *t2);
void ParseStruct(void);
void ParseClass(void);
void AddFields(void);
void ParseVector(void);
field_t* ParseField(TType *t);
field_t* CheckForField(TType *, bool = true);
void ParseTypeDef(void);
void AddVirtualTables(void);

void InitInfoTables(void);
void AddInfoTables(void);

int EvalConstExpression(int type);
float ConstFloatExpression(void);

TType *ParseExpression(void);

int CheckForFunction(char* name);
int CheckForGlobalVar(char* name);
int CheckForLocalVar(char* name);
int CheckForConstant(const char *name);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern char				tk_SourceName[MAX_FILE_NAME_LENGTH];
extern int				tk_IncludedLines;
extern tokenType_t		tk_Token;
extern int				tk_Line;
extern int				tk_Number;
extern float			tk_Float;
extern char*			tk_String;

extern int*				CodeBuffer;
extern int				CodeBufferSize;

extern int*				globals;
extern byte*			globalinfo;
extern int				numglobals;

extern TGlobalDef*		globaldefs;
extern int				numglobaldefs;

extern TFunction*		functions;
extern int				numfunctions;
extern int				numbuiltins;

extern constant_t		Constants[MAX_CONSTANTS];
extern int				numconstants;
extern int				ConstLookup[256];

extern TType			*ThisType;


extern char*    		strings;
extern int              strofs;

extern int				NumErrors;

extern localvardef_t	localdefs[MAX_LOCAL_DEFS];

extern TType			type_void;
extern TType			type_int;
extern TType			type_uint;
extern TType			type_float;
extern TType			type_string;
extern TType			type_function;
extern TType			type_state;
extern TType			type_mobjinfo;
extern TType			type_void_ptr;
extern TType			type_vector;
extern TType			type_classid;

extern int				numclasses;

extern TType			**classtypes;

// INLINE CODE -------------------------------------------------------------

//==========================================================================
//
//	PassFloat
//
//==========================================================================

inline int PassFloat(float f)
{
	union
    {
    	float	f;
        int		i;
	} v;

    v.f = f;
    return v.i;
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2001/09/24 17:31:38  dj_jl
//	Some fixes
//
//	Revision 1.5  2001/09/20 16:09:55  dj_jl
//	Added basic object-oriented support
//	
//	Revision 1.4  2001/09/12 17:29:54  dj_jl
//	VavoomUtils namespace usage
//	
//	Revision 1.3  2001/08/21 17:52:54  dj_jl
//	Added support for real string pointers, beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
