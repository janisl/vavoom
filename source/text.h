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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

//
// Fonts
//
typedef enum
{
 	font_small,
	font_yellow,
    font_big,

    NUMFONTTYPES

} font_e;

//
// Horisontal alignement
//
typedef enum
{
 	hleft,		//Left
    hcenter,	//Centred
    hright		//Right

} halign_e;

//
// Vertical alignement
//
typedef enum
{
 	vtop,		//Top
    vcenter,	//Center
    vbottom		//Bottom

} valign_e;

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void T_Init();

void T_SetFont(font_e);
void T_SetDist(int, int);
void T_SetAlign(halign_e, valign_e);
void T_SetShadow(bool);

int T_StringWidth(char*);
int T_StringHeight(char*);

int T_TextWidth(char*);
int T_TextHeight(char*);

void T_DrawString(int, int, char*);
void T_DrawString8(int, int, char*);
void T_DrawText(int, int, char*);

void T_DrawNString(int, int, char*, int);
void T_DrawNText(int, int, char*, int);

void T_DrawCursor();
void T_DrawCursorAt(int, int);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/08/15 17:20:06  dj_jl
//	Moved prototype of T_DrawString8 from console.cpp
//
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
