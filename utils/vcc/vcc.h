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

#ifndef __VCC_H__
#define __VCC_H__

// HEADER FILES ------------------------------------------------------------

#include "cmdlib.h"
using namespace VavoomUtils;

#include "../../source/progdefs.h"

//	For compatibility with engine files.
#define guard(f)
#define unguard
#define check(e)
#define Z_Malloc	Malloc
#define Z_Free		Free

class VMemberBase;

#include "array.h"
#include "stream.h"
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
#define MAX_LOCAL_DEFS			64

#define ANY_PACKAGE				((VPackage*)-1)
#define ANY_MEMBER				255

// TYPES -------------------------------------------------------------------

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
	KW_BOOL,
	KW_BREAK,
	KW_CASE,
	KW_CLASS,
	KW_CLASSID,
	KW_CONST,
	KW_CONTINUE,
	KW_DEFAULT,
	KW_DEFAULTPROPERTIES,
	KW_DELEGATE,
	KW_DO,
	KW_ELSE,
	KW_ENUM,
	KW_FALSE,
	KW_FINAL,
	KW_FLOAT,
	KW_FOR,
	KW_IF,
	KW_IMPORT,
	KW_INT,
	KW_NAME,
	KW_NATIVE,
	KW_NONE,
	KW_NULL,
	KW_PRIVATE,
	KW_READONLY,
	KW_RETURN,
	KW_SELF,
	KW_STATE,
	KW_STATIC,
	KW_STRING,
	KW_STRUCT,
	KW_SWITCH,
	KW_TRANSIENT,
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

class VMemberBase;
class VClass;
class VStruct;
class VMethod;

class TModifiers
{
public:
	enum
	{
		Native			= 0x0001,
		Static			= 0x0002,
		Abstract		= 0x0004,
		Private			= 0x0008,
		ReadOnly		= 0x0010,
		Transient		= 0x0020,
		Final			= 0x0040,
	};

	static int Parse();
	static const char* Name(int);
	static int Check(int, int);
	static int MethodAttr(int);
	static int ClassAttr(int);
	static int FieldAttr(int);
};

class TLocation
{
private:
	int		Loc;
public:
	TLocation()
	: Loc(0)
	{}
	TLocation(int SrcIdx, int Line)
	: Loc((SrcIdx << 16) | Line)
	{}
	int GetLine() const
	{
		return Loc & 0xffff;
	}
	const char* GetSource() const;
};

//
// The base class of all objects.
//
class VMemberBase
{
public:
	vuint8			MemberType;
	vint32			MemberIndex;
	VName			Name;
	VMemberBase*	Outer;
	TLocation		Loc;
	VMemberBase*	HashNext;

	static TArray<VMemberBase*>		GMembers;
	static VMemberBase*				GMembersHash[4096];

	VMemberBase(vuint8, VName, VMemberBase*, TLocation);
	virtual ~VMemberBase()
	{}

	virtual void Serialise(VStream&);
	bool IsIn(VMemberBase*) const;

	static VMemberBase* StaticFindMember(VName, VMemberBase*, vuint8);
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
	explicit TType(VClass* InClass);
	explicit TType(VStruct* InStruct);

	vuint8		type;
	vuint8		InnerType;		//	For pointers
	vuint8		ArrayInnerType;	//	For arrays
	vuint8		PtrLevel;
	int			array_dim;
	union
	{
		int			bit_mask;
		VClass*		Class;			//  Class of the reference
		VStruct*	Struct;			//  Struct data.
		VMethod*	Function;		//  Function of the delegate type.
	};

	friend VStream& operator<<(VStream&, TType&);

	bool Equals(const TType& Other) const;
	TType GetPointerInnerType() const;
	TType GetArrayInnerType() const;
	int GetSize() const;
	void CheckPassable() const;
	void CheckMatch(const TType& Other) const;
	void GetName(char* Dest) const;
};

#include "expression.h"
#include "statement.h"

class VField : public VMemberBase
{
public:
	enum { AllowedModifiers = TModifiers::Native | TModifiers::Private |
		TModifiers::ReadOnly | TModifiers::Transient };

	VField*		Next;
	TType		type;
	VMethod*	func;	// Method's function
	int			flags;

	VField(VName InName, VMemberBase* InOuter, TLocation InLoc)
	: VMemberBase(MEMBER_Field, InName, InOuter, InLoc)
	, Next(NULL)
	, type(ev_void)
	, func(0)
	, flags(0)
	{}

	void Serialise(VStream&);
	bool NeedsDestructor() const;
};

struct FInstruction
{
	vint32			Address;
	vint32			Opcode;
	vint32			Arg1;
	vint32			Arg2;
	VMemberBase*	Member;
	VName			NameArg;
	TType			TypeArg;
};

class VMethodParam
{
public:
	VExpression*	TypeExpr;
	VName			Name;
	TLocation		Loc;

	VMethodParam()
	: TypeExpr(NULL)
	, Name(NAME_None)
	{}
	~VMethodParam()
	{
		if (TypeExpr)
			delete TypeExpr;
	}
};

class VMethod : public VMemberBase
{
public:
	enum { AllowedModifiers = TModifiers::Native | TModifiers::Static |
		TModifiers::Final };

	int						NumLocals;
	int						Flags;
	TType					ReturnType;
	int						NumParams;
	int						ParamsSize;
	TType					ParamTypes[MAX_PARAMS];
	VMethodParam			Params[MAX_PARAMS];
	VStatement*				Statement;
	TArray<FInstruction>	Instructions;

	VMethod(VName InName, VMemberBase* InOuter, TLocation InLoc)
	: VMemberBase(MEMBER_Method, InName, InOuter, InLoc)
	, NumLocals(0)
	, Flags(0)
	, ReturnType(ev_void)
	, NumParams(0)
	, ParamsSize(0)
	, Statement(NULL)
	{}
	~VMethod()
	{
		if (Statement)
			delete Statement;
	}

	void Serialise(VStream&);
	void Emit();
};

class VLocalVarDef : public VMemberBase
{
public:
	int			ofs;
	TType		type;
	bool		Visible;
	bool		Cleared;

	VLocalVarDef()
	: VMemberBase(MEMBER_Field, NAME_None, NULL, TLocation())
	{}
};

class VConstant : public VMemberBase
{
public:
	vuint8		Type;
	int			value;

	VConstant(VName InName, VMemberBase* InOuter, TLocation InLoc)
	: VMemberBase(MEMBER_Const, InName, InOuter, InLoc)
	{}

	void Serialise(VStream&);
};

class VStruct : public VMemberBase
{
public:
	VStruct*		ParentStruct;
	vuint8			IsVector;
	//	Size in stack units when used as local variable.
	vint32			StackSize;
	//	Structure fields
	VField*			Fields;
	bool			Parsed;

	VStruct(VName InName, VMemberBase* InOuter, TLocation InLoc)
	: VMemberBase(MEMBER_Struct, InName, InOuter, InLoc)
	, ParentStruct(0)
	, IsVector(false)
	, StackSize(0)
	, Fields(0)
	, Parsed(true)
	{}

	void Serialise(VStream&);

	void AddField(VField* f);
	bool NeedsDestructor() const;
};

class VState : public VMemberBase
{
public:
	VName		SpriteName;
	int			frame;
	VName		ModelName;
	int			model_frame;
	float		time;
	VState*		nextstate;
	VMethod*	function;
	VName		NextStateName;
	VState*		Next;

	VState(VName InName, VMemberBase* InOuter, TLocation InLoc)
	: VMemberBase(MEMBER_State, InName, InOuter, InLoc)
	, SpriteName(NAME_None)
	, frame(0)
	, ModelName(NAME_None)
	, model_frame(0)
	, time(0)
	, nextstate(0)
	, function(0)
	, NextStateName(NAME_None)
	, Next(0)
	{}

	void Serialise(VStream&);
};

struct mobjinfo_t
{
	int		doomednum;
	VClass*	class_id;
};

class VClass : public VMemberBase
{
public:
	enum { AllowedModifiers = TModifiers::Native | TModifiers::Abstract };

	VClass*		ParentClass;
	VField*		Fields;
	VState*		States;
	VMethod*	DefaultProperties;
	bool		Parsed;

	VClass(VName InName, VMemberBase* InOuter, TLocation InLoc)
	: VMemberBase(MEMBER_Class, InName, InOuter, InLoc)
	, ParentClass(NULL)
	, Fields(NULL)
	, States(NULL)
	, DefaultProperties(NULL)
	, Parsed(true)
	{}

	void Serialise(VStream&);

	void AddField(VField* f);
	void AddState(VState* s);
};

class VPackage : public VMemberBase
{
public:
	TArray<VPackage*>	ImportedPackages;

	VPackage()
	: VMemberBase(MEMBER_Package, NAME_None, NULL, TLocation())
	{}
	VPackage(VName InName)
	: VMemberBase(MEMBER_Package, InName, NULL, TLocation())
	{}
};

struct breakInfo_t
{
	int		level;
	int		addressPtr;
};

struct continueInfo_t
{
	int		level;
	int		addressPtr;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// -- Common --

void ERR_Exit(ECompileError error, bool info, char *text, ...) __attribute__((noreturn));
void ParseError(ECompileError error);
void ParseError(ECompileError error, const char *text, ...) __attribute__ ((format(printf, 2, 3)));
void ParseError(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
void ParseError(TLocation, const char *text, ...) __attribute__ ((format(printf, 2, 3)));
void ParseWarning(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
void ParseWarning(TLocation, const char *text, ...) __attribute__ ((format(printf, 2, 3)));
void BailOut() __attribute__((noreturn));
void ERR_RemoveErrorFile();

int dprintf(const char *text, ...);

void TK_Init();
void TK_OpenSource(void *buf, size_t size);
void TK_CloseSource();
void TK_NextToken();
bool TK_Check(const char *string);
void TK_Expect(const char *string, ECompileError error);
void TK_Expect(EKeyword kwd, ECompileError error);
void TK_Expect(EPunctuation punct, ECompileError error);

void PC_Init();
void AddPackagePath(const char*);
int FindString(const char*);
int AddStatement(int);
int AddStatement(int, int);
int AddStatement(int, float);
int AddStatement(int, VName);
int AddStatement(int, VMemberBase*);
int AddStatement(int, const TType&);
int AddStatement(int, int, int);
int UndoStatement();
int GetNumInstructions();
void FixupJump(int, int);
void FixupJump(int);
void WriteBreaks();
void WriteContinues(int address);
void EmitClearStrings(int Start, int End);
void BeginCode(VMethod*);
void EndCode(VMethod*);
void PC_WriteObject(char*);
void PC_DumpAsm(char*);
VPackage* LoadPackage(VName);

int EvalConstExpression(VClass*InClass, int type);
float ConstFloatExpression();

void EmitPushNumber(int);
void EmitLocalAddress(int);

void PA_Parse();

void InitTypes();
TType MakePointerType(const TType& type);
TType MakeArrayType(const TType& type, int elcount);
TType CheckForTypeKeyword();
TType CheckForType(VClass* InClass);
TType CheckForType(VClass* InClass, VName Name);
VClass* CheckForClass();
VClass* CheckForClass(VName Name);
VMethod* CheckForFunction(VClass*, VName);
VConstant* CheckForConstant(VClass* InClass, VName);
VField* CheckForStructField(VStruct*, VName, TLocation);
VField* CheckForField(VName, VClass*, bool = true);
VMethod* CheckForMethod(VName, VClass*);
void AddConstant(VClass* InClass, VName Name, int type, int value);
int CheckForLocalVar(VName);
void EmitCode();

void InitInfoTables();
void AddToMobjInfo(int Index, VClass* Class);
void AddToScriptIds(int Index, VClass* Class);
VState* CheckForState(VName StateName, VClass* InClass);
VState* FindState(VName StateName, VClass* InClass);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TLocation		tk_Location;
extern int				tk_IncludedLines;
extern ETokenType		tk_Token;
extern int				tk_Number;
extern float			tk_Float;
extern char*			tk_String;
extern int				tk_StringI;
extern EKeyword			tk_Keyword;
extern EPunctuation		tk_Punct;
extern VName			tk_Name;

extern VPackage*			CurrentPackage;
extern int					numbuiltins;

extern TArray<mobjinfo_t>	mobj_info;
extern TArray<mobjinfo_t>	script_ids;

extern int				NumErrors;

extern TType			SelfType;
extern VClass*			SelfClass;

extern VLocalVarDef		localdefs[MAX_LOCAL_DEFS];
extern int				numlocaldefs;
extern int				localsofs;

extern int						maxlocalsofs;
extern TArray<breakInfo_t>		BreakInfo;
extern int						BreakLevel;
extern int						BreakNumLocalsOnStart;
extern TArray<continueInfo_t> 	ContinueInfo;
extern int						ContinueLevel;
extern int						ContinueNumLocalsOnStart;
extern TType					FuncRetType;

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

inline bool TK_Check(VName Name)
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
