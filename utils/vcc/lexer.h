//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: vcc.h 1681 2006-08-27 17:53:39Z dj_jl $
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
	TK_Bool,
	TK_Break,
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
	TK_Get,
	TK_If,
	TK_Import,
	TK_Int,
	TK_Name,
	TK_Native,
	TK_None,
	TK_Null,
	TK_Optional,
	TK_Out,
	TK_Private,
	TK_ReadOnly,
	TK_Replication,
	TK_Return,
	TK_Self,
	TK_Set,
	TK_State,
	TK_Static,
	TK_String,
	TK_Struct,
	TK_Switch,
	TK_Transient,
	TK_True,
	TK_Vector,
	TK_Void,
	TK_While,
	TK_States,
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

	static TArray<char*>	SourceFiles;

	friend class VLexer;

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

	char			ASCIIToChrCode[256];
	vuint8			ASCIIToHexDigit[256];
	char			TokenStringBuffer[MAX_QUOTED_LENGTH];
	bool			IncLineNumber;
	bool			NewLine;
	bool			SourceOpen;
	char*			FileStart;
	char*			FilePtr;
	char*			FileEnd;
	char			Chr;
	int 			SourceIdx;
	int 			Line;

	void NextChr();
	void ProcessNumberToken();
	void ProcessChar();
	void ProcessQuoteToken();
	void ProcessSingleQuoteToken();
	void ProcessLetterToken();
	void ProcessSpecialToken();
	void ProcessFileName();
	int AddSourceFile(const char*);

public:
	EToken				Token;
	TLocation			Location;
	vint32				Number;
	float				Float;
	char*				String;
	VName				Name;

	static const char*	TokenNames[];

	VLexer();
	void OpenSource(void*, size_t);
	~VLexer();

	void NextToken();
	bool Check(EToken);
	void Expect(EToken);
	void Expect(EToken, ECompileError);
};
