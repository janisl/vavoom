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

struct VSplashInfo
{
	VName		Name;

	VClass*		SmallClass;
	float		SmallClip;
	VName		SmallSound;

	VClass*		BaseClass;
	VClass*		ChunkClass;
	float		ChunkXVelMul;
	float		ChunkYVelMul;
	float		ChunkZVelMul;
	float		ChunkBaseZVel;
	VName		Sound;
	enum
	{
		F_NoAlert	= 0x00000001,
	};
	vuint32		Flags;
};

struct VTerrainInfo
{
	VName		Name;
	VName		Splash;
	enum
	{
		F_Liquid	= 0x00000001,
	};
	vuint32		Flags;
	float		FootClip;
	vint32		DamageTimeMask;
	vint32		DamageAmount;
	VName		DamageType;
	float		Friction;
	float		MoveFactor;
	float		StepVolume;
	float		WalkingStepTime;
	float		RunningStepTime;
	VName		LeftStepSounds;
	VName		RightStepSounds;
};

struct VTerrainType
{
	int				Pic;
	VName			TypeName;
	VTerrainInfo*	Info;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, Button)

//	Terrain types, maby not right place for them
static TArray<VSplashInfo>		SplashInfos;
static TArray<VTerrainInfo>		TerrainInfos;
static TArray<VTerrainType>		TerrainTypes;

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

	VButton* But = (VButton*)GLevel->SpawnThinker(VButton::StaticClass());
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

//**************************************************************************
//
//	Terrain types
//
//**************************************************************************

//==========================================================================
//
//	GetSplashInfo
//
//==========================================================================

static VSplashInfo* GetSplashInfo(VName Name)
{
	guard(GetSplashInfo);
	for (int i = 0; i < SplashInfos.Num(); i++)
	{
		if (SplashInfos[i].Name == Name)
		{
			return &SplashInfos[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	GetTerrainInfo
//
//==========================================================================

static VTerrainInfo* GetTerrainInfo(VName Name)
{
	guard(GetTerrainInfo);
	for (int i = 0; i < TerrainInfos.Num(); i++)
	{
		if (TerrainInfos[i].Name == Name)
		{
			return &TerrainInfos[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	ParseTerrainScript
//
//==========================================================================

static void ParseTerrainScript(VScriptParser* sc)
{
	guard(ParseTerrainScript);
	while (!sc->AtEnd())
	{
		if (sc->Check("splash"))
		{
			sc->ExpectString();
			VSplashInfo* SInfo = GetSplashInfo(*sc->String);
			if (!SInfo)
			{
				SInfo = &SplashInfos.Alloc();
			}
			SInfo->Name = *sc->String;
			SInfo->SmallClass = NULL;
			SInfo->SmallClip = 0;
			SInfo->SmallSound = NAME_None;
			SInfo->BaseClass = NULL;
			SInfo->ChunkClass = NULL;
			SInfo->ChunkXVelMul = 0;
			SInfo->ChunkYVelMul = 0;
			SInfo->ChunkZVelMul = 0;
			SInfo->ChunkBaseZVel = 0;
			SInfo->Sound = NAME_None;
			SInfo->Flags = 0;
			sc->Expect("{");
			while (!sc->Check("}"))
			{
				if (sc->Check("smallclass"))
				{
					sc->ExpectString();
					SInfo->SmallClass = VClass::FindClass(*sc->String);
				}
				else if (sc->Check("smallclip"))
				{
					sc->ExpectFloat();
					SInfo->SmallClip = sc->Float;
				}
				else if (sc->Check("smallsound"))
				{
					sc->ExpectString();
					SInfo->SmallSound = *sc->String;
				}
				else if (sc->Check("baseclass"))
				{
					sc->ExpectString();
					SInfo->BaseClass = VClass::FindClass(*sc->String);
				}
				else if (sc->Check("chunkclass"))
				{
					sc->ExpectString();
					SInfo->ChunkClass = VClass::FindClass(*sc->String);
				}
				else if (sc->Check("chunkxvelshift"))
				{
					sc->ExpectNumber();
					SInfo->ChunkXVelMul = sc->Number < 0 ? 0.0 :
						(1 << sc->Number) / 256.0;
				}
				else if (sc->Check("chunkyvelshift"))
				{
					sc->ExpectNumber();
					SInfo->ChunkYVelMul = sc->Number < 0 ? 0.0 :
						(1 << sc->Number) / 256.0;
				}
				else if (sc->Check("chunkzvelshift"))
				{
					sc->ExpectNumber();
					SInfo->ChunkZVelMul = sc->Number < 0 ? 0.0 :
						(1 << sc->Number) / 256.0;
				}
				else if (sc->Check("chunkbasezvel"))
				{
					sc->ExpectFloat();
					SInfo->ChunkBaseZVel = sc->Float;
				}
				else if (sc->Check("sound"))
				{
					sc->ExpectString();
					SInfo->Sound = *sc->String;
				}
				else if (sc->Check("noalert"))
				{
					SInfo->Flags |= VSplashInfo::F_NoAlert;
				}
				else
				{
					sc->Error("Unknown command");
				}
			}
		}
		else if (sc->Check("terrain"))
		{
			sc->ExpectString();
			VTerrainInfo* TInfo = GetTerrainInfo(*sc->String);
			if (!TInfo)
			{
				TInfo = &TerrainInfos.Alloc();
			}
			TInfo->Name = *sc->String;
			TInfo->Splash = NAME_None;
			TInfo->Flags = 0;
			TInfo->FootClip = 0;
			TInfo->DamageTimeMask = 0;
			TInfo->DamageAmount = 0;
			TInfo->DamageType = NAME_None;
			TInfo->Friction = 0.0;
			sc->Expect("{");
			while (!sc->Check("}"))
			{
				if (sc->Check("splash"))
				{
					sc->ExpectString();
					TInfo->Splash = *sc->String;
				}
				else if (sc->Check("liquid"))
				{
					TInfo->Flags |= VTerrainInfo::F_Liquid;
				}
				else if (sc->Check("footclip"))
				{
					sc->ExpectFloat();
					TInfo->FootClip = sc->Float;
				}
				else if (sc->Check("damagetimemask"))
				{
					sc->ExpectNumber();
					TInfo->DamageTimeMask = sc->Number;
				}
				else if (sc->Check("damageamount"))
				{
					sc->ExpectNumber();
					TInfo->DamageAmount = sc->Number;
				}
				else if (sc->Check("damagetype"))
				{
					sc->ExpectString();
					TInfo->DamageType = *sc->String;
				}
				else if (sc->Check("friction"))
				{
					sc->ExpectFloat();
					int friction, movefactor;

					// Same calculations as in Sector_SetFriction special.
					// A friction of 1.0 is equivalent to ORIG_FRICTION.

					friction = (int)(0x1EB8 * (sc->Float * 100)) / 0x80 + 0xD001;
					friction = MID(0, friction, 0x10000);

					if (friction > 0xe800)	// ice
						movefactor = ((0x10092 - friction) * 1024) / 4352 + 568;
					else
						movefactor = ((friction - 0xDB34) * (0xA)) / 0x80;

					if (movefactor < 32)
						movefactor = 32;

					TInfo->Friction = (1.0 - (float)friction / (float)0x10000) * 35.0;
					TInfo->MoveFactor = float(movefactor) / float(0x10000);
				}
				else if (sc->Check("stepvolume"))
				{
					sc->ExpectFloat();
					TInfo->StepVolume = sc->Float;
				}
				else if (sc->Check("walkingsteptime"))
				{
					sc->ExpectFloat();
					TInfo->WalkingStepTime = sc->Float;
				}
				else if (sc->Check("runningsteptime"))
				{
					sc->ExpectFloat();
					TInfo->RunningStepTime = sc->Float;
				}
				else if (sc->Check("leftstepsounds"))
				{
					sc->ExpectString();
					TInfo->LeftStepSounds = *sc->String;
				}
				else if (sc->Check("rightstepsounds"))
				{
					sc->ExpectString();
					TInfo->RightStepSounds = *sc->String;
				}
				else
				{
					sc->Error("Unknown command");
				}
			}
		}
		else if (sc->Check("floor"))
		{
			sc->ExpectName8();
			int Pic = GTextureManager.CheckNumForName(sc->Name8, TEXTYPE_Flat,
				false, false);
			sc->ExpectString();
			bool Found = false;
			for (int i = 0; i < TerrainTypes.Num(); i++)
			{
				if (TerrainTypes[i].Pic == Pic)
				{
					TerrainTypes[i].TypeName = *sc->String;
					Found = true;
					break;
				}
			}
			if (!Found)
			{
				VTerrainType& T = TerrainTypes.Alloc();
				T.Pic = Pic;
				T.TypeName = *sc->String;
			}
		}
		else if (sc->Check("ifdoom") || sc->Check("ifheretic") ||
			sc->Check("ifhexen") || sc->Check("ifstrife") || sc->Check("endif"))
		{
			GCon->Log("Vavoom doesn't support conditional game commands");
		}
		else
		{
			sc->Error("Unknown command");
		}
	}
	delete sc;
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
	//	Create default terrain
	VTerrainInfo& DefT = TerrainInfos.Alloc();
	DefT.Name = "Solid";
	DefT.Splash = NAME_None;
	DefT.Flags = 0;
	DefT.FootClip = 0;
	DefT.DamageTimeMask = 0;
	DefT.DamageAmount = 0;
	DefT.DamageType = NAME_None;
	DefT.Friction = 0.0;

	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == "terrain")
		{
			ParseTerrainScript(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)));
		}
	}
	for (int i = 0; i < TerrainTypes.Num(); i++)
	{
		TerrainTypes[i].Info = GetTerrainInfo(TerrainTypes[i].TypeName);
		if (!TerrainTypes[i].Info)
		{
			TerrainTypes[i].Info = &TerrainInfos[0];
		}
	}
	unguard;
}

//==========================================================================
//
//	SV_TerrainType
//
//==========================================================================

VTerrainInfo* SV_TerrainType(int pic)
{
	guard(SV_TerrainType);
	for (int i = 0; i < TerrainTypes.Num(); i++)
	{
		if (TerrainTypes[i].Pic == pic)
		{
			return TerrainTypes[i].Info;
		}
	}
	return &TerrainInfos[0];
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
	SplashInfos.Clear();
	TerrainInfos.Clear();
	TerrainTypes.Clear();
	unguard;
}

IMPLEMENT_FUNCTION(VObject, GetSplashInfo)
{
	P_GET_NAME(Name);
	RET_PTR(GetSplashInfo(Name));
}

IMPLEMENT_FUNCTION(VObject, GetTerrainInfo)
{
	P_GET_NAME(Name);
	RET_PTR(GetTerrainInfo(Name));
}
