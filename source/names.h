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

// Log messages.
REGISTER_NAME(Log)
REGISTER_NAME(Init)
REGISTER_NAME(Dev)
REGISTER_NAME(DevNet)

//	Native class names.
REGISTER_NAME(Object)
REGISTER_NAME(Level)
REGISTER_NAME(Subsystem)
REGISTER_NAME(SoftwareDrawer)
REGISTER_NAME(OpenGLDrawer)
REGISTER_NAME(Direct3DDrawer)
REGISTER_NAME(Drawer)
REGISTER_NAME(OpenALDevice)
REGISTER_NAME(SoundDevice)
REGISTER_NAME(DefaultSoundDevice)
REGISTER_NAME(GC)
REGISTER_NAME(ModalWindow)
REGISTER_NAME(RootWindow)
REGISTER_NAME(Window)
REGISTER_NAME(ACS)
REGISTER_NAME(Entity)
REGISTER_NAME(ViewEntity)
REGISTER_NAME(Thinker)

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
//	Revision 1.4  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//
//	Revision 1.3  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/18 18:57:11  dj_jl
//	Added global name subsystem
//	
//**************************************************************************
