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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class  TVec
{
 public:
	float		x;
	float		y;
	float		z;

	TVec(void) { }

	TVec(float Ax, float Ay, float Az)
	{
		x = Ax;
		y = Ay;
		z = Az;
	}

	TVec (const float f[3])
	{
    	x = f[0];
    	y = f[1];
    	z = f[2];
	}

	const float& operator[](int i) const
	{
    	return (&x)[i];
	}

	float& operator[](int i)
	{
	    return (&x)[i];
	}

	TVec &operator += (const TVec &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	TVec &operator -= (const TVec &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	TVec &operator *= (float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	TVec &operator /= (float scale)
	{
		x /= scale;
		y /= scale;
		z /= scale;
		return *this;
	}

	TVec operator + (void) const
	{
		return *this;
	}

	TVec operator - (void) const
	{
		return TVec(-x,	-y, -z);
	}

	float Length(void) const
	{
		return sqrt(x * x + y * y + z * z);
	}
};

inline TVec operator + (const TVec &v1, const TVec &v2)
{
	return TVec(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

inline TVec operator - (const TVec &v1, const TVec &v2)
{
	return TVec(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

inline TVec operator * (const TVec& v, float s)
{
	return TVec(s * v.x, s * v.y, s * v.z);
}

inline TVec operator * (float s, const TVec& v)
{
	return TVec(s * v.x, s * v.y, s * v.z);
}

inline TVec operator / (const TVec& v, float s)
{
	return TVec(v.x / s, v.y / s, v.z / s);
}

inline bool operator == (const TVec& v1, const TVec& v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

inline bool operator != (const TVec& v1, const TVec& v2)
{
	return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z;
}

inline float Length(const TVec &v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline TVec Normalize(const TVec& v)
{
   return v / v.Length();
}

inline float DotProduct(const TVec& v1, const TVec& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline TVec CrossProduct(const TVec &v1, const TVec &v2)
{
	return TVec(v1.y * v2.z - v1.z * v2.y,
				v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x);
}

inline ostream &operator << (ostream &strm, const TVec &v)
{
	return strm << '(' << v.x << ',' << v.y << ',' << v.z << ')';
}

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
