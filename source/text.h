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
	hcentre,	//Centred
	hright		//Right
};

//
// Vertical alignement
//
enum valign_e
{
	vtop,		//Top
	vcentre,	//Centre
	vbottom		//Bottom
};

class VFont;

void T_Init();
void T_Shutdown();

void T_SetFont(font_e);
void T_SetAlign(halign_e, valign_e);
void T_SetShadow(bool);

int T_StringWidth(VFont* Font, const VStr&);
int T_StringHeight(VFont* Font, const VStr&);

int T_TextWidth(VFont* Font, const VStr&);
int T_TextHeight(VFont* Font, const VStr&);

void T_DrawString(int, int, const VStr&);
void T_DrawString8(int, int, const VStr&);
void T_DrawText(int, int, const VStr&);
int T_DrawTextW(int, int, const VStr&, int);

void T_DrawCursor();
void T_DrawCursorAt(int, int);
