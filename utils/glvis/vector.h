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

class  TVec
{
 public:
	double		x;
	double		y;

	TVec(void) { }

	TVec(double Ax, double Ay)
	{
		x = Ax;
		y = Ay;
	}

	TVec (const double f[2])
	{
		x = f[0];
		y = f[1];
	}

	const double& operator[](int i) const
	{
		return (&x)[i];
	}

	double& operator[](int i)
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

	TVec &operator *= (double scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	TVec &operator /= (double scale)
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

	double Length(void) const
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

inline TVec operator * (const TVec& v, double s)
{
	return TVec(s * v.x, s * v.y);
}

inline TVec operator * (double s, const TVec& v)
{
	return TVec(s * v.x, s * v.y);
}

inline TVec operator / (const TVec& v, double s)
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

inline double Length(const TVec &v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

inline TVec Normalize(const TVec& v)
{
   return v / v.Length();
}

inline double DotProduct(const TVec& v1, const TVec& v2)
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
	double		dist;

	void Set(const TVec &Anormal, double Adist)
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

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//
//	Revision 1.4  2002/01/03 18:35:14  dj_jl
//	Switched to doubles, some fixes
//	
//	Revision 1.3  2001/08/24 17:08:34  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
