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

enum
{
	GAME_Doom				= 0x01,
	GAME_Heretic			= 0x02,
	GAME_Hexen				= 0x04,
	GAME_Strife				= 0x08,
	GAME_Raven				= GAME_Heretic | GAME_Hexen,
	GAME_Any				= 0x0f,
};

enum { MAX_DECORATE_TRANSLATIONS = 0xffff };

struct VLineSpecInfo
{
	VStr		Name;
	int			Number;
};

void ReadLineSpecialInfos();
void ProcessDecorateScripts();

extern TArray<VLineSpecInfo>	LineSpecialInfos;
