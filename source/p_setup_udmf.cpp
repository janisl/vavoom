//**************************************************************************
//**
//**    ##   ##    ##    ##   ##   ####     ####   ###     ###
//**    ##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**     ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**     ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**      ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**       #    ##    ##    #      ####     ####   ##       ##
//**
//**    $Id$
//**
//**    Copyright (C) 1999-2006 Jānis Legzdiņš
//**
//**    This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**    This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************
//**
//**    Do all the WAD I/O, get map description, set up initial state and
//**  misc. LUTs.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	ML_PASSUSE_BOOM				= 0x0200,	//	Boom's ML_PASSUSE flag (conflicts with ML_REPEAT_SPECIAL)

	MTF_AMBUSH		= 0x0008,	// Deaf monsters/do not react to sound.
	MTF_DORMANT		= 0x0010,	// The thing is dormant
	MTF_GSINGLE		= 0x0100,	// Appearing in game modes
	MTF_GCOOP		= 0x0200,
	MTF_GDEATHMATCH	= 0x0400,
	MTF_SHADOW		= 0x0800,
	MTF_ALTSHADOW	= 0x1000,
	MTF_FRIENDLY	= 0x2000,
	MTF_STANDSTILL	= 0x4000,
};

class VUdmfParser
{
public:
	struct VParsedLine
	{
		line_t		L;
		int			V1Index;
		int			V2Index;
	};

	struct VParsedSide
	{
		side_t		S;
		VStr		TopTexture;
		VStr		MidTexture;
		VStr		BotTexture;
		int			SectorIndex;
	};

	bool				bExtended;
	bool				bCanTranslate;
	TArray<vertex_t>	ParsedVertexes;
	TArray<sector_t>	ParsedSectors;
	TArray<VParsedLine>	ParsedLines;
	TArray<VParsedSide>	ParsedSides;
	TArray<mthing_t>	ParsedThings;

	void Parse(VLevel*, int);
	void ParseVertex(VScriptParser&);
	void ParseSector(VScriptParser&, VLevel*);
	void ParseLineDef(VScriptParser&);
	void ParseSideDef(VScriptParser&);
	void ParseThing(VScriptParser&);
	void Flag(int&, int, const VStr&);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VUdmfParser::Parse
//
//==========================================================================

void VUdmfParser::Parse(VLevel* Level, int Lump)
{
	guard(VUdmfParser::Parse);
	VScriptParser sc("textmap", W_CreateLumpReaderNum(Lump));
	sc.SetCMode(true);

	//	Get namespace name.
	sc.Expect("namespace");
	sc.Expect("=");
	sc.ExpectString();
	VStr Namespace = sc.String;
	sc.Expect(";");
	bExtended = false;
	bCanTranslate = true;
	//	Vavoom's namespace.
	if (!Namespace.ICmp("Vavoom"))
	{
		bExtended = true;
	}
	//	Standard namespaces.
	else if (!Namespace.ICmp("Doom"))
	{
	}
	else if (!Namespace.ICmp("Heretic"))
	{
	}
	else if (!Namespace.ICmp("Hexen"))
	{
		bExtended = true;
	}
	else if (!Namespace.ICmp("Strife"))
	{
	}
	//	ZDoom namespaces.
	else if (!Namespace.ICmp("ZDoom"))
	{
		bExtended = true;
	}
	else if (!Namespace.ICmp("ZDoomTranslated"))
	{
	}
	else
	{
		//	Unknown namespace.
		bCanTranslate = false;
	}

	while (!sc.AtEnd())
	{
		if (sc.Check("vertex"))
		{
			ParseVertex(sc);
		}
		else if (sc.Check("sector"))
		{
			ParseSector(sc, Level);
		}
		else if (sc.Check("linedef"))
		{
			ParseLineDef(sc);
		}
		else if (sc.Check("sidedef"))
		{
			ParseSideDef(sc);
		}
		else if (sc.Check("thing"))
		{
			ParseThing(sc);
		}
		else
		{
			sc.Error("Syntax error");
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseVertex
//
//==========================================================================

void VUdmfParser::ParseVertex(VScriptParser& sc)
{
	guard(VUdmfParser::ParseVertex);
	//	Allocate a new vertex.
	vertex_t& V = ParsedVertexes.Alloc();
	V = TVec(0, 0, 0);

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		//	Get key and value.
		sc.ExpectString();
		VStr Key = sc.String;
		sc.Expect("=");
		sc.ExpectString();
		VStr Val = sc.String;
		if (Val == "-" && sc.CheckFloat())
		{
			Val += sc.String;
		}
		sc.Expect(";");

		if (!Key.ICmp("x"))
		{
			V.x = atof(*Val);
		}
		else if (!Key.ICmp("y"))
		{
			V.y = atof(*Val);
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseSector
//
//==========================================================================

void VUdmfParser::ParseSector(VScriptParser& sc, VLevel* Level)
{
	guard(VUdmfParser::ParseSector);
	sector_t& S = ParsedSectors.Alloc();
	memset(&S, 0, sizeof(sector_t));
	S.floor.Set(TVec(0, 0, 1), 0);
	S.floor.XScale = 1.0;
	S.floor.YScale = 1.0;
	S.floor.Alpha = 1.0;
	S.floor.MirrorAlpha = 1.0;
	S.floor.LightSourceSector = -1;
	S.ceiling.Set(TVec(0, 0, -1), 0);
	S.ceiling.XScale = 1.0;
	S.ceiling.YScale = 1.0;
	S.ceiling.Alpha = 1.0;
	S.ceiling.MirrorAlpha = 1.0;
	S.ceiling.LightSourceSector = -1;
	S.params.lightlevel = 160;
	S.params.LightColour = 0x00ffffff;
	S.seqType = -1;	// default seqType
	S.Gravity = 1.0;	// default sector gravity of 1.0

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		sc.ExpectString();
		VStr Key = sc.String;
		sc.Expect("=");
		sc.ExpectString();
		VStr Val = sc.String;
		if (Val == "-" && sc.CheckFloat())
		{
			Val += sc.String;
		}
		sc.Expect(";");

		if (!Key.ICmp("heightfloor"))
		{
			float FVal = atof(*Val);
			S.floor.dist = FVal;
			S.floor.TexZ = FVal;
			S.floor.minz = FVal;
			S.floor.maxz = FVal;
		}
		else if (!Key.ICmp("heightceiling"))
		{
			float FVal = atof(*Val);
			S.ceiling.dist = -FVal;
			S.ceiling.TexZ = FVal;
			S.ceiling.minz = FVal;
			S.ceiling.maxz = FVal;
		}
		else if (!Key.ICmp("texturefloor"))
		{
			S.floor.pic = Level->TexNumForName(*Val, TEXTYPE_Flat);
		}
		else if (!Key.ICmp("textureceiling"))
		{
			S.ceiling.pic = Level->TexNumForName(*Val, TEXTYPE_Flat);
		}
		else if (!Key.ICmp("lightlevel"))
		{
			S.params.lightlevel = atoi(*Val);
		}
		else if (!Key.ICmp("special"))
		{
			S.special = atoi(*Val);
		}
		else if (!Key.ICmp("id"))
		{
			S.tag = atoi(*Val);
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseLineDef
//
//==========================================================================

void VUdmfParser::ParseLineDef(VScriptParser& sc)
{
	guard(VUdmfParser::ParseLineDef);
	VParsedLine& L = ParsedLines.Alloc();
	memset(&L, 0, sizeof(VParsedLine));
	L.V1Index = -1;
	L.V2Index = -1;
	L.L.alpha = 1.0;
	L.L.LineTag = bExtended ? -1 : 0;
	L.L.sidenum[0] = -1;
	L.L.sidenum[1] = -1;
	bool HavePassUse = false;

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		sc.ExpectString();
		VStr Key = sc.String;
		sc.Expect("=");
		sc.ExpectString();
		VStr Val = sc.String;
		if (Val == "-" && sc.CheckFloat())
		{
			Val += sc.String;
		}
		sc.Expect(";");

		if (!Key.ICmp("id"))
		{
			L.L.LineTag = atoi(*Val);
		}
		else if (!Key.ICmp("v1"))
		{
			L.V1Index = atoi(*Val);
		}
		else if (!Key.ICmp("v2"))
		{
			L.V2Index = atoi(*Val);
		}
		else if (!Key.ICmp("blocking"))
		{
			Flag(L.L.flags, ML_BLOCKING, Val);
		}
		else if (!Key.ICmp("blockmonsters"))
		{
			Flag(L.L.flags, ML_BLOCKMONSTERS, Val);
		}
		else if (!Key.ICmp("twosided"))
		{
			Flag(L.L.flags, ML_TWOSIDED, Val);
		}
		else if (!Key.ICmp("dontpegtop"))
		{
			Flag(L.L.flags, ML_DONTPEGTOP, Val);
		}
		else if (!Key.ICmp("dontpegbottom"))
		{
			Flag(L.L.flags, ML_DONTPEGBOTTOM, Val);
		}
		else if (!Key.ICmp("secret"))
		{
			Flag(L.L.flags, ML_SECRET, Val);
		}
		else if (!Key.ICmp("blocksound"))
		{
			Flag(L.L.flags, ML_SOUNDBLOCK, Val);
		}
		else if (!Key.ICmp("dontdraw"))
		{
			Flag(L.L.flags, ML_DONTDRAW, Val);
		}
		else if (!Key.ICmp("mapped"))
		{
			Flag(L.L.flags, ML_MAPPED, Val);
		}
		else if (!Key.ICmp("passuse"))
		{
			if (bExtended)
			{
				HavePassUse = !Val.ICmp("true");
			}
			else
			{
				Flag(L.L.flags, ML_PASSUSE_BOOM, Val);
			}
		}
		else if (!Key.ICmp("translucent"))
		{
			L.L.alpha = 0.666;
		}
		else if (!Key.ICmp("jumpover"))
		{
			Flag(L.L.flags, ML_RAILING, Val);
		}
		else if (!Key.ICmp("blockfloaters"))
		{
			Flag(L.L.flags, ML_BLOCK_FLOATERS, Val);
		}
		else if (!Key.ICmp("special"))
		{
			L.L.special = atoi(*Val);
		}
		else if (!Key.ICmp("arg0"))
		{
			L.L.arg1 = atoi(*Val);
		}
		else if (!Key.ICmp("arg1"))
		{
			L.L.arg2 = atoi(*Val);
		}
		else if (!Key.ICmp("arg2"))
		{
			L.L.arg3 = atoi(*Val);
		}
		else if (!Key.ICmp("arg3"))
		{
			L.L.arg4 = atoi(*Val);
		}
		else if (!Key.ICmp("arg4"))
		{
			L.L.arg5 = atoi(*Val);
		}
		else if (!Key.ICmp("sidefront"))
		{
			L.L.sidenum[0] = atoi(*Val);
		}
		else if (!Key.ICmp("sideback"))
		{
			L.L.sidenum[1] = atoi(*Val);
		}

		if (bExtended)
		{
			if (!Key.ICmp("playercross"))
			{
				Flag(L.L.SpacFlags, 1 << SPAC_Cross, Val);
			}
			else if (!Key.ICmp("playeruse"))
			{
				Flag(L.L.SpacFlags, 1 << SPAC_Use, Val);
			}
			else if (!Key.ICmp("monstercross"))
			{
				Flag(L.L.SpacFlags, 1 << SPAC_MCross, Val);
			}
			else if (!Key.ICmp("monsteruse"))
			{
				//Flag(L.L.SpacFlags, 1 << SPAC_CROSS, Val);
			}
			else if (!Key.ICmp("impact"))
			{
				Flag(L.L.SpacFlags, 1 << SPAC_Impact, Val);
			}
			else if (!Key.ICmp("playerpush"))
			{
				Flag(L.L.SpacFlags, 1 << SPAC_Push, Val);
			}
			else if (!Key.ICmp("monsterpush"))
			{
				//Flag(L.L.SpacFlags, 1 << SPAC_CROSS, Val);
			}
			else if (!Key.ICmp("missilecross"))
			{
				Flag(L.L.SpacFlags, 1 << SPAC_PCross, Val);
			}
			else if (!Key.ICmp("repeatspecial"))
			{
				Flag(L.L.flags, ML_REPEAT_SPECIAL, Val);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseSideDef
//
//==========================================================================

void VUdmfParser::ParseSideDef(VScriptParser& sc)
{
	guard(VUdmfParser::ParseSideDef);
	VParsedSide& S = ParsedSides.Alloc();
	memset(&S, 0, sizeof(VParsedSide));
	S.TopTexture = "-";
	S.MidTexture = "-";
	S.BotTexture = "-";

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		sc.ExpectString();
		VStr Key = sc.String;
		sc.Expect("=");
		sc.ExpectString();
		VStr Val = sc.String;
		if (Val == "-" && sc.CheckFloat())
		{
			Val += sc.String;
		}
		sc.Expect(";");

		if (!Key.ICmp("offsetx"))
		{
			float FVal = atof(*Val);
			S.S.TopTextureOffset = FVal;
			S.S.MidTextureOffset = FVal;
			S.S.BotTextureOffset = FVal;
		}
		else if (!Key.ICmp("offsety"))
		{
			float FVal = atof(*Val);
			S.S.TopRowOffset = FVal;
			S.S.MidRowOffset = FVal;
			S.S.BotRowOffset = FVal;
		}
		else if (!Key.ICmp("texturetop"))
		{
			S.TopTexture = Val;
		}
		else if (!Key.ICmp("texturebottom"))
		{
			S.BotTexture = Val;
		}
		else if (!Key.ICmp("texturemiddle"))
		{
			S.MidTexture = Val;
		}
		else if (!Key.ICmp("sector"))
		{
			S.SectorIndex = atoi(*Val);
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseThing
//
//==========================================================================

void VUdmfParser::ParseThing(VScriptParser& sc)
{
	guard(VUdmfParser::ParseThing);
	mthing_t& T = ParsedThings.Alloc();
	memset(&T, 0, sizeof(mthing_t));

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		sc.ExpectString();
		VStr Key = sc.String;
		sc.Expect("=");
		sc.ExpectString();
		VStr Val = sc.String;
		if (Val == "-" && sc.CheckFloat())
		{
			Val += sc.String;
		}
		sc.Expect(";");

		if (!Key.ICmp("x"))
		{
			T.x = atof(*Val);
		}
		else if (!Key.ICmp("y"))
		{
			T.y = atof(*Val);
		}
		else if (!Key.ICmp("height"))
		{
			T.height = atof(*Val);
		}
		else if (!Key.ICmp("angle"))
		{
			T.angle = atoi(*Val);
		}
		else if (!Key.ICmp("type"))
		{
			T.type = atoi(*Val);
		}
		else if (!Key.ICmp("ambush"))
		{
			Flag(T.options, MTF_AMBUSH, Val);
		}
		else if (!Key.ICmp("single"))
		{
			Flag(T.options, MTF_GSINGLE, Val);
		}
		else if (!Key.ICmp("dm"))
		{
			Flag(T.options, MTF_GDEATHMATCH, Val);
		}
		else if (!Key.ICmp("coop"))
		{
			Flag(T.options, MTF_GCOOP, Val);
		}
		else if (!Key.ICmp("friend"))
		{
			Flag(T.options, MTF_FRIENDLY, Val);
		}
		else if (!Key.ICmp("standing"))
		{
			Flag(T.options, MTF_STANDSTILL, Val);
		}
		else if (!Key.ICmp("strifeally"))
		{
			Flag(T.options, MTF_FRIENDLY, Val);
		}
		else if (!Key.ICmp("translucent"))
		{
			Flag(T.options, MTF_SHADOW, Val);
		}
		else if (!Key.ICmp("invisible"))
		{
			Flag(T.options, MTF_ALTSHADOW, Val);
		}
		else if (!Key.ICmp("skill1"))
		{
			Flag(T.SkillClassFilter, 0x0001, Val);
		}
		else if (!Key.ICmp("skill2"))
		{
			Flag(T.SkillClassFilter, 0x0002, Val);
		}
		else if (!Key.ICmp("skill3"))
		{
			Flag(T.SkillClassFilter, 0x0004, Val);
		}
		else if (!Key.ICmp("skill4"))
		{
			Flag(T.SkillClassFilter, 0x0008, Val);
		}
		else if (!Key.ICmp("skill5"))
		{
			Flag(T.SkillClassFilter, 0x0010, Val);
		}
		else if (!Key.ICmp("skill6"))
		{
			Flag(T.SkillClassFilter, 0x0020, Val);
		}
		else if (!Key.ICmp("skill7"))
		{
			Flag(T.SkillClassFilter, 0x0040, Val);
		}
		else if (!Key.ICmp("skill8"))
		{
			Flag(T.SkillClassFilter, 0x0080, Val);
		}
		else if (!Key.ICmp("skill9"))
		{
			Flag(T.SkillClassFilter, 0x0100, Val);
		}
		else if (!Key.ICmp("skill10"))
		{
			Flag(T.SkillClassFilter, 0x0200, Val);
		}
		else if (!Key.ICmp("skill11"))
		{
			Flag(T.SkillClassFilter, 0x0400, Val);
		}
		else if (!Key.ICmp("skill12"))
		{
			Flag(T.SkillClassFilter, 0x0800, Val);
		}
		else if (!Key.ICmp("skill13"))
		{
			Flag(T.SkillClassFilter, 0x1000, Val);
		}
		else if (!Key.ICmp("skill14"))
		{
			Flag(T.SkillClassFilter, 0x2000, Val);
		}
		else if (!Key.ICmp("skill15"))
		{
			Flag(T.SkillClassFilter, 0x4000, Val);
		}
		else if (!Key.ICmp("skill16"))
		{
			Flag(T.SkillClassFilter, 0x8000, Val);
		}

		if (bExtended)
		{
			if (!Key.ICmp("id"))
			{
				T.tid = atoi(*Val);
			}
			else if (!Key.ICmp("dormant"))
			{
				Flag(T.options, MTF_DORMANT, Val);
			}
			else if (!Key.ICmp("class1"))
			{
				Flag(T.SkillClassFilter, 0x00010000, Val);
			}
			else if (!Key.ICmp("class2"))
			{
				Flag(T.SkillClassFilter, 0x00020000, Val);
			}
			else if (!Key.ICmp("class3"))
			{
				Flag(T.SkillClassFilter, 0x00040000, Val);
			}
			else if (!Key.ICmp("class4"))
			{
				Flag(T.SkillClassFilter, 0x00080000, Val);
			}
			else if (!Key.ICmp("class5"))
			{
				Flag(T.SkillClassFilter, 0x00100000, Val);
			}
			else if (!Key.ICmp("class6"))
			{
				Flag(T.SkillClassFilter, 0x00200000, Val);
			}
			else if (!Key.ICmp("class7"))
			{
				Flag(T.SkillClassFilter, 0x00400000, Val);
			}
			else if (!Key.ICmp("class8"))
			{
				Flag(T.SkillClassFilter, 0x00800000, Val);
			}
			else if (!Key.ICmp("class9"))
			{
				Flag(T.SkillClassFilter, 0x01000000, Val);
			}
			else if (!Key.ICmp("class10"))
			{
				Flag(T.SkillClassFilter, 0x02000000, Val);
			}
			else if (!Key.ICmp("class11"))
			{
				Flag(T.SkillClassFilter, 0x04000000, Val);
			}
			else if (!Key.ICmp("class12"))
			{
				Flag(T.SkillClassFilter, 0x08000000, Val);
			}
			else if (!Key.ICmp("class13"))
			{
				Flag(T.SkillClassFilter, 0x10000000, Val);
			}
			else if (!Key.ICmp("class14"))
			{
				Flag(T.SkillClassFilter, 0x20000000, Val);
			}
			else if (!Key.ICmp("class15"))
			{
				Flag(T.SkillClassFilter, 0x40000000, Val);
			}
			else if (!Key.ICmp("class16"))
			{
				Flag(T.SkillClassFilter, 0x80000000, Val);
			}
			else if (!Key.ICmp("special"))
			{
				T.special = atoi(*Val);
			}
			else if (!Key.ICmp("arg0"))
			{
				T.arg1 = atoi(*Val);
			}
			else if (!Key.ICmp("arg1"))
			{
				T.arg2 = atoi(*Val);
			}
			else if (!Key.ICmp("arg2"))
			{
				T.arg3 = atoi(*Val);
			}
			else if (!Key.ICmp("arg3"))
			{
				T.arg4 = atoi(*Val);
			}
			else if (!Key.ICmp("arg4"))
			{
				T.arg5 = atoi(*Val);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::Flag
//
//==========================================================================

void VUdmfParser::Flag(int& Field, int Mask, const VStr& Val)
{
	guard(VUdmfParser::Flag);
	if (!Val.ICmp("true"))
	{
		Field |= Mask;
	}
	else
	{
		Field &= ~Mask;
	}
	unguard;
}

//==========================================================================
//
//	VLevel::LoadTextMap
//
//==========================================================================

void VLevel::LoadTextMap(int Lump)
{
	guard(VLevel::LoadTextMap);
	VUdmfParser Parser;
	Parser.Parse(this, Lump);

	if (Parser.bExtended)
	{
		LevelFlags |= LF_Extended;
	}

	//	Copy vertexes.
	NumVertexes = Parser.ParsedVertexes.Num();
	Vertexes = new vertex_t[NumVertexes];
	memcpy(Vertexes, Parser.ParsedVertexes.Ptr(), sizeof(vertex_t) * NumVertexes);

	//	Copy sectors.
	NumSectors = Parser.ParsedSectors.Num();
	Sectors = new sector_t[NumSectors];
	for (int i = 0; i < NumSectors; i++)
	{
		sector_t& S = Sectors[i];
		Sectors[i] = Parser.ParsedSectors[i];

		//	Region
		sec_region_t* region = new sec_region_t;
		memset(region, 0, sizeof(*region));
		region->floor = &S.floor;
		region->ceiling = &S.ceiling;
		region->params = &S.params;
		S.topregion = region;
		S.botregion = region;
	}
	HashSectors();

	//	Copy line defs.
	NumLines = Parser.ParsedLines.Num();
	Lines = new line_t[NumLines];
	for (int i = 0; i < NumLines; i++)
	{
		Lines[i] = Parser.ParsedLines[i].L;
		if (Parser.ParsedLines[i].V1Index < 0 ||
			Parser.ParsedLines[i].V1Index >= NumVertexes)
		{
			Host_Error("Bad vertex index %d", Parser.ParsedLines[i].V1Index);
		}
		if (Parser.ParsedLines[i].V2Index < 0 ||
			Parser.ParsedLines[i].V2Index >= NumVertexes)
		{
			Host_Error("Bad vertex index %d", Parser.ParsedLines[i].V2Index);
		}
		Lines[i].v1 = &Vertexes[Parser.ParsedLines[i].V1Index];
		Lines[i].v2 = &Vertexes[Parser.ParsedLines[i].V2Index];
	}

	if (!(LevelFlags & LF_Extended))
	{
		//	Translate level to Hexen format
		GGameInfo->eventTranslateLevel(this);
	}

	//	Copy side defs.
	NumSides = Parser.ParsedSides.Num();
	CreateSides();
	side_t* sd = Sides;
	for (int i = 0; i < NumSides; i++, sd++)
	{
		VUdmfParser::VParsedSide& Src = Parser.ParsedSides[sd->BottomTexture];
		int Spec = sd->MidTexture;
		int Tag = sd->TopTexture;
		*sd = Src.S;

		if (Src.SectorIndex < 0 || Src.SectorIndex >= NumSectors)
		{
			Host_Error("Bad sector index %d", Src.SectorIndex);
		}
		sd->Sector = &Sectors[Src.SectorIndex];

		switch (Spec)
		{
		case LNSPEC_LineTranslucent:
			//	In BOOM midtexture can be translucency table lump name.
			sd->MidTexture = GTextureManager.CheckNumForName(
				VName(*Src.MidTexture, VName::AddLower8),
				TEXTYPE_Wall, true, true);
			if (sd->MidTexture == -1)
			{
				sd->MidTexture = 0;
			}
			sd->TopTexture = TexNumForName(*Src.TopTexture, TEXTYPE_Wall);
			sd->BottomTexture = TexNumForName(*Src.BotTexture, TEXTYPE_Wall);
			break;

		case LNSPEC_TransferHeights:
			sd->MidTexture = TexNumForName(*Src.MidTexture, TEXTYPE_Wall, true);
			sd->TopTexture = TexNumForName(*Src.TopTexture, TEXTYPE_Wall, true);
			sd->BottomTexture = TexNumForName(*Src.BotTexture, TEXTYPE_Wall, true);
			break;

		case LNSPEC_StaticInit:
			{
				bool HaveCol;
				bool HaveFade;
				vuint32 Col;
				vuint32 Fade;
				sd->MidTexture = TexNumForName(*Src.MidTexture, TEXTYPE_Wall);
				int TmpTop = TexNumOrColour(*Src.TopTexture, TEXTYPE_Wall,
					HaveCol, Col);
				sd->BottomTexture = TexNumOrColour(*Src.BotTexture, TEXTYPE_Wall,
					HaveFade, Fade);
				if (HaveCol || HaveFade)
				{
					for (int j = 0; j < NumSectors; j++)
					{
						if (Sectors[j].tag == Tag)
						{
							if (HaveCol)
							{
								Sectors[j].params.LightColour = Col;
							}
							if (HaveFade)
							{
								Sectors[j].params.Fade = Fade;
							}
						}
					}
				}
				sd->TopTexture = TmpTop;
			}
			break;

		default:
			sd->MidTexture = TexNumForName(*Src.MidTexture, TEXTYPE_Wall);
			sd->TopTexture = TexNumForName(*Src.TopTexture, TEXTYPE_Wall);
			sd->BottomTexture = TexNumForName(*Src.BotTexture, TEXTYPE_Wall);
			break;
		}
	}

	//	Copy things.
	NumThings = Parser.ParsedThings.Num();
	Things = new mthing_t[NumThings];
	memcpy(Things, Parser.ParsedThings.Ptr(), sizeof(mthing_t) * NumThings);
	unguard;
}
