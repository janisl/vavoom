//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

// status bar height at bottom of screen
#define SB_REALHEIGHT	(sb_height * ScreenHeight / 200)

// TYPES -------------------------------------------------------------------

typedef enum
{
	SBW_PATCH,
	SBW_PATCHI,
    SBW_NUMBER,
    SBW_INUMBER,
    SBW_MNUMBER,
    SBW_NUMBER9,
    SBW_NUMBER12,
    SBW_SHADE,
} sb_widget_type_t;

typedef enum
{
	SB_VIEW_NORMAL,
    SB_VIEW_AUTOMAP,
    SB_VIEW_FULLSCREEN

} sb_view_t;

typedef enum
{
 	SB_STYLE_NORMAL,
    SB_STYLE_TRANSLUCENT,
    SB_STYLE_SHADOWED

} sb_style_t;

struct sb_widget_t
{
	sb_widget_t			*next;
    sb_widget_type_t	type;
    int					x;
    int					y;
	int					pindex;
    sb_style_t			style;
    boolean				*on;
    int					pparam;
    int					*data;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SB_Init(void);
void SB_Drawer(void);
void SB_Ticker(void);
boolean	SB_Responder(event_t* ev);
void SB_Start(void);// Called when the console player is spawned on each level.
sb_widget_t *SB_CreateWidget(sb_widget_type_t type, int x, int y, int pindex, sb_style_t style, boolean *on, int pparam, int *data);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int		sb_height;

