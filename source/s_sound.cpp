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
	guard(S_Init);
	S_InitSfx();
	S_InitMusic();
	CD_Init();

	S_InitScript();
	SN_InitSequenceScript();
	unguard;
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
	guard(S_Shutdown);
	CD_Shutdown();
	S_ShutdownMusic();
	S_ShutdownSfx();
	unguard;
}

//==========================================================================
//
//	S_StartSong
//
//==========================================================================

void S_StartSong(char* song, int track, boolean loop)
{
	guard(S_StartSong);
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
	unguard;
}

//==========================================================================
//
//	S_PauseSound
//
//==========================================================================

void S_PauseSound(void)
{
	guard(S_PauseSound);
	if (CDMusic)
	{
    	CmdBuf << "CD Pause\n";
	}
	else
	{
    	CmdBuf << "Music Pause\n";
	}
	unguard;
}

//==========================================================================
//
//	S_ResumeSound
//
//==========================================================================

void S_ResumeSound(void)
{
	guard(S_ResumeSound);
	if (CDMusic)
	{
    	CmdBuf << "CD resume\n";
	}
	else
	{
    	CmdBuf << "Music resume\n";
	}
	unguard;
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
	guard(S_Start);
	SN_StopAllSequences();
	S_StopAllSound();

	strcpy(mapSong, info.songLump);
	mapCDTrack = info.cdTrack;

	StartMusic();
	unguard;
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
	guard(S_UpdateSounds);
	if (cd_music && !CDMusic)
    {
    	CmdBuf << "Music Stop\n";
		CDMusic = true;
		StartMusic();
    }
	if (!cd_music && CDMusic)
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
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//
//	Revision 1.4  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
