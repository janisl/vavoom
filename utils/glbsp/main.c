//------------------------------------------------------------------------
// MAIN : Main program for glBSP
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
#include <assert.h>

#include "blockmap.h"
#include "level.h"
#include "node.h"
#include "seg.h"
#include "structs.h"
#include "wad.h"


#define VERS  "1.91"


static char *input_wad  = NULL;
static char *output_wad = NULL;

static int num_files = 0;


/* ----- user information ----------------------------- */

static void ShowTitle(void)
{
  PrintMsg(
    "\n"
    "*** GL BSP Node Builder.  " VERS " (C) 2000 Andrew Apted.  ***\n"
    // -JL- let people know that this version was modified by me
    "*** Additional changes (C) 2001 JÆnis Legzdi·ý         ***\n"
    "*** Based on BSP 2.3 (C) 1998 Colin Reed, Lee Killough ***\n\n");
}

static void ShowUsage(void)
{
  PrintMsg(
    "This GL Node Builder is based on BSP 2.3, which was created\n"
    "from the basic theory stated in DEU5 (OBJECTS.C)\n"
    "\n"
    "Credits should go to :-\n"
    "  Andy Baker & Marc Pullen   for lots of help (testing etc..)\n"
    "  Colin Reed & Lee Killough  for creating the original BSP\n"
    "  Matt Fell                  for the Doom Specs\n"
    "  Raphael Quinet             for DEU and the original idea\n"
    "  ... and everyone who helped with the original BSP.\n"
    "\n"
    "This program is free software, under the terms of the GNU General\n"
    "Public License, and comes with ABSOLUTELY NO WARRANTY.  See the\n"
    "accompanying documentation for more details.\n"
    "\n"
    "Usage: glbsp [options] input.wad [ -o output.wad ]\n"
    "\n"
    "For a list of the available options, type: glbsp -help\n"
  );

  exit(1);
}

static void ShowHelp(void)
{
  PrintMsg(
    "Usage: glbsp [options] input.wad [ -o output.wad ]\n"
    "\n"
    "General Options:\n"
    "  -factor <nnn>    Changes the cost assigned to SEG splits\n"
    "  -noreject        Does not clobber the reject map\n"
    "  -noprog          Does not show progress indicator\n"
    "  -warn            Show extra warning messages\n"
// -JL- Hexen format is detected automaticly
//    "  -hexen           Handle Hexen wads\n"
    "  -packsides       Pack sidedefs (remove duplicates)\n"
    "  -v1              Output V1.0 vertices (backwards compat.)\n"
    "\n"
    "Rarely Useful:\n"
    "  -loadall         Loads all data from source wad (don't copy)\n"
    "  -nogl            Does not compute the GL-friendly nodes\n"
    "  -nonormal        Does not add (if missing) the normal nodes\n"
    "  -forcenormal     Forces the normal nodes to be recomputed\n"
    "  -forcegwa        Forces the output file to be GWA style\n"
    "  -keepsect        Don't prune unused sectors\n"
    "  -noprune         Don't prune anything that is unused\n"
  );

  exit(1);
}


/* ----- option parsing ----------------------------- */

typedef struct option_s
{
  char *name;
  void *var;
  enum { NONE, INT, STRING } kind;
}
option_t;

static option_t option_list[] =
{
  { "o",           &output_wad,    STRING },
  { "noreject",    &no_reject,     NONE   },
  { "noprog",      &no_progress,   NONE   },
  { "warn",        &mini_warnings, NONE   },
// -JL- Hexen format is detected automaticly
//  { "hexen",       &hexen_mode,    NONE   },
  { "packsides",   &pack_sides,    NONE   },
  { "v1",          &v1_vert,       NONE   },

  { "factor",      &factor,        INT    },
  { "loadall",     &load_all,      NONE   },
  { "nogl",        &no_gl,         NONE   },
  { "nonormal",    &no_normal,     NONE   },
  { "forcenormal", &force_normal,  NONE   },
  { "forcegwa",    &gwa_mode,      NONE   },
  { "keepsect",    &keep_sect,     NONE   },
  { "noprune",     &no_prune,      NONE   },

  { NULL, NULL, NONE }
};

static void ParseOptions(int argc, char **argv)
{
  char *opt_str;
  option_t *cur;

  int *temp_int;
  char **temp_str;

  // skip program name
  argv++; argc--;

  if (argc == 0)
    ShowUsage();
 
  if (strcmp(argv[0], "/?") == 0)
    ShowHelp();

  while (argc > 0)
  {
    if (argv[0][0] != '-')
    {
      // --- ORDINARY FILENAME ---

      if (num_files >= 1)
        FatalError("Too many filenames.  Use the -o option", argv[0]);

      input_wad = argv[0];
      num_files++;

      argv++; argc--;
      continue;
    }

    // --- AN OPTION ---

    opt_str = & argv[0][1];

    // handle GNU style options beginning with `--'
    if (opt_str[0] == '-')
      opt_str++;

    if (StrCaseCmp(opt_str, "help") == 0 ||
        StrCaseCmp(opt_str, "h") == 0)
    {
      ShowHelp();
    }

    // find option in list
    for (cur = option_list; cur->name; cur++)
    {
      if (StrCaseCmp(opt_str, cur->name) == 0)
        break;
    }

    if (! cur->name)
      FatalError("Unknown option: %s", argv[0]);

    switch (cur->kind)
    {
      case NONE:
        temp_int = (int *) cur->var;
        (*temp_int)++;
        argv++; argc--;
        break;

      case INT:
        // need an extra argument
        if (argc < 2)
          FatalError("Missing number for the %s option", argv[0]);

        temp_int = (int *) cur->var;
        *temp_int = (int) strtol(argv[1], NULL, 10);
        argv += 2; argc -= 2;
        break;

      case STRING:
        // need an extra argument
        if (argc < 2)
          FatalError("Missing name for the %s option", argv[0]);

        temp_str = (char **) cur->var;

        if (*temp_str != NULL)
          FatalError("The %s option cannot be used twice", argv[0]);
          
        *temp_str = argv[1];
        argv += 2; argc -= 2;
        break;
    }
  }
}

static void CheckOptions(void)
{
  if (!input_wad)
    FatalError("Missing input filename");

  if (!output_wad)
  {
    output_wad = ReplaceExtension(input_wad, "gwa");
    gwa_mode = 1;

    PrintMsg("* No output file specified. Using: %s\n\n", output_wad);
  }
  else if (CheckExtension(output_wad, "gwa"))
    gwa_mode = 1;

  if (strcmp(input_wad, output_wad) == 0)
  {
    PrintMsg("* Output file is same as input file. Using -loadall\n\n");
    load_all = 1;
  }

  if (no_normal && force_normal)
    FatalError("-forcenormal and -nonormal cannot be used together");
  
  if (no_normal && no_gl)
    FatalError("-nonormal and -nogl specified: nothing to do !");
  
  if (gwa_mode && no_gl)
    FatalError("-nogl with GWA file: nothing to do !");
 
  if (gwa_mode && force_normal)
    FatalError("-forcenormal used, but GWA files don't have normal nodes");
  
  if (no_prune && pack_sides)
    FatalError("-noprune and -packsides cannot be used together");

  if (factor <= 0)
    FatalError("Bad factor value: %d", factor);
}


/* ----- build nodes for a single level --------------------------- */

static void HandleLevel(void)
{
  superblock_t *seg_list;
  node_t *root_node;
  subsec_t *root_sub;

  LoadLevel();

  InitBlockmap();

  // create initial segs
  seg_list = CreateSegs();

  StartProgress(100);

  // recursively create nodes
  BuildNodes(seg_list, &root_node, &root_sub, 0);
  FreeSuper(seg_list);

  ClearProgress();

  ClockwiseBspTree(root_node);

  PrintMsg("Built %d NODES, %d SSECTORS, %d SEGS, %d VERTEXES\n",
      num_nodes, num_subsecs, num_segs, num_normal_vert + num_gl_vert);

  if (root_node)
    PrintMsg("Heights of left and right subtrees = (%d,%d)\n",
        ComputeHeight(root_node->r.node), ComputeHeight(root_node->l.node));

  SaveLevel(root_node);

  FreeLevel();
  FreeQuickAllocCuts();
  FreeQuickAllocSupers();
}


/* ----- Main Program -------------------------------------- */

int main(int argc, char **argv)
{
  InitProgress();

  ShowTitle();

  ParseOptions(argc, argv);
  CheckOptions();

  // opens and reads directory from the input wad
  ReadWadFile(input_wad);

  PrintMsg("\nCreating nodes using tunable factor of %d\n", factor);

  while (FindNextLevel())
  {
    HandleLevel();
  }

  // writes all the lumps to the output wad
  WriteWadFile(output_wad);

  // close wads and free memory
  CloseWads();

  TermProgress();

  return 0;
}

