//----------------------------------------------------------------------------
//  EDGE Screen Handling
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

#ifndef __V_SCREENS_H__
#define __V_SCREENS_H__

#include "dm_type.h"

typedef struct screen_s screen_t;

// screen_t:
// The struct used by all rendering routines to describe where to
// draw.
struct screen_s
{
  // unique number for this screen. Changes whenever screen is resized.
  long id;
  // Dimensions
  int width;
  int height;
  // The address difference between two vertically adjacent pixels
  int pitch;
  // Bytes Per Pixel
  int bytepp;
  // The data area to draw to
  byte *data;
  // Parent screen: The screen this screen is part of if it's a subscreen.
  // Otherwise this is NULL.
  screen_t *parent;
  // The number of children this screen has. You must destroy all children
  // before changing the data pointer (ie. resize or destroy the screen).
  int children;
  // position within parent if it's a subscreen
  int x, y;
};

void V_EmptyScreen(screen_t *scr);
void V_DestroyScreen(screen_t *scr);
void V_InitScreen(screen_t *s, int width, int height, int bytepp);
screen_t *V_CreateScreen(int width, int height, int bytepp);
screen_t *V_CreateEmptyScreen(void);
void V_InitSubScreen(screen_t *s, screen_t *p, int x, int y, int width, int height);
screen_t *V_CreateSubScreen(screen_t *p, int x, int y, int width, int height);
void V_MoveSubScreen(screen_t *s, int x, int y);
screen_t *V_ResizeScreen(screen_t *s, int newwidth, int newheight, int newbytepp);
int V_GetPitch(int width, int bytepp);
boolean_t V_ScreensIntersect(screen_t *s1, screen_t * s2);
int V_ScreenHasCurrentRes(screen_t *scr);

#endif // __V_SCREENS_H__
