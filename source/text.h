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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

//
// Fonts
//
enum font_e
{
	font_small,
	font_yellow,
	font_big,

	NUMFONTTYPES
};

//
// Horisontal alignement
//
enum halign_e
{
	hleft,		//Left
	hcenter,	//Centred
	hright		//Right
};

//
// Vertical alignement
//
enum valign_e
{
	vtop,		//Top
	vcenter,	//Center
	vbottom		//Bottom
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void T_Init();
void T_Shutdown();

void T_SetFont(font_e);
void T_SetDist(int, int);
void T_SetAlign(halign_e, valign_e);
void T_SetShadow(bool);

int T_StringWidth(const char*);
int T_StringHeight(const char*);

int T_TextWidth(const char*);
int T_TextHeight(const char*);

void T_DrawString(int, int, const char*);
void T_DrawString8(int, int, const char*);
void T_DrawText(int, int, const char*);
int T_DrawTextW(int x, int y, const char* String, int w);

void T_DrawNString(int, int, const char*, int);
void T_DrawNText(int, int, const char*, int);

void T_DrawCursor();
void T_DrawCursorAt(int, int);

// PUBLIC DATA DECLARATIONS ------------------------------------------------
