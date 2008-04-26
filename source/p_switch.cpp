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
	SWITCH_Top,
	SWITCH_Middle,
	SWITCH_Bottom
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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, Button)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VLevelInfo::ChangeSwitchTexture
//
//	Function that changes wall texture.
//	Tell it if switch is ok to use again (1=yes, it's a button).
//
//==========================================================================

bool VLevelInfo::ChangeSwitchTexture(int sidenum, bool useAgain,
	VName DefaultSound, bool& Quest)
{
	guard(VLevelInfo::ChangeSwitchTexture);
	int texTop = XLevel->Sides[sidenum].toptexture;
	int texMid = XLevel->Sides[sidenum].midtexture;
	int texBot = XLevel->Sides[sidenum].bottomtexture;

	for (int  i = 0; i < Switches.Num(); i++)
	{
		EBWhere where;
		TSwitch* sw = Switches[i];

		if (sw->Tex == texTop)
		{
			where = SWITCH_Top;
			XLevel->Sides[sidenum].toptexture = sw->Frames[0].Texture;
		}
		else if (sw->Tex == texMid)
		{
			where = SWITCH_Middle;
			XLevel->Sides[sidenum].midtexture = sw->Frames[0].Texture;
		}
		else if (sw->Tex == texBot)
		{
			where = SWITCH_Bottom;
			XLevel->Sides[sidenum].bottomtexture = sw->Frames[0].Texture;
		}
		else
		{
			continue;
		}

		bool PlaySound;
		if (useAgain || sw->NumFrames > 1)
		{
			PlaySound = StartButton(sidenum, where, i, DefaultSound,
				useAgain);
		}
		else
		{
			PlaySound = true;
		}

		if (PlaySound)
		{
			SectorStartSound(XLevel->Sides[sidenum].sector, sw->Sound ?
				sw->Sound : GSoundManager->GetSoundID(DefaultSound), 0, 1, 1);
		}
		Quest = sw->Quest;
		return true;
	}
	Quest = false;
	return false;
	unguard;
}

//==========================================================================
//
//	VLevelInfo::StartButton
//
//	Start a button counting down till it turns off.
//
//==========================================================================

bool VLevelInfo::StartButton(int sidenum, vuint8 w, int SwitchDef,
	VName DefaultSound, bool UseAgain)
{
	guard(VLevelInfo::StartButton);
	// See if button is already pressed
	for (TThinkerIterator<VButton> Btn(XLevel); Btn; ++Btn)
	{
		if (Btn->Side == sidenum)
		{
			//	Force advancing to the next frame
			Btn->Timer = 0.001;
			return false;
		}
	}

	VButton* But = (VButton*)XLevel->SpawnThinker(VButton::StaticClass());
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
			Level->SectorStartSound(XLevel->Sides[Side].sector,
				Def->Sound ? Def->Sound :
				GSoundManager->GetSoundID(DefaultSound), 0, 1, 1);
			UseAgain = false;
		}

		bool KillMe = AdvanceFrame();
		if (Where == SWITCH_Middle)
		{
			XLevel->Sides[Side].midtexture = Def->Frames[Frame].Texture;
		}
		else if (Where == SWITCH_Bottom)
		{
			XLevel->Sides[Side].bottomtexture = Def->Frames[Frame].Texture;
		}
		else
		{ // TEXTURE_TOP
			XLevel->Sides[Side].toptexture = Def->Frames[Frame].Texture;
		}
		if (KillMe)
		{
			DestroyThinker();
		}
	}
	unguard;
}

//==========================================================================
//
//	VButton::AdvanceFrame
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
