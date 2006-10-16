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
	vint32		SwitchDef;
	vint32		Frame;
	float		Timer;
	VName		DefaultSound;
	bool		UseAgain;

	void Serialise(VStream&);
	void Tick(float);
	bool AdvanceFrame();
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

static bool P_StartButton(int sidenum, EBWhere w, int SwitchDef,
	VName DefaultSound, bool UseAgain)
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
			//	Force advancing to the next frame
			((VButton*)Th)->Timer = 0.001;
			return false;
		}
	}

	VButton* But = Spawn<VButton>();
	GLevel->AddThinker(But);
	But->Side = sidenum;
	But->Where = w;
	But->SwitchDef = SwitchDef;
	But->Frame = -1;
	But->DefaultSound = DefaultSound;
	But->UseAgain = UseAgain;
	But->AdvanceFrame();
	return true;
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
		EBWhere where;
		TSwitch* sw = Switches[i];

		if (sw->Tex == texTop)
		{
			where = SWITCH_TOP;
		}
		else if (sw->Tex == texMid)
		{
			where = SWITCH_MIDDLE;
		}
		else if (sw->Tex == texBot)
		{
			where = SWITCH_BOTTOM;
		}
		else
		{
			continue;
		}

		SV_SetLineTexture(sidenum, where, sw->Frames[0].Texture);
		bool PlaySound;
		if (useAgain || sw->NumFrames > 1)
			PlaySound = P_StartButton(sidenum, where, i, DefaultSound,
				useAgain);
		else
			PlaySound = true;
		if (PlaySound)
		{
			SV_SectorStartSound(GLevel->Sides[sidenum].sector, sw->Sound ?
				sw->Sound : GSoundManager->GetSoundID(DefaultSound), 0, 1, 1);
		}
		return;
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
		<< STRM_INDEX(SwitchDef)
		<< STRM_INDEX(Frame)
		<< Timer;
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
		TSwitch* Def = Switches[SwitchDef];
		if (Frame == Def->NumFrames - 1)
		{
			SwitchDef = Def->PairIndex;
			Def = Switches[Def->PairIndex];
			Frame = -1;
			SV_SectorStartSound(GLevel->Sides[Side].sector,
				Def->Sound ? Def->Sound :
				GSoundManager->GetSoundID(DefaultSound), 0, 1, 1);
			UseAgain = false;
		}

		bool KillMe = AdvanceFrame();
		SV_SetLineTexture(Side, Where, Def->Frames[Frame].Texture);
		if (KillMe)
		{
			SetFlags(_OF_DelayedDestroy);
		}
	}
	unguard;
}

//==========================================================================
//
//	VButton::Tick
//
//==========================================================================

bool VButton::AdvanceFrame()
{
	guard(VButton::AdvanceFrame);
	Frame++;
	bool Ret = false;
	TSwitch* Def = Switches[SwitchDef];
	if (Frame == Def->NumFrames - 1)
	{
		if (UseAgain)
		{
			Timer = BUTTONTIME;
		}
		else
		{
			Ret = true;
		}
	}
	else
	{
		if (Def->Frames[Frame].RandomRange)
		{
			Timer = (Def->Frames[Frame].BaseTime + Random() *
				Def->Frames[Frame].RandomRange) / 35.0;
		}
		else
		{
			Timer = Def->Frames[Frame].BaseTime / 35.0;
		}
	}
	return Ret;
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
