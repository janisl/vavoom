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
//  TSizeBuf::Alloc
//
//==========================================================================

void TSizeBuf::Alloc(int startsize)
{
	if (startsize < 256)
		startsize = 256;
	Data = (byte*)Z_Malloc(startsize, PU_STATIC, 0);
	MaxSize = startsize;
	CurSize = 0;
	Overflowed = false;
}

//==========================================================================
//
//  TSizeBuf::Free
//
//==========================================================================

void TSizeBuf::Free(void)
{
	Z_Free(Data);
	Data = NULL;
	MaxSize = 0;
	CurSize = 0;
}

//==========================================================================
//
//  TSizeBuf::Clear
//
//==========================================================================

void TSizeBuf::Clear(void)
{
	CurSize = 0;
}

//==========================================================================
//
//  TSizeBuf::GetSpace
//
//==========================================================================

void *TSizeBuf::GetSpace(int length)
{
	void    *data;
	
	if (CurSize + length > MaxSize)
	{
		if (!AllowOverflow)
			Sys_Error("TSizeBuf::GetSpace: overflow without allowoverflow set");
		
		if (length > MaxSize)
			Sys_Error("TSizeBuf::GetSpace: %i is > full buffer size", length);
			
		Overflowed = true;
		con << "TSizeBuf::GetSpace: overflow";
		Clear();
	}

	data = Data + CurSize;
	CurSize += length;
	
	return data;
}

//==========================================================================
//
//  TSizeBuf::Write
//
//==========================================================================

void TSizeBuf::Write(const void *data, int length)
{
	memcpy(GetSpace(length), data, length);
}

//==========================================================================
//
//  TSizeBuf::Print
//
//==========================================================================

void TTextBuf::Print(const char *data)
{
	int             len;

	len = strlen(data) + 1;

	if (!CurSize || Data[CurSize - 1])
	{
		memcpy((byte *)GetSpace(len), data, len); // no trailing 0
	}
	else
	{
		memcpy((byte *)GetSpace(len - 1) - 1, data, len); // write over trailing 0
	}
}

//==========================================================================
//
//  TSizeBuf::PrintInt
//
//==========================================================================

void TTextBuf::PrintInt(int data)
{
	char		buf[16];

	sprintf(buf, "%d", data);
    Print(buf);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
