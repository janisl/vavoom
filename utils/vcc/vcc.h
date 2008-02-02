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

//	C headers
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../../source/common.h"

void* Malloc(size_t size);
void Free(void* ptr);

//	For compatibility with engine files.
#define Z_Malloc	Malloc
#define Z_Free		Free
#define Sys_Error	FatalError

#include "../../source/array.h"
#include "../../source/stream.h"
#include "../../source/names.h"
#include "../../source/name.h"
#include "../../source/str.h"
#include "../../source/progdefs.h"
#include "../../source/vc_location.h"

// MACROS ------------------------------------------------------------------

#define OPCODE_STATS

#define MAX_FILE_NAME_LENGTH	512
#define MAX_QUOTED_LENGTH		256
#define MAX_IDENTIFIER_LENGTH	64

#define ANY_PACKAGE				((VPackage*)-1)
#define ANY_MEMBER				255

#define TEXT_COLOUR_ESCAPE		'\034'

// TYPES -------------------------------------------------------------------

enum
{
	CLASS_Native		= 0x01,
	CLASS_Abstract		= 0x02,
};

enum ECompileError
{
	ERR_NONE,
	//  File errors
	ERR_CANT_OPEN_FILE,
	ERR_CANT_OPEN_DBGFILE,
	//  Tokeniser erros
	ERR_BAD_RADIX_CONSTANT,
	ERR_STRING_TOO_LONG,
	ERR_EOF_IN_STRING,
	ERR_NEW_LINE_INSIDE_QUOTE,
	ERR_UNKNOWN_ESC_CHAR,
	ERR_IDENTIFIER_TOO_LONG,
	ERR_BAD_CHARACTER,
	//  Syntax errors
	ERR_MISSING_LPAREN,
	ERR_MISSING_RPAREN,
	ERR_MISSING_LBRACE,
	ERR_MISSING_RBRACE,
	ERR_MISSING_COLON,
	ERR_MISSING_SEMICOLON,
	ERR_UNEXPECTED_EOF,
	ERR_BAD_DO_STATEMENT,
	ERR_INVALID_IDENTIFIER,
	ERR_FUNCTION_REDECLARED,
	ERR_MISSING_RFIGURESCOPE,
	ERR_BAD_ARRAY,
	ERR_EXPR_TYPE_MISTMATCH,

	NUM_ERRORS
};

#include "lexer.h"

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
		Optional		= 0x0080,
		Out				= 0x0100,
		Spawner			= 0x0200,
	};

	static int Parse(VLexer&);
	static const char* Name(int);
	static int Check(int, int, TLocation);
	static int MethodAttr(int);
	static int ClassAttr(int);
	static int FieldAttr(int);
	static int PropAttr(int);
	static int ParmAttr(int);
};

class VClass;
class VStruct;
class VMethod;
class VEmitContext;
class VPackage;

class VFieldType
{
public:
	VFieldType() :
		Type(TYPE_Void), InnerType(TYPE_Void), ArrayInnerType(TYPE_Void),
		PtrLevel(0), ArrayDim(0), Class(0)
	{}
	VFieldType(EType Atype) :
		Type(Atype), InnerType(TYPE_Void), ArrayInnerType(TYPE_Void),
		PtrLevel(0), ArrayDim(0), Class(0)
	{}
	explicit VFieldType(VClass* InClass);
	explicit VFieldType(VStruct* InStruct);

	vuint8		Type;
	vuint8		InnerType;		//	For pointers
	vuint8		ArrayInnerType;	//	For arrays
	vuint8		PtrLevel;
	int			ArrayDim;
	union
	{
		vuint32		BitMask;
		VClass*		Class;			//  Class of the reference
		VStruct*	Struct;			//  Struct data.
		VMethod*	Function;		//  Function of the delegate type.
	};

	friend VStream& operator<<(VStream&, VFieldType&);

	bool Equals(const VFieldType&) const;
	VFieldType MakePointerType() const;
	VFieldType GetPointerInnerType() const;
	VFieldType MakeArrayType(int, TLocation) const;
	VFieldType MakeDynamicArrayType(TLocation) const;
	VFieldType GetArrayInnerType() const;
	int GetSize() const;
	void CheckPassable(TLocation) const;
	void CheckMatch(TLocation, const VFieldType&) const;
	VStr GetName() const;
};

class VLabel
{
private:
	friend class VEmitContext;

	int			Index;

	VLabel(int AIndex)
	: Index(AIndex)
	{}

public:
	VLabel()
	: Index(-1)
	{}
	bool IsDefined() const
	{
		return Index != -1;
	}
};

#include "expression.h"
#include "statement.h"

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

	static TArray<const char*>		PackagePath;
	static TArray<VPackage*>		LoadedPackages;

	VMemberBase(vuint8, VName, VMemberBase*, TLocation);
	virtual ~VMemberBase();

	bool IsIn(VMemberBase*) const;

	virtual void Serialise(VStream&);

	static void AddPackagePath(const char*);
	static VPackage* LoadPackage(VName, TLocation);
	static VMemberBase* StaticFindMember(VName, VMemberBase*, vuint8);

	//FIXME This looks ugly.
	static VFieldType CheckForType(VClass*, VName);
	static VClass* CheckForClass(VName);
};

class VField : public VMemberBase
{
public:
	enum { AllowedModifiers = TModifiers::Native | TModifiers::Private |
		TModifiers::ReadOnly | TModifiers::Transient };

	VField*			Next;
	VFieldType		Type;
	VExpression*	TypeExpr;
	VMethod*		Func;	// Method's function
	vuint32 		Modifiers;
	vuint32			Flags;
	VMethod*		ReplCond;

	VField(VName, VMemberBase*, TLocation);
	~VField();

	void Serialise(VStream&);
	bool NeedsDestructor() const;
	bool Define();

	friend VStream& operator<<(VStream& Strm, VField*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

class VProperty : public VMemberBase
{
public:
	enum { AllowedModifiers = TModifiers::Native | TModifiers::Final };

	VFieldType		Type;
	VMethod*		GetFunc;
	VMethod*		SetFunc;
	VField*			DefaultField;
	vuint32			Flags;

	vuint32 		Modifiers;
	VExpression*	TypeExpr;
	VName			DefaultFieldName;

	VProperty(VName, VMemberBase*, TLocation);
	~VProperty();

	void Serialise(VStream&);
	bool Define();

	friend VStream& operator<<(VStream& Strm, VProperty*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

struct FInstruction
{
	vint32			Address;
	vint32			Opcode;
	vint32			Arg1;
	vint32			Arg2;
	VMemberBase*	Member;
	VName			NameArg;
	VFieldType		TypeArg;
};

class VMethodParam
{
public:
	VExpression*	TypeExpr;
	VName			Name;
	TLocation		Loc;
	int				Modifiers;

	VMethodParam()
	: TypeExpr(NULL)
	, Name(NAME_None)
	, Modifiers(0)
	{}
	~VMethodParam()
	{
		if (TypeExpr)
			delete TypeExpr;
	}
};

class VLocalVarDef
{
public:
	VName			Name;
	TLocation		Loc;
	int				Offset;
	VFieldType		Type;
	bool			Visible;
	vuint8			ParamFlags;

	VLocalVarDef()
	{}
};

class VMethod : public VMemberBase
{
public:
	enum { AllowedModifiers = TModifiers::Native | TModifiers::Static |
		TModifiers::Final | TModifiers::Spawner };
	enum { AllowedParmModifiers = TModifiers::Optional  | TModifiers::Out };

	int						NumLocals;
	int						Flags;
	VFieldType				ReturnType;
	int						NumParams;
	int						ParamsSize;
	VFieldType				ParamTypes[MAX_PARAMS];
	vuint8					ParamFlags[MAX_PARAMS];
	TArray<FInstruction>	Instructions;
	VMethod*				SuperMethod;
	VMethod*				ReplCond;

	vint32					Modifiers;
	VExpression*			ReturnTypeExpr;
	VMethodParam			Params[MAX_PARAMS];
	VStatement*				Statement;

	VMethod(VName, VMemberBase*, TLocation);
	~VMethod();

	void Serialise(VStream&);
	bool Define();
	void Emit();
	void DumpAsm();

	friend VStream& operator<<(VStream& Strm, VMethod*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

class VConstant : public VMemberBase
{
public:
	vuint8			Type;
	union
	{
		vint32		Value;
		float		FloatValue;
	};

	VExpression*	ValueExpr;
	VConstant*		PrevEnumValue;

	VConstant(VName, VMemberBase*, TLocation);
	~VConstant();

	void Serialise(VStream&);
	bool Define();
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

	VName			ParentStructName;
	TLocation		ParentStructLoc;
	bool			Defined;

	VStruct(VName, VMemberBase*, TLocation);

	void Serialise(VStream&);

	void AddField(VField* f);
	VField* CheckForField(VName);
	bool NeedsDestructor() const;
	bool Define();
	bool DefineMembers();

	friend VStream& operator<<(VStream& Strm, VStruct*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

class VState : public VMemberBase
{
public:
	enum { FF_FULLBRIGHT	= 0x80 };	// flag in Frame

	//	State info
	VName			SpriteName;
	vint32			Frame;
	float			Time;
	vint32			Misc1;
	vint32			Misc2;
	VState*			NextState;
	VMethod*		Function;

	//	Linked list of states
	VState*			Next;

	//	Compile time variables
	VName			GotoLabel;
	vint32			GotoOffset;
	VName			FunctionName;

	VState(VName, VMemberBase*, TLocation);
	~VState();

	void Serialise(VStream&);
	bool Define();
	void Emit();

	friend VStream& operator<<(VStream& Strm, VState*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

struct mobjinfo_t
{
	int			DoomEdNum;
	int			GameFilter;
	VClass*		Class;
};

struct VRepField
{
	VName			Name;
	TLocation		Loc;
	VMemberBase*	Member;
};

struct VRepInfo
{
	bool				Reliable;
	VMethod*			Cond;
	TArray<VRepField>	RepFields;
};

struct VStateLabel
{
	TLocation	Loc;
	VName		Name;
	VState*		State;
	VName		GotoLabel;
	vint32		GotoOffset;

	VStateLabel()
	: Name(NAME_None)
	, State(NULL)
	, GotoLabel(NAME_None)
	, GotoOffset(0)
	{}

	friend VStream& operator<<(VStream& Strm, VStateLabel& Lbl)
	{
		return Strm << Lbl.Name << Lbl.State;
	}
};

class VClass : public VMemberBase
{
public:
	enum { AllowedModifiers = TModifiers::Native | TModifiers::Abstract };

	VClass*		ParentClass;
	VField*		Fields;
	VState*		States;
	VMethod*	DefaultProperties;

	VName					ParentClassName;
	TLocation				ParentClassLoc;
	int						Modifiers;
	VExpression*			GameExpr;
	VExpression*			MobjInfoExpr;
	VExpression*			ScriptIdExpr;
	TArray<VStruct*>		Structs;
	TArray<VConstant*>		Constants;
	TArray<VProperty*>		Properties;
	TArray<VMethod*>		Methods;
	bool					Defined;
	TArray<VRepInfo>		RepInfos;
	TArray<VStateLabel>		StateLabels;

	VClass(VName, VMemberBase*, TLocation);
	~VClass();

	void Serialise(VStream&);

	void AddConstant(VConstant*);
	void AddField(VField*);
	void AddProperty(VProperty*);
	void AddState(VState*);
	void AddMethod(VMethod*);

	VMethod* CheckForFunction(VName);
	VMethod* CheckForMethod(VName);
	VConstant* CheckForConstant(VName);
	VField* CheckForField(TLocation, VName, VClass*, bool = true);
	VProperty* CheckForProperty(VName);
	VStateLabel* CheckForStateLabel(VName, bool = true);
	VState* ResolveStateLabel(TLocation, VName, int);

	bool Define();
	bool DefineMembers();
	void Emit();

	friend VStream& operator<<(VStream& Strm, VClass*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

struct VImportedPackage
{
	VName		Name;
	TLocation	Loc;
	VPackage*	Pkg;
};

class VPackage : public VMemberBase
{
private:
	struct TStringInfo
	{
		int		Offs;
		int		Next;
	};

	TArray<TStringInfo>			StringInfo;
	int							StringLookup[256];

	static int StringHashFunc(const char*);

public:
	TArray<VImportedPackage>	PackagesToLoad;

	TArray<char>				Strings;

	TArray<mobjinfo_t>			MobjInfo;
	TArray<mobjinfo_t>			ScriptIds;

	TArray<VConstant*>			ParsedConstants;
	TArray<VStruct*>			ParsedStructs;
	TArray<VClass*>				ParsedClasses;

	int							NumBuiltins;

	VPackage();
	VPackage(VName InName);

	int FindString(const char*);

	VConstant* CheckForConstant(VName);

	void Emit();
	void WriteObject(const char*);
};

class VParser
{
private:
	VLexer&			Lex;
	VPackage*		Package;
	bool			CheckForLocal;

	VExpression* ParseDotMethodCall(VExpression*, VName, TLocation);
	VExpression* ParseBaseMethodCall(VName, TLocation);
	VExpression* ParseMethodCallOrCast(VName, TLocation);
	VLocalDecl* ParseLocalVar(VExpression* TypeExpr);
	VExpression* ParseExpressionPriority0();
	VExpression* ParseExpressionPriority1();
	VExpression* ParseExpressionPriority2();
	VExpression* ParseExpressionPriority3();
	VExpression* ParseExpressionPriority4();
	VExpression* ParseExpressionPriority5();
	VExpression* ParseExpressionPriority6();
	VExpression* ParseExpressionPriority7();
	VExpression* ParseExpressionPriority8();
	VExpression* ParseExpressionPriority9();
	VExpression* ParseExpressionPriority10();
	VExpression* ParseExpressionPriority11();
	VExpression* ParseExpressionPriority12();
	VExpression* ParseExpressionPriority13();
	VExpression* ParseExpressionPriority14();
	VExpression* ParseExpression();
	VStatement* ParseStatement();
	VCompound* ParseCompoundStatement();
	VExpression* ParseType();
	void ParseMethodDef(VExpression*, VName, TLocation, VClass*, vint32, bool);
	void ParseDelegate(VExpression*, VField*);
	void ParseDefaultProperties(VClass*);
	void ParseStruct(VClass*, bool);
	VName ParseStateString();
	void ParseStates(VClass*);
	void ParseReplication(VClass*);
	void ParseClass();

public:
	VParser(VLexer& ALex, VPackage* APackage)
	: Lex(ALex)
	, Package(APackage)
	{}
	void Parse();
};

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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void FatalError(char *text, ...) __attribute__((noreturn, format(printf, 1, 2)));
void ParseError(TLocation, ECompileError error);
void ParseError(TLocation, ECompileError error, const char *text, ...) __attribute__ ((format(printf, 3, 4)));
void ParseError(TLocation, const char *text, ...) __attribute__ ((format(printf, 2, 3)));
void ParseWarning(TLocation, const char *text, ...) __attribute__ ((format(printf, 2, 3)));
void BailOut() __attribute__((noreturn));

int dprintf(const char *text, ...);

char *va(const char *text, ...) __attribute__ ((format(printf, 1, 2)));

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern bool						GBigEndian;

extern int						NumErrors;

extern VStatementInfo			StatementInfo[NUM_OPCODES];

#endif
