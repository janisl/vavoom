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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define SMOOTHSTEP(x) ((x) * (x) * (3.0 - 2.0 * (x)))

enum { NUMVERTEXNORMALS = 162 };

// TYPES -------------------------------------------------------------------

struct VScriptSubModel
{
	struct VFrame
	{
		int		Index;
		int		PositionIndex;
		float	AlphaStart;
		float	AlphaEnd;
		TVec	Offset;
		TVec	Scale;
		int		SkinIndex;
	};

	VMeshModel*			Model;
	VMeshModel*			PositionModel;
	int					SkinAnimSpeed;
	int					SkinAnimRange;
	int					Version;
	TArray<VFrame>		Frames;
	TArray<VName>		Skins;
	bool				FullBright;
	bool				NoShadow;
};

struct VScriptModel
{
	VName						Name;
	TArray<VScriptSubModel>		SubModels;
};

struct VScriptedModelFrame
{
	int			Number;
	float		Inter;
	int			ModelIndex;
	int			FrameIndex;
	float		AngleStart;
	float		AngleEnd;
	float		AlphaStart;
	float		AlphaEnd;
};

struct VClassModelScript
{
	VName						Name;
	VModel*						Model;
	TArray<VScriptedModelFrame>	Frames;
};

struct VModel
{
	VStr						Name;
	TArray<VScriptModel>		Models;
	VClassModelScript*			DefaultClass;
};

struct TVertMap
{
	int		VertIndex;
	int		STIndex;
};

struct VTempEdge
{
	vuint16				Vert1;
	vuint16				Vert2;
	vuint16				OrigVert1;
	vuint16				OrigVert2;
	vint16				Tri1;
	vint16				Tri2;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// precalculated dot products for quantized angles
float					r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anorm_dots.h"
;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<VModel*>				mod_known;
static TArray<VMeshModel*>			GMeshModels;
static TArray<VClassModelScript*>	ClassModels;

static float			r_avertexnormals[NUMVERTEXNORMALS][3] =
{
#include "anorms.h"
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	R_InitModels
//
//==========================================================================

void R_InitModels()
{
	guard(R_InitModels);
	for (int Lump = W_IterateFile(-1, "models/models.xml"); Lump != -1;
		Lump = W_IterateFile(Lump, "models/models.xml"))
	{
		VStream* Strm = W_CreateLumpReaderNum(Lump);
		check(Strm);

		//	Parse the file.
		VXmlDocument* Doc = new VXmlDocument();
		Doc->Parse(*Strm, "models/models.xml");
		delete Strm;
		Strm = NULL;

		for (VXmlNode* N = Doc->Root.FindChild("include"); N; N = N->FindNext())
		{
			Mod_FindName(N->GetAttribute("file"));
		}

		delete Doc;
		Doc = NULL;
	}
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
		delete mod_known[i];
		mod_known[i] = NULL;
	}
	mod_known.Clear();

	for (int i = 0; i < GMeshModels.Num(); i++)
	{
		if (GMeshModels[i]->Data)
		{
			Z_Free(GMeshModels[i]->Data);
		}
		delete GMeshModels[i];
		GMeshModels[i] = NULL;
	}
	GMeshModels.Clear();

	for (int i = 0; i < ClassModels.Num(); i++)
	{
		delete ClassModels[i];
		ClassModels[i] = NULL;
	}
	ClassModels.Clear();
	unguard;
}

//==========================================================================
//
//	Mod_FindMeshModel
//
//==========================================================================

static VMeshModel* Mod_FindMeshModel(const VStr& name)
{
	guard(Mod_FindMeshModel);
	if (name.IsEmpty())
	{
		Sys_Error("Mod_ForName: NULL name");
	}

	//
	// search the currently loaded models
	//
	for (int i = 0; i < GMeshModels.Num(); i++)
	{
		if (GMeshModels[i]->Name == name)
		{
			return GMeshModels[i];
		}
	}

	VMeshModel* mod = new VMeshModel();
	mod->Name = name;
	mod->Data = NULL;
	GMeshModels.Append(mod);

	return mod;
	unguard;
}

//==========================================================================
//
//	ParseModelScript
//
//==========================================================================

static void ParseModelScript(VModel* Mdl, VStream& Strm)
{
	guard(ParseModelScript);
	//	Parse XML file.
	VXmlDocument* Doc = new VXmlDocument();
	Doc->Parse(Strm, Mdl->Name);

	//	Verify that it's a model definition file.
	if (Doc->Root.Name != "vavoom_model_definition")
	{
		Sys_Error("%s is not a valid model definition file", *Mdl->Name);
	}

	Mdl->DefaultClass = NULL;

	//	Process model definitions.
	for (VXmlNode* N = Doc->Root.FindChild("model"); N; N = N->FindNext())
	{
		VScriptModel& SMdl = Mdl->Models.Alloc();
		SMdl.Name = *N->GetAttribute("name");

		//	Process model parts.
		for (VXmlNode* SN = N->FindChild("md2"); SN; SN = SN->FindNext())
		{
			VScriptSubModel& Md2 = SMdl.SubModels.Alloc();
			Md2.Model = Mod_FindMeshModel(SN->GetAttribute("file").ToLower().FixFileSlashes());

			//	Version
			Md2.Version = -1;
			if (SN->HasAttribute("version"))
			{
				Md2.Version = atoi(*SN->GetAttribute("version"));
			}

			//	Position model
			Md2.PositionModel = NULL;
			if (SN->HasAttribute("position_file"))
			{
				Md2.PositionModel = Mod_FindMeshModel(SN->GetAttribute(
					"position_file").ToLower().FixFileSlashes());
			}

			//	Skin animation
			Md2.SkinAnimSpeed = 0;
			Md2.SkinAnimRange = 0;
			if (SN->HasAttribute("skin_anim_speed"))
			{
				Md2.SkinAnimSpeed = atoi(*SN->GetAttribute("skin_anim_speed"));
				Md2.SkinAnimRange = atoi(*SN->GetAttribute("skin_anim_range"));
			}

			//	Base offset
			TVec Offset(0.0, 0.0, 0.0);
			if (SN->HasAttribute("offset_x"))
			{
				Offset.x = atof(*SN->GetAttribute("offset_x"));
			}
			if (SN->HasAttribute("offset_y"))
			{
				Offset.y = atof(*SN->GetAttribute("offset_y"));
			}
			if (SN->HasAttribute("offset_z"))
			{
				Offset.z = atof(*SN->GetAttribute("offset_z"));
			}

			//	Base scaling
			TVec Scale(1.0, 1.0, 1.0);
			if (SN->HasAttribute("scale"))
			{
				Scale.x = atof(*SN->GetAttribute("scale"));
				Scale.y = Scale.x;
				Scale.z = Scale.x;
			}
			if (SN->HasAttribute("scale_x"))
			{
				Scale.x = atof(*SN->GetAttribute("scale_x"));
			}
			if (SN->HasAttribute("scale_y"))
			{
				Scale.y = atof(*SN->GetAttribute("scale_y"));
			}
			if (SN->HasAttribute("scale_z"))
			{
				Scale.z = atof(*SN->GetAttribute("scale_z"));
			}

			//	Full-bright flag.
			Md2.FullBright = false;
			if (SN->HasAttribute("fullbright"))
			{
				Md2.FullBright = !SN->GetAttribute("fullbright").ICmp("true");
			}

			//	No shadow flag.
			Md2.NoShadow = false;
			if (SN->HasAttribute("noshadow"))
			{
				Md2.NoShadow = !SN->GetAttribute("noshadow").ICmp("true");
			}

			//	Process frames.
			for (VXmlNode* FN = SN->FindChild("frame"); FN; FN = FN->FindNext())
			{
				VScriptSubModel::VFrame& F = Md2.Frames.Alloc();
				F.Index = atoi(*FN->GetAttribute("index"));

				//	Position model frame index
				F.PositionIndex = 0;
				if (FN->HasAttribute("position_index"))
				{
					F.PositionIndex = atoi(*FN->GetAttribute("position_index"));
				}

				//	Offset
				F.Offset = Offset;
				if (FN->HasAttribute("offset_x"))
				{
					F.Offset.x = atof(*FN->GetAttribute("offset_x"));
				}
				if (FN->HasAttribute("offset_y"))
				{
					F.Offset.y = atof(*FN->GetAttribute("offset_y"));
				}
				if (FN->HasAttribute("offset_z"))
				{
					F.Offset.z = atof(*FN->GetAttribute("offset_z"));
				}

				//	Scale
				F.Scale = Scale;
				if (FN->HasAttribute("scale"))
				{
					F.Scale.x = atof(*FN->GetAttribute("scale"));
					F.Scale.y = F.Scale.x;
					F.Scale.z = F.Scale.x;
				}
				if (FN->HasAttribute("scale_x"))
				{
					F.Scale.x = atof(*FN->GetAttribute("scale_x"));
				}
				if (FN->HasAttribute("scale_y"))
				{
					F.Scale.y = atof(*FN->GetAttribute("scale_y"));
				}
				if (FN->HasAttribute("scale_z"))
				{
					F.Scale.z = atof(*FN->GetAttribute("scale_z"));
				}

				//	Alpha
				F.AlphaStart = 1.0;
				F.AlphaEnd = 1.0;
				if (FN->HasAttribute("alpha_start"))
				{
					F.AlphaStart = atof(*FN->GetAttribute("alpha_start"));
				}
				if (FN->HasAttribute("alpha_end"))
				{
					F.AlphaEnd = atof(*FN->GetAttribute("alpha_end"));
				}

				//	Skin index
				F.SkinIndex = -1;
				if (FN->HasAttribute("skin_index"))
				{
					F.SkinIndex = atoi(*FN->GetAttribute("skin_index"));
				}
			}

			//	Process skins.
			for (VXmlNode* SkN = SN->FindChild("skin"); SkN; SkN = SkN->FindNext())
			{
				Md2.Skins.Append(*SkN->GetAttribute("file").ToLower().FixFileSlashes());
			}
		}
	}

	bool ClassDefined = false;
	for (VXmlNode* CN = Doc->Root.FindChild("class"); CN; CN = CN->FindNext())
	{
		VClassModelScript* Cls = new VClassModelScript();
		Cls->Model = Mdl;
		Cls->Name = *CN->GetAttribute("name");
		if (!Mdl->DefaultClass)
			Mdl->DefaultClass = Cls;
		ClassModels.Append(Cls);
		ClassDefined = true;

		//	Process frames
		for (VXmlNode* N = CN->FindChild("state"); N; N = N->FindNext())
		{
			VScriptedModelFrame& F = Cls->Frames.Alloc();
			F.Number = atoi(*N->GetAttribute("index"));
			F.FrameIndex = atoi(*N->GetAttribute("frame_index"));
			F.ModelIndex = -1;
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
				Sys_Error("%s has no model %s", *Mdl->Name, *MdlName);
			}

			F.Inter = 0.0;
			if (N->HasAttribute("inter"))
			{
				F.Inter = atof(*N->GetAttribute("inter"));
			}

			F.AngleStart = 0.0;
			F.AngleEnd = 0.0;
			if (N->HasAttribute("angle_start"))
			{
				F.AngleStart = atof(*N->GetAttribute("angle_start"));
			}
			if (N->HasAttribute("angle_end"))
			{
				F.AngleEnd = atof(*N->GetAttribute("angle_end"));
			}
	
			F.AlphaStart = 1.0;
			F.AlphaEnd = 1.0;
			if (N->HasAttribute("alpha_start"))
			{
				F.AlphaStart = atof(*N->GetAttribute("alpha_start"));
			}
			if (N->HasAttribute("alpha_end"))
			{
				F.AlphaEnd = atof(*N->GetAttribute("alpha_end"));
			}
		}
		if (!Cls->Frames.Num())
		{
			Sys_Error("%s class %s has no states defined",
				*Mdl->Name, *Cls->Name);
		}
	}
	if (!ClassDefined)
	{
		Sys_Error("%s defined no classes", *Mdl->Name);
	}

	//	We don't need the XML file anymore.
	delete Doc;
	Doc = NULL;
	unguard;
}

//==========================================================================
//
//	Mod_FindName
//
//==========================================================================

VModel* Mod_FindName(const VStr& name)
{
	guard(Mod_FindName);
	if (name.IsEmpty())
	{
		Sys_Error("Mod_ForName: NULL name");
	}

	//
	// search the currently loaded models
	//
	for (int i = 0; i < mod_known.Num(); i++)
	{
		if (mod_known[i]->Name == name)
		{
			return mod_known[i];
		}
	}

	VModel* mod = new VModel();
	mod->Name = name;
	mod_known.Append(mod);
	//
	// load the file
	//
	VStream* Strm = FL_OpenFileRead(mod->Name);
	if (!Strm)
	{
		Sys_Error("Couldn't load %s", *mod->Name);
	}
	ParseModelScript(mod, *Strm);
	delete Strm;
	Strm = NULL;
	return mod;
	unguard;
}

//==========================================================================
//
//	AddEdge
//
//==========================================================================

static void AddEdge(TArray<VTempEdge>& Edges, int Vert1, int OrigVert1,
	int Vert2, int OrigVert2, int Tri)
{
	guard(AddEdge);
	//	Check for a match. Compare original vertex indices since texture
	// coordinates are not important here.
	for (int i = 0; i < Edges.Num(); i++)
	{
		VTempEdge& E = Edges[i];
		if (E.Tri2 == -1 && E.OrigVert1 == OrigVert2 && E.OrigVert2 == OrigVert1)
		{
			E.Tri2 = Tri;
			return;
		}
	}

	//	Add new edge
	VTempEdge& E = Edges.Alloc();
	E.Vert1 = Vert1;
	E.Vert2 = Vert2;
	E.OrigVert1 = OrigVert1;
	E.OrigVert2 = OrigVert2;
	E.Tri1 = Tri;
	E.Tri2 = -1;
	unguard;
}

//==========================================================================
//
//	Mod_SwapAliasModel
//
//==========================================================================

static void Mod_SwapAliasModel(VMeshModel* mod)
{
	guard(Mod_SwapAliasModel);
	mmdl_t				*pmodel;
	mstvert_t			*pstverts;
	mtriangle_t			*ptri;
	mframe_t			*pframe;
	vint32				*pcmds;

	pmodel = mod->Data;
	mod->Uploaded = false;
	mod->VertsBuffer = 0;
	mod->IndexBuffer = 0;

	//
	// endian-adjust and swap the data, starting with the alias model header
	//
	for (int i = 0; i < (int)sizeof(mmdl_t) / 4; i++)
	{
		((vint32*)pmodel)[i] = LittleLong(((vint32*)pmodel)[i]);
	}

	if (pmodel->version != ALIAS_VERSION)
		Sys_Error("%s has wrong version number (%i should be %i)",
			*mod->Name, pmodel->version, ALIAS_VERSION);

	if (pmodel->numverts <= 0)
		Sys_Error("model %s has no vertices", *mod->Name);

	if (pmodel->numverts > MAXALIASVERTS)
		Sys_Error("model %s has too many vertices", *mod->Name);

	if (pmodel->numstverts <= 0)
		Sys_Error("model %s has no texture vertices", *mod->Name);

	if (pmodel->numstverts > MAXALIASSTVERTS)
		Sys_Error("model %s has too many texture vertices", *mod->Name);

	if (pmodel->numtris <= 0)
		Sys_Error("model %s has no triangles", *mod->Name);

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
	for (int i = 0; i < pmodel->numstverts; i++)
	{
		pstverts[i].s = LittleShort(pstverts[i].s);
		pstverts[i].t = LittleShort(pstverts[i].t);
	}

	//
	// triangles
	//
	TArray<TVertMap> VertMap;
	TArray<VTempEdge> Edges;
	mod->Tris.SetNum(pmodel->numtris);
	ptri = (mtriangle_t *)((byte*)pmodel + pmodel->ofstris);
	for (int i = 0; i < pmodel->numtris; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			ptri[i].vertindex[j] = LittleShort(ptri[i].vertindex[j]);
			ptri[i].stvertindex[j] = LittleShort(ptri[i].stvertindex[j]);

			bool Found = false;
			for (int vi = 0; vi < VertMap.Num(); vi++)
			{
				if (VertMap[vi].VertIndex == ptri[i].vertindex[j] &&
					VertMap[vi].STIndex == ptri[i].stvertindex[j])
				{
					Found = true;
					mod->Tris[i].VertIndex[j] = vi;
					break;
				}
			}
			if (!Found)
			{
				mod->Tris[i].VertIndex[j] = VertMap.Num();
				TVertMap& V = VertMap.Alloc();
				V.VertIndex = ptri[i].vertindex[j];
				V.STIndex = ptri[i].stvertindex[j];
			}
		}
		for (int j = 0; j < 3; j++)
		{
			AddEdge(Edges, mod->Tris[i].VertIndex[j], ptri[i].vertindex[j],
				mod->Tris[i].VertIndex[(j + 1) % 3], ptri[i].vertindex[(j + 1) % 3], i);
		}
	}

	mod->Edges.SetNum(Edges.Num());
	for (int i = 0; i < Edges.Num(); i++)
	{
		mod->Edges[i].Vert1 = Edges[i].Vert1;
		mod->Edges[i].Vert2 = Edges[i].Vert2;
		mod->Edges[i].Tri1 = Edges[i].Tri1;
		mod->Edges[i].Tri2 = Edges[i].Tri2;
	}

	//
	//	Calculate remapped ST verts.
	//
	mod->STVerts.SetNum(VertMap.Num());
	for (int i = 0; i < VertMap.Num(); i++)
	{
		mod->STVerts[i].S = (float)pstverts[VertMap[i].STIndex].s / (float)pmodel->skinwidth;
		mod->STVerts[i].T = (float)pstverts[VertMap[i].STIndex].t / (float)pmodel->skinheight;
	}

	//
	// frames
	//
	mod->Frames.SetNum(pmodel->numframes);
	mod->AllVerts.SetNum(pmodel->numframes * VertMap.Num());
	mod->AllNormals.SetNum(pmodel->numframes * VertMap.Num());
	mod->AllPlanes.SetNum(pmodel->numframes * pmodel->numtris);
	pframe = (mframe_t *)((byte*)pmodel + pmodel->ofsframes);
	for (int i = 0; i < pmodel->numframes; i++)
	{
		pframe->scale[0] = LittleFloat(pframe->scale[0]);
		pframe->scale[1] = LittleFloat(pframe->scale[1]);
		pframe->scale[2] = LittleFloat(pframe->scale[2]);
		pframe->scale_origin[0] = LittleFloat(pframe->scale_origin[0]);
		pframe->scale_origin[1] = LittleFloat(pframe->scale_origin[1]);
		pframe->scale_origin[2] = LittleFloat(pframe->scale_origin[2]);

		VMeshFrame& Frame = mod->Frames[i];
		Frame.Verts = &mod->AllVerts[i * VertMap.Num()];
		Frame.Normals = &mod->AllNormals[i * VertMap.Num()];
		Frame.Planes = &mod->AllPlanes[i * pmodel->numtris];
		Frame.VertsOffset = 0;
		Frame.NormalsOffset = 0;
		trivertx_t* Verts = (trivertx_t *)(pframe + 1);
		for (int j = 0; j < VertMap.Num(); j++)
		{
			trivertx_t& Vert = Verts[VertMap[j].VertIndex];
			Frame.Verts[j].x = Vert.v[0] * pframe->scale[0] + pframe->scale_origin[0];
			Frame.Verts[j].y = Vert.v[1] * pframe->scale[1] + pframe->scale_origin[1];
			Frame.Verts[j].z = Vert.v[2] * pframe->scale[2] + pframe->scale_origin[2];
			Frame.Normals[j] = r_avertexnormals[Vert.lightnormalindex];
		}
		for (int j = 0; j < pmodel->numtris; j++)
		{
			TVec v1 = Frame.Verts[mod->Tris[j].VertIndex[0]];
			TVec v2 = Frame.Verts[mod->Tris[j].VertIndex[1]];
			TVec v3 = Frame.Verts[mod->Tris[j].VertIndex[2]];
			TVec d1 = v2 - v3;
			TVec d2 = v1 - v3;
			TVec PlaneNormal = Normalise(CrossProduct(d1, d2));
			float PlaneDist = DotProduct(PlaneNormal, v3);
			Frame.Planes[j].Set(PlaneNormal, PlaneDist);
		}
		pframe = (mframe_t*)((byte*)pframe + pmodel->framesize);
	}

	//
	// commands
	//
	pcmds = (vint32*)((byte*)pmodel + pmodel->ofscmds);
	for (int i = 0; i < pmodel->numcmds; i++)
	{
		pcmds[i] = LittleLong(pcmds[i]);
	}

	//
	//	Skins
	//
	mskin_t* pskindesc = (mskin_t *)((byte *)pmodel + pmodel->ofsskins);
	for (int i = 0; i < pmodel->numskins; i++)
	{
		mod->Skins.Append(*VStr(pskindesc[i].name).ToLower());
	}
	unguard;
}

//==========================================================================
//
//	Mod_Extradata
//
//	Loads the data if needed
//
//==========================================================================

static mmdl_t* Mod_Extradata(VMeshModel* mod)
{
	guard(Mod_Extradata);
	if (mod->Data)
	{
		return mod->Data;
	}

	//
	// load the file
	//
	VStream* Strm = FL_OpenFileRead(mod->Name);
	if (!Strm)
	{
		Sys_Error("Couldn't load %s", *mod->Name);
	}

	mod->Data = (mmdl_t*)Z_Malloc(Strm->TotalSize());
	Strm->Serialise(mod->Data, Strm->TotalSize());
	delete Strm;
	Strm = NULL;

	if (LittleLong(*(vuint32*)mod->Data) != IDPOLY2HEADER)
	{
		Sys_Error("model %s is not a md2 model", *mod->Name);
	}

	// swap model
	Mod_SwapAliasModel(mod);

	return mod->Data;
	unguard;
}

//==========================================================================
//
//	PositionModel
//
//==========================================================================

static void PositionModel(TVec& Origin, TAVec& Angles, VMeshModel* wpmodel,
	int InFrame)
{
	guard(PositionModel);
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
	AngleVectors(Angles, md_forward, md_left, md_up);
	md_left = -md_left;
	Origin += md_forward * p[0].x + md_left * p[0].y + md_up * p[0].z;
	TAVec wangles;
	VectorAngles(p[1] - p[0], wangles);
	Angles.yaw = AngleMod(Angles.yaw + wangles.yaw);
	Angles.pitch = AngleMod(Angles.pitch + wangles.pitch);
	unguard;
}

//==========================================================================
//
//	FindFrame
//
//==========================================================================

static int FindFrame(const VClassModelScript& Cls, int Frame, float Inter)
{
	guard(FindFrame);
	int Ret = -1;
	for (int i = 0; i < Cls.Frames.Num(); i++)
	{
		if (Cls.Frames[i].Number == Frame && Cls.Frames[i].Inter <= Inter)
		{
			Ret = i;
		}
	}
	return Ret;
	unguard;
}

//==========================================================================
//
//	FindNextFrame
//
//==========================================================================

static int FindNextFrame(const VClassModelScript& Cls, int FIdx, int Frame,
	float Inter, float& InterpFrac)
{
	guard(FindNextFrame);
	const VScriptedModelFrame& FDef = Cls.Frames[FIdx];
	if (FIdx < Cls.Frames.Num() - 1 &&
		Cls.Frames[FIdx + 1].Number == FDef.Number)
	{
		InterpFrac = (Inter - FDef.Inter) / (Cls.Frames[FIdx + 1].Inter -
			FDef.Inter);
		return FIdx + 1;
	}
	InterpFrac = (Inter - FDef.Inter) / (1.0 - FDef.Inter);
	return FindFrame(Cls, Frame, 0);
	unguard;
}

//==========================================================================
//
//	DrawModel
//
//==========================================================================

static void DrawModel(VLevel* Level, const TVec& Org, const TAVec& Angles,
	float ScaleX, float ScaleY, VClassModelScript& Cls, int FIdx, int NFIdx,
	VTextureTranslation* Trans, int ColourMap, int Version, vuint32 Light,
	vuint32 Fade, float Alpha, bool Additive, bool IsViewModel, float Inter,
	bool Interpolate, const TVec& LightPos, float LightRadius, ERenderPass Pass)
{
	guard(DrawModel);
	VScriptedModelFrame& FDef = Cls.Frames[FIdx];
	VScriptedModelFrame& NFDef = Cls.Frames[NFIdx];
	VScriptModel& ScMdl = Cls.Model->Models[FDef.ModelIndex];
	for (int i = 0; i < ScMdl.SubModels.Num(); i++)
	{
		VScriptSubModel& SubMdl = ScMdl.SubModels[i];
		if (SubMdl.Version != -1 && SubMdl.Version != Version)
		{
			continue;
		}
		if (FDef.FrameIndex >= SubMdl.Frames.Num())
		{
			GCon->Logf("Bad sub-model frame index %d", FDef.FrameIndex);
			continue;
		}
		if (Interpolate && NFDef.FrameIndex >= SubMdl.Frames.Num() &&
			NFDef.ModelIndex != FDef.ModelIndex)
		{
			NFDef.FrameIndex = FDef.FrameIndex;
			Interpolate = false;
			continue;
		}
		if (Interpolate && FDef.ModelIndex != NFDef.ModelIndex)
		{
			Interpolate = false;
		}
		if (NFDef.FrameIndex >= SubMdl.Frames.Num())
		{
			continue;
		}
		VScriptSubModel::VFrame& F = SubMdl.Frames[FDef.FrameIndex];
		VScriptSubModel::VFrame& NF = SubMdl.Frames[NFDef.FrameIndex];

		//	Locate the proper data.
		mmdl_t* pmdl = (mmdl_t*)Mod_Extradata(SubMdl.Model);

		//	Skin aniations.
		int Md2SkinIdx = 0;
		if (F.SkinIndex >= 0)
		{
			Md2SkinIdx = F.SkinIndex;
		}
		else if (SubMdl.SkinAnimSpeed)
		{
			Md2SkinIdx = int((Level ? Level->Time : 0) * SubMdl.SkinAnimSpeed) %
				SubMdl.SkinAnimRange;
		}

		//	Get the proper skin texture ID.
		int SkinID;
		if (SubMdl.Skins.Num())
		{
			//	Skins defined in definition file override all skins in MD2 file.
			if (Md2SkinIdx < 0 || Md2SkinIdx >= SubMdl.Skins.Num())
			{
				SkinID = GTextureManager.AddFileTexture(
					SubMdl.Skins[0], TEXTYPE_Skin);
			}
			else
			{
				SkinID = GTextureManager.AddFileTexture(
					SubMdl.Skins[Md2SkinIdx], TEXTYPE_Skin);
			}
		}
		else
		{
			if (Md2SkinIdx < 0 || Md2SkinIdx >= pmdl->numskins)
			{
				SkinID = GTextureManager.AddFileTexture(
					SubMdl.Model->Skins[0], TEXTYPE_Skin);
			}
			else
			{
				SkinID = GTextureManager.AddFileTexture(
					SubMdl.Model->Skins[Md2SkinIdx], TEXTYPE_Skin);
			}
		}

		//	Get and verify frame number.
		int Md2Frame = F.Index;
		if (Md2Frame >= pmdl->numframes || Md2Frame < 0)
		{
			GCon->Logf(NAME_Dev, "no such frame %d in %s", Md2Frame,
				*SubMdl.Model->Name);
			Md2Frame = 0;
			//	Stop further warnings.
			F.Index = 0;
		}

		//  Get and verify next frame number.
		int Md2NextFrame = NF.Index;
		if (Md2NextFrame >= pmdl->numframes || Md2NextFrame < 0)
		{
			GCon->Logf(NAME_Dev, "no such next frame %d in %s", Md2NextFrame,
				*SubMdl.Model->Name);
			Md2NextFrame = 0;
			//	Stop further warnings.
			NF.Index = 0;
		}

		//	Position
		TVec Md2Org = Org;

		//	Angle
		TAVec Md2Angle = Angles;
		if (FDef.AngleStart || FDef.AngleEnd != 1.0)
		{
			Md2Angle.yaw = AngleMod(Md2Angle.yaw + FDef.AngleStart +
				(FDef.AngleEnd - FDef.AngleStart) * Inter);
		}

		//	Position model
		if (SubMdl.PositionModel)
		{
			PositionModel(Md2Org, Md2Angle, SubMdl.PositionModel, F.PositionIndex);
		}

		//	Alpha
		float Md2Alpha = Alpha;
		if (FDef.AlphaStart != 1.0 || FDef.AlphaEnd != 1.0)
		{
			Md2Alpha *= FDef.AlphaStart + (FDef.AlphaEnd - FDef.AlphaStart) * Inter;
		}
		if (F.AlphaStart != 1.0 || F.AlphaEnd != 1.0)
		{
			Md2Alpha *= F.AlphaStart + (F.AlphaEnd - F.AlphaStart) * Inter;
		}
		switch (Pass)
		{
		case RPASS_Normal:
			break;

		case RPASS_Ambient:
		case RPASS_ShadowVolumes:
		case RPASS_Light:
		case RPASS_Textures:
		case RPASS_Fog:
			if (Md2Alpha < 1 || SubMdl.NoShadow)
			{
				continue;
			}
			break;

		case RPASS_NonShadow:
			if (Md2Alpha >= 1.0 && !Additive && !SubMdl.NoShadow)
			{
				continue;
			}
			break;
		}

		float smooth_inter = Interpolate ? SMOOTHSTEP(Inter) : 0.0;

		//	Scale, in case of models thing's ScaleX scales x and y and ScaleY
		// scales z.
		TVec Scale;
		if (Interpolate)
		{
			// Interpolate Scale
			Scale.x = (F.Scale.x + smooth_inter * (NF.Scale.x - F.Scale.x) * ScaleX);
			Scale.y = (F.Scale.y + smooth_inter * (NF.Scale.y - F.Scale.y) * ScaleX);
			Scale.z = (F.Scale.z + smooth_inter * (NF.Scale.z - F.Scale.z) * ScaleY);
		}
		else
		{
			Scale.x = F.Scale.x * ScaleX;
			Scale.y = F.Scale.y * ScaleX;
			Scale.z = F.Scale.z * ScaleY;
		}

		TVec Offset;
		if (Interpolate)
		{
			// Interpolate Offsets too
			Offset.x = ((1 - smooth_inter) * F.Offset.x + (smooth_inter) * NF.Offset.x);
			Offset.y = ((1 - smooth_inter) * F.Offset.y + (smooth_inter) * NF.Offset.y);
			Offset.z = ((1 - smooth_inter) * F.Offset.z + (smooth_inter) * NF.Offset.z);
		}
		else
		{
			Offset.x = F.Offset.x;
			Offset.y = F.Offset.y;
			Offset.z = F.Offset.z;
		}

		//	Light
		vuint32 Md2Light = Light;
		if (SubMdl.FullBright)
		{
			Md2Light = 0xffffffff;
		}

		switch (Pass)
		{
		case RPASS_Normal:
		case RPASS_NonShadow:
			Drawer->DrawAliasModel(Md2Org, Md2Angle, Offset, Scale,
				SubMdl.Model, Md2Frame, Md2NextFrame, GTextureManager(SkinID),
				Trans, ColourMap, Md2Light, Fade, Md2Alpha, Additive,
				IsViewModel, smooth_inter, Interpolate);
			break;

		case RPASS_Ambient:
			Drawer->DrawAliasModelAmbient(Md2Org, Md2Angle, Offset, Scale,
				SubMdl.Model, Md2Frame, Md2NextFrame, GTextureManager(SkinID),
				Md2Light, smooth_inter, Interpolate);
			break;

		case RPASS_ShadowVolumes:
			Drawer->DrawAliasModelShadow(Md2Org, Md2Angle, Offset, Scale,
				SubMdl.Model, Md2Frame, Md2NextFrame, smooth_inter, Interpolate,
				LightPos, LightRadius);
			break;

		case RPASS_Light:
			Drawer->DrawAliasModelLight(Md2Org, Md2Angle, Offset, Scale,
				SubMdl.Model, Md2Frame, Md2NextFrame, GTextureManager(SkinID),
				smooth_inter, Interpolate);
			break;

		case RPASS_Textures:
			Drawer->DrawAliasModelTextures(Md2Org, Md2Angle, Offset, Scale,
				SubMdl.Model, Md2Frame, Md2NextFrame, GTextureManager(SkinID),
				Trans, ColourMap, smooth_inter, Interpolate);
			break;

		case RPASS_Fog:
			Drawer->DrawAliasModelFog(Md2Org, Md2Angle, Offset, Scale,
				SubMdl.Model, Md2Frame, Md2NextFrame, GTextureManager(SkinID),
				Fade, smooth_inter, Interpolate);
			break;
		}
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::DrawAliasModel
//
//==========================================================================

bool VRenderLevelShared::DrawAliasModel(const TVec& Org, const TAVec& Angles,
	float ScaleX, float ScaleY, VModel* Mdl, int Frame, int NextFrame,
	VTextureTranslation* Trans, int Version, vuint32 Light, vuint32 Fade,
	float Alpha, bool Additive, bool IsViewModel, float Inter, bool Interpolate,
	ERenderPass Pass)
{
	guard(VRenderLevelShared::DrawAliasModel);
	int FIdx = FindFrame(*Mdl->DefaultClass, Frame, Inter);
	if (FIdx == -1)
	{
		return false;
	}
	float InterpFrac;
	int NFIdx = FindNextFrame(*Mdl->DefaultClass, FIdx, NextFrame, Inter,
		InterpFrac);
	if (NFIdx == -1)
	{
		NFIdx = FIdx;
		Interpolate = false;
	}

	DrawModel(Level, Org, Angles, ScaleX, ScaleY, *Mdl->DefaultClass, FIdx,
		NFIdx, Trans, ColourMap, Version, Light, Fade, Alpha, Additive,
		IsViewModel, InterpFrac, Interpolate, CurrLightPos, CurrLightRadius,
		Pass);
	return true;
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::DrawAliasModel
//
//==========================================================================

bool VRenderLevelShared::DrawAliasModel(const TVec& Org, const TAVec& Angles,
	float ScaleX, float ScaleY, VState* State, VState* NextState,
	VTextureTranslation* Trans, int Version, vuint32 Light, vuint32 Fade,
	float Alpha, bool Additive, bool IsViewModel, float Inter, bool Interpolate,
	ERenderPass Pass)
{
	guard(VRenderLevelShared::DrawAliasModel);
	VClassModelScript* Cls = NULL;
	for (int i = 0; i < ClassModels.Num(); i++)
	{
		if (ClassModels[i]->Name == State->Outer->Name)
		{
			Cls = ClassModels[i];
		}
	}
	if (!Cls)
	{
		return false;
	}

	int FIdx = FindFrame(*Cls, State->InClassIndex, Inter);
	if (FIdx == -1)
	{
		return false;
	}

	float InterpFrac;
	int NFIdx = FindNextFrame(*Cls, FIdx, NextState->InClassIndex, Inter,
		InterpFrac);
	if (NFIdx == -1)
	{
		NFIdx = FIdx;
		Interpolate = false;
	}

	DrawModel(Level, Org, Angles, ScaleX, ScaleY, *Cls, FIdx, NFIdx, Trans,
		ColourMap, Version, Light, Fade, Alpha, Additive, IsViewModel,
		InterpFrac, Interpolate, CurrLightPos, CurrLightRadius, Pass);
	return true;
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::DrawEntityModel
//
//==========================================================================

bool VRenderLevelShared::DrawEntityModel(VEntity* Ent, vuint32 Light, vuint32 Fade,
	float Alpha, bool Additive, float Inter, ERenderPass Pass)
{
	guard(VRenderLevelShared::DrawEntityModel);
	VState* DispState = (Ent->EntityFlags & VEntity::EF_UseDispState) ?
		Ent->DispState : Ent->State;
	// Check if we want to interpolate model frames
	bool Interpolate;
	if (!r_interpolate_frames)
	{
		Interpolate = false;
	}
	else
	{
		Interpolate = true;
	}
	if (Ent->EntityFlags & VEntity::EF_FixedModel)
	{
		if (!FL_FileExists(VStr("models/") + Ent->FixedModelName))
		{
			GCon->Logf("Can't find %s", *Ent->FixedModelName);
			return false;
		}
		VModel* Mdl = Mod_FindName(VStr("models/") + Ent->FixedModelName);
		if (!Mdl)
		{
			return false;
		}
		return DrawAliasModel(Ent->Origin - TVec(0, 0, Ent->FloorClip),
			Ent->Angles, Ent->ScaleX, Ent->ScaleY, Mdl,
			DispState->InClassIndex,
			DispState->NextState ? DispState->NextState->InClassIndex :
			DispState->InClassIndex, GetTranslation(Ent->Translation),
			Ent->ModelVersion, Light, Fade, Alpha, Additive, false, Inter,
			Interpolate, Pass);
	}
	else
	{
		return DrawAliasModel(Ent->Origin - TVec(0, 0, Ent->FloorClip),
			Ent->Angles, Ent->ScaleX, Ent->ScaleY, DispState,
			DispState->NextState ? DispState->NextState : DispState,
			GetTranslation(Ent->Translation), Ent->ModelVersion, Light, Fade,
			Alpha, Additive, false, Inter, Interpolate, Pass);
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::CheckAliasModelFrame
//
//==========================================================================

bool VRenderLevelShared::CheckAliasModelFrame(VEntity* Ent, float Inter)
{
	guard(VRenderLevelShared::CheckAliasModelFrame);
	if (Ent->EntityFlags & VEntity::EF_FixedModel)
	{
		if (!FL_FileExists(VStr("models/") + Ent->FixedModelName))
		{
			return false;
		}
		VModel* Mdl = Mod_FindName(VStr("models/") + Ent->FixedModelName);
		if (!Mdl)
		{
			return false;
		}
		return FindFrame(*Mdl->DefaultClass, Ent->State->InClassIndex, Inter) != -1;
	}
	else
	{
		VClassModelScript* Cls = NULL;
		for (int i = 0; i < ClassModels.Num(); i++)
		{
			if (ClassModels[i]->Name == Ent->State->Outer->Name)
			{
				Cls = ClassModels[i];
			}
		}
		if (!Cls)
		{
			return false;
		}
		return FindFrame(*Cls, Ent->State->InClassIndex, Inter) != -1;
	}
	unguard;
}

//==========================================================================
//
//	R_DrawModelFrame
//
//==========================================================================

void R_DrawModelFrame(const TVec& Origin, float Angle, VModel* Model,
	int Frame, int NextFrame, const char* Skin, int TranslStart,
	int TranslEnd, int Colour, float Inter)
{
	guard(R_DrawModelFrame);
	bool Interpolate = true;
	int FIdx = FindFrame(*Model->DefaultClass, Frame, Inter);
	if (FIdx == -1)
	{
		return;
	}

	float InterpFrac;
	int NFIdx = FindNextFrame(*Model->DefaultClass, FIdx, NextFrame, Inter,
		InterpFrac);
	if (NFIdx == -1)
	{
		NFIdx = 0;
		Interpolate = false;
	}

	viewangles.yaw = 180;
	viewangles.pitch = 0;
	viewangles.roll = 0;
	AngleVectors(viewangles, viewforward, viewright, viewup);
	vieworg = TVec(0, 0, 0);

	refdef_t	rd;

	rd.x = 0;
	rd.y = 0;
	rd.width = ScreenWidth;
	rd.height = ScreenHeight;
	rd.fovx = tan(DEG2RAD(90) / 2);
	rd.fovy = rd.fovx * 3.0 / 4.0;
	rd.drawworld = false;
	rd.DrawCamera = false;

	Drawer->SetupView(NULL, &rd);
	Drawer->SetupViewOrg();

	TAVec Angles;
	Angles.yaw = Angle;
	Angles.pitch = 0;
	Angles.roll = 0;

	DrawModel(NULL, Origin, Angles, 1.0, 1.0, *Model->DefaultClass, FIdx,
		NFIdx, R_GetCachedTranslation(R_SetMenuPlayerTrans(TranslStart,
		TranslEnd, Colour), NULL), 0, 0, 0xffffffff, 0, 1.0, false, false,
		InterpFrac, Interpolate, TVec(), 0, RPASS_Normal);

	Drawer->EndView();
	unguard;
}

//==========================================================================
//
//	R_DrawStateModelFrame
//
//==========================================================================

bool R_DrawStateModelFrame(VState* State, VState* NextState, float Inter,
	 const TVec& Origin, float Angle)
{
	VClassModelScript* Cls = NULL;
	bool Interpolate = true;
	for (int i = 0; i < ClassModels.Num(); i++)
	{
		if (ClassModels[i]->Name == State->Outer->Name)
		{
			Cls = ClassModels[i];
		}
	}
	if (!Cls)
	{
		return false;
	}
	int FIdx = FindFrame(*Cls, State->InClassIndex, Inter);
	if (FIdx == -1)
	{
		return false;
	}
	float InterpFrac;
	int NFIdx = FindNextFrame(*Cls, FIdx, NextState->InClassIndex, Inter,
		InterpFrac);
	if (NFIdx == -1)
	{
		NFIdx = 0;
		Interpolate = false;
	}

	viewangles.yaw = 180;
	viewangles.pitch = 0;
	viewangles.roll = 0;
	AngleVectors(viewangles, viewforward, viewright, viewup);
	vieworg = TVec(0, 0, 0);

	refdef_t	rd;

	rd.x = 0;
	rd.y = 0;
	rd.width = ScreenWidth;
	rd.height = ScreenHeight;
	rd.fovx = tan(DEG2RAD(90) / 2);
	rd.fovy = rd.fovx * rd.height / rd.width / PixelAspect;
	rd.drawworld = false;
	rd.DrawCamera = false;

	Drawer->SetupView(NULL, &rd);
	Drawer->SetupViewOrg();

	TAVec Angles;
	Angles.yaw = Angle;
	Angles.pitch = 0;
	Angles.roll = 0;

	DrawModel(NULL, Origin, Angles, 1.0, 1.0, *Cls, FIdx, NFIdx, NULL, 0, 0,
		0xffffffff, 0, 1.0, false, false, InterpFrac, Interpolate,
		TVec(), 0, RPASS_Normal);

	Drawer->EndView();
	return true;
}
