//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

TCvarI				sfx_volume("sfx_volume", "8", CVAR_ARCHIVE);
TCvarI				music_volume("music_volume", "8", CVAR_ARCHIVE);
TCvarI				swap_stereo("swap_stereo", "0", CVAR_ARCHIVE);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char			mapSong[12];
static int			mapCDTrack;

static TCvarI		cd_music("use_cd_music", "0", CVAR_ARCHIVE);
static boolean		CDMusic = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	S_Init
//
// 	Initializes sound stuff, including volume
// 	Sets channels, SFX and music volume,
// allocates channel buffer, sets S_sfx lookup.
//
//==========================================================================

void S_Init(void)
{
	S_InitSfx();
	S_InitMusic();
	CD_Init();

	S_InitScript();
	SN_InitSequenceScript();
}

//==========================================================================
//
//	S_Shutdown
//
//	Shuts down all sound stuff
//
//==========================================================================

void S_Shutdown(void)
{
	CD_Shutdown();
	S_ShutdownMusic();
	S_ShutdownSfx();
}

//==========================================================================
//
//	S_StartSong
//
//==========================================================================

void S_StartSong(char* song, int track, boolean loop)
{
	if (CDMusic)
	{
    	if (loop)
			CmdBuf << "CD Loop " << track << "\n";
		else
			CmdBuf << "CD Play " << track << "\n";
	}
	else
	{
    	if (loop)
			CmdBuf << "Music Loop " << song << "\n";
		else
			CmdBuf << "Music Play " << song << "\n";
	}
}

//==========================================================================
//
//	S_PauseSound
//
//==========================================================================

void S_PauseSound(void)
{
	if (CDMusic)
	{
    	CmdBuf << "CD Pause\n";
	}
	else
	{
    	CmdBuf << "Music Pause\n";
	}
}

//==========================================================================
//
//	S_ResumeSound
//
//==========================================================================

void S_ResumeSound(void)
{
	if (CDMusic)
	{
    	CmdBuf << "CD resume\n";
	}
	else
	{
    	CmdBuf << "Music resume\n";
	}
}

//==========================================================================
//
//  StartMusic
//
//==========================================================================

static void StartMusic(void)
{
	S_StartSong(mapSong, mapCDTrack, true);
}

//==========================================================================
//
//	S_Start
//
//	Per level startup code. Kills playing sounds at start of level,
// determines music if any, changes music.
//
//==========================================================================

void S_Start(const mapInfo_t &info)
{
	SN_StopAllSequences();
	S_StopAllSound();

	strcpy(mapSong, info.songLump);
	mapCDTrack = info.cdTrack;

	StartMusic();
}	

//==========================================================================
//
// S_UpdateSounds
//
// Updates music & sounds
//
//==========================================================================

void S_UpdateSounds(void)
{
	if ((int)cd_music && !CDMusic)
    {
    	CmdBuf << "Music Stop\n";
		CDMusic = true;
		StartMusic();
    }
	if (!(int)cd_music && CDMusic)
    {
    	CmdBuf << "CD Stop\n";
		CDMusic = false;
		StartMusic();
    }

	// Update any Sequences
	SN_UpdateActiveSequences();

	S_UpdateSfx();
	S_UpdateMusic();
	CD_Update();
}

