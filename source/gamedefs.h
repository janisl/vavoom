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
#include <ctype.h>
#include <math.h>

#include "build.h"		//	Build settings
#include "common.h"		//	Common types
#include "misc.h"		//	Misc utilites
#include "zone.h"		//	Zone memory allocation
#include "archive.h"
#include "array.h"		//	Dynamic arrays
#include "vector.h"
#include "maths.h"
#include "sizebuf.h"
#include "message.h"
#include "cvar.h"
#include "crc.h"
#include "infostr.h"
#include "debug.h"		//	Debug file
#include "system.h"		//	System specific routines
#include "files.h"
#include "wad.h"		//	WAD file I/O routines
#include "scripts.h"	//	Script parsing
#include "names.h"
#include "name.h"
#include "progs.h"		//	Progs
#include "vobject.h"
#include "vclass.h"
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

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.25  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//
//	Revision 1.24  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.23  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.22  2002/07/15 17:51:09  dj_jl
//	Made VSubsystem global.
//	
//	Revision 1.21  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.20  2002/04/11 16:39:21  dj_jl
//	Seperated build setings.
//	
//	Revision 1.19  2002/03/28 18:06:11  dj_jl
//	Updated version.
//	
//	Revision 1.18  2002/03/09 18:05:33  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.17  2002/01/28 18:45:16  dj_jl
//	Changed version
//	
//	Revision 1.16  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.15  2002/01/03 18:38:25  dj_jl
//	Added guard macros and core dumps
//	
//	Revision 1.14  2001/12/27 17:41:09  dj_jl
//	Switched to release
//	
//	Revision 1.13  2001/12/18 19:07:46  dj_jl
//	Added global name subsystem
//	
//	Revision 1.12  2001/12/12 19:26:40  dj_jl
//	Added dynamic arrays
//	
//	Revision 1.11  2001/12/01 17:39:26  dj_jl
//	Back to work
//	
//	Revision 1.10  2001/11/09 14:36:57  dj_jl
//	Switched to release
//	
//	Revision 1.9  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.8  2001/10/04 17:25:42  dj_jl
//	Added support for compiling with disabled asm
//	
//	Revision 1.7  2001/09/12 17:33:01  dj_jl
//	Back to development mode
//	
//	Revision 1.6  2001/08/31 17:29:45  dj_jl
//	Changes for release
//	
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
