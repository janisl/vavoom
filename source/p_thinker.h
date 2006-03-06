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

//
//	VThinker
//
//	Doubly linked list of actors and other special elements of a level.
//
class VThinker : public VObject
{
	DECLARE_CLASS(VThinker, VObject, 0)

	VLevel*			XLevel;		//	Level object.
	VLevelInfo*		Level;		//	Level info object.

	VThinker*		Prev;
	VThinker*		Next;

	static int		FIndex_Tick;

	VThinker();

	void Serialise(VStream&);

	//	VThinker interface.
	virtual void Tick(float DeltaTime);
};

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2006/03/06 13:12:12  dj_jl
//	Client now uses entity class.
//
//**************************************************************************
