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
#include "cl_local.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	CMD_Side,
	CMD_Sector,
	CMD_PolyObj,
	CMD_StaticLight,
	CMD_NewLevel,
	CMD_PreRender,
	CMD_Line,
	CMD_CamTex,

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
, Lines(NULL)
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
		delete[] Lines;
		delete[] Sides;
		delete[] Sectors;
		delete[] PolyObjs;
		Lines = NULL;
		Sides = NULL;
		Sectors = NULL;
		PolyObjs = NULL;
		CameraTextures.Clear();
	}

	Level = ALevel;

	if (Level)
	{
		Lines = new rep_line_t[Level->NumLines];
		memcpy(Lines, Level->BaseLines, sizeof(rep_line_t) * Level->NumLines);
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
//	VLevelChannel::SendNewLevel
//
//==========================================================================

void VLevelChannel::SendNewLevel()
{
	guard(VLevelChannel::SendNewLevel);
	guardSlow(NewLevel);
	VMessageOut Msg(this);
	Msg.bReliable = true;
	Msg.WriteInt(CMD_NewLevel, CMD_MAX);
	VStr MapName = *Level->MapName;
	Msg << svs.serverinfo << MapName;
	Msg.WriteInt(svs.max_clients, MAXPLAYERS + 1);
	Msg.WriteInt(deathmatch, 256);
	SendMessage(&Msg);
	unguardSlow;

	guardSlow(StaticLights);
	SendStaticLights();
	unguardSlow;

	guardSlow(PreRender);
	VMessageOut Msg(this);
	Msg.bReliable = true;
	Msg.WriteInt(CMD_PreRender, CMD_MAX);
	SendMessage(&Msg);
	unguardSlow;
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

	for (int i = 0; i < Level->NumLines; i++)
	{
		line_t* Line = &Level->Lines[i];
		//if (!Connection->SecCheckFatPVS(Line->sector))
		//	continue;

		rep_line_t* RepLine = &Lines[i];
		if (Line->alpha == RepLine->alpha)
			continue;

		Msg.WriteInt(CMD_Line, CMD_MAX);
		Msg.WriteInt(i, Level->NumLines);
		Msg.WriteBit(Line->alpha != RepLine->alpha);
		if (Line->alpha != RepLine->alpha)
		{
			Msg << Line->alpha;
			RepLine->alpha = Line->alpha;
		}
	}

	for (int i = 0; i < Level->NumSides; i++)
	{
		side_t* Side = &Level->Sides[i];
		if (!Connection->SecCheckFatPVS(Side->sector))
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
		Msg.WriteBit(Side->Flags != RepSide->Flags);
		if (Side->Flags != RepSide->Flags)
		{
			Msg.WriteInt(Side->Flags, 0x000f);
			RepSide->Flags = Side->Flags;
		}
	}

	for (int i = 0; i < Level->NumSectors; i++)
	{
		sector_t* Sec = &Level->Sectors[i];
		if (!Connection->SecCheckFatPVS(Sec) &&
			!(Sec->SectorFlags & sector_t::SF_ExtrafloorSource) &&
			!(Sec->SectorFlags & sector_t::SF_TransferSource))
			continue;

		rep_sector_t* RepSec = &Sectors[i];
		bool FloorChanged = RepSec->floor_dist != Sec->floor.dist ||
			mround(RepSec->floor_xoffs) != mround(Sec->floor.xoffs) ||
			mround(RepSec->floor_yoffs) != mround(Sec->floor.yoffs);
		bool CeilChanged = RepSec->ceil_dist != Sec->ceiling.dist ||
			mround(RepSec->ceil_xoffs) != mround(Sec->ceiling.xoffs) ||
			mround(RepSec->ceil_yoffs) != mround(Sec->ceiling.yoffs);
		bool LightChanged = abs(RepSec->lightlevel - Sec->params.lightlevel) >= 4;
		bool FadeChanged = RepSec->Fade != Sec->params.Fade;
		if (RepSec->floor_pic == Sec->floor.pic &&
			RepSec->ceil_pic == Sec->ceiling.pic &&
			!FloorChanged && !CeilChanged && !LightChanged && !FadeChanged)
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
			Msg.WriteBit(RepSec->floor_dist != Sec->floor.dist);
			if (RepSec->floor_dist != Sec->floor.dist)
				Msg << Sec->floor.dist;
			Msg.WriteBit(mround(RepSec->floor_xoffs) != mround(Sec->floor.xoffs));
			if (mround(RepSec->floor_xoffs) != mround(Sec->floor.xoffs))
				Msg.WriteInt(mround(Sec->floor.xoffs) & 63, 64);
			Msg.WriteBit(mround(RepSec->floor_yoffs) != mround(Sec->floor.yoffs));
			if (mround(RepSec->floor_yoffs) != mround(Sec->floor.yoffs))
				Msg.WriteInt(mround(Sec->floor.yoffs) & 63, 64);
		}
		Msg.WriteBit(CeilChanged);
		if (CeilChanged)
		{
			Msg.WriteBit(RepSec->ceil_dist != Sec->ceiling.dist);
			if (RepSec->ceil_dist != Sec->ceiling.dist)
				Msg << Sec->ceiling.dist;
			Msg.WriteBit(mround(RepSec->ceil_xoffs) != mround(Sec->ceiling.xoffs));
			if (mround(RepSec->ceil_xoffs) != mround(Sec->ceiling.xoffs))
				Msg.WriteInt(mround(Sec->ceiling.xoffs) & 63, 64);
			Msg.WriteBit(mround(RepSec->ceil_yoffs) != mround(Sec->ceiling.yoffs));
			if (mround(RepSec->ceil_yoffs) != mround(Sec->ceiling.yoffs))
				Msg.WriteInt(mround(Sec->ceiling.yoffs) & 63, 64);
		}
		Msg.WriteBit(LightChanged);
		if (LightChanged)
		{
			Msg.WriteInt(Sec->params.lightlevel >> 2, 256);
		}
		Msg.WriteBit(FadeChanged);
		if (FadeChanged)
		{
			Msg << Sec->params.Fade;
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
		RepSec->Fade = Sec->params.Fade;
	}

	for (int i = 0; i < Level->NumPolyObjs; i++)
	{
		polyobj_t* Po = &Level->PolyObjs[i];
		if (!Connection->CheckFatPVS(Po->subsector))
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

	for (int i = 0; i < Level->CameraTextures.Num(); i++)
	{
		//	Grow replication array if needed.
		if (CameraTextures.Num() == i)
		{
			VCameraTextureInfo& C = CameraTextures.Alloc();
			C.Camera = NULL;
			C.TexNum = -1;
			C.FOV = 0;
		}

		VCameraTextureInfo& Cam = Level->CameraTextures[i];
		VCameraTextureInfo& RepCam = CameraTextures[i];
		VEntity* CamEnt = Cam.Camera;
		if (CamEnt && !Connection->ObjMap->CanSerialiseObject(CamEnt))
		{
dprintf("Can't serialise\n");
			CamEnt = NULL;
		}
		if (CamEnt == RepCam.Camera && Cam.TexNum == RepCam.TexNum &&
			Cam.FOV == RepCam.FOV)
		{
			continue;
		}

dprintf("Camera %d %p\n", i, CamEnt);
		//	Send message
		Msg.WriteInt(CMD_CamTex, CMD_MAX);
		Msg.WriteInt(i, 0xff);
		Connection->ObjMap->SerialiseObject(Msg, *(VObject**)&CamEnt);
		Msg.WriteInt(Cam.TexNum, 0xffff);
		Msg.WriteInt(Cam.FOV, 360);

		//	Update replication info.
		RepCam.Camera = CamEnt;
		RepCam.TexNum = Cam.TexNum;
		RepCam.FOV = Cam.FOV;
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
				if (Msg.ReadBit())
					Side->Flags = Msg.ReadInt(0x000f);
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
				if (Msg.ReadBit())
				{
					Msg << Sec->params.Fade;
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

		case CMD_NewLevel:
#ifdef CLIENT
			CL_ParseServerInfo(Msg);
#endif
			break;

		case CMD_PreRender:
			Level->RenderData->PreRender();
#ifdef CLIENT
			CL_SignonReply();
#endif
			break;

		case CMD_CamTex:
			{
				int i = Msg.ReadInt(0xff);
				while (Level->CameraTextures.Num() <= i)
				{
					VCameraTextureInfo& C = Level->CameraTextures.Alloc();
					C.Camera = NULL;
					C.TexNum = -1;
					C.FOV = 0;
				}
				VCameraTextureInfo& Cam = Level->CameraTextures[i];
				Connection->ObjMap->SerialiseObject(Msg, *(VObject**)&Cam.Camera);
				Cam.TexNum = Msg.ReadInt(0xffff);
				Cam.FOV = Msg.ReadInt(360);
dprintf("Received camera %d %p\n", i, Cam.Camera);
			}
		}
	}
	unguard;
}
