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
//**
//**	Builtins.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

#define PF(name)			static void PF_##name(void)
#define PF_M(cname, name)	static void PF_##cname##__##name(void)
#define _(name)				{#name, PF_##name, VObject::StaticClass()}
#define __(name)			{#name, name, VObject::StaticClass()}

// TYPES -------------------------------------------------------------------

enum
{
	MSG_SV_DATAGRAM,
	MSG_SV_RELIABLE,
	MSG_SV_SIGNON,
	MSG_SV_CLIENT,
	MSG_CL_MESSAGE
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

VEntity *SV_SpawnMobj(VClass *Class);
void SV_ForceLightning(void);
void SV_SetFloorPic(int i, int texture);
void SV_SetCeilPic(int i, int texture);
VClass* SV_FindClassFromEditorId(int Id);
VClass* SV_FindClassFromScriptId(int Id);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VMessage			*pr_msg;

// CODE --------------------------------------------------------------------

//**************************************************************************
//
//	Stack routines
//
//**************************************************************************

//==========================================================================
//
//	Push
//
//==========================================================================

static void Push(int value)
{
	*(pr_stackPtr++) = value;
}

//==========================================================================
//
// Pop
//
//==========================================================================

static int Pop(void)
{
	return *(--pr_stackPtr);
}

//==========================================================================
//
//	Pushf
//
//==========================================================================

static void Pushf(float value)
{
	*((float*)pr_stackPtr++) = value;
}

//==========================================================================
//
//	Popf
//
//==========================================================================

static float Popf(void)
{
	return *((float*)--pr_stackPtr);
}

//==========================================================================
//
//	Pushv
//
//==========================================================================

static void Pushv(const TVec &v)
{
	Pushf(v.x);
	Pushf(v.y);
	Pushf(v.z);
}

//==========================================================================
//
//	Popv
//
//==========================================================================

static TVec Popv(void)
{
	TVec v;
	v.z = Popf();
	v.y = Popf();
	v.x = Popf();
	return v;
}

//==========================================================================
//
//	Popf
//
//==========================================================================

static VName PopName()
{
	return *((VName*)--pr_stackPtr);
}

//**************************************************************************
//
//	Vararg strings
//
//**************************************************************************

static char		vastring[1024];

static char *PF_FormatString(void)
{
	int		count;
	int		params[16];
	char*	str;
	int		pi;

	count = Pop();
	for (pi = count - 1; pi >= 0; pi--)
	{
		params[pi] = Pop();
	}
	str = (char*)Pop();

	char *src = str;
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

			 case 'i':
			 case 'd':
				strcat(vastring, va("%d", params[pi]));
				pi++;
				break;

			 case 'x':
				strcat(vastring, va("%x", params[pi]));
				pi++;
				break;

			 case 'f':
				strcat(vastring, va("%f", ((float*)params)[pi]));
				pi++;
				break;

			 case 'n':
				strcat(vastring, *((VName*)params)[pi]);
				pi++;
				break;

			 case 's':
				strcat(vastring, (char*)params[pi]);
				pi++;
				break;

			 case 'p':
				strcat(vastring, va("%p", ((void**)params)[pi]));
				pi++;
				break;

			 case 'v':
				strcat(vastring, va("(%f,%f,%f)", ((float*)params)[pi],
					((float*)params)[pi + 1], ((float*)params)[pi + 2]));
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

//**************************************************************************
//
//  Error functions
//
//**************************************************************************

//==========================================================================
//
//	PF_Error
//
//==========================================================================

PF(Error)
{
    Host_Error(PF_FormatString());
}

//==========================================================================
//
//	PF_FatalError
//
//==========================================================================

PF(FatalError)
{
    Sys_Error(PF_FormatString());
}

//**************************************************************************
//
//	Cvar functions
//
//**************************************************************************

//==========================================================================
//
//	PF_CreateCvar
//
//==========================================================================

PF(CreateCvar)
{
	int name;
	int def;
	int flags;

	flags = Pop();
	def = Pop();
	name = Pop();
	new VCvar((char*)name, (char*)def, flags);
}

//==========================================================================
//
//  PF_GetCvar
//
//==========================================================================

PF(GetCvar)
{
	int		name;

	name = Pop();
	Push(VCvar::GetInt((char*)name));
}

//==========================================================================
//
//  PF_SetCvar
//
//==========================================================================

PF(SetCvar)
{
	int		name;
	int		value;

    value = Pop();
    name = Pop();
    VCvar::Set((char*)name, value);
}

//==========================================================================
//
//  PF_GetCvarF
//
//==========================================================================

PF(GetCvarF)
{
	int		name;

	name = Pop();
	Pushf(VCvar::GetFloat((char*)name));
}

//==========================================================================
//
//  PF_SetCvarF
//
//==========================================================================

PF(SetCvarF)
{
	int		name;
	float	value;

    value = Popf();
    name = Pop();
    VCvar::Set((char*)name, value);
}

//==========================================================================
//
//  PF_GetCvarS
//
//==========================================================================

PF(GetCvarS)
{
	int		name;

    name = Pop();
    Push((int)VCvar::GetCharp((char*)name));
}

//==========================================================================
//
//  PF_SetCvarS
//
//==========================================================================

PF(SetCvarS)
{
	int		name;
	int		value;

    value = Pop();
    name = Pop();
    VCvar::Set((char*)name, (char*)value);
}

//**************************************************************************
//
//	Math functions
//
//**************************************************************************

//==========================================================================
//
//	PF_AngleMod360
//
//==========================================================================

PF(AngleMod360)
{
	float an;

	an = Popf();
	Pushf(AngleMod(an));
}

//==========================================================================
//
//	PF_AngleMod180
//
//==========================================================================

PF(AngleMod180)
{
	float an;

	an = Popf();
	Pushf(AngleMod180(an));
}

//==========================================================================
//
//	PF_abs
//
//==========================================================================

PF(abs)
{
	int i;

    i = Pop();
    Push(abs(i));
}

//==========================================================================
//
//	PF_fabs
//
//==========================================================================

PF(fabs)
{
	float i;

    i = Popf();
    Pushf(fabs(i));
}

//==========================================================================
//
//	PF_sin
//
//==========================================================================

PF(sin)
{
	float an;

    an = Popf();
    Pushf(msin(an));
}

//==========================================================================
//
//	PF_cos
//
//==========================================================================

PF(cos)
{
	float an;

	an = Popf();
    Pushf(mcos(an));
}

//==========================================================================
//
//	PF_tan
//
//==========================================================================

PF(tan)
{
	float an;

	an = Popf();
    Pushf(mtan(an));
}

//==========================================================================
//
//	PF_atan
//
//==========================================================================

PF(atan)
{
	float	slope;

	slope = Popf();
    Pushf(RAD2DEG(atan(slope)));
}

//==========================================================================
//
//	PF_atan2
//
//==========================================================================

PF(atan2)
{
	float	y;
	float	x;

	x = Popf();
	y = Popf();
    Pushf(matan(y, x));
}

//==========================================================================
//
//	PF_Length
//
//==========================================================================

PF(Length)
{
	TVec		vec;

	vec = Popv();
	Pushf(Length(vec));
}

//==========================================================================
//
//	PF_Normalise
//
//==========================================================================

PF(Normalise)
{
	TVec		vec;

	vec = Popv();
	Pushv(Normalise(vec));
}

//==========================================================================
//
//	PF_DotProduct
//
//==========================================================================

PF(DotProduct)
{
	TVec	v1;
	TVec	v2;

	v2 = Popv();
	v1 = Popv();
	Pushf(DotProduct(v1, v2));
}

//==========================================================================
//
//	PF_CrossProduct
//
//==========================================================================

PF(CrossProduct)
{
	TVec	v1;
	TVec	v2;

	v2 = Popv();
	v1 = Popv();
	Pushv(CrossProduct(v1, v2));
}

//==========================================================================
//
//	PF_AngleVectors
//
//==========================================================================

PF(AngleVectors)
{
	TAVec		*angles;
	TVec		*vforward;
	TVec		*vright;
	TVec		*vup;

	vup = (TVec*)Pop();
	vright = (TVec*)Pop();
	vforward = (TVec*)Pop();
	angles = (TAVec*)Pop();
	AngleVectors(*angles, *vforward, *vright, *vup);
}

//==========================================================================
//
//	PF_AngleVector
//
//==========================================================================

PF(AngleVector)
{
	TAVec		*angles;
	TVec		*vec;

	vec = (TVec*)Pop();
	angles = (TAVec*)Pop();
	AngleVector(*angles, *vec);
}

//==========================================================================
//
//	PF_VectorAngles
//
//==========================================================================

PF(VectorAngles)
{
	TVec		*vec;
	TAVec		*angles;

	angles = (TAVec*)Pop();
	vec = (TVec*)Pop();
	VectorAngles(*vec, *angles);
}

//==========================================================================
//
//  PF_GetPlanePointZ
//
//==========================================================================

PF(GetPlanePointZ)
{
	TPlane*		plane;
	TVec		point;

	point = Popv();
	plane = (TPlane*)Pop();
	Pushf(plane->GetPointZ(point));
}

//==========================================================================
//
//  PF_PointOnPlaneSide
//
//==========================================================================

PF(PointOnPlaneSide)
{
	TVec		point;
	TPlane*		plane;

	plane = (TPlane*)Pop();
	point = Popv();
	Push(plane->PointOnSide(point));
}

//==========================================================================
//
//	PF_RotateDirectionVector
//
//==========================================================================

PF(RotateDirectionVector)
{
	TVec	vec;
	TAVec	rot;

	rot.roll = Popf();
	rot.yaw = Popf();
	rot.pitch = Popf();
	vec = Popv();

	TAVec angles;
	TVec out;

	VectorAngles(vec, angles);
	angles.pitch += rot.pitch;
	angles.yaw += rot.yaw;
	angles.roll += rot.roll;
	AngleVector(angles, out);
	Pushv(out);
}

//==========================================================================
//
//  PF_VectorRotateAroundZ
//
//==========================================================================

PF(VectorRotateAroundZ)
{
	TVec*		vec;
	float		angle;

	angle = Popf();
	vec = (TVec*)Pop();

	float dstx = vec->x * mcos(angle) - vec->y * msin(angle);
	float dsty = vec->x * msin(angle) + vec->y * mcos(angle);

	vec->x = dstx;
	vec->y = dsty;
}

//**************************************************************************
//
//	String functions
//
//**************************************************************************

//==========================================================================
//
//	PF_ptrtos
//
//==========================================================================

PF(ptrtos)
{
	//	Nothing to do
}

//==========================================================================
//
//	PF_strgetchar
//
//==========================================================================

PF(strgetchar)
{
	char*	str;
	int		i;

	i = Pop();
	str = (char*)Pop();
	Push(byte(str[i]));
}

//==========================================================================
//
//	PF_strsetchar
//
//==========================================================================

PF(strsetchar)
{
	char*	str;
	int		i;
	int		chr;

	chr = Pop();
	i = Pop();
	str = (char*)Pop();
	str[i] = chr;
}

//==========================================================================
//
//	PF_strlen
//
//==========================================================================

PF(strlen)
{
	int		s;

	s = Pop();
	Push(strlen((char*)s));
}

//==========================================================================
//
//	PF_strcmp
//
//==========================================================================

PF(strcmp)
{
	int		s1;
	int		s2;

	s2 = Pop();
	s1 = Pop();
	Push(strcmp((char*)s1, (char*)s2));
}

//==========================================================================
//
//	PF_stricmp
//
//==========================================================================

PF(stricmp)
{
	int		s1;
	int		s2;

	s2 = Pop();
	s1 = Pop();
	Push(stricmp((char*)s1, (char*)s2));
}

//==========================================================================
//
//	PF_strcpy
//
//==========================================================================

PF(strcpy)
{
	int		s1;
	int		s2;

	s2 = Pop();
	s1 = Pop();
	strcpy((char*)s1, (char*)s2);
}

//==========================================================================
//
//	PF_strclr
//
//==========================================================================

PF(strclr)
{
	char*	s;

	s = (char*)Pop();
	s[0] = 0;
}

//==========================================================================
//
//	PF_strcat
//
//==========================================================================

PF(strcat)
{
	int		s1;
	int		s2;

	s2 = Pop();
	s1 = Pop();
	strcat((char*)s1, (char*)s2);
}

//==========================================================================
//
//	PF_strlwr
//
//==========================================================================

PF(strlwr)
{
	char*	s;

	s = (char*)Pop();
	while (*s)
	{
		*s = tolower(*s);
		s++;
	}
}

//==========================================================================
//
//	PF_strupr
//
//==========================================================================

PF(strupr)
{
	char*	s;

	s = (char*)Pop();
	while (*s)
	{
		*s = toupper(*s);
		s++;
	}
}

//==========================================================================
//
//	PF_sprint
//
//==========================================================================

PF(sprint)
{
	int		dst;

	PF_FormatString();
	dst = Pop();
	strcpy((char*)dst, vastring);
}

//==========================================================================
//
//	PF_va
//
//==========================================================================

PF(va)
{
	Push((int)PF_FormatString());
}

//==========================================================================
//
//	PF_atoi
//
//==========================================================================

PF(atoi)
{
	int		str;

	str = Pop();
	Push(atoi((char*)str));
}

//==========================================================================
//
//	PF_atof
//
//==========================================================================

PF(atof)
{
	int		str;

	str = Pop();
	Pushf(atof((char*)str));
}

//==========================================================================
//
//	PF_StrToName
//
//==========================================================================

PF(StrToName)
{
	int		str;

	str = Pop();
	Push(VName((char*)str).GetIndex());
}

//**************************************************************************
//
//	Random numbers
//
//**************************************************************************

//==========================================================================
//
//	PF_Random
//
//==========================================================================

PF(Random)
{
	Pushf(Random());
}

//==========================================================================
//
//	PF_P_Random
//
//==========================================================================

PF(P_Random)
{
	Push(rand() & 0xff);
}

//**************************************************************************
//
//	Texture utils
//
//**************************************************************************

//==========================================================================
//
//	PF_CheckTextureNumForName
//
//==========================================================================

PF(CheckTextureNumForName)
{
	int		name;

	name = Pop();
	Push(GTextureManager.CheckNumForName(VName((char*)name, VName::AddLower8),
		TEXTYPE_Wall, true, false));
}

//==========================================================================
//
//	PF_TextureNumForName
//
//==========================================================================

PF(TextureNumForName)
{
	int		name;

	name = Pop();
	Push(GTextureManager.NumForName(VName((char*)name, VName::AddLower8),
		TEXTYPE_Wall, true, false));
}

//==========================================================================
//
//	PF_CheckFlatNumForName
//
//==========================================================================

PF(CheckFlatNumForName)
{
	int		name;

	name = Pop();
	Push(GTextureManager.CheckNumForName(VName((char*)name,
		VName::AddLower8), TEXTYPE_Flat, true, false));
}

//==========================================================================
//
//	PF_FlatNumForName
//
//==========================================================================

PF(FlatNumForName)
{
	int		name;

	name = Pop();
	Push(GTextureManager.NumForName(VName((char*)name, VName::AddLower8),
		TEXTYPE_Flat, true, false));
}

//==========================================================================
//
//	PF_TextureHeight
//
//==========================================================================

PF(TextureHeight)
{
	int			pic;

	pic = Pop();
	Pushf(GTextureManager.TextureHeight(pic));
}

//**************************************************************************
//
//	Message IO functions
//
//**************************************************************************

//==========================================================================
//
//	PF_MSG_Select
//
//==========================================================================

PF(MSG_Select)
{
	int			msgtype;

	msgtype = Pop();
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

//==========================================================================
//
//	PF_MSG_SelectClientMsg
//
//==========================================================================

#ifdef SERVER

PF(MSG_SelectClientMsg)
{
	int			msgtype;
	VBasePlayer	*client;

	client = (VBasePlayer*)Pop();
	msgtype = Pop();
	switch (msgtype)
	{
	 case MSG_SV_CLIENT:
		pr_msg = &client->Message;
		break;
	}
}

#endif

//==========================================================================
//
//	PF_MSG_CheckSpace
//
//==========================================================================

PF(MSG_CheckSpace)
{
	int		len;

	len = Pop();
	Push(pr_msg->CheckSpace(len));
}

//==========================================================================
//
//	PF_MSG_WriteByte
//
//==========================================================================

PF(MSG_WriteByte)
{
	int		d;

	d = Pop();
	*pr_msg << (byte)d;
}

//==========================================================================
//
//	PF_MSG_WriteShort
//
//==========================================================================

PF(MSG_WriteShort)
{
	int		d;

	d = Pop();
	*pr_msg << (short)d;
}

//==========================================================================
//
//	PF_MSG_WriteLong
//
//==========================================================================

PF(MSG_WriteLong)
{
	int		d;

	d = Pop();
	*pr_msg << d;
}

//==========================================================================
//
//	PF_MSG_ReadChar
//
//==========================================================================

PF(MSG_ReadChar)
{
	Push((char)net_msg.ReadByte());
}

//==========================================================================
//
//	PF_MSG_ReadByte
//
//==========================================================================

PF(MSG_ReadByte)
{
	Push((byte)net_msg.ReadByte());
}

//==========================================================================
//
//	PF_MSG_ReadShort
//
//==========================================================================

PF(MSG_ReadShort)
{
	Push((short)net_msg.ReadShort());
}

//==========================================================================
//
//	PF_MSG_ReadWord
//
//==========================================================================

PF(MSG_ReadWord)
{
	Push((word)net_msg.ReadShort());
}

//==========================================================================
//
//	PF_MSG_ReadLong
//
//==========================================================================

PF(MSG_ReadLong)
{
	int l;

	net_msg >> l;
	Push(l);
}

//==========================================================================
//
//	PF_SpawnObject
//
//==========================================================================

PF(SpawnObject)
{
	VClass *Class;

	Class = (VClass *)Pop();
	Push((int)VObject::StaticSpawnObject(Class, PU_STRING));
}

//==========================================================================
//
//	PF_FindClass
//
//==========================================================================

PF(FindClass)
{
	VName	Name;

	Name = PopName();
	Push((int)VClass::FindClass(*Name));
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

PF(bprint)
{
	SV_BroadcastPrintf(PF_FormatString());
}

//==========================================================================
//
//	PF_cprint
//
//==========================================================================

PF(cprint)
{
	VBasePlayer*	player;

	PF_FormatString();
    player = (VBasePlayer*)Pop();
	SV_ClientPrintf(player, vastring);
}

//==========================================================================
//
//	PF_centerprint
//
//==========================================================================

PF(centerprint)
{
	VBasePlayer*	player;

	PF_FormatString();
    player = (VBasePlayer*)Pop();
	SV_ClientCenterPrintf(player, vastring);
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

PF(LineOpenings)
{
	line_t	*linedef;
	TVec	point;

	point = Popv();
	linedef = (line_t*)Pop();
	Push((int)SV_LineOpenings(linedef, point));
}

//==========================================================================
//
//	PF_P_BoxOnLineSide
//
//==========================================================================

PF(P_BoxOnLineSide)
{
	float	*tmbox;
	line_t	*ld;

	ld = (line_t*)Pop();
	tmbox = (float*)Pop();
	Push(P_BoxOnLineSide(tmbox, ld));
}

//==========================================================================
//
//  PF_P_BlockThingsIterator
//
//==========================================================================

PF(P_BlockThingsIterator)
{
	int			x;
	int			y;
	VObject*	SelfObj;
	VName		FuncName;
	VMethod*	func = NULL;

	FuncName = PopName();
	SelfObj = (VObject*)Pop();
	y = Pop();
	x = Pop();
	func = SelfObj->GetClass()->FindFunctionChecked(FuncName);
	Push(SV_BlockThingsIterator(x, y, NULL, SelfObj, func));
}

//==========================================================================
//
//	PF_P_PathTraverse
//
//==========================================================================

PF(P_PathTraverse)
{
	float		x1;
	float		y1;
	float		x2;
	float		y2;
	int			flags;
	VName		FuncName;
	VObject*	SelfObj;
	VMethod*	func;

	FuncName = PopName();
	SelfObj = (VObject*)Pop();
	flags = Pop();
	y2 = Popf();
	x2 = Popf();
	y1 = Popf();
	x1 = Popf();
	func = SelfObj->GetClass()->FindFunctionChecked(FuncName);
	Push(SV_PathTraverse(x1, y1, x2, y2, flags, NULL, SelfObj, func));
}

//==========================================================================
//
//	PF_FindThingGap
//
//==========================================================================

PF(FindThingGap)
{
	sec_region_t	*gaps;
	TVec			point;
	float			z1;
	float			z2;

	z2 = Popf();
	z1 = Popf();
	point = Popv();
	gaps = (sec_region_t*)Pop();
	Push((int)SV_FindThingGap(gaps, point, z1, z2));
}

//==========================================================================
//
//	PF_FindOpening
//
//==========================================================================

PF(FindOpening)
{
	opening_t	*gaps;
	float		z1;
	float		z2;

	z2 = Popf();
	z1 = Popf();
	gaps = (opening_t*)Pop();
	Push((int)SV_FindOpening(gaps, z1, z2));
}

//==========================================================================
//
//	PF_PointInRegion
//
//==========================================================================

PF(PointInRegion)
{
	sector_t	*sector;
	TVec		p;

	p = Popv();
	sector = (sector_t*)Pop();
	Push((int)SV_PointInRegion(sector, p));
}

//==========================================================================
//
//	PF_AddExtraFloor
//
//==========================================================================

PF(AddExtraFloor)
{
	line_t		*line;
	sector_t	*dst;

	dst = (sector_t*)Pop();
	line = (line_t*)Pop();
	Push((int)AddExtraFloor(line, dst));
	sv_signon << (byte)svc_extra_floor
				<< (short)(line - GLevel->Lines)
				<< (short)(dst - GLevel->Sectors);
}

//==========================================================================
//
//	PF_SwapPlanes
//
//==========================================================================

PF(SwapPlanes)
{
	sector_t	*s;

	s = (sector_t *)Pop();
	SwapPlanes(s);
	sv_signon << (byte)svc_swap_planes << (short)(s - GLevel->Sectors);
}

//==========================================================================
//
//	PF_MapBlock
//
//==========================================================================

PF(MapBlock)
{
	float x;

	x = Popf();
	Push(MapBlock(x));
}

//==========================================================================
//
//	PF_P_ChangeSector
//
//==========================================================================

PF(P_ChangeSector)
{
	sector_t*	sec;
	int			crunch;

	crunch = Pop();
	sec = (sector_t *)Pop();
	Push(P_ChangeSector(sec, crunch));
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

PF(NewMobjThinker)
{
	VClass *Class;

	Class = (VClass *)Pop();
	Push((int)SV_SpawnMobj(Class));
}

//==========================================================================
//
//	PF_NextMobj
//
//==========================================================================

PF(NextMobj)
{
	VThinker*	th;

	th = (VThinker*)Pop();
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
			Push((int)th);
			return;
		}
		th = th->Next;
	}
	Push(0);
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

PF(NewSpecialThinker)
{
	VClass		*Class;
	VThinker	*spec;

	Class = (VClass *)Pop();
	spec = (VThinker*)VObject::StaticSpawnObject(Class, PU_LEVSPEC);
	GLevel->AddThinker(spec);
	Push((int)spec);
}

//==========================================================================
//
//  PF_RemoveSpecialThinker
//
//==========================================================================

PF(RemoveSpecialThinker)
{
	VThinker	*spec;

    spec = (VThinker*)Pop();
    spec->SetFlags(_OF_DelayedDestroy);
}

//==========================================================================
//
//  PF_P_ChangeSwitchTexture
//
//==========================================================================

PF(P_ChangeSwitchTexture)
{
	line_t* 	line;
	int 		useAgain;

    useAgain = Pop();
    line = (line_t*)Pop();
	P_ChangeSwitchTexture(line, useAgain);
}

//==========================================================================
//
//	PF_NextThinker
//
//==========================================================================

PF(NextThinker)
{
	VThinker*	th;
	VClass*		Class;

	Class = (VClass*)Pop();
	th = (VThinker*)Pop();
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
			Push((int)th);
			return;
		}
		th = th->Next;
	}
	Push(0);
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

PF(SpawnPolyobj)
{
	float 	x;
	float 	y;
	int 	tag;
	int 	flags;

   	flags = Pop();
    tag = Pop();
    y = Popf();
    x = Popf();
	PO_SpawnPolyobj(x, y, tag, flags);
}

//==========================================================================
//
//	PF_AddAnchorPoint
//
//==========================================================================

PF(AddAnchorPoint)
{
	float 	x;
	float 	y;
	int 	tag;

    tag = Pop();
    y = Popf();
    x = Popf();
	PO_AddAnchorPoint(x, y, tag);
}

//==========================================================================
//
//	PF_GetPolyobj
//
//==========================================================================

PF(GetPolyobj)
{
	int 	polyNum;

    polyNum = Pop();
	Push((int)PO_GetPolyobj(polyNum));
}

//==========================================================================
//
//	PF_GetPolyobjMirror
//
//==========================================================================

PF(GetPolyobjMirror)
{
	int 	polyNum;

    polyNum = Pop();
	Push(PO_GetPolyobjMirror(polyNum));
}

//==========================================================================
//
//  PF_PO_MovePolyobj
//
//==========================================================================

PF(PO_MovePolyobj)
{
	int 	num;
	float 	x;
	float 	y;

	y = Popf();
    x = Popf();
    num = Pop();
	Push(PO_MovePolyobj(num, x, y));
}

//==========================================================================
//
//	PF_PO_RotatePolyobj
//
//==========================================================================

PF(PO_RotatePolyobj)
{
	int num;
	float angle;

	angle = Popf();
    num = Pop();
	Push(PO_RotatePolyobj(num, angle));
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

PF(StartACS)
{
	int		num;
    int		map;
	int 	arg1;
	int 	arg2;
	int 	arg3;
    VEntity	*activator;
    line_t	*line;
    int		side;
	bool	Always;
	bool	WantResult;

	WantResult = !!Pop();
	Always = !!Pop();
    side = Pop();
	line = (line_t*)Pop();
    activator = (VEntity*)Pop();
	arg3 = Pop();
	arg2 = Pop();
	arg1 = Pop();
    map = Pop();
    num = Pop();
	Push(P_StartACS(num, map, arg1, arg2, arg3, activator, line, side,
		Always, WantResult));
}

//==========================================================================
//
//  PF_SuspendACS
//
//==========================================================================

PF(SuspendACS)
{
	int 	number;
	int 	map;

    map = Pop();
    number = Pop();
	Push(P_SuspendACS(number, map));
}

//==========================================================================
//
//  PF_TerminateACS
//
//==========================================================================

PF(TerminateACS)
{
	int 	number;
	int 	map;

    map = Pop();
    number = Pop();
	Push(P_TerminateACS(number, map));
}

//==========================================================================
//
//  PF_TagFinished
//
//==========================================================================

PF(TagFinished)
{
	int		tag;

    tag = Pop();
	P_TagFinished(tag);
}

//==========================================================================
//
//  PF_PolyobjFinished
//
//==========================================================================

PF(PolyobjFinished)
{
	int		tag;

    tag = Pop();
	P_PolyobjFinished(tag);
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

PF(StartSoundAtVolume)
{
	VEntity*	mobj;
    int			sound;
	int			channel;
    int			vol;

    vol = Pop();
	channel = Pop();
    sound = Pop();
    mobj = (VEntity*)Pop();
    SV_StartSound(mobj, sound, channel, vol);
}

//==========================================================================
//
//	PF_SectorStartSound
//
//==========================================================================

PF(SectorStartSound)
{
	sector_t*	sec;
    int			sound;
	int			channel;

	channel = Pop();
    sound = Pop();
    sec = (sector_t*)Pop();
	SV_SectorStartSound(sec, sound, channel, 127);
}

//==========================================================================
//
//	PF_SectorStopSound
//
//==========================================================================

PF(SectorStopSound)
{
	sector_t*	sec;
	int			channel;

	channel = Pop();
    sec = (sector_t*)Pop();
    SV_SectorStopSound(sec, channel);
}

//==========================================================================
//
//	PF_GetSoundPlayingInfo
//
//==========================================================================

PF(GetSoundPlayingInfo)
{
	int			mobj;
    int			id;

    id = Pop();
    mobj = Pop();
#ifdef CLIENT
	Push(S_GetSoundPlayingInfo(mobj, id));
#else
	Push(0);
#endif
}

//==========================================================================
//
//	PF_GetSoundID
//
//==========================================================================

PF(GetSoundID)
{
	VName	Name;

    Name = PopName();
	Push(S_GetSoundID(Name));
}

//==========================================================================
//
//  PF_SetSeqTrans
//
//==========================================================================

PF(SetSeqTrans)
{
	VName	Name;
	int		Num;
	int		SeqType;

	SeqType = Pop();
	Num = Pop();
	Name = PopName();
	SN_SetSeqTrans(Name, Num, SeqType);
}

//==========================================================================
//
//  PF_GetSeqTrans
//
//==========================================================================

PF(GetSeqTrans)
{
	int		Num;
	int		SeqType;

	SeqType = Pop();
	Num = Pop();
	Push(SN_GetSeqTrans(Num, SeqType).GetIndex());
}

//==========================================================================
//
//  PF_SectorStartSequence
//
//==========================================================================

PF(SectorStartSequence)
{
	sector_t*	sec;
	VName		name;

    name = PopName();
    sec = (sector_t*)Pop();
	SV_SectorStartSequence(sec, *name);
}

//==========================================================================
//
//  PF_SectorStopSequence
//
//==========================================================================

PF(SectorStopSequence)
{
	sector_t*	sec;

    sec = (sector_t*)Pop();
	SV_SectorStopSequence(sec);
}

//==========================================================================
//
//  PF_PolyobjStartSequence
//
//==========================================================================

PF(PolyobjStartSequence)
{
	polyobj_t*	poly;
	VName		name;

    name = PopName();
    poly = (polyobj_t*)Pop();
	SV_PolyobjStartSequence(poly, *name);
}

//==========================================================================
//
//  PF_PolyobjStopSequence
//
//==========================================================================

PF(PolyobjStopSequence)
{
	polyobj_t*	poly;

    poly = (polyobj_t*)Pop();
	SV_PolyobjStopSequence(poly);
}

//==========================================================================
//
//	PF_ClearPlayer
//
//==========================================================================

PF(ClearPlayer)
{
	VBasePlayer	*pl;

    pl = (VBasePlayer*)Pop();

	pl->PClass = 0;
	pl->ForwardMove = 0;
	pl->SideMove = 0;
	pl->FlyMove = 0;
	pl->Buttons = 0;
	pl->Impulse = 0;
	pl->MO = NULL;
	pl->PlayerState = 0;
	pl->ViewOrg = TVec(0, 0, 0);
	pl->PlayerFlags &= ~VBasePlayer::PF_FixAngle;
	pl->Health = 0;
	pl->Items = 0;
	pl->PlayerFlags &= ~VBasePlayer::PF_AttackDown;
	pl->PlayerFlags &= ~VBasePlayer::PF_UseDown;
	pl->ExtraLight = 0;
	pl->FixedColormap = 0;
	pl->Palette = 0;
	memset(pl->CShifts, 0, sizeof(pl->CShifts));
	pl->PSpriteSY = 0;
	memset((byte*)pl + sizeof(VBasePlayer), 0,
		pl->GetClass()->ClassSize - sizeof(VBasePlayer));
}

//==========================================================================
//
//  PF_G_ExitLevel
//
//==========================================================================

PF(G_ExitLevel)
{
	int Position;

	Position = Pop();
	G_ExitLevel(Position);
}

//==========================================================================
//
//  PF_G_SecretExitLevel
//
//==========================================================================

PF(G_SecretExitLevel)
{
	int Position;

	Position = Pop();
	G_SecretExitLevel(Position);
}

//==========================================================================
//
//  PF_G_Completed
//
//==========================================================================

PF(G_Completed)
{
	int		map;
	int		pos;
	int		SaveAngle;

	SaveAngle = Pop();
	pos = Pop();
	map = Pop();
	G_Completed(map, pos, SaveAngle);
}

//==========================================================================
//
//	PF_P_GetThingFloorType
//
//==========================================================================

PF(TerrainType)
{
	int			pic;

	pic = Pop();
	Push(SV_TerrainType(pic));
}

//==========================================================================
//
//	PF_P_GetPlayerNum
//
//==========================================================================

PF(P_GetPlayerNum)
{
	VBasePlayer*	player;

	player = (VBasePlayer*)Pop();
	Push(SV_GetPlayerNum(player));
}

//==========================================================================
//
//	PF_SB_Start
//
//==========================================================================

PF(SB_Start)
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

PF(P_ForceLightning)
{
	SV_ForceLightning();
}

//==========================================================================
//
//	PF_SetFloorPic
//
//==========================================================================

PF(SetFloorPic)
{
	sector_t	*sec;
	int 		texture;

	texture = Pop();
	sec = (sector_t*)Pop();
	SV_SetFloorPic(sec - GLevel->Sectors, texture);
}

//==========================================================================
//
//	PF_SetCeilPic
//
//==========================================================================

PF(SetCeilPic)
{
	sector_t	*sec;
	int			texture;

	texture = Pop();
	sec = (sector_t*)Pop();
	SV_SetCeilPic(sec - GLevel->Sectors, texture);
}

//==========================================================================
//
//	PF_SetLineTexture
//
//==========================================================================

PF(SetLineTexture)
{
	int side;
	int position;
	int texture;

	texture = Pop();
	position = Pop();
	side = Pop();
	SV_SetLineTexture(side, position, texture);
}

//==========================================================================
//
//	PF_SetLineTransluc
//
//==========================================================================

PF(SetLineTransluc)
{
	line_t	*line;
	int		trans;

	trans = Pop();
	line = (line_t*)Pop();
	SV_SetLineTransluc(line, trans);
}

//==========================================================================
//
//	PF_SendFloorSlope
//
//==========================================================================

PF(SendFloorSlope)
{
	sector_t	*sector;

	sector = (sector_t*)Pop();
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

PF(SendCeilingSlope)
{
	sector_t	*sector;

	sector = (sector_t*)Pop();
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

PF(SetSecLightColor)
{
	sector_t	*sector;
	int			Col;

	Col = Pop();
	sector = (sector_t*)Pop();
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

PF(SetFloorLightSector)
{
	sector_t	*Sector;
	sector_t	*SrcSector;

	SrcSector = (sector_t*)Pop();
	Sector = (sector_t*)Pop();
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

PF(SetCeilingLightSector)
{
	sector_t	*Sector;
	sector_t	*SrcSector;

	SrcSector = (sector_t*)Pop();
	Sector = (sector_t*)Pop();
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

PF(SetHeightSector)
{
	sector_t	*Sector;
	sector_t	*SrcSector;
	int			Flags;

	Flags = Pop();
	SrcSector = (sector_t*)Pop();
	Sector = (sector_t*)Pop();
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

PF(FindModel)
{
	char *name;

	name = (char*)Pop();
	Push(SV_FindModel(name));
}

//==========================================================================
//
//	PF_GetModelIndex
//
//==========================================================================

PF(GetModelIndex)
{
	VName Name;

	Name = PopName();
	Push(SV_GetModelIndex(Name));
}

//==========================================================================
//
//	PF_FindSkin
//
//==========================================================================

PF(FindSkin)
{
	char *name;

	name = (char*)Pop();
	Push(SV_FindSkin(name));
}

//==========================================================================
//
//	PF_FindClassFromEditorId
//
//==========================================================================

PF(FindClassFromEditorId)
{
	int		Id;

	Id = Pop();
	Push((int)SV_FindClassFromEditorId(Id));
}

//==========================================================================
//
//	PF_FindClassFromScriptId
//
//==========================================================================

PF(FindClassFromScriptId)
{
	int		Id;

	Id = Pop();
	Push((int)SV_FindClassFromScriptId(Id));
}

//==========================================================================
//
//	PF_ChangeMusic
//
//==========================================================================

PF(ChangeMusic)
{
	const char*		SongName;

	SongName = (const char*)Pop();
	SV_ChangeMusic(SongName);
}

//==========================================================================
//
//  PF_FindSectorFromTag
//
//==========================================================================

PF(FindSectorFromTag)
{
	int		tag;
	int		start;

	start = Pop();
	tag = Pop();
	int Ret = -1;
	for (int i = start + 1; i < GLevel->NumSectors; i++)
		if (GLevel->Sectors[i].tag == tag)
		{
			Ret = i;
			break;
		}
	Push(Ret);
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

PF(SetVirtualScreen)
{
	int			Width;
	int			Height;

	Height = Pop();
	Width = Pop();
	SCR_SetVirtualScreen(Width, Height);
}

//==========================================================================
//
//	PF_R_RegisterPic
//
//==========================================================================

PF(R_RegisterPic)
{
	int name;
	int type;

	type = Pop();
	name = Pop();
	Push(GTextureManager.AddPatch(VName((char*)name, VName::AddLower8),
		TEXTYPE_Pic));
}

//==========================================================================
//
//	PF_R_RegisterPicPal
//
//==========================================================================

PF(R_RegisterPicPal)
{
	int name;
	int type;
	int palname;

	palname = Pop();
	type = Pop();
	name = Pop();
	Push(GTextureManager.AddRawWithPal(VName((char*)name, VName::AddLower8),
		VName((char*)palname, VName::AddLower8)));
}

//==========================================================================
//
//	PF_R_GetPicInfo
//
//==========================================================================

PF(R_GetPicInfo)
{
	int			handle;
	picinfo_t	*info;

	info = (picinfo_t*)Pop();
	handle = Pop();
	GTextureManager.GetTextureInfo(handle, info);
}

//==========================================================================
//
//	PF_R_DrawPic
//
//==========================================================================

PF(R_DrawPic)
{
	int			x;
	int			y;
	int			handle;

	handle = Pop();
	y = Pop();
	x = Pop();
	R_DrawPic(x, y, handle);
}

//==========================================================================
//
//	PF_R_DrawPic2
//
//==========================================================================

PF(R_DrawPic2)
{
	int			x;
	int			y;
	int			handle;
	int			trans;

	trans = Pop();
	handle = Pop();
	y = Pop();
	x = Pop();
	R_DrawPic(x, y, handle, trans);
}

//==========================================================================
//
//	PF_R_DrawShadowedPic
//
//==========================================================================

PF(R_DrawShadowedPic)
{
	int			x;
	int			y;
	int			handle;

	handle = Pop();
	y = Pop();
	x = Pop();
	R_DrawShadowedPic(x, y, handle);
}

//==========================================================================
//
//	PF_R_InstallSprite
//
//==========================================================================

PF(R_InstallSprite)
{
	int		name;
	int		index;

	index = Pop();
	name = Pop();
	R_InstallSprite((char*)name, index);
}

//==========================================================================
//
//	PF_R_DrawSpritePatch
//
//==========================================================================

PF(R_DrawSpritePatch)
{
	int		x;
	int		y;
	int		sprite;
	int		frame;
	int		rot;
	int		translation;

	translation = Pop();
	rot = Pop();
	frame = Pop();
	sprite = Pop();
	y = Pop();
	x = Pop();
	R_DrawSpritePatch(x, y, sprite, frame, rot, translation);
}

//==========================================================================
//
//	PF_InstallModel
//
//==========================================================================

PF(InstallModel)
{
	int			name;

	name = Pop();
	if (FL_FindFile((char*)name))
	{
		Push((int)Mod_FindName((char*)name));
	}
	else
	{
		Push(0);
	}
}

//==========================================================================
//
//	PF_R_DrawModelFrame
//
//==========================================================================

PF(R_DrawModelFrame)
{
	TVec		origin;
	float		angle;
	VModel		*model;
	int			frame;
	int			skin;

	skin = Pop();
	frame = Pop();
	model = (VModel*)Pop();
	angle = Popf();
	origin = Popv();
	R_DrawModelFrame(origin, angle, model, frame, (char*)skin);
}

//==========================================================================
//
//	PF_R_FillRectWithFlat
//
//==========================================================================

PF(R_FillRectWithFlat)
{
	int		x;
	int		y;
	int		width;
	int		height;
	int		name;

	name = Pop();
	height = Pop();
	width = Pop();
	y = Pop();
	x = Pop();
	R_FillRectWithFlat(x, y, width, height, (char*)name);
}

//==========================================================================
//
//	PF_R_ShadeRect
//
//==========================================================================

void R_ShadeRect(int x, int y, int width, int height, int shade);

PF(R_ShadeRect)
{
	int		x;
	int		y;
	int		width;
	int		height;
	int		shade;

	shade = Pop();
	height = Pop();
	width = Pop();
	y = Pop();
	x = Pop();
	R_ShadeRect(x, y, width, height, shade);
}

//==========================================================================
//
//	PF_R_FillRect
//
//==========================================================================

PF(R_FillRect)
{
	int		x;
	int		y;
	int		width;
	int		height;
	int		coulor;

	coulor = Pop();
	height = Pop();
	width = Pop();
	y = Pop();
	x = Pop();
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

PF(T_SetFont)
{
	int font;

	font = Pop();
	T_SetFont((font_e)font);
}

//==========================================================================
//
//	PF_T_SetAlign
//
//==========================================================================

PF(T_SetAlign)
{
	int			halign;
	int			valign;

	valign = Pop();
	halign = Pop();
	T_SetAlign((halign_e)halign, (valign_e)valign);
}

//==========================================================================
//
//	PF_T_SetDist
//
//==========================================================================

PF(T_SetDist)
{
	int			hdist;
	int			vdist;

	vdist = Pop();
	hdist = Pop();
	T_SetDist(hdist, vdist);
}

//==========================================================================
//
//	PF_T_SetShadow
//
//==========================================================================

PF(T_SetShadow)
{
	bool		state;

	state = !!Pop();
	T_SetShadow(state);
}

//==========================================================================
//
//	PF_T_TextWidth
//
//==========================================================================

PF(T_TextWidth)
{
	int			text;

	text = Pop();
	Push(T_TextWidth((char*)text));
}

//==========================================================================
//
//	PF_T_TextHeight
//
//==========================================================================

PF(T_TextHeight)
{
	int			text;

	text = Pop();
	Push(T_TextHeight((char*)text));
}

//==========================================================================
//
//	PF_T_DrawText
//
//==========================================================================

PF(T_DrawText)
{
	int			x;
	int			y;
	int			txt;

	txt = Pop();
	y = Pop();
	x = Pop();
	T_DrawText(x, y, (char*)txt);
}

//==========================================================================
//
//	PF_T_DrawNText
//
//==========================================================================

PF(T_DrawNText)
{
	int			x;
	int			y;
	int			txt;
	int			n;

	n = Pop();
	txt = Pop();
	y = Pop();
	x = Pop();
	T_DrawNText(x, y, (char*)txt, n);
}

//==========================================================================
//
//	PF_T_DrawTextW
//
//==========================================================================

PF(T_DrawTextW)
{
	int			x;
	int			y;
	int			txt;
	int			w;

	w = Pop();
	txt = Pop();
	y = Pop();
	x = Pop();
	T_DrawTextW(x, y, (char*)txt, w);
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

PF(LocalSound)
{
	VName	name;

	name = PopName();
	S_StartSoundName(*name);
}

//==========================================================================
//
//	PF_IsLocalSoundPlaying
//
//==========================================================================

PF(IsLocalSoundPlaying)
{
	VName	name;

	name = PopName();
	Push(S_GetSoundPlayingInfo(0, S_GetSoundID(*name)));
}

//==========================================================================
//
//	PF_StopLocalSounds
//
//==========================================================================

PF(StopLocalSounds)
{
	S_StopSound(0, 0);
}

//==========================================================================
//
//	PF_InputLine_SetValue
//
//==========================================================================

PF(TranslateKey)
{
	int ch;

	ch = Pop();
	Push(IN_TranslateKey(ch));
}

#endif

//==========================================================================
//
//	PF_print
//
//==========================================================================

PF(print)
{
	GCon->Log(PF_FormatString());
}

//==========================================================================
//
//	PF_dprint
//
//==========================================================================

PF(dprint)
{
	GCon->Log(NAME_Dev, PF_FormatString());
}

//==========================================================================
//
//	PF_Cmd_CheckParm
//
//==========================================================================

PF(Cmd_CheckParm)
{
	int		str;

	str = Pop();
	Push(VCommand::CheckParm((char*)str));
}

//==========================================================================
//
//	CmdBuf_AddText
//
//==========================================================================

PF(CmdBuf_AddText)
{
	GCmdBuf << PF_FormatString();
}

//==========================================================================
//
//	PF_Info_ValueForKey
//
//==========================================================================

PF(Info_ValueForKey)
{
	int		info;
	int		key;

	key = Pop();
	info = Pop();
	Push((int)Info_ValueForKey((char*)info, (char*)key));
}

//==========================================================================
//
//	PF_itof
//
//==========================================================================

PF(itof)
{
	int			x;

	x = Pop();
	Pushf((float)x);
}

//==========================================================================
//
//	PF_ftoi
//
//==========================================================================

PF(ftoi)
{
	float		x;

	x = Popf();
	Push((int)x);
}

//==========================================================================
//
//	PF_WadLumpPresent
//
//==========================================================================

PF(WadLumpPresent)
{
	int			name;

	name = Pop();
	Push(W_CheckNumForName(VName((char*)name, VName::AddLower8)) >= 0);
}

//==========================================================================
//
//	Temporary menu stuff
//
//==========================================================================

#ifdef CLIENT

struct slist_t;

char* P_GetMapName(int map);
char* P_GetMapLumpName(int map);
char *P_TranslateMap(int map);
void KeyNameForNum(int KeyNr, char* NameString);

void StartSearch(void);
slist_t * GetSlist(void);

PF(P_GetMapName)
{
	int		map;

	map = Pop();
	Push((int)P_GetMapName(map));
}

PF(P_GetMapLumpName)
{
	int		map;

	map = Pop();
	Push((int)P_GetMapLumpName(map));
}

PF(P_TranslateMap)
{
	int map;

	map = Pop();
	Push((int)P_TranslateMap(map));
}

PF(KeyNameForNum)
{
	int		keynum;
	int		str;

	str = Pop();
	keynum = Pop();
	KeyNameForNum(keynum, (char*)str);
}

PF(IN_GetBindingKeys)
{
	int			name;
	int			*key1;
	int			*key2;

	key2 = (int*)Pop();
	key1 = (int*)Pop();
	name = Pop();
	IN_GetBindingKeys((char*)name, *key1, *key2);
}

PF(IN_SetBinding)
{
	int			keynum;
	int			ondown;
	int			onup;

	onup = Pop();
	ondown = Pop();
	keynum = Pop();
	IN_SetBinding(keynum, (char*)ondown, (char*)onup);
}

PF(SV_GetSaveString)
{
	int		i;
	int		buf;

	buf = Pop();
	i = Pop();
#ifdef SERVER
	Push(SV_GetSaveString(i, (char*)buf));
#else
	Push(0);
#endif
}

PF(GetSlist)
{
	Push((int)GetSlist());
}

void LoadTextLump(char *name, char *buf, int bufsize);

PF(LoadTextLump)
{
	int			name;
	char		*buf;
	int			bufsize;

	bufsize = Pop();
	buf = (char*)Pop();
	name = Pop();
	LoadTextLump((char*)name, buf, bufsize);
}

PF(AllocDlight)
{
	int key = Pop();
	Push((int)CL_AllocDlight(key));
}

PF(NewParticle)
{
	Push((int)R_NewParticle());
}

#endif

PF(StateIsInRange)
{
	int MaxDepth = Pop();
	VState* End = (VState*)Pop();
	VState* Start = (VState*)Pop();
	VState* State = (VState*)Pop();
	Push(State->IsInRange(Start, End, MaxDepth));
}

//**************************************************************************
//
//		BUILTIN INFO TABLE
//
//**************************************************************************

builtin_info_t BuiltinInfo[] =
{
	//	Error functions
	_(Error),
	_(FatalError),

	//	Cvar functions
	_(CreateCvar),
	_(GetCvar),
	_(SetCvar),
	_(GetCvarF),
	_(SetCvarF),
	_(GetCvarS),
	_(SetCvarS),

	//	Math functions
	_(AngleMod360),
	_(AngleMod180),
	_(abs),
	_(fabs),
	_(sin),
	_(cos),
	_(tan),
	_(atan),
	_(atan2),
	_(Normalise),
	_(Length),
	_(DotProduct),
	_(CrossProduct),
	_(AngleVectors),
	_(AngleVector),
	_(VectorAngles),
	_(GetPlanePointZ),
	_(PointOnPlaneSide),
	_(RotateDirectionVector),
	_(VectorRotateAroundZ),

	//	String functions
	_(ptrtos),
	_(strgetchar),
	_(strsetchar),
	_(strlen),
	_(strcmp),
	_(stricmp),
	_(strcpy),
	_(strclr),
	_(strcat),
	_(strlwr),
	_(strupr),
	_(sprint),
	_(va),
	_(atoi),
	_(atof),

	//	Random numbers
	_(Random),
	_(P_Random),

	//	Textures
	_(CheckTextureNumForName),
	_(TextureNumForName),
	_(CheckFlatNumForName),
	_(FlatNumForName),
	_(TextureHeight),

	//	Message IO functions
	_(MSG_Select),
	_(MSG_CheckSpace),
	_(MSG_WriteByte),
	_(MSG_WriteShort),
	_(MSG_WriteLong),
	_(MSG_ReadChar),
	_(MSG_ReadByte),
	_(MSG_ReadShort),
	_(MSG_ReadWord),
	_(MSG_ReadLong),

	//	Printinf in console
	_(print),
	_(dprint),

	_(itof),
	_(ftoi),
	_(StrToName),
	_(Cmd_CheckParm),
	_(CmdBuf_AddText),
	_(Info_ValueForKey),
	_(WadLumpPresent),
	_(SpawnObject),
	_(FindClass),
	_(StateIsInRange),

#ifdef CLIENT
	_(P_GetMapName),
	_(P_GetMapLumpName),
	_(P_TranslateMap),
	_(KeyNameForNum),
	_(IN_GetBindingKeys),
	_(IN_SetBinding),
	_(SV_GetSaveString),
	__(StartSearch),
	_(GetSlist),

	_(LoadTextLump),
	_(AllocDlight),
	_(NewParticle),

	//	Graphics
	_(SetVirtualScreen),
	_(R_RegisterPic),
	_(R_RegisterPicPal),
	_(R_GetPicInfo),
	_(R_DrawPic),
	_(R_DrawPic2),
	_(R_DrawShadowedPic),
	_(R_InstallSprite),
	_(R_DrawSpritePatch),
	_(InstallModel),
	_(R_DrawModelFrame),
	_(R_FillRectWithFlat),
	_(R_ShadeRect),
	_(R_FillRect),

	//	Text
	_(T_SetFont),
	_(T_SetAlign),
	_(T_SetDist),
	_(T_SetShadow),
	_(T_TextWidth),
	_(T_TextHeight),
	_(T_DrawText),
	_(T_DrawNText),
	_(T_DrawTextW),
	__(T_DrawCursor),

	//	Client side sound
	_(LocalSound),
	_(IsLocalSoundPlaying),
	_(StopLocalSounds),

	_(TranslateKey),
#endif
#ifdef SERVER
	//	Print functions
	_(bprint),
	_(cprint),
	_(centerprint),

	//	Map utilites
	_(LineOpenings),
	_(P_BoxOnLineSide),
	_(P_BlockThingsIterator),
	_(P_PathTraverse),
	_(FindThingGap),
	_(FindOpening),
	_(PointInRegion),
	_(AddExtraFloor),
	_(SwapPlanes),
	_(MapBlock),
	_(P_ChangeSector),

	//	Mobj utilites
	_(NewMobjThinker),
	_(NextMobj),

	//	Special thinker utilites
	_(NewSpecialThinker),
	_(RemoveSpecialThinker),
	_(P_ChangeSwitchTexture),
	_(NextThinker),

	//	Polyobj functions
	_(SpawnPolyobj),
	_(AddAnchorPoint),
	_(GetPolyobj),
	_(GetPolyobjMirror),
	_(PO_MovePolyobj),
	_(PO_RotatePolyobj),

	//	ACS functions
	_(StartACS),
	_(SuspendACS),
	_(TerminateACS),
	_(TagFinished),
	_(PolyobjFinished),

	//	Sound functions
	_(StartSoundAtVolume),
	_(SectorStartSound),
	_(SectorStopSound),
	_(GetSoundPlayingInfo),
	_(GetSoundID),
	_(SetSeqTrans),
	_(GetSeqTrans),
	_(SectorStartSequence),
	_(SectorStopSequence),
	_(PolyobjStartSequence),
	_(PolyobjStopSequence),

	_(G_ExitLevel),
	_(G_SecretExitLevel),
	_(G_Completed),
	_(P_GetPlayerNum),
	_(SB_Start),
	_(ClearPlayer),
	_(TerrainType),
	_(P_ForceLightning),
	_(SetFloorPic),
	_(SetCeilPic),
	_(SetLineTexture),
	_(SetLineTransluc),
	_(SendFloorSlope),
	_(SendCeilingSlope),
	_(SetSecLightColor),
	_(SetFloorLightSector),
	_(SetCeilingLightSector),
	_(SetHeightSector),
	_(FindModel),
	_(GetModelIndex),
	_(FindSkin),
	_(FindClassFromEditorId),
	_(FindClassFromScriptId),
	_(MSG_SelectClientMsg),
	_(ChangeMusic),
	_(FindSectorFromTag),
#endif
	{NULL, NULL, NULL}
};

//**************************************************************************
//
//	$Log$
//	Revision 1.82  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.81  2006/03/29 22:32:27  dj_jl
//	Changed console variables and command buffer to use dynamic strings.
//	
//	Revision 1.80  2006/03/18 16:51:15  dj_jl
//	Renamed type class names, better code serialisation.
//	
//	Revision 1.79  2006/03/13 18:32:45  dj_jl
//	Added function to check if a state is in the range.
//	
//	Revision 1.78  2006/03/12 12:54:49  dj_jl
//	Removed use of bitfields for portability reasons.
//	
//	Revision 1.77  2006/03/10 19:31:25  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.76  2006/03/06 13:05:50  dj_jl
//	Thunbker list in level, client now uses entity class.
//	
//	Revision 1.75  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//	
//	Revision 1.74  2006/03/02 23:24:35  dj_jl
//	Wad lump names stored as names.
//	
//	Revision 1.73  2006/02/28 18:06:28  dj_jl
//	Put thinkers back in linked list.
//	
//	Revision 1.72  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.71  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//	
//	Revision 1.70  2006/02/17 19:22:46  dj_jl
//	Builtins belong to object class.
//	
//	Revision 1.69  2006/02/15 23:27:40  dj_jl
//	Added script ID class attribute.
//	
//	Revision 1.68  2006/02/09 22:35:54  dj_jl
//	Moved all client game code to classes.
//	
//	Revision 1.67  2006/02/05 18:52:44  dj_jl
//	Moved common utils to level info class or built-in.
//	
//	Revision 1.66  2006/02/05 14:11:00  dj_jl
//	Fixed conflict with Solaris.
//	
//	Revision 1.65  2006/01/10 19:47:00  dj_jl
//	Added builtin for changing the music.
//	
//	Revision 1.64  2005/12/11 21:37:00  dj_jl
//	Made path traversal callbacks class members.
//	
//	Revision 1.63  2005/12/07 22:53:26  dj_jl
//	Moved compiler generated data out of globals.
//	
//	Revision 1.62  2005/11/24 20:09:23  dj_jl
//	Removed unused fields from Object class.
//	
//	Revision 1.61  2005/11/20 12:38:50  dj_jl
//	Implemented support for sound sequence extensions.
//	
//	Revision 1.60  2005/11/08 20:57:15  dj_jl
//	Removed playing sound till done.
//	
//	Revision 1.59  2005/11/05 16:01:32  dj_jl
//	Added builtin to stop local sounds.
//	
//	Revision 1.58  2005/11/05 15:50:07  dj_jl
//	Voices played as normal sounds.
//	
//	Revision 1.57  2005/11/03 22:47:17  dj_jl
//	Builtin for drawing coloured rectangles.
//	
//	Revision 1.56  2005/06/04 13:59:02  dj_jl
//	Adding support for Boom fake sectors.
//	
//	Revision 1.55  2005/05/26 16:53:59  dj_jl
//	Created texture manager class
//	
//	Revision 1.54  2005/03/28 07:28:19  dj_jl
//	Transfer lighting and other BOOM stuff.
//	
//	Revision 1.53  2005/03/16 15:04:44  dj_jl
//	More work on line specials.
//	
//	Revision 1.52  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.51  2004/12/22 07:49:13  dj_jl
//	More extended ACS support, more linedef flags.
//	
//	Revision 1.50  2004/08/18 18:05:46  dj_jl
//	Support for higher virtual screen resolutions.
//	
//	Revision 1.49  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//	
//	Revision 1.48  2003/10/31 07:50:44  dj_jl
//	Line texture setting function
//	
//	Revision 1.47  2003/09/26 16:58:42  dj_jl
//	Wrapped text printing
//	
//	Revision 1.46  2003/07/11 16:45:20  dj_jl
//	Made array of players with pointers
//	
//	Revision 1.45  2003/03/08 12:10:13  dj_jl
//	API fixes.
//	
//	Revision 1.44  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.43  2002/08/28 16:41:09  dj_jl
//	Merged VMapObject with VEntity, some natives.
//	
//	Revision 1.42  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.41  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.40  2002/07/13 07:48:08  dj_jl
//	Moved some global functions to Entity class.
//	
//	Revision 1.39  2002/06/22 07:10:42  dj_jl
//	Added FindClass.
//	
//	Revision 1.38  2002/04/11 16:44:44  dj_jl
//	Got rid of some warnings.
//	
//	Revision 1.37  2002/03/28 18:03:53  dj_jl
//	Added SV_GetModelIndex
//	
//	Revision 1.36  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.35  2002/03/02 17:27:48  dj_jl
//	Renamed builtin Spawn to SpawnObject
//	
//	Revision 1.34  2002/02/26 17:53:08  dj_jl
//	Fixes for menus.
//	
//	Revision 1.33  2002/02/22 18:09:52  dj_jl
//	Some improvements, beautification.
//	
//	Revision 1.32  2002/02/15 19:12:03  dj_jl
//	Property namig style change
//	
//	Revision 1.31  2002/02/02 19:20:41  dj_jl
//	VMethod pointers used instead of the function numbers
//	
//	Revision 1.30  2002/01/29 18:17:27  dj_jl
//	Fixed saving of mobj pointers
//	
//	Revision 1.29  2002/01/12 18:04:01  dj_jl
//	Added unarchieving of names
//	
//	Revision 1.28  2002/01/11 18:22:41  dj_jl
//	Started to use names in progs
//	
//	Revision 1.27  2002/01/11 08:08:26  dj_jl
//	Added names to progs
//	Added sector plane swapping
//	
//	Revision 1.26  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.25  2001/12/27 17:33:29  dj_jl
//	Removed thinker list
//	
//	Revision 1.24  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.23  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.22  2001/12/04 18:16:28  dj_jl
//	Player models and skins handled by server
//	
//	Revision 1.21  2001/12/01 17:43:13  dj_jl
//	Renamed ClassBase to VObject
//	
//	Revision 1.20  2001/11/09 14:33:35  dj_jl
//	Moved input line to progs
//	Builtins for accessing and changing characters in strings
//	
//	Revision 1.19  2001/10/27 07:50:55  dj_jl
//	Some new builtins
//	
//	Revision 1.18  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.17  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.16  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.15  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.14  2001/10/02 17:36:08  dj_jl
//	Removed status bar widgets
//	
//	Revision 1.13  2001/09/27 17:34:22  dj_jl
//	Fixed bug with input line
//	
//	Revision 1.12  2001/09/27 17:03:20  dj_jl
//	Support for multiple mobj classes
//	
//	Revision 1.11  2001/09/25 17:05:34  dj_jl
//	Added stricmp
//	
//	Revision 1.10  2001/09/24 17:35:24  dj_jl
//	Support for thinker classes
//	
//	Revision 1.9  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.8  2001/08/30 17:45:35  dj_jl
//	Sound channels, moving messsage box to progs
//	
//	Revision 1.7  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
//	
//	Revision 1.6  2001/08/21 17:39:22  dj_jl
//	Real string pointers in progs
//	
//	Revision 1.5  2001/08/17 17:43:40  dj_jl
//	LINUX fixes
//	
//	Revision 1.4  2001/08/15 17:21:47  dj_jl
//	Added model drawing for menu
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
