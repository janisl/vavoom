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
//**	
//**	Main game header file.
//**	
//**************************************************************************

#ifndef _GAMEDEFS_H
#define _GAMEDEFS_H

//	C headers
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cmath>

#include "build.h"		//	Build settings
#include "common.h"		//	Common types
#include "misc.h"		//	Misc utilites
#include "args.h"		//	Command line arguments.
#include "zone.h"		//	Zone memory allocation
#include "array.h"		//	Dynamic arrays
#include "stream.h"		//	Streams
#include "names.h"
#include "name.h"
#include "str.h"		//	Strings
#include "vector.h"
#include "maths.h"
#include "message.h"
#include "cvar.h"
#include "crc.h"
#include "infostr.h"
#include "debug.h"		//	Debug file
#include "system.h"		//	System specific routines
#include "files.h"
#include "wad.h"		//	WAD file I/O routines
#include "scripts.h"	//	Script parsing
#include "progs.h"		//	Progs
#include "vclass.h"
#include "vobject.h"
#include "input.h"		//	Input from keyboard, mouse and joystick
#include "network.h"
#include "waddefs.h"    // 	External data types
#include "video.h"		//	Graphics
#include "screen.h"
#include "automap.h"
#include "level.h"		//	Level data
#include "mapinfo.h"
#include "host.h"
#include "render.h"
#include "text.h"
#include "sound.h"
#include "menu.h"
#include "console.h"
#include "cmd.h"
#include "sbar.h"
#include "chat.h"
#include "finale.h"
#include "save.h"
#include "client.h"
#include "server.h"
#include "protocol.h"
#include "imission.h"
#include "p_thinker.h"
#include "p_levelinfo.h"
#include "p_entity.h"
#include "player.h"
#include "p_gameinfo.h"

#endif
