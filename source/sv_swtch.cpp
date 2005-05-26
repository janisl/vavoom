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
//**
//**	Switches, buttons. Two-state animation. Exits.
//**
//**************************************************************************

//==================================================================
//
//      CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
//==================================================================

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define BUTTONTIME	1.0					// 1 second

// TYPES -------------------------------------------------------------------

enum EBWhere
{
	SWITCH_TOP,
	SWITCH_MIDDLE,
	SWITCH_BOTTOM
};

struct TButton
{
    int		Side;
    EBWhere	Where;
    int		Texture;
    float	Timer;
};

struct TTerrainType
{
	int		Pic;
	int		Type;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

//	Buttons
static TArray<TButton>		ButtonList;

//	Terrain types, maby not right place for them
static TArray<TTerrainType>	TerrainTypes;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  P_ClearButtons
//
//==========================================================================

void P_ClearButtons(void)
{
	guard(P_ClearButtons);
	ButtonList.Empty();
	unguard;
}

//==========================================================================
//
//  P_StartButton
//
//	Start a button counting down till it turns off.
//
//==========================================================================

static void P_StartButton(int sidenum, EBWhere w, int texture, float time)
{
	guard(P_StartButton);
    // See if button is already pressed
    for (TArray<TButton>::TIterator b(ButtonList); b; ++b)
    {
		if (b->Side == sidenum)
		{
		    return;
		}
    }

    TButton *but = new(ButtonList) TButton;
	but->Side = sidenum;
	but->Where = w;
	but->Texture = texture;
	but->Timer = time;
	unguard;
}

//==========================================================================
//
//  P_ChangeSwitchTexture
//
//	Function that changes wall texture.
//	Tell it if switch is ok to use again (1=yes, it's a button).
//
//==========================================================================

void P_ChangeSwitchTexture(line_t* line, int useAgain)
{
	guard(P_ChangeSwitchTexture);
	int sidenum = line->sidenum[0];
    int texTop = GLevel->Sides[sidenum].toptexture;
    int texMid = GLevel->Sides[sidenum].midtexture;
    int texBot = GLevel->Sides[sidenum].bottomtexture;

    for (TArray<TSwitch>::TIterator sw(Switches); sw; ++sw)
    {
		int fromTex;
		int toTex;
		EBWhere where;

		if (sw->Tex1 == texTop)
		{
			fromTex = sw->Tex1;
			toTex = sw->Tex2;
			where = SWITCH_TOP;
		}
		else if (sw->Tex1 == texMid)
	    {
			fromTex = sw->Tex1;
			toTex = sw->Tex2;
			where = SWITCH_MIDDLE;
	    }
	    else if (sw->Tex1 == texBot)
		{
			fromTex = sw->Tex1;
			toTex = sw->Tex2;
			where = SWITCH_BOTTOM;
		}
		else if (sw->Tex2 == texTop)
		{
			fromTex = sw->Tex2;
			toTex = sw->Tex1;
			where = SWITCH_TOP;
		}
		else if (sw->Tex2 == texMid)
	    {
			fromTex = sw->Tex2;
			toTex = sw->Tex1;
			where = SWITCH_MIDDLE;
	    }
	    else if (sw->Tex2 == texBot)
		{
			fromTex = sw->Tex2;
			toTex = sw->Tex1;
			where = SWITCH_BOTTOM;
		}
		else
		{
			continue;
		}

		SV_SectorStartSound(GLevel->Sides[sidenum].sector, sw->Sound, 0, 127);
		SV_SetLineTexture(sidenum, where, toTex);
	    if (useAgain)
		{
			P_StartButton(sidenum, where, fromTex, BUTTONTIME);
		}
	    return;
    }
	unguard;
}

//==========================================================================
//
//	P_UpdateButtons
//
//==========================================================================

void P_UpdateButtons(void)
{
	guard(P_UpdateButtons);
	//  DO BUTTONS
	for (TArray<TButton>::TIterator b(ButtonList); b; ++b)
	{
		b->Timer -= host_frametime;
		if (b->Timer <= 0.0)
		{
			SV_SetLineTexture(b->Side, b->Where, b->Texture);
			b.RemoveCurrent();
		}
	}
	unguard;
}

//**************************************************************************
//
//	Terrain types
//
//**************************************************************************

//==========================================================================
//
// P_InitTerrainTypes
//
//==========================================================================

void P_InitTerrainTypes(void)
{
	guard(P_InitTerrainTypes);
	SC_Open("TERRAINS");
	while (SC_GetString())
	{
		int pic = GTextureManager.CheckNumForName(FName(sc_String,
			FNAME_AddLower8), TEXTYPE_Flat, true, true);
		SC_MustGetNumber();
		if (pic != -1)
		{
			TTerrainType *tt = new(TerrainTypes) TTerrainType;
			tt->Pic = pic;
			tt->Type = sc_Number;
		}
	}
	SC_Close();
	TerrainTypes.Shrink();
	unguard;
}

//==========================================================================
//
//	SV_TerrainType
//
//==========================================================================

int SV_TerrainType(int pic)
{
	guard(SV_TerrainType);
	for (TArray<TTerrainType>::TIterator tt(TerrainTypes); tt; ++tt)
	{
		if (tt->Pic == pic)
		{
			return tt->Type;
		}
	}
	return 0;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.11  2005/05/26 16:53:59  dj_jl
//	Created texture manager class
//
//	Revision 1.10  2005/05/03 15:00:11  dj_jl
//	Moved switch list, animdefs enhancements.
//	
//	Revision 1.9  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.8  2002/07/13 07:50:58  dj_jl
//	Added guarding.
//	
//	Revision 1.7  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/12/12 19:26:40  dj_jl
//	Added dynamic arrays
//	
//	Revision 1.5  2001/10/09 17:20:42  dj_jl
//	Fixed switch sounds
//	
//	Revision 1.4  2001/08/29 17:55:42  dj_jl
//	Added sound channels
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
