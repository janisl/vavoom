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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean			messageToPrint = false;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

//
//	MESSAGE BOX
//
static int		msgBoxX;
static int		msgBoxY;
static int		msgBoxW;
static int		msgBoxH;
static char		messageString[256];	// ...and here is the message string!
static boolean	messageNeedsInput;	// timed message = no input from user
static void    	(*messageRoutine)(int response);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	MB_Drawer
//
//==========================================================================

void MB_Drawer(void)
{
    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
    	char	*flat;

        switch (Game)
        {
         default:	   flat = "FLAT20";   break;
         case Heretic: flat = "FLOOR04";  break;
         case Hexen:   flat = "F_019";    break;
		 case Strife:  flat = "F_PAVE01"; break;
        }
        R_FillRectWithFlat(msgBoxX, msgBoxY, msgBoxW, msgBoxH, flat);
        R_DrawBorder(msgBoxX, msgBoxY, msgBoxW, msgBoxH);
        T_SetFont(font_small);
		T_SetAlign(hcenter, vcenter);
		T_DrawText(160, 100, messageString);
    }
}

//==========================================================================
//
//  MB_StartMessage
//
//==========================================================================

void MB_StartMessage(char* string, void* routine, boolean input)
{
    messageToPrint = true;
    strcpy(messageString, string);
    messageRoutine = (void(*)(int))routine;
    messageNeedsInput = input;

	T_SetFont(font_small);
	msgBoxW = (T_TextWidth (messageString) + 16) & ~15;
   	msgBoxH = (T_TextHeight(messageString) + 16) & ~15;
    if (msgBoxW > 304) msgBoxW = 304;
    if (msgBoxH > 184) msgBoxH = 184;
    msgBoxX = (320 - msgBoxW) / 2;
    msgBoxY = (200 - msgBoxH) / 2;
}

//==========================================================================
//
//  MB_Responder
//
//==========================================================================

boolean MB_Responder(event_t *event)
{
    // Take care of any messages that need input
	if (messageToPrint && event->type == ev_keydown)
    {
		if (messageNeedsInput &&
	    	event->data1 != ' ' &&
	    	event->data1 != 'n' &&
	    	event->data1 != 'y' &&
	    	event->data1 != K_ESCAPE &&
	    	event->data1 != K_MOUSE1 &&
	    	event->data1 != K_MOUSE2)
		{
	    	return true;
		}
		
		messageToPrint = false;
		if (messageRoutine)
	    	messageRoutine(event->data1);
			
//		S_StartSound(sfx1_swtchx);
		return true;
    }
	return false;
}
