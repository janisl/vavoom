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
	CMD_Sector,
	CMD_PolyObj,
	CMD_StaticLight,

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
, Sectors(NULL)
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
		delete[] Sectors;
		delete[] PolyObjs;
		Sides = NULL;
		Sectors = NULL;
		PolyObjs = NULL;
	}

	Level = ALevel;

	if (Level)
	{
		Sides = new rep_side_t[Level->NumSides];
		memcpy(Sides, Level->BaseSides, sizeof(rep_side_t) * Level->NumSides);
		Sectors = new rep_sector_t[Level->NumSectors];
		memcpy(Sectors, Level->BaseSectors, sizeof(rep_sector_t) * Level->NumSectors);
		PolyObjs = new rep_polyobj_t[Level->NumPolyObjs];
		memcpy(PolyObjs, Level->BasePolyObjs, sizeof(rep_polyobj_t) * Level->NumPolyObjs);
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
		side_t* Side = &Level->Sides[i];
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

	for (int i = 0; i < Level->NumSectors; i++)
	{
		sector_t* Sec = &Level->Sectors[i];
		if (!SV_SecCheckFatPVS(Sec) && !(Sec->SectorFlags & sector_t::SF_ExtrafloorSource))
			continue;

		rep_sector_t* RepSec = &Sectors[i];
		bool FloorChanged = fabs(RepSec->floor_dist - Sec->floor.dist) >= 1.0 ||
			RepSec->floor_xoffs != Sec->floor.xoffs ||
			RepSec->floor_yoffs != Sec->floor.yoffs;
		bool CeilChanged = fabs(RepSec->ceil_dist - Sec->ceiling.dist) >= 1.0 ||
			RepSec->ceil_xoffs != Sec->ceiling.xoffs ||
			RepSec->ceil_yoffs != Sec->ceiling.yoffs;
		bool LightChanged = abs(RepSec->lightlevel - Sec->params.lightlevel) >= 4;
		if (RepSec->floor_pic == Sec->floor.pic &&
			RepSec->ceil_pic == Sec->ceiling.pic &&
			!FloorChanged && !CeilChanged && !LightChanged)
			continue;

		Msg.WriteInt(CMD_Sector, CMD_MAX);
		Msg.WriteInt(i, Level->NumSectors);
		Msg.WriteBit(RepSec->floor_pic != Sec->floor.pic);
		if (RepSec->floor_pic != Sec->floor.pic)
		{
			Msg.WriteInt(Sec->floor.pic, MAX_VUINT16);
		}
		Msg.WriteBit(RepSec->ceil_pic != Sec->ceiling.pic);
		if (RepSec->ceil_pic != Sec->ceiling.pic)
		{
			Msg.WriteInt(Sec->ceiling.pic, MAX_VUINT16);
		}
		Msg.WriteBit(FloorChanged);
		if (FloorChanged)
		{
			Msg.WriteBit(fabs(RepSec->floor_dist - Sec->floor.dist) >= 1.0);
			if (fabs(RepSec->floor_dist - Sec->floor.dist) >= 1.0)
				Msg << Sec->floor.dist;
			Msg.WriteBit(RepSec->floor_xoffs != Sec->floor.xoffs);
			if (RepSec->floor_xoffs != Sec->floor.xoffs)
				Msg.WriteInt((vint32)Sec->floor.xoffs & 63, 64);
			Msg.WriteBit(RepSec->floor_yoffs != Sec->floor.yoffs);
			if (RepSec->floor_yoffs != Sec->floor.yoffs)
				Msg.WriteInt((vint32)Sec->floor.yoffs & 63, 64);
		}
		Msg.WriteBit(CeilChanged);
		if (CeilChanged)
		{
			Msg.WriteBit(fabs(RepSec->ceil_dist - Sec->ceiling.dist) >= 1.0);
			if (fabs(RepSec->ceil_dist - Sec->ceiling.dist) >= 1.0)
				Msg << Sec->ceiling.dist;
			Msg.WriteBit(RepSec->ceil_xoffs != Sec->ceiling.xoffs);
			if (RepSec->ceil_xoffs != Sec->ceiling.xoffs)
				Msg.WriteInt((vint32)Sec->ceiling.xoffs & 63, 64);
			Msg.WriteBit(RepSec->ceil_yoffs != Sec->ceiling.yoffs);
			if (RepSec->ceil_yoffs != Sec->ceiling.yoffs)
				Msg.WriteInt((vint32)Sec->ceiling.yoffs & 63, 64);
		}
		Msg.WriteBit(LightChanged);
		if (LightChanged)
		{
			Msg.WriteInt(Sec->params.lightlevel >> 2, 256);
		}

		RepSec->floor_pic = Sec->floor.pic;
		RepSec->floor_dist = Sec->floor.dist;
		RepSec->floor_xoffs = Sec->floor.xoffs;
		RepSec->floor_yoffs = Sec->floor.yoffs;
		RepSec->ceil_pic = Sec->ceiling.pic;
		RepSec->ceil_dist = Sec->ceiling.dist;
		RepSec->ceil_xoffs = Sec->ceiling.xoffs;
		RepSec->ceil_yoffs = Sec->ceiling.yoffs;
		RepSec->lightlevel = Sec->params.lightlevel;
	}

	for (int i = 0; i < Level->NumPolyObjs; i++)
	{
		polyobj_t* Po = &Level->PolyObjs[i];
		if (!SV_CheckFatPVS(Po->subsector))
			continue;

		rep_polyobj_t* RepPo = &PolyObjs[i];
		if (RepPo->startSpot.x == Po->startSpot.x &&
			RepPo->startSpot.y == Po->startSpot.y &&
			RepPo->angle == Po->angle)
			continue;

		Msg.WriteInt(CMD_PolyObj, CMD_MAX);
		Msg.WriteInt(i, Level->NumPolyObjs);
		Msg.WriteBit(RepPo->startSpot.x != Po->startSpot.x);
		if (RepPo->startSpot.x != Po->startSpot.x)
		{
			Msg << Po->startSpot.x;
		}
		Msg.WriteBit(RepPo->startSpot.y != Po->startSpot.y);
		if (RepPo->startSpot.y != Po->startSpot.y)
		{
			Msg << Po->startSpot.y;
		}
		Msg.WriteBit(RepPo->angle != Po->angle);
		if (RepPo->angle != Po->angle)
		{
			Msg << Po->angle;
		}

		RepPo->startSpot = Po->startSpot;
		RepPo->angle = Po->angle;
	}

	if (Msg.GetNumBits())
	{
		SendMessage(&Msg);
	}
	unguard;
}

//==========================================================================
//
//	VLevelChannel::SendStaticLights
//
//==========================================================================

void VLevelChannel::SendStaticLights()
{
	guard(VLevelChannel::SendStaticLights);
	for (int i = 0; i < Level->NumStaticLights; i++)
	{
		rep_light_t& L = Level->StaticLights[i];
		VMessageOut Msg(this);
		Msg.bReliable = true;
		Msg.WriteInt(CMD_StaticLight, CMD_MAX);
		Msg << L.Origin << L.Radius << L.Colour;
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

		case CMD_Sector:
			{
				sector_t* Sec = &Level->Sectors[Msg.ReadInt(Level->NumSectors)];
				float PrevFloorDist = Sec->floor.dist;
				float PrevCeilDist = Sec->ceiling.dist;
				if (Msg.ReadBit())
					Sec->floor.pic = Msg.ReadInt(MAX_VUINT16);
				if (Msg.ReadBit())
					Sec->ceiling.pic = Msg.ReadInt(MAX_VUINT16);
				if (Msg.ReadBit())
				{
					if (Msg.ReadBit())
						Msg << Sec->floor.dist;
					if (Msg.ReadBit())
						Sec->floor.xoffs = Msg.ReadInt(64);
					if (Msg.ReadBit())
						Sec->floor.yoffs = Msg.ReadInt(64);
				}
				if (Msg.ReadBit())
				{
					if (Msg.ReadBit())
						Msg << Sec->ceiling.dist;
					if (Msg.ReadBit())
						Sec->ceiling.xoffs = Msg.ReadInt(64);
					if (Msg.ReadBit())
						Sec->ceiling.yoffs = Msg.ReadInt(64);
				}
				if (Msg.ReadBit())
				{
					Sec->params.lightlevel = Msg.ReadInt(256) << 2;
				}
				if (PrevFloorDist != Sec->floor.dist ||
					PrevCeilDist != Sec->ceiling.dist)
				{
					CalcSecMinMaxs(Sec);
				}
			}
			break;

		case CMD_PolyObj:
			{
				polyobj_t* Po = &Level->PolyObjs[Msg.ReadInt(Level->NumPolyObjs)];
				TVec Pos = Po->startSpot;
				if (Msg.ReadBit())
					Msg << Pos.x;
				if (Msg.ReadBit())
					Msg << Pos.y;
				if (Pos != Po->startSpot)
				{
					Level->MovePolyobj(Po->tag, Pos.x - Po->startSpot.x,
						Pos.y - Po->startSpot.y);
				}
				if (Msg.ReadBit())
				{
					float a;
					Msg << a;
					Level->RotatePolyobj(Po->tag, a - Po->angle);
				}
			}
			break;

		case CMD_StaticLight:
			{
				TVec		Origin;
				float		Radius;
				vuint32		Colour;
				Msg << Origin << Radius << Colour;
				Level->RenderData->AddStaticLight(Origin, Radius, Colour);
			}
			break;
		}
	}
	unguard;
}
