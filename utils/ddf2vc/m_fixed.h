//----------------------------------------------------------------------------
//  EDGE Fixed Point Stuff
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

#ifndef __M_FIXED__
#define __M_FIXED__

//
// Fixed point, 32bit as <-16bits . 16bits->
//
#define FRACBITS    16
#define FRACUNIT    (1 << FRACBITS)
#define FRACSQRT2   0x16A0A

typedef int fixed_t;

#endif
