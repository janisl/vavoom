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
//**
//**	Status bar code.
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

extern int				viewheight;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int 			sb_height = 32;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static sb_widget_t*		widgets = NULL;
static sb_widget_t*		last_widget = NULL;

static int				sb_pics[1024];
static picinfo_t		sb_pic_info[1024];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  SB_LoadPatches
//
//==========================================================================

static void SB_LoadPatches(void)
{
	int		i;
    int		num;

	num = clpr.GlobalNumForName("sb_patch_names");
    for (i = 0; clpr.GetGlobal(i + num); i++)
    {
    	sb_pics[i] = R_RegisterPic(clpr.GetString(clpr.GetGlobal(num + i)), PIC_PATCH);
		R_GetPicInfo(sb_pics[i], &sb_pic_info[i]);
    }
}

//==========================================================================
//
//  SB_CreateWidget
//
//==========================================================================

sb_widget_t *SB_CreateWidget(sb_widget_type_t type, int x, int y,
	int pindex, sb_style_t style, boolean *on, int pparam, int *data)
{
	sb_widget_t	*widget;

    widget = (sb_widget_t *)Z_StrMalloc(sizeof(*widget));
    widget->next = NULL;
    widget->type = type;
    widget->x = x;
    widget->y = y;
    widget->pindex = pindex;
    widget->style = style;
    widget->on = on;
    widget->pparam = pparam;
    widget->data = data;
	if (widgets)
    {
    	last_widget->next = widget;
    }
    else
    {
    	widgets = widget;
    }
	last_widget = widget;
	return widget;
}

//==========================================================================
//
//	SB_DrawPatch
//
//==========================================================================

static int SB_DrawPatch(int x, int y, int index, sb_style_t style, int pparam)
{
	switch (style)
    {
     case SB_STYLE_NORMAL:
		R_DrawPic(x, y, sb_pics[index]);
        break;
     case SB_STYLE_TRANSLUCENT:
		R_DrawPic(x, y, sb_pics[index], pparam);
        break;
     case SB_STYLE_SHADOWED:
		R_DrawShadowedPic(x, y, sb_pics[index]);
        break;
	}
	return sb_pic_info[index].width;
}

//==========================================================================
//
//	SB_DrawPatchR
//
//==========================================================================

static int SB_DrawPatchR(int x, int y, int index, sb_style_t style, int pparam)
{
	x -= sb_pic_info[index].width;
	switch (style)
    {
     case SB_STYLE_NORMAL:
		R_DrawPic(x, y, sb_pics[index]);
        break;
     case SB_STYLE_TRANSLUCENT:
		R_DrawPic(x, y, sb_pics[index], pparam);
        break;
     case SB_STYLE_SHADOWED:
		R_DrawShadowedPic(x, y, sb_pics[index]);
        break;
	}
	return sb_pic_info[index].width;
}

//==========================================================================
//
//	ShadeLine
//
//==========================================================================

void R_ShadeRect(int x, int y, int width, int height, int shade)
{
	Drawer->ShadeRect((int)(x * fScaleX), (int)(y * fScaleY),
		(int)((x + width) * fScaleX) - (int)(x * fScaleX),
		(int)((y + height) * fScaleY) - (int)(y * fScaleY), shade);
}

//==========================================================================
//
//	ShadeLine
//
//==========================================================================

static void ShadeLine(int x, int y, int height, int shade)
{
	R_ShadeRect(x, y, 1, height, 9 + shade * 2);
}

//==========================================================================
//
//  SB_DrawWidget
//
//==========================================================================

static void SB_DrawWidget(sb_widget_t *widget)
{
	switch (widget->type)
    {
     case SBW_PATCH:
     	SB_DrawPatch(widget->x, widget->y, widget->pindex, widget->style,
     		widget->pparam);
     	break;

     case SBW_PATCHI:
		if (*widget->data >= 0)
	     	SB_DrawPatch(widget->x, widget->y, widget->pindex + *widget->data,
	     		widget->style, widget->pparam);
     	break;
	 case SBW_NUMBER:
	    // if non-number, do not draw it
	    if (*widget->data != 1994)
        {
		    int		num = *widget->data;
    
		    int		w = sb_pic_info[widget->pindex].width;
	    	int		x = widget->x;
    
		    int		neg = num < 0;

            if (widget->style == SB_STYLE_SHADOWED)
            	w += 3;
		    if (neg)
			{
				num = -num;
			}

	        // in the special case of 0, you draw 0
	        if (!num)
	        	SB_DrawPatch(x - w, widget->y, widget->pindex,
	        		widget->style, widget->pparam);

	        // draw the new number
	        while (num)
			{
		    	x -= w;
		    	SB_DrawPatch(x, widget->y, widget->pindex + num % 10,
		    		widget->style, widget->pparam);
		    	num /= 10;
			}

			// draw a minus sign if necessary
			if (neg)
				SB_DrawPatch(x - 8, widget->y, widget->pindex + 10,
					widget->style, widget->pparam);
		}
		break;
	 case SBW_INUMBER:
	    if (*widget->data > 1)
        {
		    int		num = *widget->data;
	    	int		x = widget->x + 8;
    
	        // draw the new number
	        while (num)
			{
		    	x -= 4;
		    	SB_DrawPatch(x, widget->y, widget->pindex + num % 10,
		    		widget->style, widget->pparam);
		    	num /= 10;
			}
		}
		break;
	 case SBW_MNUMBER:
	    if (*widget->data > 0)
        {
		    int		num = *widget->data;
	    	int		x = widget->x + 12;
    
	        // draw the new number
	        while (num)
			{
		    	x -= 4;
		    	SB_DrawPatch(x, widget->y, widget->pindex + num % 10,
		    		widget->style, widget->pparam);
		    	num /= 10;
			}
		}
		break;
	 case SBW_NUMBER9:
	    // if non-number, do not draw it
	    if (*widget->data != 1994)
        {
		    int		num = *widget->data;
    
		    int		w = 9;
	    	int		x = widget->x + 27;
    
		    int		neg = num < 0;

            if (num < -9)
            {
            	//	Lame message
				SB_DrawPatch(widget->x + 1, widget->y + 1, widget->pindex + 11,
		    		widget->style, widget->pparam);
                break;
			}
		    if (neg)
			{
				num = -num;
			}

	        // in the special case of 0, you draw 0
	        if (!num)
	        	SB_DrawPatch(x - w, widget->y, widget->pindex,
	        		widget->style, widget->pparam);

	        // draw the new number
	        while (num)
			{
		    	x -= w;
		    	SB_DrawPatch(x, widget->y, widget->pindex + num % 10,
		    		widget->style, widget->pparam);
		    	num /= 10;
			}

			// draw a minus sign if necessary
			if (neg)
				SB_DrawPatch(x - 8, widget->y, widget->pindex + 10,
					widget->style, widget->pparam);
		}
		break;
	 case SBW_NUMBER12:
	    // if non-number, do not draw it
	    if (*widget->data != 1994)
        {
		    int		num = *widget->data;
	    	int		x = widget->x + 36;
    
	        // in the special case of 0, you draw 0
	        if (!num)
	        	SB_DrawPatchR(x, widget->y, widget->pindex,
	        		widget->style, widget->pparam);

	        // draw the new number
	        while (num)
			{
		    	x -= SB_DrawPatchR(x, widget->y, widget->pindex + num % 10,
		    		widget->style, widget->pparam);
		    	num /= 10;
		    	x -= 3;
			}
		}
		break;
	 case SBW_SHADE:
     	ShadeLine(widget->x, widget->y, widget->pindex, widget->pparam);
        break;
    }
}

//==========================================================================
//
//  SB_DrawWidgets
//
//==========================================================================

static void SB_DrawWidgets(void)
{
	sb_widget_t*	widget;

	for (widget = widgets; widget; widget = widget->next)
    {
		if (*widget->on)
    	{
    		SB_DrawWidget(widget);
	    }
    }
}

//==========================================================================
//
//  SB_DestroyWidgets
//
//==========================================================================

static void SB_DestroyWidgets(void)
{
	sb_widget_t*	widget;

	for (widget = widgets; widget; widget = widget->next)
    {
    	Z_Free(widget);
    }
	widgets = NULL;
}

//==========================================================================
//
//  SB_Init
//
//==========================================================================

void SB_Init(void)
{
	if (Game == Strife)
	{
		sb_height = 32;
		return;
	}
	sb_height = clpr.GetGlobal("sb_height");

	SB_LoadPatches();
}

//==========================================================================
//
//  SB_Ticker
//
//==========================================================================

void SB_Ticker(void)
{
	if (cls.signon == SIGNONS)
	    clpr.Exec("SB_UpdateWidgets");
}

//==========================================================================
//
//	SB_Responder
//
//==========================================================================

boolean SB_Responder(event_t *)
{
	return false;
}

//==========================================================================
//
//	SB_Drawer
//
//==========================================================================

void SB_Drawer(void)
{
	if (Game == Strife)
	{
		return;
	}
	sb_view_t	sb_view;

	sb_view = automapactive ? SB_VIEW_AUTOMAP : viewheight == ScreenHeight ?
    	SB_VIEW_FULLSCREEN : SB_VIEW_NORMAL;
	//	Update widget visibility
	clpr.Exec("SB_OnDraw", sb_view);

	SB_DrawWidgets();
}

//==========================================================================
//
//  SB_Start
//
//==========================================================================

void SB_Start(void)
{
    SB_DestroyWidgets();
	clpr.Exec("SB_CreateWidgets");
}

//==========================================================================
//
//	DrawSoundInfo
//
//	Displays sound debugging information.
//
//==========================================================================

#if 0
static void DrawSoundInfo(void)
{
	int i;
	SoundInfo_t s;
	ChanInfo_t *c;
	char text[32];
	int x;
	int y;
	int xPos[7] = {1, 75, 112, 156, 200, 230, 260};

	if(level.time&16)
	{
		MN_DrTextA("*** SOUND DEBUG INFO ***", xPos[0], 20);
	}
	S_GetChannelInfo(&s);
	if(s.channelCount == 0)
	{
		return;
	}
	x = 0;
	MN_DrTextA("NAME", xPos[x++], 30);
	MN_DrTextA("MO.T", xPos[x++], 30);
	MN_DrTextA("MO.X", xPos[x++], 30);
	MN_DrTextA("MO.Y", xPos[x++], 30);
	MN_DrTextA("ID", xPos[x++], 30);
	MN_DrTextA("PRI", xPos[x++], 30);
	MN_DrTextA("DIST", xPos[x++], 30);
	for(i = 0; i < s.channelCount; i++)
	{
		c = &s.chan[i];
		x = 0;
		y = 40+i*10;
		if(c->mo == NULL)
		{ // Channel is unused
			MN_DrTextA("------", xPos[0], y);
			continue;
		}
		sprintf(text, "%s", c->name);
		M_ForceUppercase(text);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->type);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->x>>FRACBITS);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->y>>FRACBITS);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->id);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->priority);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->distance);
		MN_DrTextA(text, xPos[x++], y);
	}
}
#endif

