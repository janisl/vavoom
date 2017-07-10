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

class VViewClipper
{
private:
	struct VClipNode;

	VClipNode*		FreeClipNodes;
	VClipNode*		ClipHead;
	VClipNode*		ClipTail;
	TVec			Origin;
	VLevel*			Level;

	VClipNode* NewClipNode();
	void RemoveClipNode(VClipNode*);
	void DoAddClipRange(float, float);
	bool DoIsRangeVisible(float, float);

public:
	VViewClipper();
	~VViewClipper();
	void ClearClipNodes(const TVec&, VLevel*);
	void ClipInitFrustrumRange(const TAVec&, const TVec&, const TVec&,
		const TVec&, float, float);
	void ClipToRanges(const VViewClipper&);
	void AddClipRange(float, float);
	bool IsRangeVisible(float, float);
	bool ClipIsFull();
	float PointToClipAngle(const TVec&);
	bool ClipIsBBoxVisible(float*, bool, const TVec& = TVec(0, 0, 0), float = 0);
	bool ClipCheckRegion(subregion_t*, subsector_t*, bool, const TVec& = TVec(0, 0, 0), float = 0);
	bool ClipCheckSubsector(subsector_t*, bool, const TVec& = TVec(0, 0, 0), float = 0);
	void ClipAddSubsectorSegs(subsector_t*, bool, TPlane* = NULL, const TVec& = TVec(0, 0, 0), float = 0);
};
