//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

class  TVec
{
 public:
	float		x;
	float		y;

	TVec(void) { }

	TVec(float Ax, float Ay)
	{
		x = Ax;
		y = Ay;
	}

	TVec (const float f[2])
	{
    	x = f[0];
    	y = f[1];
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
		return *this;
	}

	TVec &operator -= (const TVec &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	TVec &operator *= (float scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	TVec &operator /= (float scale)
	{
		x /= scale;
		y /= scale;
		return *this;
	}

	TVec operator + (void) const
	{
		return *this;
	}

	TVec operator - (void) const
	{
		return TVec(-x,	-y);
	}

	float Length(void) const
	{
		return sqrt(x * x + y * y);
	}
};

inline TVec operator + (const TVec &v1, const TVec &v2)
{
	return TVec(v1.x + v2.x, v1.y + v2.y);
}

inline TVec operator - (const TVec &v1, const TVec &v2)
{
	return TVec(v1.x - v2.x, v1.y - v2.y);
}

inline TVec operator * (const TVec& v, float s)
{
	return TVec(s * v.x, s * v.y);
}

inline TVec operator * (float s, const TVec& v)
{
	return TVec(s * v.x, s * v.y);
}

inline TVec operator / (const TVec& v, float s)
{
	return TVec(v.x / s, v.y / s);
}

inline bool operator == (const TVec& v1, const TVec& v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

inline bool operator != (const TVec& v1, const TVec& v2)
{
	return v1.x != v2.x || v1.y != v2.y;
}

inline float Length(const TVec &v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

inline TVec Normalize(const TVec& v)
{
   return v / v.Length();
}

inline float DotProduct(const TVec& v1, const TVec& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

inline ostream &operator << (ostream &strm, const TVec &v)
{
	return strm << '(' << v.x << ',' << v.y << ')';
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
		normal = Normalize(TVec(dir.y, -dir.x));
		dist = DotProduct(point, normal);
	}

	//	Initializes vertical plane from 2 points
	void Set2Points(const TVec &v1, const TVec &v2)
	{
		SetPointDir(v1, v2 - v1);
	}
};

