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
//	TLocation
//
//	Describes location in a source file.
//
//==========================================================================

class TLocation
{
private:
	int		Loc;

	static TArray<VStr>		SourceFiles;

public:
	TLocation()
	: Loc(0)
	{}
	TLocation(int SrcIdx, int Line)
	: Loc((SrcIdx << 16) | Line)
	{}
	int GetLine() const
	{
		return Loc & 0xffff;
	}
	VStr GetSource() const;

	static int AddSourceFile(const VStr&);
	static void ClearSourceFiles();
};
