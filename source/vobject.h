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

#define DECLARE_BASE_CLASS(TThisType, TSuper, TStaticFlags)

#define DECLARE_CLASS(TThisType, TSuper, TStaticFlags)

// ENUMERATIONS ------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VClass
{
public:
	virtual ~VClass(void) { }
};

class VObject:public VClass
{
	DECLARE_BASE_CLASS(VClass, VClass, 0)
public:
	int *vtable;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2001/12/01 17:43:13  dj_jl
//	Renamed ClassBase to VObject
//
//**************************************************************************
