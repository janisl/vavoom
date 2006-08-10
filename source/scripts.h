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

class VScriptParser
{
public:
	VScriptParser(const VStr&, VStream*);
	~VScriptParser();

	int 		Line;
	bool		End;
	bool		Crossed;
	VStr		String;
	VName		Name8;
	int 		Number;
	double 		Float;

	bool AtEnd();
	bool GetString();
	void ExpectString();
	void ExpectName8();
	bool Check(const char*);
	void Expect(const char*);
	bool CheckNumber();
	void ExpectNumber();
	bool CheckFloat();
	void ExpectFloat();
	void UnGet();
	void Error(const char*);

private:
	VStr		ScriptName;
	char*		ScriptBuffer;
	char*		ScriptPtr;
	char*		ScriptEndPtr;
	int			ScriptSize;
	bool		AlreadyGot;
};
