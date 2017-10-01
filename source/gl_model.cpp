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

#include "gl_local.h"

// MACROS ------------------------------------------------------------------

#define outv(idx, offs) \
			p_glVertexAttrib1fARB(ShadowsModelShadowOffsetLoc, offs); \
			glArrayElement(index ## idx);

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	AliasSetUpTransform
//
//==========================================================================

static void AliasSetUpTransform(const TVec& modelorg, const TAVec& angles,
	const TVec& Offset, const TVec& Scale, VMatrix4& RotationMatrix)
{
	VMatrix4 t3matrix = VMatrix4::Identity;
	t3matrix[0][0] = Scale.x;
	t3matrix[1][1] = Scale.y;
	t3matrix[2][2] = Scale.z;

	t3matrix[0][3] = Scale.x * Offset.x;
	t3matrix[1][3] = Scale.y * Offset.y;
	t3matrix[2][3] = Scale.z * Offset.z;

	TVec alias_forward, alias_right, alias_up;
	AngleVectors(angles, alias_forward, alias_right, alias_up);

	VMatrix4 t2matrix = VMatrix4::Identity;
	for (int i = 0; i < 3; i++)
	{
		t2matrix[i][0] = alias_forward[i];
		t2matrix[i][1] = -alias_right[i];
		t2matrix[i][2] = alias_up[i];
	}

	t2matrix[0][3] = modelorg[0];
	t2matrix[1][3] = modelorg[1];
	t2matrix[2][3] = modelorg[2];

	RotationMatrix = t2matrix * t3matrix;
}

//==========================================================================
//
//	AliasSetUpNormalTransform
//
//==========================================================================

static void AliasSetUpNormalTransform(const TAVec& angles, const TVec& Scale,
	VMatrix4& RotationMatrix)
{
	TVec alias_forward, alias_right, alias_up;
	AngleVectors(angles, alias_forward, alias_right, alias_up);

	VMatrix4 t3matrix = VMatrix4::Identity;
	t3matrix[0][0] = Scale.x;
	t3matrix[1][1] = Scale.y;
	t3matrix[2][2] = Scale.z;

	VMatrix4 t2matrix = VMatrix4::Identity;
	for (int i = 0; i < 3; i++)
	{
		t2matrix[i][0] = alias_forward[i];
		t2matrix[i][1] = -alias_right[i];
		t2matrix[i][2] = alias_up[i];
	}

	RotationMatrix = t2matrix * t3matrix;

	if (fabs(Scale.x) != fabs(Scale.y) || fabs(Scale.x) != fabs(Scale.z))
	{
		//	Non-uniform scale, do full inverse transpose.
		RotationMatrix = RotationMatrix.Inverse().Transpose();
	}
}

//==========================================================================
//
//	VOpenGLDrawer::UploadModel
//
//==========================================================================

void VOpenGLDrawer::UploadModel(VMeshModel* Mdl)
{
	guard(VOpenGLDrawer::UploadModel);
	if (Mdl->Uploaded)
	{
		return;
	}

	//	Create buffer.
	p_glGenBuffersARB(1, &Mdl->VertsBuffer);
	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, Mdl->VertsBuffer);
	int Size = sizeof(VMeshSTVert) * Mdl->STVerts.Num() +
		sizeof(TVec) * Mdl->STVerts.Num() * 2 * Mdl->Frames.Num();
	p_glBufferDataARB(GL_ARRAY_BUFFER_ARB, Size, NULL, GL_STATIC_DRAW_ARB);

	//	Upload data
	p_glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(VMeshSTVert) * Mdl->STVerts.Num(),
		&Mdl->STVerts[0]);
	p_glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(VMeshSTVert) * Mdl->STVerts.Num(),
		sizeof(TVec) * Mdl->AllVerts.Num(), &Mdl->AllVerts[0]);
	p_glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(VMeshSTVert) * Mdl->STVerts.Num() +
		sizeof(TVec) * Mdl->AllVerts.Num(), sizeof(TVec) * Mdl->AllNormals.Num(),
		&Mdl->AllNormals[0]);

	//	Pre-calculate offsets.
	for (int i = 0; i < Mdl->Frames.Num(); i++)
	{
		Mdl->Frames[i].VertsOffset = sizeof(VMeshSTVert) * Mdl->STVerts.Num() +
			i * sizeof(TVec) * Mdl->STVerts.Num();
		Mdl->Frames[i].NormalsOffset = sizeof(VMeshSTVert) * Mdl->STVerts.Num() +
			sizeof(TVec) * Mdl->AllVerts.Num() +
			i * sizeof(TVec) * Mdl->STVerts.Num();
	}

	//	Indexes
	p_glGenBuffersARB(1, &Mdl->IndexBuffer);
	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Mdl->IndexBuffer);
	p_glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 6 * Mdl->Tris.Num(),
		&Mdl->Tris[0], GL_STATIC_DRAW_ARB);

	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	Mdl->Uploaded = true;
	UploadedModels.Append(Mdl);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::UnloadModels
//
//==========================================================================

void VOpenGLDrawer::UnloadModels()
{
	guard(VOpenGLDrawer::UnloadModels);
	for (int i = 0; i < UploadedModels.Num(); i++)
	{
		p_glDeleteBuffersARB(1, &UploadedModels[i]->VertsBuffer);
		p_glDeleteBuffersARB(1, &UploadedModels[i]->IndexBuffer);
		UploadedModels[i]->Uploaded = false;
	}
	UploadedModels.Clear();
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawAliasModel
//
//==========================================================================

void VOpenGLDrawer::DrawAliasModel(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, VMeshModel* Mdl, int frame, int nextframe,
	VTexture* Skin, VTextureTranslation* Trans, int CMap, vuint32 light,
	vuint32 Fade, float Alpha, bool Additive, bool is_view_model, float Inter,
	bool Interpolate, bool ForceDepthUse, bool AllowTransparency)
{
	guard(VOpenGLDrawer::DrawAliasModel);
	if (is_view_model)
	{
		// hack the depth range to prevent view model from poking into walls
		glDepthRange(0.0, 0.3);
	}

	//
	// get lighting information
	//
	float shadelightr = ((light >> 16) & 255) / 255.0;
	float shadelightg = ((light >> 8) & 255) / 255.0;
	float shadelightb = (light & 255) / 255.0;
	float* shadedots = r_avertexnormal_dots[((int)(angles.yaw * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];

	//
	// draw all the triangles
	//
	glPushMatrix();
	glTranslatef(origin.x, origin.y, origin.z);

	glRotatef(angles.yaw,  0, 0, 1);
	glRotatef(angles.pitch,  0, 1, 0);
	glRotatef(angles.roll,  1, 0, 0);

	glScalef(Scale.x, Scale.y, Scale.z);
	glTranslatef(Offset.x, Offset.y, Offset.z);

	mmdl_t* pmdl = Mdl->Data;
	mframe_t* framedesc = (mframe_t*)((byte *)pmdl + pmdl->ofsframes + frame * pmdl->framesize);
	mframe_t* nextframedesc = (mframe_t*)((byte *)pmdl + pmdl->ofsframes + nextframe * pmdl->framesize);

	if (!HaveShaders || !HaveVertexBufferObject)
	{
		// Interpolate Scales
		TVec scale_origin;
		if (Interpolate)
		{
			scale_origin[0] = ((1 - Inter) * framedesc->scale_origin[0] + Inter * nextframedesc->scale_origin[0]);
			scale_origin[1] = ((1 - Inter) * framedesc->scale_origin[1] + Inter * nextframedesc->scale_origin[1]);
			scale_origin[2] = ((1 - Inter) * framedesc->scale_origin[2] + Inter * nextframedesc->scale_origin[2]);
		}
		else
		{
			scale_origin[0] = framedesc->scale_origin[0];
			scale_origin[1] = framedesc->scale_origin[1];
			scale_origin[2] = framedesc->scale_origin[2];
		}
		glTranslatef(scale_origin[0], scale_origin[1], scale_origin[2]);

		TVec scale;
		if (Interpolate)
		{
			scale[0] = framedesc->scale[0] + Inter * (nextframedesc->scale[0] - framedesc->scale[0]) * Scale.x;
			scale[1] = framedesc->scale[1] + Inter * (nextframedesc->scale[1] - framedesc->scale[1]) * Scale.y;
			scale[2] = framedesc->scale[2] + Inter * (nextframedesc->scale[2] - framedesc->scale[2]) * Scale.z;
		}
		else
		{
			scale[0] = framedesc->scale[0];
			scale[1] = framedesc->scale[1];
			scale[2] = framedesc->scale[2];
		}
		glScalef(scale[0], scale[1], scale[2]);
	}

	SetPic(Skin, Trans, CMap);

	glEnable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);
	glAlphaFunc(GL_GREATER, 0.0);
	glEnable(GL_BLEND);

	if (HaveShaders)
	{
		p_glUseProgramObjectARB(SurfModelProgram);
		p_glUniform1iARB(SurfModelTextureLoc, 0);
		p_glUniform1iARB(SurfModelFogTypeLoc, r_fog & 3);

		if (Alpha < 1.0)
		{
			p_glUniform1fARB(ShadowsModelAlphaLoc, Alpha);
		}
		else
		{
			p_glUniform1fARB(ShadowsModelAlphaLoc, 1.0);
		}

		if (Fade)
		{
			p_glUniform1iARB(SurfModelFogEnabledLoc, GL_TRUE);
			p_glUniform4fARB(SurfModelFogColourLoc,
				((Fade >> 16) & 255) / 255.0,
				((Fade >> 8) & 255) / 255.0,
				(Fade & 255) / 255.0, Alpha);
			p_glUniform1fARB(SurfModelFogDensityLoc, Fade == FADE_LIGHT ? 0.3 : r_fog_density);
			p_glUniform1fARB(SurfModelFogStartLoc, Fade == FADE_LIGHT ? 1.0 : r_fog_start);
			p_glUniform1fARB(SurfModelFogEndLoc, Fade == FADE_LIGHT ? 1024.0 * r_fade_factor : r_fog_end);
		}
		else
		{
			p_glUniform1iARB(SurfModelFogEnabledLoc, GL_FALSE);
		}

		if (AllowTransparency)
		{
			p_glUniform1iARB(SurfModelAllowTransparency, GL_TRUE);
		}
		else
		{
			p_glUniform1iARB(SurfModelAllowTransparency, GL_FALSE);
		}
		p_glUniform1fARB(SurfModelInterLoc, Inter);
	}
	else
	{
		if (!model_lighting)
		{
			SetColour((light & 0x00ffffff) | (int(255 * Alpha) << 24));
		}
		SetFade(Fade);
	}

	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}

	if (HaveShaders && HaveVertexBufferObject)
	{
		UploadModel(Mdl);
		VMeshFrame* FrameDesc = &Mdl->Frames[frame];
		VMeshFrame* NextFrameDesc = &Mdl->Frames[nextframe];

		p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, Mdl->VertsBuffer);
		p_glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)FrameDesc->VertsOffset);
		p_glEnableVertexAttribArrayARB(0);
		p_glVertexAttribPointerARB(SurfModelVert2Loc, 3, GL_FLOAT, GL_FALSE, 0,
			(void*)NextFrameDesc->VertsOffset);
		p_glEnableVertexAttribArrayARB(SurfModelVert2Loc);
		p_glVertexAttribPointerARB(SurfModelTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
		p_glEnableVertexAttribArrayARB(SurfModelTexCoordLoc);
		p_glUniform3fARB(SurfModelViewOrigin, vieworg.x, vieworg.y, vieworg.z);
		p_glVertexAttrib4fARB(SurfModelLightValLoc,
			((light >> 16) & 255) / 255.0,
			((light >> 8) & 255) / 255.0,
			(light & 255) / 255.0, Alpha);

		p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Mdl->IndexBuffer);
		if (Alpha < 1.0 && !ForceDepthUse || AllowTransparency)
		{
			glDepthMask(GL_FALSE);
		}
		p_glDrawRangeElementsEXT(GL_TRIANGLES, 0, Mdl->STVerts.Num() - 1,
			Mdl->Tris.Num() * 3, GL_UNSIGNED_SHORT, 0);
		if (Alpha < 1.0 && !ForceDepthUse || AllowTransparency)
		{
			glDepthMask(GL_TRUE);
		}
		p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		p_glDisableVertexAttribArrayARB(0);
		p_glDisableVertexAttribArrayARB(SurfModelVert2Loc);
		p_glDisableVertexAttribArrayARB(SurfModelTexCoordLoc);
		p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	else
	{
		trivertx_t* verts = (trivertx_t *)(framedesc + 1);
		trivertx_t* verts2 = (trivertx_t *)(nextframedesc + 1);
		int* order = (int *)((byte *)pmdl + pmdl->ofscmds);

		while (*order)
		{
			// get the vertex count and primitive type
			int count = *order++;
			if (Alpha < 1.0 && !ForceDepthUse)
			{
				glDepthMask(GL_FALSE);
			}
			if (count < 0)
			{
				count = -count;
				glBegin(GL_TRIANGLE_FAN);
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
			}
			if (Alpha < 1.0 && !ForceDepthUse)
			{
				glDepthMask(GL_TRUE);
			}

			do
			{
				if (HaveShaders)
				{
					// texture coordinates come from the draw list
					p_glVertexAttrib2fARB(SurfModelTexCoordLoc, ((float *)order)[0], ((float *)order)[1]);
					order += 2;

					// normals and vertexes come from the frame list
					int index = *order++;
					if (model_lighting)
					{
						float l = shadedots[verts[index].lightnormalindex];
						p_glVertexAttrib4fARB(SurfModelLightValLoc,
							l * shadelightr, l * shadelightg, l * shadelightb, Alpha);
					}
					else
					{
						p_glVertexAttrib4fARB(SurfModelLightValLoc,
							((light >> 16) & 255) / 255.0,
							((light >> 8) & 255) / 255.0,
							(light & 255) / 255.0, Alpha);
					}
					p_glVertexAttrib3fARB(SurfModelVert2Loc,
						verts2[index].v[0], verts2[index].v[1], verts2[index].v[2]);
					glVertex3f(verts[index].v[0], verts[index].v[1], verts[index].v[2]);
				}
				else
				{
					// texture coordinates come from the draw list
					glTexCoord2f(((float *)order)[0], ((float *)order)[1]);
					order += 2;

					// normals and vertexes come from the frame list
					int index = *order++;
					if (model_lighting)
					{
						float l = shadedots[verts[index].lightnormalindex];
						glColor4f(l * shadelightr, l * shadelightg, l * shadelightb, Alpha);
					}
					if (Interpolate)
					{
						glVertex3f((1 - Inter) * verts[index].v[0] + Inter * verts2[index].v[0],
							(1 - Inter) * verts[index].v[1] + Inter * verts2[index].v[1],
							(1 - Inter) * verts[index].v[2] + Inter * verts2[index].v[2]);
					}
					else
					{
						glVertex3f(verts[index].v[0], verts[index].v[1], verts[index].v[2]);
					}
				}
			} while (--count);

			glEnd();
		}
	}
	glDisable(GL_BLEND);
	glShadeModel(GL_FLAT);
	glAlphaFunc(GL_GREATER, 0.333);
	glDisable(GL_ALPHA_TEST);
	if (Additive)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glPopMatrix();
	if (is_view_model)
	{
		glDepthRange(0.0, 1.0);
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawAliasModelAmbient
//
//==========================================================================

void VOpenGLDrawer::DrawAliasModelAmbient(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, VMeshModel* Mdl, int frame, int nextframe,
	VTexture* Skin, vuint32 light, float Alpha, float Inter, bool Interpolate,
	bool ForceDepth, bool AllowTransparency)
{
	guard(VOpenGLDrawer::DrawAliasModelAmbient);
	UploadModel(Mdl);
	VMeshFrame* FrameDesc = &Mdl->Frames[frame];
	VMeshFrame* NextFrameDesc = &Mdl->Frames[nextframe];

	SetPic(Skin, NULL, CM_Default);

	VMatrix4 RotationMatrix;
	AliasSetUpTransform(origin, angles, Offset, Scale, RotationMatrix);
	VMatrix4 normalmatrix;
	AliasSetUpNormalTransform(angles, Scale, normalmatrix);
	float NormalMat[3][3];
	NormalMat[0][0] = normalmatrix[0][0];
	NormalMat[0][1] = normalmatrix[0][1];
	NormalMat[0][2] = normalmatrix[0][2];
	NormalMat[1][0] = normalmatrix[1][0];
	NormalMat[1][1] = normalmatrix[1][1];
	NormalMat[1][2] = normalmatrix[1][2];
	NormalMat[2][0] = normalmatrix[2][0];
	NormalMat[2][1] = normalmatrix[2][1];
	NormalMat[2][2] = normalmatrix[2][2];

	p_glUseProgramObjectARB(ShadowsModelAmbientProgram);
	p_glUniform1iARB(ShadowsModelAmbientTextureLoc, 0);
	p_glUniform1fARB(ShadowsModelAmbientInterLoc, Inter);
	p_glUniform4fARB(ShadowsModelAmbientLightLoc,
		((light >> 16) & 255) / 255.0,
		((light >> 8) & 255) / 255.0,
		(light & 255) / 255.0, Alpha);
	p_glUniformMatrix4fvARB(ShadowsModelAmbientModelToWorldMatLoc, 1, GL_FALSE, RotationMatrix[0]);
	p_glUniformMatrix3fvARB(ShadowsModelAmbientNormalToWorldMatLoc, 1, GL_FALSE, NormalMat[0]);

	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, Mdl->VertsBuffer);
	p_glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)FrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(0);
	p_glVertexAttribPointerARB(ShadowsModelAmbientVertNormalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)FrameDesc->NormalsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelAmbientVertNormalLoc);
	p_glVertexAttribPointerARB(ShadowsModelAmbientVert2Loc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)NextFrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelAmbientVert2Loc);
	p_glVertexAttribPointerARB(ShadowsModelAmbientVert2NormalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)NextFrameDesc->NormalsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelAmbientVert2NormalLoc);
	p_glVertexAttribPointerARB(ShadowsModelAmbientTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	p_glEnableVertexAttribArrayARB(ShadowsModelAmbientTexCoordLoc);
	p_glUniform3fARB(ShadowsModelAmbientViewOrigin, vieworg.x, vieworg.y, vieworg.z);
	if (Alpha < 1.0)
	{
		p_glUniform1fARB(ShadowsModelAmbientAlphaLoc, Alpha);
	}
	else
	{
		p_glUniform1fARB(ShadowsModelAmbientAlphaLoc, 1.0);
	}

	p_glUniform1iARB(ShadowsModelAmbientAllowTransparency, GL_FALSE);

	glEnable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);
	glAlphaFunc(GL_GREATER, 0.0);
	glEnable(GL_BLEND);

	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Mdl->IndexBuffer);
	if (Alpha < 1.0 && !ForceDepth)
	{
		glDepthMask(GL_FALSE);
	}
	p_glDrawRangeElementsEXT(GL_TRIANGLES, 0, Mdl->STVerts.Num() - 1, Mdl->Tris.Num() * 3, GL_UNSIGNED_SHORT, 0);
	if (Alpha < 1.0 && !ForceDepth)
	{
		glDepthMask(GL_TRUE);
	}
	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	p_glDisableVertexAttribArrayARB(0);
	p_glDisableVertexAttribArrayARB(ShadowsModelAmbientVertNormalLoc);
	p_glDisableVertexAttribArrayARB(ShadowsModelAmbientVert2Loc);
	p_glDisableVertexAttribArrayARB(ShadowsModelAmbientVert2NormalLoc);
	p_glDisableVertexAttribArrayARB(ShadowsModelAmbientTexCoordLoc);
	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glDisable(GL_BLEND);
	glAlphaFunc(GL_GREATER, 0.333);
	glShadeModel(GL_FLAT);
	glDisable(GL_ALPHA_TEST);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawAliasModelTextures
//
//==========================================================================

void VOpenGLDrawer::DrawAliasModelTextures(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, VMeshModel* Mdl, int frame, int nextframe,
	VTexture* Skin, VTextureTranslation* Trans, int CMap, float Alpha, float Inter,
	bool Interpolate, bool ForceDepth, bool AllowTransparency)
{
	guard(VOpenGLDrawer::DrawAliasModelTextures);
	UploadModel(Mdl);
	VMeshFrame* FrameDesc = &Mdl->Frames[frame];
	VMeshFrame* NextFrameDesc = &Mdl->Frames[nextframe];

	SetPic(Skin, Trans, CMap);

	VMatrix4 RotationMatrix;
	AliasSetUpTransform(origin, angles, Offset, Scale, RotationMatrix);
	VMatrix4 normalmatrix;
	AliasSetUpNormalTransform(angles, Scale, normalmatrix);
	float NormalMat[3][3];
	NormalMat[0][0] = normalmatrix[0][0];
	NormalMat[0][1] = normalmatrix[0][1];
	NormalMat[0][2] = normalmatrix[0][2];
	NormalMat[1][0] = normalmatrix[1][0];
	NormalMat[1][1] = normalmatrix[1][1];
	NormalMat[1][2] = normalmatrix[1][2];
	NormalMat[2][0] = normalmatrix[2][0];
	NormalMat[2][1] = normalmatrix[2][1];
	NormalMat[2][2] = normalmatrix[2][2];

	p_glUseProgramObjectARB(ShadowsModelTexturesProgram);
	p_glUniform1iARB(ShadowsModelTexturesTextureLoc, 0);
	p_glUniform1fARB(ShadowsModelTexturesInterLoc, Inter);
	p_glUniformMatrix4fvARB(ShadowsModelTexturesModelToWorldMatLoc, 1, GL_FALSE, RotationMatrix[0]);
	p_glUniformMatrix3fvARB(ShadowsModelTexturesNormalToWorldMatLoc, 1, GL_FALSE, NormalMat[0]);

	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, Mdl->VertsBuffer);
	p_glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)FrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(0);
	p_glVertexAttribPointerARB(ShadowsModelTexturesVertNormalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)FrameDesc->NormalsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelTexturesVertNormalLoc);
	p_glVertexAttribPointerARB(ShadowsModelTexturesVert2Loc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)NextFrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelTexturesVert2Loc);
	p_glVertexAttribPointerARB(ShadowsModelTexturesVert2NormalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)NextFrameDesc->NormalsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelTexturesVert2NormalLoc);
	p_glVertexAttribPointerARB(ShadowsModelTexturesTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	p_glEnableVertexAttribArrayARB(ShadowsModelTexturesTexCoordLoc);
	p_glUniform3fARB(ShadowsModelTexturesViewOrigin, vieworg.x, vieworg.y, vieworg.z);

	if (Alpha < 1.0)
	{
		p_glUniform1fARB(ShadowsModelTexturesAlphaLoc, Alpha);
	}
	else
	{
		p_glUniform1fARB(ShadowsModelTexturesAlphaLoc, 1.0);
	}

	if (AllowTransparency)
	{
		p_glUniform1iARB(ShadowsModelTexturesAllowTransparency, GL_TRUE);
	}
	else
	{
		p_glUniform1iARB(ShadowsModelTexturesAllowTransparency, GL_FALSE);
	}
	glEnable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);
	glAlphaFunc(GL_GREATER, 0.0);

	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Mdl->IndexBuffer);
	glDepthMask(GL_FALSE);
	p_glDrawRangeElementsEXT(GL_TRIANGLES, 0, Mdl->STVerts.Num() - 1, Mdl->Tris.Num() * 3, GL_UNSIGNED_SHORT, 0);
	glDepthMask(GL_TRUE);
	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	p_glDisableVertexAttribArrayARB(0);
	p_glDisableVertexAttribArrayARB(ShadowsModelLightVertNormalLoc);
	p_glDisableVertexAttribArrayARB(ShadowsModelTexturesVert2Loc);
	p_glDisableVertexAttribArrayARB(ShadowsModelLightVert2NormalLoc);
	p_glDisableVertexAttribArrayARB(ShadowsModelTexturesTexCoordLoc);
	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glShadeModel(GL_FLAT);
	glAlphaFunc(GL_GREATER, 0.333);
	glDisable(GL_ALPHA_TEST);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::BeginModelsLightPass
//
//==========================================================================

void VOpenGLDrawer::BeginModelsLightPass(TVec& LightPos, float Radius, vuint32 Colour)
{
	guard(VOpenGLDrawer::BeginModelsLightPass);
	p_glUseProgramObjectARB(ShadowsModelLightProgram);
	p_glUniform1iARB(ShadowsModelLightTextureLoc, 0);
	p_glUniform3fARB(ShadowsModelLightLightPosLoc, LightPos.x, LightPos.y, LightPos.z);
	p_glUniform1fARB(ShadowsModelLightLightRadiusLoc, Radius);
	p_glUniform3fARB(ShadowsModelLightLightColourLoc,
		((Colour >> 16) & 255) / 255.0,
		((Colour >> 8) & 255) / 255.0,
		(Colour & 255) / 255.0);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawAliasModelLight
//
//==========================================================================

void VOpenGLDrawer::DrawAliasModelLight(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, VMeshModel* Mdl, int frame, int nextframe,
	VTexture* Skin, float Alpha, float Inter, bool Interpolate, bool AllowTransparency)
{
	guard(VOpenGLDrawer::DrawAliasModelLight);
	UploadModel(Mdl);
	VMeshFrame* FrameDesc = &Mdl->Frames[frame];
	VMeshFrame* NextFrameDesc = &Mdl->Frames[nextframe];

	VMatrix4 RotationMatrix;
	AliasSetUpTransform(origin, angles, Offset, Scale, RotationMatrix);
	VMatrix4 normalmatrix;
	AliasSetUpNormalTransform(angles, Scale, normalmatrix);
	float NormalMat[3][3];
	NormalMat[0][0] = normalmatrix[0][0];
	NormalMat[0][1] = normalmatrix[0][1];
	NormalMat[0][2] = normalmatrix[0][2];
	NormalMat[1][0] = normalmatrix[1][0];
	NormalMat[1][1] = normalmatrix[1][1];
	NormalMat[1][2] = normalmatrix[1][2];
	NormalMat[2][0] = normalmatrix[2][0];
	NormalMat[2][1] = normalmatrix[2][1];
	NormalMat[2][2] = normalmatrix[2][2];

	SetPic(Skin, NULL, CM_Default);

	p_glUniform1fARB(ShadowsModelLightInterLoc, Inter);
	p_glUniformMatrix4fvARB(ShadowsModelLightModelToWorldMatLoc, 1, GL_FALSE, RotationMatrix[0]);
	p_glUniformMatrix3fvARB(ShadowsModelLightNormalToWorldMatLoc, 1, GL_FALSE, NormalMat[0]);

	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, Mdl->VertsBuffer);
	p_glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)FrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(0);
	p_glVertexAttribPointerARB(ShadowsModelLightVertNormalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)FrameDesc->NormalsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelLightVertNormalLoc);
	p_glVertexAttribPointerARB(ShadowsModelLightVert2Loc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)NextFrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelLightVert2Loc);
	p_glVertexAttribPointerARB(ShadowsModelLightVert2NormalLoc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)NextFrameDesc->NormalsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelLightVert2NormalLoc);
	p_glVertexAttribPointerARB(ShadowsModelLightTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	p_glEnableVertexAttribArrayARB(ShadowsModelLightTexCoordLoc);

	if (Alpha < 1.0)
	{
		p_glUniform1fARB(ShadowsModelAlphaLoc, Alpha);
	}
	else
	{
		p_glUniform1fARB(ShadowsModelAlphaLoc, 1.0);
	}

	if (AllowTransparency)
	{
		p_glUniform1iARB(ShadowsModelLightAllowTransparency, GL_TRUE);
	}
	else
	{
		p_glUniform1iARB(ShadowsModelLightAllowTransparency, GL_FALSE);
	}
	p_glUniform3fARB(ShadowsModelLightViewOrigin, vieworg.x, vieworg.y, vieworg.z);

	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Mdl->IndexBuffer);
	p_glDrawRangeElementsEXT(GL_TRIANGLES, 0, Mdl->STVerts.Num() - 1, Mdl->Tris.Num() * 3, GL_UNSIGNED_SHORT, 0);
	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	p_glDisableVertexAttribArrayARB(0);
	p_glDisableVertexAttribArrayARB(ShadowsModelLightVertNormalLoc);
	p_glDisableVertexAttribArrayARB(ShadowsModelLightVert2Loc);
	p_glDisableVertexAttribArrayARB(ShadowsModelLightVert2NormalLoc);
	p_glDisableVertexAttribArrayARB(ShadowsModelLightTexCoordLoc);
	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::BeginModelsShadowsPass
//
//==========================================================================

void VOpenGLDrawer::BeginModelsShadowsPass(TVec& LightPos, float LightRadius)
{
	guard(VOpenGLDrawer::BeginModelsShadowsPass);
	p_glUseProgramObjectARB(ShadowsModelShadowProgram);
	p_glUniform3fARB(ShadowsModelShadowLightPosLoc, LightPos.x, LightPos.y, LightPos.z);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawAliasModelShadow
//
//==========================================================================

void VOpenGLDrawer::DrawAliasModelShadow(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, VMeshModel* Mdl, int frame, int nextframe,
	float Inter, bool Interpolate, const TVec& LightPos, float LightRadius)
{
	guard(VOpenGLDrawer::DrawAliasModelShadow);
	UploadModel(Mdl);
	VMeshFrame* FrameDesc = &Mdl->Frames[frame];
	VMeshFrame* NextFrameDesc = &Mdl->Frames[nextframe];

	VMatrix4 RotationMatrix;
	AliasSetUpTransform(origin, angles, Offset, Scale, RotationMatrix);

	VMatrix4 InvRotationMatrix = RotationMatrix.Inverse();
	TVec LocalLightPos = InvRotationMatrix.Transform(LightPos);

	TArray<bool> PlaneSides;
	PlaneSides.SetNum(Mdl->Tris.Num());
	VMeshFrame* PlanesFrame = Inter >= 0.5 ? NextFrameDesc : FrameDesc;
	TPlane* P = PlanesFrame->Planes;
	for (int i = 0; i < Mdl->Tris.Num(); i++, P++)
	{
		// Planes facing to the light
		PlaneSides[i] = DotProduct(LocalLightPos, P->normal) - P->dist > 0.0 && 
						DotProduct(LocalLightPos, P->normal) - P->dist <= LightRadius;
	}

	p_glUniform1fARB(ShadowsModelShadowInterLoc, Inter);
	p_glUniformMatrix4fvARB(ShadowsModelShadowModelToWorldMatLoc, 1, GL_FALSE, RotationMatrix[0]);

	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, Mdl->VertsBuffer);
	p_glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)FrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(0);
	p_glVertexAttribPointerARB(ShadowsModelShadowVert2Loc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)NextFrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelShadowVert2Loc);

	float Shadow_Offset = M_INFINITY;

	glBegin(GL_TRIANGLES);
	p_glVertexAttrib1fARB(ShadowsModelShadowOffsetLoc, 0);
	for (int i = 0; i < Mdl->Tris.Num(); i++)
	{
		if (PlaneSides[i])
		{
			glArrayElement(Mdl->Tris[i].VertIndex[0]);
			glArrayElement(Mdl->Tris[i].VertIndex[1]);
			glArrayElement(Mdl->Tris[i].VertIndex[2]);
		}
	}

	p_glVertexAttrib1fARB(ShadowsModelShadowOffsetLoc, Shadow_Offset);
	for (int i = 0; i < Mdl->Tris.Num(); i++)
	{
		if (PlaneSides[i])
		{
			glArrayElement(Mdl->Tris[i].VertIndex[2]);
			glArrayElement(Mdl->Tris[i].VertIndex[1]);
			glArrayElement(Mdl->Tris[i].VertIndex[0]);
		}
	}
	glEnd();

	for (int i = 0; i < Mdl->Edges.Num(); i++)
	{
		//	Edges with no matching pair are drawn only if corresponding triangle
		// is facing light, other are drawn if facing light changes.
		if ((Mdl->Edges[i].Tri2 == -1 && PlaneSides[Mdl->Edges[i].Tri1]) ||
			(Mdl->Edges[i].Tri2 != -1 && PlaneSides[Mdl->Edges[i].Tri1] != PlaneSides[Mdl->Edges[i].Tri2]))
		{
			int index1 = Mdl->Edges[i].Vert1;
			int index2 = Mdl->Edges[i].Vert2;

			glBegin(GL_TRIANGLE_STRIP);
			if (PlaneSides[Mdl->Edges[i].Tri1])
			{
				outv(1, 0);
				outv(1, Shadow_Offset);
				outv(2, 0);
				outv(2, Shadow_Offset);
			}
			else
			{
				outv(2, 0);
				outv(2, Shadow_Offset);
				outv(1, 0);
				outv(1, Shadow_Offset);
			}
			glEnd();
		}
	}
	p_glUniform3fARB(ShadowsModelShadowViewOrigin, vieworg.x, vieworg.y, vieworg.z);

	p_glDisableVertexAttribArrayARB(0);
	p_glDisableVertexAttribArrayARB(ShadowsModelShadowVert2Loc);
	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawAliasModelFog
//
//==========================================================================

void VOpenGLDrawer::DrawAliasModelFog(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, VMeshModel* Mdl, int frame, int nextframe,
	VTexture* Skin, vuint32 Fade, float Alpha, float Inter, bool Interpolate,
	bool AllowTransparency)
{
	guard(VOpenGLDrawer::DrawAliasModelFog);
	UploadModel(Mdl);
	VMeshFrame* FrameDesc = &Mdl->Frames[frame];
	VMeshFrame* NextFrameDesc = &Mdl->Frames[nextframe];

	SetPic(Skin, NULL, CM_Default);

	VMatrix4 RotationMatrix;
	AliasSetUpTransform(origin, angles, Offset, Scale, RotationMatrix);

	p_glUseProgramObjectARB(ShadowsModelFogProgram);
	p_glUniform1iARB(ShadowsModelFogTextureLoc, 0);
	p_glUniform1fARB(ShadowsModelFogInterLoc, Inter);
	p_glUniformMatrix4fvARB(ShadowsModelFogModelToWorldMatLoc, 1, GL_FALSE, RotationMatrix[0]);
	p_glUniform1iARB(ShadowsModelFogFogTypeLoc, r_fog & 3);
	p_glUniform4fARB(ShadowsModelFogFogColourLoc,
		((Fade >> 16) & 255) / 255.0,
		((Fade >> 8) & 255) / 255.0,
		(Fade & 255) / 255.0, Alpha);
	p_glUniform1fARB(ShadowsModelFogFogDensityLoc, Fade == FADE_LIGHT ? 0.3 : r_fog_density);
	p_glUniform1fARB(ShadowsModelFogFogStartLoc, Fade == FADE_LIGHT ? 1.0 : r_fog_start);
	p_glUniform1fARB(ShadowsModelFogFogEndLoc, Fade == FADE_LIGHT ? 1024.0 * r_fade_factor : r_fog_end);

	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, Mdl->VertsBuffer);
	p_glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)FrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(0);
	p_glVertexAttribPointerARB(ShadowsModelFogVert2Loc, 3, GL_FLOAT, GL_FALSE, 0,
		(void*)NextFrameDesc->VertsOffset);
	p_glEnableVertexAttribArrayARB(ShadowsModelFogVert2Loc);
	p_glVertexAttribPointerARB(ShadowsModelFogTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	p_glEnableVertexAttribArrayARB(ShadowsModelFogTexCoordLoc);
	p_glUniform3fARB(ShadowsModelFogViewOrigin, vieworg.x, vieworg.y, vieworg.z);
	if (Alpha < 1.0)
	{
		p_glUniform1fARB(ShadowsModelFogAlphaLoc, Alpha);
	}
	else
	{
		p_glUniform1fARB(ShadowsModelFogAlphaLoc, 1.0);
	}

	if (AllowTransparency)
	{
		p_glUniform1iARB(ShadowsModelFogAllowTransparency, GL_TRUE);
	}
	else
	{
		p_glUniform1iARB(ShadowsModelFogAllowTransparency, GL_FALSE);
	}
	glEnable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);
	glAlphaFunc(GL_GREATER, 0.0);
	glEnable(GL_BLEND);

	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Mdl->IndexBuffer);
	glDepthMask(GL_FALSE);
	p_glDrawRangeElementsEXT(GL_TRIANGLES, 0, Mdl->STVerts.Num() - 1, Mdl->Tris.Num() * 3, GL_UNSIGNED_SHORT, 0);
	glDepthMask(GL_TRUE);
	p_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	p_glDisableVertexAttribArrayARB(0);
	p_glDisableVertexAttribArrayARB(ShadowsModelFogVert2Loc);
	p_glDisableVertexAttribArrayARB(ShadowsModelFogTexCoordLoc);
	p_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glDisable(GL_BLEND);
	glAlphaFunc(GL_GREATER, 0.333);
	glShadeModel(GL_FLAT);
	glDisable(GL_ALPHA_TEST);
	unguard;
}
