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

#ifndef GLVIS_H
#define GLVIS_H

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

// MACROS ------------------------------------------------------------------

#if !defined __GNUC__ && !defined __attribute__
#define __attribute__(whatever)
#endif

// TYPES -------------------------------------------------------------------

class GLVisError
{
 public:
	GLVisError(const char *, ...) __attribute__((format(printf, 2, 3)));

	char message[256];
};

class TGLVis
{
 public:
	TGLVis(void) : Malloc(NULL), Free(NULL), fastvis(false), verbose(false),
		testlevel(2), num_specified_maps(0)
	{
	}

	void Build(const char *srcfile);
	virtual void DisplayMessage(const char *text, ...)
		__attribute__((format(printf, 2, 3))) = 0;
	virtual void DisplayStartMap(const char *levelname) = 0;
	virtual void DisplayBaseVisProgress(int count, int total) = 0;
	virtual void DisplayPortalVisProgress(int count, int total) = 0;
	virtual void DisplayMapDone(int accepts, int total) = 0;

	void *(*Malloc)(size_t);
	void (*Free)(void *);

	bool fastvis;
	bool verbose;
	bool no_reject;

	int testlevel;

	int num_specified_maps;
	char specified_maps[100][16];
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//
//	Revision 1.6  2001/10/18 17:41:47  dj_jl
//	Added reject building
//	
//	Revision 1.5  2001/09/20 16:38:05  dj_jl
//	Moved TGLVis out of namespace
//	
//	Revision 1.4  2001/09/12 17:28:38  dj_jl
//	Created glVIS plugin
//	
//	Revision 1.3  2001/08/24 17:08:34  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
