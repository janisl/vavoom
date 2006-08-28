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

#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TModifiers::Parse
//
//	Parse supported modifiers.
//
//==========================================================================

int TModifiers::Parse(VLexer& Lex)
{
	int Modifiers = 0;
	bool done = false;
	do
	{
		if (Lex.Check(TK_Native))
		{
			Modifiers |= Native;
		}
		else if (Lex.Check(TK_Static))
		{
			Modifiers |= Static;
		}
		else if (Lex.Check(TK_Abstract))
		{
			Modifiers |= Abstract;
		}
		else if (Lex.Check(TK_Private))
		{
			Modifiers |= Private;
		}
		else if (Lex.Check(TK_ReadOnly))
		{
			Modifiers |= ReadOnly;
		}
		else if (Lex.Check(TK_Transient))
		{
			Modifiers |= Transient;
		}
		else if (Lex.Check(TK_Final))
		{
			Modifiers |= Final;
		}
		else
		{
			done = true;
		}
	} while (!done);
	return Modifiers;
}

//==========================================================================
//
//	TModifiers::Name
//
//	Return string representation of a modifier.
//
//==========================================================================

const char* TModifiers::Name(int Modifier)
{
	switch (Modifier)
	{
	case Native:	return "native";
	case Static:	return "static";
	case Abstract:	return "abstract";
	case Private:	return "private";
	case ReadOnly:	return "readonly";
	case Transient:	return "transient";
	case Final:		return "final";
	}
	return "";
}

//==========================================================================
//
//	TModifiers::Check
//
//	Verify that modifiers are valid in current context.
//
//==========================================================================

int TModifiers::Check(int Modifers, int Allowed, TLocation l)
{
	int Bad = Modifers & ~Allowed;
	if (Bad)
	{
		for (int i = 0; i < 32; i++)
			if (Bad & (1 << i))
				ParseError(l, "%s modifier is not allowed", Name(1 << i));
		return Modifers & Allowed;
	}
	return Modifers;
}

//==========================================================================
//
//	TModifiers::MethodAttr
//
//	Convert modifiers to method attributes.
//
//==========================================================================

int TModifiers::MethodAttr(int Modifiers)
{
	int Attributes = 0;
	if (Modifiers & Native)
		Attributes |= FUNC_Native;
	if (Modifiers & Static)
		Attributes |= FUNC_Static;
	if (Modifiers & Final)
		Attributes |= FUNC_Final;
	return Attributes;
}

//==========================================================================
//
//	TModifiers::ClassAttr
//
//	Convert modifiers to class attributes.
//
//==========================================================================

int TModifiers::ClassAttr(int Modifiers)
{
	int Attributes = 0;
	if (Modifiers & Native)
		Attributes |= CLASS_Native;
	if (Modifiers & Abstract)
		Attributes |= CLASS_Abstract;
	return Attributes;
}

//==========================================================================
//
//	TModifiers::FieldAttr
//
//	Convert modifiers to field attributes.
//
//==========================================================================

int TModifiers::FieldAttr(int Modifiers)
{
	int Attributes = 0;
	if (Modifiers & Native)
		Attributes |= FIELD_Native;
	if (Modifiers & Transient)
		Attributes |= FIELD_Transient;
	if (Modifiers & Private)
		Attributes |= FIELD_Private;
	if (Modifiers & ReadOnly)
		Attributes |= FIELD_ReadOnly;
	return Attributes;
}
