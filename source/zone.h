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
	PU_VIDEO		= 5,	// high, return NULL on failure
	PU_LEVEL		= 50,	// static until level exited
	PU_LEVSPEC		= 51,	// a special thinker in a level
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

inline void* operator new(size_t Size, const char* FileName, int LineNumber)
{
	return Z_Malloc(Size, PU_STATIC, 0, FileName, LineNumber);
}

inline void operator delete(void* Ptr, const char* FileName, int LineNumber)
{
	Z_Free(Ptr, FileName, LineNumber);
}

inline void* operator new[](size_t Size, const char* FileName, int LineNumber)
{
	return Z_Malloc(Size, PU_STATIC, 0, FileName, LineNumber);
}

inline void operator delete[](void* Ptr, const char* FileName, int LineNumber)
{
	Z_Free(Ptr, FileName, LineNumber);
}

inline void* operator new(size_t Size)
{
	return Z_Malloc(Size, PU_STATIC, 0, "", 0);
}

inline void operator delete(void* Ptr)
{
	Z_Free(Ptr, "", 0);
}

inline void* operator new[](size_t Size)
{
	return Z_Malloc(Size, PU_STATIC, 0, "", 0);
}

inline void operator delete[](void* Ptr)
{
	Z_Free(Ptr, "", 0);
}

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

#define ZONE_DEBUG_NEW				new(__FILE__, __LINE__)
#undef new
#define new							ZONE_DEBUG_NEW

#else

void* Z_Malloc(int size, int tag, void** ptr);
void* Z_Calloc(int size, int tag, void** user);

void Z_Resize(void** ptr, int size);
void Z_ChangeTag(void* ptr, int tag);

void Z_Free(void* ptr);

inline void* operator new(size_t Size)
{
	return Z_Malloc(Size, PU_STATIC, 0);
}

inline void operator delete(void* Ptr)
{
	Z_Free(Ptr);
}

inline void* operator new[](size_t Size)
{
	return Z_Malloc(Size, PU_STATIC, 0);
}

inline void operator delete[](void* Ptr)
{
	Z_Free(Ptr);
}

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

#ifdef ZONE_DEBUG_NEW
#undef new
#endif
inline void* operator new(size_t Size, EZoneTag Tag)
{
	return Z_Malloc(Size, Tag, NULL);
}
#ifdef ZONE_DEBUG_NEW
#define new ZONE_DEBUG_NEW
#endif
