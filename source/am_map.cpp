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
//**	The automap code
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// player radius for movement checking
#define PLAYERRADIUS	16.0
#define MAPBLOCKUNITS	128

#define AM_W			320
#define AM_H			(200 - sb_height)

// scale on entry
#define INITSCALEMTOF		0.2

#define AMSTR_FOLLOWON		"Follow Mode ON"
#define AMSTR_FOLLOWOFF		"Follow Mode OFF"

#define AMSTR_GRIDON		"Grid ON"
#define AMSTR_GRIDOFF		"Grid OFF"

#define AMSTR_MARKEDSPOT	"Marked Spot"
#define AMSTR_MARKSCLEARED	"All Marks Cleared"

#define AM_STARTKEY		K_TAB
#define AM_PANUPKEY		K_UPARROW
#define AM_PANDOWNKEY	K_DOWNARROW
#define AM_PANLEFTKEY	K_LEFTARROW
#define AM_PANRIGHTKEY	K_RIGHTARROW
#define AM_ZOOMINKEY	'='
#define AM_ZOOMOUTKEY	'-'
#define AM_ENDKEY		K_TAB
#define AM_GOBIGKEY		'0'
#define AM_FOLLOWKEY	'f'
#define AM_GRIDKEY		'g'
#define AM_MARKKEY		'm'
#define AM_CLEARMARKKEY	'c'

// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC	4
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        1.02
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       (1.0 / 1.02)

#define AM_NUMMARKPOINTS	10

// translates between frame-buffer and map distances
#define FTOM(x)			((float)(x) * scale_ftom)
#define MTOF(x)			((int)((x) * scale_mtof))
// translates between frame-buffer and map coordinates
#define CXMTOF(x)		(f_x + MTOF((x) - m_x))
#define CYMTOF(y)		(f_y + (f_h - MTOF((y) - m_y)))

// Automap colors
#define WALLCOLORS			REDS
#define TSWALLCOLORS		GRAYS
#define FDWALLCOLORS		BROWNS
#define CDWALLCOLORS		YELLOWS
#define THINGCOLORS			GREENS
#define SECRETWALLCOLORS	WALLCOLORS
#define TELEPORTCOLORS		(Game < Heretic ? WALLCOLORS + 8 : WALLCOLORS)
#define GRIDCOLORS			BLUES

// the following is crap
#define LINE_NEVERSEE		ML_DONTDRAW

#define NUMALIAS			3 // Number of antialiased lines.

// TYPES -------------------------------------------------------------------

struct fpoint_t
{
	int			x;
	int			y;
};

struct fline_t
{
	fpoint_t	a;
	fpoint_t	b;
};

struct mpoint_t
{
	float		x;
	float		y;
};

struct mline_t
{
	mpoint_t	a;
	mpoint_t	b;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean    		automapactive = false;

int 			am_cheating = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// For use if I do walls with outsides/insides
static int		REDS;
static int		BLUES;
static int		GREENS;
static int		GRAYS;
static int		BROWNS;
static int		YELLOWS;
static int		BLACK;
static int		WHITE;
static int		PARCH;
static int		BLOODRED;
static int		BLUEKEY;
static int		YELLOWKEY;
static int		GREENKEY;

static int 		their_colors[MAXPLAYERS];

static int 		grid = 0;

static int 		leveljuststarted = 1; 	// kluge until AM_LevelInit() is called

// location of window on screen
static int 		f_x;
static int		f_y;

// size of window on screen
static int 		f_w;
static int		f_h;

static mpoint_t	m_paninc; // how far the window pans each tic (map coords)
static float	mtof_zoommul; // how far the window zooms in each tic (map coords)
static float	ftom_zoommul; // how far the window zooms in each tic (fb coords)

static float	m_x;	// LL x,y where the window is on the map (map coords)
static float	m_y;
static float	m_x2;	// UR x,y where the window is on the map (map coords)
static float	m_y2;

// width/height of window on map (map coords)
static float	m_w;
static float	m_h;

// based on level size
static float	min_x;
static float	min_y;
static float	max_x;
static float	max_y;

static float	max_w; // max_x-min_x,
static float	max_h; // max_y-min_y

// based on player size
static float	min_w;
static float	min_h;


static float	min_scale_mtof; // used to tell when to stop zooming out
static float	max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static float	old_m_x;
static float	old_m_y;
static float	old_m_w;
static float	old_m_h;

// old location used by the Follower routine
static mpoint_t	f_oldloc;

// used by MTOF to scale from map-to-frame-buffer coords
static float	scale_mtof = INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static float	scale_ftom;

static float	start_scale_mtof = INITSCALEMTOF;

static mpoint_t	oldplr;

static boolean	use_marks = false;
static int		marknums[10]; // numbers used for marking by the automap
static mpoint_t	markpoints[AM_NUMMARKPOINTS]; // where the points are
static int 		markpointnum = 0; // next point to be assigned

static TCvarI	followplayer("followplayer", "1", CVAR_ARCHIVE); // specifies whether to follow the player around

static int		mappic;
static int		mapheight;
static short	mapystart=0; // y-value for the start of the map bitmap...used in the paralax stuff.
static short	mapxstart=0; //x-value for the bitmap.

static boolean	stopped = true;

static byte antialias0[NUMALIAS][8] =
{
	{176, 178, 180, 182, 184, 186, 188, 190},
	{64, 66, 68, 70, 72, 74, 76, 78},
	{231, 161, 162, 163, 164, 165, 166, 167}
};
static byte antialias1[NUMALIAS][8] =
{
	{96, 97, 98, 99, 100, 101, 102, 103},
	{110, 109, 108, 107, 106, 105, 104, 103},
	{75, 76, 77, 78, 79, 80, 81, 103}
};
static byte antialias2[NUMALIAS][8] =
{
	{ 83, 84, 85, 86, 87, 88, 89, 90 },
	{ 96, 96, 95, 94, 93, 92, 91, 90 },
	{ 107, 108, 109, 110, 111, 112, 89, 90 }
};
static byte antialias[NUMALIAS][8];

//
//	The vector graphics for the automap.
//

//
//  A line drawing of the player pointing right, starting from the middle.
//
#define R	(8.0 * PLAYERRADIUS / 7.0)
static mline_t player_arrow1[] =
{
    { { -R + R / 8.0, 0.0 }, { R, 0.0 } }, // -----
    { { R, 0.0 }, { R - R / 2.0, R / 4.0 } },  // ----->
    { { R, 0.0 }, { R - R / 2.0, -R / 4.0 } },
    { { -R + R / 8.0, 0.0 }, { -R - R / 8.0, R / 4.0 } }, // >---->
    { { -R + R / 8.0, 0.0 }, { -R - R / 8.0, -R / 4.0 } },
    { { -R + 3.0 * R / 8.0, 0.0 }, { -R + R / 8.0, R / 4.0 } }, // >>--->
    { { -R + 3.0 * R / 8.0, 0.0 }, { -R + R / 8.0, -R / 4.0 } }
};
#define NUMPLYRLINES1 (sizeof(player_arrow1) / sizeof(mline_t))

static mline_t player_arrow2[] =
{
  { { -R + R / 4.0, 0.0 }, { 0.0, 0.0} }, // center line.
  { { -R + R / 4.0, R / 8.0 }, { R, 0.0} }, // blade
  { { -R + R / 4.0, -R / 8.0 }, { R, 0.0 } },
  { { -R + R / 4.0, -R / 4.0 }, { -R + R / 4.0, R / 4.0 } }, // crosspiece
  { { -R + R / 8.0, -R / 4.0 }, { -R + R / 8.0, R / 4.0 } },
  { { -R + R / 8.0, -R / 4.0 }, { -R + R / 4.0, -R / 4.0 } }, //crosspiece connectors
  { { -R + R / 8.0, R / 4.0 }, { -R + R / 4.0, R / 4.0 } },
  { { -R - R / 4.0, R / 8.0 }, { -R - R / 4.0, -R / 8.0 } }, //pommel
  { { -R - R / 4.0, R / 8.0 }, { -R + R / 8.0, R / 8.0 } },
  { { -R - R / 4.0, -R / 8 }, { -R + R / 8.0, -R / 8.0 } }
};
#define NUMPLYRLINES2 (sizeof(player_arrow2) / sizeof(mline_t))

#undef R

static mline_t	*player_arrow;
static int		NUMPLYRLINES;

#define R	1.0
static mline_t thintriangle_guy[] =
{
    { { -.5 * R, -.7 * R }, { R, 0.0 } },
    { { R, 0.0 }, { -.5 * R, .7 * R } },
    { { -.5 * R, .7 * R }, { -.5 * R, -.7 * R } }
};
#undef R
#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy) / sizeof(mline_t))

static TCvar		ShowKills("ShowKills", "0");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	AM_Init
//
//==========================================================================

void AM_Init(void)
{
    switch (Game)
    {
     case Doom:
     case Doom2:
		REDS = (256 - 5 * 16);
		BLUES = (256 - 4 * 16 + 8);
		GREENS = (7 * 16);
		GRAYS = (6 * 16);
		BROWNS = (4 * 16);
		YELLOWS = (256 - 32 + 7);
		BLACK = 0;
		WHITE = (256 - 47);

	    their_colors[0] = GREENS;
	    their_colors[1] = GRAYS;
	    their_colors[2] = BROWNS;
	    their_colors[3] = REDS;

		player_arrow = player_arrow1;
		NUMPLYRLINES = NUMPLYRLINES1;
		memcpy(antialias, antialias0, sizeof(antialias));
     	break;

     case Heretic:
		REDS = (12 * 8);
		BLUES = (256 - 4 * 16 + 8);
		GREENS = (33 * 8);
		GRAYS = (5 * 8);
		BROWNS = (14 * 8);
		YELLOWS = (10 * 8);
		BLACK = 0;
		WHITE = (4 * 8);
		PARCH = (13 * 8 - 1);
		BLOODRED = 150;
		BLUEKEY = 197;
		YELLOWKEY = 144;
		GREENKEY = 220;

		their_colors[0] = GREENKEY;
		their_colors[1] = YELLOWKEY;
		their_colors[2] = BLOODRED;
		their_colors[3] = BLUEKEY;

		player_arrow = player_arrow2;
		NUMPLYRLINES = NUMPLYRLINES2;
		memcpy(antialias, antialias1, sizeof(antialias));
     	break;

     case Hexen:
		REDS = 12*8;
		BLUES = (256-4*16+8);
		GREENS = (33*8);
		GRAYS = (5*8);
		BROWNS = (14*8);
		YELLOWS = 10*8;
		BLACK = 0;
		WHITE = 4 * 8;
		PARCH = 13*8-1;
		BLOODRED = 177;
		BLUEKEY = 157;
		YELLOWKEY = 137;
		GREENKEY = 198;

		their_colors[0] = 157; // Blue
		their_colors[0] = 177; // Red
		their_colors[0] = 137; // Yellow
		their_colors[0] = 198; // Green
		their_colors[0] = 215; // Jade
		their_colors[0] = 32;  // White
		their_colors[0] = 106; // Hazel
		their_colors[0] = 234; // Purple

		player_arrow = player_arrow2;
		NUMPLYRLINES = NUMPLYRLINES2;
		memcpy(antialias, antialias2, sizeof(antialias));
     	break;

	 case Strife:
		break;
    }
}

//==========================================================================
//
//  AM_activateNewScale
//
//==========================================================================

static void AM_activateNewScale(void)
{
    m_x += m_w / 2.0;
    m_y += m_h / 2.0;
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    m_x -= m_w / 2.0;
    m_y -= m_h / 2.0;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

//==========================================================================
//
//  AM_saveScaleAndLoc
//
//==========================================================================

static void AM_saveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

//==========================================================================
//
//  AM_restoreScaleAndLoc
//
//==========================================================================

static void AM_restoreScaleAndLoc(void)
{
    m_w = old_m_w;
    m_h = old_m_h;
    if (!followplayer)
    {
		m_x = old_m_x;
		m_y = old_m_y;
    }
    else
    {
		m_x = cl.vieworg.x - m_w / 2.0;
		m_y = cl.vieworg.y - m_h / 2.0;
    }
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

    // Change the scaling multipliers
    scale_mtof = (float)f_w / m_w;
    scale_ftom = 1.0 / scale_mtof;
}

//==========================================================================
//
//  AM_findMinMaxBoundaries
//
//	Determines bounding box of all vertices, sets global variables
// controlling zoom range.
//
//==========================================================================

static void AM_findMinMaxBoundaries(void)
{
    int i;
	float	a;
	float	b;

    min_x = min_y =  99999.0;
    max_x = max_y = -99999.0;
  
    for (i=0;i<cl_level.numvertexes;i++)
    {
		if (cl_level.vertexes[i].x < min_x)
		    min_x = cl_level.vertexes[i].x;
		else if (cl_level.vertexes[i].x > max_x)
		    max_x = cl_level.vertexes[i].x;
    
		if (cl_level.vertexes[i].y < min_y)
		    min_y = cl_level.vertexes[i].y;
		else if (cl_level.vertexes[i].y > max_y)
		    max_y = cl_level.vertexes[i].y;
    }
  
    max_w = max_x - min_x;
    max_h = max_y - min_y;

    min_w = 2.0 * PLAYERRADIUS; // const? never changed?
    min_h = 2.0 * PLAYERRADIUS;

    a = (float)f_w / max_w;
    b = (float)f_h / max_h;
  
    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = (float)f_h / (2.0 * PLAYERRADIUS);
}

//==========================================================================
//
//  AM_changeWindowLoc
//
//==========================================================================

static void AM_changeWindowLoc(void)
{
    if (m_paninc.x || m_paninc.y)
    {
		followplayer = 0;
		f_oldloc.x = 99999.0;
    }

    m_x += m_paninc.x;
    m_y += m_paninc.y;

	if (m_x + m_w / 2.0 > max_x)
	{
  		m_x = max_x - m_w / 2.0;
	}
	else if (m_x + m_w / 2.0 < min_x)
	{
  		m_x = min_x - m_w / 2.0;
	}
	if (m_y + m_h / 2.0 > max_y)
	{
  		m_y = max_y - m_h / 2.0;
	}
	else if (m_y + m_h / 2.0 < min_y)
	{
  		m_y = min_y - m_h / 2.0;
	}

	m_x2 = m_x + m_w;
	m_y2 = m_y + m_h;
}

//==========================================================================
//
//  AM_addMark
//
//	adds a marker at the current location
//
//==========================================================================

static void AM_addMark(void)
{
    markpoints[markpointnum].x = m_x + m_w / 2.0;
    markpoints[markpointnum].y = m_y + m_h / 2.0;
    markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;
}

//==========================================================================
//
//  AM_clearMarks
//
//==========================================================================

static void AM_clearMarks(void)
{
    int		i;

    for (i=0; i<AM_NUMMARKPOINTS; i++)
		markpoints[i].x = -1.0; // means empty
    markpointnum = 0;
}

//==========================================================================
//
//  AM_initVariables
//
//==========================================================================

static void AM_initVariables(void)
{
    automapactive = true;

    f_oldloc.x = 99999.0;

    m_paninc.x = m_paninc.y = 0.0;
    ftom_zoommul = 1.0;
    mtof_zoommul = 1.0;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

	oldplr.x = cl.vieworg.x;
	oldplr.y = cl.vieworg.y;
    m_x = cl.vieworg.x - m_w / 2.0;
    m_y = cl.vieworg.y - m_h / 2.0;
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

//==========================================================================
//
//  AM_loadPics
//
//==========================================================================

static void AM_loadPics(void)
{
    int		i;

	if (W_CheckNumForName("AMMNUM0") >= 0)
    {
	    for (i = 0; i < 10; i++)
    	{
			marknums[i] = R_RegisterPic(va("AMMNUM%d", i), PIC_PATCH);
    	}
		use_marks = true;
	}

	mappic = R_RegisterPic("AUTOPAGE", PIC_RAW);
	mapheight = W_LumpLength(W_GetNumForName("AUTOPAGE")) / 320;
}

//==========================================================================
//
//  AM_LevelInit
//
//	should be called at the start of every level
//	right now, i figure it out myself
//
//==========================================================================

static void AM_LevelInit(void)
{
    leveljuststarted = 0;

    f_x = f_y = 0;
    f_w = ScreenWidth;
    f_h = ScreenHeight - SB_REALHEIGHT;

   	AM_clearMarks();
	mapxstart = mapystart = 0;

    AM_findMinMaxBoundaries();
    scale_mtof = min_scale_mtof / 0.7;
    if (scale_mtof > max_scale_mtof)
		scale_mtof = min_scale_mtof;
    scale_ftom = 1.0 / scale_mtof;
	start_scale_mtof = scale_mtof;
}

//==========================================================================
//
//  AM_Stop
//
//==========================================================================

void AM_Stop(void)
{
    automapactive = false;
    stopped = true;
}

//==========================================================================
//
//  AM_Start
//
//==========================================================================

static void AM_Start(void)
{
    static char lastmap[12] = "";

    if (!stopped)
    	AM_Stop();
   	stopped = false;
    if (strcmp(lastmap, cl_level.mapname))
    {
		AM_LevelInit();
		strcpy(lastmap, cl_level.mapname);
    }
    AM_initVariables();
    AM_loadPics();
}

//==========================================================================
//
//  AM_minOutWindowScale
//
//	set the window scale to the maximum size
//
//==========================================================================

static void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = 1.0 / scale_mtof;
    AM_activateNewScale();
}

//==========================================================================
//
//  AM_maxOutWindowScale
//
// 	set the window scale to the minimum size
//
//==========================================================================

static void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = 1.0 / scale_mtof;
    AM_activateNewScale();
}

//==========================================================================
//
//  AM_Responder
//
//	Handle events (user inputs) in automap mode
//
//==========================================================================

boolean AM_Responder(event_t* ev)
{
    int 		rc;
    static int	bigstate=0;

    rc = false;

    if (!automapactive)
    {
		if (ev->type == ev_keydown && ev->data1 == AM_STARTKEY)
		{
	    	AM_Start();
		    rc = true;
		}
    }

    else if (ev->type == ev_keydown)
    {

		rc = true;
		switch (ev->data1)
		{
		 case AM_PANRIGHTKEY: // pan right
	    	if (!followplayer) m_paninc.x = FTOM(F_PANINC);
		    else rc = false;
		    break;
		 case AM_PANLEFTKEY: // pan left
		    if (!followplayer) m_paninc.x = -FTOM(F_PANINC);
	    	else rc = false;
		    break;
		 case AM_PANUPKEY: // pan up
	    	if (!followplayer) m_paninc.y = FTOM(F_PANINC);
		    else rc = false;
		    break;
		 case AM_PANDOWNKEY: // pan down
		    if (!followplayer) m_paninc.y = -FTOM(F_PANINC);
	    	else rc = false;
		    break;
		 case AM_ZOOMOUTKEY: // zoom out
	    	mtof_zoommul = M_ZOOMOUT;
		    ftom_zoommul = M_ZOOMIN;
		    break;
		 case AM_ZOOMINKEY: // zoom in
		    mtof_zoommul = M_ZOOMIN;
	    	ftom_zoommul = M_ZOOMOUT;
		    break;
		 case AM_ENDKEY:
	    	bigstate = 0;
		    AM_Stop();
	    	break;
		 case AM_GOBIGKEY:
		    bigstate = !bigstate;
	    	if (bigstate)
		    {
				AM_saveScaleAndLoc();
				AM_minOutWindowScale();
	    	}
		    else AM_restoreScaleAndLoc();
		    break;
		 case AM_FOLLOWKEY:
		    followplayer = !followplayer;
	    	f_oldloc.x = 99999.0;
		    con << (followplayer ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF) << endl;
		    break;
		 case AM_GRIDKEY:
		    grid = !grid;
	    	con << (grid ? AMSTR_GRIDON : AMSTR_GRIDOFF) << endl;
		    break;
		 case AM_MARKKEY:
         	if (use_marks)
            {
		    	con << AMSTR_MARKEDSPOT << " " << markpointnum << endl;
			    AM_addMark();
			}
            else
            {
				rc = false;
            }
    		break;
		 case AM_CLEARMARKKEY:
         	if (use_marks)
            {
			    AM_clearMarks();
		    	con << AMSTR_MARKSCLEARED << endl;
			}
            else
            {
            	rc = false;
            }
			break;
	     default:
			rc = false;
		}
    }

    else if (ev->type == ev_keyup)
    {
		rc = false;
		switch (ev->data1)
		{
		  case AM_PANRIGHTKEY:
		    if (!followplayer) m_paninc.x = 0.0;
		    break;
		  case AM_PANLEFTKEY:
		    if (!followplayer) m_paninc.x = 0.0;
		    break;
		  case AM_PANUPKEY:
		    if (!followplayer) m_paninc.y = 0.0;
		    break;
		  case AM_PANDOWNKEY:
		    if (!followplayer) m_paninc.y = 0.0;
		    break;
		  case AM_ZOOMOUTKEY:
		  case AM_ZOOMINKEY:
		    mtof_zoommul = 1.0;
		    ftom_zoommul = 1.0;
		    break;
		}
    }
    return rc;
}

//==========================================================================
//
//  AM_changeWindowScale
//
//	Zooming
//
//==========================================================================

static void AM_changeWindowScale(void)
{
    // Change the scaling multipliers
    scale_mtof = scale_mtof * mtof_zoommul;
    scale_ftom = 1.0 / scale_mtof;

    if (scale_mtof < min_scale_mtof)
		AM_minOutWindowScale();
    else if (scale_mtof > max_scale_mtof)
		AM_maxOutWindowScale();
    else
		AM_activateNewScale();
}

//==========================================================================
//
//  AM_doFollowPlayer
//
//==========================================================================

static void AM_doFollowPlayer(void)
{
    if (f_oldloc.x != cl.vieworg.x || f_oldloc.y != cl.vieworg.y)
    {
		m_x = FTOM(MTOF(cl.vieworg.x)) - m_w / 2.0;
		m_y = FTOM(MTOF(cl.vieworg.y)) - m_h / 2.0;
		m_x2 = m_x + m_w;
		m_y2 = m_y + m_h;
		f_oldloc.x = cl.vieworg.x;
		f_oldloc.y = cl.vieworg.y;
    }
}

//==========================================================================
//
//  AM_Ticker
//
//	Updates on Game Tick
//
//==========================================================================

void AM_Ticker(void)
{
    if (!automapactive)
		return;

    if (followplayer)
		AM_doFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != 1.0)
		AM_changeWindowScale();

    // Change x,y location
    if (m_paninc.x || m_paninc.y)
		AM_changeWindowLoc();
}

//==========================================================================
//
//  AM_clearFB
//
//	Clear automap frame buffer.
//
//==========================================================================

static void AM_clearFB(void)
{
	int dmapx;
	int dmapy;

	if (followplayer)
	{
		dmapx = MTOF(cl.vieworg.x) - MTOF(oldplr.x);
		dmapy = MTOF(oldplr.y) - MTOF(cl.vieworg.y);

		oldplr.x = cl.vieworg.x;
		oldplr.y = cl.vieworg.y;
		mapxstart -= dmapx>>1;
		mapystart -= dmapy>>1;

  		while (mapxstart >= AM_W)
			mapxstart -= AM_W;
		while (mapxstart < 0)
			mapxstart += AM_W;
		while (mapystart >= mapheight)
			mapystart -= mapheight;
		while (mapystart < 0)
			mapystart += mapheight;
	}
	else
	{
		mapxstart -= MTOF(m_paninc.x) >> 1;
		mapystart += MTOF(m_paninc.y) >> 1;
		if (mapxstart >= AM_W)
			mapxstart -= AM_W;
		if (mapxstart < 0)
			mapxstart += AM_W;
		if (mapystart >= mapheight)
			mapystart -= mapheight;
		if (mapystart < 0)
			mapystart += mapheight;
	}

	//blit the automap background to the screen.
	for (int y = mapystart - mapheight; y < AM_H; y += mapheight)
	{
		for (int x = mapxstart - AM_W; x < AM_W; x += AM_W)
		{
			R_DrawPic(x, y, mappic, 0);
		}
	}
}

//==========================================================================
//
//  AM_clipMline
//
// 	Automap clipping of lines.
//
// 	Based on Cohen-Sutherland clipping algorithm but with a slightly faster
// reject and precalculated slopes. If the speed is needed, use a hash
// algorithm to handle the common cases.
//
//==========================================================================

static boolean AM_clipMline(mline_t* ml, fline_t* fl)
{
    enum
    {
		LEFT	= 1,
		RIGHT	= 2,
		BOTTOM	= 4,
		TOP		= 8
    };
    
    register int	outcode1 = 0;
    register int	outcode2 = 0;
    register int	outside;
    
    fpoint_t		tmp;
    int				dx;
    int				dy;


#define DOOUTCODE(oc, mx, my) \
    (oc) = 0; \
    if ((my) < 0) (oc) |= TOP; \
    else if ((my) >= f_h) (oc) |= BOTTOM; \
    if ((mx) < 0) (oc) |= LEFT; \
    else if ((mx) >= f_w) (oc) |= RIGHT;

    
    // do trivial rejects and outcodes
    if (ml->a.y > m_y2)
		outcode1 = TOP;
    else if (ml->a.y < m_y)
		outcode1 = BOTTOM;

    if (ml->b.y > m_y2)
		outcode2 = TOP;
    else if (ml->b.y < m_y)
		outcode2 = BOTTOM;
    
    if (outcode1 & outcode2)
		return false; // trivially outside

    if (ml->a.x < m_x)
		outcode1 |= LEFT;
    else if (ml->a.x > m_x2)
		outcode1 |= RIGHT;
    
    if (ml->b.x < m_x)
		outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
		outcode2 |= RIGHT;
    
    if (outcode1 & outcode2)
		return false; // trivially outside

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);

    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    DOOUTCODE(outcode2, fl->b.x, fl->b.y);

    if (outcode1 & outcode2)
		return false;

    while (outcode1 | outcode2)
    {
		// may be partially inside box
		// find an outside point
		if (outcode1)
		    outside = outcode1;
		else
		    outside = outcode2;
	
		// clip to each side
		if (outside & TOP)
		{
		    dy = fl->a.y - fl->b.y;
	    	dx = fl->b.x - fl->a.x;
		    tmp.x = fl->a.x + (dx*(fl->a.y))/dy;
		    tmp.y = 0;
		}
		else if (outside & BOTTOM)
		{
		    dy = fl->a.y - fl->b.y;
	    	dx = fl->b.x - fl->a.x;
		    tmp.x = fl->a.x + (dx*(fl->a.y-f_h))/dy;
		    tmp.y = f_h-1;
		}
		else if (outside & RIGHT)
		{
		    dy = fl->b.y - fl->a.y;
	    	dx = fl->b.x - fl->a.x;
		    tmp.y = fl->a.y + (dy*(f_w-1 - fl->a.x))/dx;
		    tmp.x = f_w-1;
		}
		else if (outside & LEFT)
		{
		    dy = fl->b.y - fl->a.y;
	    	dx = fl->b.x - fl->a.x;
		    tmp.y = fl->a.y + (dy*(-fl->a.x))/dx;
		    tmp.x = 0;
		}

		if (outside == outcode1)
		{
		    fl->a = tmp;
	    	DOOUTCODE(outcode1, fl->a.x, fl->a.y);
		}
		else
		{
		    fl->b = tmp;
	    	DOOUTCODE(outcode2, fl->b.x, fl->b.y);
		}
	
		if (outcode1 & outcode2)
		    return false; // trivially outside
    }

    return true;
}
#undef DOOUTCODE

//==========================================================================
//
//	AM_drawFline
//
//	Classic Bresenham w/ whatever optimizations needed for speed
//
//==========================================================================

static void AM_drawFline(fline_t* fl, int color)
{
	Drawer->DrawLine(fl->a.x, fl->a.y, color, fl->b.x, fl->b.y, color);
}

//==========================================================================
//
//  AM_drawMline
//
//	Clip lines, draw visible part sof lines.
//
//==========================================================================

static void AM_drawMline(mline_t *ml, int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
		AM_drawFline(&fl, color); // draws it on frame buffer using fb coords
}

//==========================================================================
//
//  AM_drawGrid
//
//	Draws flat (floor/ceiling tile) aligned grid lines.
//
//==========================================================================

static void AM_drawGrid(int color)
{
	float	x, y;
	float	start, end;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if ((FX(start - cl_level.bmaporgx))%(MAPBLOCKUNITS<<FRACBITS))
		start += FL((MAPBLOCKUNITS<<FRACBITS)
		    - ((FX(start - cl_level.bmaporgx))%(MAPBLOCKUNITS<<FRACBITS)));
    end = m_x + m_w;

    // draw vertical gridlines
    ml.a.y = m_y;
    ml.b.y = m_y + m_h;
    for (x = start; x < end; x += (float)MAPBLOCKUNITS)
    {
		ml.a.x = x;
		ml.b.x = x;
		AM_drawMline(&ml, color);
    }

    // Figure out start of horizontal gridlines
    start = m_y;
    if ((FX(start - cl_level.bmaporgy))%(MAPBLOCKUNITS<<FRACBITS))
		start += FL((MAPBLOCKUNITS<<FRACBITS)
		    - ((FX(start - cl_level.bmaporgy))%(MAPBLOCKUNITS<<FRACBITS)));
    end = m_y + m_h;

    // draw horizontal gridlines
    ml.a.x = m_x;
    ml.b.x = m_x + m_w;
    for (y = start; y < end; y += (float)MAPBLOCKUNITS)
    {
		ml.a.y = y;
		ml.b.y = y;
		AM_drawMline(&ml, color);
    }

}

//==========================================================================
//
//  AM_drawWalls
//
//	Determines visible lines, draws them.
//	This is LineDef based, not LineSeg based.
//
//==========================================================================

static void AM_drawWalls(void)
{
    int 			i;
    static mline_t	l;

    for (i=0; i<cl_level.numlines; i++)
    {
		l.a.x = cl_level.lines[i].v1->x;
		l.a.y = cl_level.lines[i].v1->y;
		l.b.x = cl_level.lines[i].v2->x;
		l.b.y = cl_level.lines[i].v2->y;
		if (am_cheating || (cl_level.lines[i].flags & ML_MAPPED))
		{
		    if ((cl_level.lines[i].flags & LINE_NEVERSEE) && !am_cheating)
				continue;
		    if (!cl_level.lines[i].backsector)
		    {
				AM_drawMline(&l, WALLCOLORS);
		    }
		    else
		    {
            	switch (Game)
                {
                 case Doom:
                 case Doom2:
					if (cl_level.lines[i].special == 39)
					{
						// teleporters
					    AM_drawMline(&l, TELEPORTCOLORS);
					}
					else if (cl_level.lines[i].flags & ML_SECRET) // secret door
					{
					    if (am_cheating) AM_drawMline(&l, SECRETWALLCOLORS);
					    else AM_drawMline(&l, WALLCOLORS);
					}
					else if (cl_level.lines[i].backsector->floor.minz
						   != cl_level.lines[i].frontsector->floor.minz)
					{
					    AM_drawMline(&l, FDWALLCOLORS); // floor level change
					}
					else if (cl_level.lines[i].backsector->ceiling.maxz
						   != cl_level.lines[i].frontsector->ceiling.maxz)
					{
					    AM_drawMline(&l, CDWALLCOLORS); // ceiling level change
					}
					else if (am_cheating)
					{
					    AM_drawMline(&l, TSWALLCOLORS);
					}
                    break;

				 case Heretic:
					if (cl_level.lines[i].special == 39)
					{
						// teleporters
					    AM_drawMline(&l, TELEPORTCOLORS);
					}
					else if (cl_level.lines[i].flags & ML_SECRET) // secret door
					{
						if (am_cheating) AM_drawMline(&l, 0);
						else AM_drawMline(&l, WALLCOLORS);
					}
					else if(cl_level.lines[i].special > 25 && cl_level.lines[i].special < 35)
					{
						switch(cl_level.lines[i].special)
						{
						 case 26:
						 case 32:
							AM_drawMline(&l, BLUEKEY);
							break;
						 case 27:
						 case 34:
							AM_drawMline(&l, YELLOWKEY);
							break;
						 case 28:
						 case 33:
							AM_drawMline(&l, GREENKEY);
							break;
						 default:
							break;
						}
					}
					else if (cl_level.lines[i].backsector->floor.minz
						   != cl_level.lines[i].frontsector->floor.minz)
					{
					  AM_drawMline(&l, FDWALLCOLORS); // floor level change
					}
					else if (cl_level.lines[i].backsector->ceiling.maxz
					   != cl_level.lines[i].frontsector->ceiling.maxz)
					{
						AM_drawMline(&l, CDWALLCOLORS); // ceiling level change
					}
					else if (am_cheating)
					{
						AM_drawMline(&l, TSWALLCOLORS);
					}
                    break;

				 case Hexen:
					if (cl_level.lines[i].flags & ML_SECRET) // secret door
					{
						if (am_cheating) AM_drawMline(&l, 0);
						else AM_drawMline(&l, WALLCOLORS);
					}
					else if(cl_level.lines[i].special == 13 || cl_level.lines[i].special == 83)
					{ // Locked door line -- all locked doors are greed
						AM_drawMline(&l, GREENKEY);
					}
					else if(cl_level.lines[i].special == 70 || cl_level.lines[i].special == 71)
					{ // intra-level teleports are blue
						AM_drawMline(&l, BLUEKEY);
					}
					else if(cl_level.lines[i].special == 74 || cl_level.lines[i].special == 75)
					{ // inter-level teleport/game-winning exit -- both are red
						AM_drawMline(&l, BLOODRED);
					}
					else if (cl_level.lines[i].backsector->floor.minz
					   != cl_level.lines[i].frontsector->floor.minz)
					{
						AM_drawMline(&l, FDWALLCOLORS); // floor level change
					}
					else if (cl_level.lines[i].backsector->ceiling.maxz
					   != cl_level.lines[i].frontsector->ceiling.maxz)
					{
						AM_drawMline(&l, CDWALLCOLORS); // ceiling level change
					}
					else if (am_cheating)
					{
						AM_drawMline(&l, TSWALLCOLORS);
					}
                    break;

				 case Strife:
					break;
				}
		    }
		}
		else if (cl.items & IT_ALL_MAP)
		{
		    if (!(cl_level.lines[i].flags & LINE_NEVERSEE)) AM_drawMline(&l, GRAYS+3);
		}
    }
}

//==========================================================================
//
//  AM_rotate
//
//	Rotation in 2D. Used to rotate player arrow line character.
//
//==========================================================================

static void AM_rotate(float* x, float* y, angle_t a)
{
	float	tmpx;

    tmpx = *x * mcos(a) - *y * msin(a);
    *y   = *x * msin(a) + *y * mcos(a);
    *x = tmpx;
}

//==========================================================================
//
//  AM_drawLineCharacter
//
//==========================================================================

static void AM_drawLineCharacter(mline_t* lineguy, int lineguylines,
	float scale, angle_t angle, int color, float x, float y)
{
    int		i;
    mline_t	l;

    for (i=0;i<lineguylines;i++)
    {
		l.a.x = lineguy[i].a.x;
		l.a.y = lineguy[i].a.y;

		if (scale)
		{
	    	l.a.x = scale * l.a.x;
		    l.a.y = scale * l.a.y;
		}

		if (angle)
		    AM_rotate(&l.a.x, &l.a.y, angle);

		l.a.x += x;
		l.a.y += y;

		l.b.x = lineguy[i].b.x;
		l.b.y = lineguy[i].b.y;

		if (scale)
		{
	    	l.b.x = scale * l.b.x;
		    l.b.y = scale * l.b.y;
		}

		if (angle)
		    AM_rotate(&l.b.x, &l.b.y, angle);
	
		l.b.x += x;
		l.b.y += y;

		AM_drawMline(&l, color);
    }
}

//==========================================================================
//
//  AM_drawPlayers
//
//==========================================================================

static void AM_drawPlayers(void)
{
#ifdef FIXME
    int			i;
    player_t*	p;
    int			their_color = -1;
    int			color;

    if (!netgame)
    {
#endif
	    AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0.0,
	    	cl.viewangles.yaw, WHITE, cl.vieworg.x, cl.vieworg.y);
#ifdef FIXME
		return;
    }

    for (i=0;i<MAXPLAYERS;i++)
    {
		their_color++;
		p = &players[i];

		if ((deathmatch && !singledemo) && p != plr)
		    continue;

		if (!playeringame[i])
	    	continue;

/*		if (Game != Hexen && p->powers[pw_invisibility])
        {
        	if (Game != Heretic)
			    color = 246; // *close* to black
			else
	    		color = 102; // *close* to the automap color
		}
		else*/
        {
		    color = their_colors[their_color];
		}
	
		AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0.0, p->mo->angle,
			color, p->mo->x, p->mo->y);
    }
#endif
}

//==========================================================================
//
//  AM_drawThings
//
//==========================================================================

static void AM_drawThings(int colors)
{
    int			i;

	for (i = 0; i < MAX_MOBJS; i++)
	{
		if (cl_mobjs[i].in_use)
		{
		    AM_drawLineCharacter(thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
				16.0, cl_mobjs[i].angles.yaw, colors, cl_mobjs[i].origin.x,
				cl_mobjs[i].origin.y);
		}
	}
}

//==========================================================================
//
//  AM_drawMarks
//
//==========================================================================

static void AM_drawMarks(void)
{
    int i, fx, fy, w, h;

    for (i = 0; i < AM_NUMMARKPOINTS; i++)
    {
		if (markpoints[i].x != -1.0)
		{
		    //      w = LittleShort(marknums[i]->width);
		    //      h = LittleShort(marknums[i]->height);
		    w = 5; // because something's wrong with the wad, i guess
		    h = 6; // because something's wrong with the wad, i guess
		    fx = (int)(CXMTOF(markpoints[i].x) * fScaleXI);
		    fy = (int)(CYMTOF(markpoints[i].y) * fScaleXI);
		    if (fx >= f_x && fx <= f_w - w && fy >= f_y && fy <= f_h - h)
				R_DrawPic(fx, fy, marknums[i]);
		}
    }
}

//===========================================================================
//
//	DrawWorldTimer
//
//===========================================================================

static void DrawWorldTimer(void)
{
	int days;
	int hours;
	int minutes;
	int seconds;
	int worldTimer;
	char timeBuffer[15];
	char dayBuffer[20];

//FIXME
	worldTimer = 0;//players[consoleplayer].worldTimer;

	if (!worldTimer) return;

	worldTimer /= 35;
	days = worldTimer/86400;
	worldTimer -= days*86400;
	hours = worldTimer/3600;
	worldTimer -= hours*3600;
	minutes = worldTimer/60;
	worldTimer -= minutes*60;
	seconds = worldTimer;

	T_SetFont(font_small);
    T_SetAlign(hleft, vtop);
	sprintf(timeBuffer, "%.2d : %.2d : %.2d", hours, minutes,seconds);
    T_DrawString(240, 8, timeBuffer);

	if (days)
	{
		if (days==1)
		{
			sprintf(dayBuffer, "%.2d DAY", days);
		}
		else
		{
			sprintf(dayBuffer, "%.2d DAYS", days);
		}
	    T_DrawString(240, 20, dayBuffer);
		if (days >= 5)
		{
		    T_DrawString(230, 35, "YOU FREAK!!!");
		}
	}
}

//===========================================================================
//
//	AM_DrawDeathmatchStats
//
//===========================================================================

static void AM_DrawDeathmatchStats(void)
{
#ifdef FIXME
	int i, j, k, m;
	int fragCount[MAXPLAYERS];
	int order[MAXPLAYERS];
	char textBuffer[80];
	int yPosition;

	for(i = 0; i < MAXPLAYERS; i++)
	{
		fragCount[i] = 0;
		order[i] = -1;
	}
	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(!playeringame[i])
		{
			continue;
		}
		else
		{
			for(j = 0; j < MAXPLAYERS; j++)
			{
				if(playeringame[j])
				{
					fragCount[i] += players[i].frags[j];
				}
			}
			for(k = 0; k < MAXPLAYERS; k++)
			{
				if(order[k] == -1)
				{
					order[k] = i;
					break;
				}
				else if(fragCount[i] > fragCount[order[k]])
				{
					for(m = MAXPLAYERS-1; m > k; m--)
					{
						 order[m] = order[m-1];
					}
					order[k] = i;
					break;
				}
			}
		}
	}
	yPosition = 15;
	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(!playeringame[order[i]])
		{
			continue;
		}
		else
		{
			T_SetFont(font_small);
		    T_SetAlign(hleft, vtop);
		    T_DrawString(8, yPosition, PlayerName[order[i]]);
			sprintf(textBuffer, "%d", fragCount[order[i]]);
		    T_DrawString(80, yPosition, textBuffer);
			yPosition += 10;
		}
	}
#endif
}

//==========================================================================
//
//	AM_CheckVariables
//
//==========================================================================

static void AM_CheckVariables(void)
{
	float		a;
	float		b;
	float		old_mtof_zoommul;

	//	Check for screen resolution change
    if (f_w != ScreenWidth || f_h != ScreenHeight - SB_REALHEIGHT)
	{
		old_mtof_zoommul = mtof_zoommul;
		mtof_zoommul = scale_mtof / start_scale_mtof;

	    f_w = ScreenWidth;
    	f_h = ScreenHeight - SB_REALHEIGHT;

	    a = (float)f_w / max_w;
    	b = (float)f_h / max_h;
  
	    min_scale_mtof = a < b ? a : b;
    	max_scale_mtof = (float)f_h / (2.0 * PLAYERRADIUS);

	    scale_mtof = min_scale_mtof / 0.7;
	    if (scale_mtof > max_scale_mtof)
			scale_mtof = min_scale_mtof;
	    scale_ftom = 1.0 / scale_mtof;
		start_scale_mtof = scale_mtof;

		AM_changeWindowScale();

		mtof_zoommul = old_mtof_zoommul;
	}
}

//==========================================================================
//
//  AM_Drawer
//
//==========================================================================

void AM_Drawer(void)
{
    if (!automapactive)
    	return;

	AM_CheckVariables();
    AM_clearFB();
	Drawer->StartAutomap();
	if (grid) AM_drawGrid(GRIDCOLORS);
    AM_drawWalls();
    AM_drawPlayers();
    if (am_cheating == 2) AM_drawThings(THINGCOLORS);
	Drawer->EndAutomap();
    if (use_marks) AM_drawMarks();
	DrawWorldTimer();
	T_SetFont(font_small);
    T_SetAlign(hleft, vbottom);
	T_DrawText(20, 200 - sb_height - 7, cl_level.level_name);
	if (ShowKills.value && cl.maxclients > 1 && cl.deathmatch)
	{
		AM_DrawDeathmatchStats();
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
