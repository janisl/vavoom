//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

//  Key shifting
static const char 	shiftxform[] =
{

    0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31,
    ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '<', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    ')', // shift-0
    '!', // shift-1
    '@', // shift-2
    '#', // shift-3
    '$', // shift-4
    '%', // shift-5
    '^', // shift-6
    '&', // shift-7
    '*', // shift-8
    '(', // shift-9
    ':',
    ':', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', // shift-[
    '|', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    '}', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  TILine::Init
//
//==========================================================================

void TILine::Init(void)
{
    lm = 0;
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
    if (len != lm && len)
		Data[--len] = 0;
}

//==========================================================================
//
//  TILine::AddPrefix
//
//==========================================================================

void TILine::AddPrefix(char* str)
{
    while (*str)
		AddChar(*(str++));
    lm = len;
}

//==========================================================================
//
//  TILine::TranslatedKey
//
// 	Wrapper function for handling general keyed input.
//	Returns true if it ate the key
//
//==========================================================================

bool TILine::TranslatedKey(byte ch)
{
    if (ch >= ' ' && ch < 128)
  		AddChar((char)ch);
    else if (ch == K_BACKSPACE)
	    DelChar();
	else if (ch != K_ENTER)
		return false; // did not eat key

    return true; // ate the key
}

//==========================================================================
//
//  TILine::Key
//
//==========================================================================

bool TILine::Key(byte ch)
{
   	if (shiftdown && ch < 128)
		ch = shiftxform[ch];

	return TranslatedKey(ch);
}

