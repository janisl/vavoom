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
#include "cl_local.h"

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
//  TILine::Init
//
//==========================================================================

void TILine::Init(void)
{
    len = 0;
    Data[0] = 0;
}

//==========================================================================
//
//  TILine::AddChar
//
//==========================================================================

void TILine::AddChar(char ch)
{
    if (len < MAX_ILINE_LENGTH)
    {
		Data[len++] = ch;
		Data[len] = 0;
    }
}

//==========================================================================
//
//  TILine::DelChar
//
//==========================================================================

void TILine::DelChar(void)
{
    if (len)
	{
		Data[--len] = 0;
	}
}

//==========================================================================
//
//  TILine::Key
//
// 	Wrapper function for handling general keyed input.
//	Returns true if it ate the key
//
//==========================================================================

bool TILine::Key(byte ch)
{
	if (ch >= ' ' && ch < 128)
	{
		ch = GInput->TranslateKey(ch);
		AddChar((char)ch);
	}
	else if (ch == K_BACKSPACE)
		DelChar();
	else if (ch != K_ENTER && ch != K_PADENTER)
		return false; // did not eat key

	return true; // ate the key
}
