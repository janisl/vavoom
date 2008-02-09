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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VScriptsParser : public VObject
{
	DECLARE_CLASS(VScriptsParser, VObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VScriptsParser)

	VScriptParser*		Int;

	void Destroy();
	void CheckInt();

	DECLARE_FUNCTION(OpenLumpName)
	DECLARE_FUNCTION(get_String)
	DECLARE_FUNCTION(get_Number)
	DECLARE_FUNCTION(get_Float)
	DECLARE_FUNCTION(SetCMode)
	DECLARE_FUNCTION(AtEnd)
	DECLARE_FUNCTION(GetString)
	DECLARE_FUNCTION(ExpectString)
	DECLARE_FUNCTION(Check)
	DECLARE_FUNCTION(Expect)
	DECLARE_FUNCTION(CheckNumber)
	DECLARE_FUNCTION(ExpectNumber)
	DECLARE_FUNCTION(CheckFloat)
	DECLARE_FUNCTION(ExpectFloat)
	DECLARE_FUNCTION(UnGet)
	DECLARE_FUNCTION(ScriptError)
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, ScriptsParser)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VScriptParser::VScriptParser
//
//==========================================================================

VScriptParser::VScriptParser(const VStr& name, VStream* Strm)
: Line(1)
, End(false)
, Crossed(false)
, QuotedString(false)
, ScriptName(name)
, AlreadyGot(false)
, CMode(false)
, Escape(true)
, SrcIdx(-1)
{
	guard(VScriptParser::VScriptParser);
	ScriptSize = Strm->TotalSize();
	ScriptBuffer = new char[ScriptSize + 1];
	Strm->Serialise(ScriptBuffer, ScriptSize);
	ScriptBuffer[ScriptSize] = 0;
	delete Strm;
	ScriptPtr = ScriptBuffer;
	ScriptEndPtr = ScriptPtr + ScriptSize;

	//	Skip garbage some editors add in the begining of UTF-8 files.
	if ((vuint8)ScriptPtr[0] == 0xef && (vuint8)ScriptPtr[1] == 0xbb &&
		(vuint8)ScriptPtr[2] == 0xbf)
	{
		ScriptPtr += 3;
	}
	unguard;
}

//==========================================================================
//
//	VScriptParser::~VScriptParser
//
//==========================================================================

VScriptParser::~VScriptParser()
{
	guard(VScriptParser::~VScriptParser);
	delete[] ScriptBuffer;
	unguard;
}

//==========================================================================
//
//	VScriptParser::SetCMode
//
//==========================================================================

void VScriptParser::SetCMode(bool val)
{
	CMode = val;
}

//==========================================================================
//
//	VScriptParser::SetEscape
//
//==========================================================================

void VScriptParser::SetEscape(bool val)
{
	Escape = val;
}

//==========================================================================
//
//	VScriptParser::AtEnd
//
//==========================================================================

bool VScriptParser::AtEnd()
{
	guard(VScriptParser::AtEnd);
	if (GetString())
	{
		UnGet();
		return false;
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VScriptParser::GetString
//
//==========================================================================

bool VScriptParser::GetString()
{
	guard(VScriptParser::GetString);
	//	Check if we already have a token available.
	if (AlreadyGot)
	{
		AlreadyGot = false;
		return true;
	}

	//	Check for end of script.
	if (ScriptPtr >= ScriptEndPtr)
	{
		End = true;
		return false;
	}

	Crossed = false;
	QuotedString = false;
	bool foundToken = false;
	while (foundToken == false)
	{
		//	Skip whitespace.
		while (*ScriptPtr <= 32)
		{
			if (ScriptPtr >= ScriptEndPtr)
			{
				End = true;
				return false;
			}
			//	Check for new-line character.
			if (*ScriptPtr++ == '\n')
			{
				Line++;
				Crossed = true;
			}
		}

		//	Check for end of script.
		if (ScriptPtr >= ScriptEndPtr)
		{
			End = true;
			return false;
		}

		//	Check for coments
		if ((!CMode && *ScriptPtr == ';') ||
			(ScriptPtr[0] == '/' && ScriptPtr[1] == '/'))
		{
			// Skip comment
			while (*ScriptPtr++ != '\n')
			{
				if (ScriptPtr >= ScriptEndPtr)
				{
					End = true;
					return false;
				}
			}
			Line++;
			Crossed = true;
		}
		else if (ScriptPtr[0] == '/' && ScriptPtr[1] == '*')
		{
			// Skip comment
			ScriptPtr += 2;
			while (ScriptPtr[0] != '*' || ScriptPtr[1] != '/')
			{
				if (ScriptPtr >= ScriptEndPtr)
				{
					End = true;
					return false;
				}
				//	Check for new-line character.
				if (*ScriptPtr == '\n')
				{
					Line++;
					Crossed = true;
				}
				ScriptPtr++;
			}
			ScriptPtr += 2;
		}
		else
		{
			// Found a token
			foundToken = true;
		}
	}

	String.Clean();
	if (*ScriptPtr == '\"')
	{
		//	Quoted string
		QuotedString = true;
		ScriptPtr++;
		while (ScriptPtr < ScriptEndPtr && *ScriptPtr != '\"')
		{
			if (Escape && ScriptPtr[0] == '\\' && ScriptPtr[1] == '\"')
			{
				ScriptPtr++;
			}
			else if (ScriptPtr[0] == '\r' && ScriptPtr[1] == '\n')
			{
				//	Convert from DOS format to UNIX format.
				ScriptPtr++;
			}
			if (*ScriptPtr == '\n')
			{
				if (CMode)
				{
					if (!Escape || String.Length() == 0 ||
						String[String.Length() - 1] != '\\')
					{
						Error("Unterminated string constant");
					}
					else
					{
						//	Remove the \ character.
						String = VStr(String, 0, String.Length() - 1);
					}
				}
				Line++;
				Crossed = true;
			}
			String += *ScriptPtr++;
		}
		ScriptPtr++;
	}
	else if (CMode)
	{
		if ((ScriptPtr[0] == '&' && ScriptPtr[1] == '&') ||
			(ScriptPtr[0] == '=' && ScriptPtr[1] == '=') ||
			(ScriptPtr[0] == '|' && ScriptPtr[1] == '|') ||
			(ScriptPtr[0] == '<' && ScriptPtr[1] == '<') ||
			(ScriptPtr[0] == '>' && ScriptPtr[1] == '>') ||
			(ScriptPtr[0] == ':' && ScriptPtr[1] == ':'))
		{
			//	Special double-character token
			String += *ScriptPtr++;
			String += *ScriptPtr++;
		}
		else if (*ScriptPtr == '-' && ((ScriptPtr[1] >= '0' &&
			ScriptPtr[1] <= '9') || (ScriptPtr[1] == '.' &&
			ScriptPtr[2] >= '0' && ScriptPtr[2] <= '9')))
		{
			//	Negative number
			String += *ScriptPtr++;
			while ((vuint8)*ScriptPtr > 32 &&
				!strchr("`~!@#$%^&*(){}[]/=\\?-+|;:<>,\"", *ScriptPtr))
			{
				String += *ScriptPtr++;
				if (ScriptPtr == ScriptEndPtr)
				{
					break;
				}
			}
		}
		else if ((ScriptPtr[0] >= '0' && ScriptPtr[0] <= '9') ||
			(ScriptPtr[0] == '.' && ScriptPtr[1] >= '0' && ScriptPtr[1] <= '9'))
		{
			//	Number
			while (*ScriptPtr > 32 &&
				!strchr("`~!@#$%^&*(){}[]/=\\?-+|;:<>,\"", *ScriptPtr))
			{
				String += *ScriptPtr++;
				if (ScriptPtr == ScriptEndPtr)
				{
					break;
				}
			}
		}
		else if (strchr("`~!@#$%^&*(){}[]/=\\?-+|;:<>,.", *ScriptPtr))
		{
			//	Special single-character token
			String += *ScriptPtr++;
		}
		else
		{
			//	Normal string
			while (*ScriptPtr > 32 &&
				!strchr("`~!@#$%^&*(){}[]/=\\?-+|;:<>,\".", *ScriptPtr))
			{
				String += *ScriptPtr++;
				if (ScriptPtr == ScriptEndPtr)
				{
					break;
				}
			}
		}
	}
	else
	{
		//	Special single-character tokens
		if (strchr("{}|=", *ScriptPtr))
		{
			String += *ScriptPtr++;
		}
		else
		{
			//	Normal string
			while (*ScriptPtr > 32 && !strchr("{}|=;\"", *ScriptPtr) &&
				(ScriptPtr[0] != '/' || ScriptPtr[1] != '/') &&
				(ScriptPtr[0] != '/' || ScriptPtr[1] != '*'))
			{
				String += *ScriptPtr++;
				if (ScriptPtr == ScriptEndPtr)
				{
					break;
				}
			}
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VScriptParser::ExpectString
//
//==========================================================================

void VScriptParser::ExpectString()
{
	guard(VScriptParser::ExpectString);
	if (!GetString())
	{
		Error("Missing string.");
	}
	unguard;
}

//==========================================================================
//
//	VScriptParser::ExpectName8
//
//==========================================================================

void VScriptParser::ExpectName8()
{
	guard(VScriptParser::ExpectName8);
	ExpectString();
	if (String.Length() > 8)
	{
		Error("Name is too long");
	}
	Name8 = VName(*String, VName::AddLower8);
	unguard;
}

//==========================================================================
//
//	VScriptParser::Check
//
//==========================================================================

bool VScriptParser::Check(const char* str)
{
	guard(VScriptParser::Check);
	if (GetString())
	{
		if (!String.ICmp(str))
		{
			return true;
		}
		UnGet();
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VScriptParser::Expect
//
//==========================================================================

void VScriptParser::Expect(const char* name)
{
	guard(VScriptParser::Expect);
	ExpectString();
	if (String.ICmp(name))
	{
		Error(va("Bad syntax, %s expected", name));
	}
	unguard;
}

//==========================================================================
//
//	VScriptParser::CheckIdentifier
//
//==========================================================================

bool VScriptParser::CheckIdentifier()
{
	guard(VScriptParser::CheckIdentifier);
	//	Quted strings are not valid identifiers.
	if (GetString() && !QuotedString)
	{
		if (String.Length() < 1)
		{
			return false;
		}

		//	Identifier must start with a letter or underscore.
		char c = String[0];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
		{
			return false;
		}

		//	It must be followed by letters, numbers and underscores.
		for (size_t i = 1; i < String.Length(); i++)
		{
			c = String[i];
			if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') || c == '_'))
			{
				return false;
			}
		}
		return true;
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VScriptParser::ExpectIdentifier
//
//==========================================================================

void VScriptParser::ExpectIdentifier()
{
	guard(VScriptParser::ExpectIdentifier);
	if (!CheckIdentifier())
	{
		Error(va("Identifier expected, got '%s'", *String));
	}
	unguard;
}

//==========================================================================
//
//	VScriptParser::CheckNumber
//
//==========================================================================

bool VScriptParser::CheckNumber()
{
	guard(VScriptParser::CheckNumber);
	if (GetString())
	{
		char* stopper;
		Number = strtol(*String, &stopper, 0);
		if (*stopper == 0)
		{
			return true;
		}
		UnGet();
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VScriptParser::ExpectNumber
//
//==========================================================================

void VScriptParser::ExpectNumber()
{
	guard(VScriptParser::ExpectNumber);
	if (GetString())
	{
		char* stopper;
		Number = strtol(*String, &stopper, 0);
		if (*stopper != 0)
		{
			Error(va("Bad numeric constant \"%s\".\n", *String));
		}
	}
	else
	{
		Error("Missing integer.");
	}
	unguard;
}

//==========================================================================
//
//	VScriptParser::CheckFloat
//
//==========================================================================

bool VScriptParser::CheckFloat()
{
	guard(VScriptParser::CheckFloat);
	if (GetString())
	{
		char* stopper;
		Float = strtod(*String, &stopper);
		if (*stopper == 0)
		{
			return true;
		}
		UnGet();
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VScriptParser::ExpectFloat
//
//==========================================================================

void VScriptParser::ExpectFloat()
{
	guard(VScriptParser::ExpectFloat);
	if (GetString())
	{
		char* stopper;
		Float = strtod(*String, &stopper);
		if (*stopper != 0)
		{
			Error(va("Bad floating point constant \"%s\"", *String));
		}
	}
	else
	{
		Error("Missing float.");
	}
	unguard;
}

//==========================================================================
//
//	VScriptParser::UnGet
//
//	Assumes there is a valid string in sc_String.
//
//==========================================================================

void VScriptParser::UnGet()
{
	guard(VScriptParser::UnGet);
	AlreadyGot = true;
	unguard;
}

//==========================================================================
//
//	VScriptParser::Error
//
//==========================================================================

void VScriptParser::Error(const char* message)
{
	guard(VScriptParser::Error)
	const char* Msg = message ? message : "Bad syntax.";
	Sys_Error("Script error, \"%s\" line %d: %s", *ScriptName, Line, Msg);
	unguard;
}

//==========================================================================
//
//	VScriptParser::GetLoc
//
//==========================================================================

TLocation VScriptParser::GetLoc()
{
	guardSlow(VScriptParser::GetLoc);
	if (SrcIdx == -1)
	{
		SrcIdx = TLocation::AddSourceFile(ScriptName);
	}
	return TLocation(SrcIdx, Line);
	unguardSlow;
}

//==========================================================================
//
//	VScriptsParser::Destroy
//
//==========================================================================

void VScriptsParser::Destroy()
{
	guard(VScriptsParser::Destroy);
	if (Int)
	{
		delete Int;
		Int = NULL;
	}
	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	VScriptsParser::CheckInt
//
//==========================================================================

void VScriptsParser::CheckInt()
{
	guard(VScriptsParser::CheckInt);
	if (!Int)
	{
		Sys_Error("No script currently open");
	}
	unguard;
}

//==========================================================================
//
//	VScriptsParser natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VScriptsParser, OpenLumpName)
{
	P_GET_NAME(Name);
	P_GET_SELF;
	if (Self->Int)
	{
		delete Self->Int;
		Self->Int = NULL;
	}
	Self->Int = new VScriptParser(*Name, W_CreateLumpReaderName(Name));
}

IMPLEMENT_FUNCTION(VScriptsParser, get_String)
{
	P_GET_SELF;
	Self->CheckInt();
	RET_STR(Self->Int->String);
}

IMPLEMENT_FUNCTION(VScriptsParser, get_Number)
{
	P_GET_SELF;
	Self->CheckInt();
	RET_INT(Self->Int->Number);
}

IMPLEMENT_FUNCTION(VScriptsParser, get_Float)
{
	P_GET_SELF;
	Self->CheckInt();
	RET_FLOAT(Self->Int->Float);
}

IMPLEMENT_FUNCTION(VScriptsParser, SetCMode)
{
	P_GET_BOOL(On);
	P_GET_SELF;
	Self->CheckInt();
	Self->Int->SetCMode(On);
}

IMPLEMENT_FUNCTION(VScriptsParser, AtEnd)
{
	P_GET_SELF;
	Self->CheckInt();
	RET_BOOL(Self->Int->AtEnd());
}

IMPLEMENT_FUNCTION(VScriptsParser, GetString)
{
	P_GET_SELF;
	Self->CheckInt();
	RET_BOOL(Self->Int->GetString());
}

IMPLEMENT_FUNCTION(VScriptsParser, ExpectString)
{
	P_GET_SELF;
	Self->CheckInt();
	Self->Int->ExpectString();
}

IMPLEMENT_FUNCTION(VScriptsParser, Check)
{
	P_GET_STR(Text);
	P_GET_SELF;
	Self->CheckInt();
	RET_BOOL(Self->Int->Check(*Text));
}

IMPLEMENT_FUNCTION(VScriptsParser, Expect)
{
	P_GET_STR(Text);
	P_GET_SELF;
	Self->CheckInt();
	Self->Int->Expect(*Text);
}

IMPLEMENT_FUNCTION(VScriptsParser, CheckNumber)
{
	P_GET_SELF;
	Self->CheckInt();
	RET_BOOL(Self->Int->CheckNumber());
}

IMPLEMENT_FUNCTION(VScriptsParser, ExpectNumber)
{
	P_GET_SELF;
	Self->CheckInt();
	Self->Int->ExpectNumber();
}

IMPLEMENT_FUNCTION(VScriptsParser, CheckFloat)
{
	P_GET_SELF;
	Self->CheckInt();
	RET_BOOL(Self->Int->CheckFloat());
}

IMPLEMENT_FUNCTION(VScriptsParser, ExpectFloat)
{
	P_GET_SELF;
	Self->CheckInt();
	Self->Int->ExpectFloat();
}

IMPLEMENT_FUNCTION(VScriptsParser, UnGet)
{
	P_GET_SELF;
	Self->CheckInt();
	Self->Int->UnGet();
}

IMPLEMENT_FUNCTION(VScriptsParser, ScriptError)
{
	VStr Msg = PF_FormatString();
	P_GET_SELF;
	Self->CheckInt();
	Self->Int->Error(*Msg);
}
