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

enum
{
	PROPTYPE_Reference,
	PROPTYPE_ClassID,
	PROPTYPE_Name,
	PROPTYPE_String,
};

struct FPropertyInfo
{
	int			Type;
	int			Offset;
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
	FFunction	**ClassVTable;
	void (*ClassConstructor)(void*);

	int			ClassNumMethods;

	int				NumPropertyInfo;
	FPropertyInfo	*PropertyInfo;

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

	FFunction *FindFunction(FName InName);
	FFunction *FindFunctionChecked(FName InName);
	int GetFunctionIndex(FName InName);
};

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2002/05/03 17:06:23  dj_jl
//	Mangling of string pointers.
//
//	Revision 1.6  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.5  2002/02/26 17:54:26  dj_jl
//	Importing special property info from progs and using it in saving.
//	
//	Revision 1.4  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.3  2002/01/11 08:15:40  dj_jl
//	Removed FFunction
//	
//	Revision 1.2  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/27 17:35:42  dj_jl
//	Split VClass in seperate module
//	
//**************************************************************************
