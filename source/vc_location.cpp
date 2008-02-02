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

#ifdef IN_VCC
#include "../utils/vcc/vcc.h"
#else
#include "gamedefs.h"
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

TArray<VStr>		TLocation::SourceFiles;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TLocation::AddSourceFile
//
//==========================================================================

int TLocation::AddSourceFile(const VStr& SName)
{
	//	Find it.
	for (int i = 0; i < SourceFiles.Num(); i++)
		if (SName == SourceFiles[i])
			return i;

	//	Not found, add it.
	return SourceFiles.Append(SName);
}

//==========================================================================
//
//	TLocation::GetSource
//
//==========================================================================

VStr TLocation::GetSource() const
{
	if (!Loc)
		return "(external)";
	return SourceFiles[Loc >> 16];
}

//==========================================================================
//
//	TLocation::ClearSourceFiles
//
//==========================================================================

void TLocation::ClearSourceFiles()
{
	SourceFiles.Clear();
}
