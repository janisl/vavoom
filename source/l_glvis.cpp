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
#include "../utils/glvis/glvis.h"

// MACROS ------------------------------------------------------------------

#define MESSAGE1	"VAVOOM IS NOW BUILDING THE VIS DATA..."
#define MESSAGE2	"THIS ONLY HAS TO BE DONE ONCE FOR THIS WAD"

#define BARX		32
#define BARY		96
#define BARW		(320 - 2 * BARX)
#define BARH		8

#define BARTEXTX	32
#define BARTEXTY	64

// TYPES -------------------------------------------------------------------

class TGLVisGUI : public TGLVis
{
 public:
	void DisplayMessage(const char *text, ...)
		__attribute__((format(printf, 2, 3)));
	void DisplayStartMap(const char *levelname);
	void DisplayBaseVisProgress(int count, int total);
	void DisplayPortalVisProgress(int count, int total);
	void DisplayMapDone(int accepts, int total);

	double lastprog;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TGLVisGUI	GLVis;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static float barx;
static float barw;
static float bary1;
static float bary2;
static float barborderw;
static float barborderh;

static TCvarI glvis_fast("glvis_fast", "1", CVAR_ARCHIVE);
static TCvarI glvis_noreject("glvis_noreject", "1", CVAR_ARCHIVE);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TGLVisGUI::DisplayMessage
//
//==========================================================================

void TGLVisGUI::DisplayMessage(const char *text, ...)
{
	va_list		args;
	char		message[256];

	va_start(args, text);
	vsprintf(message, text, args);
	va_end(args);
	GCon->Log(NAME_Dev, message);
}

//==========================================================================
//
//	TGLVisGUI::DisplayStartMap
//
//==========================================================================

void TGLVisGUI::DisplayStartMap(const char *name)
{
	Drawer->StartUpdate();

	Drawer->FillRect(0, 0, ScreenWidth, ScreenHeight, 0xff000000);

	T_SetFont(font_small);
	T_SetAlign(hcenter, vcenter);
	T_DrawText(160, 8, MESSAGE1);
	T_DrawText(160, 24, MESSAGE2);
	T_SetAlign(hleft, vtop);
	T_DrawText(BARTEXTX, BARTEXTY, va("Creating vis data for %s", name));

	Drawer->FillRect(barx - barborderw, bary1 - barborderh,
		barx + barw + barborderw, bary2 + barborderh, 0xffff0000);
	Drawer->FillRect(barx, bary1, barx + barw, bary2, 0xff000000);

	if (glvis_fast)
	{
		T_SetAlign(hcenter, vcenter);
		T_DrawText(160, 160, "Using fast mode\nIt's highly recomended that you rebuild\nfull vis data using standalone utility.");
	}

	Drawer->Update();
}

//==========================================================================
//
//	TGLVisGUI::DisplayBaseVisProgress
//
//==========================================================================

void TGLVisGUI::DisplayBaseVisProgress(int count, int total)
{
	if (count > 0 && count < total && Sys_Time() - lastprog < 0.2)
	{
		return;
	}
	Drawer->BeginDirectUpdate();
	Drawer->FillRect(barx, bary1, barx + count * barw / total, bary2, 0xff007700);
	Drawer->EndDirectUpdate();
	lastprog = Sys_Time();
}

//==========================================================================
//
//	TGLVisGUI::DisplayPortalVisProgress
//
//==========================================================================

void TGLVisGUI::DisplayPortalVisProgress(int count, int total)
{
	if (count > 0 && count < total && Sys_Time() - lastprog < 0.2)
	{
		return;
	}
	Drawer->BeginDirectUpdate();
	Drawer->FillRect(barx, bary1, barx + count * barw / total, bary2, 0xff00ff00);
	Drawer->EndDirectUpdate();
	lastprog = Sys_Time();
}

//==========================================================================
//
//	TGLVisGUI::DisplayMapDone
//
//==========================================================================

void TGLVisGUI::DisplayMapDone(int accepts, int total)
{
	GCon->Logf(NAME_Dev, "%d accepts, %d rejects, %d%",
		accepts, total - accepts, accepts * 100 / total);
}

//==========================================================================
//
//	GLVisMalloc
//
//==========================================================================

static void *GLVisMalloc(size_t size)
{
	return Z_Calloc(size);
}

//==========================================================================
//
//	GLVisFree
//
//==========================================================================

static void GLVisFree(void *ptr)
{
	Z_Free(ptr);
}

//==========================================================================
//
//	GLVis_BuildPVS
//
//==========================================================================

void GLVis_BuildPVS(const char *srcfile)
{
	try
	{
		barx = BARX * fScaleX;
		barw = BARW * fScaleX;
		bary1 = BARY * fScaleY;
		bary2 = (BARY + BARH) * fScaleY;
		barborderw = 2 * fScaleX;
		barborderh = 2 * fScaleY;

		GLVis.Malloc = GLVisMalloc;
		GLVis.Free = GLVisFree;
		GLVis.fastvis = !!glvis_fast;
		GLVis.no_reject = !!glvis_noreject;
		GLVis.Build(srcfile);
	}
	catch (GLVisError &e)
	{
		Sys_Error("%s", e.message);
	}
}

//==========================================================================
//
//	COMMAND glVIS
//
//==========================================================================

COMMAND(glVIS)
{
	if (Argc() > 1)
	{
		GLVis_BuildPVS(Argv(1));
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2003/03/08 12:08:04  dj_jl
//	Beautification.
//
//	Revision 1.6  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.5  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/10/27 07:47:21  dj_jl
//	Noreject option, fast mode warning
//	
//	Revision 1.3  2001/09/20 16:23:40  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/09/14 16:52:14  dj_jl
//	Added dynamic build of GWA file
//	
//	Revision 1.1  2001/09/12 17:37:47  dj_jl
//	Added glBSP and glVIS plugins
//	
//**************************************************************************
