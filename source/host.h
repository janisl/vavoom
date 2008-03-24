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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void Host_Init(void);
void Host_Shutdown(void);
void Host_Frame(void);
void __attribute__((noreturn, format(printf, 1, 2))) __declspec(noreturn)
	Host_EndGame(const char *message, ...);
void __attribute__((noreturn, format(printf, 1, 2))) __declspec(noreturn)
	Host_Error(const char *error, ...);
const char *Host_GetCoreDump(void);
bool Host_StartTitleMap();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern VCvarI		developer;

extern bool			host_initialised;
extern bool			host_request_exit;

extern int			host_frametics;
extern double		host_frametime;
extern double		host_time;
extern double		realtime;
extern int			host_framecount;
extern bool			host_standalone;
extern bool			host_titlemap;

extern vuint32		host_cycles[16];
