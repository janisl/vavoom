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
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

//==========================================================================
//
//  Graphics
//
//==========================================================================

// posts are runs of non masked source pixels
struct post_t
{
	byte		topdelta;       // -1 is the last post in a column
	byte		length;         // length data bytes follows
};

// column_t is a list of 0 or more post_t, (byte)-1 terminated
typedef post_t column_t;

//  Patches.
//  A patch holds one or more columns.
//  Patches are used for sprites and all masked pictures, and we compose
// textures from the TEXTURE1/2 lists of patches.
struct patch_t
{
	short		width;          // bounding box size
	short		height;
	short		leftoffset;     // pixels to the left of origin
	short		topoffset;      // pixels below the origin
	int			columnofs[8];   // only [width] used
	// the [0] is &columnofs[width]
};

struct vpic_t
{
	short	width;
	short	height;
	byte	data[4];
};
