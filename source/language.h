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

class VLanguage
{
public:
	VLanguage();
	~VLanguage();

	void FreeData();
	void LoadStrings(const char*);

	VStr Find(VName) const;
	VStr operator[](VName) const;

	VName GetStringId(const VStr&);
	void ReplaceString(VName, const VStr&);

private:
	struct VLangEntry;

	TMap<VName, VLangEntry>*	Table;

	void FreeNonDehackedStrings();
	void ParseLanguageScript(vint32, const char*, bool, vint32);
	VStr HandleEscapes(VStr);
};

extern VLanguage		GLanguage;
