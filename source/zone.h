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
//**
//**	Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//**	Remark: this was the only stuff that, according
//**  to John Carmack, might have been useful for Quake.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// PU - purge tags.
// Tags < 100 are not overwritten until freed.
#define PU_STATIC		1	// static entire execution time
#define PU_SOUND		2	// static while playing
#define PU_MUSIC		3	// static while playing
#define PU_STRING		4	// in minizone
#define PU_VIDEO		5	// high, return NULL on failure
#define PU_LOW			6	// low (allocated from start of memory)
#define PU_HIGH			7	// high (allocated from end of memory)
#define PU_LEVEL		50	// low, static until level exited
#define PU_LEVSPEC		51  // low, a special thinker in a level
// Tags >= 100 are purgable whenever needed.
#define PU_PURGELEVEL	100
#define PU_CACHE		101
#define PU_TEMP			102	// high

// TYPES -------------------------------------------------------------------

class ZoneError:public VavoomError
{
public:
	explicit ZoneError(const char *text) : VavoomError(text) { }
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void Z_Init(void* base, int size);

void *Z_Malloc(int size, int tag, void** ptr);
void *Z_Calloc(int size, int tag, void** user);

void Z_Resize(void** ptr, int size);
void Z_ChangeTag(void *ptr, int tag);

void Z_Free(void *ptr);
void Z_FreeTag(int tag);

void Z_CheckHeap(void);
int Z_FreeMemory(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

// INLINE FUNCTIONS AND TEMPLATES ------------------------------------------

inline void *Z_Malloc(int size)
{
	return Z_Malloc(size, PU_STATIC, 0);
}

inline void *Z_Calloc(int size)
{
	return Z_Calloc(size, PU_STATIC, 0);
}

inline void *Z_StrMalloc(int size)
{
	return Z_Malloc(size, PU_STRING, 0);
}

inline void *Z_StrCalloc(int size)
{
	return Z_Calloc(size, PU_STRING, 0);
}

inline char *Z_StrDup(const char *src)
{
	int len = strlen(src);
	char *buf = (char*)Z_Malloc(len + 1, PU_STRING, 0);
	strcpy(buf, src);
	return buf;
}

template<class T> T* Z_New(void)
{
	return (T*)Z_Malloc(sizeof(T));
}

template<class T> T* Z_New(int elem_count)
{
	return (T*)Z_Malloc(elem_count * sizeof(T));
}

template<class T> T* Z_New(int tag, void** owner)
{
	return (T*)Z_Malloc(sizeof(T), tag, owner);
}

template<class T> T* Z_New(int elem_count, int tag, void** owner)
{
	return (T*)Z_Malloc(elem_count * sizeof(T), tag, owner);
}

template<class T> T* Z_CNew(void)
{
	return (T*)Z_Calloc(sizeof(T));
}

template<class T> T* Z_CNew(int elem_count)
{
	return (T*)Z_Calloc(elem_count * sizeof(T));
}

template<class T> T* Z_CNew(int tag, void** owner)
{
	return (T*)Z_Calloc(sizeof(T), tag, owner);
}

template<class T> T* Z_CNew(int elem_count, int tag, void** owner)
{
	return (T*)Z_Calloc(elem_count * sizeof(T), tag, owner);
}

//**************************************************************************
//
//	$Log$
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
