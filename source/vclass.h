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

struct FFunction
{
	FName	Name;
	int		FirstStatement;
	short	NumParms;
	short	NumLocals;
	short	Type;
	short	Flags;
};

//==========================================================================
//
//	VClass
//
//==========================================================================

class VClass:public VObject
{
	DECLARE_CLASS(VClass, VObject, 0)

	VClass*		ParentClass;

	size_t		ClassSize;
	dword		ClassFlags;
	int*		ClassVTable;
	void (*ClassConstructor)(void*);

	FFunction*	Methods;
	int			ClassNumMethods;
	byte*		PropretiesInfo;

	static VClass *FindClass(const char *);

	VClass(void);
	VClass(ENativeConstructor, size_t ASize, dword AClassFlags,
		VClass *AParent, const char *AName, int AFlags, 
		void(*ACtor)(void*));

	bool IsChildOf(const VClass *SomeBaseClass) const
	{
		for (const VClass *c = this; c; c = c->GetSuperClass())
		{
			if (SomeBaseClass == c)
			{
				return true;
			}
		}
		return false;
	}

	VClass *GetSuperClass(void) const
	{
		return ParentClass;
	}
};

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//
//	Revision 1.1  2001/12/27 17:35:42  dj_jl
//	Split VClass in seperate module
//	
//**************************************************************************
