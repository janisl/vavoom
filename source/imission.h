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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct scores_t
{
	char		name[64];
	boolean		active;
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
void IM_Ticker(void);
void IM_Drawer(void);
void IM_SkipIntermission(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern scores_t			scores[MAXPLAYERS];
extern im_t				im;


