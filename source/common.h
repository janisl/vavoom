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

typedef unsigned short	 	word;

//==========================================================================
//
//	Timing
//
//==========================================================================

#define clock(var)		var -= Sys_Cycles()
#define unclock(var)	var += Sys_Cycles() - 34

//==========================================================================
//
//	Basic templates
//
//==========================================================================

template<class T> T Min(T val1, T val2)
{
	return val1 < val2 ? val1 : val2;
}

template<class T> T Max(T val1, T val2)
{
	return val1 > val2 ? val1 : val2;
}

template<class T> T Clamp(T val, T low, T high)
{
	return val < low ? low : val > high ? high : val;
}

//==========================================================================
//
//	Forward declarations
//
//==========================================================================

class	VName;
class	VStr;
class	VStream;

class	VMemberBase;
class	VPackage;
class	VField;
class	VMethod;
class	VState;
class	VConstant;
class	VStruct;
class	VClass;
class	VNetObjectsMap;
struct	mobjinfo_t;

class	VObject;
