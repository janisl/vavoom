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

class TModifiers
{
public:
	enum
	{
		Native			= 0x0001,
		Static			= 0x0002,
		Abstract		= 0x0004,
		Private			= 0x0008,
		ReadOnly		= 0x0010,
		Transient		= 0x0020,
		Final			= 0x0040,
		Optional		= 0x0080,
		Out				= 0x0100,
		Spawner			= 0x0200,
	};

	static int Parse(VLexer&);
	static const char* Name(int);
	static int Check(int, int, TLocation);
	static int MethodAttr(int);
	static int ClassAttr(int);
	static int FieldAttr(int);
	static int PropAttr(int);
	static int ParmAttr(int);
};
