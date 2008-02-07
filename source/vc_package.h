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

class VProgsReader;

//==========================================================================
//
//	mobjinfo_t
//
//==========================================================================

struct mobjinfo_t
{
	int			DoomEdNum;
	vint32		GameFilter;
	VClass*		Class;

	friend VStream& operator<<(VStream&, mobjinfo_t&);
};

//==========================================================================
//
//	VImportedPackage
//
//==========================================================================

struct VImportedPackage
{
	VName		Name;
	TLocation	Loc;
	VPackage*	Pkg;
};

//==========================================================================
//
//	VPackage
//
//==========================================================================

class VPackage : public VMemberBase
{
private:
	struct TStringInfo
	{
		int		Offs;
		int		Next;
	};

	TArray<TStringInfo>			StringInfo;
	int							StringLookup[256];

	static int StringHashFunc(const char*);

public:
	//	Shared fields
	TArray<char>				Strings;

	//	Compiler fields
	TArray<VImportedPackage>	PackagesToLoad;

	TArray<mobjinfo_t>			MobjInfo;
	TArray<mobjinfo_t>			ScriptIds;

	TArray<VConstant*>			ParsedConstants;
	TArray<VStruct*>			ParsedStructs;
	TArray<VClass*>				ParsedClasses;

	int							NumBuiltins;

	//	Run-time fields
	vuint16						Checksum;
	VProgsReader*				Reader;

	VPackage();
	VPackage(VName InName);
	~VPackage();

	void Serialise(VStream&);

	int FindString(const char*);
	VConstant* FindConstant(VName);

	void Emit();
	void WriteObject(const VStr&);

	friend inline VStream& operator<<(VStream& Strm, VPackage*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};
