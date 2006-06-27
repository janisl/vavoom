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
