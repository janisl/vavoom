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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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

void P_ClearButtons()
{
	guard(P_ClearButtons);
	ButtonList.Clear();
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
	for (int i = 0; i < ButtonList.Num(); i++)
    {
		if (ButtonList[i].Side == sidenum)
		{
		    return;
		}
    }

    TButton& but = ButtonList.Alloc();
	but.Side = sidenum;
	but.Where = w;
	but.Texture = texture;
	but.Timer = time;
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

void P_ChangeSwitchTexture(line_t* line, bool useAgain, VName DefaultSound)
{
	guard(P_ChangeSwitchTexture);
	int sidenum = line->sidenum[0];
	int texTop = GLevel->Sides[sidenum].toptexture;
	int texMid = GLevel->Sides[sidenum].midtexture;
	int texBot = GLevel->Sides[sidenum].bottomtexture;

	for (int  i = 0; i < Switches.Num(); i++)
	{
		int fromTex;
		int toTex;
		EBWhere where;
		TSwitch* sw = &Switches[i];

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

		int Sound = sw->Sound;
		if (!Sound)
		{
			Sound = GSoundManager->GetSoundID(DefaultSound);
		}
		SV_SectorStartSound(GLevel->Sides[sidenum].sector, Sound, 0, 127);
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

void P_UpdateButtons()
{
	guard(P_UpdateButtons);
	//  DO BUTTONS
	for (int i = 0; i < ButtonList.Num(); i++)
	{
		TButton& b = ButtonList[i];
		b.Timer -= host_frametime;
		if (b.Timer <= 0.0)
		{
			SV_SetLineTexture(b.Side, b.Where, b.Texture);
			ButtonList.RemoveIndex(i);
			i--;
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

void P_InitTerrainTypes()
{
	guard(P_InitTerrainTypes);
	SC_Open("TERRAINS");
	while (SC_GetString())
	{
		int pic = GTextureManager.CheckNumForName(VName(sc_String,
			VName::AddLower8), TEXTYPE_Flat, true, true);
		SC_MustGetNumber();
		if (pic != -1)
		{
			TTerrainType& tt = TerrainTypes.Alloc();
			tt.Pic = pic;
			tt.Type = sc_Number;
		}
	}
	SC_Close();
	TerrainTypes.Condense();
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
	for (int i = 0; i < TerrainTypes.Num(); i++)
	{
		if (TerrainTypes[i].Pic == pic)
		{
			return TerrainTypes[i].Type;
		}
	}
	return 0;
	unguard;
}

//==========================================================================
//
// P_FreeTerrainTypes
//
//==========================================================================

void P_FreeTerrainTypes()
{
	guard(P_FreeTerrainTypes);
	TerrainTypes.Clear();
	unguard;
}
