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

#ifndef __GF1_H__
#define __GF1_H__

#pragma pack(push, 1)

//	Don't know what the differences are
#define GF1_MAGIC1		"GF1PATCH110\0"
#define GF1_MAGIC2		"GF1PATCH100\0"
#define GF1_PATCH_ID	"ID#000002\0"

struct GF1PatchHeader
{
	char		Magic[12];
	char		Id[10];
	char		Description[60];
	uint8		NumInstruments;
	uint8		NumVoices;
	uint8		NumChannels;
	uint16		NumWaveForms;
	uint16		MasterVolume;
	uint32		DataSize;
	uint8		Reserved[36];
};

struct GF1InstrumentHeader
{
	uint16		Instrument;
	char		Name[16];
	uint32		Size;
	uint8		NumLayers;
	uint8		Reserved[40];
};

struct GF1LayerHeader
{
	uint8		Duplicate;
	uint8		Layer;
	uint32		Size;
	uint8		NumSamples;
	uint8		Reserved[40];
};

struct GF1SampleHeader
{
	char		WaveName[7];
	uint8		Fractions;
	uint32		DataLength;
	uint32		LoopStart;
	uint32		LoopEnd;
	uint16		SampleRate;
	uint32		LowFreq;
	uint32		HighFreq;
	uint32		RootFreq;
	int16		Tuning;
	uint8		Panning;
	uint8		EnvelopeRate[6];
	uint8		EnvelopeOffset[6];
	uint8		TremoloSweep;
	uint8		TremoloRate;
	uint8		TremoloDepth;
	uint8		VibratoSweep;
	uint8		VibratoRate;
	uint8		VibratoDepth;
	uint8		Modes;
	int16		ScaleFrequency;
	uint16		ScaleFactor;
	uint8		Reserved[36];
};

#pragma pack(pop)

#endif
