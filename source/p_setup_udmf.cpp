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
	//	Supported namespaces. Use bits to have faster cheks.
	enum
	{
		//	Standard namespaces.
		NS_Doom				= 0x01,
		NS_Heretic			= 0x02,
		NS_Hexen			= 0x04,
		NS_Strife			= 0x08,
		//	Vavoom's namespace.
		NS_Vavoom			= 0x10,
		//	ZDoom's namespaces.
		NS_ZDoom			= 0x20,
		NS_ZDoomTranslated	= 0x40,
	};

	enum
	{
		TK_None,
		TK_Int,
		TK_Float,
		TK_String,
		TK_Identifier,
	};

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

	VScriptParser		sc;
	bool				bExtended;
	vuint8				NS;
	VStr				Key;
	int					ValType;
	int					ValInt;
	float				ValFloat;
	VStr				Val;
	TArray<vertex_t>	ParsedVertexes;
	TArray<sector_t>	ParsedSectors;
	TArray<VParsedLine>	ParsedLines;
	TArray<VParsedSide>	ParsedSides;
	TArray<mthing_t>	ParsedThings;

	VUdmfParser(int);
	void Parse(VLevel*, const mapInfo_t&);
	void ParseVertex();
	void ParseSector(VLevel*);
	void ParseLineDef(const mapInfo_t&);
	void ParseSideDef();
	void ParseThing();
	void ParseKey();
	int CheckInt();
	float CheckFloat();
	bool CheckBool();
	VStr CheckString();
	void Flag(int&, int);
	void Flag(vuint32&, int);
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
//	VUdmfParser::VUdmfParser
//
//==========================================================================

VUdmfParser::VUdmfParser(int Lump)
: sc("textmap", W_CreateLumpReaderNum(Lump))
{
}

//==========================================================================
//
//	VUdmfParser::Parse
//
//==========================================================================

void VUdmfParser::Parse(VLevel* Level, const mapInfo_t& MInfo)
{
	guard(VUdmfParser::Parse);
	sc.SetCMode(true);

	bExtended = false;

	//	Get namespace name.
	sc.Expect("namespace");
	sc.Expect("=");
	sc.ExpectString();
	VStr Namespace = sc.String;
	sc.Expect(";");
	//	Vavoom's namespace.
	if (!Namespace.ICmp("Vavoom"))
	{
		NS = NS_Vavoom;
		bExtended = true;
	}
	//	Standard namespaces.
	else if (!Namespace.ICmp("Doom"))
	{
		NS = NS_Doom;
	}
	else if (!Namespace.ICmp("Heretic"))
	{
		NS = NS_Heretic;
	}
	else if (!Namespace.ICmp("Hexen"))
	{
		NS = NS_Hexen;
		bExtended = true;
	}
	else if (!Namespace.ICmp("Strife"))
	{
		NS = NS_Strife;
	}
	//	ZDoom namespaces.
	else if (!Namespace.ICmp("ZDoom"))
	{
		NS = NS_ZDoom;
		bExtended = true;
	}
	else if (!Namespace.ICmp("ZDoomTranslated"))
	{
		NS = NS_ZDoomTranslated;
	}
	else
	{
		//	Unknown namespace.
		NS = 0;
	}

	while (!sc.AtEnd())
	{
		if (sc.Check("vertex"))
		{
			ParseVertex();
		}
		else if (sc.Check("sector"))
		{
			ParseSector(Level);
		}
		else if (sc.Check("linedef"))
		{
			ParseLineDef(MInfo);
		}
		else if (sc.Check("sidedef"))
		{
			ParseSideDef();
		}
		else if (sc.Check("thing"))
		{
			ParseThing();
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

void VUdmfParser::ParseVertex()
{
	guard(VUdmfParser::ParseVertex);
	//	Allocate a new vertex.
	vertex_t& V = ParsedVertexes.Alloc();
	V = TVec(0, 0, 0);

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		ParseKey();
		if (!Key.ICmp("x"))
		{
			V.x = CheckFloat();
		}
		else if (!Key.ICmp("y"))
		{
			V.y = CheckFloat();
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseSector
//
//==========================================================================

void VUdmfParser::ParseSector(VLevel* Level)
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
	S.Zone = -1;

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		ParseKey();
		if (!Key.ICmp("heightfloor"))
		{
			float FVal = CheckFloat();
			S.floor.dist = FVal;
			S.floor.TexZ = FVal;
			S.floor.minz = FVal;
			S.floor.maxz = FVal;
		}
		else if (!Key.ICmp("heightceiling"))
		{
			float FVal = CheckFloat();
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
			S.params.lightlevel = CheckInt();
		}
		else if (!Key.ICmp("special"))
		{
			S.special = CheckInt();
		}
		else if (!Key.ICmp("id"))
		{
			S.tag = CheckInt();
		}

		//	Extensions.
		if (NS & (NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("xpanningfloor"))
			{
				S.floor.xoffs = CheckFloat();
			}
			else if (!Key.ICmp("ypanningfloor"))
			{
				S.floor.yoffs = CheckFloat();
			}
			else if (!Key.ICmp("xpanningceiling"))
			{
				S.ceiling.xoffs = CheckFloat();
			}
			else if (!Key.ICmp("ypanningceiling"))
			{
				S.ceiling.yoffs = CheckFloat();
			}
			else if (!Key.ICmp("xscalefloor"))
			{
				S.floor.XScale = CheckFloat();
			}
			else if (!Key.ICmp("yscalefloor"))
			{
				S.floor.YScale = CheckFloat();
			}
			else if (!Key.ICmp("xscaleceiling"))
			{
				S.ceiling.XScale = CheckFloat();
			}
			else if (!Key.ICmp("yscaleceiling"))
			{
				S.ceiling.YScale = CheckFloat();
			}
			else if (!Key.ICmp("rotationfloor"))
			{
				S.floor.Angle = CheckFloat();
			}
			else if (!Key.ICmp("rotationceiling"))
			{
				S.ceiling.Angle = CheckFloat();
			}
			else if (!Key.ICmp("gravity"))
			{
				S.Gravity = CheckFloat();
			}
			else if (!Key.ICmp("lightcolor"))
			{
				S.params.LightColour = ParseHex(*CheckString());
			}
			else if (!Key.ICmp("fadecolor"))
			{
				S.params.Fade = ParseHex(*CheckString());
			}
			else if (!Key.ICmp("silent"))
			{
				Flag(S.SectorFlags, sector_t::SF_Silent);
			}
			else if (!Key.ICmp("nofallingdamage"))
			{
				Flag(S.SectorFlags, sector_t::SF_NoFallingDamage);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseLineDef
//
//==========================================================================

void VUdmfParser::ParseLineDef(const mapInfo_t& MInfo)
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
	if (MInfo.Flags & MAPINFOF_ClipMidTex)
	{
		L.L.flags |= ML_CLIP_MIDTEX;
	}
	if (MInfo.Flags & MAPINFOF_WrapMidTex)
	{
		L.L.flags |= ML_WRAP_MIDTEX;
	}
	bool HavePassUse = false;

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		ParseKey();
		if (!Key.ICmp("id"))
		{
			L.L.LineTag = CheckInt();
		}
		else if (!Key.ICmp("v1"))
		{
			L.V1Index = CheckInt();
		}
		else if (!Key.ICmp("v2"))
		{
			L.V2Index = CheckInt();
		}
		else if (!Key.ICmp("blocking"))
		{
			Flag(L.L.flags, ML_BLOCKING);
		}
		else if (!Key.ICmp("blockmonsters"))
		{
			Flag(L.L.flags, ML_BLOCKMONSTERS);
		}
		else if (!Key.ICmp("twosided"))
		{
			Flag(L.L.flags, ML_TWOSIDED);
		}
		else if (!Key.ICmp("dontpegtop"))
		{
			Flag(L.L.flags, ML_DONTPEGTOP);
		}
		else if (!Key.ICmp("dontpegbottom"))
		{
			Flag(L.L.flags, ML_DONTPEGBOTTOM);
		}
		else if (!Key.ICmp("secret"))
		{
			Flag(L.L.flags, ML_SECRET);
		}
		else if (!Key.ICmp("blocksound"))
		{
			Flag(L.L.flags, ML_SOUNDBLOCK);
		}
		else if (!Key.ICmp("dontdraw"))
		{
			Flag(L.L.flags, ML_DONTDRAW);
		}
		else if (!Key.ICmp("mapped"))
		{
			Flag(L.L.flags, ML_MAPPED);
		}
		else if (!Key.ICmp("special"))
		{
			L.L.special = CheckInt();
		}
		else if (!Key.ICmp("arg0"))
		{
			L.L.arg1 = CheckInt();
		}
		else if (!Key.ICmp("arg1"))
		{
			L.L.arg2 = CheckInt();
		}
		else if (!Key.ICmp("arg2"))
		{
			L.L.arg3 = CheckInt();
		}
		else if (!Key.ICmp("arg3"))
		{
			L.L.arg4 = CheckInt();
		}
		else if (!Key.ICmp("arg4"))
		{
			L.L.arg5 = CheckInt();
		}
		else if (!Key.ICmp("sidefront"))
		{
			L.L.sidenum[0] = CheckInt();
		}
		else if (!Key.ICmp("sideback"))
		{
			L.L.sidenum[1] = CheckInt();
		}

		//	Doom specific flags.
		if (NS & (NS_Doom | NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("passuse"))
			{
				if (bExtended)
				{
					HavePassUse = CheckBool();
				}
				else
				{
					Flag(L.L.flags, ML_PASSUSE_BOOM);
				}
			}
		}

		//	Strife specific flags.
		if (NS & (NS_Strife | NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("translucent"))
			{
				L.L.alpha = CheckBool() ? 0.666 : 1.0;
			}
			else if (!Key.ICmp("jumpover"))
			{
				Flag(L.L.flags, ML_RAILING);
			}
			else if (!Key.ICmp("blockfloaters"))
			{
				Flag(L.L.flags, ML_BLOCK_FLOATERS);
			}
		}

		//	Hexen's extensions.
		if (NS & (NS_Hexen | NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("playercross"))
			{
				Flag(L.L.SpacFlags, SPAC_Cross);
			}
			else if (!Key.ICmp("playeruse"))
			{
				Flag(L.L.SpacFlags, SPAC_Use);
			}
			else if (!Key.ICmp("monstercross"))
			{
				Flag(L.L.SpacFlags, SPAC_MCross);
			}
			else if (!Key.ICmp("monsteruse"))
			{
				Flag(L.L.SpacFlags, SPAC_MUse);
			}
			else if (!Key.ICmp("impact"))
			{
				Flag(L.L.SpacFlags, SPAC_Impact);
			}
			else if (!Key.ICmp("playerpush"))
			{
				Flag(L.L.SpacFlags, SPAC_Push);
			}
			else if (!Key.ICmp("monsterpush"))
			{
				Flag(L.L.SpacFlags, SPAC_MPush);
			}
			else if (!Key.ICmp("missilecross"))
			{
				Flag(L.L.SpacFlags, SPAC_PCross);
			}
			else if (!Key.ICmp("repeatspecial"))
			{
				Flag(L.L.flags, ML_REPEAT_SPECIAL);
			}
		}

		//	Extensions.
		if (NS & (NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("alpha"))
			{
				L.L.alpha = CheckFloat();
				L.L.alpha = MID(0, L.L.alpha, 1);
			}
			else if (!Key.ICmp("renderstyle"))
			{
				VStr RS = CheckString();
				if (!RS.ICmp("translucent"))
				{
					L.L.flags &= ~ML_ADDITIVE;
				}
				else if (!RS.ICmp("add"))
				{
					L.L.flags |= ML_ADDITIVE;
				}
				else
				{
					sc.Message("Bad render style");
				}
			}
			else if (!Key.ICmp("anycross"))
			{
				Flag(L.L.SpacFlags, SPAC_AnyCross);
			}
			else if (!Key.ICmp("monsteractivate"))
			{
				Flag(L.L.flags, ML_MONSTERSCANACTIVATE);
			}
			else if (!Key.ICmp("blockplayers"))
			{
				Flag(L.L.flags, ML_BLOCKPLAYERS);
			}
			else if (!Key.ICmp("blockeverything"))
			{
				Flag(L.L.flags, ML_BLOCKEVERYTHING);
			}
			else if (!Key.ICmp("firstsideonly"))
			{
				Flag(L.L.flags, ML_FIRSTSIDEONLY);
			}
			else if (!Key.ICmp("zoneboundary"))
			{
				Flag(L.L.flags, ML_ZONEBOUNDARY);
			}
			else if (!Key.ICmp("clipmidtex"))
			{
				Flag(L.L.flags, ML_CLIP_MIDTEX);
			}
			else if (!Key.ICmp("wrapmidtex"))
			{
				Flag(L.L.flags, ML_WRAP_MIDTEX);
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

void VUdmfParser::ParseSideDef()
{
	guard(VUdmfParser::ParseSideDef);
	VParsedSide& S = ParsedSides.Alloc();
	memset(&S, 0, sizeof(VParsedSide));
	S.TopTexture = "-";
	S.MidTexture = "-";
	S.BotTexture = "-";
	float XOffs = 0;
	float YOffs = 0;

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		ParseKey();
		if (!Key.ICmp("offsetx"))
		{
			XOffs = CheckFloat();
		}
		else if (!Key.ICmp("offsety"))
		{
			YOffs = CheckFloat();
		}
		else if (!Key.ICmp("texturetop"))
		{
			S.TopTexture = CheckString();
		}
		else if (!Key.ICmp("texturebottom"))
		{
			S.BotTexture = CheckString();
		}
		else if (!Key.ICmp("texturemiddle"))
		{
			S.MidTexture = CheckString();
		}
		else if (!Key.ICmp("sector"))
		{
			S.SectorIndex = CheckInt();
		}

		//	Extensions.
		if (NS & (NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("offsetx_top"))
			{
				S.S.TopTextureOffset = CheckFloat();
			}
			else if (!Key.ICmp("offsety_top"))
			{
				S.S.TopRowOffset = CheckFloat();
			}
			else if (!Key.ICmp("offsetx_mid"))
			{
				S.S.MidTextureOffset = CheckFloat();
			}
			else if (!Key.ICmp("offsety_mid"))
			{
				S.S.MidRowOffset = CheckFloat();
			}
			else if (!Key.ICmp("offsetx_bottom"))
			{
				S.S.BotTextureOffset = CheckFloat();
			}
			else if (!Key.ICmp("offsety_bottom"))
			{
				S.S.BotRowOffset = CheckFloat();
			}
			else if (!Key.ICmp("light"))
			{
				S.S.Light = CheckInt();
			}
			else if (!Key.ICmp("lightabsolute"))
			{
				Flag(S.S.Flags, SDF_ABSLIGHT);
			}
		}
	}

	S.S.TopTextureOffset += XOffs;
	S.S.MidTextureOffset += XOffs;
	S.S.BotTextureOffset += XOffs;
	S.S.TopRowOffset += YOffs;
	S.S.MidRowOffset += YOffs;
	S.S.BotRowOffset += YOffs;
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseThing
//
//==========================================================================

void VUdmfParser::ParseThing()
{
	guard(VUdmfParser::ParseThing);
	mthing_t& T = ParsedThings.Alloc();
	memset(&T, 0, sizeof(mthing_t));

	sc.Expect("{");
	while (!sc.Check("}"))
	{
		ParseKey();
		if (!Key.ICmp("x"))
		{
			T.x = CheckFloat();
		}
		else if (!Key.ICmp("y"))
		{
			T.y = CheckFloat();
		}
		else if (!Key.ICmp("height"))
		{
			T.height = CheckFloat();
		}
		else if (!Key.ICmp("angle"))
		{
			T.angle = CheckInt();
		}
		else if (!Key.ICmp("type"))
		{
			T.type = CheckInt();
		}
		else if (!Key.ICmp("ambush"))
		{
			Flag(T.options, MTF_AMBUSH);
		}
		else if (!Key.ICmp("single"))
		{
			Flag(T.options, MTF_GSINGLE);
		}
		else if (!Key.ICmp("dm"))
		{
			Flag(T.options, MTF_GDEATHMATCH);
		}
		else if (!Key.ICmp("coop"))
		{
			Flag(T.options, MTF_GCOOP);
		}
		else if (!Key.ICmp("skill1"))
		{
			Flag(T.SkillClassFilter, 0x0001);
		}
		else if (!Key.ICmp("skill2"))
		{
			Flag(T.SkillClassFilter, 0x0002);
		}
		else if (!Key.ICmp("skill3"))
		{
			Flag(T.SkillClassFilter, 0x0004);
		}
		else if (!Key.ICmp("skill4"))
		{
			Flag(T.SkillClassFilter, 0x0008);
		}
		else if (!Key.ICmp("skill5"))
		{
			Flag(T.SkillClassFilter, 0x0010);
		}
		else if (!Key.ICmp("skill6"))
		{
			Flag(T.SkillClassFilter, 0x0020);
		}
		else if (!Key.ICmp("skill7"))
		{
			Flag(T.SkillClassFilter, 0x0040);
		}
		else if (!Key.ICmp("skill8"))
		{
			Flag(T.SkillClassFilter, 0x0080);
		}
		else if (!Key.ICmp("skill9"))
		{
			Flag(T.SkillClassFilter, 0x0100);
		}
		else if (!Key.ICmp("skill10"))
		{
			Flag(T.SkillClassFilter, 0x0200);
		}
		else if (!Key.ICmp("skill11"))
		{
			Flag(T.SkillClassFilter, 0x0400);
		}
		else if (!Key.ICmp("skill12"))
		{
			Flag(T.SkillClassFilter, 0x0800);
		}
		else if (!Key.ICmp("skill13"))
		{
			Flag(T.SkillClassFilter, 0x1000);
		}
		else if (!Key.ICmp("skill14"))
		{
			Flag(T.SkillClassFilter, 0x2000);
		}
		else if (!Key.ICmp("skill15"))
		{
			Flag(T.SkillClassFilter, 0x4000);
		}
		else if (!Key.ICmp("skill16"))
		{
			Flag(T.SkillClassFilter, 0x8000);
		}

		//	MBF friendly flag.
		if (NS & (NS_Hexen | NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("friend"))
			{
				Flag(T.options, MTF_FRIENDLY);
			}
		}

		//	Strife specific flags.
		if (NS & (NS_Hexen | NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("standing"))
			{
				Flag(T.options, MTF_STANDSTILL);
			}
			else if (!Key.ICmp("strifeally"))
			{
				Flag(T.options, MTF_FRIENDLY);
			}
			else if (!Key.ICmp("translucent"))
			{
				Flag(T.options, MTF_SHADOW);
			}
			else if (!Key.ICmp("invisible"))
			{
				Flag(T.options, MTF_ALTSHADOW);
			}
		}

		//	Hexen's extensions.
		if (NS & (NS_Hexen | NS_Vavoom | NS_ZDoom | NS_ZDoomTranslated))
		{
			if (!Key.ICmp("id"))
			{
				T.tid = CheckInt();
			}
			else if (!Key.ICmp("dormant"))
			{
				Flag(T.options, MTF_DORMANT);
			}
			else if (!Key.ICmp("class1"))
			{
				Flag(T.SkillClassFilter, 0x00010000);
			}
			else if (!Key.ICmp("class2"))
			{
				Flag(T.SkillClassFilter, 0x00020000);
			}
			else if (!Key.ICmp("class3"))
			{
				Flag(T.SkillClassFilter, 0x00040000);
			}
			else if (!Key.ICmp("class4"))
			{
				Flag(T.SkillClassFilter, 0x00080000);
			}
			else if (!Key.ICmp("class5"))
			{
				Flag(T.SkillClassFilter, 0x00100000);
			}
			else if (!Key.ICmp("class6"))
			{
				Flag(T.SkillClassFilter, 0x00200000);
			}
			else if (!Key.ICmp("class7"))
			{
				Flag(T.SkillClassFilter, 0x00400000);
			}
			else if (!Key.ICmp("class8"))
			{
				Flag(T.SkillClassFilter, 0x00800000);
			}
			else if (!Key.ICmp("class9"))
			{
				Flag(T.SkillClassFilter, 0x01000000);
			}
			else if (!Key.ICmp("class10"))
			{
				Flag(T.SkillClassFilter, 0x02000000);
			}
			else if (!Key.ICmp("class11"))
			{
				Flag(T.SkillClassFilter, 0x04000000);
			}
			else if (!Key.ICmp("class12"))
			{
				Flag(T.SkillClassFilter, 0x08000000);
			}
			else if (!Key.ICmp("class13"))
			{
				Flag(T.SkillClassFilter, 0x10000000);
			}
			else if (!Key.ICmp("class14"))
			{
				Flag(T.SkillClassFilter, 0x20000000);
			}
			else if (!Key.ICmp("class15"))
			{
				Flag(T.SkillClassFilter, 0x40000000);
			}
			else if (!Key.ICmp("class16"))
			{
				Flag(T.SkillClassFilter, 0x80000000);
			}
			else if (!Key.ICmp("special"))
			{
				T.special = CheckInt();
			}
			else if (!Key.ICmp("arg0"))
			{
				T.arg1 = CheckInt();
			}
			else if (!Key.ICmp("arg1"))
			{
				T.arg2 = CheckInt();
			}
			else if (!Key.ICmp("arg2"))
			{
				T.arg3 = CheckInt();
			}
			else if (!Key.ICmp("arg3"))
			{
				T.arg4 = CheckInt();
			}
			else if (!Key.ICmp("arg4"))
			{
				T.arg5 = CheckInt();
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VUdmfParser::ParseKey
//
//==========================================================================

void VUdmfParser::ParseKey()
{
	guard(VUdmfParser::ParseKey);
	//	Get key and value.
	sc.ExpectString();
	Key = sc.String;
	sc.Expect("=");

	ValType = TK_None;
	if (sc.Check("+"))
	{
		if (sc.CheckNumber())
		{
			ValType = TK_Int;
			ValInt = sc.Number;
		}
		else if (sc.CheckFloat())
		{
			ValType = TK_Float;
			ValFloat = sc.Float;
		}
		else
		{
			sc.Message("Numeric constant expected");
		}
	}
	else if (sc.Check("-"))
	{
		if (sc.CheckNumber())
		{
			ValType = TK_Int;
			ValInt = -sc.Number;
		}
		else if (sc.CheckFloat())
		{
			ValType = TK_Float;
			ValFloat = -sc.Float;
		}
		else
		{
			sc.Message("Numeric constant expected");
		}
	}
	else if (sc.CheckNumber())
	{
		ValType = TK_Int;
		ValInt = sc.Number;
	}
	else if (sc.CheckFloat())
	{
		ValType = TK_Float;
		ValFloat = sc.Float;
	}
	else if (sc.CheckQuotedString())
	{
		ValType = TK_String;
		Val = sc.String;
	}
	else
	{
		sc.ExpectString();
		ValType = TK_Identifier;
		Val = sc.String;
	}
	sc.Expect(";");
	unguard;
}

//==========================================================================
//
//	VUdmfParser::CheckInt
//
//==========================================================================

int VUdmfParser::CheckInt()
{
	guardSlow(VUdmfParser::CheckInt);
	if (ValType != TK_Int)
	{
		sc.Message(va("Integer value expected for key %s", *Key));
	}
	return ValInt;
	unguardSlow;
}

//==========================================================================
//
//	VUdmfParser::CheckFloat
//
//==========================================================================

float VUdmfParser::CheckFloat()
{
	guardSlow(VUdmfParser::CheckFloat);
	if (ValType != TK_Int && ValType != TK_Float)
	{
		sc.Message(va("Float value expected for key %s", *Key));
	}
	return ValType == TK_Int ? ValInt : ValFloat;
	unguardSlow;
}

//==========================================================================
//
//	VUdmfParser::CheckBool
//
//==========================================================================

bool VUdmfParser::CheckBool()
{
	guardSlow(VUdmfParser::CheckBool);
	if (ValType == TK_Identifier)
	{
		if (!Val.ICmp("true"))
		{
			return true;
		}
		if (!Val.ICmp("false"))
		{
			return false;
		}
	}
	sc.Message(va("Boolean value expected for key %s", *Key));
	return false;
	unguardSlow;
}

//==========================================================================
//
//	VUdmfParser::CheckString
//
//==========================================================================

VStr VUdmfParser::CheckString()
{
	guardSlow(VUdmfParser::CheckString);
	if (ValType != TK_String)
	{
		sc.Message(va("String value expected for key %s", *Key));
	}
	return Val;
	unguardSlow;
}

//==========================================================================
//
//	VUdmfParser::Flag
//
//==========================================================================

void VUdmfParser::Flag(int& Field, int Mask)
{
	guard(VUdmfParser::Flag);
	if (CheckBool())
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
//	VUdmfParser::Flag
//
//==========================================================================

void VUdmfParser::Flag(vuint32& Field, int Mask)
{
	guard(VUdmfParser::Flag);
	if (CheckBool())
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

void VLevel::LoadTextMap(int Lump, const mapInfo_t& MInfo)
{
	guard(VLevel::LoadTextMap);
	VUdmfParser Parser(Lump);
	Parser.Parse(this, MInfo);

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
