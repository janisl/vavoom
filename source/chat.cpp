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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool					chatmodeon;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TILine			w_chat;
static VCvarS ChatMacro0("Chatmacro0", "No", CVAR_Archive);
static VCvarS ChatMacro1("Chatmacro1", "I'm ready to kick butt!", CVAR_Archive);
static VCvarS ChatMacro2("Chatmacro2", "I'm OK.", CVAR_Archive);
static VCvarS ChatMacro3("Chatmacro3", "I'm not looking too good!", CVAR_Archive);
static VCvarS ChatMacro4("Chatmacro4", "Help!", CVAR_Archive);
static VCvarS ChatMacro5("Chatmacro5", "You suck!", CVAR_Archive);
static VCvarS ChatMacro6("Chatmacro6", "Next time, scumbag...", CVAR_Archive);
static VCvarS ChatMacro7("Chatmacro7", "Come here!", CVAR_Archive);
static VCvarS ChatMacro8("Chatmacro8", "I'll take care of it.", CVAR_Archive);
static VCvarS ChatMacro9("Chatmacro9", "Yes", CVAR_Archive);
static VCvarS *chat_macros[10] =
{
    &ChatMacro0,
    &ChatMacro1,
    &ChatMacro2,
    &ChatMacro3,
    &ChatMacro4,
    &ChatMacro5,
    &ChatMacro6,
    &ChatMacro7,
    &ChatMacro8,
    &ChatMacro9,
};

// CODE --------------------------------------------------------------------

//===========================================================================
//
//	CT_Init
//
// 	Initialise chat mode data
//
//===========================================================================

void CT_Init()
{
	chatmodeon = false;
}

//===========================================================================
//
//	CT_Stop
//
//===========================================================================

static void CT_Stop()
{
	chatmodeon = false;
}

//===========================================================================
//
// CT_Responder
//
//===========================================================================

bool CT_Responder(event_t *ev)
{
	bool		eatkey;

	if (!chatmodeon || ev->type != ev_keydown)
	{
		return false;
	}

	if (GInput->AltDown)
	{
		if (ev->data1 >= '0' && ev->data1 <= '9')
		{
			GCmdBuf << "Say " << *chat_macros[ev->data1 - '0'] << "\n";
			CT_Stop();
			return true;
		}
	}

	eatkey = w_chat.Key((byte)ev->data1);
	if (ev->data1 == K_ENTER || ev->data1 == K_PADENTER)
	{
		GCmdBuf << "Say " << w_chat.Data << "\n";
		CT_Stop();
		return true;
	}
	else if (ev->data1 == K_ESCAPE)
	{
		CT_Stop();
		return true;
	}
	return eatkey;
}

//==========================================================================
//
//	COMMAND ChatMode
//
//==========================================================================

COMMAND(ChatMode)
{
	w_chat.Init();
	chatmodeon = true;
}

//===========================================================================
//
// CT_Drawer
//
//===========================================================================

void CT_Drawer()
{
	if (chatmodeon)
	{
		T_SetFont(SmallFont);
		T_SetAlign(hleft, vtop);
		T_DrawString(25, 10, w_chat.Data);
		T_DrawCursor();
	}
}
