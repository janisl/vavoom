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
// models.c -- model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define	MAX_MOD_KNOWN	256

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static VModel* Mod_LoadModel(VModel* mod);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VModel	mod_known[MAX_MOD_KNOWN];
static int		mod_numknown;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Mod_Extradata
//
//	Caches the data if needed
//
//==========================================================================

void *Mod_Extradata(VModel* mod)
{
	guard(Mod_Extradata);
	void	*r;
	
	r = mod->data;
	if (r)
		return r;

	Mod_LoadModel(mod);
	
	if (!mod->data)
		Sys_Error("Mod_Extradata: caching failed");
	return mod->data;
	unguard;
}

//==========================================================================
//
//	Mod_FindName
//
//==========================================================================

VModel* Mod_FindName(const char *name)
{
	guard(Mod_FindName);
	int		i;
	VModel*	mod;

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
	unguard;
}

//==========================================================================
//
//	Mod_SwapAliasModel
//
//==========================================================================

static void Mod_SwapAliasModel(VModel* mod)
{
	guard(Mod_SwapAliasModel);
	int					i, j;
	mmdl_t				*pmodel;
	mstvert_t			*pstverts;
	mtriangle_t			*ptri;
	mframe_t			*pframe;
	int					*pcmds;

	pmodel = (mmdl_t*)mod->data;

	//
	// endian-adjust and swap the data, starting with the alias model header
	//
	for (i = 0; i < (int)sizeof(mmdl_t) / 4; i++)
	{
		((int*)pmodel)[i] = LittleLong(((int*)pmodel)[i]);
	}

	if (pmodel->version != ALIAS_VERSION)
		Sys_Error("%s has wrong version number (%i should be %i)",
			mod->name, pmodel->version, ALIAS_VERSION);

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

	if (pmodel->numframes < 1)
		Sys_Error("Mod_LoadAliasModel: Invalid # of frames: %d\n", pmodel->numframes);

	//
	// base s and t vertices
	//
	pstverts = (mstvert_t*)((byte*)pmodel + pmodel->ofsstverts);
	for (i = 0; i < pmodel->numstverts; i++)
	{
		pstverts[i].s = LittleShort(pstverts[i].s);
		pstverts[i].t = LittleShort(pstverts[i].t);
	}

	//
	// triangles
	//
	ptri = (mtriangle_t *)((byte*)pmodel + pmodel->ofstris);
	for (i = 0; i < pmodel->numtris; i++)
	{
		for (j = 0; j < 3; j++)
		{
			ptri[i].vertindex[j] = LittleShort(ptri[i].vertindex[j]);
			ptri[i].stvertindex[j] = LittleShort(ptri[i].stvertindex[j]);
		}
	}

	//
	// frames
	//
	pframe = (mframe_t *)((byte*)pmodel + pmodel->ofsframes);
	for (i = 0; i < pmodel->numframes; i++)
	{
		pframe->scale[0] = LittleFloat(pframe->scale[0]);
		pframe->scale[1] = LittleFloat(pframe->scale[1]);
		pframe->scale[2] = LittleFloat(pframe->scale[2]);
		pframe->scale_origin[0] = LittleFloat(pframe->scale_origin[0]);
		pframe->scale_origin[1] = LittleFloat(pframe->scale_origin[1]);
		pframe->scale_origin[2] = LittleFloat(pframe->scale_origin[2]);
		pframe = (mframe_t*)((byte*)pframe + pmodel->framesize);
	}

	//
	// commands
	//
	pcmds = (int*)((byte*)pmodel + pmodel->ofscmds);
	for (i = 0; i < pmodel->numcmds; i++)
	{
		pcmds[i] = LittleLong(pcmds[i]);
	}
	unguard;
}

//==========================================================================
//
//	Mod_LoadModel
//
//	Loads a model into the cache
//
//==========================================================================

static VModel* Mod_LoadModel(VModel* mod)
{
	guard(Mod_LoadModel);
	if (mod->data)
	{
		return mod;
	}

	//
	// load the file
	//
	if (FL_ReadFile(mod->name, &mod->data, PU_CACHE) < 0)
		Sys_Error("Couldn't load %s", mod->name);
	
	if (LittleLong(*(unsigned *)mod->data) != IDPOLY2HEADER)
		Sys_Error("model %s is not a md2 model", mod->name);

	// swap model
	Mod_SwapAliasModel(mod);

	return mod;
	unguard;
}

//==========================================================================
//
//	R_PositionWeaponModel
//
//==========================================================================

void R_PositionWeaponModel(VEntity* wpent, VModel* wpmodel, int InFrame)
{
	guard(R_PositionWeaponModel);
	mmdl_t *pmdl = (mmdl_t*)Mod_Extradata(wpmodel);
	int frame = InFrame;
	if ((frame >= pmdl->numframes) || (frame < 0))
	{
		frame = 0;
	}
	mtriangle_t *ptris = (mtriangle_t*)((byte*)pmdl + pmdl->ofstris);
	mframe_t *pframe = (mframe_t*)((byte*)pmdl + pmdl->ofsframes +
		frame * pmdl->framesize);
	trivertx_t *pverts = (trivertx_t *)(pframe + 1);
	TVec p[3];
	for (int vi = 0; vi < 3; vi++)
	{
		p[vi].x = pverts[ptris[0].vertindex[vi]].v[0] * pframe->scale[0] + pframe->scale_origin[0];
		p[vi].y = pverts[ptris[0].vertindex[vi]].v[1] * pframe->scale[1] + pframe->scale_origin[1];
		p[vi].z = pverts[ptris[0].vertindex[vi]].v[2] * pframe->scale[2] + pframe->scale_origin[2];
	}
	TVec md_forward, md_left, md_up;
	AngleVectors(wpent->Angles, md_forward, md_left, md_up);
	md_left = -md_left;
	wpent->Origin += md_forward * p[0].x + md_left * p[0].y + md_up * p[0].z;
	TAVec wangles;
	VectorAngles(p[1] - p[0], wangles);
	wpent->Angles.yaw = AngleMod(wpent->Angles.yaw + wangles.yaw);
	wpent->Angles.pitch = AngleMod(wpent->Angles.pitch + wangles.pitch);
	unguard;
}
