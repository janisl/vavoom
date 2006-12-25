//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: level.h 1741 2006-09-26 23:26:48Z dj_jl $
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

class VLanguage
{
public:
	VLanguage();
	~VLanguage();

	void LoadStrings(const char*);
	void FreeData();

	VStr Find(VName) const;
	VStr operator[](VName) const;

private:
	struct VLangEntry;

	enum { HASH_SIZE = 256 };

	VLangEntry*		HashTable[HASH_SIZE];

	void ParseLanguageScript(vint32, const char*, bool, vint32);
	VStr HandleEscapes(VStr);
};

extern VLanguage		GLanguage;
