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

#include <stdlib.h>
#include <string.h>

#include "timidity.h"

namespace LibTimidity
{

#include "sf2.h"

/*-------------------------------------------------------------------------*/
/* * * * * * * * * * * * * * * * * load_riff.h * * * * * * * * * * * * * * */
/*-------------------------------------------------------------------------*/
struct RIFF_Chunk
{
	uint32 magic;
	uint32 length;
	uint32 subtype;
	uint8  *data;
	RIFF_Chunk *child;
	RIFF_Chunk *next;
};

extern RIFF_Chunk* LoadRIFF(FILE *src);
extern void FreeRIFF(RIFF_Chunk *chunk);
extern void PrintRIFF(RIFF_Chunk *chunk, int level);

#define FOURCC_LIST    0x5453494c   /* "LIST" */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct Sf2Data
{
	RIFF_Chunk*		Riff;

	SFVersion		Version;

	int16*			SampleData;
	int				SampleDataSize;

	SFPresetHeader*	PresetHeaders;
	int				NumPresetHeaders;
	SFBag*			PresetBags;
	int				NumPresetBags;
	SFMod*			PresetModulators;
	int				NumPresetModulators;
	SFGen*			PresetGenerators;
	int				NumPresetGenerators;
	SFInst*			InstrumentHeaders;
	int				NumInstrumentHeaders;
	SFBag*			InstrumentBags;
	int				NumInstrumentBags;
	SFMod*			InstrumentModulators;
	int				NumInstrumentModulators;
	SFGen*			InstrumentGenerators;
	int				NumInstrumentGenerators;
	SFSample*		SampleHeaders;
	int				NumSampleHeaders;
};

enum
{
	//  Internal flag for marking invalid samples.
	SFSAMPLE_Bad		= 0x4000,
};

class ErrBadFile { };

struct Sf2GenInfo
{
	int16			Default;
};

//InstrGenData[SFGEN_
Sf2GenInfo GenInfos[SFGEN_EndOper] =
{
	{ 0 },  //  SFGEN_StartAddrsOffset
	{ 0 },  //  SFGEN_EndAddrsOffset
	{ 0 },  //  SFGEN_StartloopAddrsOffset
	{ 0 },  //  SFGEN_EndloopAddrsOffset
	{ 0 },  //  SFGEN_StartAddrsCoarseOffset
	{ 0 },  //  SFGEN_ModLfoToPitch
	{ 0 },  //  SFGEN_VibLfoToPitch
	{ 0 },  //  SFGEN_ModEnvToPitch
	{ 13500 },  //  SFGEN_InitialFilterFc
	{ 0 },  //  SFGEN_InitialFilterQ
	{ 0 },  //  SFGEN_ModLfoToFilterFc
	{ 0 },  //  SFGEN_ModEnvToFilterFc
	{ 0 },  //  SFGEN_EndAddrsCoarseOffset
	{ 0 },  //  SFGEN_ModLfoToVolume
	{ 0 },  //  SFGEN_Unused1
	{ 0 },  //  SFGEN_ChorusEffectsSend
	{ 0 },  //  SFGEN_ReverbEffectsSend
	{ 0 },  //  SFGEN_Pan
	{ 0 },  //  SFGEN_Unused2
	{ 0 },  //  SFGEN_Unused3
	{ 0 },  //  SFGEN_Unused4
	{ -12000 },  //  SFGEN_DelayModLfo
	{ 0 },  //  SFGEN_FreqModLfo
	{ -12000 },  //  SFGEN_DelayVibLfo
	{ 0 },  //  SFGEN_FreqVibLfo
	{ -12000 },  //  SFGEN_DelayModEnv
	{ -12000 },  //  SFGEN_AttackModEnv
	{ -12000 },  //  SFGEN_HoldModEnv
	{ -12000 },  //  SFGEN_DecayModEnv
	{ 0 },  //  SFGEN_SustainModEnv
	{ -12000 },  //  SFGEN_ReleaseModEnv
	{ 0 },  //  SFGEN_KeynumToModEnvHold
	{ 0 },  //  SFGEN_KeynumToModEnvDecay
	{ -12000 },  //  SFGEN_DelayVolEnv
	{ -12000 },  //  SFGEN_AttackVolEnv
	{ -12000 },  //  SFGEN_HoldVolEnv
	{ -12000 },  //  SFGEN_DecayVolEnv
	{ 0 },  //  SFGEN_SustainVolEnv
	{ -12000 },  //  SFGEN_ReleaseVolEnv
	{ 0 },  //  SFGEN_KeynumToVolEnvHold
	{ 0 },  //  SFGEN_KeynumToVolEnvDecay
	{ 0 },  //  SFGEN_Instrument
	{ 0 },  //  SFGEN_Reserved1
	{ 0 },  //  SFGEN_KeyRange
	{ 0 },  //  SFGEN_VelRange
	{ 0 },  //  SFGEN_StartloopAddrsCoarseOffset
	{ 0 },  //  SFGEN_Keynum
	{ 0 },  //  SFGEN_Velocity
	{ 0 },  //  SFGEN_InitialAttenuation
	{ 0 },  //  SFGEN_Reserved2
	{ 0 },  //  SFGEN_EndloopAddrsCoarseOffset
	{ 0 },  //  SFGEN_CoarseTune
	{ 0 },  //  SFGEN_FineTune
	{ 0 },  //  SFGEN_SampleId
	{ 0 },  //  SFGEN_SampleModes
	{ 0 },  //  SFGEN_Reserved3
	{ 100 },  //  SFGEN_ScaleTuning
	{ 0 },  //  SFGEN_ExclusiveClass
	{ -1 },  //  SFGEN_OverridingRootKey
	{ 0 },  //  SFGEN_Unused5
};

//==========================================================================
//
//	Timidity_FreeSf2
//
//==========================================================================

void Timidity_FreeSf2(Sf2Data* font)
{
	if (font->Riff)
	{
		FreeRIFF(font->Riff);
	}
	free(font);
}

//==========================================================================
//
//	ReadIfil
//
//==========================================================================

static void ReadIfil(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Verify length
	if (chunk->length != 4)
	{
		throw ErrBadFile();
	}

	SFVersion* Ver = (SFVersion*)chunk->data;
	font->Version.Major = LE_SHORT(Ver->Major);
	font->Version.Minor = LE_SHORT(Ver->Minor);
}

//==========================================================================
//
//	ReadInfo
//
//==========================================================================

static void ReadInfo(Sf2Data* font, RIFF_Chunk* list)
{
	for (RIFF_Chunk* chunk = list->child; chunk; chunk = chunk->next)
	{
		uint32 magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic)
		{
		case FOURCC_ifil:
			ReadIfil(font, chunk);
			break;
		}
	}
}

//==========================================================================
//
//	ReadSmpl
//
//==========================================================================

static void ReadSmpl(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->SampleData)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 2
	if (chunk->length % 2)
	{
		throw ErrBadFile();
	}

	font->SampleData = (int16*)chunk->data;
	font->SampleDataSize = chunk->length;
}

//==========================================================================
//
//	ReadSdta
//
//==========================================================================

static void ReadSdta(Sf2Data* font, RIFF_Chunk* list)
{
	for (RIFF_Chunk* chunk = list->child; chunk; chunk = chunk->next)
	{
		uint32 magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic)
		{
		case FOURCC_smpl:
			ReadSmpl(font, chunk);
			break;
		}
	}
}

//==========================================================================
//
//	ReadPhdr
//
//==========================================================================

static void ReadPhdr(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->PresetHeaders)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 38
	if (chunk->length % 38)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least 1 record and a terminal record
	if (chunk->length < 2 * 38)
	{
		throw ErrBadFile();
	}

	font->PresetHeaders = (SFPresetHeader*)chunk->data;
	font->NumPresetHeaders = chunk->length / sizeof(SFPresetHeader);
	int LastBagIndex = 0;
	for (int i = 0; i < font->NumPresetHeaders; i++)
	{
		SFPresetHeader* Hdr = &font->PresetHeaders[i];
		Hdr->Preset = LE_SHORT(Hdr->Preset);
		Hdr->Bank = LE_SHORT(Hdr->Bank);
		Hdr->BagNdx = LE_SHORT(Hdr->BagNdx);
		Hdr->Library = LE_LONG(Hdr->Library);
		Hdr->Genre = LE_LONG(Hdr->Genre);
		Hdr->Morphology = LE_LONG(Hdr->Morphology);

		//  Verify that bag indexes are sequential
		if (i > 0 && Hdr->BagNdx < LastBagIndex)
		{
			throw ErrBadFile();
		}
		LastBagIndex = Hdr->BagNdx;
	}
}

//==========================================================================
//
//	ReadPbag
//
//==========================================================================

static void ReadPbag(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->PresetBags)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 4
	if (chunk->length % 4)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least 1 record and a terminal record
	if (chunk->length < 2 * 4)
	{
		throw ErrBadFile();
	}

	font->PresetBags = (SFBag*)chunk->data;
	font->NumPresetBags = chunk->length / sizeof(SFBag);
	int LastGenIndex = 0;
	int LastModIndex = 0;
	for (int i = 0; i < font->NumPresetBags; i++)
	{
		SFBag* Bag = &font->PresetBags[i];
		Bag->GenNdx = LE_SHORT(Bag->GenNdx);
		Bag->ModNdx = LE_SHORT(Bag->ModNdx);

		//  Verify that generator and modulator indexes are sequential
		if (i > 0 && (Bag->GenNdx < LastGenIndex || Bag->GenNdx < LastModIndex))
		{
			throw ErrBadFile();
		}
		LastGenIndex = Bag->GenNdx;
		LastModIndex = Bag->ModNdx;
	}
}

//==========================================================================
//
//	ReadPmod
//
//==========================================================================

static void ReadPmod(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->PresetModulators)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 10
	if (chunk->length % 10)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least a terminal record
	if (chunk->length < 10)
	{
		throw ErrBadFile();
	}

	font->PresetModulators = (SFMod*)chunk->data;
	font->NumPresetModulators = chunk->length / sizeof(SFMod);
	for (int i = 0; i < font->NumPresetModulators; i++)
	{
		SFMod* Mod = &font->PresetModulators[i];
		Mod->SrcOper = LE_SHORT(Mod->SrcOper);
		Mod->DestOper = LE_SHORT(Mod->DestOper);
		Mod->Amount = LE_SHORT(Mod->Amount);
		Mod->AmtSrcOper = LE_SHORT(Mod->AmtSrcOper);
		Mod->TransOper = LE_SHORT(Mod->TransOper);
	}
}

//==========================================================================
//
//	ReadPgen
//
//==========================================================================

static void ReadPgen(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->PresetGenerators)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 4
	if (chunk->length % 4)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least 1 record and a terminal record
	if (chunk->length < 2 * 4)
	{
		throw ErrBadFile();
	}

	font->PresetGenerators = (SFGen*)chunk->data;
	font->NumPresetGenerators = chunk->length / sizeof(SFGen);
	for (int i = 0; i < font->NumPresetGenerators; i++)
	{
		SFGen* Gen = &font->PresetGenerators[i];
		Gen->Oper = LE_SHORT(Gen->Oper);
		if (Gen->Oper != SFGEN_KeyRange && Gen->Oper != SFGEN_VelRange)
			Gen->Amount = LE_SHORT(Gen->Amount);

		//  Map invalid operator values to an unused generator.
		if (Gen->Oper >= SFGEN_EndOper)
			Gen->Oper = SFGEN_Reserved1;
	}
}

//==========================================================================
//
//	ReadInst
//
//==========================================================================

static void ReadInst(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->InstrumentHeaders)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 22
	if (chunk->length % 22)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least 1 record and a terminal record
	if (chunk->length < 2 * 22)
	{
		throw ErrBadFile();
	}

	font->InstrumentHeaders = (SFInst*)chunk->data;
	font->NumInstrumentHeaders = chunk->length / sizeof(SFInst);
	int LastBagIndex = 0;
	for (int i = 0; i < font->NumInstrumentHeaders; i++)
	{
		SFInst* Hdr = &font->InstrumentHeaders[i];
		Hdr->BagNdx = LE_SHORT(Hdr->BagNdx);

		//  Verify that bag indexes are sequential
		if (i > 0 && Hdr->BagNdx < LastBagIndex)
		{
			throw ErrBadFile();
		}
		LastBagIndex = Hdr->BagNdx;
	}
}

//==========================================================================
//
//	ReadIbag
//
//==========================================================================

static void ReadIbag(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->InstrumentBags)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 4
	if (chunk->length % 4)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least 1 record and a terminal record
	if (chunk->length < 2 * 4)
	{
		throw ErrBadFile();
	}

	font->InstrumentBags = (SFBag*)chunk->data;
	font->NumInstrumentBags = chunk->length / sizeof(SFBag);
	int LastGenIndex = 0;
	int LastModIndex = 0;
	for (int i = 0; i < font->NumInstrumentBags; i++)
	{
		SFBag* Bag = &font->InstrumentBags[i];
		Bag->GenNdx = LE_SHORT(Bag->GenNdx);
		Bag->ModNdx = LE_SHORT(Bag->ModNdx);

		//  Verify that generator and modulator indexes are sequential
		if (i > 0 && (Bag->GenNdx < LastGenIndex || Bag->GenNdx < LastModIndex))
		{
			throw ErrBadFile();
		}
		LastGenIndex = Bag->GenNdx;
		LastModIndex = Bag->ModNdx;
	}
}

//==========================================================================
//
//	ReadImod
//
//==========================================================================

static void ReadImod(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->InstrumentModulators)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 10
	if (chunk->length % 10)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least a terminal record
	if (chunk->length < 10)
	{
		throw ErrBadFile();
	}

	font->InstrumentModulators = (SFMod*)chunk->data;
	font->NumInstrumentModulators = chunk->length / sizeof(SFMod);
	for (int i = 0; i < font->NumInstrumentModulators; i++)
	{
		SFMod* Mod = &font->InstrumentModulators[i];
		Mod->SrcOper = LE_SHORT(Mod->SrcOper);
		Mod->DestOper = LE_SHORT(Mod->DestOper);
		Mod->Amount = LE_SHORT(Mod->Amount);
		Mod->AmtSrcOper = LE_SHORT(Mod->AmtSrcOper);
		Mod->TransOper = LE_SHORT(Mod->TransOper);
	}
}

//==========================================================================
//
//	ReadIgen
//
//==========================================================================

static void ReadIgen(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->InstrumentGenerators)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 4
	if (chunk->length % 4)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least 1 record and a terminal record
	if (chunk->length < 2 * 4)
	{
		throw ErrBadFile();
	}

	font->InstrumentGenerators = (SFGen*)chunk->data;
	font->NumInstrumentGenerators = chunk->length / sizeof(SFGen);
	for (int i = 0; i < font->NumInstrumentGenerators; i++)
	{
		SFGen* Gen = &font->InstrumentGenerators[i];
		Gen->Oper = LE_SHORT(Gen->Oper);
		if (Gen->Oper != SFGEN_KeyRange && Gen->Oper != SFGEN_VelRange)
			Gen->Amount = LE_SHORT(Gen->Amount);

		//  Map invalid operator values to an unused generator.
		if (Gen->Oper >= SFGEN_EndOper)
			Gen->Oper = SFGEN_Reserved1;
	}
}

//==========================================================================
//
//	ReadShdr
//
//==========================================================================

static void ReadShdr(Sf2Data* font, RIFF_Chunk* chunk)
{
	//  Check for duplicate chunk.
	if (font->SampleHeaders)
	{
		throw ErrBadFile();
	}
	//  Verify length is a multiple of 46
	if (chunk->length % 46)
	{
		throw ErrBadFile();
	}
	//  Verify that it contains at least 1 record and a terminal record
	if (chunk->length < 2 * 46)
	{
		throw ErrBadFile();
	}

	font->SampleHeaders = (SFSample*)chunk->data;
	font->NumSampleHeaders = chunk->length / sizeof(SFSample);
	for (int i = 0; i < font->NumSampleHeaders; i++)
	{
		SFSample* Spl = &font->SampleHeaders[i];
		Spl->Start = LE_LONG(Spl->Start);
		Spl->End = LE_LONG(Spl->End);
		Spl->Startloop = LE_LONG(Spl->Startloop);
		Spl->Endloop = LE_LONG(Spl->Endloop);
		Spl->SampleRate = LE_LONG(Spl->SampleRate);
		Spl->SampleLink = LE_SHORT(Spl->SampleLink);
		Spl->SampleType = LE_SHORT(Spl->SampleType);
	}
}

//==========================================================================
//
//	ReadPdta
//
//==========================================================================

static void ReadPdta(Sf2Data* font, RIFF_Chunk* list)
{
	for (RIFF_Chunk* chunk = list->child; chunk; chunk = chunk->next)
	{
		uint32 magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic)
		{
		case FOURCC_phdr:
			ReadPhdr(font, chunk);
			break;
		case FOURCC_pbag:
			ReadPbag(font, chunk);
			break;
		case FOURCC_pmod:
			ReadPmod(font, chunk);
			break;
		case FOURCC_pgen:
			ReadPgen(font, chunk);
			break;
		case FOURCC_inst:
			ReadInst(font, chunk);
			break;
		case FOURCC_ibag:
			ReadIbag(font, chunk);
			break;
		case FOURCC_imod:
			ReadImod(font, chunk);
			break;
		case FOURCC_igen:
			ReadIgen(font, chunk);
			break;
		case FOURCC_shdr:
			ReadShdr(font, chunk);
			break;
		}
	}
}

//==========================================================================
//
//	LoadSf2Data
//
//==========================================================================

static void LoadSf2Data(Sf2Data* font)
{
	if (font->Riff->subtype != FOURCC_sfbk)  
	{
		throw ErrBadFile();
	}

	for (RIFF_Chunk* chunk = font->Riff->child; chunk; chunk = chunk->next)
	{
		uint32 magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic)
		{
		case FOURCC_INFO:
			ReadInfo(font, chunk);
			break;
		case FOURCC_sdta:
			ReadSdta(font, chunk);
			break;
		case FOURCC_pdta:
			ReadPdta(font, chunk);
			break;
		}
	}

	//  We only support version 2.x
	if (font->Version.Major != 2)
	{
		throw ErrBadFile();
	}

	//  Make sure all data has been loaded.
	if (!font->SampleData ||
		!font->PresetHeaders ||
		!font->PresetBags ||
		!font->PresetModulators ||
		!font->PresetGenerators ||
		!font->InstrumentHeaders ||
		!font->InstrumentBags ||
		!font->InstrumentModulators ||
		!font->InstrumentGenerators ||
		!font->SampleHeaders)
	{
		throw ErrBadFile();
	}

	//  Validate indexes. We already checked if they are sequential, so just check last indexes.
	if (font->PresetHeaders[font->NumPresetHeaders - 1].BagNdx > font->NumPresetBags - 1 ||
		font->PresetBags[font->NumPresetBags-1].GenNdx > font->NumPresetGenerators - 1 ||
		font->PresetBags[font->NumPresetBags-1].ModNdx > font->NumPresetModulators - 1 ||
		font->InstrumentHeaders[font->NumInstrumentHeaders-1].BagNdx > font->NumInstrumentBags - 1 ||
		font->InstrumentBags[font->NumInstrumentBags-1].GenNdx > font->NumInstrumentGenerators - 1 ||
		font->InstrumentBags[font->NumInstrumentBags-1].ModNdx > font->NumInstrumentModulators - 1)
	{
		throw ErrBadFile();
	}

	//  Validate instrument and sample header indexes.
	for (int i = 0; i < font->NumPresetGenerators - 1; i++)
	{
		if (font->PresetGenerators[i].Oper == SFGEN_Instrument &&
			font->PresetGenerators[i].Amount > font->NumInstrumentHeaders - 1)
		{
			throw ErrBadFile();
		}
	}
	for (int i = 0; i < font->NumInstrumentGenerators - 1; i++)
	{
		if (font->InstrumentGenerators[i].Oper == SFGEN_SampleId &&
			font->InstrumentGenerators[i].Amount > font->NumSampleHeaders - 1)
		{
			throw ErrBadFile();
		}
	}

	//  Check samples
	for (int i = 0; i < font->NumSampleHeaders - 1; i++)
	{
		SFSample* Spl = &font->SampleHeaders[i];
		int start = Spl->Start;
		int end = Spl->End;
		int startloop = Spl->Startloop;
		int endloop = Spl->Endloop;
		if (startloop < start)
		{
			Spl->SampleType |= SFSAMPLE_Bad;
		}
		if (endloop < startloop)
		{
			Spl->SampleType |= SFSAMPLE_Bad;
		}
		if (end < endloop)
		{
			Spl->SampleType |= SFSAMPLE_Bad;
		}
		if (end > font->SampleDataSize / 2)
		{
			Spl->SampleType |= SFSAMPLE_Bad;
		}
	}
	for (int i = 0; i < font->NumSampleHeaders - 1; i++)
	{
		for (int j = i + 1; j < font->NumSampleHeaders - 1; j++)
		{
			if (font->SampleHeaders[i].Start <= font->SampleHeaders[j].End &&
				font->SampleHeaders[i].End >= font->SampleHeaders[j].Start)
			{
				font->SampleHeaders[i].SampleType |= SFSAMPLE_Bad;
				font->SampleHeaders[j].SampleType |= SFSAMPLE_Bad;
			}
		}
	}
}

//==========================================================================
//
//	Timidity_LoadSF2
//
//==========================================================================

Sf2Data* Timidity_LoadSF2(const char* FileName)
{
	FILE* f = open_file(FileName, 1, OF_NORMAL);
	if (!f)
	{
		return NULL;
	}
	RIFF_Chunk* Riff = LoadRIFF(f);
	close_file(f);
	if (!Riff)
	{
		return NULL;
	}

	Sf2Data* font = (Sf2Data*)safe_malloc(sizeof(Sf2Data));
	memset(font, 0, sizeof(Sf2Data));
	font->Riff = Riff;
	try
	{
		LoadSf2Data(font);
	}
	catch (ErrBadFile&)
	{
		Timidity_FreeSf2(font);
		return NULL;
	}
	return font;
}

//==========================================================================
//
//	FindPreset
//
//==========================================================================

static int FindPreset(Sf2Data* font, int Bank, int Instr)
{
	if (!font)
	{
		return -1;
	}

	for (int i = 0; i < font->NumPresetHeaders - 1; i++)
	{
		if (font->PresetHeaders[i].Bank == Bank && font->PresetHeaders[i].Preset == Instr)
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
//	AddGenerators
//
//==========================================================================

static void AddGenerators(int16* Gens, int16* Add)
{
	for (int i = 0; i < SFGEN_EndOper; i++)
	{
		Gens[i] += Add[i];
	}
}

//==========================================================================
//
//	to_msec
//
//	convert timecents to sec
//
//==========================================================================

static double to_msec(int timecent)
{
	if (timecent == 0x80000000 || timecent == 0)
	{
		return 0.0;
	}
	return 1000.0 * pow(2.0, (double)(timecent) / 1200.0);
}

//==========================================================================
//
//	to_offset
//
//	convert from 8bit value to fractional offset (15.15)
//
//==========================================================================

static int32 to_offset(int offset)
{
	return (int32)offset << (7+15);
}

//==========================================================================
//
//	calc_rate
//
//	calculate ramp rate in fractional unit;
//	diff = 8bit, time = msec
//
//==========================================================================

static int32 calc_rate(MidiSong *song, int diff, int sample_rate, double msec)
{
	double rate;

	if (msec < 6)
	{
		msec = 6;
	}
	if (diff == 0)
	{
		diff = 255;
	}
	diff <<= (7+15);
	rate = ((double)diff / OUTPUT_RATE) * song->control_ratio * 1000.0 / msec;
	return (int32)rate;
}

//==========================================================================
//
//	to_normalized_percent
//
//	convert decipercent to {0..1}
//
//==========================================================================

static double to_normalized_percent(int decipercent)
{
	if (decipercent < 0)
	{
		return 0;
	}
	return ((double)decipercent) / 1000.0;
}

//==========================================================================
//
//	LoadPreset
//
//==========================================================================

static Instrument* LoadPreset(Sf2Data* font, MidiSong* song, int PresetIndex, bool Drum, int DrumNote)
{
	if (font == NULL)
	{
		return NULL;
	}

	if (PresetIndex > (font->NumPresetHeaders - 1))
	{
		return NULL;
	}

	int PresetBagFrom = font->PresetHeaders[PresetIndex].BagNdx;
	int PresetBagTo = font->PresetHeaders[PresetIndex + 1].BagNdx;

	int NumSamples = 0;
	for (int PBagIdx = PresetBagFrom; PBagIdx < PresetBagTo; PBagIdx++)
	{
		int PresetGenFrom = font->PresetBags[PBagIdx].GenNdx;
		int PresetGenTo = font->PresetBags[PBagIdx + 1].GenNdx;
		for (int PGenIdx = PresetGenFrom; PGenIdx < PresetGenTo; PGenIdx++)
		{
			SFGen* PresetGen = &(font->PresetGenerators[PGenIdx]);
			if (PresetGen->Oper == SFGEN_Instrument)
			{
				int InstrBagFrom = font->InstrumentHeaders[PresetGen->Amount].BagNdx;
				int InstrBagTo = font->InstrumentHeaders[PresetGen->Amount + 1].BagNdx;
				for (int IBagIdx = InstrBagFrom; IBagIdx < InstrBagTo; IBagIdx++)
				{
					int InstrGenFrom = font->InstrumentBags[IBagIdx].GenNdx;
					int InstrGenTo = font->InstrumentBags[IBagIdx + 1].GenNdx;
					for (int IGenIdx = InstrGenFrom; IGenIdx < InstrGenTo; IGenIdx++)
					{
						SFGen* InstrGen = &font->InstrumentGenerators[IGenIdx];
						if (InstrGen->Oper == SFGEN_SampleId)
						{
							SFSample* SampleHdr = &font->SampleHeaders[InstrGen->Amount];
							if ((SampleHdr->SampleType & SFSAMPLE_Bad) ||
								(SampleHdr->SampleType & SFSAMPLE_RomSample))
							{
								break;
							}
							NumSamples++;
						}
					}
				}
			}
		}
	}
	if (Drum)
	{
		NumSamples = 1;
	}

	Instrument* ip = (Instrument*)safe_malloc(sizeof(Instrument));
	ip->type = INST_SF2;
	ip->samples = NumSamples;
	ip->sample = (Sample*)safe_malloc(sizeof(Sample) * ip->samples);
	memset(ip->sample, 0, ip->samples * sizeof(*ip->sample));

	int16 PresetDefaultGenData[SFGEN_EndOper];
	memset(PresetDefaultGenData, 0, sizeof(PresetDefaultGenData));
	bool IsDefaultPresetZone = true;

	int SampleIdx = 0;
	for (int PBagIdx = PresetBagFrom; PBagIdx < PresetBagTo; PBagIdx++)
	{
		int presKeyLo = 0;
		int presKeyHi = 127;
		int presVelLo = 0;
		int presVelHi = 127;
		int16 PresetGenData[SFGEN_EndOper];
		memcpy(PresetGenData, PresetDefaultGenData, sizeof(PresetDefaultGenData));

		int PresetGenFrom = font->PresetBags[PBagIdx].GenNdx;
		int PresetGenTo = font->PresetBags[PBagIdx + 1].GenNdx;
		for (int PGenIdx = PresetGenFrom; PGenIdx < PresetGenTo; PGenIdx++)
		{
			SFGen* PresetGen = &font->PresetGenerators[PGenIdx];
			if (PresetGen->Oper == SFGEN_KeyRange)
			{
				presKeyLo = PresetGen->Range.Lo;
				presKeyHi = PresetGen->Range.Hi;
			}
			else if (PresetGen->Oper == SFGEN_VelRange)
			{
				presVelLo =  PresetGen->Range.Lo;
				presVelHi = PresetGen->Range.Hi;
			}
			else if (PresetGen->Oper == SFGEN_Instrument)
			{
				int16 InstrDefaultGenData[SFGEN_EndOper];
				for (int i = 0; i < SFGEN_EndOper; i++)
				{
					InstrDefaultGenData[i] = GenInfos[i].Default;
				}
				bool IsDefaultInstrZone = true;

				int InstrBagFrom = font->InstrumentHeaders[PresetGen->Amount].BagNdx;
				int InstrBagTo = font->InstrumentHeaders[PresetGen->Amount + 1].BagNdx;
				for (int IBagIdx = InstrBagFrom; IBagIdx < InstrBagTo; IBagIdx++)
				{
					int keyLo = 0;
					int keyHi = 127;
					int velLo = 0;
					int velHi = 127;
					int16 InstrGenData[SFGEN_EndOper];
					memcpy(InstrGenData, InstrDefaultGenData, sizeof(InstrGenData));

					int InstrGenFrom = font->InstrumentBags[IBagIdx].GenNdx;
					int InstrGenTo = font->InstrumentBags[IBagIdx + 1].GenNdx;
					for (int IGenIdx = InstrGenFrom; IGenIdx < InstrGenTo; IGenIdx++)
					{
						SFGen* InstrGen = &font->InstrumentGenerators[IGenIdx];
						if (InstrGen->Oper == SFGEN_KeyRange)
						{
							if (Drum && DrumNote != InstrGen->Range.Lo)
							{
								break;
							}
							keyLo = InstrGen->Range.Lo;
							keyHi = InstrGen->Range.Hi;
						}
						else if (InstrGen->Oper == SFGEN_VelRange)
						{
							velLo = InstrGen->Range.Lo;
							velHi = InstrGen->Range.Hi;
						}
						else if (InstrGen->Oper == SFGEN_SampleId)
						{
							SFSample* SampleHdr = &font->SampleHeaders[InstrGen->Amount];
							if ((SampleHdr->SampleType & SFSAMPLE_Bad) ||
								(SampleHdr->SampleType & SFSAMPLE_RomSample))
							{
								break;
							}

							int Start = (int)InstrGenData[SFGEN_StartAddrsOffset] +
								(int) InstrGenData[SFGEN_StartAddrsCoarseOffset] * 32768 +
								SampleHdr->Start;

							int End = (int)InstrGenData[SFGEN_EndAddrsOffset] +
								(int)InstrGenData[SFGEN_EndAddrsCoarseOffset] * 32768 +
								SampleHdr->End;

							int Startloop = (int)InstrGenData[SFGEN_StartloopAddrsOffset] +
								(int)InstrGenData[SFGEN_StartloopAddrsCoarseOffset] * 32768 +
								SampleHdr->Startloop;

							int Endloop = (int)InstrGenData[SFGEN_EndloopAddrsOffset] +
								(int)InstrGenData[SFGEN_EndloopAddrsCoarseOffset] * 32768 +
								SampleHdr->Endloop;

							int OrigKey;
							if (InstrGenData[SFGEN_OverridingRootKey] >= 0 &&
								InstrGenData[SFGEN_OverridingRootKey] < 128)
							{
								OrigKey = InstrGenData[SFGEN_OverridingRootKey];
							}
							else if (SampleHdr->OriginalPitch <  128)
							{
								OrigKey = SampleHdr->OriginalPitch;
							}
							else
							{
								OrigKey = 60; 
							}

							AddGenerators(InstrGenData, PresetGenData);

							Sample *sp = &ip->sample[SampleIdx++];
							sp->loop_start = Startloop - Start;
							sp->loop_end = Endloop - Start;
							sp->data_length = End - Start;
							sp->sample_rate = SampleHdr->SampleRate;
							sp->low_vel = velLo;
							sp->high_vel = velHi;
							sp->low_freq = freq_table[keyLo];
							sp->high_freq = freq_table[keyHi];
							sp->root_freq = freq_table[OrigKey];
							sp->volume = 1;
							sp->panning = 127 * (InstrGenData[SFGEN_Pan] + 500) / 1000;
							sp->note_to_use = (int8)InstrGenData[SFGEN_Keynum];
							if (InstrGenData[SFGEN_SampleModes] == SFSAMPLEMODE_ContLoop)
							{
								sp->modes = MODES_LOOPING | MODES_SUSTAIN | MODES_ENVELOPE;
							}
							else if (InstrGenData[SFGEN_SampleModes] == SFSAMPLEMODE_KeyLoop)
							{
								sp->modes = MODES_LOOPING | MODES_SUSTAIN | MODES_ENVELOPE;
							}
							else
							{
								sp->modes = MODES_SUSTAIN | MODES_ENVELOPE;
							}
							sp->data = (int16*)safe_malloc(sp->data_length * 2);
							int16* Src = font->SampleData + Start;
							int16* Dst = sp->data;
							for (int i = 0; i < sp->data_length; i++)
							{
								*Dst = LE_SHORT(*Src);
								Src++;
								Dst++;
							}
							sp->data_length <<= FRACTION_BITS;
							sp->loop_start <<= FRACTION_BITS;
							sp->loop_end <<= FRACTION_BITS;

							double attack = to_msec(InstrGenData[SFGEN_AttackVolEnv]);
							double hold = to_msec(InstrGenData[SFGEN_HoldVolEnv]);
							double decay = to_msec(InstrGenData[SFGEN_DecayVolEnv]);
							double release = to_msec(InstrGenData[SFGEN_ReleaseVolEnv]);
							int sustain = (int)((1.0 - to_normalized_percent(InstrGenData[SFGEN_SustainVolEnv])) * 250.0);

							sp->envelope_offset[ATTACK] = to_offset(255);
							sp->envelope_rate[ATTACK] = calc_rate(song, 255, sp->sample_rate, attack);

							sp->envelope_offset[HOLD] = to_offset(250);
							sp->envelope_rate[HOLD] = calc_rate(song, 5, sp->sample_rate, hold);

							sp->envelope_offset[DECAY] = to_offset(sustain);
							sp->envelope_rate[DECAY] = calc_rate(song, 255 - sustain, sp->sample_rate, decay);

							sp->envelope_offset[RELEASE] = to_offset(0);
							sp->envelope_rate[RELEASE] = calc_rate(song, 5 + sustain, sp->sample_rate, release);

							sp->envelope_offset[RELEASEB] = to_offset(0);
							sp->envelope_rate[RELEASEB] = to_offset(1);

							sp->envelope_offset[RELEASEC] = to_offset(0);
							sp->envelope_rate[RELEASEC] = to_offset(1);

							if (Drum)
							{
								return ip;
							}

							IsDefaultInstrZone = false;
						}
						else
						{
							InstrGenData[InstrGen->Oper] = InstrGen->Amount;
						}
					}

					if (IsDefaultInstrZone)
					{
						memcpy(InstrDefaultGenData, InstrGenData, sizeof(InstrGenData));
					}
					IsDefaultInstrZone = false;
				}
				IsDefaultPresetZone = false;
			}
			else
			{
				PresetGenData[PresetGen->Oper] = PresetGen->Amount;
			}
		}

		if (IsDefaultPresetZone)
		{
			memcpy(PresetDefaultGenData, PresetGenData, sizeof(PresetDefaultGenData));
		}
		IsDefaultPresetZone = false;
	}

	return ip;
}

//==========================================================================
//
//	load_instrument_sf2
//
//==========================================================================

Instrument* load_instrument_sf2(MidiSong* song, int Bank, int Instr, bool Drum)
{
	if (!song->sf2_font)
	{ 
		return NULL; 
	}

	int PresetIndex; 
	if (Drum)
	{
		PresetIndex = FindPreset(song->sf2_font, 128, 0);
	}
	else
	{
		PresetIndex = FindPreset(song->sf2_font, Bank, Instr);
		if (PresetIndex < 0)
		{
			PresetIndex = FindPreset(song->sf2_font, 0, Instr);
		}
	}
	if (PresetIndex < 0)
	{
		return NULL; 
	}
	return LoadPreset(song->sf2_font, song, PresetIndex, Drum, Instr);
}

}
