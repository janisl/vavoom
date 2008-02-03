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

#ifndef __vc_local_h__
#define __vc_local_h__

#include "gamedefs.h"
#include "progdefs.h"

class VLabel
{
private:
	friend class VEmitContext;

	int			Index;

	VLabel(int AIndex)
	: Index(AIndex)
	{}

public:
	VLabel()
	: Index(-1)
	{}
	bool IsDefined() const
	{
		return Index != -1;
	}
};

class VLocalVarDef
{
public:
	VName			Name;
	TLocation		Loc;
	int				Offset;
	VFieldType		Type;
	bool			Visible;
	vuint8			ParamFlags;

	VLocalVarDef()
	{}
};

#define FatalError	Sys_Error

#include "vc_emit_context.h"

#endif
