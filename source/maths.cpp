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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

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
//	mlog2
//
//==========================================================================

int mlog2(int val)
{
	int answer = 0;
	while (val >>= 1)
		answer++;
	return answer;
}

//==========================================================================
//
//	AngleMod
//
//==========================================================================

float AngleMod(float angle)
{
#if 1
	while (angle < 0.0)
		angle += 360.0;
	while (angle >= 360.0)
		angle -= 360.0;
#else
	angle = (360.0/65536) * ((int)(angle*(65536/360.0)) & 65535);
#endif
	return angle;
}

//==========================================================================
//
//	AngleMod180
//
//==========================================================================

float AngleMod180(float angle)
{
#if 1
	while (angle < -180.0)
		angle += 360.0;
	while (angle >= 180.0)
		angle -= 360.0;
#else
	angle += 180;
	angle = (360.0/65536) * ((int)(angle*(65536/360.0)) & 65535);
	angle -= 180;
#endif
	return angle;
}

//==========================================================================
//
//	AngleVectors
//
//==========================================================================

void AngleVectors(const TAVec &angles, TVec &forward, TVec &right, TVec &up)
{
	double		ay, ap, ar;
	double		sr, sp, sy, cr, cp, cy;

	ay = DEG2RAD(angles.yaw);
	ap = DEG2RAD(angles.pitch);
	ar = DEG2RAD(angles.roll);

	sy = sin(ay);
	cy = cos(ay);
	sp = sin(ap);
	cp = cos(ap);
	sr = sin(ar);
	cr = cos(ar);

	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;
	right[0] = -sr * sp * cy + cr * sy;
	right[1] = -sr * sp * sy - cr * cy;
	right[2] = -sr * cp;
	up[0] = cr * sp * cy + sr * sy;
	up[1] = cr * sp * sy - sr * cy;
	up[2] = cr * cp;
}

//==========================================================================
//
//	AngleVector
//
//==========================================================================

void AngleVector(const TAVec &angles, TVec &forward)
{
	float		sp, sy, cp, cy;
	
	sy = msin(angles.yaw);
	cy = mcos(angles.yaw);
	sp = msin(angles.pitch);
	cp = mcos(angles.pitch);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

//==========================================================================
//
//	VectorAngles
//
//==========================================================================

void VectorAngles(const TVec &vec, TAVec &angles)
{
	double length = sqrt(vec.x * vec.x + vec.y * vec.y);
	if (!length)
	{
		cond << "Length 0, z = " << vec.z << endl;
		angles.pitch = 0;
		angles.yaw = 0;
		angles.roll = 0;
		return;
	}
	angles.pitch = -matan(vec.z, length);
	angles.yaw = matan(vec.y, vec.x);
	angles.roll = 0;
}

//==========================================================================
//
//	ProjectPointOnPlane
//
//==========================================================================

void ProjectPointOnPlane(TVec &dst, const TVec &p, const TVec &normal)
{
	float d;
	float inv_denom;

	inv_denom = 1.0F / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	dst = p - d * (normal * inv_denom);
}

//==========================================================================
//
//	PerpendicularVector
//
//	assumes "src" is normalized
//
//==========================================================================

void PerpendicularVector(TVec &dst, const TVec &src)
{
	int pos;
	int i;
	float minelem = 1.0F;
	TVec tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for (pos = 0, i = 0; i < 3; i++)
	{
		if (fabs(src[i]) < minelem)
		{
			pos = i;
			minelem = fabs(src[i]);
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane(dst, tempvec, src);

	/*
	** normalize the result
	*/
	dst = Normalize(dst);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2001/12/18 19:07:15  dj_jl
//	Changed AngleMod to a more precise version
//
//	Revision 1.7  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.6  2001/10/27 07:51:27  dj_jl
//	Beautification
//	
//	Revision 1.5  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.4  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
