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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:53  dj_jl
//**	Update with Id-s and Log-s, some fixes
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

boolean					chatmodeon;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TILine			w_chat;
static TCvarS			chat_macros[10] =
{
    TCvarS("Chatmacro0", "No", CVAR_ARCHIVE),
    TCvarS("Chatmacro1", "I'm ready to kick butt!", CVAR_ARCHIVE),
    TCvarS("Chatmacro2", "I'm OK.", CVAR_ARCHIVE),
    TCvarS("Chatmacro3", "I'm not looking too good!", CVAR_ARCHIVE),
    TCvarS("Chatmacro4", "Help!", CVAR_ARCHIVE),
    TCvarS("Chatmacro5", "You suck!", CVAR_ARCHIVE),
    TCvarS("Chatmacro6", "Next time, scumbag...", CVAR_ARCHIVE),
    TCvarS("Chatmacro7", "Come here!", CVAR_ARCHIVE),
    TCvarS("Chatmacro8", "I'll take care of it.", CVAR_ARCHIVE),
    TCvarS("Chatmacro9", "Yes", CVAR_ARCHIVE)
};

// CODE --------------------------------------------------------------------

//===========================================================================
//
//	CT_Init
//
// 	Initialize chat mode data
//
//===========================================================================

void CT_Init(void)
{
	chatmodeon = false;
}

//===========================================================================
//
//	CT_Stop
//
//===========================================================================

static void CT_Stop(void)
{
	chatmodeon = false;
}

//===========================================================================
//
// CT_Responder
//
//===========================================================================

boolean CT_Responder(event_t *ev)
{
    boolean		eatkey = false;

	if (!chatmodeon || ev->type != ev_keydown)
	{
		return false;
	}

	if (altdown)
	{
		if (ev->data1 >= '0' && ev->data1 <= '9')
		{
			CmdBuf << "Say " << chat_macros[ev->data1 - '0'] << "\n";
			CT_Stop();
			return true;
		}
	}

   	eatkey = w_chat.Key((byte)ev->data1);
	if (ev->data1 == K_ENTER)
	{
		CmdBuf << "Say " << w_chat.Data << "\n";
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

void CT_Drawer(void)
{
	if (chatmodeon)
	{
		T_SetFont(font_small);
        T_SetAlign(hleft, vtop);
        T_DrawString(25, 10, w_chat.Data);
       	T_DrawCursor();
	}
}

