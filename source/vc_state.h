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

//==========================================================================
//
//	VState
//
//==========================================================================

class VState : public VMemberBase
{
public:
	//
	// 	Frame flags:
	// 	handles maximum brightness (torches, muzzle flare, light sources)
	//
	enum { FF_FULLBRIGHT	= 0x80 };	// flag in Frame
	enum { FF_FRAMEMASK		= 0x7f };

	//	Persistent fields
	//	State info
	VName			SpriteName;
	vint32			Frame;
	float			Time;
	vint32			Misc1;
	vint32			Misc2;
	VState*			NextState;
	VMethod*		Function;
	//	Linked list of states
	VState*			Next;

	//	Compile time fields
	VName			GotoLabel;
	vint32			GotoOffset;
	VName			FunctionName;

	//	Run-time fields
	vint32			SpriteIndex;
	vint32			InClassIndex;
	vint32			NetId;
	VState*			NetNext;

	VState(VName, VMemberBase*, TLocation);
	~VState();

	void Serialise(VStream&);

	bool Define();
	void Emit();
	bool IsInRange(VState*, VState*, int);
	bool IsInSequence(VState*);
	VState* GetPlus(int, bool);

	friend inline VStream& operator<<(VStream& Strm, VState*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};
