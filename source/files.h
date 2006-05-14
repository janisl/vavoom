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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void FL_Init();
void FL_Shutdown();

VStr FL_FindFile(const VStr& fname);
int FL_ReadFile(const VStr& name, void** buffer);
bool FL_WriteFile(const VStr& name, const void* source, int length);
void FL_CreatePath(const VStr& Path);

VStream* FL_OpenFileRead(const VStr& Name);
VStream* FL_OpenFileWrite(const VStr& Name);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern bool		fl_devmode;
extern VStr		fl_basedir;
extern VStr		fl_savedir;
extern VStr		fl_gamedir;
