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
//**	Builtins.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

#define PF(name)			static void PF_##name(void)
#define _(name)				{#name, PF_##name}
#define __(name)			{#name, name}

#define PROG_TO_STR(ofs)	((char*)(ofs))
#define STR_TO_PROG(str)	(int(str))

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

int GetMobjNum(mobj_t *mobj);
mobj_t* SetMobjPtr(int archiveNum);

mobj_t *SV_SpawnMobj(void);
void SV_RemoveMobj(mobj_t *mobj);
void SV_ForceLightning(void);
void SV_SetFloorPic(int i, int texture);
void SV_SetCeilPic(int i, int texture);

float R_TextureHeight(int pic);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char*			pr_strings;
extern int				*pr_stackPtr;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TMessage			*pr_msg;

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
	str = PROG_TO_STR(Pop());

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

			 case 's':
				strcat(vastring, PROG_TO_STR(params[pi]));
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
				cond << "PF_FormatString: Unknown format identifier " << *src << endl;
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
		cond << "PF_FormatString: Not all params were used\n";
	}
	if (pi > count)
	{
		cond << "PF_FormatString: Param count overflow\n";
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

static void PF_Error(void)
{
    Host_Error(PF_FormatString());
}

//==========================================================================
//
//	PF_FatalError
//
//==========================================================================

static void PF_FatalError(void)
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
//  PF_GetCvar
//
//==========================================================================

static void PF_GetCvar(void)
{
	int		name;

    name = Pop();
    Push(Cvar_Value(PROG_TO_STR(name)));
}

//==========================================================================
//
//  PF_SetCvar
//
//==========================================================================

static void PF_SetCvar(void)
{
	int		name;
	int		value;

    value = Pop();
    name = Pop();
    Cvar_Set(PROG_TO_STR(name), value);
}

//==========================================================================
//
//  PF_GetCvarF
//
//==========================================================================

static void PF_GetCvarF(void)
{
	int		name;

    name = Pop();
    Pushf(Cvar_Float(PROG_TO_STR(name)));
}

//==========================================================================
//
//  PF_SetCvarF
//
//==========================================================================

static void PF_SetCvarF(void)
{
	int		name;
	float	value;

    value = Popf();
    name = Pop();
    Cvar_Set(PROG_TO_STR(name), value);
}

//==========================================================================
//
//  PF_GetCvarS
//
//==========================================================================

static void PF_GetCvarS(void)
{
	int		name;

    name = Pop();
    Push((int)STR_TO_PROG(Cvar_String(PROG_TO_STR(name))));
}

//==========================================================================
//
//  PF_SetCvarS
//
//==========================================================================

static void PF_SetCvarS(void)
{
	int		name;
	int		value;

    value = Pop();
    name = Pop();
    Cvar_Set(PROG_TO_STR(name), PROG_TO_STR(value));
}

//**************************************************************************
//
//	Math functions
//
//**************************************************************************

//==========================================================================
//
//	PF_sin
//
//==========================================================================

static void PF_sin(void)
{
	angle_t		an;

    an = Pop();
    Pushf(msin(an));
}

//==========================================================================
//
//	PF_cos
//
//==========================================================================

static void PF_cos(void)
{
	angle_t		an;

	an = Pop();
    Pushf(mcos(an));
}

//==========================================================================
//
//	PF_tan
//
//==========================================================================

static void PF_tan(void)
{
	angle_t		an;

	an = Pop();
    Pushf(mtan(an));
}

//==========================================================================
//
//	PF_atan
//
//==========================================================================

static void PF_atan(void)
{
	float	slope;

	slope = Popf();
    Push(RAD2BAM(atan(slope)));
}

//==========================================================================
//
//	PF_atan2
//
//==========================================================================

static void PF_atan2(void)
{
	float	y;
	float	x;

	x = Popf();
	y = Popf();
    Push(matan(y, x));
}

//==========================================================================
//
//	PF_Length
//
//==========================================================================

static void PF_Length(void)
{
	TVec		vec;

	vec = Popv();
	Pushf(Length(vec));
}

//==========================================================================
//
//	PF_Normalize
//
//==========================================================================

static void PF_Normalize(void)
{
	TVec		vec;

	vec = Popv();
	Pushv(Normalize(vec));
}

//==========================================================================
//
//	PF_DotProduct
//
//==========================================================================

static void PF_DotProduct(void)
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

static void PF_CrossProduct(void)
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

static void PF_AngleVectors(void)
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

static void PF_AngleVector(void)
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

static void PF_VectorAngles(void)
{
	TVec		*vec;
	TAVec		*angles;

	angles = (TAVec*)Pop();
	vec = (TVec*)Pop();
	VectorAngles(*vec, *angles);
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

static void PF_ptrtos(void)
{
	char	*ptr;

	ptr = (char*)Pop();
	Push(STR_TO_PROG(ptr));
}

//==========================================================================
//
//	PF_strlen
//
//==========================================================================

static void PF_strlen(void)
{
	int		s;

	s = Pop();
	Push(strlen(PROG_TO_STR(s)));
}

//==========================================================================
//
//	PF_strcmp
//
//==========================================================================

static void PF_strcmp(void)
{
	int		s1;
	int		s2;

	s2 = Pop();
	s1 = Pop();
	Push(strcmp(PROG_TO_STR(s1), PROG_TO_STR(s2)));
}

//==========================================================================
//
//	PF_strcpy
//
//==========================================================================

static void PF_strcpy(void)
{
	int		s1;
	int		s2;

	s2 = Pop();
	s1 = Pop();
	strcpy(PROG_TO_STR(s1), PROG_TO_STR(s2));
}

//==========================================================================
//
//	PF_strclr
//
//==========================================================================

static void PF_strclr(void)
{
	int		s;

	s = Pop();
	PROG_TO_STR(s)[0] = 0;
}

//==========================================================================
//
//	PF_strcat
//
//==========================================================================

static void PF_strcat(void)
{
	int		s1;
	int		s2;

	s2 = Pop();
	s1 = Pop();
	strcat(PROG_TO_STR(s1), PROG_TO_STR(s2));
}

//==========================================================================
//
//	PF_sprint
//
//==========================================================================

static void PF_sprint(void)
{
	int		dst;

	PF_FormatString();
	dst = Pop();
	strcpy(PROG_TO_STR(dst), vastring);
}

//==========================================================================
//
//	PF_va
//
//==========================================================================

static void PF_va(void)
{
	Push(STR_TO_PROG(PF_FormatString()));
}

//==========================================================================
//
//	PF_atoi
//
//==========================================================================

static void PF_atoi(void)
{
	int		str;

	str = Pop();
	Push(atoi(PROG_TO_STR(str)));
}

//==========================================================================
//
//	PF_atof
//
//==========================================================================

static void PF_atof(void)
{
	int		str;

	str = Pop();
	Pushf(atof(PROG_TO_STR(str)));
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

static void PF_Random(void)
{
	Pushf(Random());
}

//==========================================================================
//
//	PF_P_Random
//
//==========================================================================

static void PF_P_Random(void)
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

static void	PF_CheckTextureNumForName(void)
{
	int		name;

	name = Pop();
	Push(R_CheckTextureNumForName(PROG_TO_STR(name)));
}

//==========================================================================
//
//	PF_TextureNumForName
//
//==========================================================================

static void	PF_TextureNumForName(void)
{
	int		name;

	name = Pop();
	Push(R_TextureNumForName(PROG_TO_STR(name)));
}

//==========================================================================
//
//	PF_CheckFlatNumForName
//
//==========================================================================

static void	PF_CheckFlatNumForName(void)
{
	int		name;

	name = Pop();
	Push(R_CheckFlatNumForName(PROG_TO_STR(name)));
}

//==========================================================================
//
//	PF_FlatNumForName
//
//==========================================================================

static void	PF_FlatNumForName(void)
{
	int		name;

	name = Pop();
	Push(R_FlatNumForName(PROG_TO_STR(name)));
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
	Pushf(R_TextureHeight(pic));
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
	player_t	*client;

	client = (player_t*)Pop();
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
	 case MSG_SV_CLIENT:
		pr_msg = &client->message;
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

static void PF_bprint(void)
{
	SV_BroadcastPrintf(PF_FormatString());
}

//==========================================================================
//
//	PF_cprint
//
//==========================================================================

static void PF_cprint(void)
{
	player_t*	player;

	PF_FormatString();
    player = (player_t*)Pop();
	SV_ClientPrintf(player, vastring);
}

//==========================================================================
//
//	PF_centerprint
//
//==========================================================================

static void PF_centerprint(void)
{
	player_t*	player;

	PF_FormatString();
    player = (player_t*)Pop();
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

static void PF_LineOpenings(void)
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

static void PF_P_BoxOnLineSide(void)
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

static void PF_P_BlockThingsIterator(void)
{
	int		x;
    int		y;
    int		func;

	func = Pop();
    y = Pop();
    x = Pop();
	Push(SV_BlockThingsIterator(x, y, NULL, func));
}

//==========================================================================
//
//	P_BlockLinesIterator
//
//==========================================================================

static void PF_P_BlockLinesIterator(void)
{
	int			x;
	int			y;
	int			func;

	func = Pop();
	y = Pop();
	x = Pop();
	Push(SV_BlockLinesIterator(x, y, NULL, func));
}

//==========================================================================
//
//	PF_P_PathTraverse
//
//==========================================================================

static void PF_P_PathTraverse(void)
{
	float	x1;
	float	y1;
	float	x2;
	float	y2;
	int		flags;
	int		trav;

	trav = Pop();
	flags = Pop();
	y2 = Popf();
	x2 = Popf();
	y1 = Popf();
	x1 = Popf();
	Push(SV_PathTraverse(x1, y1, x2, y2, flags, NULL, trav));
}

//==========================================================================
//
//	PF_FindThingGap
//
//==========================================================================

static void PF_FindThingGap(void)
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

static void PF_FindOpening(void)
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

static void PF_PointInRegion(void)
{
	sector_t	*sector;
	TVec		p;

	p = Popv();
	sector = (sector_t*)Pop();
	Push((int)SV_PointInRegion(sector, p));
}

//==========================================================================
//
//	PF_PointContents
//
//==========================================================================

static void PF_PointContents(void)
{
	sector_t	*sector;
	TVec		p;

	p = Popv();
	sector = (sector_t*)Pop();
	Push(SV_PointContents(sector, p));
}

//==========================================================================
//
//	PF_AddExtraFloor
//
//==========================================================================

static void PF_AddExtraFloor(void)
{
	line_t		*line;
	sector_t	*dst;

	dst = (sector_t*)Pop();
	line = (line_t*)Pop();
	Push((int)AddExtraFloor(line, dst));
	sv_signon << (byte)svc_extra_floor
				<< (short)(line - level.lines)
				<< (short)(dst - level.sectors);
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

static void PF_NewMobjThinker(void)
{
	Push((int)SV_SpawnMobj());
}

//==========================================================================
//
//  PF_RemoveThinker
//
//==========================================================================

static void PF_RemoveMobjThinker(void)
{
	mobj_t		*mobj;

    mobj = (mobj_t*)Pop();
	SV_RemoveMobj(mobj);
}

//==========================================================================
//
//	PF_P_SetThingPosition
//
//==========================================================================

static void PF_P_SetThingPosition(void)
{
	mobj_t*		thing;

    thing = (mobj_t*)Pop();
    SV_LinkToWorld(thing);
}

//==========================================================================
//
//	PF_P_UnsetThingPosition
//
//==========================================================================

static void PF_P_UnsetThingPosition(void)
{
	mobj_t*		thing;

    thing = (mobj_t*)Pop();
    SV_UnlinkFromWorld(thing);
}

//==========================================================================
//
//	PF_NextMobj
//
//==========================================================================

static void PF_NextMobj(void)
{
    thinker_t*	th;

    th = (thinker_t*)Pop();
	if (!th)
    {
    	th = &level.thinkers;
	}
    for (th = th->next; th != &level.thinkers; th = th->next)
    {
        if (th->function == (think_t)P_MobjThinker)
        {
            Push((int)th);
            return;
		}
    }
	Push(0);
}

//==========================================================================
//
//	PF_P_CheckSight
//
//==========================================================================

static void PF_P_CheckSight(void)
{
	mobj_t*		mobj1;
    mobj_t*		mobj2;

	mobj2 = (mobj_t*)Pop();
    mobj1 = (mobj_t*)Pop();
    Push(P_CheckSight(mobj1, mobj2));
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

static void PF_NewSpecialThinker(void)
{
	int			func;
	special_t	*spec;
	
    func = Pop();
    spec = (special_t*)Z_Malloc(sizeof(*spec), PU_LEVSPEC, 0);
	memset(spec, 0, sizeof(*spec));
    spec->function = (think_t)P_SpecialThinker;
    spec->funcnum = func;
    P_AddThinker(spec);
	Push((int)spec);
}

//==========================================================================
//
//  PF_RemoveSpecialThinker
//
//==========================================================================

static void PF_RemoveSpecialThinker(void)
{
	special_t	*spec;

    spec = (special_t*)Pop();
    P_RemoveThinker(spec);
}

//==========================================================================
//
//  PF_P_ChangeSwitchTexture
//
//==========================================================================

static void PF_P_ChangeSwitchTexture(void)
{
	line_t* 	line;
	int 		useAgain;

    useAgain = Pop();
    line = (line_t*)Pop();
	P_ChangeSwitchTexture(line, useAgain);
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

static void PF_SpawnPolyobj(void)
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

static void PF_AddAnchorPoint(void)
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

static void PF_GetPolyobj(void)
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

static void PF_GetPolyobjMirror(void)
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

static void PF_PO_MovePolyobj(void)
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

static void PF_PO_RotatePolyobj(void)
{
	int 	num;
	angle_t angle;

	angle = Pop();
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

static void PF_StartACS(void)
{
	int		num;
    int		map;
	int 	*args;
    mobj_t	*activator;
    line_t	*line;
    int		side;

    side = Pop();
	line = (line_t*)Pop();
    activator = (mobj_t*)Pop();
	args = (int*)Pop();
    map = Pop();
    num = Pop();
	Push(P_StartACS(num, map, args, activator, line, side));
}

//==========================================================================
//
//  PF_SuspendACS
//
//==========================================================================

static void PF_SuspendACS(void)
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

static void PF_TerminateACS(void)
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

static void PF_TagFinished(void)
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

static void PF_PolyobjFinished(void)
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
//	PF_StartSound
//
//==========================================================================

static void PF_StartSound(void)
{
	mobj_t*		mobj;
    int			sound;

    sound = Pop();
    mobj = (mobj_t*)Pop();
	SV_StartSound(mobj, sound, 127);
}

//==========================================================================
//
//	PF_StartSoundAtVolume
//
//==========================================================================

static void PF_StartSoundAtVolume(void)
{
	mobj_t*		mobj;
    int			sound;
    int			vol;

    vol = Pop();
    sound = Pop();
    mobj = (mobj_t*)Pop();
    SV_StartSound(mobj, sound, vol);
}

//==========================================================================
//
//	PF_StopSound
//
//==========================================================================

static void PF_StopSound(void)
{
	mobj_t*		mobj;

    mobj = (mobj_t*)Pop();
    SV_StopSound(mobj);
}

//==========================================================================
//
//	PF_GetSoundPlayingInfo
//
//==========================================================================

static void PF_GetSoundPlayingInfo(void)
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

static void PF_GetSoundID(void)
{
	int		str;

    str = Pop();
	Push(S_GetSoundID(PROG_TO_STR(str)));
}

//==========================================================================
//
//  PF_StartSequence
//
//==========================================================================

static void PF_StartSequence(void)
{
	mobj_t*		mobj;
	int 		name;

    name = Pop();
    mobj = (mobj_t*)Pop();
	SV_StartSequence(mobj, PROG_TO_STR(name));
}

//==========================================================================
//
//  PF_StopSequence
//
//==========================================================================

static void PF_StopSequence(void)
{
	mobj_t*		mobj;

    mobj = (mobj_t*)Pop();
	SV_StopSequence(mobj);
}

//**************************************************************************
//
//	Savegame archieve / unarchieve utilite functions
//
//**************************************************************************

//==========================================================================
//
//	PF_SectorToNum
//
//==========================================================================

static void PF_SectorToNum(void)
{
	sector_t*	sector;

	sector = (sector_t*)Pop();
	if (sector)
		Push(sector - level.sectors);
	else
    	Push(-1);
}

//==========================================================================
//
//	PF_NumToSector
//
//==========================================================================

static void PF_NumToSector(void)
{
	int		num;

    num = Pop();
	if (num >= 0)
	    Push((int)&level.sectors[num]);
	else
    	Push(0);
}

//==========================================================================
//
//  PF_MobjToNum
//
//==========================================================================

static void PF_MobjToNum(void)
{
	mobj_t*		mobj;

    mobj = (mobj_t*)Pop();
    Push(GetMobjNum(mobj));
}

//==========================================================================
//
//  PF_NumToMobj
//
//==========================================================================

static void PF_NumToMobj(void)
{
	int 	archiveNum;

	archiveNum = Pop();
    Push((int)SetMobjPtr(archiveNum));
}

//==========================================================================
//
//	PF_ClearPlayer
//
//==========================================================================

static void PF_ClearPlayer(void)
{
	player_t	*pl;

    pl = (player_t*)Pop();

	pl->pclass = 0;
	pl->forwardmove = 0;
	pl->sidemove = 0;
	pl->flymove = 0;
	pl->buttons = 0;
	pl->impulse = 0;
	pl->mo = 0;
	pl->playerstate = 0;
	pl->vieworg = TVec(0, 0, 0);
	pl->fixangle = 0;
	pl->health = 0;
	pl->items = 0;
	pl->attackdown = 0;
	pl->usedown = 0;
	pl->extralight = 0;
	pl->fixedcolormap = 0;
	pl->palette = 0;
	memset(pl->cshifts, 0, sizeof(pl->cshifts));
    memset(pl->psprites, 0, sizeof(pl->psprites));
	pl->pspriteSY = 0;
	memset(pl->user_fields, 0, sizeof(pl->user_fields));
}

//==========================================================================
//
//  PF_G_ExitLevel
//
//==========================================================================

static void PF_G_ExitLevel(void)
{
    G_ExitLevel();
}

//==========================================================================
//
//  PF_G_SecretExitLevel
//
//==========================================================================

static void PF_G_SecretExitLevel(void)
{
    G_SecretExitLevel();
}

//==========================================================================
//
//  PF_G_Completed
//
//==========================================================================

static void PF_G_Completed(void)
{
	int		map;
    int		pos;

    pos = Pop();
    map = Pop();
    G_Completed(map, pos);
}

//==========================================================================
//
//	PF_P_GetThingFloorType
//
//==========================================================================

static void PF_TerrainType(void)
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

static void PF_P_GetPlayerNum(void)
{
	player_t*	player;
	int 		i;

    player = (player_t*)Pop();
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (player == &players[i])
		{
		    Push(i);
            return;
		}
	}
	Push(0);
}

//==========================================================================
//
// 	PF_PointInSubsector
//
//==========================================================================

static void PF_PointInSubsector(void)
{
	float		x;
    float		y;

    y = Popf();
    x = Popf();
    Push((int)SV_PointInSubsector(x, y));
}

//==========================================================================
//
//	PF_SB_Start
//
//==========================================================================

static void PF_SB_Start(void)
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

static void PF_P_ForceLightning(void)
{
	SV_ForceLightning();
}

//==========================================================================
//
//	PF_SetFloorPic
//
//==========================================================================

static void	PF_SetFloorPic(void)
{
	sector_t	*sec;
 	int 		texture;

	texture = Pop();
	sec = (sector_t*)Pop();
	SV_SetFloorPic(sec - level.sectors, texture);
}

//==========================================================================
//
//	PF_SetCeilPic
//
//==========================================================================

static void	PF_SetCeilPic(void)
{
	sector_t	*sec;
 	int 		texture;

	texture = Pop();
	sec = (sector_t*)Pop();
	SV_SetCeilPic(sec - level.sectors, texture);
}

//==========================================================================
//
//	PF_SetLineTransluc
//
//==========================================================================

static void PF_SetLineTransluc(void)
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

static void	PF_SendFloorSlope(void)
{
	sector_t	*sector;

	sector = (sector_t*)Pop();
	sv_signon << (byte)svc_sec_floor_plane
			<< (word)(sector - level.sectors)
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

static void	PF_SendCeilingSlope(void)
{
	sector_t	*sector;

	sector = (sector_t*)Pop();
	sv_signon << (byte)svc_sec_ceil_plane
			<< (word)(sector - level.sectors)
			<< sector->ceiling.normal.x
			<< sector->ceiling.normal.y
			<< sector->ceiling.normal.z
			<< sector->ceiling.dist;
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
//	PF_R_RegisterPic
//
//==========================================================================

PF(R_RegisterPic)
{
	int name;
	int type;

	type = Pop();
	name = Pop();
	Push(R_RegisterPic(PROG_TO_STR(name), type));
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
	Push(R_RegisterPicPal(PROG_TO_STR(name), type, PROG_TO_STR(palname)));
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
	R_GetPicInfo(handle, info);
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
	R_InstallSprite(PROG_TO_STR(name), index);
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
	if (FL_FindFile(PROG_TO_STR(name), NULL))
	{
		Push((int)Mod_FindName(PROG_TO_STR(name)));
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
	angle_t		angle;
	model_t		*model;
	int			frame;
	int			skin;

	skin = Pop();
	frame = Pop();
	model = (model_t*)Pop();
	angle = Pop();
	origin = Popv();
	R_DrawModelFrame(origin, angle, model, frame, PROG_TO_STR(skin));
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
	R_FillRectWithFlat(x, y, width, height, PROG_TO_STR(name));
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
//	PF_T_SetShadow
//
//==========================================================================

PF(T_SetShadow)
{
	boolean		state;

	state = Pop();
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
	Push(T_TextWidth(PROG_TO_STR(text)));
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
	Push(T_TextHeight(PROG_TO_STR(text)));
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
	T_DrawText(x, y, PROG_TO_STR(txt));
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
	T_DrawNText(x, y, PROG_TO_STR(txt), n);
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
	int		name;

	name = Pop();
	S_StartSoundName(PROG_TO_STR(name));
}

//==========================================================================
//
//	PF_LocalSoundTillDone
//
//==========================================================================

PF(LocalSoundTillDone)
{
	int		name;

	name = Pop();
	S_PlayTillDone(PROG_TO_STR(name));
}

//**************************************************************************
//
//	Input line
//
//**************************************************************************

//==========================================================================
//
//	PF_InputLine_Init
//
//==========================================================================

PF(InputLine_Init)
{
	TILine	*iline;

	iline = (TILine*)Pop();
	iline->Init();
}

//==========================================================================
//
//	PF_InputLine_SetValue
//
//==========================================================================

PF(InputLine_SetValue)
{
	TILine	*iline;
	int		text;

	text = Pop();
	iline = (TILine*)Pop();
	char *ch = PROG_TO_STR(text);
	while (*ch)
	{
		iline->AddChar(*ch);
	}
}

//==========================================================================
//
//	PF_InputLine_Key
//
//==========================================================================

PF(InputLine_Key)
{
	TILine	*iline;
	int		key;

	key = Pop();
	iline = (TILine*)Pop();
	Push(iline->Key(key));
}

//==========================================================================
//
//  PF_SB_CreateWidget
//
//==========================================================================

static void PF_SB_CreateWidget(void)
{
	int		type;
	int		x;
	int 	y;
	int 	pindex;
	int		style;
	boolean *on;
	int 	pparam;
    int 	*data;

    data = (int*)Pop();
	pparam = Pop();
    on = (boolean*)Pop();
    style = Pop();
    pindex = Pop();
    y = Pop();
    x = Pop();
    type = Pop();
	Push((int)SB_CreateWidget((sb_widget_type_t)type, x, y, pindex,
		(sb_style_t)style, on, pparam, data));
}
#endif

//==========================================================================
//
//	PF_print
//
//==========================================================================

static void PF_print(void)
{
	con << PF_FormatString();
}

//==========================================================================
//
//	PF_dprint
//
//==========================================================================

static void PF_dprint(void)
{
	cond << PF_FormatString();
}

//==========================================================================
//
//	PF_Cmd_CheckParm
//
//==========================================================================

static void PF_Cmd_CheckParm(void)
{
	int		str;

    str = Pop();
    Push(Cmd_CheckParm(PROG_TO_STR(str)));
}

//==========================================================================
//
//	CmdBuf_AddText
//
//==========================================================================

static void PF_CmdBuf_AddText(void)
{
	CmdBuf << PF_FormatString();
}

//==========================================================================
//
//	PF_Info_ValueForKey
//
//==========================================================================

static void	PF_Info_ValueForKey(void)
{
	int		info;
	int		key;

	key = Pop();
	info = Pop();
	Push(STR_TO_PROG(Info_ValueForKey(PROG_TO_STR(info), PROG_TO_STR(key))));
}

//==========================================================================
//
//	PF_itof
//
//==========================================================================

static void PF_itof(void)
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

static void PF_ftoi(void)
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
	Push(W_CheckNumForName(PROG_TO_STR(name)) >= 0);
}

//==========================================================================
//
//	Temporary menu stuff
//
//==========================================================================

#ifdef CLIENT

struct slist_t;

char* P_GetMapName(int map);
void KeyNameForNum(int KeyNr, char* NameString);

void StartSearch(void);
void GetSlist(slist_t *slist);
void DrawTextBox(char *string);

int		mb_func;

void ProgResponse(int key)
{
	clpr.Exec(mb_func, key);
}

PF(MB_StartMessage)
{
	int		str;
	int		inp;

	inp = Pop();
	mb_func = Pop();
	str = Pop();
	MB_StartMessage(PROG_TO_STR(str), (void*)ProgResponse, inp);
}

PF(P_GetMapName)
{
	int		map;

	map = Pop();
	Push(STR_TO_PROG(P_GetMapName(map)));
}

PF(KeyNameForNum)
{
	int		keynum;
	int		str;

	str = Pop();
	keynum = Pop();
	KeyNameForNum(keynum, PROG_TO_STR(str));
}

PF(IN_GetBindingKeys)
{
	int			name;
	int			*key1;
	int			*key2;

	key2 = (int*)Pop();
	key1 = (int*)Pop();
	name = Pop();
	IN_GetBindingKeys(PROG_TO_STR(name), *key1, *key2);
}

PF(IN_SetBinding)
{
	int			keynum;
	int			ondown;
	int			onup;

	onup = Pop();
	ondown = Pop();
	keynum = Pop();
	IN_SetBinding(keynum, PROG_TO_STR(ondown), PROG_TO_STR(onup));
}

PF(SV_GetSaveString)
{
	int		i;
	int		buf;

	buf = Pop();
	i = Pop();
#ifdef SERVER
	Push(SV_GetSaveString(i, PROG_TO_STR(buf)));
#else
	Push(0);
#endif
}

PF(GetSlist)
{
	slist_t		*slist;

	slist = (slist_t*)Pop();
	GetSlist(slist);
}

PF(DrawTextBox)
{
	int		string;

	string = Pop();
	DrawTextBox(PROG_TO_STR(string));
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
	LoadTextLump(PROG_TO_STR(name), buf, bufsize);
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

//**************************************************************************
//
//		BUILTIN INFO TABLE
//
//**************************************************************************

builtin_info_t BuiltinInfo[] =
{
#ifdef CLIENT
	_(MB_StartMessage),
	_(P_GetMapName),
	_(KeyNameForNum),
	_(IN_GetBindingKeys),
	_(IN_SetBinding),
	_(SV_GetSaveString),
	__(StartSearch),
	_(GetSlist),
	_(DrawTextBox),

	_(LoadTextLump),
	_(AllocDlight),
	_(NewParticle),
#endif

	//	Error functions
	{"Error", PF_Error},
	{"FatalError", PF_FatalError},

	//	Cvar functions
    {"GetCvar", PF_GetCvar},
    {"SetCvar", PF_SetCvar},
    {"GetCvarF", PF_GetCvarF},
    {"SetCvarF", PF_SetCvarF},
    {"GetCvarS", PF_GetCvarS},
    {"SetCvarS", PF_SetCvarS},

	//	Math functions
	{"sin", PF_sin},
	{"cos", PF_cos},
	{"tan", PF_tan},
	{"atan", PF_atan},
	{"atan2", PF_atan2},
	{"Normalize", PF_Normalize},
	{"Length", PF_Length},
	{"DotProduct", PF_DotProduct},
	{"CrossProduct", PF_CrossProduct},
	{"AngleVectors", PF_AngleVectors},
	{"AngleVector", PF_AngleVector},
	{"VectorAngles", PF_VectorAngles},

	//	String functions
	{"ptrtos", PF_ptrtos},
	{"strlen", PF_strlen},
	{"strcmp", PF_strcmp},
	{"strcpy", PF_strcpy},
	{"strclr", PF_strclr},
	{"strcat", PF_strcat},
	{"sprint", PF_sprint},
	{"va", PF_va},
	{"atoi", PF_atoi},
	{"atof", PF_atof},

	//	Random numbers
    {"Random", PF_Random},
    {"P_Random", PF_P_Random},

	//	Textures
	{"CheckTextureNumForName", PF_CheckTextureNumForName},
	{"TextureNumForName", PF_TextureNumForName},
	{"CheckFlatNumForName", PF_CheckFlatNumForName},
	{"FlatNumForName", PF_FlatNumForName},
	_(TextureHeight),

	//	Message IO functions
	_(MSG_Select),
	_(MSG_WriteByte),
	_(MSG_WriteShort),
	_(MSG_WriteLong),
	_(MSG_ReadChar),
	_(MSG_ReadByte),
	_(MSG_ReadShort),
	_(MSG_ReadWord),
	_(MSG_ReadLong),

	//	Printinf in console
	{"print", PF_print},
	{"dprint", PF_dprint},

	{"itof", PF_itof},
	{"ftoi", PF_ftoi},
    {"Cmd_CheckParm", PF_Cmd_CheckParm},
	{"CmdBuf_AddText", PF_CmdBuf_AddText},
	{"Info_ValueForKey", PF_Info_ValueForKey},
	_(WadLumpPresent),

#ifdef CLIENT
	//	Graphics
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

	//	Text
	_(T_SetFont),
	_(T_SetAlign),
	_(T_SetShadow),
	_(T_TextWidth),
	_(T_TextHeight),
	_(T_DrawText),
	_(T_DrawNText),
	__(T_DrawCursor),

	//	Client side sound
	_(LocalSound),
	_(LocalSoundTillDone),

	//	Input line
	_(InputLine_Init),
	_(InputLine_SetValue),
	_(InputLine_Key),

	_(SB_CreateWidget),
#endif
#ifdef SERVER
	//	Print functions
	{"bprint", PF_bprint},
	{"cprint", PF_cprint},
	{"centerprint", PF_centerprint},

	//	Map utilites
	{"LineOpenings", PF_LineOpenings},
	{"P_BoxOnLineSide", PF_P_BoxOnLineSide},
    {"P_BlockThingsIterator", PF_P_BlockThingsIterator},
	{"P_BlockLinesIterator", PF_P_BlockLinesIterator},
	{"P_PathTraverse", PF_P_PathTraverse},
	{"FindThingGap", PF_FindThingGap},
	{"FindOpening", PF_FindOpening},
	{"PointInRegion", PF_PointInRegion},
	{"PointContents", PF_PointContents},
	{"AddExtraFloor", PF_AddExtraFloor},

	//	Mobj utilites
    {"NewMobjThinker", PF_NewMobjThinker},
    {"RemoveMobjThinker", PF_RemoveMobjThinker},
    {"P_SetThingPosition", PF_P_SetThingPosition},
    {"P_UnsetThingPosition", PF_P_UnsetThingPosition},
    {"NextMobj", PF_NextMobj},
    {"P_CheckSight", PF_P_CheckSight},

    //	Special thinker utilites
    {"NewSpecialThinker", PF_NewSpecialThinker},
    {"RemoveSpecialThinker", PF_RemoveSpecialThinker},
    {"P_ChangeSwitchTexture", PF_P_ChangeSwitchTexture},

    //	Polyobj functions
    {"SpawnPolyobj", PF_SpawnPolyobj},
    {"AddAnchorPoint", PF_AddAnchorPoint},
	{"GetPolyobj", PF_GetPolyobj},
    {"GetPolyobjMirror", PF_GetPolyobjMirror},
	{"PO_MovePolyobj", PF_PO_MovePolyobj},
 	{"PO_RotatePolyobj", PF_PO_RotatePolyobj},

	//	ACS functions
	{"StartACS", PF_StartACS},
    {"SuspendACS", PF_SuspendACS},
    {"TerminateACS", PF_TerminateACS},
    {"TagFinished", PF_TagFinished},
    {"PolyobjFinished", PF_PolyobjFinished},

	//	Sound functions
    {"StartSound", PF_StartSound},
    {"StartSoundAtVolume", PF_StartSoundAtVolume},
    {"StopSound", PF_StopSound},
    {"GetSoundPlayingInfo", PF_GetSoundPlayingInfo},
    {"GetSoundID", PF_GetSoundID},
    {"StartSequence", PF_StartSequence},
    {"StopSequence", PF_StopSequence},

    //  Savegame archieve / unarchieve utilite functions
    {"SectorToNum", PF_SectorToNum},
    {"NumToSector", PF_NumToSector},
    {"NumToMobj", PF_NumToMobj},
    {"MobjToNum", PF_MobjToNum},

    {"G_ExitLevel", PF_G_ExitLevel},
    {"G_SecretExitLevel", PF_G_SecretExitLevel},
    {"G_Completed", PF_G_Completed},
    {"PointInSubsector", PF_PointInSubsector},
    {"P_GetPlayerNum", PF_P_GetPlayerNum},
    {"SB_Start", PF_SB_Start},
    {"ClearPlayer", PF_ClearPlayer},
	{"TerrainType", PF_TerrainType},
    {"P_ForceLightning", PF_P_ForceLightning},
	{"SetFloorPic", PF_SetFloorPic},
	{"SetCeilPic", PF_SetCeilPic},
	{"SetLineTransluc", PF_SetLineTransluc},
	{"SendFloorSlope", PF_SendFloorSlope},
	{"SendCeilingSlope", PF_SendCeilingSlope},
#endif
    {NULL, NULL}
};

//**************************************************************************
//
//	$Log$
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
