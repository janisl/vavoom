//----------------------------------------------------------------------------
//  EDGE Video Code for 8-Bit Colour. 
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2001  The EDGE Team.
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------
//
//  Based on the DOOM source code, released by Id Software under the
//  following copyright:
//
//    Copyright (C) 1993-1996 by id Software, Inc.
//
//----------------------------------------------------------------------------

#ifndef __V_VIDEO1__
#define __V_VIDEO1__

#include "dm_type.h"
#include "dm_defs.h"

// Needed because we are refering to patches.
#include "r_data.h"
#include "v_screen.h"

// -AJA- 1999/07/04: added the following misc drawing functions:
void V_DrawPixel8(screen_t * scr, int x, int y, int c);
void V_DrawLine8(screen_t * scr, int x1, int y1, int x2, int y2, int c);
void V_DrawBox8(screen_t * scr, int x, int y, int w, int h, int c);
void V_DrawBoxAlpha8(screen_t * scr, int x, int y, int w, int h, int c, fixed_t alpha);

void V_CopyRect8(screen_t * dest, screen_t * src, int srcx, int srcy, int width, int height, int destx, int desty);

void V_CopyScreen8(screen_t * dest, screen_t * src);

#endif
