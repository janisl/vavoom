//------------------------------------------------------------------------
// WAD : WAD read/write functions.
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000 Andrew Apted
//
//  Based on `BSP 2.3' by Colin Reed, Lee Killough and others.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//------------------------------------------------------------------------

#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <errno.h>

#include "blockmap.h"
#include "level.h"
#include "node.h"
#include "seg.h"
#include "structs.h"
#include "wad.h"


#define DEBUG_DIR   0
#define DEBUG_LUMP  0


int hexen_mode=0;
int load_all=0;
int no_reject=0;
int no_gl=0;
int no_normal=0;
int force_normal=0;
int gwa_mode=0;
int v1_vert=0;
int keep_sect=0;
int no_prune=0;
int pack_sides=0;

FILE *in_file = NULL;
FILE *out_file = NULL;

wad_t wad;


/* ---------------------------------------------------------------- */


// -JL- Included lump with script sources
#define NUM_LEVEL_LUMPS  12
// -JL- Included PVS lump
#define NUM_GL_LUMPS	 5

static const char *level_lumps[NUM_LEVEL_LUMPS]=
{
  "THINGS", "LINEDEFS", "SIDEDEFS", "VERTEXES", "SEGS", 
  "SSECTORS", "NODES", "SECTORS", "REJECT", "BLOCKMAP",
  "BEHAVIOR"  // <-- hexen support
  // -JL- Lump with script sources
  , "SCRIPTS"
};

static const char *gl_lumps[NUM_GL_LUMPS]=
{
  "GL_VERT", "GL_SEGS", "GL_SSECT", "GL_NODES"
  // -JL- PVS (Potentially Visible Set) lump
  , "GL_PVS"
};


//
// CheckMagic
//
static int CheckMagic(const char type[4])
{
  if ((type[0] == 'I' || type[0] == 'P') && 
       type[1] == 'W' && type[2] == 'A' && type[3] == 'D')
  {
    return TRUE;
  }
  
  return FALSE;
}


//
// CheckLevelName
//
// Tests if the entry name matches ExMy or MAPxx.
//
static int CheckLevelName(const char *name)
{
  // Doom 1:

  if (name[0]=='E' && name[2]=='M' &&
      name[1]>='0' && name[1]<='9' &&
      name[3]>='0' && name[3]<='9' && name[4]==0)
  {
    return TRUE;
  }

  if (name[0]=='E' && name[2]=='M' &&
      name[1]>='0' && name[1]<='9' &&
      name[3]>='0' && name[3]<='9' &&
      name[4]>='0' && name[4]<='9' && name[5]==0)
  {
    return TRUE;
  }

  // Doom 2:

  if (name[0]=='M' && name[1]=='A' && name[2]=='P' &&
      name[3]>='0' && name[3]<='9' &&
      name[4]>='0' && name[4]<='9' && name[5]==0)
  {
    return TRUE;
  }
  
  return FALSE;
}


//
// CheckLevelLumpName
//
// Tests if the entry name is one of the level lumps.
//
static int CheckLevelLumpName(const char *name)
{
  int i;

  for (i=0; i < NUM_LEVEL_LUMPS; i++)
  {
    if (strcmp(name, level_lumps[i]) == 0)
      return TRUE;
  }
  
  return FALSE;
}


//
// CheckGLLumpName
//
// Tests if the entry name matches GL_ExMy or GL_MAPxx, or one of the
// GL lump names.
//
static int CheckGLLumpName(const char *name)
{
  int i;

  if (name[0] != 'G' || name[1] != 'L' || name[2] != '_')
    return 0;

  for (i=0; i < NUM_GL_LUMPS; i++)
  {
    if (strcmp(name, gl_lumps[i]) == 0)
      return TRUE;
  }
  
  return CheckLevelName(name+3);
}


//
// NewLump
//
// Create new lump.  `name' must be allocated storage.
//
static lump_t *NewLump(char *name)
{
  lump_t *cur;

  cur = SysCalloc(sizeof(lump_t));

  cur->name = name;
  cur->start = cur->new_start = -1;
  cur->flags = 0;
  cur->length = 0;
  cur->space = 0;
  cur->data = NULL;
  cur->level_list = NULL;
  cur->level_gl_list = NULL;
  cur->level_buddy = NULL;

  return cur;
}


//
// FreeLump
//
static void FreeLump(lump_t *lump)
{
  // free level lumps, if any
  if (lump->flags & LUMP_IS_LEVEL)
  {
    while (lump->level_list)
    {
      lump_t *head = lump->level_list;
      lump->level_list = head->next;

      // the ol' recursion trick... :)
      FreeLump(head);
    }
  }
  
  // free GL level lumps, if any
  if (lump->flags & LUMP_IS_GL_LEVEL)
  {
    while (lump->level_gl_list)
    {
      lump_t *head = lump->level_gl_list;
      lump->level_gl_list = head->next;

      // the ol' recursion trick... :)
      FreeLump(head);
    }
  }

  // check `data' here, since it gets freed in WriteLumpData()
  if (lump->data)
    SysFree(lump->data);
  
  SysFree(lump->name);
  SysFree(lump);
}


//
// ReadHeader
//
static void ReadHeader(const char *filename)
{
  int len;
  raw_wad_header_t header;

  len = fread(&header, sizeof(header), 1, in_file);

  if (len != 1)
    FatalError("Trouble reading wad header for %s : %s", 
      filename, strerror(errno));

  if (! CheckMagic(header.type))
    FatalError("%s does not appear to be a wad file -- bad magic", filename);

  wad.kind = (header.type[0] == 'I') ? IWAD : PWAD;
  
  wad.num_entries = UINT32(header.num_entries);
  wad.dir_start   = UINT32(header.dir_start);

  // initialise stuff
  wad.dir_head = NULL;
  wad.dir_tail = NULL;
  wad.current_level = NULL;
}


//
// ReadDirEntry
//
static void ReadDirEntry(void)
{
  int len;
  raw_wad_entry_t entry;
  lump_t *lump;
  
  ShowProgress(0);

  len = fread(&entry, sizeof(entry), 1, in_file);

  if (len != 1)
    FatalError("Trouble reading wad directory");

  lump = NewLump(SysStrndup(entry.name, 8));

  lump->start = UINT32(entry.start);
  lump->length = UINT32(entry.length);

  // ignore previous GL lump info
  if (CheckGLLumpName(lump->name))
  {
    #if DEBUG_DIR
    PrintDebug("Discarding previous GL info: %s\n", lump->name);
    #endif

    FreeLump(lump);
    wad.num_entries--;

    return;
  }

  // mark the lump as `ignorable' when in GWA mode.
  if (gwa_mode)
    lump->flags |= LUMP_IGNORE_ME;

  // --- LEVEL MARKERS ---

  if (CheckLevelName(lump->name))
  {
    // NOTE !  Level marks can have data (in Hexen anyway).
    if (load_all)
      lump->flags |= LUMP_READ_ME;
    else
      lump->flags |= LUMP_COPY_ME;

    // OK, start a new level

    lump->flags |= LUMP_IS_LEVEL;
    wad.current_level = lump;

    #if DEBUG_DIR
    PrintDebug("Read dir... %s :\n", lump->name);
    #endif

    // link it in
    lump->next = NULL;
    lump->prev = wad.dir_tail;

    if (wad.dir_tail)
      wad.dir_tail->next = lump;
    else
      wad.dir_head = lump;

    wad.dir_tail = lump;

    return;
  }

  // --- LEVEL LUMPS ---

  if (wad.current_level)
  {
    if (CheckLevelLumpName(lump->name))
    {
      // check for duplicates
      if (FindLevelLump(lump->name))
      {
        PrintWarn("Duplicate entry `%s' ignored in %s\n",
            lump->name, wad.current_level->name);

        FreeLump(lump);
        wad.num_entries--;

        return;
      }

      #if DEBUG_DIR
      PrintDebug("Read dir... |--- %s\n", lump->name);
      #endif

      // mark it to be loaded
      lump->flags |= LUMP_READ_ME;
    
      // link it in
      lump->next = wad.current_level->level_list;
      lump->prev = NULL;

      if (lump->next)
        lump->next->prev = lump;

      wad.current_level->level_list = lump;

      return;
    }
      
    // OK, non-level lump.  End the previous level.

    wad.current_level = NULL;
  }

  // --- ORDINARY LUMPS ---

  #if DEBUG_DIR
  PrintDebug("Read dir... %s\n", lump->name);
  #endif

  if (CheckLevelLumpName(lump->name))
    PrintWarn("Level lump `%s' found outside any level\n", lump->name);

  // maybe load data
  if (load_all)
    lump->flags |= LUMP_READ_ME;
  else
    lump->flags |= LUMP_COPY_ME;

  // link it in
  lump->next = NULL;
  lump->prev = wad.dir_tail;

  if (wad.dir_tail)
    wad.dir_tail->next = lump;
  else
    wad.dir_head = lump;

  wad.dir_tail = lump;
}


//
// ReadDirectory
//
static void ReadDirectory(void)
{
  int i;
  int total_entries = wad.num_entries;

  fseek(in_file, wad.dir_start, SEEK_SET);

  for (i=0; i < total_entries; i++)
  {
    ReadDirEntry();
  }
}


//
// ReadLumpData
//
static void ReadLumpData(lump_t *lump)
{
  int len;

  ShowProgress(0);

  #if DEBUG_LUMP
  PrintDebug("Reading... %s (%d)\n", lump->name, lump->length);
  #endif
  
  if (lump->length == 0)
    return;

  lump->data = SysCalloc(lump->length);

  fseek(in_file, lump->start, SEEK_SET);

  len = fread(lump->data, lump->length, 1, in_file);

  if (len != 1)
  {
    if (wad.current_level)
      PrintWarn("Trouble reading lump `%s' in %s\n",
          lump->name, wad.current_level->name);
    else
      PrintWarn("Trouble reading lump `%s'\n", lump->name);
  }

  lump->flags &= ~LUMP_READ_ME;
}


//
// ReadAllLumps
//
// Returns number of entries read.
//
static int ReadAllLumps(void)
{
  lump_t *cur, *L;
  int count = 0;

  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    count++;

    if (cur->flags & LUMP_READ_ME)
      ReadLumpData(cur);

    if (cur->flags & LUMP_IS_LEVEL)
    {
      for (L=cur->level_list; L; L=L->next)
      {
        count++;

        if (L->flags & LUMP_READ_ME)
          ReadLumpData(L);
      }
    }
  }

  return count;
}


/* ---------------------------------------------------------------- */


//
// WriteHeader
//
static void WriteHeader(void)
{
  int len;
  raw_wad_header_t header;

  switch (wad.kind)
  {
    case IWAD:
      strncpy(header.type, "IWAD", 4);
      break;

    case PWAD:
      strncpy(header.type, "PWAD", 4);
      break;
  }

  header.num_entries = UINT32(wad.num_entries);
  header.dir_start   = UINT32(wad.dir_start);

  len = fwrite(&header, sizeof(header), 1, out_file);

  if (len != 1)
    PrintWarn("Trouble writing wad header\n");
}


//
// CreateGLMarker
//
lump_t *CreateGLMarker(lump_t *level)
{
  lump_t *cur;
  char name_buf[16];

  sprintf(name_buf, "GL_%s", level->name);

  cur = NewLump(SysStrdup(name_buf));
  cur->flags = LUMP_IS_GL_LEVEL;

  // link it in
  cur->next = level->next;
  cur->prev = level;

  if (cur->next)
    cur->next->prev = cur;

  level->next = cur;
  level->level_buddy = cur;

  return cur;
}


//
// SortLumps
//
// Algorithm is pretty simple: for each of the names, if such a lump
// exists in the list, move it to the head of the list.  By going
// backwards through the names, we ensure the correct order.
//
static void SortLumps(lump_t ** list, const char **names, int count)
{
  int i;
  lump_t *cur;

  for (i=count-1; i >= 0; i--)
  {
    for (cur=(*list); cur; cur=cur->next)
    {
      if (strcmp(cur->name, names[i]) != 0)
        continue;

      // unlink it
      if (cur->next)
        cur->next->prev = cur->prev;

      if (cur->prev)
        cur->prev->next = cur->next;
      else
        (*list) = cur->next;
      
      // add to head
      cur->next = (*list);
      cur->prev = NULL;

      if (cur->next)
        cur->next->prev = cur;

      (*list) = cur;

      // continue with next name (important !)
      break;
    }
  }
}


//
// RecomputeDirectory
//
// Calculates all the lump offsets for the directory.
//
static void RecomputeDirectory(void)
{
  lump_t *cur, *L;

  wad.num_entries = 0;
  wad.dir_start = sizeof(raw_wad_header_t);
  
  // run through all the lumps, computing the `new_start' fields, the
  // number of lumps in the directory, the directory starting pos, and
  // also sorting the lumps in the levels.

  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    if (cur->flags & LUMP_IGNORE_ME)
      continue;

    cur->new_start = wad.dir_start;

    wad.dir_start += cur->length;
    wad.num_entries++;

    if (cur->flags & LUMP_IS_LEVEL)
    {
      SortLumps(&cur->level_list, level_lumps, NUM_LEVEL_LUMPS);

      for (L=cur->level_list; L; L=L->next)
      {
        if (L->flags & LUMP_IGNORE_ME)
          continue;

        L->new_start = wad.dir_start;

        wad.dir_start += L->length;
        wad.num_entries++;
      }
    }

    if (cur->flags & LUMP_IS_GL_LEVEL)
    {
      SortLumps(&cur->level_gl_list, gl_lumps, NUM_GL_LUMPS);

      for (L=cur->level_gl_list; L; L=L->next)
      {
        if (L->flags & LUMP_IGNORE_ME)
          continue;

        L->new_start = wad.dir_start;

        wad.dir_start += L->length;
        wad.num_entries++;
      }
    }
  }
}


//
// WriteLumpData
//
static void WriteLumpData(lump_t *lump)
{
  int len;

  ShowProgress(0);

  #if DEBUG_LUMP
  if (lump->flags & LUMP_COPY_ME)
    PrintDebug("Copying... %s (%d)\n", lump->name, lump->length);
  else
    PrintDebug("Writing... %s (%d)\n", lump->name, lump->length);
  #endif
  
  if (ftell(out_file) != lump->new_start)
    PrintWarn("Consistency failure writing %s (%08X, %08X\n", 
      lump->name, ftell(out_file), lump->new_start);
       
  if (lump->length == 0)
    return;

  if (lump->flags & LUMP_COPY_ME)
  {
    lump->data = SysCalloc(lump->length);

    fseek(in_file, lump->start, SEEK_SET);

    len = fread(lump->data, lump->length, 1, in_file);

    if (len != 1)
      PrintWarn("Trouble reading lump %s to copy\n", lump->name);
  }

  len = fwrite(lump->data, lump->length, 1, out_file);
   
  if (len != 1)
    PrintWarn("Trouble writing lump %s\n", lump->name);
  
  SysFree(lump->data);

  lump->data = NULL;
}


//
// WriteAllLumps
//
// Returns number of entries written.
//
static int WriteAllLumps(void)
{
  lump_t *cur, *L;
  int count = 0;

  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    if (cur->flags & LUMP_IGNORE_ME)
      continue;

    WriteLumpData(cur);
    count++;

    if (cur->flags & LUMP_IS_LEVEL)
    {
      for (L=cur->level_list; L; L=L->next)
      {
        if (L->flags & LUMP_IGNORE_ME)
          continue;

        WriteLumpData(L);
        count++;
      }
    }

    if (cur->flags & LUMP_IS_GL_LEVEL)
    {
      for (L=cur->level_gl_list; L; L=L->next)
      {
        if (L->flags & LUMP_IGNORE_ME)
          continue;

        WriteLumpData(L);
        count++;
      }
    }
  }

  fflush(out_file);

  return count;
}


//
// WriteDirEntry
//
static void WriteDirEntry(lump_t *lump)
{
  int len;
  raw_wad_entry_t entry;

  ShowProgress(0);

  strncpy(entry.name, lump->name, 8);

  entry.start = UINT32(lump->new_start);
  entry.length = UINT32(lump->length);

  len = fwrite(&entry, sizeof(entry), 1, out_file);

  if (len != 1)
    PrintWarn("Trouble writing wad directory\n");
}


//
// WriteDirectory
//
// Returns number of entries written.
//
static int WriteDirectory(void)
{
  lump_t *cur, *L;
  int count = 0;
  
  if (ftell(out_file) != wad.dir_start)
    PrintWarn("Consistency failure writing lump directory "
      "(%08X,%08X)\n", ftell(out_file), wad.dir_start);

  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    if (cur->flags & LUMP_IGNORE_ME)
      continue;

    WriteDirEntry(cur);
    count++;

    #if DEBUG_DIR
    if (cur->flags & (LUMP_IS_LEVEL | LUMP_IS_GL_LEVEL))
      PrintDebug("Write dir... %s\n", cur->name);
    else
      PrintDebug("Write dir... %s :\n", cur->name);
    #endif

    if (cur->flags & LUMP_IS_LEVEL)
    {
      for (L=cur->level_list; L; L=L->next)
      {
        if (cur->flags & LUMP_IGNORE_ME)
          continue;

        #if DEBUG_DIR
        PrintDebug("Write dir... |--- %s\n", L->name);
        #endif

        WriteDirEntry(L);
        count++;
      }
    }

    if (cur->flags & LUMP_IS_GL_LEVEL)
    {
      for (L=cur->level_gl_list; L; L=L->next)
      {
        if (cur->flags & LUMP_IGNORE_ME)
          continue;

        #if DEBUG_DIR
        PrintDebug("Write dir... |--- %s\n", L->name);
        #endif

        WriteDirEntry(L);
        count++;
      }
    }
  }

  fflush(out_file);

  return count;
}


/* ---------------------------------------------------------------- */


//
// CheckExtension
//
int CheckExtension(const char *filename, const char *ext)
{
  int A = strlen(filename) - 1;
  int B = strlen(ext) - 1;

  for (; B >= 0; B--, A--)
  {
    if (A < 0)
      return 0;
    
    if (toupper(filename[A]) != toupper(ext[B]))
      return 0;
  }

  return (A >= 1) && (filename[A] == '.');
}

//
// ReplaceExtension
//
char *ReplaceExtension(const char *filename, const char *ext)
{
  char *dot_pos;
  char buffer[512];

  strcpy(buffer, filename);
  
  dot_pos = strrchr(buffer, '.');

  if (dot_pos)
    dot_pos[1] = 0;
  
  strcat(buffer, ext);

  return SysStrdup(buffer);
}


//
// CreateLevelLump
//
lump_t *CreateLevelLump(const char *name)
{
  lump_t *cur;

  #if DEBUG_DIR
  PrintDebug("Create Lump... %s\n", name);
  #endif

  // already exists ?
  for (cur=wad.current_level->level_list; cur; cur=cur->next)
  {
    if (strcmp(name, cur->name) == 0)
      break;
  }
  
  if (cur)
  {
    if (cur->data)
      SysFree(cur->data);
    
    cur->data = NULL;
    cur->length = 0;
    cur->space  = 0;

    return cur;
  }

  // nope, allocate new one
  cur = NewLump(SysStrdup(name));

  // link it in
  cur->next = wad.current_level->level_list;
  cur->prev = NULL;

  if (cur->next)
    cur->next->prev = cur;

  wad.current_level->level_list = cur;

  return cur;
}


//
// CreateGLLump
//
lump_t *CreateGLLump(const char *name)
{
  lump_t *cur;
  lump_t *gl_level;

  #if DEBUG_DIR
  PrintDebug("Create GL Lump... %s\n", name);
  #endif

  // create GL level marker if necessary
  if (! wad.current_level->level_buddy)
    CreateGLMarker(wad.current_level);
  
  gl_level = wad.current_level->level_buddy;

  // check if already exists
  for (cur=gl_level->level_gl_list; cur; cur=cur->next)
  {
    if (strcmp(name, cur->name) == 0)
      break;
  }

  if (cur)
  {
    if (cur->data)
      SysFree(cur->data);
    
    cur->data = NULL;
    cur->length = 0;
    cur->space  = 0;

    return cur;
  }

  // nope, allocate new one
  cur = NewLump(SysStrdup(name));

  // link it in
  cur->next = gl_level->level_gl_list;
  cur->prev = NULL;

  if (cur->next)
    cur->next->prev = cur;

  gl_level->level_gl_list = cur;

  return cur;
}


//
// AppendLevelLump
//
void AppendLevelLump(lump_t *lump, void *data, int length)
{
  if (length == 0)
    return;

  if (lump->length == 0)
  {
    lump->space = MAX(length, 64);
    lump->data = SysCalloc(lump->space);
  }
  else if (lump->space < length)
  {
    lump->space = MAX(length, 64);
    lump->data = SysRealloc(lump->data, lump->length + lump->space);
  }

  memcpy(((char *)lump->data) + lump->length, data, length);

  lump->length += length;
  lump->space  -= length;
}


//
// FindNextLevel
//
int FindNextLevel(void)
{
  lump_t *cur;
  
  if (wad.current_level)
    cur = wad.current_level->next;
  else
    cur = wad.dir_head;

  while (cur && ! (cur->flags & LUMP_IS_LEVEL))
    cur=cur->next;

  wad.current_level = cur;

  return (cur != NULL);
}


//
// FindLevelLump
//
lump_t *FindLevelLump(const char *name)
{
  lump_t *cur = wad.current_level->level_list;

  while (cur && (strcmp(cur->name, name) != 0))
    cur=cur->next;

  return cur;
}


//
// ReadWadFile
//
void ReadWadFile(char *filename)
{
  int check;

  // open input wad file & read header
  in_file = fopen(filename, "rb");

  if (! in_file)
    FatalError("Cannot open WAD file %s : %s", filename, strerror(errno));

  ReadHeader(filename);

  PrintMsg("Opened %cWAD file : %s\n", (wad.kind == IWAD) ? 'I' : 'P', 
      filename); 
  PrintMsg("Reading %d dir entries at 0x%X\n", wad.num_entries, 
      wad.dir_start);

  StartProgress(0);

  // read directory
  ReadDirectory();

  // now read lumps
  check = ReadAllLumps();

  if (check != wad.num_entries)
    PrintWarn("Read directory count consistency failure (%d,%d)\n",
      check, wad.num_entries);
  
  wad.current_level = NULL;

  ClearProgress();
}


//
// WriteWadFile
//
void WriteWadFile(char *filename)
{
  int check1, check2;

  RecomputeDirectory();

  // create output wad file & write the header
  out_file = fopen(filename, "wb");

  if (! out_file)
    FatalError("Could not open output WAD file: %s", strerror(errno));

  WriteHeader();

  StartProgress(0);

  // now write all the lumps to the output wad
  check1 = WriteAllLumps();

  // finally, write out the directory
  check2 = WriteDirectory();

  if (check1 != wad.num_entries || check2 != wad.num_entries)
    PrintWarn("Write directory count consistency failure (%d,%d,%d\n",
      check1, check2, wad.num_entries);

  ClearProgress();

  PrintMsg("\nSaved WAD as %s\n", filename);
}


//
// CloseWads
//
void CloseWads(void)
{
  if (in_file)
    fclose(in_file);
  
  if (out_file)
    fclose(out_file);
  
  // free directory entries
  while (wad.dir_head)
  {
    lump_t *head = wad.dir_head;
    wad.dir_head = head->next;

    FreeLump(head);
  }
}

