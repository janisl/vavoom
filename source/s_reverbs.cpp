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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VReverbInfo Psychotic =
{
	NULL,
	"Psychotic",
	0x1900,
	true,
	{ 25, 1.0f,	0.50f, -1000,  -151,   0,   7.56f,  0.91f, 1.0f,  -626,  0.020f, 0.0f,0.0f,0.0f,   774, 0.030f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 4.00f, 1.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
};

static VReverbInfo Dizzy =
{
	&Psychotic,
	"Dizzy",
	0x1800,
	true,
	{ 24, 1.8f,	0.60f, -1000,  -400,   0,   17.23f, 0.56f, 1.0f,  -1713, 0.020f, 0.0f,0.0f,0.0f,  -613, 0.030f, 0.0f,0.0f,0.0f, 0.250f, 1.00f, 0.81f, 0.310f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
};

static VReverbInfo Drugged =
{
	&Dizzy,
	"Drugged",
	0x1700,
	true,
	{ 23, 1.9f,	0.50f, -1000,  0,      0,   8.39f,  1.39f, 1.0f,  -115,  0.002f, 0.0f,0.0f,0.0f,   985, 0.030f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 1.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
};

static VReverbInfo Underwater =
{
	&Drugged,
	"Underwater",
	0x1600,
	true,
	{ 22, 1.8f,	1.00f, -1000,  -4000,  0,   1.49f,  0.10f, 1.0f,   -449, 0.007f, 0.0f,0.0f,0.0f,  1700, 0.011f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 1.18f, 0.348f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo SewerPipe =
{
	&Underwater,
	"Sewer Pipe",
	0x1500,
	true,
	{ 21, 1.7f,	0.80f, -1000,  -1000,  0,   2.81f,  0.14f, 1.0f,    429, 0.014f, 0.0f,0.0f,0.0f,  1023, 0.021f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  80.0f,  60.0f, 0x3f }
};

static VReverbInfo ParkingLot =
{
	&SewerPipe,
	"Parking Lot",
	0x1400,
	true,
	{ 20, 8.3f,	1.00f, -1000,  0,      0,   1.65f,  1.50f, 1.0f,  -1363, 0.008f, 0.0f,0.0f,0.0f, -1153, 0.012f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
};

static VReverbInfo Plain =
{
	&ParkingLot,
	"Plain",
	0x1300,
	true,
	{ 19, 42.5f,	0.21f, -1000,  -2000,  0,   1.49f,  0.50f, 1.0f,  -2466, 0.179f, 0.0f,0.0f,0.0f, -1926, 0.100f, 0.0f,0.0f,0.0f, 0.250f, 1.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  21.0f, 100.0f, 0x3f }
};

static VReverbInfo Quarry =
{
	&Plain,
	"Quarry",
	0x1200,
	true,
	{ 18, 17.5f,	1.00f, -1000,  -1000,  0,   1.49f,  0.83f, 1.0f, -10000, 0.061f, 0.0f,0.0f,0.0f,   500, 0.025f, 0.0f,0.0f,0.0f, 0.125f, 0.70f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Mountains =
{
	&Quarry,
	"Mountains",
	0x1100,
	true,
	{ 17, 100.0f, 0.27f, -1000,  -2500,  0,   1.49f,  0.21f, 1.0f,  -2780, 0.300f, 0.0f,0.0f,0.0f, -1434, 0.100f, 0.0f,0.0f,0.0f, 0.250f, 1.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  27.0f, 100.0f, 0x1f }
};

static VReverbInfo City =
{
	&Mountains,
	"City",
	0x1000,
	true,
	{ 16, 7.5f,	0.50f, -1000,  -800,   0,   1.49f,  0.67f, 1.0f,  -2273, 0.007f, 0.0f,0.0f,0.0f, -1691, 0.011f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  50.0f, 100.0f, 0x3f }
};

static VReverbInfo Forest =
{
	&City,
	"Forest",
	0x0F00,
	true,
	{ 15, 38.0f,	0.30f, -1000,  -3300,  0,   1.49f,  0.54f, 1.0f,  -2560, 0.162f, 0.0f,0.0f,0.0f,  -229, 0.088f, 0.0f,0.0f,0.0f, 0.125f, 1.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,  79.0f, 100.0f, 0x3f }
};

static VReverbInfo Alley =
{
	&Forest,
	"Alley",
	0x0E00,
	true,
	{ 14, 7.5f,	0.30f, -1000,  -270,   0,   1.49f,  0.86f, 1.0f,  -1204, 0.007f, 0.0f,0.0f,0.0f,    -4, 0.011f, 0.0f,0.0f,0.0f, 0.125f, 0.95f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo StoneCorridor =
{
	&Alley,
	"Stone Corridor",
	0x0D00,
	true,
	{ 13, 13.5f,	1.00f, -1000,  -237,   0,   2.70f,  0.79f, 1.0f,  -1214, 0.013f, 0.0f,0.0f,0.0f,   395, 0.020f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Hallway =
{
	&StoneCorridor,
	"Hallway",
	0x0C00,
	true,
	{ 12, 1.8f,	1.00f, -1000,  -300,   0,   1.49f,  0.59f, 1.0f,  -1219, 0.007f, 0.0f,0.0f,0.0f,   441, 0.011f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo CarpettedHallway =
{
	&Hallway,
	"Carpetted Hallway",
	0x0B00,
	true,
	{ 11, 1.9f,	1.00f, -1000,  -4000,  0,   0.30f,  0.10f, 1.0f,  -1831, 0.002f, 0.0f,0.0f,0.0f, -1630, 0.030f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Hangar =
{
	&CarpettedHallway,
	"Hangar",
	0x0A00,
	true,
	{ 10, 50.3f,	1.00f, -1000,  -1000,  0,   10.05f, 0.23f, 1.0f,   -602, 0.020f, 0.0f,0.0f,0.0f,   198, 0.030f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Arena =
{
	&Hangar,
	"Arena",
	0x0900,
	true,
	{ 9, 36.2f,	1.00f, -1000,  -698,   0,   7.24f,  0.33f, 1.0f,  -1166, 0.020f, 0.0f,0.0f,0.0f,    16, 0.030f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Cave =
{
	&Arena,
	"Cave",
	0x0800,
	true,
	{ 8, 14.6f,	1.00f, -1000,  0,      0,   2.91f,  1.30f, 1.0f,   -602, 0.015f, 0.0f,0.0f,0.0f,  -302, 0.022f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x1f }
};

static VReverbInfo ConcertHall =
{
	&Cave,
	"Concert Hall",
	0x0700,
	true,
	{ 7, 19.6f,	1.00f, -1000,  -500,   0,   3.92f,  0.70f, 1.0f,  -1230, 0.020f, 0.0f,0.0f,0.0f,    -2, 0.029f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Auditorium =
{
	&ConcertHall,
	"Auditorium",
	0x0600,
	true,
	{ 6, 21.6f,	1.00f, -1000,  -476,   0,   4.32f,  0.59f, 1.0f,   -789, 0.020f, 0.0f,0.0f,0.0f,  -289, 0.030f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo StoneRoom =
{
	&Auditorium,
	"Stone Room",
	0x0500,
	true,
	{ 5, 11.6f,	1.00f, -1000,  -300,   0,   2.31f,  0.64f, 1.0f,   -711, 0.012f, 0.0f,0.0f,0.0f,    83, 0.017f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo LivingRoom =
{
	&StoneRoom,
	"Living Room",
	0x0400,
	true,
	{ 4, 2.5f,	1.00f, -1000,  -6000,  0,   0.50f,  0.10f, 1.0f,  -1376, 0.003f, 0.0f,0.0f,0.0f, -1104, 0.004f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Bathroom =
{
	&LivingRoom,
	"Bathroom",
	0x0300,
	true,
	{ 3, 1.4f,	1.00f, -1000,  -1200,  0,   1.49f,  0.54f, 1.0f,   -370, 0.007f, 0.0f,0.0f,0.0f,  1030, 0.011f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f,  60.0f, 0x3f }
};

static VReverbInfo Room =
{
	&Bathroom,
	"Room",
	0x0200,
	true,
	{ 2, 1.9f,	1.00f, -1000,  -454,   0,   0.40f,  0.83f, 1.0f,  -1646, 0.002f, 0.0f,0.0f,0.0f,    53, 0.003f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo PaddedCell =
{
	&Room,
	"Padded Cell",
	0x0100,
	true,
	{ 1, 1.4f,	1.00f, -1000,  -6000,  0,   0.17f,  0.10f, 1.0f,  -1204, 0.001f, 0.0f,0.0f,0.0f,   207, 0.002f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Generic =
{
	&PaddedCell,
	"Generic",
	0x0001,
	true,
	{ 0, 7.5f,	1.00f, -1000,  -100,   0,   1.49f,  0.83f, 1.0f,  -2602, 0.007f, 0.0f,0.0f,0.0f,   200, 0.011f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0x3f }
};

static VReverbInfo Off =
{
	&Generic,
	"Off",
	0x0000,
	true,
	{ 0, 7.5f,	1.00f, -10000, -10000, 0,   1.00f,  1.00f, 1.0f,  -2602, 0.007f, 0.0f,0.0f,0.0f,   200, 0.011f, 0.0f,0.0f,0.0f, 0.250f, 0.00f, 0.25f, 0.000f, -5.0f, 5000.0f, 250.0f, 0.0f,   0.0f,   0.0f, 0x33f }
};

static const VReverbInfo* const DefaultEnvironments[26] =
{
	&Off, &PaddedCell, &Room, &Bathroom, &LivingRoom, &StoneRoom, &Auditorium,
	&ConcertHall, &Cave, &Arena, &Hangar, &CarpettedHallway, &Hallway,
	&StoneCorridor, &Alley, &Forest, &City, &Mountains, &Quarry, &Plain,
	&ParkingLot, &SewerPipe, &Underwater, &Drugged, &Dizzy, &Psychotic
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	DoInt
//
//==========================================================================

static void DoInt(VScriptParser* sc, int& Val, int Min, int Max)
{
	sc->ExpectNumber();
	Val = MID(Min, sc->Number, Max);
}

//==========================================================================
//
//	DoFloat
//
//==========================================================================

static void DoFloat(VScriptParser* sc, float& Val, float Min, float Max)
{
	sc->ExpectFloat();
	Val = MID(Min, sc->Float, Max);
}

//==========================================================================
//
//	DoBool
//
//==========================================================================

static void DoBool(VScriptParser* sc, int& Flags, int Mask)
{
	if (sc->Check("true"))
	{
		Flags |= Mask;
	}
	else if (sc->Check("false"))
	{
		Flags &= ~Mask;
	}
	else
	{
		sc->Error("Bad syntax");
	}
}

//==========================================================================
//
//	VSoundManager::ParseReverbs
//
//==========================================================================

void VSoundManager::ParseReverbs(VScriptParser* sc)
{
	guard(VSoundManager::ParseReverbs);
	if (!Environments)
	{
		Environments = &Off;
	}

	while (!sc->AtEnd())
	{
		//	Name and ID.
		sc->ExpectString();
		VStr Name = sc->String;
		sc->ExpectNumber();
		int Id1 = sc->Number;
		sc->ExpectNumber();
		int Id2 = sc->Number;

		VReverbProperties Prop;
		Prop.Flags = 0;

		bool HaveEnvironment = false;
		bool HaveEnvironmentSize = false;
		bool HaveEnvironmentDiffusion = false;
		bool HaveRoom = false;
		bool HaveRoomHF = false;
		bool HaveRoomLF = false;
		bool HaveDecayTime = false;
		bool HaveDecayHFRatio = false;
		bool HaveDecayLFRatio = false;
		bool HaveReflections = false;
		bool HaveReflectionsDelay = false;
		bool HaveReflectionsPanX = false;
		bool HaveReflectionsPanY = false;
		bool HaveReflectionsPanZ = false;
		bool HaveReverb = false;
		bool HaveReverbDelay = false;
		bool HaveReverbPanX = false;
		bool HaveReverbPanY = false;
		bool HaveReverbPanZ = false;
		bool HaveEchoTime = false;
		bool HaveEchoDepth = false;
		bool HaveModulationTime = false;
		bool HaveModulationDepth = false;
		bool HaveAirAbsorptionHF = false;
		bool HaveHFReference = false;
		bool HaveLFReference = false;
		bool HaveRoomRolloffFactor = false;
		bool HaveDiffusion = false;
		bool HaveDensity = false;
		int HaveFlags = 0;

		sc->Expect("{");
		while (!sc->Check("}"))
		{
			if (sc->Check("Environment"))
			{
				sc->ExpectNumber();
				if (sc->Number < 0 || sc->Number > 25)
				{
					sc->Error("Environment number out of range");
				}
				Prop.Environment = sc->Number;
				HaveEnvironment = true;
			}
			else if (sc->Check("EnvironmentSize"))
			{
				DoFloat(sc, Prop.EnvironmentSize, 1.0, 100.0);
				HaveEnvironmentSize = true;
			}
			else if (sc->Check("EnvironmentDiffusion"))
			{
				DoFloat(sc, Prop.EnvironmentDiffusion, 0.0, 1.0);
				HaveEnvironmentDiffusion = true;
			}
			else if (sc->Check("Room"))
			{
				DoInt(sc, Prop.Room, -10000, 0);
				HaveRoom = true;
			}
			else if (sc->Check("RoomHF"))
			{
				DoInt(sc, Prop.RoomHF, -10000, 0);
				HaveRoomHF = true;
			}
			else if (sc->Check("RoomLF"))
			{
				DoInt(sc, Prop.RoomLF, -10000, 0);
				HaveRoomLF = true;
			}
			else if (sc->Check("DecayTime"))
			{
				DoFloat(sc, Prop.DecayTime, 0.1, 20.0);
				HaveDecayTime = true;
			}
			else if (sc->Check("DecayHFRatio"))
			{
				DoFloat(sc, Prop.DecayHFRatio, 0.1, 2.0);
				HaveDecayHFRatio = true;
			}
			else if (sc->Check("DecayLFRatio"))
			{
				DoFloat(sc, Prop.DecayLFRatio, 0.1, 2.0);
				HaveDecayLFRatio = true;
			}
			else if (sc->Check("Reflections"))
			{
				DoInt(sc, Prop.Reflections, -10000, 1000);
				HaveReflections = true;
			}
			else if (sc->Check("ReflectionsDelay"))
			{
				DoFloat(sc, Prop.ReflectionsDelay, 0.0, 0.3);
				HaveReflectionsDelay = true;
			}
			else if (sc->Check("ReflectionsPanX"))
			{
				DoFloat(sc, Prop.ReflectionsPanX, -2000.0, 2000.0);
				HaveReflectionsPanX = true;
			}
			else if (sc->Check("ReflectionsPanY"))
			{
				DoFloat(sc, Prop.ReflectionsPanY, -2000.0, 2000.0);
				HaveReflectionsPanY = true;
			}
			else if (sc->Check("ReflectionsPanZ"))
			{
				DoFloat(sc, Prop.ReflectionsPanZ, -2000.0, 2000.0);
				HaveReflectionsPanZ = true;
			}
			else if (sc->Check("Reverb"))
			{
				DoInt(sc, Prop.Reverb, -10000, 2000);
				HaveReverb = true;
			}
			else if (sc->Check("ReverbDelay"))
			{
				DoFloat(sc, Prop.ReverbDelay, 0.0, 0.1);
				HaveReverbDelay = true;
			}
			else if (sc->Check("ReverbPanX"))
			{
				DoFloat(sc, Prop.ReverbPanX, -2000.0, 2000.0);
				HaveReverbPanX = true;
			}
			else if (sc->Check("ReverbPanY"))
			{
				DoFloat(sc, Prop.ReverbPanY, -2000.0, 2000.0);
				HaveReverbPanY = true;
			}
			else if (sc->Check("ReverbPanZ"))
			{
				DoFloat(sc, Prop.ReverbPanZ, -2000.0, 2000.0);
				HaveReverbPanZ = true;
			}
			else if (sc->Check("EchoTime"))
			{
				DoFloat(sc, Prop.EchoTime, 0.075, 0.25);
				HaveEchoTime = true;
			}
			else if (sc->Check("EchoDepth"))
			{
				DoFloat(sc, Prop.EchoDepth, 0.0, 1.0);
				HaveEchoDepth = true;
			}
			else if (sc->Check("ModulationTime"))
			{
				DoFloat(sc, Prop.ModulationTime, 0.04, 4.0);
				HaveModulationTime = true;
			}
			else if (sc->Check("ModulationDepth"))
			{
				DoFloat(sc, Prop.ModulationDepth, 0.0, 1.0);
				HaveModulationDepth = true;
			}
			else if (sc->Check("AirAbsorptionHF"))
			{
				DoFloat(sc, Prop.AirAbsorptionHF, -100.0, 0.0);
				HaveAirAbsorptionHF = true;
			}
			else if (sc->Check("HFReference"))
			{
				DoFloat(sc, Prop.HFReference, 1000.0, 20000.0);
				HaveHFReference = true;
			}
			else if (sc->Check("LFReference"))
			{
				DoFloat(sc, Prop.LFReference, 20.0, 1000.0);
				HaveLFReference = true;
			}
			else if (sc->Check("RoomRolloffFactor"))
			{
				DoFloat(sc, Prop.RoomRolloffFactor, 0.0, 10.0);
				HaveRoomRolloffFactor = true;
			}
			else if (sc->Check("Diffusion"))
			{
				DoFloat(sc, Prop.Diffusion, 0.0, 100.0);
				HaveDiffusion = true;
			}
			else if (sc->Check("Density"))
			{
				DoFloat(sc, Prop.Density, 0.0, 100.0);
				HaveDensity = true;
			}
			else if (sc->Check("bReflectionsScale"))
			{
				DoBool(sc, Prop.Flags, REVERBF_ReflectionsScale);
				HaveFlags |= REVERBF_ReflectionsScale;
			}
			else if (sc->Check("bReflectionsDelayScale"))
			{
				DoBool(sc, Prop.Flags, REVERBF_ReflectionsDelayScale);
				HaveFlags |= REVERBF_ReflectionsDelayScale;
			}
			else if (sc->Check("bDecayTimeScale"))
			{
				DoBool(sc, Prop.Flags, REVERBF_DecayTimeScale);
				HaveFlags |= REVERBF_DecayTimeScale;
			}
			else if (sc->Check("bDecayHFLimit"))
			{
				DoBool(sc, Prop.Flags, REVERBF_DecayHFLimit);
				HaveFlags |= REVERBF_DecayHFLimit;
			}
			else if (sc->Check("bReverbScale"))
			{
				DoBool(sc, Prop.Flags, REVERBF_ReverbScale);
				HaveFlags |= REVERBF_ReverbScale;
			}
			else if (sc->Check("bReverbDelayScale"))
			{
				DoBool(sc, Prop.Flags, REVERBF_ReverbDelayScale);
				HaveFlags |= REVERBF_ReverbDelayScale;
			}
			else if (sc->Check("bEchoTimeScale"))
			{
				DoBool(sc, Prop.Flags, REVERBF_EchoTimeScale);
				HaveFlags |= REVERBF_EchoTimeScale;
			}
			else if (sc->Check("bModulationTimeScale"))
			{
				DoBool(sc, Prop.Flags, REVERBF_ModulationTimeScale);
				HaveFlags |= REVERBF_ModulationTimeScale;
			}
			else
			{
				sc->Error("Bad syntax");
			}
		}

		if (!HaveEnvironment)
		{
			sc->Error(va("Environment %s is mising an Environment field", *Name));
		}

		const VReverbProperties& Def = DefaultEnvironments[Prop.Environment]->Props;
		if (!HaveEnvironmentSize)
		{
			Prop.EnvironmentSize = Def.EnvironmentSize;
		}
		if (!HaveEnvironmentDiffusion)
		{
			Prop.EnvironmentDiffusion = Def.EnvironmentDiffusion;
		}
		if (!HaveRoom)
		{
			Prop.Room = Def.Room;
		}
		if (!HaveRoomHF)
		{
			Prop.RoomHF = Def.RoomHF;
		}
		if (!HaveRoomLF)
		{
			Prop.RoomLF = Def.RoomLF;
		}
		if (!HaveDecayTime)
		{
			Prop.DecayTime = Def.DecayTime;
		}
		if (!HaveDecayHFRatio)
		{
			Prop.DecayHFRatio = Def.DecayHFRatio;
		}
		if (!HaveDecayLFRatio)
		{
			Prop.DecayLFRatio = Def.DecayLFRatio;
		}
		if (!HaveReflections)
		{
			Prop.Reflections = Def.Reflections;
		}
		if (!HaveReflectionsDelay)
		{
			Prop.ReflectionsDelay = Def.ReflectionsDelay;
		}
		if (!HaveReflectionsPanX)
		{
			Prop.ReflectionsPanX = Def.ReflectionsPanX;
		}
		if (!HaveReflectionsPanY)
		{
			Prop.ReflectionsPanY = Def.ReflectionsPanY;
		}
		if (!HaveReflectionsPanZ)
		{
			Prop.ReflectionsPanZ = Def.ReflectionsPanZ;
		}
		if (!HaveReverb)
		{
			Prop.Reverb = Def.Reverb;
		}
		if (!HaveReverbDelay)
		{
			Prop.ReverbDelay = Def.ReverbDelay;
		}
		if (!HaveReverbPanX)
		{
			Prop.ReverbPanX = Def.ReverbPanX;
		}
		if (!HaveReverbPanY)
		{
			Prop.ReverbPanY = Def.ReverbPanY;
		}
		if (!HaveReverbPanZ)
		{
			Prop.ReverbPanZ = Def.ReverbPanZ;
		}
		if (!HaveEchoTime)
		{
			Prop.EchoTime = Def.EchoTime;
		}
		if (!HaveEchoDepth)
		{
			Prop.EchoDepth = Def.EchoDepth;
		}
		if (!HaveModulationTime)
		{
			Prop.ModulationTime = Def.ModulationTime;
		}
		if (!HaveModulationDepth)
		{
			Prop.ModulationDepth = Def.ModulationDepth;
		}
		if (!HaveAirAbsorptionHF)
		{
			Prop.AirAbsorptionHF = Def.AirAbsorptionHF;
		}
		if (!HaveHFReference)
		{
			Prop.HFReference = Def.HFReference;
		}
		if (!HaveLFReference)
		{
			Prop.LFReference = Def.LFReference;
		}
		if (!HaveRoomRolloffFactor)
		{
			Prop.RoomRolloffFactor = Def.RoomRolloffFactor;
		}
		if (!HaveDiffusion)
		{
			Prop.Diffusion = Def.Diffusion;
		}
		if (!HaveDensity)
		{
			Prop.Density = Def.Density;
		}
		Prop.Flags |= Def.Flags & ~HaveFlags;

		VReverbInfo* NewEnv = new VReverbInfo;
		char* NewName = new char[Name.Length() + 1];
		strcpy(NewName, *Name);
		NewEnv->Next = NULL;
		NewEnv->Name = NewName;
		NewEnv->Id = (Id1 << 8) | Id2;
		NewEnv->Builtin = false;
		NewEnv->Props = Prop;

		//	Find a slot for the new environment.
		VReverbInfo* Check = Environments;
		VReverbInfo** Ptr = &Environments;
		while (Check && Check->Id < NewEnv->Id)
		{
			Ptr = &Check->Next;
			Check = Check->Next;
		}

		//	Check for redeclared environment.
		if (Check && Check->Id == NewEnv->Id)
		{
			//	Don't allow replacements of builtin environments.
			if (!Check->Builtin)
			{
				NewEnv->Next = Check->Next;
				*Ptr = NewEnv;
				delete[] const_cast<char*>(Check->Name);
				delete Check;
			}
			else
			{
				delete[] NewName;
				delete NewEnv;
			}
		}
		else
		{
			NewEnv->Next = Check;
			*Ptr = NewEnv;
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	VSoundManager::FindEnvironment
//
//==========================================================================

VReverbInfo* VSoundManager::FindEnvironment(int Id)
{
	guard(VSoundManager::FindEnvironment);
	VReverbInfo* Check = Environments;
	while (Check->Next && Check->Next->Id <= Id)
	{
		Check = Check->Next;
	}
	return Check;
	unguard;
}
