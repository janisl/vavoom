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

struct TSwitch
{
	int Tex1;
	int Tex2;
	int Sound;
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

//	Switches
static TArray<TSwitch>		Switches;

//	Buttons
static TArray<TButton>		ButtonList;

//	Terrain types, maby not right place for them
static TArray<TTerrainType>	TerrainTypes;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	P_InitSwitchList
//
//	Only called at game initialization.
//
//==========================================================================

void P_InitSwitchList(void)
{
	int t1;
	int t2;

	SC_Open("switches");
	while (SC_GetString())
	{
		t1 = R_CheckTextureNumForName(sc_String);
		SC_MustGetString();
		t2 = R_CheckTextureNumForName(sc_String);
		SC_MustGetString();
    	if ((t1 < 0) || (t2 < 0))
		{
			continue;
		}
		TSwitch *sw = new(Switches) TSwitch;
		sw->Sound = S_GetSoundID(sc_String);
    	sw->Tex1 = t1;
    	sw->Tex2 = t2;
	}
	SC_Close();
	Switches.Shrink();
}

//==========================================================================
//
//  P_ClearButtons
//
//==========================================================================

void P_ClearButtons(void)
{
	ButtonList.Empty();
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
	int sidenum = line->sidenum[0];
    int texTop = level.sides[sidenum].toptexture;
    int texMid = level.sides[sidenum].midtexture;
    int texBot = level.sides[sidenum].bottomtexture;

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

		SV_SectorStartSound(level.sides[sidenum].sector, sw->Sound, 0, 127);
		SV_SetLineTexture(sidenum, where, toTex);
	    if (useAgain)
		{
			P_StartButton(sidenum, where, fromTex, BUTTONTIME);
		}
	    return;
    }
}

//==========================================================================
//
//	P_UpdateButtons
//
//==========================================================================

void P_UpdateButtons(void)
{
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
	SC_Open("TERRAINS");
	while (SC_GetString())
	{
		int		pic;

		pic = R_CheckFlatNumForName(sc_String);
		if (pic == -1)
		{
			pic = R_CheckTextureNumForName(sc_String);
		}
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
}

//==========================================================================
//
//	SV_TerrainType
//
//==========================================================================

int SV_TerrainType(int pic)
{
	for (TArray<TTerrainType>::TIterator tt(TerrainTypes); tt; ++tt)
	{
		if (tt->Pic == pic)
		{
			return tt->Type;
		}
	}
	return 0;
}

//**************************************************************************
//
//	$Log$
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
