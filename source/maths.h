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

	TAVec()
	{}
	TAVec(float APitch, float AYaw, float ARoll)
	: pitch(APitch)
	, yaw(AYaw)
	, roll(ARoll)
	{}
	friend VStream& operator << (VStream &Strm, TAVec &v)
	{
		return Strm << v.pitch << v.yaw << v.roll;
	}
};

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

int mlog2(int val);
int mround(float);

float AngleMod(float angle);
float AngleMod180(float angle);
void AngleVectors(const TAVec &angles, TVec &forward, TVec &right, TVec &up);
void AngleVector(const TAVec &angles, TVec &forward);
void VectorAngles(const TVec &vec, TAVec &angles);
void VectorsAngles(const TVec &forward, const TVec &right, const TVec &up,
	TAVec &angles);

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

enum
{
	PLANE_X,
	PLANE_Y,
	PLANE_Z,
	PLANE_NEG_X,
	PLANE_NEG_Y,
	PLANE_NEG_Z,
	PLANE_ANY
};

class TPlane
{
 public:
	TVec		normal;
	float		dist;
	int			type;
	int			signbits;
	int			reserved1;
	int			reserved2;

	void CalcBits()
	{
		if (normal.x == 1.0)
			type = PLANE_X;
		else if (normal.y == 1.0)
			type = PLANE_Y;
		else if (normal.z == 1.0)
			type = PLANE_Z;
		else if (normal.x == -1.0)
			type = PLANE_NEG_X;
		else if (normal.y == -1.0)
			type = PLANE_NEG_Y;
		else if (normal.z == -1.0)
			type = PLANE_NEG_Z;
		else
			type = PLANE_ANY;

		signbits = 0;
		if (normal.x < 0.0)
			signbits |= 1;
		if (normal.y < 0.0)
			signbits |= 2;
		if (normal.z < 0.0)
			signbits |= 4;
	}

	void Set(const TVec &Anormal, float Adist)
	{
		normal = Anormal;
		dist = Adist;
		CalcBits();
	}

	//	Initialises vertical plane from point and direction
	void SetPointDir(const TVec &point, const TVec &dir)
	{
		normal = Normalise(TVec(dir.y, -dir.x, 0));
		dist = DotProduct(point, normal);
		CalcBits();
	}

	//	Initialises vertical plane from 2 points
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
	int PointOnSide(const TVec& point) const
	{
		return DotProduct(point, normal) - dist < 0;
	}

	//	Returns side 0 (front), 1 (back), or 2 (on).
	int PointOnSide2(const TVec& point) const
	{
		float dot = DotProduct(point, normal) - dist;
		return dot < -0.1 ? 1 : dot > 0.1 ? 0 : 2;
	}
};
