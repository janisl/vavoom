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
	int			leavecluster;
	VName		EnterMap;
	int			entercluster;
	VStr		LeaveName;
	VStr		EnterName;
	int			totalkills;
	int			totalitems;
	int			totalsecret;
	float		time;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void IM_Start();
void IM_SkipIntermission();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern scores_t			scores[MAXPLAYERS];
extern im_t				im;
