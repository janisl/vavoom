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
//**
//**	Vavoom global name types.
//**
//**************************************************************************

//	Maximum length of a name
enum { NAME_SIZE = 64 };

//
//	VNameEntry
//
//	Entry in the names table.
//
struct VNameEntry
{
	VNameEntry*		HashNext;			//	Next name for this hash list.
	vint32			Index;				//	Index of the name.
	char			Name[NAME_SIZE];	//	Name value.

	friend VStream& operator<<(VStream&, VNameEntry&);
	friend VNameEntry* AllocateNameEntry(const char* Name, vint32 Index, 
		VNameEntry* HashNext);
};

//
//	VName
//
//	Names are stored as indexes in the global name table. They are stored once
// and only once. All names are case-sensitive.
//
class VName
{
protected:
	vint32						Index;

	static TArray<VNameEntry*>	Names;
	static VNameEntry*			HashTable[4096];
	static bool					Initialised;

public:
	//	Different types of finding a name.
	enum ENameFindType
	{
		Find,		// Find a name, return 0 if it doesn't exist.
		Add,		// Find a name, add it if it doesn't exist.
		AddLower8,	// Find or add lowercased, max length 8 name.
	};

	//	Constructors.
	VName()
	{}
	VName(EName N) : Index(N)
	{}
	VName(const char*, ENameFindType = Add);

	//	Ancestors.
	const char* operator*() const
	{
		return Names[Index]->Name;
	}
	vint32 GetIndex() const
	{
		return Index;
	}

	//	Comparison operators.
	bool operator==(const VName& Other) const
	{
		return Index == Other.Index;
	}
	bool operator!=(const VName& Other) const
	{
		return Index != Other.Index;
	}

	//	Global functions.
	static void StaticInit();
	static void StaticExit();

	static int GetNumNames()
	{
		return Names.Num();
	}
	static VNameEntry* GetEntry(int i)
	{
		return Names[i];
	}
	static const char* SafeString(EName N)
	{
		return Initialised ? Names[N]->Name : "Uninitialised";
	}
};

inline vuint32 GetTypeHash(const VName N)
{
	return N.GetIndex();
}

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//
//	Revision 1.6  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.5  2005/11/22 19:10:38  dj_jl
//	Cleaned up a bit.
//	
//	Revision 1.4  2005/05/26 16:49:14  dj_jl
//	Added lowercased max 8 chars names.
//	
//	Revision 1.3  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/18 18:57:11  dj_jl
//	Added global name subsystem
//	
//**************************************************************************
