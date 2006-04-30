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
//**	Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//**	Remark: this was the only stuff that, according
//**  to John Carmack, might have been useful for Quake.
//**
//**************************************************************************

#define ZONE_DEBUG		1

// PU - purge tags.
enum EZoneTag
{
	// Tags < 100 are not overwritten until freed.
	PU_STATIC		= 1,	// static entire execution time
	PU_SOUND		= 2,	// static while playing
	PU_MUSIC		= 3,	// static while playing
	PU_VIDEO		= 5,	// high, return NULL on failure
	PU_HIGH			= 7,	// high (allocated from end of memory)
	PU_LEVEL		= 50,	// static until level exited
	PU_LEVSPEC		= 51,	// a special thinker in a level
	// Tags >= 100 are purgable whenever needed.
	PU_PURGELEVEL	= 100,
	PU_CACHE		= 101,
	PU_TEMP			= 102,	// high
};

class ZoneError : public VavoomError
{
public:
	explicit ZoneError(const char* text) : VavoomError(text) { }
};

void Z_Init(void* base, int size);
void Z_Shutdown();

void Z_CheckHeap();
int Z_FreeMemory();

void Z_FreeTag(int tag);

#ifdef ZONE_DEBUG

void* Z_Malloc(int size, int tag, void** ptr, const char* FileName, int LineNumber);
void* Z_Calloc(int size, int tag, void** user, const char* FileName, int LineNumber);

void Z_Resize(void** ptr, int size, const char* FileName, int LineNumber);
void Z_ChangeTag(void* ptr, int tag, const char* FileName, int LineNumber);

void Z_Free(void* ptr, const char* FileName, int LineNumber);

inline void* Z_New(int size, int tag, void** ptr, const char* FileName, int LineNumber)
{
	return Z_Malloc(size, tag, ptr, FileName, LineNumber);
}

inline void* Z_CNew(int size, int tag, void** ptr, const char* FileName, int LineNumber)
{
	return Z_Calloc(size, tag, ptr, FileName, LineNumber);
}

inline char *Z_StrDup(const char *src, const char* FileName, int LineNumber)
{
	int len = strlen(src);
	char *buf = (char*)Z_Malloc(len + 1, PU_STATIC, 0, FileName, LineNumber);
	strcpy(buf, src);
	return buf;
}

#define Z_Malloc(size, tag, ptr)	Z_Malloc(size, tag, ptr, __FILE__, __LINE__)
#define Z_Calloc(size, tag, user)	Z_Calloc(size, tag, user, __FILE__, __LINE__)

#define Z_Resize(ptr, size)			Z_Resize(ptr, size, __FILE__, __LINE__)
#define Z_ChangeTag(ptr, tag)		Z_ChangeTag(ptr, tag, __FILE__, __LINE__)

#define Z_Free(ptr)					Z_Free(ptr, __FILE__, __LINE__)

#define Z_New(T, elem_count, tag, owner)	(T*)Z_New(elem_count * sizeof(T), tag, owner, __FILE__, __LINE__)
#define Z_CNew(T, elem_count, tag, owner)	(T*)Z_CNew(elem_count * sizeof(T), tag, owner, __FILE__, __LINE__)

#define Z_StrDup(src)				Z_StrDup(src, __FILE__, __LINE__)

#else

void* Z_Malloc(int size, int tag, void** ptr);
void* Z_Calloc(int size, int tag, void** user);

void Z_Resize(void** ptr, int size);
void Z_ChangeTag(void* ptr, int tag);

void Z_Free(void* ptr);

inline char *Z_StrDup(const char *src)
{
	int len = strlen(src);
	char *buf = (char*)Z_Malloc(len + 1, PU_STATIC, 0);
	strcpy(buf, src);
	return buf;
}

#define Z_New(T, elem_count, tag, owner)	(T*)Z_Malloc(elem_count * sizeof(T), tag, owner)
#define Z_CNew(T, elem_count, tag, owner)	(T*)Z_Calloc(elem_count * sizeof(T), tag, owner)

#endif

inline void* operator new(size_t Size, EZoneTag Tag)
{
	return Z_Malloc(Size, Tag, NULL);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2005/05/26 16:56:26  dj_jl
//	Added operator new
//
//	Revision 1.8  2003/03/08 12:08:05  dj_jl
//	Beautification.
//	
//	Revision 1.7  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.5  2001/10/08 17:26:18  dj_jl
//	Started to use exceptions
//	
//	Revision 1.4  2001/09/12 17:34:42  dj_jl
//	Added memory allocation templates
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
