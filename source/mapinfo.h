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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct mapInfo_t
{
	char	lumpname[12];
	int		cluster;    // Defines what cluster level belongs to
	int		warpTrans;  // Actual map number in case maps are not sequential
	char	nextMap[12];// Map to teleport to upon exit of timed deathmatch
	int		cdTrack;    // CD track to play during level
	char	name[32];   // Name of map
	int		sky1Texture;// Default sky texture
	int		sky2Texture;// Alternate sky displayed in Sky2 sectors
	float	sky1ScrollDelta;// Default sky texture speed
	float	sky2ScrollDelta;// Alternate sky texture speed
	boolean	doubleSky;  // parallax sky: sky2 behind sky1
	boolean	lightning;  // Use of lightning on the level flashes from sky1 to sky2
	char	fadetable[12];// Fade table {fogmap}
	char	songLump[12];// Background music (MUS or MIDI)
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void InitMapInfo(void);
void P_GetMapInfo(const char *map, mapInfo_t &info);
void P_PutMapSongLump(int map, const char *lumpName);
int P_GetCDStartTrack(void);
int P_GetCDEnd1Track(void);
int P_GetCDEnd2Track(void);
int P_GetCDEnd3Track(void);
int P_GetCDIntermissionTrack(void);
int P_GetCDTitleTrack(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2001/10/08 17:30:23  dj_jl
//	Renamed to mapinfo.*
//
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
