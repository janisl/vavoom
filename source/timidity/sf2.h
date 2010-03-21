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
//**	Copyright (C) 1999-2010 Jānis Legzdiņš
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

#ifndef __SF2_H__
#define __SF2_H__

#define mmioFOURCC(A, B, C, D)    \
(((A) <<  0) | ((B) <<  8) | ((C) << 16) | ((D) << 24))

#define FOURCC_sfbk		mmioFOURCC('s', 'f', 'b', 'k')
#define FOURCC_INFO		mmioFOURCC('I', 'N', 'F', 'O')
#define FOURCC_ifil		mmioFOURCC('i', 'f', 'i', 'l')
#define FOURCC_isng		mmioFOURCC('i', 's', 'n', 'g')
#define FOURCC_INAM		mmioFOURCC('I', 'N', 'A', 'M')
#define FOURCC_irom		mmioFOURCC('i', 'r', 'o', 'm')
#define FOURCC_iver		mmioFOURCC('i', 'v', 'e', 'r')
#define FOURCC_ICRD		mmioFOURCC('I', 'C', 'R', 'D')
#define FOURCC_IENG		mmioFOURCC('I', 'E', 'N', 'G')
#define FOURCC_IPRD		mmioFOURCC('I', 'P', 'R', 'D')
#define FOURCC_ICOP		mmioFOURCC('I', 'C', 'O', 'P')
#define FOURCC_ICMT		mmioFOURCC('I', 'C', 'M', 'T')
#define FOURCC_ISFT		mmioFOURCC('I', 'S', 'F', 'T')
#define FOURCC_sdta		mmioFOURCC('s', 'd', 't', 'a')
#define FOURCC_smpl		mmioFOURCC('s', 'm', 'p', 'l')
#define FOURCC_pdta		mmioFOURCC('p', 'd', 't', 'a')
#define FOURCC_phdr		mmioFOURCC('p', 'h', 'd', 'r')
#define FOURCC_pbag		mmioFOURCC('p', 'b', 'a', 'g')
#define FOURCC_pmod		mmioFOURCC('p', 'm', 'o', 'd')
#define FOURCC_pgen		mmioFOURCC('p', 'g', 'e', 'n')
#define FOURCC_inst		mmioFOURCC('i', 'n', 's', 't')
#define FOURCC_ibag		mmioFOURCC('i', 'b', 'a', 'g')
#define FOURCC_imod		mmioFOURCC('i', 'm', 'o', 'd')
#define FOURCC_igen		mmioFOURCC('i', 'g', 'e', 'n')
#define FOURCC_shdr		mmioFOURCC('s', 'h', 'd', 'r')

#pragma pack(push, 1)

//  Generators.
enum
{
	SFGEN_StartAddrsOffset,				//  0
	SFGEN_EndAddrsOffset,				//  1
	SFGEN_StartloopAddrsOffset,			//  2
	SFGEN_EndloopAddrsOffset,			//  3
	SFGEN_StartAddrsCoarseOffset,		//  4
	SFGEN_ModLfoToPitch,				//  5
	SFGEN_VibLfoToPitch,				//  6
	SFGEN_ModEnvToPitch,				//  7
	SFGEN_InitialFilterFc,				//  8
	SFGEN_InitialFilterQ,				//  9
	SFGEN_ModLfoToFilterFc,				//  10
	SFGEN_ModEnvToFilterFc,				//  11
	SFGEN_EndAddrsCoarseOffset,			//  12
	SFGEN_ModLfoToVolume,				//  13
	SFGEN_Unused1,						//  14
	SFGEN_ChorusEffectsSend,			//  15
	SFGEN_ReverbEffectsSend,			//  16
	SFGEN_Pan,							//  17
	SFGEN_Unused2,						//  18
	SFGEN_Unused3,						//  19
	SFGEN_Unused4,						//  20
	SFGEN_DelayModLfo,					//  21
	SFGEN_FreqModLfo,					//  22
	SFGEN_DelayVibLfo,					//  23
	SFGEN_FreqVibLfo,					//  24
	SFGEN_DelayModEnv,					//  25
	SFGEN_AttackModEnv,					//  26
	SFGEN_HoldModEnv,					//  27
	SFGEN_DecayModEnv,					//  28
	SFGEN_SustainModEnv,				//  29
	SFGEN_ReleaseModEnv,				//  30
	SFGEN_KeynumToModEnvHold,			//  31
	SFGEN_KeynumToModEnvDecay,			//  32
	SFGEN_DelayVolEnv,					//  33
	SFGEN_AttackVolEnv,					//  34
	SFGEN_HoldVolEnv,					//  35
	SFGEN_DecayVolEnv,					//  36
	SFGEN_SustainVolEnv,				//  37
	SFGEN_ReleaseVolEnv,				//  38
	SFGEN_KeynumToVolEnvHold,			//  39
	SFGEN_KeynumToVolEnvDecay,			//  40
	SFGEN_Instrument,					//  41
	SFGEN_Reserved1,					//  42
	SFGEN_KeyRange,						//  43
	SFGEN_VelRange,						//  44
	SFGEN_StartloopAddrsCoarseOffset,	//  45
	SFGEN_Keynum,						//  46
	SFGEN_Velocity,						//  47
	SFGEN_InitialAttenuation,			//  48
	SFGEN_Reserved2,					//  49
	SFGEN_EndloopAddrsCoarseOffset,		//  50
	SFGEN_CoarseTune,					//  51
	SFGEN_FineTune,						//  52
	SFGEN_SampleId,						//  53
	SFGEN_SampleModes,					//  54
	SFGEN_Reserved3,					//  55
	SFGEN_ScaleTuning,					//  56
	SFGEN_ExclusiveClass,				//  57
	SFGEN_OverridingRootKey,			//  58
	SFGEN_Unused5,						//  59
	SFGEN_EndOper,						//  60
};

enum
{
	SFMODSRC_NoControler			= 0,
	SFMODSRC_NoteOnVelocity			= 2,
	SFMODSRC_NoteOnKeyNumber		= 3,
	SFMODSRC_PolyPressure			= 10,
	SFMODSRC_ChannelPressure		= 13,
	SFMODSRC_PitchWheel				= 14,
	SFMODSRC_PitchWheelSensitivity	= 16,
};

enum
{
	SFMODTYPE_Linear,	//  0
	SFMODTYPE_Concave,	//  1
	SFMODTYPE_Convex,	//  2
	SFMODTYPE_Switch,	//  3
};

enum
{
	SFMODTRANS_Linear,	//  0
};

enum
{
	SFSAMPLE_MonoSample		= 1,
	SFSAMPLE_RightSample	= 2,
	SFSAMPLE_LeftSample		= 4,
	SFSAMPLE_LinkedSample	= 8,

	SFSAMPLE_RomSample		= 0x8000,
};

enum
{
	SFSAMPLEMODE_NoLoop,	//  0
	SFSAMPLEMODE_ContLoop,	//  1
	SFSAMPLEMODE_Unused,	//  2
	SFSAMPLEMODE_KeyLoop,	//  3
};

struct SFVersion
{
	uint16		Major;
	uint16		Minor;
};

struct SFPresetHeader
{
	char		PresetName[20];
	uint16		Preset;
	uint16		Bank;
	uint16		BagNdx;
	uint32		Library;
	uint32		Genre;
	uint32		Morphology;
};

struct SFBag
{
	uint16		GenNdx;
	uint16		ModNdx;
};

struct SFMod
{
	uint16		SrcOper;
	uint16		DestOper;
	int16		Amount;
	uint16		AmtSrcOper;
	uint16		TransOper;
};

struct SFRange
{
	uint8		Lo;
	uint8		Hi;
};

struct SFGen
{
	uint16		Oper;
	union
	{
		SFRange	Range;
		int16	Amount;
	};
};

struct SFInst
{
	char		InstName[20];
	uint16		BagNdx;
};

struct SFSample
{
	char		SampleName[20];
	uint32		Start;
	uint32		End;
	uint32		Startloop;
	uint32		Endloop;
	uint32		SampleRate;
	uint8		OriginalPitch;
	int8		PitchCorrection;
	uint16		SampleLink;
	uint16		SampleType;
};

#pragma pack(pop)

#endif
