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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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
	char		name[64];
	dword		bActive:1;
	int			frags[MAXPLAYERS];
	int			killcount;
	int			itemcount;
	int			secretcount;
	float		time;
	char		userinfo[MAX_INFO_STRING];
};

struct im_t
{
	char		leavemap[12];
	int			leavecluster;
	char		entermap[12];
	int			entercluster;
	char		leave_name[32];
	char		enter_name[32];
	int			totalkills;
	int			totalitems;
	int			totalsecret;
	float		time;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void IM_Start(void);
void IM_SkipIntermission(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern scores_t			scores[MAXPLAYERS];
extern im_t				im;

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2006/02/09 22:35:54  dj_jl
//	Moved all client game code to classes.
//
//	Revision 1.5  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.4  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
