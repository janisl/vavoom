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
//**	Header file registering global hardcoded Vavoom names.
//**
//**************************************************************************

// Macros ------------------------------------------------------------------

// Define a message as an enumeration.
#ifndef REGISTER_NAME
	#define REGISTER_NAME(name)	NAME_##name,
	#define REG_NAME_HIGH(name)	NAME_##name,
	#define REGISTERING_ENUM
	enum EName {
#endif

// Hardcoded names which are not messages ----------------------------------

// Special zero value, meaning no name.
REG_NAME_HIGH(None)

// Closing -----------------------------------------------------------------

#ifdef REGISTERING_ENUM
	};
	#undef REGISTER_NAME
	#undef REG_NAME_HIGH
	#undef REGISTERING_ENUM
#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//
//**************************************************************************
