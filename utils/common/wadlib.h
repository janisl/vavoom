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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct lumpinfo_t
{
	char	name[12];
	int		position;
	int		size;
};

class TIWadFile
{
 public:
	TIWadFile(void)
	{
		handle = NULL;
	}
	~TIWadFile(void)
	{
		if (handle)
		{
			Close();
		}
	}
	void Open(const char* filename);
	int LumpNumForName(const char* name);
	const char* LumpName(int lump)
	{
		return lump >= numlumps ? "" : lumpinfo[lump].name;
	}
	int LumpSize(int lump)
	{
		return lump >= numlumps ? 0 : lumpinfo[lump].size;
	}
	void* GetLump(int lump);
	void* GetLumpName(const char* name)
	{
		return GetLump(LumpNumForName(name));
	}
	void Close(void);

	FILE*			handle;
	char			wadid[4];
	lumpinfo_t*		lumpinfo;
	int				numlumps;
};

class TOWadFile
{
 public:
	TOWadFile(void)
	{
		handle = NULL;
	}
	~TOWadFile(void)
	{
		if (handle)
		{
			fclose(handle);
		}
	}
	void Open(const char *filename, const char *Awadid);
	void AddLump(const char *name, const void *data, int size);
	void Close(void);

	FILE*			handle;
	char			wadid[4];
	lumpinfo_t*		lumpinfo;
	int				numlumps;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//==========================================================================
//
//	CleanupName
//
//==========================================================================

inline void CleanupName(const char *src, char *dst)
{
	int i;
	for (i = 0; i < 8 && src[i]; i++)
	{
		dst[i] = toupper(src[i]);
	}
	for (; i < 12; i++)
	{
		dst[i] = 0;
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/08/30 17:47:47  dj_jl
//	Overflow protection
//
//	Revision 1.3  2001/08/24 17:08:34  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
