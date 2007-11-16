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

//
// Fonts
//
class VFont
{
protected:
	struct FFontChar
	{
		int				Char;
		VTexture*		Tex;
	};

	VName				Name;
	VFont*				Next;

	//	Font characters
	TArray<FFontChar>	Chars;
	//	Fast look-up for ASCII characters.
	int					AsciiChars[128];
	//	Range of available characters.
	int					FirstChar;
	int					LastChar;

	//	Width of the space character.
	int					SpaceWidth;
	//	Height of the font.
	int					FontHeight;
	//	Additional distance betweeen characters.
	int					Kerning;

	rgba_t				Translation[256];

	static VFont*		Fonts;

	int FindChar(int) const;

	static void MarkUsedColours(VTexture*, bool*);

public:
	VFont(VName, const VStr&, int, int, int);

	VTexture* GetChar(int, int*) const;
	int GetCharWidth(int) const;

	int GetSpaceWidth() const
	{
		return SpaceWidth;
	}
	int GetHeight() const
	{
		return FontHeight;
	}
	int GetKerning() const
	{
		return Kerning;
	}

	static void StaticShutdown();
	static VFont* FindFont(VName);
};
