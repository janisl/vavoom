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
//**	Memory allocation
//**
//**	Mostly based on memory allocator of Doom 3.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
//#include "../libs/core/core.h"

// MACROS ------------------------------------------------------------------

#define SMALLID				0x22
#define LARGEID				0x33

// TYPES -------------------------------------------------------------------

enum
{
	ALIGN = 4
};

#define ALIGN_SIZE(bytes)	(((bytes) + ALIGN - 1) & ~(ALIGN - 1))

#define SMALL_HEADER_SIZE	ALIGN_SIZE(sizeof(vuint8) + sizeof(vuint8))
#define LARGE_HEADER_SIZE	ALIGN_SIZE(sizeof(void*) + sizeof(vuint8))

#define SMALL_ALIGN(bytes)	(ALIGN_SIZE((bytes) + SMALL_HEADER_SIZE) - SMALL_HEADER_SIZE)

struct MemDebug_t
{
	const char*		FileName;
	int				LineNumber;
	int				Size;
	MemDebug_t*		Prev;
	MemDebug_t*		Next;
};

class TMemZone
{
public:
	void Init();
	void Shutdown();

	void* Alloc(size_t Bytes);
	void Free(void* Ptr);

private:
	struct VPage
	{
		void*		Data;
		size_t		Size;

		VPage*		Prev;
		VPage*		Next;
	};

	size_t			PageSize;

	void*			SmallFirstFree[256 / ALIGN + 1];
	VPage*			SmallPage;
	size_t			SmallOffset;
	VPage*			SmallUsedPages;

	VPage*			LargeFirstUsedPage;

	VPage* AllocPage(size_t Bytes);
	void FreePage(VPage* Page);

	void* SmallAlloc(size_t Bytes);
	void SmallFree(void* Ptr);

	void* LargeAlloc(size_t Bytes);
	void LargeFree(void* Ptr);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void Z_MemDebugDump();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TMemZone*	mainzone;
static MemDebug_t*	MemDebug;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TMemZone::Init
//
//==========================================================================

void TMemZone::Init()
{
	guard(TMemZone::Init);
	PageSize = 65536 - sizeof(VPage);

	memset(SmallFirstFree, 0, sizeof(SmallFirstFree));
	SmallPage = AllocPage(PageSize);
	SmallOffset = 0;
	SmallUsedPages = NULL;

	LargeFirstUsedPage = NULL;
	unguard;
}

//==========================================================================
//
//	TMemZone::Shutdown
//
//==========================================================================

void TMemZone::Shutdown()
{
	guard(TMemZone::Shutdown);
	//	Free small allocation pages.
	for (VPage* Page = SmallUsedPages; Page;)
	{
		VPage* Next = Page->Next;
		FreePage(Page);
		Page = Next;
	}
	FreePage(SmallPage);

	//	Free large allocation pages.
	for (VPage* Page = LargeFirstUsedPage; Page;)
	{
		VPage* Next = Page->Next;
		FreePage(Page);
		Page = Next;
	}
	unguard;
}

//==========================================================================
//
//	TMemZone::Alloc
//
//==========================================================================

void* TMemZone::Alloc(size_t Bytes)
{
	guard(TMemZone::Alloc);
	if (Bytes < 256)
	{
		return SmallAlloc(Bytes);
	}
	return LargeAlloc(Bytes);
	unguard;
}

//==========================================================================
//
//	TMemZone::Free
//
//==========================================================================

void TMemZone::Free(void* Ptr)
{
	guard(TMemZone::Free);
	switch (((vuint8*)Ptr)[-1])
	{
	case SMALLID:
		SmallFree(Ptr);
		break;
	case LARGEID:
		LargeFree(Ptr);
		break;
	default:
		Sys_Error("Invalid memory block");
	}
	unguard;
}

//==========================================================================
//
//	TMemZone::AllocPage
//
//==========================================================================

TMemZone::VPage* TMemZone::AllocPage(size_t Bytes)
{
	guard(TMemZone::AllocPage);
	size_t Size = Bytes + sizeof(VPage);
	VPage* P = (VPage*)::malloc(Size);
	if (!P)
	{
		Sys_Error("Failed to allocate %d bytes", Bytes);
	}
	P->Data = P + 1;
	P->Size = Bytes;
	P->Prev = NULL;
	P->Next = NULL;
	return P;
	unguard;
}

//==========================================================================
//
//	TMemZone::FreePage
//
//==========================================================================

void TMemZone::FreePage(TMemZone::VPage* Page)
{
	guard(TMemZone::FreePage);
	check(Page);
	::free(Page);
	unguard;
}

//==========================================================================
//
//	TMemZone::SmallAlloc
//
//==========================================================================

void* TMemZone::SmallAlloc(size_t Bytes)
{
	guard(TMemZone::SmallAlloc);
	//	We need enough memory for the free list.
	if (Bytes < sizeof(void*))
	{
		Bytes = sizeof(void*);
	}

	//	Align the size.
	Bytes = SMALL_ALIGN(Bytes);
	vuint8* SmallBlock = (vuint8*)SmallFirstFree[Bytes / ALIGN];
	if (SmallBlock)
	{
		vuint8* Ptr = SmallBlock + SMALL_HEADER_SIZE;
		SmallFirstFree[Bytes / ALIGN] = *(void**)Ptr;
		Ptr[-1] = SMALLID;
		return Ptr;
	}

	size_t BytesLeft = PageSize - SmallOffset;
	if (BytesLeft < Bytes + SMALL_HEADER_SIZE)
	{
		//	Add current page to the used ones.
		SmallPage->Next = SmallUsedPages;
		SmallUsedPages = SmallPage;
		SmallPage = AllocPage(PageSize);
		SmallOffset = 0;
	}

	SmallBlock = (vuint8*)SmallPage->Data + SmallOffset;
	vuint8* Ptr = SmallBlock + SMALL_HEADER_SIZE;
	SmallBlock[0] = (vuint8)(Bytes / ALIGN);
	Ptr[-1] = SMALLID;
	SmallOffset += Bytes + SMALL_HEADER_SIZE;
	return Ptr;
	unguard;
}

//==========================================================================
//
//	TMemZone::SmallFree
//
//==========================================================================

void TMemZone::SmallFree(void* Ptr)
{
	guard(TMemZone::SmallFree);
	((vuint8*)Ptr)[-1] = 0;

	vuint8* Block = (vuint8*)Ptr - SMALL_HEADER_SIZE;
	size_t Idx = *Block;
	check(Idx <= 256 / ALIGN);

	*((void**)Ptr) = SmallFirstFree[Idx];
	SmallFirstFree[Idx] = Block;
	unguard;
}

//==========================================================================
//
//	TMemZone::LargeAlloc
//
//==========================================================================

void* TMemZone::LargeAlloc(size_t Bytes)
{
	guard(TMemZone::LargeAlloc);
	VPage* P = AllocPage(Bytes + LARGE_HEADER_SIZE);

	vuint8* Ptr = (vuint8*)P->Data + LARGE_HEADER_SIZE;
	*(void**)P->Data = P;
	Ptr[-1] = LARGEID;

	//	Link to 'large used page list'
	P->Prev = NULL;
	P->Next = LargeFirstUsedPage;
	if (P->Next)
	{
		P->Next->Prev = P;
	}
	LargeFirstUsedPage = P;

	return Ptr;
	unguard;
}

//==========================================================================
//
//	TMemZone::LargeFree
//
//==========================================================================

void TMemZone::LargeFree(void* Ptr)
{
	guard(TMemZone::LargeFree);
	((vuint8*)Ptr)[-1] = 0;

	//	Get page pointer
	VPage* P = (VPage*)(*((void**)(((vuint8*)Ptr) - LARGE_HEADER_SIZE)));

	//	Unlink from doubly linked list
	if (P->Prev)
	{
		P->Prev->Next = P->Next;
	}
	if (P->Next)
	{
		P->Next->Prev = P->Prev;
	}
	if (P == LargeFirstUsedPage)
	{
		LargeFirstUsedPage = P->Next;
	}
	P->Next = P->Prev = NULL;

	FreePage(P);
	unguard;
}

//==========================================================================
//
//  Z_Init
//
//==========================================================================

void Z_Init()
{
	guard(Z_Init);
	mainzone = (TMemZone*)::malloc(sizeof(TMemZone));
	mainzone->Init();
	unguard;
}

//==========================================================================
//
//  Z_Shutdown
//
//==========================================================================

void Z_Shutdown()
{
	if (mainzone)
	{
#ifdef ZONE_DEBUG
		Z_MemDebugDump();
#endif
		mainzone->Shutdown();
		::free(mainzone);
		mainzone = NULL;
	}
}

#ifdef ZONE_DEBUG

#undef Z_Malloc
#undef Z_Calloc
#undef Z_Resize
#undef Z_Free

//==========================================================================
//
//	Z_Malloc
//
//==========================================================================

void *Z_Malloc(int size, const char* FileName, int LineNumber)
{
	guard(Z_Malloc);
	if (!size)
	{
		return NULL;
	}
	if (!mainzone)
	{
		return ::malloc(size);
	}

	void* ptr = mainzone->Alloc(size + sizeof(MemDebug_t));
	if (!ptr)
	{
		Sys_Error("Z_Malloc: failed on allocation of %d bytes", size);
	}

	MemDebug_t* m = (MemDebug_t*)ptr;
	m->FileName = FileName;
	m->LineNumber = LineNumber;
	m->Size = size;
	m->Next = MemDebug;
	if (MemDebug)
		MemDebug->Prev = m;
	MemDebug = m;

	return (byte*)ptr + sizeof(MemDebug_t);
	unguard;
}

//==========================================================================
//
//  Z_Calloc
//
//==========================================================================

void *Z_Calloc(int size, const char* FileName, int LineNumber)
{
	guard(Z_Calloc);
	return memset(Z_Malloc(size, FileName, LineNumber), 0, size);
	unguard;
}

//==========================================================================
//
//	Z_Free
//
//==========================================================================

void Z_Free(void* ptr, const char* FileName, int LineNumber)
{
	guard(Z_Free);
	if (!ptr)
	{
		return;
	}
	if (!mainzone)
	{
		//::free(ptr);
		dprintf("Z_Free after Z_Shutdown at %s:%d\n", FileName, LineNumber);
		return;
	}

	//	Unlink debug info.
	MemDebug_t* m = (MemDebug_t*)((char*)ptr - sizeof(MemDebug_t));
	if (m->Next)
		m->Next->Prev = m->Prev;
	if (m == MemDebug)
		MemDebug = m->Next;
	else
		m->Prev->Next = m->Next;

	mainzone->Free((char*)ptr - sizeof(MemDebug_t));
	unguard;
}

//==========================================================================
//
//	Z_MemDebugDump
//
//==========================================================================

static void Z_MemDebugDump()
{
	int NumBlocks = 0;
	for (MemDebug_t* m = MemDebug; m; m = m->Next)
	{
		GCon->Logf("block %p size %8d at %s:%d", m + 1, m->Size,
			m->FileName, m->LineNumber);
		NumBlocks++;
	}
	GCon->Logf("%d blocks allocated", NumBlocks);
}

//==========================================================================
//
//	COMMAND MemDebugDump
//
//==========================================================================

COMMAND(MemDebugDump)
{
	Z_MemDebugDump();
}

#else

//==========================================================================
//
//	Z_Malloc
//
//==========================================================================

void *Z_Malloc(int size)
{
	guard(Z_Malloc);
	if (!size)
	{
		return NULL;
	}
	if (!mainzone)
	{
		return ::malloc(size);
	}

	void* ptr = mainzone->Alloc(size);
	if (!ptr)
	{
		Sys_Error("Z_Malloc: failed on allocation of %d bytes", size);
	}
	return ptr;
	unguard;
}

//==========================================================================
//
//  Z_Calloc
//
//==========================================================================

void *Z_Calloc(int size)
{
	guard(Z_Calloc);
	return memset(Z_Malloc(size), 0, size);
	unguard;
}

//==========================================================================
//
//	Z_Free
//
//==========================================================================

void Z_Free(void* ptr)
{
	guard(Z_Free);
	if (!ptr)
	{
		return;
	}
	if (!mainzone)
	{
		//::free(ptr);
		dprintf("Z_Free after Z_Shutdown\n");
		return;
	}

	mainzone->Free(ptr);
	unguard;
}

#endif
