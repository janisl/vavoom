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
	void DumpHeap(ostream &str);
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
    int			extra;
    memblock_t*	start;
    memblock_t* rover;
    memblock_t* newblock;
    memblock_t*	base;

#ifdef PARANOID
	CheckHeap();
#endif

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
}

//==========================================================================
//
//	TMemZone::Free
//
//==========================================================================

void TMemZone::Free(void* ptr)
{
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
}

//==========================================================================
//
//	TMemZone::FreeTag
//
//==========================================================================

void TMemZone::FreeTag(int tag)
{
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
}

//==========================================================================
//
//	TMemZone::CheckHeap
//
//==========================================================================

void TMemZone::CheckHeap(void)
{
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
}

//==========================================================================
//
//	TMemZone::FreeMemory
//
//==========================================================================

int TMemZone::FreeMemory(void)
{
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
	cond << "Free memory " << free << ", largest block " << largest
		<< ", free blocks " << numblocks << endl;
	cond << "Purgable memory " << purgable << ", largest block "
		<< largestpurgable << ", total blocks " << purgableblocks << endl;		
    return free;
}

//==========================================================================
//
//	TMemZone::DumpHeap
//
//==========================================================================

void TMemZone::DumpHeap(ostream &str)
{
	memblock_t*	block;
	
	str << "zone size: " << Size << "  location: " << (void*)this << endl;

	for (block = BlockList.next; ; block = block->next)
	{
		str << va("block:%p    size:%7i    user:%8p    tag:%3i\n",
			block, block->size, block->user, block->tag);
		
		if (block->next == &BlockList)
		{
			// all blocks have been hit
			break;
		}
	
		if ((byte *)block + block->size != (byte *)block->next)
		{
			str << "ERROR: block size does not touch the next block\n";
		}

		if (block->next->prev != block)
		{
	    	str << "ERROR: next block doesn't have proper back link\n";
		}

		if (!block->tag && !block->next->tag)
		{
			str << "ERROR: two consecutive free blocks\n";
		}
	}
	return;
}

//==========================================================================
//
//  Z_Init
//
//==========================================================================

void Z_Init(void* base, int size)
{
	int			minsize = 256 * 1024;

	int p = M_CheckParm("-minzone");
	if (p && p < myargc - 1)
	{
		minsize = (int)(1024 * atof(myargv[p + 1]));
	}
	mainzone = new(base) TMemZone(size);
	minizone = new(Z_Malloc(minsize)) TMemZone(minsize);
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
			tag == PU_LEVEL || tag == PU_LEVSPEC || tag == PU_LOW);
	}
	if (!ptr)
	{
		if (tag == PU_LEVEL || tag == PU_LEVSPEC)
		{
	    	Host_Error("Z_Malloc: failed on allocation of %d bytes", size);
		}
		else if (tag != PU_VIDEO)
		{
			Z_FreeMemory();
    		Sys_Error("Z_Malloc: failed on allocation of %d bytes", size);
		}
	}
	return ptr;
}

//==========================================================================
//
//  Z_Calloc
//
//==========================================================================

void *Z_Calloc(int size, int tag, void **user)
{
 	return memset(Z_Malloc(size, tag, user), 0, size);
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
    memblock_t	*block;

    block = (memblock_t *)((char*)(*ptr) - sizeof(memblock_t));

    if (block->id != ZONEID)
		Sys_Error("Z_Resize: resize a pointer without ZONEID");
	block->zone->Resize(ptr, size);
}

//==========================================================================
//
//	Z_ChangeTag
//
//==========================================================================

void Z_ChangeTag(void* ptr,int tag)
{
    memblock_t*	block;
	
    block = (memblock_t *)((char*)ptr - sizeof(memblock_t));

    if (block->id != ZONEID)
		Sys_Error("Z_ChangeTag: freed a pointer without ZONEID");

    if (tag >= PU_PURGELEVEL && !block->user)
		Sys_Error("Z_ChangeTag: an owner is required for purgable blocks");

    block->tag = tag;
}

//==========================================================================
//
//	Z_Free
//
//==========================================================================

void Z_Free(void* ptr)
{
    memblock_t*		block;
	
    block = (memblock_t *)((char*)ptr - sizeof(memblock_t));

    if (block->id != ZONEID)
		Sys_Error("Z_Free: freed a pointer without ZONEID");
	block->zone->Free(ptr);
}

//==========================================================================
//
//	Z_FreeTag
//
//==========================================================================

void Z_FreeTag(int tag)
{
	mainzone->FreeTag(tag);
}

//==========================================================================
//
//	Z_CheckHeap
//
//==========================================================================

void Z_CheckHeap(void)
{
	mainzone->CheckHeap();
}

//==========================================================================
//
//	Z_FreeMemory
//
//==========================================================================

int Z_FreeMemory(void)
{
	return mainzone->FreeMemory();
}

//==========================================================================
//
//	COMMAND DumpHeap
//
//==========================================================================

COMMAND(DumpHeap)
{
	if (Argc() == 1)
	{
		mainzone->DumpHeap(con);
		con << endl;
		minizone->DumpHeap(con);
		return;
	}
	ofstream f(va("%s/%s", fl_gamedir, Argv(1)));
	mainzone->DumpHeap(f);
	f << endl;
	minizone->DumpHeap(f);
	f.close();
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
