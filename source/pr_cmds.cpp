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
//**
//**	Builtins.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

VEntity *SV_SpawnMobj(VClass *Class);
void SV_ForceLightning();
void SV_SetFloorPic(int i, int texture);
void SV_SetCeilPic(int i, int texture);
VClass* SV_FindClassFromEditorId(int Id);
VClass* SV_FindClassFromScriptId(int Id);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

VMessage			*pr_msg;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	PR_PushStr
//
//==========================================================================

void PR_PushStr(const VStr& value)
{
	pr_stackPtr->p = NULL;
	*(VStr*)&pr_stackPtr->p = value;
	pr_stackPtr++;
}

//==========================================================================
//
//	PR_PopStr
//
//==========================================================================

VStr PR_PopStr()
{
	--pr_stackPtr;
	VStr Ret = *(VStr*)&pr_stackPtr->p;
	((VStr*)&pr_stackPtr->p)->Clean();
	return Ret;
}

//**************************************************************************
//
//	Vararg strings
//
//**************************************************************************

static char		vastring[1024];

const char* PF_FormatString()
{
	VStack	params[16];
	int		pi;

	P_GET_INT(count);
	for (pi = count - 1; pi >= 0; pi--)
	{
		params[pi] = *(--pr_stackPtr);
	}
	P_GET_STR(str);

	const char *src = *str;
	char *dst = vastring;
	memset(vastring, 0, sizeof(vastring));
	pi = 0;
	while (*src)
	{
		if (*src == '%')
		{
			src++;
			switch (*src)
			{
			case '%':
				*dst = *src;
				break;

			case 'c':
				strcat(vastring, va("%c", params[pi].i));
				pi++;
				break;

			case 'i':
			case 'd':
				strcat(vastring, va("%d", params[pi].i));
				pi++;
				break;

			case 'x':
				strcat(vastring, va("%x", params[pi].i));
				pi++;
				break;

			case 'f':
				strcat(vastring, va("%f", params[pi].f));
				pi++;
				break;

			case 'n':
				strcat(vastring, **(VName*)&params[pi].i);
				pi++;
				break;

			case 's':
				strcat(vastring, **(VStr*)&params[pi].p);
				((VStr*)&params[pi].p)->Clean();
				pi++;
				break;

			case 'p':
				strcat(vastring, va("%p", params[pi].p));
				pi++;
				break;

			case 'v':
				strcat(vastring, va("(%f,%f,%f)", params[pi].f,
					params[pi + 1].f, params[pi + 2].f));
				pi += 3;
				break;

			default:
				GCon->Logf(NAME_Dev, "PF_FormatString: Unknown format identifier %s", *src);
				src--;
				*dst = *src;
				break;
			}
			dst = vastring + strlen(vastring);
		}
		else
		{
			*dst = *src;
			dst++;
		}
		src++;
	}
	if (pi < count)
	{
		GCon->Log(NAME_Dev, "PF_FormatString: Not all params were used");
	}
	if (pi > count)
	{
		GCon->Log(NAME_Dev, "PF_FormatString: Param count overflow");
	}
	return vastring;
}

#ifdef SERVER

//**************************************************************************
//
//	Print functions
//
//**************************************************************************

//==========================================================================
//
//	PF_bprint
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, bprint)
{
	SV_BroadcastPrintf(PF_FormatString());
}

//**************************************************************************
//
//	Map utilites
//
//**************************************************************************

//==========================================================================
//
//	PF_LineOpenings
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, LineOpenings)
{
	P_GET_VEC(point);
	P_GET_PTR(line_t, linedef);
	RET_PTR(SV_LineOpenings(linedef, point));
}

//==========================================================================
//
//	PF_P_BoxOnLineSide
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, P_BoxOnLineSide)
{
	P_GET_PTR(line_t, ld);
	P_GET_PTR(float, tmbox);
	RET_INT(P_BoxOnLineSide(tmbox, ld));
}

//==========================================================================
//
//  PF_P_BlockThingsIterator
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, P_BlockThingsIterator)
{
	P_GET_NAME(FuncName);
	P_GET_REF(VObject, SelfObj);
	P_GET_INT(y);
	P_GET_INT(x);
	VMethod* func = SelfObj->GetClass()->FindFunctionChecked(FuncName);
	RET_BOOL(SV_BlockThingsIterator(x, y, NULL, SelfObj, func));
}

//==========================================================================
//
//	PF_P_PathTraverse
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, P_PathTraverse)
{
	P_GET_NAME(FuncName);
	P_GET_REF(VObject, SelfObj);
	P_GET_INT(flags);
	P_GET_FLOAT(y2);
	P_GET_FLOAT(x2);
	P_GET_FLOAT(y1);
	P_GET_FLOAT(x1);
	VMethod* func = SelfObj->GetClass()->FindFunctionChecked(FuncName);
	RET_BOOL(SV_PathTraverse(x1, y1, x2, y2, flags, NULL, SelfObj, func));
}

//==========================================================================
//
//	PF_FindThingGap
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, FindThingGap)
{
	P_GET_FLOAT(z2);
	P_GET_FLOAT(z1);
	P_GET_VEC(point);
	P_GET_PTR(sec_region_t, gaps);
	RET_PTR(SV_FindThingGap(gaps, point, z1, z2));
}

//==========================================================================
//
//	PF_FindOpening
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, FindOpening)
{
	P_GET_FLOAT(z2);
	P_GET_FLOAT(z1);
	P_GET_PTR(opening_t, gaps);
	RET_PTR(SV_FindOpening(gaps, z1, z2));
}

//==========================================================================
//
//	PF_PointInRegion
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, PointInRegion)
{
	P_GET_VEC(p);
	P_GET_PTR(sector_t, sector);
	RET_PTR(SV_PointInRegion(sector, p));
}

//==========================================================================
//
//	PF_AddExtraFloor
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, AddExtraFloor)
{
	P_GET_PTR(sector_t, dst);
	P_GET_PTR(line_t, line);
	RET_PTR(AddExtraFloor(line, dst));
	sv_signon << (byte)svc_extra_floor
				<< (short)(line - GLevel->Lines)
				<< (short)(dst - GLevel->Sectors);
}

//==========================================================================
//
//	PF_SwapPlanes
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SwapPlanes)
{
	P_GET_PTR(sector_t, s);
	SwapPlanes(s);
	sv_signon << (byte)svc_swap_planes << (short)(s - GLevel->Sectors);
}

//==========================================================================
//
//	PF_MapBlock
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, MapBlock)
{
	P_GET_FLOAT(x);
	RET_INT(MapBlock(x));
}

//==========================================================================
//
//	PF_P_ChangeSector
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, P_ChangeSector)
{
	P_GET_INT(crunch);
	P_GET_PTR(sector_t, sec);
	RET_BOOL(P_ChangeSector(sec, crunch));
}

//**************************************************************************
//
//	Mobj utilites
//
//**************************************************************************

//==========================================================================
//
//  PF_NewMobjThiker
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, NewMobjThinker)
{
	P_GET_PTR(VClass, Class);
	RET_REF(SV_SpawnMobj(Class));
}

//==========================================================================
//
//	PF_NextMobj
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, NextMobj)
{
	P_GET_REF(VThinker, th);
	if (!th)
	{
		th = GLevel->ThinkerHead;
	}
	else
	{
		th = th->Next;
	}
	while (th)
	{
		if (th->IsA(VEntity::StaticClass()))
		{
			RET_REF(th);
			return;
		}
		th = th->Next;
	}
	RET_REF(0);
}

//**************************************************************************
//
//	Special thinker utilites
//
//**************************************************************************

//==========================================================================
//
//  PF_NewSpecialThinker
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, NewSpecialThinker)
{
	VThinker	*spec;

	P_GET_PTR(VClass, Class);
	spec = (VThinker*)VObject::StaticSpawnObject(Class);
	GLevel->AddThinker(spec);
	RET_REF(spec);
}

//==========================================================================
//
//  PF_RemoveSpecialThinker
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, RemoveSpecialThinker)
{
	P_GET_REF(VThinker, spec);
	spec->SetFlags(_OF_DelayedDestroy);
}

//==========================================================================
//
//  PF_P_ChangeSwitchTexture
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, P_ChangeSwitchTexture)
{
	P_GET_NAME(DefaultSound);
	P_GET_BOOL(useAgain);
	P_GET_PTR(line_t, line);
	P_ChangeSwitchTexture(line, useAgain, DefaultSound);
}

//==========================================================================
//
//	PF_NextThinker
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, NextThinker)
{
	P_GET_PTR(VClass, Class);
	P_GET_REF(VThinker, th);
	if (!th)
	{
		th = GLevel->ThinkerHead;
	}
	else
	{
		th = th->Next;
	}
	while (th)
	{
		if (th->IsA(Class))
		{
			RET_REF(th);
			return;
		}
		th = th->Next;
	}
	RET_REF(0);
}

//**************************************************************************
//
//	Polyobj functons
//
//**************************************************************************

//==========================================================================
//
//	PF_SpawnPolyobj
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SpawnPolyobj)
{
	P_GET_INT(flags);
	P_GET_INT(tag);
	P_GET_FLOAT(y);
	P_GET_FLOAT(x);
	PO_SpawnPolyobj(x, y, tag, flags);
}

//==========================================================================
//
//	PF_AddAnchorPoint
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, AddAnchorPoint)
{
	P_GET_INT(tag);
	P_GET_FLOAT(y);
	P_GET_FLOAT(x);
	PO_AddAnchorPoint(x, y, tag);
}

//==========================================================================
//
//	PF_GetPolyobj
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, GetPolyobj)
{
	P_GET_INT(polyNum);
	RET_PTR(PO_GetPolyobj(polyNum));
}

//==========================================================================
//
//	PF_GetPolyobjMirror
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, GetPolyobjMirror)
{
	P_GET_INT(polyNum);
	RET_INT(PO_GetPolyobjMirror(polyNum));
}

//==========================================================================
//
//  PF_PO_MovePolyobj
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, PO_MovePolyobj)
{
	P_GET_FLOAT(y);
	P_GET_FLOAT(x);
	P_GET_INT(num);
	RET_BOOL(PO_MovePolyobj(num, x, y));
}

//==========================================================================
//
//	PF_PO_RotatePolyobj
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, PO_RotatePolyobj)
{
	P_GET_FLOAT(angle);
	P_GET_INT(num);
	RET_BOOL(PO_RotatePolyobj(num, angle));
}

//**************************************************************************
//
//	ACS functions
//
//**************************************************************************

//==========================================================================
//
//  PF_StartACS
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, StartACS)
{
	P_GET_BOOL(WantResult);
	P_GET_BOOL(Always);
	P_GET_INT(side);
	P_GET_PTR(line_t, line);
	P_GET_REF(VEntity, activator);
	P_GET_INT(arg3);
	P_GET_INT(arg2);
	P_GET_INT(arg1);
	P_GET_INT(map);
	P_GET_INT(num);
	RET_BOOL(P_StartACS(num, map, arg1, arg2, arg3, activator, line, side,
		Always, WantResult));
}

//==========================================================================
//
//  PF_SuspendACS
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SuspendACS)
{
	P_GET_INT(map);
	P_GET_INT(number);
	RET_BOOL(P_SuspendACS(number, map));
}

//==========================================================================
//
//  PF_TerminateACS
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, TerminateACS)
{
	P_GET_INT(map);
	P_GET_INT(number);
	RET_BOOL(P_TerminateACS(number, map));
}

//**************************************************************************
//
//  Sound functions
//
//**************************************************************************

//==========================================================================
//
//	PF_StartSoundAtVolume
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, StartSoundAtVolume)
{
	P_GET_INT(vol);
	P_GET_INT(channel);
	P_GET_INT(sound);
	P_GET_REF(VEntity, mobj);
	SV_StartSound(mobj, sound, channel, vol);
}

//==========================================================================
//
//	PF_SectorStartSound
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SectorStartSound)
{
	P_GET_INT(channel);
	P_GET_INT(sound);
	P_GET_PTR(sector_t, sec);
	SV_SectorStartSound(sec, sound, channel, 127);
}

//==========================================================================
//
//	PF_SectorStopSound
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SectorStopSound)
{
	P_GET_INT(channel);
	P_GET_PTR(sector_t, sec);
	SV_SectorStopSound(sec, channel);
}

//==========================================================================
//
//	PF_GetSoundPlayingInfo
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, GetSoundPlayingInfo)
{
	P_GET_INT(id);
	P_GET_REF(VEntity, mobj);
#ifdef CLIENT
	RET_BOOL(GAudio->IsSoundPlaying(mobj->NetID, id));
#else
	RET_BOOL(false);
#endif
}

//==========================================================================
//
//	PF_GetSoundID
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, GetSoundID)
{
	P_GET_NAME(Name);
	RET_INT(GSoundManager->GetSoundID(Name));
}

//==========================================================================
//
//  PF_SetSeqTrans
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetSeqTrans)
{
	P_GET_INT(SeqType);
	P_GET_INT(Num);
	P_GET_NAME(Name);
	GSoundManager->SetSeqTrans(Name, Num, SeqType);
}

//==========================================================================
//
//  PF_GetSeqTrans
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, GetSeqTrans)
{
	P_GET_INT(SeqType);
	P_GET_INT(Num);
	RET_NAME(GSoundManager->GetSeqTrans(Num, SeqType));
}

//==========================================================================
//
//  PF_SectorStartSequence
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SectorStartSequence)
{
	P_GET_NAME(name);
	P_GET_PTR(sector_t, sec);
	SV_SectorStartSequence(sec, *name);
}

//==========================================================================
//
//  PF_SectorStopSequence
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SectorStopSequence)
{
	P_GET_PTR(sector_t, sec);
	SV_SectorStopSequence(sec);
}

//==========================================================================
//
//  PF_PolyobjStartSequence
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, PolyobjStartSequence)
{
	P_GET_NAME(name);
	P_GET_PTR(polyobj_t, poly);
	SV_PolyobjStartSequence(poly, *name);
}

//==========================================================================
//
//  PF_PolyobjStopSequence
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, PolyobjStopSequence)
{
	P_GET_PTR(polyobj_t, poly);
	SV_PolyobjStopSequence(poly);
}

//==========================================================================
//
//  PF_G_ExitLevel
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, G_ExitLevel)
{
	P_GET_INT(Position);
	G_ExitLevel(Position);
}

//==========================================================================
//
//  PF_G_SecretExitLevel
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, G_SecretExitLevel)
{
	P_GET_INT(Position);
	G_SecretExitLevel(Position);
}

//==========================================================================
//
//  PF_G_Completed
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, G_Completed)
{
	P_GET_INT(SaveAngle);
	P_GET_INT(pos);
	P_GET_INT(map);
	G_Completed(map, pos, SaveAngle);
}

//==========================================================================
//
//	PF_P_GetThingFloorType
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, TerrainType)
{
	P_GET_INT(pic);
	RET_INT(SV_TerrainType(pic));
}

//==========================================================================
//
//	PF_SB_Start
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SB_Start)
{
#ifdef CLIENT
//	SB_Start();
#endif
}

//==========================================================================
//
//  PF_P_ForceLightning
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, P_ForceLightning)
{
	SV_ForceLightning();
}

//==========================================================================
//
//	PF_SetFloorPic
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetFloorPic)
{
	P_GET_INT(texture);
	P_GET_PTR(sector_t, sec);
	SV_SetFloorPic(sec - GLevel->Sectors, texture);
}

//==========================================================================
//
//	PF_SetCeilPic
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetCeilPic)
{
	P_GET_INT(texture);
	P_GET_PTR(sector_t, sec);
	SV_SetCeilPic(sec - GLevel->Sectors, texture);
}

//==========================================================================
//
//	PF_SetLineTexture
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetLineTexture)
{
	P_GET_INT(texture);
	P_GET_INT(position);
	P_GET_INT(side);
	SV_SetLineTexture(side, position, texture);
}

//==========================================================================
//
//	PF_SetLineTransluc
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetLineTransluc)
{
	P_GET_INT(trans);
	P_GET_PTR(line_t, line);
	SV_SetLineTransluc(line, trans);
}

//==========================================================================
//
//	PF_SendFloorSlope
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SendFloorSlope)
{
	P_GET_PTR(sector_t, sector);
	sector->floor.CalcBits();
	sv_signon << (byte)svc_sec_floor_plane
			<< (word)(sector - GLevel->Sectors)
			<< sector->floor.normal.x
			<< sector->floor.normal.y
			<< sector->floor.normal.z
			<< sector->floor.dist;
}

//==========================================================================
//
//	PF_SendCeilingSlope
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SendCeilingSlope)
{
	P_GET_PTR(sector_t, sector);
	sector->ceiling.CalcBits();
	sv_signon << (byte)svc_sec_ceil_plane
			<< (word)(sector - GLevel->Sectors)
			<< sector->ceiling.normal.x
			<< sector->ceiling.normal.y
			<< sector->ceiling.normal.z
			<< sector->ceiling.dist;
}

//==========================================================================
//
//	PF_SetSecLightColor
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetSecLightColor)
{
	P_GET_INT(Col);
	P_GET_PTR(sector_t, sector);
	sector->params.LightColor = Col;
	sv_signon << (byte)svc_sec_light_color
			<< (word)(sector - GLevel->Sectors)
			<< (byte)(Col >> 16)
			<< (byte)(Col >> 8)
			<< (byte)Col;
}

//==========================================================================
//
//	PF_SetFloorLightSector
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetFloorLightSector)
{
	P_GET_PTR(sector_t, SrcSector);
	P_GET_PTR(sector_t, Sector);
	Sector->floor.LightSourceSector = SrcSector - GLevel->Sectors;
	sv_signon << (byte)svc_set_floor_light_sec
			<< (word)(Sector - GLevel->Sectors)
			<< (word)(SrcSector - GLevel->Sectors);
}

//==========================================================================
//
//	PF_SetCeilingLightSector
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetCeilingLightSector)
{
	P_GET_PTR(sector_t, SrcSector);
	P_GET_PTR(sector_t, Sector);
	Sector->ceiling.LightSourceSector = SrcSector - GLevel->Sectors;
	sv_signon << (byte)svc_set_ceil_light_sec
			<< (word)(Sector - GLevel->Sectors)
			<< (word)(SrcSector - GLevel->Sectors);
}

//==========================================================================
//
//	PF_SetHeightSector
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetHeightSector)
{
	P_GET_INT(Flags);
	P_GET_PTR(sector_t, SrcSector);
	P_GET_PTR(sector_t, Sector);
	sv_signon << (byte)svc_set_heightsec
			<< (word)(Sector - GLevel->Sectors)
			<< (word)(SrcSector - GLevel->Sectors)
			<< (byte)Flags;
}

//==========================================================================
//
//	PF_FindModel
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, FindModel)
{
	P_GET_STR(name);
	RET_INT(SV_FindModel(*name));
}

//==========================================================================
//
//	PF_GetModelIndex
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, GetModelIndex)
{
	P_GET_NAME(Name);
	RET_INT(SV_GetModelIndex(Name));
}

//==========================================================================
//
//	PF_FindSkin
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, FindSkin)
{
	P_GET_STR(name);
	RET_INT(SV_FindSkin(*name));
}

//==========================================================================
//
//	PF_FindClassFromEditorId
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, FindClassFromEditorId)
{
	P_GET_INT(Id);
	RET_PTR(SV_FindClassFromEditorId(Id));
}

//==========================================================================
//
//	PF_FindClassFromScriptId
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, FindClassFromScriptId)
{
	P_GET_INT(Id);
	RET_PTR(SV_FindClassFromScriptId(Id));
}

//==========================================================================
//
//	PF_ChangeMusic
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, ChangeMusic)
{
	P_GET_STR(SongName);
	SV_ChangeMusic(*SongName);
}

//==========================================================================
//
//  PF_FindSectorFromTag
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, FindSectorFromTag)
{
	P_GET_INT(start);
	P_GET_INT(tag);
	int Ret = -1;
	for (int i = start + 1; i < GLevel->NumSectors; i++)
		if (GLevel->Sectors[i].tag == tag)
		{
			Ret = i;
			break;
		}
	RET_INT(Ret);
}

#endif
#ifdef CLIENT

//**************************************************************************
//
//	Graphics
//
//**************************************************************************

//==========================================================================
//
//	PF_SetVirtualScreen
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, SetVirtualScreen)
{
	P_GET_INT(Height);
	P_GET_INT(Width);
	SCR_SetVirtualScreen(Width, Height);
}

//==========================================================================
//
//	PF_R_RegisterPic
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_RegisterPic)
{
	P_GET_NAME(name);
	RET_INT(GTextureManager.AddPatch(name, TEXTYPE_Pic));
}

//==========================================================================
//
//	PF_R_RegisterPicPal
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_RegisterPicPal)
{
	P_GET_NAME(palname);
	P_GET_NAME(name);
	RET_INT(GTextureManager.AddRawWithPal(name, palname));
}

//==========================================================================
//
//	PF_R_GetPicInfo
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_GetPicInfo)
{
	P_GET_PTR(picinfo_t, info);
	P_GET_INT(handle);
	GTextureManager.GetTextureInfo(handle, info);
}

//==========================================================================
//
//	PF_R_DrawPic
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_DrawPic)
{
	P_GET_INT(handle);
	P_GET_INT(y);
	P_GET_INT(x);
	R_DrawPic(x, y, handle);
}

//==========================================================================
//
//	PF_R_DrawPic2
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_DrawPic2)
{
	P_GET_INT(trans);
	P_GET_INT(handle);
	P_GET_INT(y);
	P_GET_INT(x);
	R_DrawPic(x, y, handle, trans);
}

//==========================================================================
//
//	PF_R_DrawShadowedPic
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_DrawShadowedPic)
{
	P_GET_INT(handle);
	P_GET_INT(y);
	P_GET_INT(x);
	R_DrawShadowedPic(x, y, handle);
}

//==========================================================================
//
//	PF_R_InstallSprite
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_InstallSprite)
{
	P_GET_INT(index);
	P_GET_STR(name);
	R_InstallSprite(*name, index);
}

//==========================================================================
//
//	PF_R_DrawSpritePatch
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_DrawSpritePatch)
{
	P_GET_INT(translation);
	P_GET_INT(rot);
	P_GET_INT(frame);
	P_GET_INT(sprite);
	P_GET_INT(y);
	P_GET_INT(x);
	R_DrawSpritePatch(x, y, sprite, frame, rot, translation);
}

//==========================================================================
//
//	PF_InstallModel
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, InstallModel)
{
	P_GET_STR(name);
	if (FL_FindFile(name))
	{
		RET_PTR(Mod_FindName(*name));
	}
	else
	{
		RET_PTR(0);
	}
}

//==========================================================================
//
//	PF_R_DrawModelFrame
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_DrawModelFrame)
{
	P_GET_STR(skin);
	P_GET_INT(frame);
	P_GET_PTR(VModel, model);
	P_GET_FLOAT(angle);
	P_GET_VEC(origin);
	R_DrawModelFrame(origin, angle, model, frame, *skin);
}

//==========================================================================
//
//	PF_R_FillRectWithFlat
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_FillRectWithFlat)
{
	P_GET_NAME(name);
	P_GET_INT(height);
	P_GET_INT(width);
	P_GET_INT(y);
	P_GET_INT(x);
	R_FillRectWithFlat(x, y, width, height, *name);
}

//==========================================================================
//
//	PF_R_ShadeRect
//
//==========================================================================

void R_ShadeRect(int x, int y, int width, int height, int shade);

IMPLEMENT_FUNCTION(VObject, R_ShadeRect)
{
	P_GET_INT(shade);
	P_GET_INT(height);
	P_GET_INT(width);
	P_GET_INT(y);
	P_GET_INT(x);
	R_ShadeRect(x, y, width, height, shade);
}

//==========================================================================
//
//	PF_R_FillRect
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, R_FillRect)
{
	P_GET_INT(coulor);
	P_GET_INT(height);
	P_GET_INT(width);
	P_GET_INT(y);
	P_GET_INT(x);
	Drawer->FillRect(x * fScaleX, y * fScaleY, (x + width) * fScaleX,
		(y + height) * fScaleY, coulor);
}

//**************************************************************************
//
//	Text
//
//**************************************************************************

//==========================================================================
//
//	PF_T_SetFont
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, T_SetFont)
{
	P_GET_INT(font);
	T_SetFont((font_e)font);
}

//==========================================================================
//
//	PF_T_SetAlign
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, T_SetAlign)
{
	P_GET_INT(valign);
	P_GET_INT(halign);
	T_SetAlign((halign_e)halign, (valign_e)valign);
}

//==========================================================================
//
//	PF_T_SetDist
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, T_SetDist)
{
	P_GET_INT(vdist);
	P_GET_INT(hdist);
	T_SetDist(hdist, vdist);
}

//==========================================================================
//
//	PF_T_SetShadow
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, T_SetShadow)
{
	P_GET_BOOL(state);
	T_SetShadow(state);
}

//==========================================================================
//
//	PF_T_TextWidth
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, T_TextWidth)
{
	P_GET_STR(text);
	RET_INT(T_TextWidth(*text));
}

//==========================================================================
//
//	PF_T_TextHeight
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, T_TextHeight)
{
	P_GET_STR(text);
	RET_INT(T_TextHeight(*text));
}

//==========================================================================
//
//	PF_T_DrawText
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, T_DrawText)
{
	P_GET_STR(txt);
	P_GET_INT(y);
	P_GET_INT(x);
	T_DrawText(x, y, *txt);
}

//==========================================================================
//
//	PF_T_DrawTextW
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, T_DrawTextW)
{
	P_GET_INT(w);
	P_GET_STR(txt);
	P_GET_INT(y);
	P_GET_INT(x);
	T_DrawTextW(x, y, *txt, w);
}

//**************************************************************************
//
//	Client side sound
//
//**************************************************************************

//==========================================================================
//
//	PF_LocalSound
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, LocalSound)
{
	P_GET_NAME(name);
	GAudio->PlaySound(GSoundManager->GetSoundID(name), TVec(0, 0, 0),
		TVec(0, 0, 0), 0, 0, 1);
}

//==========================================================================
//
//	PF_IsLocalSoundPlaying
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, IsLocalSoundPlaying)
{
	P_GET_NAME(name);
	RET_BOOL(GAudio->IsSoundPlaying(0, GSoundManager->GetSoundID(name)));
}

//==========================================================================
//
//	PF_StopLocalSounds
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, StopLocalSounds)
{
	GAudio->StopSound(0, 0);
}

//==========================================================================
//
//	PF_InputLine_SetValue
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, TranslateKey)
{
	P_GET_INT(ch);
	RET_STR(VStr((char)GInput->TranslateKey(ch)));
}

//==========================================================================
//
//	Temporary menu stuff
//
//==========================================================================

char* P_GetMapName(int map);
VName P_GetMapLumpName(int map);
VName P_TranslateMap(int map);

IMPLEMENT_FUNCTION(VObject, P_GetMapName)
{
	P_GET_INT(map);
	RET_STR(P_GetMapName(map));
}

IMPLEMENT_FUNCTION(VObject, P_GetMapLumpName)
{
	P_GET_INT(map);
	RET_NAME(P_GetMapLumpName(map));
}

IMPLEMENT_FUNCTION(VObject, P_TranslateMap)
{
	P_GET_INT(map);
	RET_NAME(P_TranslateMap(map));
}

IMPLEMENT_FUNCTION(VObject, KeyNameForNum)
{
	P_GET_INT(keynum);
	RET_STR(GInput->KeyNameForNum(keynum));
}

IMPLEMENT_FUNCTION(VObject, IN_GetBindingKeys)
{
	P_GET_PTR(int, key2);
	P_GET_PTR(int, key1);
	P_GET_STR(name);
	GInput->GetBindingKeys(name, *key1, *key2);
}

IMPLEMENT_FUNCTION(VObject, IN_SetBinding)
{
	P_GET_STR(onup);
	P_GET_STR(ondown);
	P_GET_INT(keynum);
	GInput->SetBinding(keynum, ondown, onup);
}

IMPLEMENT_FUNCTION(VObject, SV_GetSaveString)
{
	P_GET_PTR(VStr, buf);
	P_GET_INT(i);
#ifdef SERVER
	RET_INT(SV_GetSaveString(i, buf));
#else
	RET_INT(0);
#endif
}

IMPLEMENT_FUNCTION(VObject, GetSlist)
{
	RET_PTR(GNet->GetSlist());
}

VStr LoadTextLump(VName name);

IMPLEMENT_FUNCTION(VObject, LoadTextLump)
{
	P_GET_NAME(name);
	RET_STR(LoadTextLump(name));
}

IMPLEMENT_FUNCTION(VObject, AllocDlight)
{
	P_GET_INT(key);
	RET_PTR(CL_AllocDlight(key));
}

IMPLEMENT_FUNCTION(VObject, NewParticle)
{
	RET_PTR(R_NewParticle());
}

IMPLEMENT_FUNCTION(VObject, StartSearch)
{
	GNet->StartSearch();
}

IMPLEMENT_FUNCTION(VObject, T_DrawCursor)
{
	T_DrawCursor();
}

#endif

void PR_MSG_Select(int msgtype)
{
	switch (msgtype)
	{
#ifdef SERVER
		case MSG_SV_DATAGRAM:
			pr_msg = &sv_datagram;
			break;
		case MSG_SV_RELIABLE:
			pr_msg = &sv_reliable;
			break;
		case MSG_SV_SIGNON:
			pr_msg = &sv_signon;
			break;
#endif
#ifdef CLIENT
		case MSG_CL_MESSAGE:
			pr_msg = &cls.message;
			break;
#endif
	}
}
