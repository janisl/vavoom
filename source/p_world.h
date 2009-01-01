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

//**************************************************************************
//
//	BLOCK MAP ITERATORS
//
//	For each line/thing in the given mapblock, call the passed PIT_*
// function. If the function returns false, exit with false without checking
// anything else.
//
//**************************************************************************

//==========================================================================
//
//	VBlockLinesIterator
//
//	The validcount flags are used to avoid checking lines that are marked in
// multiple mapblocks, so increment validcount before the first call to
// SV_BlockLinesIterator, then make one or more calls to it.
//
//==========================================================================

class VBlockLinesIterator
{
private:
	VLevel*			Level;
	line_t**		LinePtr;
	polyblock_t*	PolyLink;
	vint32			PolySegIdx;
	vint32*			List;

public:
	VBlockLinesIterator(VLevel*, int, int, line_t**);
	bool GetNext();
};

//==========================================================================
//
//	VBlockThingsIterator
//
//==========================================================================

class VBlockThingsIterator
{
private:
	VEntity*	Ent;

public:
	VBlockThingsIterator(VLevel* Level, int x, int y)
	{
		if (x < 0 || x >= Level->BlockMapWidth ||
			y < 0 || y >= Level->BlockMapHeight)
		{
			Ent = NULL;
		}
		else
		{
			Ent = Level->BlockLinks[y * Level->BlockMapWidth + x];
		}
	}
	operator bool() const
	{
		return !!Ent;
	}
	void operator ++()
	{
		Ent = Ent->BlockMapNext;
	}
	VEntity* operator *() const
	{
		return Ent;
	}
	VEntity* operator ->() const
	{
		return Ent;
	}
};

//==========================================================================
//
//	VRadiusThingsIterator
//
//==========================================================================

class VRadiusThingsIterator : public VScriptIterator
{
private:
	VThinker*	Self;
	VEntity**	EntPtr;
	VEntity*	Ent;
	int			x;
	int			y;
	int			xl;
	int			xh;
	int			yl;
	int			yh;

public:
	VRadiusThingsIterator(VThinker*, VEntity**, TVec, float);
	bool GetNext();
};

//==========================================================================
//
//	VPathTraverse
//
//	Traces a line from x1,y1 to x2,y2, calling the traverser function for
// each. Returns true if the traverser function returns true for all lines.
//
//==========================================================================

class VPathTraverse : public VScriptIterator
{
private:
	TArray<intercept_t>		Intercepts;

	TPlane					trace_plane;
	TVec					trace_org;
	TVec					trace_dest;
	TVec					trace_delta;
	TVec					trace_dir;
	float					trace_len;

	int						Count;
	intercept_t*			In;
	intercept_t**			InPtr;

public:
	VPathTraverse(VThinker*, intercept_t**, float, float, float, float, int);
	bool GetNext();

private:
	void Init(VThinker*, float, float, float, float, int);
	bool AddLineIntercepts(VThinker*, int, int, bool);
	void AddThingIntercepts(VThinker*, int, int);
};

