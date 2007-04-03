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
#include "network.h"

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
//  VMessageOut::VMessageOut
//
//==========================================================================

VMessageOut::VMessageOut(VChannel* AChannel)
: VBitStreamWriter(OUT_MESSAGE_SIZE)
, ChanType(AChannel->Type)
, ChanIndex(AChannel->Index)
, AllowOverflow(true)
, bReliable(false)
, bOpen(false)
, bClose(false)
, bReceivedAck(false)
{
}

//==========================================================================
//
//  VMessageOut::Clear
//
//==========================================================================

void VMessageOut::Clear()
{
	Pos = 0;
	memset(Data.Ptr(), 0, (Max + 7) >> 3);
}

//==========================================================================
//
//	VMessageOut::operator << VMessageOut
//
//==========================================================================

VMessageOut& VMessageOut::operator << (VMessageOut &msg)
{
	guard(VMessageOut::operator << VMessageOut);
	SerialiseBits(msg.GetData(), msg.GetNumBits());
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageOut::SerialiseBits
//
//==========================================================================

void VMessageOut::SerialiseBits(void* Src, vint32 Length)
{
	guard(VMessageOut::SerialiseBits);
	VBitStreamWriter::SerialiseBits(Src, Length);
	if (bError)
	{
		if (!AllowOverflow)
			Sys_Error("VMessageOut::SerialiseBits: overflow without allowoverflow set");

		if (Length > Max)
			Sys_Error("VMessageOut::SerialiseBits: %i is > full buffer size", Length);

		GCon->Log("VMessageOut::SerialiseBits: overflow");
		Clear();
	}
	unguard;
}
