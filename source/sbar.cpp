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

extern refdef_t			refdef;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int 					sb_height = 32;

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
    char	**names;

	names = (char**)clpr.GlobalAddr("sb_patch_names");
    for (i = 0; names[i][0]; i++)
    {
    	sb_pics[i] = R_RegisterPic(names[i], PIC_PATCH);
		R_GetPicInfo(sb_pics[i], &sb_pic_info[i]);
    }
	Z_FreeTag(PU_CACHE);
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
	sb_view_t	sb_view;

	sb_view = automapactive ? SB_VIEW_AUTOMAP :
		refdef.height == ScreenHeight ? SB_VIEW_FULLSCREEN : SB_VIEW_NORMAL;
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

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2001/08/30 17:44:07  dj_jl
//	Removed memory leaks after startup
//
//	Revision 1.5  2001/08/21 17:39:22  dj_jl
//	Real string pointers in progs
//	
//	Revision 1.4  2001/08/15 17:08:15  dj_jl
//	Fixed Strife status bar
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
