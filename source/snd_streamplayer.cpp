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
#include "snd_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VStreamMusicPlayer::Init
//
//==========================================================================

void VStreamMusicPlayer::Init()
{
}

//==========================================================================
//
//	VStreamMusicPlayer::Shutdown
//
//==========================================================================

void VStreamMusicPlayer::Shutdown()
{
	guard(VStreamMusicPlayer::Shutdown);
	Stop();
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Tick
//
//==========================================================================

void VStreamMusicPlayer::Tick(float)
{
	guard(VStreamMusicPlayer::Tick);
	if (!StrmOpened)
		return;
	if (Stopping && FinishTime + 1.0 < Sys_Time())
	{
		//	Finish playback.
		Stop();
		return;
	}
	if (Paused)
	{
		//	Pause playback.
		return;
	}
	for (int Len = SoundDevice->GetStreamAvailable(); Len;
		Len = SoundDevice->GetStreamAvailable())
	{
		short* Data = SoundDevice->GetStreamBuffer();
		int StartPos = 0;
		while (!Stopping && StartPos < Len)
		{
			int SamplesDecoded = Codec->Decode(Data + StartPos * 2, Len - StartPos);
			StartPos += SamplesDecoded;
			if (Codec->Finished())
			{
				//	Stream ended.
				if (CurrLoop)
				{
					//	Restart stream.
					Codec->Restart();
				}
				else
				{
					//	We'll wait for 1 second to finish playing.
					Stopping = true;
					FinishTime = Sys_Time();
				}
			}
			else if (StartPos < Len)
			{
				//	Should never happen.
				GCon->Log("Stream decoded less but is not finished");
				Stopping = true;
				FinishTime = Sys_Time();
			}
		}
		if (Stopping)
		{
			memset(Data + StartPos * 2, 0, (Len - StartPos) * 4);
		}
		SoundDevice->SetStreamData(Data, Len);
	}
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Play
//
//==========================================================================

void VStreamMusicPlayer::Play(VAudioCodec* InCodec, const char* InName,
	bool InLoop)
{
	guard(VStreamMusicPlayer::Play);
	StrmOpened = SoundDevice->OpenStream(InCodec->SampleRate,
		InCodec->SampleBits, InCodec->NumChannels);
	if (!StrmOpened)
		return;
	Codec = InCodec;
	CurrSong = InName;
	CurrLoop = InLoop;
	Stopping = false;
	if (Paused)
	{
		Resume();
	}
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Pause
//
//==========================================================================

void VStreamMusicPlayer::Pause()
{
	guard(VStreamMusicPlayer::Pause);
	if (!StrmOpened)
		return;
	SoundDevice->PauseStream();
	Paused = true;
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Resume
//
//==========================================================================

void VStreamMusicPlayer::Resume()
{
	guard(VStreamMusicPlayer::Resume);
	if (!StrmOpened)
		return;
	SoundDevice->ResumeStream();
	Paused = false;
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Stop
//
//==========================================================================

void VStreamMusicPlayer::Stop()
{
	guard(VStreamMusicPlayer::Stop);
	if (!StrmOpened)
		return;
	delete Codec;
	Codec = NULL;
	SoundDevice->CloseStream();
	StrmOpened = false;
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::IsPlaying
//
//==========================================================================

bool VStreamMusicPlayer::IsPlaying()
{
	guard(VStreamMusicPlayer::IsPlaying);
	if (!StrmOpened)
		return false;
	return true;
	unguard;
}
