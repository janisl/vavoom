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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:56  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

#ifndef _INFO_H
#define _INFO_H

struct state_t
{
	int		sprite;
	int		frame;
	int		model_index;
	int		model_frame;
	float	time;
	int		statenum;
	int		nextstate;
	float	misc1;
    float	misc2;
	int		function;
};

struct mobjinfo_t
{
	int			spawnfunc;
    int			doomednum;
};

#ifndef __VCC_H__
extern mobjinfo_t	*mobjinfo;
#endif
#endif
