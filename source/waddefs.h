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
//**
//** 	All external data is defined here, most of the data is loaded into
//**  different structures at run time.
//**
//**************************************************************************

#ifndef _WAD_DEFS_H
#define _WAD_DEFS_H

//==========================================================================
//
//	Flags
//
//==========================================================================

//  If a texture is pegged, the texture will have the end exposed to air held
// constant at the top or bottom of the texture (stairs or pulled down
// things) and will move with a height change of one of the neighbor sectors.
// Unpegged textures allways have the first row of the texture at the top
// pixel of the line for both top and bottom textures (use next to windows).

//
// LineDef attributes.
//
#define	ML_BLOCKING			0x0001  // Solid, is an obstacle.
#define	ML_BLOCKMONSTERS	0x0002  // Blocks monsters only.
#define	ML_TWOSIDED			0x0004	// Backside will not be present at all
#define	ML_DONTPEGTOP		0x0008  // upper texture unpegged
#define	ML_DONTPEGBOTTOM	0x0010  // lower texture unpegged
#define ML_SECRET			0x0020	// don't map as two sided: IT'S A SECRET!
#define ML_SOUNDBLOCK		0x0040	// don't let sound cross two of these
#define	ML_DONTDRAW			0x0080	// don't draw on the automap
#define	ML_MAPPED			0x0100	// set if already drawn in automap
#define ML_REPEAT_SPECIAL	0x0200	// special is repeatable
#define ML_SPAC_SHIFT		10
#define ML_SPAC_MASK		0x1c00
#define GET_SPAC(_flags_)	(((_flags_) & ML_SPAC_MASK) >> ML_SPAC_SHIFT)

//
// Special activation types
//
#define SPAC_CROSS		0	// when player crosses line
#define SPAC_USE		1	// when player uses line
#define SPAC_MCROSS		2	// when monster crosses line
#define SPAC_IMPACT		3	// when projectile hits line
#define SPAC_PUSH		4	// when player/monster pushes line
#define SPAC_PCROSS		5	// when projectile crosses line

//
// Indicate a leaf.
//
#define	NF_SUBSECTOR	0x8000

//
// Map things flags
//
#define	MTF_EASY		0x0001  // Skill flags.
#define	MTF_NORMAL		0x0002
#define	MTF_HARD		0x0004
#define	MTF_AMBUSH		0x0008  // Deaf monsters/do not react to sound.
#define MTF_DORMANT		0x0010  // The thing is dormant
#define MTF_FIGHTER		0x0020  // Thing appearing in player classes
#define MTF_CLERIC		0x0040
#define MTF_MAGE		0x0080
#define MTF_GSINGLE		0x0100  // Appearing in game modes
#define MTF_GCOOP		0x0200
#define MTF_GDEATHMATCH	0x0400

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
