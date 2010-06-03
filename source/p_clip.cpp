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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct VViewClipper::VClipNode
{
	float		From;
	float		To;
	VClipNode*	Prev;
	VClipNode*	Next;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VViewClipper::VViewClipper
//
//==========================================================================

VViewClipper::VViewClipper()
: FreeClipNodes(NULL)
, ClipHead(NULL)
, ClipTail(NULL)
{
}

//==========================================================================
//
//	VViewClipper::~VViewClipper
//
//==========================================================================

VViewClipper::~VViewClipper()
{
	guard(VViewClipper::~VViewClipper);
	ClearClipNodes(TVec(), NULL);
	VClipNode* Node = FreeClipNodes;
	while (Node)
	{
		VClipNode* Next = Node->Next;
		delete Node;
		Node = NULL;
		Node = Next;
	}
	unguard;
}

//==========================================================================
//
//	VViewClipper::NewClipNode
//
//==========================================================================

VViewClipper::VClipNode* VViewClipper::NewClipNode()
{
	guard(VViewClipper::NewClipNode);
	VClipNode* Ret = FreeClipNodes;
	if (Ret)
	{
		FreeClipNodes = Ret->Next;
	}
	else
	{
		Ret = new VClipNode();
	}
	return Ret;
	unguard;
}

//==========================================================================
//
//	VViewClipper::RemoveClipNode
//
//==========================================================================

void VViewClipper::RemoveClipNode(VViewClipper::VClipNode* Node)
{
	guard(VViewClipper::RemoveClipNode);
	if (Node->Next)
	{
		Node->Next->Prev = Node->Prev;
	}
	if (Node->Prev)
	{
		Node->Prev->Next = Node->Next;
	}
	if (Node == ClipHead)
	{
		ClipHead = Node->Next;
	}
	if (Node == ClipTail)
	{
		ClipTail = Node->Prev;
	}
	Node->Next = FreeClipNodes;
	FreeClipNodes = Node;
	unguard;
}

//==========================================================================
//
//	VViewClipper::ClearClipNodes
//
//==========================================================================

void VViewClipper::ClearClipNodes(const TVec& AOrigin, VLevel* ALevel)
{
	guard(VViewClipper::ClearClipNodes);
	if (ClipHead)
	{
		ClipTail->Next = FreeClipNodes;
		FreeClipNodes = ClipHead;
	}
	ClipHead = NULL;
	ClipTail = NULL;
	Origin = AOrigin;
	Level = ALevel;
	unguard;
}

//==========================================================================
//
//	VViewClipper::ClipInitFrustrumRange
//
//==========================================================================

void VViewClipper::ClipInitFrustrumRange(const TAVec& viewangles,
	const TVec& viewforward, const TVec& viewright, const TVec& viewup,
	float fovx, float fovy)
{
	guard(VViewClipper::ClipInitFrustrumRange);
	check(!ClipHead);

	if (viewforward.z > 0.9 || viewforward.z < -0.9)
	{
		//	Looking up or down, can see behind.
		return;
	}

	TVec Pts[4];
	TVec TransPts[4];
	Pts[0] = TVec(1, fovx, fovy);
	Pts[1] = TVec(1, fovx, -fovy);
	Pts[2] = TVec(1, -fovx, fovy);
	Pts[3] = TVec(1, -fovx, -fovy);
	TVec clipforward = Normalise(TVec(viewforward.x, viewforward.y, 0.0));
	float d1 = 0;
	float d2 = 0;
	for (int i = 0; i < 4; i++)
	{
		TransPts[i].x = Pts[i].y * viewright.x + Pts[i].z * viewup.x + Pts[i].x * viewforward.x;
		TransPts[i].y = Pts[i].y * viewright.y + Pts[i].z * viewup.y + Pts[i].x * viewforward.y;
		TransPts[i].z = Pts[i].y * viewright.z + Pts[i].z * viewup.z + Pts[i].x * viewforward.z;
		if (DotProduct(TransPts[i], clipforward) <= 0)
		{
			//	Player can see behind.
			return;
		}
		float a = matan(TransPts[i].y, TransPts[i].x);
		if (a < 0)
			a += 360.0;
		float d = AngleMod180(a - viewangles.yaw);
		if (d1 > d)
			d1 = d;
		if (d2 < d)
			d2 = d;
	}
	float a1 = AngleMod(viewangles.yaw + d1);
	float a2 = AngleMod(viewangles.yaw + d2);
	if (a1 > a2)
	{
		ClipHead = NewClipNode();
		ClipTail = ClipHead;
		ClipHead->From = a2;
		ClipHead->To = a1;
		ClipHead->Prev = NULL;
		ClipHead->Next = NULL;
	}
	else
	{
		ClipHead = NewClipNode();
		ClipHead->From = 0.0;
		ClipHead->To = a1;
		ClipTail = NewClipNode();
		ClipTail->From = a2;
		ClipTail->To = 360.0;
		ClipHead->Prev = NULL;
		ClipHead->Next = ClipTail;
		ClipTail->Prev = ClipHead;
		ClipTail->Next = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VViewClipper::ClipToRanges
//
//==========================================================================

void VViewClipper::ClipToRanges(const VViewClipper& Range)
{
	guard(VViewClipper::ClipToRanges);
	if (!Range.ClipHead)
	{
		//	No ranges, everything is clipped away.
		DoAddClipRange(0.0, 360.0);
		return;
	}

	//	Add head and tail ranges.
	if (Range.ClipHead->From > 0.0)
	{
		DoAddClipRange(0.0, Range.ClipHead->From);
	}
	if (Range.ClipTail->To < 360.0)
	{
		DoAddClipRange(Range.ClipTail->To, 360.0);
	}

	//	Add middle ranges.
	for (VClipNode* N = Range.ClipHead; N->Next; N = N->Next)
	{
		DoAddClipRange(N->To, N->Next->From);
	}
	unguard;
}

//==========================================================================
//
//	VViewClipper::DoAddClipRange
//
//==========================================================================

void VViewClipper::DoAddClipRange(float From, float To)
{
	guard(VViewClipper::DoAddClipRange);
	if (!ClipHead)
	{
		ClipHead = NewClipNode();
		ClipTail = ClipHead;
		ClipHead->From = From;
		ClipHead->To = To;
		ClipHead->Prev = NULL;
		ClipHead->Next = NULL;
		return;
	}

	for (VClipNode* Node = ClipHead; Node; Node = Node->Next)
	{
		if (Node->To < From)
		{
			//	Before this range.
			continue;
		}

		if (To < Node->From)
		{
			//	Insert a new clip range before current one.
			VClipNode* N = NewClipNode();
			N->From = From;
			N->To = To;
			N->Prev = Node->Prev;
			N->Next = Node;
			if (Node->Prev)
			{
				Node->Prev->Next = N;
			}
			else
			{
				ClipHead = N;
			}
			Node->Prev = N;
			return;
		}

		if (Node->From <= From && Node->To >= To)
		{
			//	It contains this range.
			return;
		}

		if (From < Node->From)
		{
			//	Extend start of the current range.
			Node->From = From;
		}
		if (To <= Node->To)
		{
			//	End is included, so we are done here.
			return;
		}

		//	Merge with following nodes if needed.
		while (Node->Next && Node->Next->From <= To)
		{
			Node->To = Node->Next->To;
			RemoveClipNode(Node->Next);
		}
		if (To > Node->To)
		{
			//	Extend end.
			Node->To = To;
		}
		//	We are done here.
		return;
	}

	//	If we are here it means it's a new range at the end.
	VClipNode* NewTail = NewClipNode();
	NewTail->From = From;
	NewTail->To = To;
	NewTail->Prev = ClipTail;
	NewTail->Next = NULL;
	ClipTail->Next = NewTail;
	ClipTail = NewTail;
	unguard;
}

//==========================================================================
//
//	VViewClipper::AddClipRange
//
//==========================================================================

void VViewClipper::AddClipRange(float From, float To)
{
	guard(VViewClipper::AddClipRange);
	if (From > To)
	{
		DoAddClipRange(0.0, To);
		DoAddClipRange(From, 360.0);
	}
	else
	{
		DoAddClipRange(From, To);
	}
	unguard;
}

//==========================================================================
//
//	VViewClipper::DoIsRangeVisible
//
//==========================================================================

bool VViewClipper::DoIsRangeVisible(float From, float To)
{
	guard(VViewClipper::DoIsRangeVisible);
	for (VClipNode* N = ClipHead; N; N = N->Next)
	{
		if (From >= N->From && To <= N->To)
		{
			return false;
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VViewClipper::IsRangeVisible
//
//==========================================================================

bool VViewClipper::IsRangeVisible(float From, float To)
{
	guard(VViewClipper::IsRangeVisible);
	if (From > To)
	{
		return DoIsRangeVisible(0.0, To) || DoIsRangeVisible(From, 360.0);
	}
	else
	{
		return DoIsRangeVisible(From, To);
	}
	unguard;
}

//==========================================================================
//
//	VViewClipper::ClipIsFull
//
//==========================================================================

bool VViewClipper::ClipIsFull()
{
	guard(VViewClipper::ClipIsFull);
	return ClipHead && ClipHead->From == 0.0 && ClipHead->To == 360.0;
	unguard;
}

//==========================================================================
//
//	VViewClipper::PointToClipAngle
//
//==========================================================================

float VViewClipper::PointToClipAngle(const TVec& Pt)
{
	float Ret = matan(Pt.y - Origin.y, Pt.x - Origin.x);
	if (Ret < 0)
		Ret += 360.0;
	return Ret;
}

//==========================================================================
//
//	VViewClipper::ClipIsBBoxVisible
//
//==========================================================================

bool VViewClipper::ClipIsBBoxVisible(float* BBox)
{
	guard(VViewClipper::ClipIsBBoxVisible);
	if (!ClipHead)
	{
		//	No clip nodes yet.
		return true;
	}
	if (BBox[0] <= Origin.x && BBox[3] >= Origin.x &&
		BBox[1] <= Origin.y && BBox[4] >= Origin.y)
	{
		//	Viewer is inside the box.
		return true;
	}

	TVec v1;
	TVec v2;
	if (BBox[0] > Origin.x)
	{
		if (BBox[1] > Origin.y)
		{
			v1.x = BBox[3];
			v1.y = BBox[1];
			v2.x = BBox[0];
			v2.y = BBox[4];
		}
		else if (BBox[4] < Origin.y)
		{
			v1.x = BBox[0];
			v1.y = BBox[1];
			v2.x = BBox[3];
			v2.y = BBox[4];
		}
		else
		{
			v1.x = BBox[0];
			v1.y = BBox[1];
			v2.x = BBox[0];
			v2.y = BBox[4];
		}
	}
	else if (BBox[3] < Origin.x)
	{
		if (BBox[1] > Origin.y)
		{
			v1.x = BBox[3];
			v1.y = BBox[4];
			v2.x = BBox[0];
			v2.y = BBox[1];
		}
		else if (BBox[4] < Origin.y)
		{
			v1.x = BBox[0];
			v1.y = BBox[4];
			v2.x = BBox[3];
			v2.y = BBox[1];
		}
		else
		{
			v1.x = BBox[3];
			v1.y = BBox[4];
			v2.x = BBox[3];
			v2.y = BBox[1];
		}
	}
	else
	{
		if (BBox[1] > Origin.y)
		{
			v1.x = BBox[3];
			v1.y = BBox[1];
			v2.x = BBox[0];
			v2.y = BBox[1];
		}
		else
		{
			v1.x = BBox[0];
			v1.y = BBox[4];
			v2.x = BBox[3];
			v2.y = BBox[4];
		}
	}
	return IsRangeVisible(PointToClipAngle(v1), PointToClipAngle(v2));
	unguard;
}

//==========================================================================
//
//	VViewClipper::ClipCheckSubsector
//
//==========================================================================

bool VViewClipper::ClipCheckSubsector(subsector_t* Sub)
{
	guard(VViewClipper::ClipCheckSubsector);
	if (!ClipHead)
	{
		return true;
	}
	for (int i = 0; i < Sub->numlines; i++)
	{
		seg_t* line = &Level->Segs[Sub->firstline + i];

		float dist = DotProduct(Origin, line->normal) - line->dist;
		if (dist <= 0)
		{
			//	Viewer is in back side or on plane
			continue;
		}

		if (IsRangeVisible(PointToClipAngle(*line->v2),
			PointToClipAngle(*line->v1)))
		{
			return true;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VViewClipper::ClipAddSubsectorSegs
//
//==========================================================================

void VViewClipper::ClipAddSubsectorSegs(subsector_t* Sub, TPlane* Mirror)
{
	guard(VViewClipper::ClipAddSubsectorSegs);
	for (int i = 0; i < Sub->numlines; i++)
	{
		seg_t* line = &Level->Segs[Sub->firstline + i];
		if (line->backsector || !line->linedef)
		{
			//	Miniseg or two-sided line.
			continue;
		}

		float dist = DotProduct(Origin, line->normal) - line->dist;
		if (dist <= 0)
		{
			//	Viewer is in back side or on plane
			continue;
		}

		TVec v1 = *line->v1;
		TVec v2 = *line->v2;
		if (Mirror)
		{
			//	Clip seg with mirror plane.
			float Dist1 = DotProduct(v1, Mirror->normal) - Mirror->dist;
			float Dist2 = DotProduct(v2, Mirror->normal) - Mirror->dist;
			if (Dist1 <= 0 && Dist2 <= 0)
			{
				continue;
			}
			if (Dist1 > 0 && Dist2 < 0)
			{
				v2 = v1 + (v2 - v1) * Dist1 / (Dist1 - Dist2);
			}
			else if (Dist2 > 0 && Dist1 < 0)
			{
				v1 = v2 + (v1 - v2) * Dist2 / (Dist2 - Dist1);
			}
		}

		AddClipRange(PointToClipAngle(v2), PointToClipAngle(v1));
	}

	if (Sub->poly)
	{
		seg_t** polySeg = Sub->poly->segs;
		for (int polyCount = Sub->poly->numsegs; polyCount--; polySeg++)
		{
			seg_t* line = *polySeg;
			if (line->backsector || !line->linedef)
			{
				//	Miniseg or two-sided line.
				continue;
			}

			float dist = DotProduct(Origin, line->normal) - line->dist;
			if (dist <= 0)
			{
				//	Viewer is in back side or on plane
				continue;
			}

			TVec v1 = *line->v1;
			TVec v2 = *line->v2;

			AddClipRange(PointToClipAngle(v2), PointToClipAngle(v1));
		}
	}
	unguard;
}
