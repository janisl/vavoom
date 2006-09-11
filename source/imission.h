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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct scores_t
{
	VStr		name;
	enum
	{
		SF_Active	= 0x01,
	};
	vuint32		Flags;
	int			frags[MAXPLAYERS];
	int			killcount;
	int			itemcount;
	int			secretcount;
	float		time;
	VStr		userinfo;
};

struct im_t
{
	VName		LeaveMap;
	vint32		LeaveCluster;
	VStr		LeaveName;

	VName		EnterMap;
	vint32		EnterCluster;
	VStr		EnterName;

	VStr		Text;
	VName		TextFlat;
	VName		TextPic;
	VName		TextMusic;
	vint32		TextCDTrack;
	vint32		TextCDId;

	vint32		TotalKills;
	vint32		TotalItems;
	vint32		TotalSecret;
	float		Time;

	enum
	{
		IMF_NoIntermission	= 0x01,
		IMF_TextIsLump		= 0x02,
	};
	vint32		IMFlags;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void IM_Start();
void IM_SkipIntermission();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern scores_t			scores[MAXPLAYERS];
extern im_t				im;
