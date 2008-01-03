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

struct VLanguage::VLangEntry
{
	vint32			PassNum;
	VStr			Value;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VLanguage		GLanguage;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLanguage::VLanguage
//
//==========================================================================

VLanguage::VLanguage()
: Table(NULL)
{
}

//==========================================================================
//
//	VLanguage::~VLanguage
//
//==========================================================================

VLanguage::~VLanguage()
{
	FreeData();
}

//==========================================================================
//
//	VLanguage::FreeData
//
//==========================================================================

void VLanguage::FreeData()
{
	guard(VLanguage::FreeData);
	if (Table)
	{
		delete Table;
		Table = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VLanguage::FreeNonDehackedStrings
//
//==========================================================================

void VLanguage::FreeNonDehackedStrings()
{
	guard(VLanguage::FreeNonDehackedStrings);
	for (TMap<VName, VLangEntry>::TIterator It(*Table); It; ++It)
		if (It.GetValue().PassNum != 0)
			It.RemoveCurrent();
	unguard;
}

//==========================================================================
//
//	VLanguage::LoadStrings
//
//==========================================================================

void VLanguage::LoadStrings(const char* LangId)
{
	guard(VLanguage::LoadStrings);
	if (!Table)
		Table = new TMap<VName, VLangEntry>();

	FreeNonDehackedStrings();

	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_language)
		{
			int j = 1;
			if (VStr::Cmp(LangId, "**"))
			{
				ParseLanguageScript(Lump, "*", true, j++);
				ParseLanguageScript(Lump, LangId, true, j++);
				ParseLanguageScript(Lump, LangId, false, j++);
			}
			ParseLanguageScript(Lump, "**", true, j++);
		}
	}
	unguard;
}

//==========================================================================
//
//	VLanguage::ParseLanguageScript
//
//==========================================================================

void VLanguage::ParseLanguageScript(vint32 Lump, const char* InCode,
	bool ExactMatch, vint32 PassNum)
{
	guard(VLanguage::ParseLanguageScript);
	char Code[4];
	Code[0] = VStr::ToLower(InCode[0]);
	Code[1] = VStr::ToLower(InCode[1]);
	Code[2] = ExactMatch ? VStr::ToLower(InCode[2]) : 0;
	Code[3] = 0;

	VScriptParser* sc = new VScriptParser(*W_LumpName(Lump),
		W_CreateLumpReaderNum(Lump));
	sc->SetCMode(true);

	bool GotLanguageCode = false;
	bool Skip = false;

	while (!sc->AtEnd())
	{
		if (sc->Check("["))
		{
			//	Language identifiers.
			Skip = true;
			while (!sc->Check("]"))
			{
				sc->ExpectString();
				size_t Len = sc->String.Length();
				char CurCode[4];
				if (Len != 2 && Len != 3)
				{
					if (Len == 1 && sc->String[0] == '*')
					{
						CurCode[0] = '*';
						CurCode[1] = 0;
						CurCode[2] = 0;
					}
					else if (Len == 7 && !sc->String.ICmp("default"))
					{
						CurCode[0] = '*';
						CurCode[1] = '*';
						CurCode[2] = 0;
					}
					else
					{
						sc->Error(va("Language code must be 2 or 3 "
							"characters long, %s is %d characters long",
							*sc->String, Len));
						//	Shut up compiler
						CurCode[0] = 0;
						CurCode[1] = 0;
						CurCode[2] = 0;
					}
				}
				else
				{
					CurCode[0] = VStr::ToLower(sc->String[0]);
					CurCode[1] = VStr::ToLower(sc->String[1]);
					CurCode[2] = ExactMatch ? VStr::ToLower(sc->String[2]) : 0;
					CurCode[3] = 0;
				}
				if (Code[0] == CurCode[0] && Code[1] == CurCode[1] &&
					Code[2] == CurCode[2])
				{
					Skip = false;
				}
				GotLanguageCode = true;
			}
		}
		else
		{
			if (!GotLanguageCode)
			{
				sc->Error("Found a string without language specified");
			}

			//	Parse string definitions.
			if (Skip)
			{
				//	We are skipping this language.
				sc->ExpectString();
				sc->Expect("=");
				sc->ExpectString();
				while (!sc->Check(";"))
				{
					sc->ExpectString();
				}
				continue;
			}

			sc->ExpectString();
			VName Key = *sc->String.ToLower();
			sc->Expect("=");
			sc->ExpectString();
			VStr Value = HandleEscapes(sc->String);
			while (!sc->Check(";"))
			{
				sc->ExpectString();
				Value += HandleEscapes(sc->String);
			}

			//	Check for replacement.
			VLangEntry* Found = Table->Find(Key);
			if (!Found || Found->PassNum >= PassNum)
			{
				VLangEntry Entry;
				Entry.Value = Value;
				Entry.PassNum = PassNum;
				Table->Set(Key, Entry);
			}
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	VLanguage::HandleEscapes
//
//==========================================================================

VStr VLanguage::HandleEscapes(VStr Src)
{
	guard(VLanguage::HandleEscapes);
	VStr Ret;
	for (size_t i = 0; i < Src.Length(); i++)
	{
		char c = Src[i];
		if (c == '\\')
		{
			i++;
			c = Src[i];
			if (c == 'n')
				c = '\n';
			else if (c == 'r')
				c = '\r';
			else if (c == 't')
				c = '\t';
			else if (c == 'c')
				c = -127;
			else if (c == '\n')
				continue;
		}
		Ret += c;
	}
	return Ret;
	unguard;
}

//==========================================================================
//
//	VLanguage::Find
//
//==========================================================================

VStr VLanguage::Find(VName Key) const
{
	guard(VLanguage::Find);
	VLangEntry* Found = Table->Find(Key);
	return Found ? Found->Value : VStr();
	unguard;
}

//==========================================================================
//
//	VLanguage::operator[]
//
//==========================================================================

VStr VLanguage::operator[](VName Key) const
{
	guard(VLanguage::operator[]);
	VLangEntry* Found = Table->Find(Key);
	return Found ? Found->Value : VStr(Key);
	unguard;
}

//==========================================================================
//
//	VLanguage::GetStringId
//
//==========================================================================

VName VLanguage::GetStringId(const VStr& Str)
{
	guard(VLanguage::GetStringId);
	for (TMap<VName, VLangEntry>::TIterator It(*Table); It; ++It)
	{
		if (It.GetValue().Value == Str)
		{
			return It.GetKey();
		}
	}
	return NAME_None;
	unguard;
}

//==========================================================================
//
//	VLanguage::ReplaceString
//
//==========================================================================

void VLanguage::ReplaceString(VName Key, const VStr& Value)
{
	guard(VLanguage::ReplaceString);
	VLangEntry Entry;
	Entry.Value = Value;
	Entry.PassNum = 0;
	Table->Set(Key, Entry);
	unguard;
}
