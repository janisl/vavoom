//----------------------------------------------------------------------------
//  EDGE Floor and Ceiling Rendering Code
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
//
// -KM- 1998/09/27 Dynamic colourmaps
//

#ifndef __R_PLANE__
#define __R_PLANE__

#include "r_data.h"
#include "m_math.h"
#include "z_zone.h"

// -ES- 1999/03/20 Removed origyslope
extern float_t *yslope;
extern float_t *distscale;

// -ES- 1999/05/26 Externalised these.
extern int *spanstart;

#endif
