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

enum Game_t
{
 	Doom,
    Doom2,
    Heretic,
    Hexen,
	Strife	//	Just a test, not a true support
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void Host_Init(void);
void Host_Shutdown(void);
void Host_Frame(void);
void Host_EndGame(const char *message, ...)
	__attribute__((format(printf, 1, 2))) __attribute__((noreturn));
void Host_Error(const char *error, ...)
	__attribute__((format(printf, 1, 2))) __attribute__((noreturn));

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TCvarI		developer;

extern Game_t		Game;

extern boolean		host_initialized;

extern int			host_frametics;
extern double		host_frametime;
extern double		host_time;
extern double		realtime;
extern int			host_framecount;

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/08/21 17:41:33  dj_jl
//	Removed -devmaps option
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
