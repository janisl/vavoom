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

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  R_DrawBorder
//
//	Draws border
//
//==========================================================================

void R_DrawBorder(int x, int y, int w, int h)
{
	int			i;
	int			handle;

	if (Game == Doom || Game == Doom2 || Game == Strife)
    {
	    handle = R_RegisterPic("brdr_t", PIC_PATCH);
    	for (i=0 ; i<w ; i+=8)
			R_DrawPic(x + i, y - 8, handle);

	    handle = R_RegisterPic("brdr_b", PIC_PATCH);
    	for (i=0 ; i<w ; i+=8)
			R_DrawPic(x + i, y + h, handle);

	    handle = R_RegisterPic("brdr_l", PIC_PATCH);
    	for (i=0 ; i<h ; i+=8)
			R_DrawPic(x - 8, y + i, handle);

	    handle = R_RegisterPic("brdr_r", PIC_PATCH);
    	for (i=0 ; i<h ; i+=8)
			R_DrawPic(x + w, y + i, handle);

	    // Draw beveled edge.
    	R_DrawPic(x - 8, y - 8, R_RegisterPic("brdr_tl", PIC_PATCH));
	    R_DrawPic(x + w, y - 8, R_RegisterPic("brdr_tr", PIC_PATCH));
	    R_DrawPic(x - 8, y + h, R_RegisterPic("brdr_bl", PIC_PATCH));
	    R_DrawPic(x + w, y + h, R_RegisterPic("brdr_br", PIC_PATCH));
	}
	else
    {
	    handle = R_RegisterPic("bordt", PIC_PATCH);
    	for (i=0 ; i<w ; i+=16)
			R_DrawPic(x + i, y - 4, handle);

	    handle = R_RegisterPic("bordb", PIC_PATCH);
    	for (i=0 ; i<w ; i+=16)
			R_DrawPic(x + i, y + h, handle);

	    handle = R_RegisterPic("bordl", PIC_PATCH);
    	for (i=0 ; i<h ; i+=16)
			R_DrawPic(x - 4, y + i, handle);

	    handle = R_RegisterPic("bordr", PIC_PATCH);
    	for (i=0 ; i<h ; i+=16)
			R_DrawPic(x + w, y + i, handle);

	    // Draw beveled edge.
    	R_DrawPic(x - 4, y - 4, R_RegisterPic("bordtl", PIC_PATCH));
	    R_DrawPic(x + w, y - 4, R_RegisterPic("bordtr", PIC_PATCH));
	    R_DrawPic(x - 4, y + h, R_RegisterPic("bordbl", PIC_PATCH));
	    R_DrawPic(x + w, y + h, R_RegisterPic("bordbr", PIC_PATCH));
	}
}

//==========================================================================
//
//	R_DrawViewBorder
//
//==========================================================================

void R_DrawViewBorder(void)
{
    const char*	name = NULL;

	switch (Game)
    {
     case Doom:
		name = "FLOOR7_2";
        break;
	 case Doom2:
		name = "GRNROCK";
        break;
	 case Heretic:
		name = Cvar_Value("shareware") ? "FLOOR04" : "FLAT513";
        break;
	 case Hexen:
		name = "F_022";
        break;
	 case Strife:
		name = "F_PAVE01";
		break;
	}
	R_FillRectWithFlat(0, 0, 320, 200 - sb_height, name);

	R_DrawBorder(160 - screenblocks * 16,
                 (200 - sb_height - screenblocks * (200 - sb_height) / 10) / 2,
                 screenblocks * 32,
                 screenblocks * (200 - sb_height) / 10);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
