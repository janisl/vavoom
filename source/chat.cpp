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
static TCvarS ChatMacro0("Chatmacro0", "No", CVAR_ARCHIVE);
static TCvarS ChatMacro1("Chatmacro1", "I'm ready to kick butt!", CVAR_ARCHIVE);
static TCvarS ChatMacro2("Chatmacro2", "I'm OK.", CVAR_ARCHIVE);
static TCvarS ChatMacro3("Chatmacro3", "I'm not looking too good!", CVAR_ARCHIVE);
static TCvarS ChatMacro4("Chatmacro4", "Help!", CVAR_ARCHIVE);
static TCvarS ChatMacro5("Chatmacro5", "You suck!", CVAR_ARCHIVE);
static TCvarS ChatMacro6("Chatmacro6", "Next time, scumbag...", CVAR_ARCHIVE);
static TCvarS ChatMacro7("Chatmacro7", "Come here!", CVAR_ARCHIVE);
static TCvarS ChatMacro8("Chatmacro8", "I'll take care of it.", CVAR_ARCHIVE);
static TCvarS ChatMacro9("Chatmacro9", "Yes", CVAR_ARCHIVE);
static TCvarS *chat_macros[10] =
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
    boolean		eatkey;

	if (!chatmodeon || ev->type != ev_keydown)
	{
		return false;
	}

	if (altdown)
	{
		if (ev->data1 >= '0' && ev->data1 <= '9')
		{
			CmdBuf << "Say " << *chat_macros[ev->data1 - '0'] << "\n";
			CT_Stop();
			return true;
		}
	}

   	eatkey = w_chat.Key((byte)ev->data1);
	if (ev->data1 == K_ENTER || ev->data1 == K_PADENTER)
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

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2002/03/02 17:30:34  dj_jl
//	Added suport for Pad-Enter.
//
//	Revision 1.6  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2001/12/04 18:11:59  dj_jl
//	Fixes for compiling with MSVC
//	
//	Revision 1.4  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
