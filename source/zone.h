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
//**
//**	Memory Allocation.
//**
//**************************************************************************

//#define ZONE_DEBUG		1

class ZoneError : public VavoomError
{
public:
	explicit ZoneError(const char* text) : VavoomError(text) { }
};

void Z_Init();
void Z_Shutdown();

#ifdef ZONE_DEBUG

void* Z_Malloc(int size, const char* FileName, int LineNumber);
void* Z_Calloc(int size, const char* FileName, int LineNumber);
void Z_Free(void* ptr, const char* FileName, int LineNumber);

inline void* operator new(size_t Size, const char* FileName, int LineNumber)
{
	return Z_Malloc(Size, FileName, LineNumber);
}

inline void operator delete(void* Ptr, const char* FileName, int LineNumber)
{
	Z_Free(Ptr, FileName, LineNumber);
}

inline void* operator new[](size_t Size, const char* FileName, int LineNumber)
{
	return Z_Malloc(Size, FileName, LineNumber);
}

inline void operator delete[](void* Ptr, const char* FileName, int LineNumber)
{
	Z_Free(Ptr, FileName, LineNumber);
}

inline void* operator new(size_t Size)
{
	return Z_Malloc(Size, "", 0);
}

inline void operator delete(void* Ptr)
{
	Z_Free(Ptr, "", 0);
}

inline void* operator new[](size_t Size)
{
	return Z_Malloc(Size, "", 0);
}

inline void operator delete[](void* Ptr)
{
	Z_Free(Ptr, "", 0);
}

#define Z_Malloc(size)				Z_Malloc(size, __FILE__, __LINE__)
#define Z_Calloc(size)				Z_Calloc(size, __FILE__, __LINE__)
#define Z_Free(ptr)					Z_Free(ptr, __FILE__, __LINE__)

#define ZONE_DEBUG_NEW				new(__FILE__, __LINE__)
#undef new
#define new							ZONE_DEBUG_NEW

#else

void* Z_Malloc(int size);
void* Z_Calloc(int size);
void Z_Free(void* ptr);

inline void* operator new(size_t Size)
{
	return Z_Malloc(Size);
}

inline void operator delete(void* Ptr)
{
	Z_Free(Ptr);
}

inline void* operator new[](size_t Size)
{
	return Z_Malloc(Size);
}

inline void operator delete[](void* Ptr)
{
	Z_Free(Ptr);
}

#endif
