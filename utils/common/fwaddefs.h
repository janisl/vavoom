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

//==========================================================================
//
//	WAD file types
//
//==========================================================================

#pragma pack(push, 1)

typedef struct
{
    // Should be "IWAD" or "PWAD".
	char		identification[4];
	int			numlumps;
	int			infotableofs;
    
} wadinfo_t;

typedef struct
{
	int			filepos;
	int			size;
	char		name[8];
    
} filelump_t;

#pragma pack(pop)

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2004/10/11 06:49:04  dj_jl
//	Added support for version 3.0 GL nodes.
//
//	Revision 1.4  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/09/24 17:28:45  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
