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
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************
//
//                       SCREEN WIPE PACKAGE
//
//**************************************************************************


// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	wipe_None,

    // weird screen melt
    wipe_Melt,	

    // using translucency
    wipe_Translucent,

    NUMWIPES
};

typedef struct
{
    int (*init)(void);
    int (*dowipe)(int);
    int (*exit)(void);
} wipe_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int wipe_initMelt(void);
static int wipe_doMelt(int ticks);
static int wipe_exitMelt(void);

static int wipe_initTranslucent(void);
static int wipe_doTranslucent(int ticks);
static int wipe_exitTranslucent(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static byte*	wipe_scr_start = NULL;
static byte*	wipe_scr_end = NULL;
static byte*	wipe_scr = NULL;

static wipe_t	wipes[] =
{
	{NULL, NULL, NULL},
    {wipe_initMelt, wipe_doMelt, wipe_exitMelt},
    {wipe_initTranslucent, wipe_doTranslucent, wipe_exitTranslucent}
};

static int*		y;
static int		wipe_translucency;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  wipe_StartScreen
//
//==========================================================================

static void wipe_StartScreen(void)
{
    wipe_scr_start = (byte*)Z_Malloc(ScreenWidth * ScreenHeight, PU_STATIC, 0);
    memcpy(wipe_scr_start, scrn, ScreenWidth * ScreenHeight);
}

//==========================================================================
//
//	TSoftwareDrawer::InitWipe
//
//==========================================================================

bool TSoftwareDrawer::InitWipe(void)
{
	if (ScreenBPP == 8)
	{
		wipe_StartScreen();
		return true;
	}
	else
	{
		return false;
	}
}

//==========================================================================
//
//	TSoftwareDrawer::DoWipe
//
//==========================================================================

void TSoftwareDrawer::DoWipe(int wipe_num)
{
	int			nowtime;
	int			tics;
	int			wipestart;
	boolean		done;

	// wipe update
	wipe_scr_end = (byte*)Z_Malloc(ScreenWidth * ScreenHeight, PU_STATIC, 0);
	memcpy(wipe_scr_end, scrn, ScreenWidth * ScreenHeight);
	// restore start scr.
	memcpy(scrn, wipe_scr_start, ScreenWidth * ScreenHeight);

	// initial stuff
	wipes[wipe_num].init();
	wipestart = (int)(Sys_Time() * 35) - 1;

	do
	{
		do
		{
			nowtime = (int)(Sys_Time() * 35);
			tics = nowtime - wipestart;
		} while (!tics);

		wipestart = nowtime;

		wipe_scr = scrn;

		// do a piece of wipe-in
		done = wipes[wipe_num].dowipe(tics);

		C_Drawer();
		MN_Drawer();
		MB_Drawer();

		Update();
    } while (!done);

	wipes[wipe_num].exit();
	if (wipe_scr_start) Z_Free(wipe_scr_start);
	if (wipe_scr_end) Z_Free(wipe_scr_end);
	wipe_scr_start = NULL;
	wipe_scr_end = NULL;
}

//==========================================================================
//
//  wipe_shittyColMajorXform
//
//==========================================================================

static void wipe_shittyColMajorXform(short* array, int width, int height)
{
    int		x;
    int		y;
    short*	dest;

	dest = (short*) Z_Malloc(width * height * 2, PU_STATIC, 0);

    for (y=0; y<height; y++)
	 for (x=0; x<width; x++)
	    dest[x * height + y] = array[y * width + x];

    memcpy(array, dest, width * height * 2);

    Z_Free(dest);

}

//==========================================================================
//
//  wipe_initMelt
//
//==========================================================================

static int wipe_initMelt(void)
{
    int i, r;
    
    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((short*)wipe_scr_start, ScreenWidth / 2, ScreenHeight);
    wipe_shittyColMajorXform((short*)wipe_scr_end, ScreenWidth / 2, ScreenHeight);
    
    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y = (int *) Z_Malloc(ScreenWidth * sizeof(int), PU_STATIC, 0);
    y[0] = -(rand() % 16);
    for (i = 1; i < ScreenWidth; i++)
    {
		r = (rand() % 3) - 1;
		y[i] = y[i - 1] + r;
		if (y[i] > 0) y[i] = 0;
		else if (y[i] == -16) y[i] = -15;
    }

    return 0;
}

//==========================================================================
//
//  wipe_doMelt
//
//==========================================================================

static int wipe_doMelt(int ticks)
{
    int		i;
    int		j;
    int		dy;
    int		idx;

    short*	s;
    short*	d;
    boolean	done = true;
	int		width;

    width = ScreenWidth / 2;

    while (ticks--)
    {
		for (i = 0; i < width; i++)
		{
	    	if (y[i] < 0)
	    	{
				y[i]++;
            	done = false;
	    	}
	    	else if (y[i] < ScreenHeight)
			{
				dy = (y[i] < 16) ? y[i] + 1 : 8;
				if (y[i] + dy >= ScreenHeight)
					dy = ScreenHeight - y[i];
				s = &((short *)wipe_scr_end)[i * ScreenHeight];
				d = &((short *)wipe_scr)[i];
				idx = 0;
				for (j = dy + y[i]; j; j--)
				{
		    		d[idx] = *(s++);
		    		idx += width;
				}
				y[i] += dy;
				s = &((short *)wipe_scr_start)[i * ScreenHeight];
				d = &((short *)wipe_scr)[y[i] * width + i];
				idx = 0;
				for (j = ScreenHeight - y[i]; j; j--)
				{
					d[idx] = *(s++);
		    		idx += width;
				}
				done = false;
	    	}
        	else
            {
				s = &((short *)wipe_scr_end)[i * ScreenHeight];
				d = &((short *)wipe_scr)[i];
				idx = 0;
				for (j = ScreenHeight; j; j--)
				{
		    		d[idx] = *(s++);
		    		idx += width;
				}
            }
		}
    }
	return done;
}

//==========================================================================
//
//  wipe_exitMelt
//
//==========================================================================

static int wipe_exitMelt(void)
{
    Z_Free(y);
    return 0;
}

//==========================================================================
//
//  wipe_initTranslucent
//
//==========================================================================

static int wipe_initTranslucent(void)
{
    wipe_translucency = 0;
    return 0;
}

//==========================================================================
//
//  wipe_doTranslucent
//
//==========================================================================

static int wipe_doTranslucent(int ticks)
{
    boolean		invert = false;
	int			i;
    byte*		trtable;
    byte*		s;
    byte*		e;
    byte*		w;

	wipe_translucency += ticks * 2;
	i = (wipe_translucency - 5) / 10;
	//Invisible
    if (i<0)
	{
	    memcpy(wipe_scr, wipe_scr_start, ScreenWidth * ScreenHeight);
	}
    //Not translucent
    else if (i>8)
	{
	    memcpy(wipe_scr, wipe_scr_end, ScreenWidth * ScreenHeight);
	}
	else
	{
    	if (i>4)
		{
       		i = 8 - i;
        	invert = true;
		}
		trtable = tinttables[i];
        s = wipe_scr_start;
        e = wipe_scr_end;
        w = wipe_scr;
 
    	if (!invert)
			for (i=0; i<ScreenWidth * ScreenHeight; i++)
			{
				*w = trtable[(*s << 8) + *e];
                s++;
                e++;
				w++;
			}
		else
			for (i=0; i<ScreenWidth * ScreenHeight; i++)
			{
				*w = trtable[(*e << 8) + *s];
                s++;
                e++;
                w++;
			}
	}
 	return wipe_translucency >= 100;
}

//==========================================================================
//
//  wipe_exitTranslucent
//
//==========================================================================

static int wipe_exitTranslucent(void)
{
    return 0;
}

