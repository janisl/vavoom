//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
//**	ALIAS MODELS
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// little-endian "IDP2"
#define IDPOLY2HEADER		(('2'<<24)+('P'<<16)+('D'<<8)+'I')
#define ALIAS_VERSION		8

#define MAX_SKIN_HEIGHT		480

#define MAXALIASVERTS		2000	// TODO: tune this
#define MAXALIASSTVERTS		2000	// TODO: tune this

// TYPES -------------------------------------------------------------------

struct mmdl_t
{
	int			ident;
	int			version;
	int			skinwidth;
	int			skinheight;
	int			framesize;
	int			numskins;
	int			numverts;
	int			numstverts;
	int			numtris;
	int			numcmds;
	int			numframes;
	int			ofsskins;
	int			ofsstverts;
	int			ofstris;
	int			ofsframes;
	int			ofscmds;
	int			ofsend;
};

struct mskin_t
{
	char		name[64];
};

struct mstvert_t
{
	short		s;
	short		t;
};

struct mtriangle_t
{
	short		vertindex[3];
	short		stvertindex[3];
};

struct mframe_t
{
	TVec		scale;
	TVec		scale_origin;
	char		name[16];
};

struct trivertx_t
{
	byte	v[3];
	byte	lightnormalindex;
};

//
// Whole model
//

struct model_t
{
	char		name[128];
	void		*data;		// only access through Mod_Extradata
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

model_t *Mod_ForName(char *name);
model_t *Mod_FindName(char *name);
void *Mod_Extradata(model_t *mod);	// handles caching
void Mod_LoadSkin(const char *name, void **bufptr);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

