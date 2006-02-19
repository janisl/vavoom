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

#ifndef __VCC_H__
#define __VCC_H__

// HEADER FILES ------------------------------------------------------------

#include "cmdlib.h"
#include "../../source/progdefs.h"

using namespace VavoomUtils;

#include "array.h"
#include "names.h"
#include "name.h"

// MACROS ------------------------------------------------------------------

//	Number of elements in an array.
#define ARRAY_COUNT(array)				((int)(sizeof(array) / sizeof((array)[0])))

//	Offset of a struct member.
#define STRUCT_OFFSET(struc, member)	((int)&((struc *)NULL)->member)

#define MAX_FILE_NAME_LENGTH	512
#define MAX_QUOTED_LENGTH		256
#define MAX_IDENTIFIER_LENGTH	64
#define	MAX_PARAMS				16
#define MAX_LOCAL_DEFS			64

#define PF_VARARGS				0x8000
#define PF_COUNT_MASK			0x7fff

// TYPES -------------------------------------------------------------------

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
	ev_unknown,

	NUM_BASIC_TYPES
};

enum ECompileError
{
	ERR_NONE,
	//  Memory errors
	ERR_TOO_MANY_STRINGS,
	ERR_BREAK_OVERFLOW,
	ERR_CONTINUE_OVERFLOW,
	ERR_CASE_OVERFLOW,
	ERR_PARAMS_OVERFLOW,
    ERR_LOCALS_OVERFLOW,
	ERR_STATEMENTS_OVERFLOW,
	//  File errors
    ERR_CANT_OPEN_FILE,
	ERR_CANT_OPEN_DBGFILE,
	//  Tokeniser erros
	ERR_INCL_NESTING_TOO_DEEP,
	ERR_BAD_RADIX_CONSTANT,
	ERR_STRING_TOO_LONG,
	ERR_EOF_IN_STRING,
    ERR_NEW_LINE_INSIDE_QUOTE,
	ERR_UNKNOWN_ESC_CHAR,
	ERR_IDENTIFIER_TOO_LONG,
	ERR_BAD_CHARACTER,
	ERR_UNTERM_COMMENT,
	//  Syntax errors
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

enum ETokenType
{
	TK_NONE,
	TK_EOF,				//	Reached end of file
	TK_IDENTIFIER, 		//	Identifier, value: tk_String
	TK_PUNCT, 			//	Special symbol, value: tk_String
	TK_NAME,			//	Name constant, value: tk_Name
	TK_KEYWORD,			//	Keyword, value: tk_String
	TK_STRING,			//	String, value: tk_String
	TK_INTEGER,			//	Integer number, value: tk_Number
	TK_FLOAT,			//	Floating number, value: tk_Float
};

enum EKeyword
{
	KW_STATES = 1,
	KW_MOBJINFO,
	KW_SCRIPTID,
	KW_ABSTRACT,
	KW_ADDFIELDS,
	KW_BOOL,
	KW_BREAK,
	KW_CASE,
	KW_CLASS,
	KW_CLASSID,
	KW_CONTINUE,
	KW_DEFAULT,
	KW_DEFAULTPROPERTIES,
	KW_DELEGATE,
	KW_DO,
	KW_ELSE,
	KW_ENUM,
	KW_FALSE,
	KW_FLOAT,
	KW_FOR,
	KW_IF,
	KW_INT,
	KW_NAME,
	KW_NATIVE,
	KW_NONE,
	KW_NULL,
	KW_RETURN,
	KW_SELF,
	KW_STATIC,
	KW_STRING,
	KW_STRUCT,
	KW_SWITCH,
	KW_TRUE,
	KW_VECTOR,
	KW_VOID,
	KW_WHILE,
};

enum EPunctuation
{
	PU_VARARGS = 1,
	PU_LSHIFT_ASSIGN,
	PU_RSHIFT_ASSIGN,
	PU_ADD_ASSIGN,
	PU_MINUS_ASSIGN,
	PU_MULTIPLY_ASSIGN,
	PU_DIVIDE_ASSIGN,
	PU_MOD_ASSIGN,
	PU_AND_ASSIGN,
	PU_OR_ASSIGN,
	PU_XOR_ASSIGN,
	PU_EQ,
	PU_NE,
	PU_LE,
	PU_GE,
	PU_AND_LOG,
	PU_OR_LOG,
	PU_LSHIFT,
	PU_RSHIFT,
	PU_INC,
	PU_DEC,
	PU_MINUS_GT,
	PU_DCOLON,
	PU_LT,
	PU_GT,
	PU_QUEST,
	PU_AND,
	PU_OR,
	PU_XOR,
	PU_TILDE,
	PU_NOT,
	PU_PLUS,
	PU_MINUS,
	PU_ASTERISK,
	PU_SLASH,
	PU_PERCENT,
	PU_LPAREN,
	PU_RPAREN,
	PU_DOT,
	PU_COMMA,
	PU_SEMICOLON,
	PU_COLON,
	PU_ASSIGN,
	PU_LINDEX,
	PU_RINDEX,
	PU_LBRACE,
	PU_RBRACE,
};

class TClass;
class TStruct;

//
// The base class of all objects.
//
class FField
{
public:
	FName		Name;

	FField()
	: Name(NAME_None)
	{}
};

class TType
{
public:
	TType() :
		type(ev_void), InnerType(ev_void), ArrayInnerType(ev_void),
		PtrLevel(0), array_dim(0), bit_mask(0)
	{}
	TType(EType Atype) :
		type(Atype), InnerType(ev_void), ArrayInnerType(ev_void),
		PtrLevel(0), array_dim(0), bit_mask(0)
	{}
	explicit TType(TClass* InClass);
	explicit TType(TStruct* InStruct);

	byte		type;
	byte		InnerType;		//	For pointers
	byte		ArrayInnerType;	//	For arrays
	byte		PtrLevel;
	int			array_dim;
	union
	{
		int			bit_mask;
		TClass*		Class;			//  Class of the reference
		TStruct*	Struct;			//  Struct data.
		int			FuncNum;		//  Function of the delegate type.
	};

	bool Equals(const TType& Other) const;
	TType GetPointerInnerType() const;
	TType GetArrayInnerType() const;
	int GetSize() const;
	void CheckPassable() const;
	void CheckSizeIs4() const;
	void CheckMatch(const TType& Other) const;
	void GetName(char* Dest) const;
};

struct field_t : public FField
{
	int			ofs;
	TType		type;
	int			func_num;	// Method's function
	FName		TypeName;
};

class TFunction : public FField
{
public:
	TClass*		OuterClass;
	int			FirstStatement;	//	Negative numbers are builtin functions
	int			NumLocals;
	int			Flags;
	TType		ReturnType;
	int			NumParams;
	int			ParamsSize;
	TType		ParamTypes[MAX_PARAMS];

	TFunction()
	: OuterClass(0)
	, FirstStatement(0)
	, NumLocals(0)
	, Flags(0)
	, ReturnType(ev_void)
	, NumParams(0)
	, ParamsSize(0)
	{}
};

struct localvardef_t : public FField
{
	int			ofs;
	TType		type;
};

struct constant_t : public FField
{
	TClass*		OuterClass;
	int			value;
	int			HashNext;
};

class TStruct : public FField
{
public:
	TClass*			OuterClass;
	TStruct*		ParentStruct;
	bool			IsVector;
	int				Size;
	//	Structure fields
	field_t*		Fields;
	int				NumFields;
	//	Addfield info
	int				AvailableSize;
	int				AvailableOfs;

	TStruct()
	: OuterClass(0)
	, ParentStruct(0)
	, IsVector(false)
	, Size(0)
	, Fields(0)
	, NumFields(0)
	, AvailableSize(0)
	, AvailableOfs(0)
	{}
};

class TClass : public FField
{
public:
	TClass*			ParentClass;
	field_t*		Fields;
	int				NumFields;
	int				NumProperties;
	int				OfsProperties;
	int				VTable;
	int				NumMethods;
	int				Size;
	int				Index;

	TClass()
	: ParentClass(NULL)
	, Fields(NULL)
	, NumFields(0)
	, NumProperties(0)
	, OfsProperties(0)
	, VTable(0)
	, NumMethods(0)
	, Size(0)
	, Index(0)
	{}
};

struct TPropInfo
{
	short	Type;
	short	Ofs;

	TPropInfo(short	InType, short InOfs)
	: Type(InType)
	, Ofs(InOfs)
	{}
};

struct state_t
{
	int		sprite;
	int		frame;
	int		model_index;
	int		model_frame;
	float	time;
	int		nextstate;
	int		function;
	FName	statename;
	TClass*	OuterClass;
};

struct mobjinfo_t
{
    int		doomednum;
	int		class_id;
};

class TPackage : public FField
{
	TPackage()
	{}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// -- Common --

void ERR_Exit(ECompileError error, bool info, char *text, ...) __attribute__((noreturn));
void ParseError(ECompileError error);
void ParseError(ECompileError error, const char *text, ...) __attribute__ ((format(printf, 2, 3)));
void ParseError(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
void ParseWarning(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
void BailOut(void) __attribute__((noreturn));
void ERR_RemoveErrorFile(void);

int dprintf(const char *text, ...);

void TK_Init(void);
void TK_OpenSource(void *buf, size_t size);
void TK_Restart(void);
void TK_CloseSource(void);
void TK_NextToken(void);
bool TK_Check(const char *string);
void TK_Expect(const char *string, ECompileError error);
void TK_Expect(EKeyword kwd, ECompileError error);
void TK_Expect(EPunctuation punct, ECompileError error);

void PC_Init(void);
int FindString(const char *str);
int AddStatement(int statement);
int AddStatement(int statement, int parm1);
int AddStatement(int statement, int parm1, int parm2);
int UndoStatement();
int GetNumInstructions();
void FixupJump(int Pos, int JmpPos);
void FixupJump(int Pos);
void BeginCode(int);
void EndCode(int);
void PC_WriteObject(char *name);
void PC_DumpAsm(char* name);

int EvalConstExpression(TClass*InClass, int type);
float ConstFloatExpression(void);

TType ParseExpression(bool = false);

void ParseMethodDef(const TType&, field_t*, field_t*, TClass*, int);
void ParseDelegate(const TType&, field_t*, field_t*, TClass*, int);
int ParseStateCode(TClass*);
void ParseDefaultProperties(field_t*, TClass*);
void AddConstant(TClass* InClass, FName Name, int value);
void PA_Parse();

int CheckForLocalVar(FName);
void ParseLocalVar(const TType&);
void CompileMethodDef(const TType&, field_t*, field_t*, TClass*, int);
void SkipDelegate(TClass*);
void CompileStateCode(TClass*, int);
void CompileDefaultProperties(field_t*, TClass*);
void PA_Compile();

void InitTypes();
TType MakePointerType(const TType& type);
TType MakeArrayType(const TType& type, int elcount);
TType CheckForType(TClass* InClass);
TType CheckForType(TClass* InClass, FName Name);
TClass* CheckForClass();
TClass* CheckForClass(FName Name);
int CheckForFunction(TClass*, FName);
int CheckForConstant(TClass* InClass, FName);
void SkipStruct(TClass*);
void SkipAddFields(TClass*);
void CompileClass();
field_t* ParseStructField(TStruct*);
field_t* ParseClassField(TClass*);
field_t* FindConstructor(TClass*);
void AddVirtualTables();
void ParseStruct(TClass*, bool);
void AddFields(TClass*);
void ParseClass();
field_t* CheckForField(TClass*, bool = true);
field_t* CheckForField(FName, TClass*, bool = true);

void InitInfoTables();
void ParseStates(TClass*);
void AddToMobjInfo(int Index, int ClassID);
void AddToScriptIds(int Index, int ClassID);
void SkipStates(TClass*);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern char				tk_SourceName[MAX_FILE_NAME_LENGTH];
extern int				tk_IncludedLines;
extern ETokenType		tk_Token;
extern int				tk_Line;
extern int				tk_Number;
extern float			tk_Float;
extern char*			tk_String;
extern int				tk_StringI;
extern EKeyword			tk_Keyword;
extern EPunctuation		tk_Punct;
extern FName			tk_Name;

extern TArray<TFunction>	functions;
extern int					numbuiltins;

extern TArray<constant_t>	Constants;
extern int					ConstantsHash[256];

extern TArray<TClass*>		classtypes;
extern TArray<int>			vtables;
extern TArray<TPropInfo>	propinfos;

extern TArray<TStruct*>		structtypes;

extern TArray<FName>		sprite_names;
extern TArray<FName>		models;
extern TArray<state_t>		states;
extern TArray<mobjinfo_t>	mobj_info;
extern TArray<mobjinfo_t>	script_ids;

extern int				NumErrors;

extern int				CurrentPass;

extern TType			SelfType;
extern TClass*			SelfClass;

extern localvardef_t	localdefs[MAX_LOCAL_DEFS];

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

inline bool TK_Check(int s_string)
{
	if (tk_Token == TK_IDENTIFIER && tk_StringI == s_string)
	{
		TK_NextToken();
		return true;
	}
	return false;
}

inline bool TK_Check(FName Name)
{
	if (tk_Token == TK_IDENTIFIER && tk_Name == Name)
	{
		TK_NextToken();
		return true;
	}
	return false;
}

inline bool TK_Check(EKeyword kwd)
{
	if (tk_Token == TK_KEYWORD && tk_Keyword == kwd)
	{
		TK_NextToken();
		return true;
	}
	return false;
}

inline bool TK_Check(EPunctuation punct)
{
	if (tk_Token == TK_PUNCT && tk_Punct == punct)
	{
		TK_NextToken();
		return true;
	}
	return false;
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.39  2006/02/19 20:37:02  dj_jl
//	Implemented support for delegates.
//
//	Revision 1.38  2006/02/19 14:37:36  dj_jl
//	Changed type handling.
//	
//	Revision 1.37  2006/02/17 19:25:00  dj_jl
//	Removed support for progs global variables and functions.
//	
//	Revision 1.36  2006/02/15 23:27:07  dj_jl
//	Added script ID class attribute.
//	
//	Revision 1.35  2006/01/10 19:29:10  dj_jl
//	Fixed states belonging to a class.
//	
//	Revision 1.34  2005/12/14 20:53:23  dj_jl
//	State names belong to a class.
//	Structs and enums defined in a class.
//	
//	Revision 1.33  2005/12/12 20:58:47  dj_jl
//	Removed compiler limitations.
//	
//	Revision 1.32  2005/12/07 22:52:55  dj_jl
//	Moved compiler generated data out of globals.
//	
//	Revision 1.31  2005/11/30 13:14:53  dj_jl
//	Implemented instruction buffer.
//	
//	Revision 1.30  2005/11/29 19:31:43  dj_jl
//	Class and struct classes, removed namespaces, beautification.
//	
//	Revision 1.29  2005/11/24 20:42:05  dj_jl
//	Renamed opcodes, cleanup and improvements.
//	
//	Revision 1.28  2004/12/22 07:31:57  dj_jl
//	Increased argument count limit.
//	
//	Revision 1.27  2003/03/08 12:47:52  dj_jl
//	Code cleanup.
//	
//	Revision 1.26  2002/09/07 16:36:38  dj_jl
//	Support bool in function args and return type.
//	Removed support for typedefs.
//	
//	Revision 1.25  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//	
//	Revision 1.24  2002/03/16 17:54:25  dj_jl
//	Added opcode for pushing virtual function.
//	
//	Revision 1.23  2002/03/12 19:17:30  dj_jl
//	Added keyword abstract
//	
//	Revision 1.22  2002/02/26 17:52:20  dj_jl
//	Exporting special property info into progs.
//	
//	Revision 1.21  2002/02/16 16:28:36  dj_jl
//	Added support for bool variables
//	
//	Revision 1.20  2002/02/02 19:23:02  dj_jl
//	Natives declared inside class declarations.
//	
//	Revision 1.19  2002/01/17 18:19:53  dj_jl
//	New style of adding to mobjinfo, some fixes
//	
//	Revision 1.18  2002/01/12 18:06:34  dj_jl
//	New style of state functions, some other changes
//	
//	Revision 1.17  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//	
//	Revision 1.16  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//	
//	Revision 1.15  2001/12/27 17:44:02  dj_jl
//	Removed support for C++ style constructors and destructors, some fixes
//	
//	Revision 1.14  2001/12/18 19:09:41  dj_jl
//	Some extra info in progs and other small changes
//	
//	Revision 1.13  2001/12/12 19:22:22  dj_jl
//	Support for method usage as state functions, dynamic cast
//	Added dynamic arrays
//	
//	Revision 1.12  2001/12/03 19:25:44  dj_jl
//	Fixed calling of parent function
//	Added defaultproperties
//	Fixed vectors as arguments to methods
//	
//	Revision 1.11  2001/12/01 18:17:09  dj_jl
//	Fixed calling of parent method, speedup
//	
//	Revision 1.10  2001/11/09 14:42:29  dj_jl
//	References, beautification
//	
//	Revision 1.9  2001/10/27 07:54:59  dj_jl
//	Added support for constructors and destructors
//	
//	Revision 1.8  2001/10/09 17:31:55  dj_jl
//	Addfields to class disabled by default
//	
//	Revision 1.7  2001/10/02 17:44:52  dj_jl
//	Some optimizations
//	
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
