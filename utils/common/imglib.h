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
	vuint8		r;
	vuint8		g;
	vuint8		b;
};

struct rgba_t
{
	vuint8		r;
	vuint8		g;
	vuint8		b;
	vuint8		a;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void LoadImage(const char *name);
void DestroyImage();
void ConvertImageTo32Bit();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int				ImgWidth;
extern int				ImgHeight;
extern int				ImgBPP;
extern vuint8*			ImgData;
extern rgb_t			ImgPal[256];

} // namespace VavoomUtils
