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

//==========================================================================
//
//	Texture definition
//
//==========================================================================

struct maptexture_t
{
	struct mappatch_t
	{
		short		originx;
		short		originy;
		short		patch;
		short		stepdir;
		short		colormap;
	};

	char		name[8];
	short		masked;				// Unused
	byte		sscale;				// ZDoom's texture scaling
	byte		tscale;
	short		width;
	short		height;
	int			columndirectory;	// OBSOLETE
	short		patchcount;
	mappatch_t	patches[1];
};

//	Strife uses a cleaned-up version
struct maptexture_strife_t
{
	struct mappatch_t
	{
		short		originx;
		short		originy;
		short		patch;
	};

	char		name[8];
	int			masked;	//	boolean is not a good type here
	short		width;
	short		height;
	short		patchcount;
	mappatch_t	patches[1];
};

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/03/20 19:10:27  dj_jl
//	Added texture scaling.
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
