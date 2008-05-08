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
#include "args.h"		//	Command line arguments.
#include "zone.h"		//	Zone memory allocation
#include "array.h"		//	Dynamic arrays
#include "map.h"		//	Mapping of keys to values
#include "stream.h"		//	Streams
#include "bitstream.h"	//	Streams for bit-data
#include "names.h"		//	Built-in names
#include "name.h"		//	Names
#include "str.h"		//	Strings
#include "language.h"	//	Localisation
#include "misc.h"		//	Misc utilites
#include "vector.h"		//	Vector math
#include "maths.h"		//	Misc math methods
#include "cvar.h"		//	Console variables
#include "crc.h"		//	CRC calcuation
#include "infostr.h"	//	Info strings
#include "debug.h"		//	Debug file
#include "system.h"		//	System specific routines
#include "files.h"		//	File I/O routines
#include "wad.h"		//	WAD file I/O routines
#include "xml.h"		//	XML file parsing
#include "progs.h"		//	Progs
#include "vc_location.h"
#include "vc_type.h"
#include "vc_member.h"
#include "vc_field.h"
#include "vc_property.h"
#include "vc_method.h"
#include "vc_constant.h"
#include "vc_struct.h"
#include "vc_state.h"
#include "vc_class.h"
#include "vc_package.h"
#include "vc_object.h"
#include "vc_decorate.h"//	Decorate scripts
#include "vc_dehacked.h"//	DeHackEd support
#include "scripts.h"	//	Script parsing
#include "input.h"		//	Input from keyboard, mouse and joystick
#include "video.h"		//	Graphics
#include "screen.h"
#include "automap.h"
#include "level.h"		//	Level data
#include "mapinfo.h"
#include "lockdefs.h"
#include "host.h"
#include "r_public.h"
#include "text.h"
#include "sound.h"
#include "menu.h"
#include "console.h"
#include "cmd.h"
#include "sbar.h"
#include "chat.h"
#include "finale.h"
#include "save.h"
#include "server.h"
#include "imission.h"
#include "p_clip.h"
#include "p_worldinfo.h"
#include "p_thinker.h"
#include "p_levelinfo.h"
#include "p_entity.h"
#include "p_playerreplicationinfo.h"
#include "player.h"
#include "p_gameinfo.h"
#include "p_world.h"
#include "client.h"

#endif
