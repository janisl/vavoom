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
//**	ZONE MEMORY ALLOCATION
//**
//**	There is never any space between memblocks, and there will never be
//**  two contiguous free memblocks. The rover can be left pointing at a
//**  non-empty block. It is of no value to free a cachable block, because
//**  it will get overwritten automatically if needed.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define ZONEID				0x1d4a11
#define MINFRAGMENT			64

// TYPES -------------------------------------------------------------------

#ifdef ZONE_DEBUG_NEW
#undef new
#endif
inline void* operator new(size_t, void* p) { return p; }
#ifdef ZONE_DEBUG_NEW
#define new ZONE_DEBUG_NEW
#endif

class TMemZone;

struct memblock_t
{
	int			size;	// including the header and possibly tiny fragments
	int			tag;	// purgelevel, 0 if a free block
	int			id;		// should be ZONEID
	memblock_t*	next;
	memblock_t*	prev;
};

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
	int			Size;   	// total bytes malloced, including header
	memblock_t	BlockList;  // start / end cap for linked list
	memblock_t*	Rover;

	TMemZone(int Asize) : Size(Asize)
	{
		Init();
	}
	void Init();
	void *Malloc(int size);
	void Resize(void** ptr, int size);
	void Free(void* ptr);
	void CheckHeap();
	int FreeMemory();
	void DumpHeap(FOutputDevice &Ar);
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
	// set the entire zone to one free block
	memblock_t* block = (memblock_t*)((char*)this + sizeof(TMemZone));
	BlockList.next = block;
	BlockList.prev = block;
	BlockList.tag = 1;
	Rover = block;

	block->prev = block->next = &BlockList;
	block->tag = 0; // 0 indicates a free block.
	block->size = Size - sizeof(TMemZone);
	unguard;
}

//==========================================================================
//
//	TMemZone::Malloc
//
//==========================================================================

void *TMemZone::Malloc(int size)
{
	guard(TMemZone::Malloc);
	int			extra;
	memblock_t*	start;
	memblock_t* rover;
	memblock_t* newblock;
	memblock_t*	base;

	if (!size)
		size = 4;

	size = (size + 3) & ~3;

	// scan through the block list,
	// looking for the first free block
	// of sufficient size,
	// throwing out any purgable blocks along the way.

	// account for size of block header
	size += sizeof(memblock_t);

	base = Rover;
	// if there is a free block behind the rover,
	//  back up over them
	if (!base->prev->tag)
		base = base->prev;

	rover = base;
	start = base->prev;

	do
	{
		if (rover == start)
		{
			// scanned all the way around the list
			return NULL;
		}

		if (rover->tag)
		{
			// hit a block so move base past it
			base = rover = rover->next;
		}
		else
		{
			rover = rover->next;
		}
	} while (base->tag || base->size < size);

	
	// found a block big enough
	extra = base->size - size;

	if (extra > MINFRAGMENT)
	{
		// there will be a free fragment after the allocated block
		newblock = (memblock_t*)((char*)base + size);
		newblock->size = extra;

		// 0 indicates free block.
		newblock->tag = 0;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;

		base->next = newblock;
		base->size = size;
	}

	base->tag = 1;

	if (Rover == base)
	{
		// next allocation will start looking here
		Rover = base->next;
	}

	base->id = ZONEID;

	memset((void*)((char*)base + sizeof(memblock_t)), 0x6a, size - sizeof(memblock_t));
	return (void*)((char*)base + sizeof(memblock_t));
	unguard;
}

//==========================================================================
//
//	TMemZone::Resize
//
//	Resizes block
//
//==========================================================================

void TMemZone::Resize(void** ptr, int size)
{
	guard(TMemZone::Resize);
	memblock_t	*block;
	memblock_t	*other;
	void*		p;
	int			extra;

	block = (memblock_t *)((char*)(*ptr) - sizeof(memblock_t));

	//FIXME already chacked
	if (block->id != ZONEID)
		Sys_Error("Z_Resize: resize a pointer without ZONEID");

	size = (size + 3) & ~3;
	size += sizeof(memblock_t);
	if (size > block->size)
	{
		//
		//	We need a bigger block
		//

		other = block->next;
		// There is enough size to resize without moving data
		if (!other->tag && (block->size + other->size >= size))
		{
			//Merge blocks
			block->size += other->size;
			block->next = other->next;
			block->next->prev = block;
			if (Rover == other)
				Rover = block;

			// If block is too big
			extra = block->size - size;
			if (extra > MINFRAGMENT)
			{
				// there will be a free fragment after the resized block
				other = (memblock_t *)((char*)block + size );
				other->size = extra;
	
				// 0 indicates free block.
				other->tag = 0;
				other->prev = block;
				other->next = block->next;
				other->next->prev = other;

				block->next = other;
				block->size = size;
				if (Rover == block)
					Rover = other;
			}
		}
		else
		{
			// We have to allocate another block and move data
			p = Malloc(size - sizeof(memblock_t));
			memcpy(p, *ptr, block->size - sizeof(memblock_t));
			Free(*ptr);
			*ptr = p;
		}
	}
	else
	{
		//
		//	We need a smaller block or size is the same
		//

		extra = block->size - size;
		if (extra > MINFRAGMENT)
		{
			// there will be a free fragment after the resized block
			other = (memblock_t *)((char*)block + size );
			other->size = extra;
	
			// 0 indicates free block.
			other->tag = 0;
			other->prev = block;
			other->next = block->next;
			other->next->prev = other;

			block->next = other;
			block->size = size;

			block = other;
			other = block->next;
			if (!other->tag)
			{
				//	Merge two contiguous free blocks
				block->size += other->size;
				block->next = other->next;
				block->next->prev = block;
				if (Rover == other)
					Rover = block;
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	TMemZone::Free
//
//==========================================================================

void TMemZone::Free(void* ptr)
{
	guard(TMemZone::Free);
	memblock_t*		block;
	memblock_t*		other;

	block = (memblock_t *)((char*)ptr - sizeof(memblock_t));

	if (block->id != ZONEID)
		Sys_Error("Z_Free: freed a pointer without ZONEID");
		
	// mark as free
	block->id = 0;
	block->tag = 0;

	other = block->next;
	if (!other->tag)
	{
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;

		if (other == Rover)
			Rover = block;
	}
	
	other = block->prev;
	if (!other->tag)
	{
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;

		if (block == Rover)
			Rover = other;
	}
	unguard;
}

//==========================================================================
//
//	TMemZone::CheckHeap
//
//==========================================================================

void TMemZone::CheckHeap()
{
	guard(TMemZone::CheckHeap);
	memblock_t*	block;

	for (block = BlockList.next; block->next != &BlockList; block = block->next)
	{
		if ((char*)block + block->size != (char*)block->next)
			Sys_Error("Z_CheckHeap: block size does not touch the next block\n");

		if ( block->next->prev != block)
			Sys_Error("Z_CheckHeap: next block doesn't have proper back link\n");

		if (!block->tag && !block->next->tag)
			Sys_Error("Z_CheckHeap: two consecutive free blocks\n");
	}
	unguard;
}

//==========================================================================
//
//	TMemZone::FreeMemory
//
//==========================================================================

int TMemZone::FreeMemory()
{
	guard(TMemZone::FreeMemory);
	memblock_t*		block;
	int				free = 0;
	int				largest = 0;
	int				numblocks = 0;
	int				purgable = 0;
	int				largestpurgable = 0;
	int				purgableblocks = 0;

	for (block = BlockList.next; block != &BlockList; block = block->next)
	{
		if (!block->tag)
		{
			free += block->size;
			if (block->size > largest)
			{
				largest = block->size;
			}
			numblocks++;
		}
	}
	GCon->Logf(NAME_Dev, "Free memory %d, largest block %d, free blocks %d",
		free, largest, numblocks);
	GCon->Logf(NAME_Dev, "Purgable memory %d, largest block %d, total blocks %d",
		purgable, largestpurgable, purgableblocks);
	return free;
	unguard;
}

//==========================================================================
//
//	TMemZone::DumpHeap
//
//==========================================================================

void TMemZone::DumpHeap(FOutputDevice &Ar)
{
	guard(TMemZone::DumpHeap);
	memblock_t*	block;

	Ar.Logf("zone size: %d  location: %p", Size, this);

	for (block = BlockList.next; ; block = block->next)
	{
		Ar.Logf("block:%p    size:%7i    tag:%3i",
			block, block->size, block->tag);

		if (block->next == &BlockList)
		{
			// all blocks have been hit
			break;
		}
	
		if ((byte *)block + block->size != (byte *)block->next)
		{
			Ar.Log("ERROR: block size does not touch the next block");
		}

		if (block->next->prev != block)
		{
			Ar.Log("ERROR: next block doesn't have proper back link");
		}

		if (!block->tag && !block->next->tag)
		{
			Ar.Log("ERROR: two consecutive free blocks");
		}
	}
	return;
	unguard;
}

//==========================================================================
//
//  Z_Init
//
//==========================================================================

void Z_Init(void* base, int size)
{
	guard(Z_Init);
#ifdef ZONE_DEBUG_NEW
#undef new
#endif
	mainzone = new(base) TMemZone(size);
#ifdef ZONE_DEBUG_NEW
#define new ZONE_DEBUG_NEW
#endif
	unguard;
}

//==========================================================================
//
//  Z_Shutdown
//
//==========================================================================

void Z_Shutdown()
{
#ifdef ZONE_DEBUG
	mainzone->DumpHeap(*GCon);
	Z_MemDebugDump();
#endif
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
	void* ptr = mainzone->Malloc(size + sizeof(MemDebug_t));
	if (!ptr)
	{
		mainzone->DumpHeap(*GCon);
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
//	Z_Resize
//
//	Resizes block
//
//==========================================================================

void Z_Resize(void** ptr, int size, const char* FileName, int LineNumber)
{
	guard(Z_Resize);
	memblock_t*	block;
	MemDebug_t*	m;

	//	Check.
	block = (memblock_t*)((char*)(*ptr) - sizeof(memblock_t) - sizeof(MemDebug_t));
	if (block->id != ZONEID)
		Sys_Error("Z_Resize: resize a pointer without ZONEID");

	//	Unlink debug info.
	m = (MemDebug_t*)((char*)(*ptr) - sizeof(MemDebug_t));
	if (m->Next)
		m->Next->Prev = m->Prev;
	if (m == MemDebug)
		MemDebug = m->Next;
	else
		m->Prev->Next = m->Next;

	mainzone->Resize((void**)&m, size + sizeof(MemDebug_t));

	//	New debug info.
	m->FileName = FileName;
	m->LineNumber = LineNumber;
	m->Size = size;
	m->Next = MemDebug;
	if (MemDebug)
		MemDebug->Prev = m;
	MemDebug = m;

	*ptr = (byte*)m + sizeof(MemDebug_t);
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
	memblock_t*		block;

	block = (memblock_t *)((char*)ptr - sizeof(memblock_t) - sizeof(MemDebug_t));
	if (block->id != ZONEID)
		Sys_Error("Z_Free: freed a pointer without ZONEID from %s:%d", FileName, LineNumber);

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
		memblock_t* b = (memblock_t*)((byte*)m - sizeof(memblock_t));
		GCon->Logf("size %8d tag %3d at %s:%d", m->Size, b->tag,
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
	void* ptr = mainzone->Malloc(size);
	if (!ptr)
	{
		mainzone->DumpHeap(*GCon);
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
//	Z_Resize
//
//	Resizes block
//
//==========================================================================

void Z_Resize(void** ptr, int size)
{
	guard(Z_Resize);
	memblock_t	*block;

	block = (memblock_t *)((char*)(*ptr) - sizeof(memblock_t));

	if (block->id != ZONEID)
		Sys_Error("Z_Resize: resize a pointer without ZONEID");
	mainzone->Resize(ptr, size);
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
	memblock_t*		block;

	block = (memblock_t *)((char*)ptr - sizeof(memblock_t));

	if (block->id != ZONEID)
		Sys_Error("Z_Free: freed a pointer without ZONEID");
	mainzone->Free(ptr);
	unguard;
}

#endif

//==========================================================================
//
//	Z_CheckHeap
//
//==========================================================================

void Z_CheckHeap()
{
	guard(Z_CheckHeap);
	mainzone->CheckHeap();
	unguard;
}

//==========================================================================
//
//	Z_FreeMemory
//
//==========================================================================

int Z_FreeMemory()
{
	guard(Z_FreeMemory);
	return mainzone->FreeMemory();
	unguard;
}

//==========================================================================
//
//	COMMAND FreeMemory
//
//==========================================================================

COMMAND(FreeMemory)
{
	guard(COMMAND FreeMemory);
	mainzone->FreeMemory();
	unguard;
}

//==========================================================================
//
//	COMMAND DumpHeap
//
//==========================================================================

COMMAND(DumpHeap)
{
	guard(COMMAND DumpHeap);
	mainzone->DumpHeap(*GCon);
	unguard;
}
