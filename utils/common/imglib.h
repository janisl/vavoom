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

namespace VavoomUtils {

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct rgb_t
{
	byte		r;
	byte		g;
	byte		b;
};

struct rgba_t
{
	byte		r;
	byte		g;
	byte		b;
	byte		a;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void LoadImage(const char *name);
void DestroyImage(void);
void ConvertImageTo32Bit(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int				ImgWidth;
extern int				ImgHeight;
extern int				ImgBPP;
extern byte				*ImgData;
extern rgb_t			ImgPal[256];

} // namespace VavoomUtils

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/04/11 16:53:31  dj_jl
//	Added support for TGA images.
//
//	Revision 1.4  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/09/24 17:28:45  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
