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

#include <new>
#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define ZONEID				0x1d4a11
#define MINFRAGMENT			64

// TYPES -------------------------------------------------------------------

class TMemZone;

struct memblock_t
{
	int			size;	// including the header and possibly tiny fragments
	void		**user;	// NULL if a free block
	int			tag;	// purgelevel, 0 if a free block
	int			id;		// should be ZONEID
	TMemZone	*zone;	// Zone, to which this block belongs to
	int			align;
	memblock_t	*next;
	memblock_t	*prev;
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
	void Init(void);
	void *Malloc(int size, int tag, void** user, bool);
	void *MallocHigh(int size, int tag, void** user);
	void Resize(void** ptr, int size);
	void Free(void* ptr);
	void FreeTag(int tag);
	void CheckHeap(void);
	int FreeMemory(void);
	void DumpHeap(FOutputDevice &Ar);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TMemZone*	mainzone;
static TMemZone*	minizone;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TMemZone::Init
//
//==========================================================================

void TMemZone::Init(void)
{
	guard(TMemZone::Init);
    memblock_t*	block;

    // set the entire zone to one free block
    BlockList.next =
    BlockList.prev =
	block = (memblock_t *)((char*)this + sizeof(TMemZone));

    BlockList.user = (void **)this;
    BlockList.tag = PU_STATIC;
    Rover = block;
	
    block->prev = block->next = &BlockList;
    block->user = NULL;
	block->tag = 0; // 0 indicates a free block.
    block->size = Size - sizeof(TMemZone);
	unguard;
}

//==========================================================================
//
//	TMemZone::Malloc
//
//	You can pass a NULL user if the tag is < PU_PURGELEVEL.
//
//==========================================================================

void *TMemZone::Malloc(int size, int tag, void** user, bool alloc_low)
{
	guard(TMemZone::Malloc);
    int			extra;
    memblock_t*	start;
    memblock_t* rover;
    memblock_t* newblock;
    memblock_t*	base;

//	if (!size) Sys_Error("Z_Malloc: Size = 0");
	if (!tag) Sys_Error("Z_Malloc: Tried to use tag 0");

	size = (size + 3) & ~3;
    
    // scan through the block list,
    // looking for the first free block
    // of sufficient size,
    // throwing out any purgable blocks along the way.

    // account for size of block header
    size += sizeof(memblock_t);

	if (alloc_low)
	{
		base = BlockList.next;
	}
	else
	{
	    base = Rover;
	    // if there is a free block behind the rover,
    	//  back up over them
		if (!base->prev->tag)
			base = base->prev;
	}
	
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
	    	if (rover->tag < PU_PURGELEVEL)
	    	{
				// hit a block that can't be purged,
				//  so move base past it
				base = rover = rover->next;
	    	}
	    	else
	    	{
				// free the rover block (adding the size to base)
				// the rover can be the base block
				base = base->prev;
				Z_Free((char*)rover + sizeof(memblock_t));
				base = base->next;
				rover = base->next;
	    	}
		}
		else
	    	rover = rover->next;
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
		newblock->user = NULL;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;

		base->next = newblock;
		base->size = size;
    }
	
    if (user)
    {
		// mark as an in use block
		*user = (void *)((char*)base + sizeof(memblock_t));
    }
    else
    {
		if (tag >= PU_PURGELEVEL)
		{
			Sys_Error("Z_Malloc: an owner is required for purgable blocks");
		}
    }
    base->tag = tag;
	base->user = user;

	if (!alloc_low || Rover == base)
	{
		// next allocation will start looking here
		Rover = base->next;
	}
	
    base->id = ZONEID;
	base->zone = this;
    
    return (void *)((char*)base + sizeof(memblock_t));
	unguard;
}

//==========================================================================
//
//	TMemZone::MallocHigh
//
//	You can pass a NULL user if the tag is < PU_PURGELEVEL.
//
//==========================================================================

void *TMemZone::MallocHigh(int size, int tag, void** user)
{
	guard(TMemZone::MallocHigh);
    int			extra;
    memblock_t* rover;
    memblock_t* newblock;
    memblock_t*	base;

#ifdef PARANOID
	CheckHeap();
#endif

	if (!size) Sys_Error("Z_Malloc: Size = 0");
	if (!tag) Sys_Error("Z_Malloc: Tried to use tag 0");

	size = (size + 3) & ~3;
    
    // scan through the block list,
    // looking for the first free block
    // of sufficient size,
    // throwing out any purgable blocks along the way.

    // account for size of block header
    size += sizeof(memblock_t);

	base = BlockList.prev;
    rover = base;
	
    do
    {
		if (rover == &BlockList)
		{
	    	// scanned all the way around the list
			return NULL;
		}
	
		if (rover->tag)
		{
	    	if (rover->tag < PU_PURGELEVEL)
	    	{
				// hit a block that can't be purged,
				//  so move base past it
				base = rover = rover->prev;
	    	}
	    	else
	    	{
				// free the rover block (adding the size to base)
				// the rover can be the base block
				base = base->next;
				Z_Free((char*)rover + sizeof(memblock_t));
				base = base->prev;
				rover = base->prev;
	    	}
		}
		else
		{
	    	rover = rover->prev;
		}
    } while (base->tag || base->size < size);

    
    // found a block big enough
    extra = base->size - size;
    
    if (extra > MINFRAGMENT)
    {
		// there will be a free fragment before the allocated block
		newblock = (memblock_t*)((char*)base + extra);
		newblock->size = size;
	
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;

		base->next = newblock;
		base->size = extra;
		base = newblock;
    }
	
    if (user)
    {
		// mark as an in use block
		*user = (void *)((char*)base + sizeof(memblock_t));
    }
    else
    {
		if (tag >= PU_PURGELEVEL)
		{
			Sys_Error("Z_Malloc: an owner is required for purgable blocks");
		}
    }
    base->tag = tag;
	base->user = user;
    base->id = ZONEID;
	base->zone = this;
    
    return (void *)((char*)base + sizeof(memblock_t));
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
	if (block->tag >= PU_PURGELEVEL)
      	Sys_Error("Z_Resize: Cannot resize purgable block");

    size = (size + 3) & ~3;
    size += sizeof(memblock_t);
    if (size > block->size)
	{
    	//
        //	We need a bigger block
        //

        other = block->next;
		// if next block can be purged, then free it
   		if (other->tag >= PU_PURGELEVEL)
			Z_Free((char*)other + sizeof(memblock_t));
		// If next block is free, while size is not enough and
        // block after next block can be purged, free more space
		if (!other->tag)
		{
          	while ((block->size + other->size < size)
                 && (other->next->tag >= PU_PURGELEVEL))
            	Z_Free((char*)other->next + sizeof(memblock_t));
		}
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
				other->user = NULL;
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
            p = Z_Malloc(size - sizeof(memblock_t), block->tag, block->user);
            memcpy(p, *ptr, block->size - sizeof(memblock_t));
			block->user = NULL;// So Z_Free doesn't clear user's mark
            Z_Free(*ptr);
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
			other->user = NULL;
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
		
    if (block->user)
    {
		// clear the user's mark
		*block->user = 0;
    }

	// mark as free
	block->id = 0;
	block->tag = 0;
	block->user = NULL;
	
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
//	TMemZone::FreeTag
//
//==========================================================================

void TMemZone::FreeTag(int tag)
{
	guard(TMemZone::FreeTag);
	memblock_t*	block;
	memblock_t*	next;
	
	for (block = BlockList.next; block != &BlockList; block = next)
	{
		// get link before freeing
		next = block->next;
		if (block->tag == tag)
		{
			Z_Free((char*)block + sizeof(memblock_t));
		}
    }

	//	Reset rover to start of the heap
	Rover = BlockList.next;
	unguard;
}

//==========================================================================
//
//	TMemZone::CheckHeap
//
//==========================================================================

void TMemZone::CheckHeap(void)
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

int TMemZone::FreeMemory(void)
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
		if (block->tag >= PU_PURGELEVEL)
		{
	    	purgable += block->size;
			if (block->size > largest)
            {
            	largestpurgable = block->size;
			}
			purgableblocks++;
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
		Ar.Logf("block:%p    size:%7i    user:%8p    tag:%3i",
			block, block->size, block->user, block->tag);
		
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
	int			minsize = 256 * 1024;

	int p = M_CheckParm("-minzone");
	if (p && p < myargc - 1)
	{
		minsize = (int)(1024 * atof(myargv[p + 1]));
	}
	mainzone = new(base) TMemZone(size);
	minizone = new(Z_Malloc(minsize)) TMemZone(minsize);
	unguard;
}

//==========================================================================
//
//	Z_Malloc
//
//	You can pass a NULL user if the tag is < PU_PURGELEVEL.
//
//==========================================================================

void *Z_Malloc(int size, int tag, void** user)
{
	guard(Z_Malloc);
	void *ptr;
	if (tag == PU_STRING)
	{
		ptr = minizone->Malloc(size, tag, user, false);
	}
	else if (tag == PU_VIDEO || tag == PU_HIGH || tag == PU_TEMP)
	{
		ptr = mainzone->MallocHigh(size, tag, user);
	}
	else
	{
		ptr = mainzone->Malloc(size, tag, user,
			/*tag == PU_LEVEL || tag == PU_LEVSPEC || */tag == PU_LOW);
	}
	if (!ptr)
	{
		if (tag == PU_LEVEL || tag == PU_LEVSPEC)
		{
	    	Host_Error("Z_Malloc: failed on allocation of %d bytes", size);
		}
		else if (tag != PU_VIDEO)
		{
			mainzone->DumpHeap(*GCon);
			GCon->Log("");
			minizone->DumpHeap(*GCon);
    		Sys_Error("Z_Malloc: failed on allocation of %d bytes", size);
		}
	}
	return ptr;
	unguard;
}

//==========================================================================
//
//  Z_Calloc
//
//==========================================================================

void *Z_Calloc(int size, int tag, void **user)
{
	guard(Z_Calloc);
 	return memset(Z_Malloc(size, tag, user), 0, size);
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
	block->zone->Resize(ptr, size);
	unguard;
}

//==========================================================================
//
//	Z_ChangeTag
//
//==========================================================================

void Z_ChangeTag(void* ptr,int tag)
{
	guard(Z_ChangeTag);
    memblock_t*	block;
	
    block = (memblock_t *)((char*)ptr - sizeof(memblock_t));

    if (block->id != ZONEID)
		Sys_Error("Z_ChangeTag: freed a pointer without ZONEID");

    if (tag >= PU_PURGELEVEL && !block->user)
		Sys_Error("Z_ChangeTag: an owner is required for purgable blocks");

    block->tag = tag;
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
	block->zone->Free(ptr);
	unguard;
}

//==========================================================================
//
//	Z_FreeTag
//
//==========================================================================

void Z_FreeTag(int tag)
{
	guard(Z_FreeTag);
	mainzone->FreeTag(tag);
	unguard;
}

//==========================================================================
//
//	Z_CheckHeap
//
//==========================================================================

void Z_CheckHeap(void)
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

int Z_FreeMemory(void)
{
	guard(Z_FreeMemory);
	return mainzone->FreeMemory();
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
	GCon->Log("");
	minizone->DumpHeap(*GCon);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.11  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//
//	Revision 1.10  2002/07/13 07:46:21  dj_jl
//	Added guarding.
//	
//	Revision 1.9  2002/05/18 16:56:35  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.8  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.7  2001/12/27 17:36:47  dj_jl
//	Some speedup
//	
//	Revision 1.6  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.5  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.4  2001/08/30 17:44:45  dj_jl
//	Print heap dump on Z_Malloc failure
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
