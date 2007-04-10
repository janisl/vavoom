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
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	CMD_Side,
	CMD_MAX
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLevelChannel::VLevelChannel
//
//==========================================================================

VLevelChannel::VLevelChannel(VNetConnection* AConnection, vint32 AIndex,
	vuint8 AOpenedLocally)
: VChannel(AConnection, CHANNEL_Player, AIndex, AOpenedLocally)
, Level(NULL)
, Sides(NULL)
{
}

//==========================================================================
//
//	VLevelChannel::~VLevelChannel
//
//==========================================================================

VLevelChannel::~VLevelChannel()
{
	SetLevel(NULL);
}

//==========================================================================
//
//	VLevelChannel::SetLevel
//
//==========================================================================

void VLevelChannel::SetLevel(VLevel* ALevel)
{
	guard(VLevelChannel::SetLevel);
	if (Level)
	{
		delete[] Sides;
		Sides = NULL;
	}

	Level = ALevel;

	if (Level)
	{
		Sides = new rep_side_t[Level->NumSides];
		memcpy(Sides, Level->BaseSides, sizeof(rep_side_t) * Level->NumSides);
	}
	unguard;
}

//==========================================================================
//
//	VLevelChannel::Update
//
//==========================================================================

void VLevelChannel::Update()
{
	guard(VLevelChannel::Update);
	VMessageOut Msg(this);
	Msg.bReliable = true;

	for (int i = 0; i < Level->NumSides; i++)
	{
		side_t* Side = &GLevel->Sides[i];
		if (!SV_SecCheckFatPVS(Side->sector))
			continue;

		rep_side_t* RepSide = &Sides[i];
		if (Side->toptexture == RepSide->toptexture &&
			Side->bottomtexture == RepSide->bottomtexture &&
			Side->midtexture == RepSide->midtexture &&
			Side->textureoffset == RepSide->textureoffset &&
			Side->rowoffset == RepSide->rowoffset)
			continue;

		Msg.WriteInt(CMD_Side, CMD_MAX);
		Msg.WriteInt(i, Level->NumSides);
		Msg.WriteBit(Side->toptexture != RepSide->toptexture);
		if (Side->toptexture != RepSide->toptexture)
		{
			Msg.WriteInt(Side->toptexture, MAX_VUINT16);
			RepSide->toptexture = Side->toptexture;
		}
		Msg.WriteBit(Side->bottomtexture != RepSide->bottomtexture);
		if (Side->bottomtexture != RepSide->bottomtexture)
		{
			Msg.WriteInt(Side->bottomtexture, MAX_VUINT16);
			RepSide->bottomtexture = Side->bottomtexture;
		}
		Msg.WriteBit(Side->midtexture != RepSide->midtexture);
		if (Side->midtexture != RepSide->midtexture)
		{
			Msg.WriteInt(Side->midtexture, MAX_VUINT16);
			RepSide->midtexture = Side->midtexture;
		}
		Msg.WriteBit(Side->textureoffset != RepSide->textureoffset);
		if (Side->textureoffset != RepSide->textureoffset)
		{
			Msg << Side->textureoffset;
			RepSide->textureoffset = Side->textureoffset;
		}
		Msg.WriteBit(Side->rowoffset != RepSide->rowoffset);
		if (Side->rowoffset != RepSide->rowoffset)
		{
			Msg << Side->rowoffset;
			RepSide->rowoffset = Side->rowoffset;
		}
	}

	if (Msg.GetNumBits())
	{
		SendMessage(&Msg);
	}
	unguard;
}

//==========================================================================
//
//	VLevelChannel::ParsePacket
//
//==========================================================================

void VLevelChannel::ParsePacket(VMessageIn& Msg)
{
	guard(VLevelChannel::ParsePacket);
	while (!Msg.AtEnd())
	{
		int Cmd = Msg.ReadInt(CMD_MAX);
		switch (Cmd)
		{
		case CMD_Side:
			{
				side_t* Side = &Level->Sides[Msg.ReadInt(Level->NumSides)];
				if (Msg.ReadBit())
					Side->toptexture = Msg.ReadInt(MAX_VUINT16);
				if (Msg.ReadBit())
					Side->bottomtexture = Msg.ReadInt(MAX_VUINT16);
				if (Msg.ReadBit())
					Side->midtexture = Msg.ReadInt(MAX_VUINT16);
				if (Msg.ReadBit())
					Msg << Side->textureoffset;
				if (Msg.ReadBit())
					Msg << Side->rowoffset;
			}
			break;
		}
	}
	unguard;
}
