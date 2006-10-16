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

class VButton : public VThinker
{
	DECLARE_CLASS(VButton, VThinker, 0)

	vint32		Side;
	vuint8		Where;
	vint32		Texture;
	float		Timer;

	void Tick(float);
	void Serialise(VStream&);
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

IMPLEMENT_CLASS(V, Button)

//	Terrain types, maby not right place for them
static TArray<TTerrainType>	TerrainTypes;

// CODE --------------------------------------------------------------------

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
	for (VThinker* Th = GLevel->ThinkerHead; Th; Th = Th->Next)
	{
		if (!Th->IsA(VButton::StaticClass()) ||
			(Th->GetFlags() & _OF_DelayedDestroy))
		{
			continue;
		}
		if (((VButton*)Th)->Side == sidenum)
		{
			return;
		}
	}

	VButton* But = Spawn<VButton>();
	GLevel->AddThinker(But);
	But->Side = sidenum;
	But->Where = w;
	But->Texture = texture;
	But->Timer = time;
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
		SV_SectorStartSound(GLevel->Sides[sidenum].sector, Sound, 0, 1, 1);
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
//	VButton::Tick
//
//==========================================================================

void VButton::Tick(float DeltaTime)
{
	guard(VButton::Tick);
	//  DO BUTTONS
	Timer -= DeltaTime;
	if (Timer <= 0.0)
	{
		SV_SetLineTexture(Side, Where, Texture);
		SetFlags(_OF_DelayedDestroy);
	}
	unguard;
}

//==========================================================================
//
//	VButton::Serialise
//
//==========================================================================

void VButton::Serialise(VStream& Strm)
{
	guard(VButton::Serialise);
	Super::Serialise(Strm);
	Strm << STRM_INDEX(Side)
		<< Where
		<< STRM_INDEX(Texture)
		<< Timer;
	unguard;
}

//**************************************************************************
//
//	Terrain types
//
//**************************************************************************

//==========================================================================
//
// ParseTerrainTypes
//
//==========================================================================

static void ParseTerrainTypes(VScriptParser* sc)
{
	guard(ParseTerrainTypes);
	while (!sc->AtEnd())
	{
		sc->ExpectName8();
		int pic = GTextureManager.CheckNumForName(sc->Name8, TEXTYPE_Flat,
			true, true);
		sc->ExpectNumber();
		if (pic != -1)
		{
			TTerrainType& tt = TerrainTypes.Alloc();
			tt.Pic = pic;
			tt.Type = sc->Number;
		}
	}
	delete sc;
	TerrainTypes.Condense();
	unguard;
}

//==========================================================================
//
// P_InitTerrainTypes
//
//==========================================================================

void P_InitTerrainTypes()
{
	guard(P_InitTerrainTypes);
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == "terrains")
		{
			ParseTerrainTypes(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)));
		}
	}
	//	Optionally parse script file.
	if (fl_devmode && FL_FileExists("scripts/terrains.txt"))
	{
		ParseTerrainTypes(new VScriptParser("scripts/terrains.txt",
			FL_OpenFileRead("scripts/terrains.txt")));
	}
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
