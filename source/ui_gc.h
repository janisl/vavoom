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

//==========================================================================
//
//	VClipRect - clipping rectangle class
//
//==========================================================================

struct VClipRect
{
	// Default constructor
	VClipRect()
	{ 
		OriginX    = 0;
		OriginY    = 0; 
		ClipX      = 0;
		ClipY      = 0; 
		ClipWidth  = 0;
		ClipHeight = 0;
	}

	// Other convenience constructors
	VClipRect(float NewClipX,     float NewClipY,
	          float NewClipWidth, float NewClipHeight)
	{
		OriginX    = NewClipX;
		OriginY    = NewClipY;

		ClipX      = 0;
		ClipY      = 0;
		ClipWidth  = NewClipWidth;
		ClipHeight = NewClipHeight;
	}

	VClipRect(float NewOriginX,   float NewOriginY,
	          float NewClipX,     float NewClipY,
	          float NewClipWidth, float NewClipHeight)
	{
		OriginX    = NewOriginX;
		OriginY    = NewOriginY;
		ClipX      = NewClipX;
		ClipY      = NewClipY;
		ClipWidth  = NewClipWidth;
		ClipHeight = NewClipHeight;
	}

	VClipRect(VClipRect &Rect1, VClipRect &Rect2)
	{
		OriginX    = Rect1.OriginX;
		OriginY    = Rect1.OriginY;
		ClipX      = Rect1.ClipX;
		ClipY      = Rect1.ClipY;
		ClipWidth  = Rect1.ClipWidth;
		ClipHeight = Rect1.ClipHeight;
		Intersect(Rect2);
	}

	// Public methods
	void SetOrigin(float NewOriginX, float NewOriginY)
	{
		ClipX   += (OriginX - NewOriginX);
		ClipY   += (OriginY - NewOriginY);
		OriginX =  NewOriginX;
		OriginY =  NewOriginY;
	}

	void MoveOrigin(float NewDeltaX, float NewDeltaY)
	{
		SetOrigin(OriginX + NewDeltaX, OriginY + NewDeltaY);
	}

	void Intersect(float NewClipX,     float NewClipY,
	               float NewClipWidth, float NewClipHeight)
	{
		VClipRect TempRect(NewClipX + OriginX, NewClipY + OriginY, 
			NewClipWidth, NewClipHeight);
		Intersect(TempRect);
	}

	void Intersect(VClipRect &NewRect)
	{
		float fromX1, fromY1;
		float fromX2, fromY2;
		float toX1,   toY1;
		float toX2,   toY2;

		// Convert everything to absolute coordinates
		fromX1 = ClipX         + OriginX;
		fromY1 = ClipY         + OriginY;
		fromX2 = NewRect.ClipX + NewRect.OriginX;
		fromY2 = NewRect.ClipY + NewRect.OriginY;
		toX1   = fromX1        + ClipWidth;
		toY1   = fromY1        + ClipHeight;
		toX2   = fromX2        + NewRect.ClipWidth;
		toY2   = fromY2        + NewRect.ClipHeight;

		// Clip
		if (fromX1 < fromX2)
			fromX1 = fromX2;
		if (fromY1 < fromY2)
			fromY1 = fromY2;
		if (toX1 > toX2)
			toX1 = toX2;
		if (toY1 > toY2)
			toY1 = toY2;

		// Reconvert to origin of this object
		ClipX      = fromX1 - OriginX;
		ClipY      = fromY1 - OriginY;
		ClipWidth  = toX1   - fromX1;
		ClipHeight = toY1   - fromY1;
	}

	bool HasArea(void)
	{
		return ((ClipWidth > 0) && (ClipHeight > 0));
	}

	// Members
	float OriginX;		// X origin of rectangle, in absolute coordinates
	float OriginY;		// Y origin of rectangle, in absolute coordinates

	float ClipX;		// Leftmost edge of rectangle relative to origin
	float ClipY;		// Topmost edge of rectangle relative to origin
	float ClipWidth;	// Width of rectangle
	float ClipHeight;	// Height of rectangle
};

class VGC : public VObject
{
#ifdef ZONE_DEBUG_NEW
#undef new
#endif
	DECLARE_CLASS(VGC, VObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VGC)
#ifdef ZONE_DEBUG_NEW
#define new ZONE_DEBUG_NEW
#endif

	friend class VWindow;

	VClipRect ClipRect;

	void SetClipRect(const VClipRect &NewClipRect);
};
