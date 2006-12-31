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
// models.c -- model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	MODEL_Unknown,
	MODEL_MD2,
	MODEL_Script,
};

class VScriptSubModel
{
public:
	struct VFrame
	{
		int		Index;
	};

	VModel*				Model;
	TArray<VFrame>		Frames;
};

class VScriptModel
{
public:
	VName						Name;
	TArray<VScriptSubModel>		SubModels;
};

class VScriptedModelFrame
{
public:
	int			Number;
	float		Inter;
	int			ModelIndex;
	int			FrameIndex;
};

class VScriptedModel
{
public:
	TArray<VScriptModel>		Models;
	TArray<VScriptedModelFrame>	Frames;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static VModel* Mod_LoadModel(VModel* mod);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<VModel*>		mod_known;

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
	if (!name[0])
		Sys_Error("Mod_ForName: NULL name");

	//
	// search the currently loaded models
	//
	for (int i = 0; i < mod_known.Num(); i++)
	{
		if (!VStr::Cmp(mod_known[i]->name, name))
		{
			return mod_known[i];
		}
	}

	VModel* mod = new VModel();
	VStr::Cpy(mod->name, name);
	mod->data = NULL;
	mod->type = MODEL_Unknown;
	mod_known.Append(mod);

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
	vint32				*pcmds;

	pmodel = (mmdl_t*)mod->data;

	//
	// endian-adjust and swap the data, starting with the alias model header
	//
	for (i = 0; i < (int)sizeof(mmdl_t) / 4; i++)
	{
		((vint32*)pmodel)[i] = LittleLong(((vint32*)pmodel)[i]);
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
	pcmds = (vint32*)((byte*)pmodel + pmodel->ofscmds);
	for (i = 0; i < pmodel->numcmds; i++)
	{
		pcmds[i] = LittleLong(pcmds[i]);
	}
	unguard;
}

//==========================================================================
//
//	ParseModelScript
//
//==========================================================================

static void ParseModelScript(VModel* mod, VStream& Strm)
{
	guard(ParseModelScript);
	//	Free loaded XML text.
	Z_Free(mod->data);
	mod->data = NULL;

	//	Parse XML file.
	VXmlDocument* Doc = new VXmlDocument();
	Doc->Parse(Strm, mod->name);

	//	Verify that it's a model definition file.
	if (Doc->Root.Name != "vavoom_model_definition")
		Sys_Error("%s is not a valid model definition file", mod->name);

	VScriptedModel* Mdl = new VScriptedModel();
	mod->data = Mdl;
	mod->type = MODEL_Script;

	//	Process model definitions.
	for (VXmlNode* N = Doc->Root.FindChild("model"); N; N = N->FindNext())
	{
		VScriptModel& SMdl = Mdl->Models.Alloc();
		SMdl.Name = *N->GetAttribute("name");

		//	Process model parts.
		for (VXmlNode* SN = N->FindChild("md2"); SN; SN = SN->FindNext())
		{
			VScriptSubModel& Md2 = SMdl.SubModels.Alloc();
			Md2.Model = Mod_FindName(*SN->GetAttribute("file").ToLower().FixFileSlashes());

			//	Process frames.
			for (VXmlNode* FN = SN->FindChild("frame"); FN; FN = FN->FindNext())
			{
				VScriptSubModel::VFrame& F = Md2.Frames.Alloc();
				F.Index = atoi(*FN->GetAttribute("index"));
			}
		}
	}

	//	Process frames
	for (VXmlNode* N = Doc->Root.GetChild("frames")->FindChild("frame"); N; N = N->FindNext())
	{
		VScriptedModelFrame& F = Mdl->Frames.Alloc();
		F.Number = atoi(*N->GetAttribute("number"));
		F.Inter = 0.0;
		F.ModelIndex = -1;
		F.FrameIndex = atoi(*N->GetAttribute("frame_index"));
		VStr MdlName = N->GetAttribute("model");
		for (int i = 0; i < Mdl->Models.Num(); i++)
		{
			if (Mdl->Models[i].Name == *MdlName)
			{
				F.ModelIndex = i;
				break;
			}
		}
		if (F.ModelIndex == -1)
		{
			Sys_Error("%s has no model %s", mod->name, *MdlName);
		}
		if (N->HasAttribute("inter"))
		{
			F.Inter = atof(*N->GetAttribute("inter"));
		}
	}

	//	We don't need the XML file anymore.
	delete Doc;
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
	VStream* Strm = FL_OpenFileRead(mod->name);
	if (!Strm)
		Sys_Error("Couldn't load %s", mod->name);

	mod->data = Z_Malloc(Strm->TotalSize());
	Strm->Serialise(mod->data, Strm->TotalSize());

	if (LittleLong(*(vuint32*)mod->data) == IDPOLY2HEADER)
	{
		// swap model
		Mod_SwapAliasModel(mod);
		mod->type = MODEL_MD2;
	}
	else if (!VStr::NCmp((char*)mod->data, "<?xml", 5))
	{
		ParseModelScript(mod, *Strm);
	}
	else
	{
		Sys_Error("model %s is not a md2 model", mod->name);
	}

	delete Strm;

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

//==========================================================================
//
//	R_DrawAliasModel
//
//==========================================================================

bool R_DrawAliasModel(const TVec& Org, const TAVec& Angles, VModel* Mdl,
	int Frame, int SkinIdx, const char* Skin, vuint32 Light,
	int Translucency, bool IsViewModel, float Inter)
{
	guard(R_DrawAliasModel);
	Mod_Extradata(Mdl);
	if (Mdl->type == MODEL_Script)
	{
		VScriptedModel* SMdl = (VScriptedModel*)Mdl->data;
		int FIdx = -1;
		for (int i = 0; i < SMdl->Frames.Num(); i++)
		{
			if (SMdl->Frames[i].Number == Frame &&
				SMdl->Frames[i].Inter <= Inter)
			{
				FIdx = i;
			}
		}
		if (FIdx == -1)
		{
			return false;
		}

		VScriptedModelFrame& FDef = SMdl->Frames[FIdx];
		VScriptModel& ScMdl = SMdl->Models[FDef.ModelIndex];
		for (int i = 0; i < ScMdl.SubModels.Num(); i++)
		{
			VScriptSubModel& SubMdl = ScMdl.SubModels[i];
			if (FDef.FrameIndex >= SubMdl.Frames.Num())
			{
				GCon->Logf("Bad sub-model frame index %d", FDef.FrameIndex);
				continue;
			}
			Drawer->DrawAliasModel(Org, Angles, SubMdl.Model,
				SubMdl.Frames[FDef.FrameIndex].Index, SkinIdx, Skin, Light,
				Translucency, IsViewModel);
		}
		return true;
	}

	Drawer->DrawAliasModel(Org, Angles, Mdl, Frame, SkinIdx, Skin, Light,
		Translucency, IsViewModel);
	return true;
	unguard;
}

//==========================================================================
//
//	R_CheckAliasModelFrame
//
//==========================================================================

bool R_CheckAliasModelFrame(VModel* Mdl, int Frame, float Inter)
{
	guard(R_CheckAliasModelFrame);
	Mod_Extradata(Mdl);
	if (Mdl->type == MODEL_Script)
	{
		VScriptedModel* SMdl = (VScriptedModel*)Mdl->data;
		int FIdx = -1;
		for (int i = 0; i < SMdl->Frames.Num(); i++)
		{
			if (SMdl->Frames[i].Number == Frame &&
				SMdl->Frames[i].Inter <= Inter)
			{
				FIdx = i;
			}
		}
		if (FIdx == -1)
		{
			return false;
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	R_FreeModels
//
//==========================================================================

void R_FreeModels()
{
	guard(R_FreeModels);
	for (int i = 0; i < mod_known.Num(); i++)
	{
		if (mod_known[i]->data)
		{
			if (mod_known[i]->type == MODEL_Script)
			{
				delete (VScriptedModel*)mod_known[i]->data;
			}
			else
			{
				Z_Free(mod_known[i]->data);
			}
		}
		delete mod_known[i];
	}
	mod_known.Clear();
	unguard;
}
