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

#undef MIN
#undef MAX
#undef MID
#define MIN(x, y)		((x) < (y) ? (x) : (y))
#define MAX(x, y)		((x) > (y) ? (x) : (y))
#define MID(min, val, max)	MAX(min, MIN(val, max))

//==========================================================================
//
//	Angles
//
//==========================================================================

#ifndef M_PI
#define M_PI			3.14159265358979323846
#endif

#define _DEG2RAD		0.017453292519943296
#define _RAD2DEG		57.2957795130823209

#define DEG2RAD(a)		((a) * _DEG2RAD)
#define RAD2DEG(a)		((a) * _RAD2DEG)

class TAVec
{
 public:
	float pitch;
	float yaw;
	float roll;
};

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

int mlog2(int val);

float AngleMod(float angle);
float AngleMod180(float angle);
void AngleVectors(const TAVec &angles, TVec &forward, TVec &right, TVec &up);
void AngleVector(const TAVec &angles, TVec &forward);
void VectorAngles(const TVec &vec, TAVec &angles);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

inline float msin(float angle)
{
	return sin(DEG2RAD(angle));
}

inline float mcos(float angle)
{
	return cos(DEG2RAD(angle));
}

inline float mtan(float angle)
{
	return tan(DEG2RAD(angle));
}

inline float matan(float y, float x)
{
	return RAD2DEG(atan2(y, x));
}

//==========================================================================
//
//								PLANES
//
//==========================================================================

class TPlane
{
 public:
	TVec		normal;
	float		dist;

	void Set(const TVec &Anormal, float Adist)
	{
		normal = Anormal;
		dist = Adist;
	}

	//	Initializes vertical plane from point and direction
	void SetPointDir(const TVec &point, const TVec &dir)
	{
		normal = Normalize(TVec(dir.y, -dir.x, 0));
		dist = DotProduct(point, normal);
	}

	//	Initializes vertical plane from 2 points
	void Set2Points(const TVec &v1, const TVec &v2)
	{
		SetPointDir(v1, v2 - v1);
	}

	//	Get z of point with given x and y coords
	// Don't try to use it on a vertical plane
	float GetPointZ(float x, float y) const
	{
		return (dist - normal.x * x - normal.y * y) / normal.z;
	}

	float GetPointZ(const TVec &v) const
	{
		return GetPointZ(v.x, v.y);
	}

	//	Returns side 0 (front) or 1 (back).
	int PointOnSide(const TVec &point)
	{
		return DotProduct(point, normal) - dist < 0;
	}
};

//**************************************************************************
//
//	$Log$
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
