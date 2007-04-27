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
//  VMessageIn::VMessageIn
//
//==========================================================================

VMessageIn::VMessageIn(vuint8* Src, vint32 Length)
: VBitStreamReader(Src, Length)
, Next(NULL)
, ChanType(0)
, ChanIndex(0)
, bReliable(false)
, bOpen(false)
, bClose(false)
, Sequence(0)
{
}

//==========================================================================
//
//  VMessageOut::VMessageOut
//
//==========================================================================

VMessageOut::VMessageOut(VChannel* AChannel)
: VBitStreamWriter(OUT_MESSAGE_SIZE)
, Next(NULL)
, ChanType(AChannel->Type)
, ChanIndex(AChannel->Index)
, bReliable(false)
, bOpen(false)
, bClose(false)
, bReceivedAck(false)
, Sequence(0)
, Time(0)
, PacketId(0)
{
}
