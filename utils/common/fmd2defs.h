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
//**
//**	ALIAS MODELS
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// little-endian "IDP2"
#define IDPOLY2HEADER		(('2'<<24)+('P'<<16)+('D'<<8)+'I')
#define ALIAS_VERSION		8

// TYPES -------------------------------------------------------------------

struct mmdl_t
{
	int			ident;
	int			version;
	int			skinwidth;
	int			skinheight;
	int			framesize;
	int			numskins;
	int			numverts;
	int			numstverts;
	int			numtris;
	int			numcmds;
	int			numframes;
	int			ofsskins;
	int			ofsstverts;
	int			ofstris;
	int			ofsframes;
	int			ofscmds;
	int			ofsend;
};

struct mskin_t
{
	char		name[64];
};

struct mstvert_t
{
	short		s;
	short		t;
};

struct mtriangle_t
{
	short		vertindex[3];
	short		stvertindex[3];
};

struct mframe_t
{
	float		scale[3];
	float		scale_origin[3];
	char		name[16];
};

struct trivertx_t
{
	byte		v[3];
	byte		lightnormalindex;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//
//	Revision 1.1  2001/09/24 17:29:18  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
