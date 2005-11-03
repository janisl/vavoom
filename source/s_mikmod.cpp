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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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

#ifdef _WIN32
#include "winlocal.h"
#endif
#include <mikmod.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VMikModAudioCodec : public VAudioCodec
{
public:
	struct FMikModArchiveReader
	{
		MREADER		Core;
		FArchive*	Ar;
	};

	MODULE*			Module;

	static bool		MikModInitialised;
	static MDRIVER	Driver;

	//	VAudioCodec interface.
	VMikModAudioCodec(MODULE* InModule);
	~VMikModAudioCodec();
	int Decode(short* Data, int NumSamples);
	bool Finished();
	void Restart();

	//	Driver functions.
	static BOOL Drv_IsThere();
	static void Drv_Update();
	static BOOL Drv_Reset();

	//	Archive reader functions.
	static BOOL ArchiveReader_Seek(MREADER* rd, long offset, int whence);
	static long ArchiveReader_Tell(MREADER* rd);
	static BOOL ArchiveReader_Read(MREADER* rd, void *dest, size_t length);
	static int ArchiveReader_Get(MREADER* rd);
	static BOOL ArchiveReader_Eof(MREADER* rd);

	static VAudioCodec* Create(FArchive* InAr);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_AUDIO_CODEC(VMikModAudioCodec, "MikMod");

bool			VMikModAudioCodec::MikModInitialised;
MDRIVER			VMikModAudioCodec::Driver =
{
	NULL,
	"vavoom",
	"vavoom output driver",
	0,
	255,
#if (LIBMIKMOD_VERSION > 0x030106)
	"vavoom",
	NULL,
#endif
	VMikModAudioCodec::Drv_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	VC_Init,
	VC_Exit,
	VMikModAudioCodec::Drv_Reset,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	VMikModAudioCodec::Drv_Update,
	NULL,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

static TCvarI	s_mikmod_hqmixer("s_mikmod_hqmixer", "0", CVAR_ARCHIVE);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VMikModAudioCodec::VMikModAudioCodec
//
//==========================================================================

VMikModAudioCodec::VMikModAudioCodec(MODULE* InModule)
: Module(InModule)
{
}

//==========================================================================
//
//	VMikModAudioCodec::~VMikModAudioCodec
//
//==========================================================================

VMikModAudioCodec::~VMikModAudioCodec()
{
	guard(VMikModAudioCodec::~VMikModAudioCodec);
	Player_Stop();
	Player_Free(Module);
	MikMod_Exit();
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::Decode
//
//==========================================================================

int VMikModAudioCodec::Decode(short* Data, int NumSamples)
{
	guard(VMikModAudioCodec::Decode);
	return VC_WriteBytes((SBYTE*)Data, NumSamples * 4) / 4;
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::Finished
//
//==========================================================================

bool VMikModAudioCodec::Finished()
{
	return !Player_Active();
}

//==========================================================================
//
//	VMikModAudioCodec::Restart
//
//==========================================================================

void VMikModAudioCodec::Restart()
{
	guard(VMikModAudioCodec::Restart);
	Player_SetPosition(0);
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::Drv_IsThere
//
//==========================================================================

BOOL VMikModAudioCodec::Drv_IsThere()
{
	return 1;
}

//==========================================================================
//
//	VMikModAudioCodec::Drv_Update
//
//==========================================================================

void VMikModAudioCodec::Drv_Update()
{
}

//==========================================================================
//
//	VMikModAudioCodec::Drv_Reset
//
//==========================================================================

BOOL VMikModAudioCodec::Drv_Reset()
{
	guard(VMikModAudioCodec::Drv_Reset);
	VC_Exit();
	return VC_Init();
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::ArchiveReader_Seek
//
//==========================================================================

BOOL VMikModAudioCodec::ArchiveReader_Seek(MREADER* rd, long offset, int whence)
{
	guard(VMikModAudioCodec::ArchiveReader_Seek);
	FArchive* Ar = ((FMikModArchiveReader*)rd)->Ar;
	switch (whence)
	{
	case SEEK_SET:
		Ar->Seek(offset);
		break;
	case SEEK_CUR:
		Ar->Seek(Ar->Tell() + offset);
		break;
	case SEEK_END:
		Ar->Seek(Ar->TotalSize() + offset);
		break;
	}
	return !Ar->IsError();
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::ArchiveReader_Tell
//
//==========================================================================

long VMikModAudioCodec::ArchiveReader_Tell(MREADER* rd)
{
	guard(VMikModAudioCodec::ArchiveReader_Tell);
	FArchive* Ar = ((FMikModArchiveReader*)rd)->Ar;
	return Ar->Tell();
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::ArchiveReader_Read
//
//==========================================================================

BOOL VMikModAudioCodec::ArchiveReader_Read(MREADER* rd, void *dest, size_t length)
{
	guard(VMikModAudioCodec::ArchiveReader_Read);
	FArchive* Ar = ((FMikModArchiveReader*)rd)->Ar;
	Ar->Serialise(dest, length);
	return !Ar->IsError();
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::ArchiveReader_Get
//
//==========================================================================

int VMikModAudioCodec::ArchiveReader_Get(MREADER* rd)
{
	guard(VMikModAudioCodec::ArchiveReader_Get);
	FArchive* Ar = ((FMikModArchiveReader*)rd)->Ar;
	if (Ar->AtEnd())
		return EOF;
	else
	{
		byte c;
		*Ar << c;
		return c;
	}
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::ArchiveReader_Eof
//
//==========================================================================

BOOL VMikModAudioCodec::ArchiveReader_Eof(MREADER* rd)
{
	guard(VMikModAudioCodec::ArchiveReader_Eof);
	FArchive* Ar = ((FMikModArchiveReader*)rd)->Ar;
	return Ar->AtEnd();
	unguard;
}

//==========================================================================
//
//	VMikModAudioCodec::Create
//
//==========================================================================

VAudioCodec* VMikModAudioCodec::Create(FArchive* InAr)
{
	guard(VMikModAudioCodec::Create);
	if (!MikModInitialised)
	{
		//	Register our driver and all the loaders.
		MikMod_RegisterDriver(&Driver);
		MikMod_RegisterAllLoaders();
		MikModInitialised = true;
	}

	//	Set up playback parameters.
	md_mixfreq = 44100;
	md_mode = DMODE_16BITS | DMODE_SOFT_MUSIC | DMODE_STEREO;
	if (s_mikmod_hqmixer)
		md_mode |= DMODE_HQMIXER;

	//	Initialise MikMod.
	if (MikMod_Init(""))
	{
		GCon->Logf("MikMod init failed");
		return NULL;
	}

	//	Create a reader.
	FMikModArchiveReader Reader;
	Reader.Core.Eof  = ArchiveReader_Eof;
	Reader.Core.Read = ArchiveReader_Read;
	Reader.Core.Get  = ArchiveReader_Get;
	Reader.Core.Seek = ArchiveReader_Seek;
	Reader.Core.Tell = ArchiveReader_Tell;
	Reader.Ar = InAr;
	InAr->Seek(0);

	//	Try to load the song.
	MODULE* module = Player_LoadGeneric(&Reader.Core, 256, 0);
	if (!module)
	{
		//	Not a module file.
		MikMod_Exit();
		return NULL;
	}

	//	Close archive.
	InAr->Close();
	delete InAr;

	//	Start playback.
	Player_Start(module);
	return new VMikModAudioCodec(module);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2005/11/03 22:46:35  dj_jl
//	Support for any bitrate streams.
//
//	Revision 1.1  2005/10/29 15:05:13  dj_jl
//	Added MikMod driver.
//	
//**************************************************************************
