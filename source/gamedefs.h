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
//**	
//**	Main game header file.
//**	
//**************************************************************************

#ifndef _GAMEDEFS_H
#define _GAMEDEFS_H

//	C headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <ctype.h>
#include <math.h>

//	C++ headers
#include <iostream.h>
#include <iomanip.h>
#include <fstream.h>

#if !defined CLIENT && !defined SERVER
#define CLIENT
#define SERVER
#endif

#if defined __i386__ || defined _M_IX86
#define USEASM
#endif

// if rangecheck is undefined, most parameter validation debugging code
// will not be compiled
#define RANGECHECK
//#define PARANOID
#define DEVELOPER

#define VERSION			1.9
#define VERSION_MAJOR	1
#define VERSION_MINOR	9
#define VERSION_RELEASE	0
#define VERSION_INT		109
#define VERSION_TEXT	"v1.9"

//==========================================================================
//
//	SUBSYSTEMS
//
//==========================================================================

#include "common.h"		//	Common types, endianess handling
#include "misc.h"		//	Misc utilites
#include "vector.h"
#include "maths.h"
#include "sizebuf.h"
#include "message.h"
#include "cvar.h"
#include "crc.h"
#include "infostr.h"
#include "debug.h"		//	Debug file
#include "system.h"		//	System specific routines
#include "zone.h"		//	Zone memory allocation
#include "files.h"
#include "wad.h"		//	WAD file I/O routines
#include "scripts.h"	//	Script parsing
#include "progs.h"		//	Progs
#include "input.h"		//	Input from keyboard, mouse and joystick
#include "network.h"
#include "waddefs.h"    // 	External data types
#include "video.h"		//	Graphics
#include "screen.h"
#include "automap.h"
#include "level.h"		//	Level data
#include "player.h"
#include "host.h"
#include "render.h"
#include "text.h"
#include "sound.h"
#include "menu.h"
#include "play.h"
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

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/08/30 17:39:51  dj_jl
//	Moved view border and message box to progs
//
//	Revision 1.4  2001/08/04 17:25:14  dj_jl
//	Moved title / demo loop to progs
//	Removed shareware / ExtendedWAD from engine
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
