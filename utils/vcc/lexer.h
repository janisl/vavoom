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
//	EToken
//
//	Token types.
//
//==========================================================================

enum EToken
{
	TK_NoToken,
	TK_EOF,				//	Reached end of file
	TK_Identifier, 		//	Identifier, value: tk_String
	TK_NameLiteral,		//	Name constant, value: tk_Name
	TK_StringLiteral,	//	String, value: tk_String
	TK_IntLiteral,		//	Integer number, value: tk_Number
	TK_FloatLiteral,	//	Floating number, value: tk_Float

	//	Keywords
	TK_Abstract,
	TK_Array,
	TK_Bool,
	TK_Break,
	TK_Byte,
	TK_Case,
	TK_Class,
	TK_Const,
	TK_Continue,
	TK_Default,
	TK_DefaultProperties,
	TK_Delegate,
	TK_Do,
	TK_Else,
	TK_Enum,
	TK_False,
	TK_Final,
	TK_Float,
	TK_For,
	TK_Foreach,
	TK_Game,
	TK_Get,
	TK_If,
	TK_Import,
	TK_Int,
	TK_Iterator,
	TK_Name,
	TK_Native,
	TK_None,
	TK_Null,
	TK_Optional,
	TK_Out,
	TK_Private,
	TK_ReadOnly,
	TK_Reliable,
	TK_Replication,
	TK_Return,
	TK_Self,
	TK_Set,
	TK_Spawner,
	TK_State,
	TK_States,
	TK_Static,
	TK_String,
	TK_Struct,
	TK_Switch,
	TK_Transient,
	TK_True,
	TK_Unreliable,
	TK_Vector,
	TK_Void,
	TK_While,
	TK_MobjInfo,
	TK_ScriptId,

	//	Punctuation
	TK_VarArgs,
	TK_LShiftAssign,
	TK_RShiftAssign,
	TK_AddAssign,
	TK_MinusAssign,
	TK_MultiplyAssign,
	TK_DivideAssign,
	TK_ModAssign,
	TK_AndAssign,
	TK_OrAssign,
	TK_XOrAssign,
	TK_Equals,
	TK_NotEquals,
	TK_LessEquals,
	TK_GreaterEquals,
	TK_AndLog,
	TK_OrLog,
	TK_LShift,
	TK_RShift,
	TK_Inc,
	TK_Dec,
	TK_Arrow,
	TK_DColon,
	TK_Less,
	TK_Greater,
	TK_Quest,
	TK_And,
	TK_Or,
	TK_XOr,
	TK_Tilde,
	TK_Not,
	TK_Plus,
	TK_Minus,
	TK_Asterisk,
	TK_Slash,
	TK_Percent,
	TK_LParen,
	TK_RParen,
	TK_Dot,
	TK_Comma,
	TK_Semicolon,
	TK_Colon,
	TK_Assign,
	TK_LBracket,
	TK_RBracket,
	TK_LBrace,
	TK_RBrace,
};

//==========================================================================
//
//	TLocation
//
//	Describes location in a source file.
//
//==========================================================================

class TLocation
{
private:
	int		Loc;

	static TArray<VStr>		SourceFiles;

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
	VStr GetSource() const;

	static int AddSourceFile(const VStr&);
	static void ClearSourceFiles();
};

//==========================================================================
//
//	VLexer
//
//	Lexer class.
//
//==========================================================================

class VLexer
{
private:
	enum { EOF_CHARACTER = 127 };
	enum { NON_HEX_DIGIT = 255 };

	enum
	{
		CHR_EOF,
		CHR_Letter,
		CHR_Number,
		CHR_Quote,
		CHR_SingleQuote,
		CHR_Special
	};

	enum
	{
		IF_False,		//	Skipping the content
		IF_True,		//	Parsing the content
		IF_ElseFalse,	//	Else case, skipping content
		IF_ElseTrue,	//	Else case, parsing content
		IF_Skip,		//	Conditon inside curently skipped code
		IF_ElseSkip,	//	Else case inside curently skipped code
	};

	struct VSourceFile
	{
		VSourceFile*	Next;	//	Nesting stack
		VStr			FileName;
		VStr			Path;
		char*			FileStart;
		char*			FilePtr;
		char*			FileEnd;
		char			Chr;
		TLocation		Loc;
		int 			SourceIdx;
		int 			Line;
		bool			IncLineNumber;
		bool			NewLine;
		TArray<int>		IfStates;
		bool			Skipping;
	};

	char			ASCIIToChrCode[256];
	vuint8			ASCIIToHexDigit[256];
	char			TokenStringBuffer[MAX_QUOTED_LENGTH];
	bool			SourceOpen;
	char			Chr;
	TArray<VStr>	Defines;
	TArray<VStr>	IncludePath;
	VSourceFile*	Src;

	void NextChr();
	void SkipWhitespaceAndComments();
	void ProcessPreprocessor();
	void ProcessDefine();
	void ProcessIf(bool);
	void ProcessElse();
	void ProcessEndIf();
	void ProcessInclude();
	void PushSource(TLocation&, const VStr&);
	void PopSource();
	void ProcessNumberToken();
	void ProcessChar();
	void ProcessQuoteToken();
	void ProcessSingleQuoteToken();
	void ProcessLetterToken(bool);
	void ProcessSpecialToken();
	void ProcessFileName();

public:
	EToken				Token;
	TLocation			Location;
	vint32				Number;
	float				Float;
	char*				String;
	VName				Name;
	bool				NewLine;

	static const char*	TokenNames[];

	VLexer();
	~VLexer();
	void AddDefine(const VStr&);
	void AddIncludePath(const VStr&);
	void OpenSource(const VStr&);

	void NextToken();
	bool Check(EToken);
	void Expect(EToken);
	void Expect(EToken, ECompileError);
};
