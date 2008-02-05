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
//	VField
//
//==========================================================================

class VField : public VMemberBase
{
public:
	//	Persistent fields
	VField*			Next;
	VFieldType		Type;
	VMethod*		Func;
	vuint32			Flags;
	VMethod*		ReplCond;

	//	Compiler fields
	VExpression*	TypeExpr;

	//	Run-time fields
	VField*		NextReference;	//	Linked list of reference fields.
	VField*		DestructorLink;
	VField*		NextNetField;
	vint32		Ofs;
	vint32		NetIndex;

	VField(VName, VMemberBase*, TLocation);
	~VField();

	void Serialise(VStream&);
	bool NeedsDestructor() const;
	bool Define();

#ifndef IN_VCC
	static void CopyFieldValue(const vuint8*, vuint8*, const VFieldType&);
	static void SerialiseFieldValue(VStream&, vuint8*, const VFieldType&);
	static void CleanField(vuint8*, const VFieldType&);
	static void DestructField(vuint8*, const VFieldType&);
	static bool IdenticalValue(const vuint8*, const vuint8*, const VFieldType&);
	static bool NetSerialiseValue(VStream&, VNetObjectsMap*, vuint8*, const VFieldType&);
#endif

	friend inline VStream& operator<<(VStream& Strm, VField*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};
