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
// models.c -- model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

#define PU_MODEL		10

// MACROS ------------------------------------------------------------------

#define	MAX_MOD_KNOWN	256

// TYPES -------------------------------------------------------------------

struct pcx_t
{
    char			manufacturer;
    char			version;
    char			encoding;
    char			bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char			reserved;
    char			color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char			filler[58];
    unsigned char	data;		// unbounded
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static model_t *Mod_LoadModel(model_t *mod);
static void Mod_LoadAliasModel(model_t *mod, void *buffer);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int				SkinWidth;
int				SkinHeight;
byte			*SkinData;
rgb_t			SkinPal[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static model_t	mod_known[MAX_MOD_KNOWN];
static int		mod_numknown;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Mod_Extradata
//
//	Caches the data if needed
//
//==========================================================================

void *Mod_Extradata(model_t *mod)
{
	void	*r;
	
	r = mod->data;
	if (r)
		return r;

	Mod_LoadModel(mod);
	
	if (!mod->data)
		Sys_Error("Mod_Extradata: caching failed");
	return mod->data;
}

//==========================================================================
//
//	Mod_FindName
//
//==========================================================================

model_t *Mod_FindName(char *name)
{
	int		i;
	model_t	*mod;

	if (!name[0])
		Sys_Error("Mod_ForName: NULL name");
		
//
// search the currently loaded models
//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!strcmp(mod->name, name))
			break;
	}
			
	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
		{
			Sys_Error ("mod_numknown == MAX_MOD_KNOWN");
		}
		else
			mod_numknown++;
		strcpy(mod->name, name);
	}

	return mod;
}

//==========================================================================
//
//	Mod_LoadModel
//
//	Loads a model into the cache
//
//==========================================================================

static model_t *Mod_LoadModel(model_t *mod)
{
	unsigned	*buf;
	char		filename[256];

	if (mod->data)
	{
		return mod;
	}

//
// load the file
//
	if (!FL_FindFile(mod->name, filename))
		Sys_Error("Couldn't find model %s", mod->name);
	M_ReadFile(filename, (byte**)&buf);
	
//
// fill it in
//

// call the apropriate loader
	switch (LittleLong(*(unsigned *)buf))
	{
	 case IDPOLY2HEADER:
		Mod_LoadAliasModel(mod, buf);
		break;
		
	 default:
		Sys_Error("Unknown model format");
		break;
	}
	Z_Free(buf);

	return mod;
}

//==========================================================================
//
//	Mod_ForName
//
//	Loads in a model for the given name
//
//==========================================================================

model_t *Mod_ForName(char *name)
{
	model_t	*mod;

	mod = Mod_FindName(name);

	return Mod_LoadModel(mod);
}

//==========================================================================
//
//	Mod_LoadAliasModel
//
//==========================================================================

static void Mod_LoadAliasModel(model_t *mod, void *buffer)
{
	int					version;
	int					size;
	int					i;
	mmdl_t				*pinmodel;
	mmdl_t				*pmodel;
	mskin_t				*pinskin;
	mskin_t				*pskin;
	mstvert_t			*pinstverts;
	mstvert_t			*pstverts;
	mtriangle_t			*pintriangles;
	mtriangle_t			*ptri;
	mframe_t			*pinframe;
	mframe_t			*pframe;
	int					*pincmds;
	int					*pcmds;

	pinmodel = (mmdl_t *)buffer;

	version = LittleLong(pinmodel->version);
	if (version != ALIAS_VERSION)
		Sys_Error("%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);

	size = 	LittleLong(pinmodel->ofsend) +
			LittleLong(pinmodel->numskins) * LittleLong(pinmodel->skinheight) * LittleLong(pinmodel->skinwidth);
	pmodel = (mmdl_t *)Z_Malloc(size, PU_MODEL, &mod->data);

	//
	// endian-adjust and copy the data, starting with the alias model header
	//
	for (i = 0; i < (int)sizeof(mmdl_t) / 4; i++)
	{
		((int*)pmodel)[i] = LittleLong(((int*)pinmodel)[i]);
	}

	if (pmodel->skinheight > MAX_SKIN_HEIGHT)
		Sys_Error("model %s has a skin taller than %d", mod->name,
				   MAX_SKIN_HEIGHT);

	if (pmodel->numverts <= 0)
		Sys_Error("model %s has no vertices", mod->name);

	if (pmodel->numverts > MAXALIASVERTS)
		Sys_Error("model %s has too many vertices", mod->name);

	if (pmodel->numstverts <= 0)
		Sys_Error("model %s has no texture vertices", mod->name);

	if (pmodel->numstverts > MAXALIASSTVERTS)
		Sys_Error("model %s has too many texture vertices", mod->name);

	if (pmodel->numtris <= 0)
		Sys_Error("model %s has no triangles", mod->name);

	if (pmodel->skinwidth & 0x03)
		Sys_Error("Mod_LoadAliasModel: skinwidth not multiple of 4");

	if (pmodel->numskins < 1)
		Sys_Error("Mod_LoadAliasModel: Invalid # of skins: %d\n", pmodel->numskins);

	//
	// load the skins
	//
	pskin = (mskin_t*)((byte*)pmodel + pmodel->ofsskins);
	pinskin = (mskin_t*)((byte*)pinmodel + pmodel->ofsskins);

	for (i = 0; i < pmodel->numskins; i++)
	{
		strcpy(pskin[i].name, pinskin[i].name);
	}

	//
	// set base s and t vertices
	//
	pstverts = (mstvert_t*)((byte*)pmodel + pmodel->ofsstverts);
	pinstverts = (mstvert_t*)((byte*)pinmodel + pmodel->ofsstverts);

	for (i = 0; i < pmodel->numstverts; i++)
	{
		pstverts[i].s = LittleShort(pinstverts[i].s);
		pstverts[i].t = LittleShort(pinstverts[i].t);
	}

	//
	// set up the triangles
	//
	ptri = (mtriangle_t *)((byte*)pmodel + pmodel->ofstris);
	pintriangles = (mtriangle_t *)((byte*)pinmodel + pmodel->ofstris);

	for (i = 0; i < pmodel->numtris; i++)
	{
		int		j;

		for (j = 0; j < 3; j++)
		{
			ptri[i].vertindex[j] = LittleShort(pintriangles[i].vertindex[j]);
			ptri[i].stvertindex[j] = LittleShort(pintriangles[i].stvertindex[j]);
		}
	}

	//
	// load the frames
	//
	if (pmodel->numframes < 1)
		Sys_Error("Mod_LoadAliasModel: Invalid # of frames: %d\n", pmodel->numframes);

	pframe = (mframe_t *)((byte*)pmodel + pmodel->ofsframes);
	pinframe = (mframe_t *)((byte*)pinmodel + pmodel->ofsframes);

	for (i = 0; i < pmodel->numframes; i++)
	{
		trivertx_t		*pverts, *pinverts;
		int				j;

		pframe->scale[0] = LittleFloat(pinframe->scale[0]);
		pframe->scale[1] = LittleFloat(pinframe->scale[1]);
		pframe->scale[2] = LittleFloat(pinframe->scale[2]);
		pframe->scale_origin[0] = LittleFloat(pinframe->scale_origin[0]);
		pframe->scale_origin[1] = LittleFloat(pinframe->scale_origin[1]);
		pframe->scale_origin[2] = LittleFloat(pinframe->scale_origin[2]);
		strcpy(pframe->name, pinframe->name);

		pinverts = (trivertx_t*)(pinframe + 1);
		pverts = (trivertx_t*)(pframe +1);

		for (j = 0; j < pmodel->numverts; j++)
		{
			// these are all byte values, so no need to deal with endianness
			pverts[j].v[0] = pinverts[j].v[0];
			pverts[j].v[1] = pinverts[j].v[1];
			pverts[j].v[2] = pinverts[j].v[2];
			pverts[j].lightnormalindex = pinverts[j].lightnormalindex;
		}

		pframe = (mframe_t*)((byte*)pframe + pmodel->framesize);
		pinframe = (mframe_t*)((byte*)pinframe + pmodel->framesize);
	}

	//
	// load commands
	//
	pcmds = (int*)((byte*)pmodel + pmodel->ofscmds);
	pincmds = (int*)((byte*)pinmodel + pmodel->ofscmds);

	for (i = 0; i < pmodel->numcmds; i++)
	{
		pcmds[i] = LittleLong(pincmds[i]);
	}
}

//==========================================================================
//
//	LoadPCX
//
//==========================================================================

static void LoadPCX(const char *filename, void **bufptr)
{
	int			c;
	int			bytes_per_line;
	int			x, y;
	char		ch;
	pcx_t		*pcx;
	byte		*data;

	M_ReadFile(filename, (byte**)&pcx);

	if (pcx->bits_per_pixel != 8)
	{
		// we like 8 bit color planes
		Sys_Error("No 8-bit planes\n");
	}
	if (pcx->color_planes != 1)
	{
		Sys_Error("Not 8 bpp\n");
	}

	SkinWidth = LittleShort(pcx->xmax) - LittleShort(pcx->xmin) + 1;
	SkinHeight = LittleShort(pcx->ymax) - LittleShort(pcx->ymin) + 1;

	bytes_per_line = pcx->bytes_per_line;

	SkinData = (byte*)Z_Malloc(SkinWidth * SkinHeight, PU_STATIC, bufptr);

	data = &pcx->data;

	for (y = 0; y < SkinHeight; y++)
	{
		// decompress RLE encoded PCX data
		x = 0;

		while (x < bytes_per_line)
		{
			ch = *data++;
			if ((ch & 0xC0) == 0xC0)
			{
				c = (ch & 0x3F);
				ch = *data++;
			}
			else
			{
				c = 1;
			}

			while (c--)
			{
				if (x < SkinWidth)
					SkinData[y * SkinWidth + x] = ch;
				x++;
			}
		}
	}

	if (*data == 12)
	{
		data++;
		for (c = 0; c < 256; c++)
		{
			SkinPal[c].r = *data++;
			SkinPal[c].g = *data++;
			SkinPal[c].b = *data++;
		}
	}

	Z_Free(pcx);
}

//==========================================================================
//
//	Mod_LoadSkin
//
//==========================================================================

void Mod_LoadSkin(const char *name, void **bufptr)
{
	char		filename[256];

	if (!FL_FindFile(name, filename))
		Sys_Error("Couldn't find skin %s", name);
	LoadPCX(filename, bufptr);
}

