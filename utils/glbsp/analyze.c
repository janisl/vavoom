//------------------------------------------------------------------------
// ANALYZE : Analyzing level structures
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000-2003 Andrew Apted
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

#include "analyze.h"
#include "blockmap.h"
#include "level.h"
#include "node.h"
#include "reject.h"
#include "seg.h"
#include "structs.h"
#include "util.h"
#include "wad.h"


#define DEBUG_WALLTIPS  0
#define DEBUG_POLYOBJ   0
#define DEBUG_DUMMY     0

#define POLY_BOX_SZ  10

// forward declarations
static int VertexCheckMultiSectors(vertex_t *v);

// stuff needed from level.c (this file closely related)
extern vertex_t  ** lev_vertices;
extern linedef_t ** lev_linedefs;
extern sidedef_t ** lev_sidedefs;
extern sector_t  ** lev_sectors;

extern boolean_g lev_doing_normal;
extern boolean_g lev_doing_gl;


/* ----- polyobj handling ----------------------------- */

static void MarkPolyobjSector(sector_t *sector)
{
  int i;
    
  if (! sector)
    return;

# if DEBUG_POLYOBJ
  PrintDebug("  Marking SECTOR %d\n", sector->index);
# endif

  /* already marked ? */
  if (sector->has_polyobj)
    return;

  /* mark all lines of this sector as precious, to prevent the sector
   * from being split.
   */ 
  sector->has_polyobj = TRUE;

  for (i = 0; i < num_linedefs; i++)
  {
    linedef_t *L = lev_linedefs[i];

    if ((L->right && L->right->sector == sector) ||
        (L->left && L->left->sector == sector))
    {
      L->is_precious = TRUE;
    }
  }
}

static void MarkPolyobjPoint(float_g x, float_g y)
{
  int i;
  int inside_count = 0;
 
  float_g best_dist = 999999;
  linedef_t *best_match = NULL;
  sector_t *sector = NULL;

  float_g x1, y1;
  float_g x2, y2;

  // -AJA- First we handle the "awkward" cases where the polyobj sits
  //       directly on a linedef or even a vertex.  We check all lines
  //       that intersect a small box around the spawn point.

  int bminx = (int) (x - POLY_BOX_SZ);
  int bminy = (int) (y - POLY_BOX_SZ);
  int bmaxx = (int) (x + POLY_BOX_SZ);
  int bmaxy = (int) (y + POLY_BOX_SZ);

  for (i = 0; i < num_linedefs; i++)
  {
    linedef_t *L = lev_linedefs[i];

    if (CheckLinedefInsideBox(bminx, bminy, bmaxx, bmaxy,
          (int) L->start->x, (int) L->start->y,
          (int) L->end->x, (int) L->end->y))
    {
#     if DEBUG_POLYOBJ
      PrintDebug("  Touching line was %d\n", L->index);
#     endif

      if (L->left)
        MarkPolyobjSector(L->left->sector);

      if (L->right)
        MarkPolyobjSector(L->right->sector);

      inside_count++;
    }
  }

  if (inside_count > 0)
    return;

  // -AJA- Algorithm is just like in DEU: we cast a line horizontally
  //       from the given (x,y) position and find all linedefs that
  //       intersect it, choosing the one with the closest distance.
  //       If the point is sitting directly on a (two-sided) line,
  //       then we mark the sectors on both sides.

  for (i = 0; i < num_linedefs; i++)
  {
    linedef_t *L = lev_linedefs[i];

    float_g x_cut;

    x1 = L->start->x;  y1 = L->start->y;
    x2 = L->end->x;    y2 = L->end->y;

    /* check vertical range */
    if (fabs(y2 - y1) < DIST_EPSILON)
      continue;

    if ((y > (y1 + DIST_EPSILON) && y > (y2 + DIST_EPSILON)) || 
        (y < (y1 - DIST_EPSILON) && y < (y2 - DIST_EPSILON)))
      continue;

    x_cut = x1 + (x2 - x1) * (y - y1) / (y2 - y1) - x;

    if (fabs(x_cut) < fabs(best_dist))
    {
      /* found a closer linedef */

      best_match = L;
      best_dist = x_cut;
    }
  }

  if (! best_match)
  {
    PrintWarn("Bad polyobj thing at (%1.0f,%1.0f).\n", x, y);
    return;
  }

  y1 = best_match->start->y;
  y2 = best_match->end->y;

# if DEBUG_POLYOBJ
  PrintDebug("  Closest line was %d Y=%1.0f..%1.0f (dist=%1.1f)\n",
      best_match->index, y1, y2, best_dist);
# endif

  /* sanity check: shouldn't be directly on the line */
# if DEBUG_POLYOBJ
  if (fabs(best_dist) < DIST_EPSILON)
  {
    PrintDebug("  Polyobj FAILURE: directly on the line (%d)\n",
        best_match->index);
  }
# endif
 
  /* check orientation of line, to determine which side the polyobj is
   * actually on.
   */
  if ((y1 > y2) == (best_dist > 0))
    sector = best_match->right ? best_match->right->sector : NULL;
  else
    sector = best_match->left ? best_match->left->sector : NULL;

# if DEBUG_POLYOBJ
  PrintDebug("  Sector %d contains the polyobj.\n", 
      sector ? sector->index : -1);
# endif

  if (! sector)
  {
    PrintWarn("Invalid Polyobj thing at (%1.0f,%1.0f).\n", x, y);
    return;
  }

  MarkPolyobjSector(sector);
}

//
// DetectPolyobjSectors
//
// Based on code courtesy of Janis Legzdinsh.
//
void DetectPolyobjSectors(void)
{
  int i;
  int hexen_style;

  // -JL- There's a conflict between Hexen polyobj thing types and Doom thing
  //      types. In Doom type 3001 is for Imp and 3002 for Demon. To solve
  //      this problem, first we are going through all lines to see if the
  //      level has any polyobjs. If found, we also must detect what polyobj
  //      thing types are used - Hexen ones or ZDoom ones. That's why we
  //      are going through all things searching for ZDoom polyobj thing
  //      types. If any found, we assume that ZDoom polyobj thing types are
  //      used, otherwise Hexen polyobj thing types are used.

  // -JL- First go through all lines to see if level contains any polyobjs
  for (i = 0; i < num_linedefs; i++)
  {
    linedef_t *L = lev_linedefs[i];

    if (L->type == HEXTYPE_POLY_START || L->type == HEXTYPE_POLY_EXPLICIT)
      break;
  }

  if (i == num_linedefs)
  {
    // -JL- No polyobjs in this level
    return;
  }

  // -JL- Detect what polyobj thing types are used - Hexen ones or ZDoom ones
  hexen_style = TRUE;
  
  for (i = 0; i < num_things; i++)
  {
    thing_t *T = LookupThing(i);

    if (T->type == ZDOOM_PO_SPAWN_TYPE || T->type == ZDOOM_PO_SPAWNCRUSH_TYPE)
    {
      // -JL- A ZDoom style polyobj thing found
      hexen_style = FALSE;
      break;
    }
  }

# if DEBUG_POLYOBJ
  PrintDebug("Using %s style polyobj things\n",
      hexen_style ? "HEXEN" : "ZDOOM");
# endif
   
  for (i = 0; i < num_things; i++)
  {
    thing_t *T = LookupThing(i);

    float_g x = (float_g) T->x;
    float_g y = (float_g) T->y;

    // ignore everything except polyobj start spots
    if (hexen_style)
    {
      // -JL- Hexen style polyobj things
      if (T->type != PO_SPAWN_TYPE && T->type != PO_SPAWNCRUSH_TYPE)
        continue;
    }
    else
    {
      // -JL- ZDoom style polyobj things
      if (T->type != ZDOOM_PO_SPAWN_TYPE && T->type != ZDOOM_PO_SPAWNCRUSH_TYPE)
        continue;
    }

#   if DEBUG_POLYOBJ
    PrintDebug("Thing %d at (%1.0f,%1.0f) is a polyobj spawner.\n", i, x, y);
#   endif
 
    MarkPolyobjPoint(x, y);
  }
}

//
// BoxContainsThing
//
static int BoxContainsThing(const bbox_t *bbox)
{
  int i;

  for (i = 0; i < num_things; i++)
  {
    thing_t *T = LookupThing(i);

    if (T->x < bbox->minx || T->x > bbox->maxx)
      continue;

    if (T->y < bbox->miny || T->y > bbox->maxy)
      continue;

    return TRUE;
  }

  return FALSE;
}


/* ----- analysis routines ----------------------------- */

static int VertexCompare(const void *p1, const void *p2)
{
  int vert1 = ((const uint16_g *) p1)[0];
  int vert2 = ((const uint16_g *) p2)[0];

  vertex_t *A = lev_vertices[vert1];
  vertex_t *B = lev_vertices[vert2];

  if (vert1 == vert2)
    return 0;

  if ((int)A->x != (int)B->x)
    return (int)A->x - (int)B->x; 
  
  return (int)A->y - (int)B->y;
}

static int SidedefCompare(const void *p1, const void *p2)
{
  int comp;

  int side1 = ((const uint16_g *) p1)[0];
  int side2 = ((const uint16_g *) p2)[0];

  sidedef_t *A = lev_sidedefs[side1];
  sidedef_t *B = lev_sidedefs[side2];

  if (side1 == side2)
    return 0;

  // don't merge sidedefs on special lines
  if (A->on_special || B->on_special)
    return side1 - side2;

  if (A->sector != B->sector)
  {
    if (A->sector == NULL) return -1;
    if (B->sector == NULL) return +1;

    return (A->sector->index - B->sector->index);
  }

  if ((int)A->x_offset != (int)B->x_offset)
    return A->x_offset - (int)B->x_offset;

  if ((int)A->y_offset != B->y_offset)
    return (int)A->y_offset - (int)B->y_offset;

  // compare textures

  comp = memcmp(A->upper_tex, B->upper_tex, sizeof(A->upper_tex));
  if (comp) return comp;
  
  comp = memcmp(A->lower_tex, B->lower_tex, sizeof(A->lower_tex));
  if (comp) return comp;
  
  comp = memcmp(A->mid_tex, B->mid_tex, sizeof(A->mid_tex));
  if (comp) return comp;

  // sidedefs must be the same
  return 0;
}

void DetectDuplicateVertices(void)
{
  int i;
  uint16_g *array = UtilCalloc(num_vertices * sizeof(uint16_g));

  DisplayTicker();

  // sort array of indices
  for (i=0; i < num_vertices; i++)
    array[i] = i;
  
  qsort(array, num_vertices, sizeof(uint16_g), VertexCompare);

  // now mark them off
  for (i=0; i < num_vertices - 1; i++)
  {
    // duplicate ?
    if (VertexCompare(array + i, array + i+1) == 0)
    {
      vertex_t *A = lev_vertices[array[i]];
      vertex_t *B = lev_vertices[array[i+1]];

      // found a duplicate !
      if (A->equiv)
        B->equiv = A->equiv;
      else
        B->equiv = A;
    }
  }

  UtilFree(array);
}

void DetectDuplicateSidedefs(void)
{
  int i;
  uint16_g *array = UtilCalloc(num_sidedefs * sizeof(uint16_g));

  DisplayTicker();

  // sort array of indices
  for (i=0; i < num_sidedefs; i++)
    array[i] = i;
  
  qsort(array, num_sidedefs, sizeof(uint16_g), SidedefCompare);

  // now mark them off
  for (i=0; i < num_sidedefs - 1; i++)
  {
    // duplicate ?
    if (SidedefCompare(array + i, array + i+1) == 0)
    {
      sidedef_t *A = lev_sidedefs[array[i]];
      sidedef_t *B = lev_sidedefs[array[i+1]];

      // found a duplicate !
      if (A->equiv)
        B->equiv = A->equiv;
      else
        B->equiv = A;
    }
  }

  UtilFree(array);
}

void PruneLinedefs(void)
{
  int i;
  int new_num;

  DisplayTicker();

  // scan all linedefs
  for (i=0, new_num=0; i < num_linedefs; i++)
  {
    linedef_t *L = lev_linedefs[i];

    // handle duplicated vertices
    while (L->start->equiv)
    {
      L->start->ref_count--;
      L->start = L->start->equiv;
      L->start->ref_count++;
    }

    while (L->end->equiv)
    {
      L->end->ref_count--;
      L->end = L->end->equiv;
      L->end->ref_count++;
    }

    // handle duplicated sidedefs
    while (L->right && L->right->equiv)
    {
      L->right->ref_count--;
      L->right = L->right->equiv;
      L->right->ref_count++;
    }

    while (L->left && L->left->equiv)
    {
      L->left->ref_count--;
      L->left = L->left->equiv;
      L->left->ref_count++;
    }

    // remove zero length lines
    if (L->zero_len)
    {
      L->start->ref_count--;
      L->end->ref_count--;

      UtilFree(L);
      continue;
    }

    L->index = new_num;
    lev_linedefs[new_num++] = L;
  }

  if (new_num < num_linedefs)
  {
    PrintVerbose("Pruned %d zero-length linedefs\n", num_linedefs - new_num);
    num_linedefs = new_num;
  }

  if (new_num == 0)
    FatalError("Couldn't find any Linedefs");
}

void PruneVertices(void)
{
  int i;
  int new_num;
  int unused = 0;

  DisplayTicker();

  // scan all vertices
  for (i=0, new_num=0; i < num_vertices; i++)
  {
    vertex_t *V = lev_vertices[i];

    if (V->ref_count < 0)
      InternalError("Vertex %d ref_count is %d", i, V->ref_count);
    
    if (V->ref_count == 0)
    {
      if (V->equiv == NULL)
        unused++;

      UtilFree(V);
      continue;
    }

    V->index = new_num;
    lev_vertices[new_num++] = V;
  }

  if (new_num < num_vertices)
  {
    int dup_num = num_vertices - new_num - unused;

    if (unused > 0)
      PrintVerbose("Pruned %d unused vertices "
        "(this is normal if the nodes were built before)\n", unused);

    if (dup_num > 0)
      PrintVerbose("Pruned %d duplicate vertices\n", dup_num);

    num_vertices = new_num;
  }

  if (new_num == 0)
    FatalError("Couldn't find any Vertices");
 
  num_normal_vert = num_vertices;
}

void PruneSidedefs(void)
{
  int i;
  int new_num;
  int unused = 0;

  DisplayTicker();

  // scan all sidedefs
  for (i=0, new_num=0; i < num_sidedefs; i++)
  {
    sidedef_t *S = lev_sidedefs[i];

    if (S->ref_count < 0)
      InternalError("Sidedef %d ref_count is %d", i, S->ref_count);
    
    if (S->ref_count == 0)
    {
      if (S->sector)
        S->sector->ref_count--;

      if (S->equiv == NULL)
        unused++;

      UtilFree(S);
      continue;
    }

    S->index = new_num;
    lev_sidedefs[new_num++] = S;
  }

  if (new_num < num_sidedefs)
  {
    int dup_num = num_sidedefs - new_num - unused;

    if (unused > 0)
      PrintVerbose("Pruned %d unused sidedefs\n", unused);

    if (dup_num > 0)
      PrintVerbose("Pruned %d duplicate sidedefs\n", dup_num);

    num_sidedefs = new_num;
  }

  if (new_num == 0)
    FatalError("Couldn't find any Sidedefs");
}

void PruneSectors(void)
{
  int i;
  int new_num;

  DisplayTicker();

  // scan all sectors
  for (i=0, new_num=0; i < num_sectors; i++)
  {
    sector_t *S = lev_sectors[i];

    if (S->ref_count < 0)
      InternalError("Sector %d ref_count is %d", i, S->ref_count);
    
    if (S->ref_count == 0)
    {
      UtilFree(S);
      continue;
    }

    S->index = new_num;
    lev_sectors[new_num++] = S;
  }

  if (new_num < num_sectors)
  {
    PrintVerbose("Pruned %d unused sectors\n", num_sectors - new_num);
    num_sectors = new_num;
  }

  if (new_num == 0)
    FatalError("Couldn't find any Sectors");
}

void DetectDummySectors(void)
{
  // Dummy sectors are detected with the following criteria:
  //   (a) not larger than 128x128.
  //   (b) all linedefs are one-sided (isolated from main area).
  //   (c) contains NO things.

  int i;
  int count;

  char *notdummy;
  bbox_t *bboxes;

  if (num_sectors == 0)
    return;

  DisplayTicker();

  notdummy = (char *) UtilCalloc(num_sectors);
  bboxes = (bbox_t *) UtilCalloc(num_sectors * sizeof(bbox_t));

  // reset bounding boxes
  for (i = 0; i < num_sectors; i++) 
  {
    notdummy[i] = FALSE;

    bboxes[i].minx = bboxes[i].miny = SHRT_MAX;
    bboxes[i].maxx = bboxes[i].maxy = SHRT_MIN;
  }

  // pass over all linedefs, checking if two-sided and computing bboxes
  for (i = 0; i < num_linedefs; i++)
  {
    linedef_t *L = lev_linedefs[i];

    int s1 = (L->right && L->right->sector) ? L->right->sector->index : -1;
    int s2 = (L->left  && L->left->sector)  ? L->left->sector->index  : -1;

    if (L->zero_len)
      continue;

    if ((L->left && L->right) || 
        VertexCheckMultiSectors(L->start) ||
        VertexCheckMultiSectors(L->end))
    {
      if (s1 >= 0) notdummy[s1] = TRUE;
      if (s2 >= 0) notdummy[s2] = TRUE;
    }
    else  // update BBOX
    {
      float_g x1 = L->start->x;
      float_g y1 = L->start->y;
      float_g x2 = L->end->x;
      float_g y2 = L->end->y;

      int minx = MIN((int)floor(x1), (int)floor(x2));
      int miny = MIN((int)floor(y1), (int)floor(y2));
      int maxx = MAX((int)ceil(x1), (int)ceil(x2));
      int maxy = MAX((int)ceil(y1), (int)ceil(y2));

      if (s1 >= 0)
      {
        bboxes[s1].minx = MIN(bboxes[s1].minx, minx);
        bboxes[s1].miny = MIN(bboxes[s1].miny, miny);
        bboxes[s1].maxx = MAX(bboxes[s1].maxx, maxx);
        bboxes[s1].maxy = MAX(bboxes[s1].maxy, maxy);
      }
      if (s2 >= 0)
      {
        bboxes[s2].minx = MIN(bboxes[s2].minx, minx);
        bboxes[s2].miny = MIN(bboxes[s2].miny, miny);
        bboxes[s2].maxx = MAX(bboxes[s2].maxx, maxx);
        bboxes[s2].maxy = MAX(bboxes[s2].maxy, maxy);
      }
    }
  }

  // check the bboxes, and count the dummies
  count = 0;

  for (i = 0; i < num_sectors; i++)
  {
    if (notdummy[i])
      continue;

    if (lev_sectors[i]->coalesce || lev_sectors[i]->has_polyobj)
    {
      notdummy[i] = TRUE;
      continue;
    }

    // ignore sectors which have no linedefs
    if (bboxes[i].minx == SHRT_MAX)
    {
      notdummy[i] = TRUE;
      continue;
    }

#   if DEBUG_DUMMY
    PrintDebug("  Bounding box for isolated sector %d: (%d,%d) .. (%d,%d)\n",
        i, bboxes[i].minx, bboxes[i].miny, bboxes[i].maxx, bboxes[i].maxy);
#   endif

    if (bboxes[i].maxx - bboxes[i].minx > 128 ||
        bboxes[i].maxy - bboxes[i].miny > 128)
    {
      notdummy[i] = TRUE;
      continue;
    }

    // check if the sector contains a thing
    if (BoxContainsThing(bboxes + i))
    {
#     if DEBUG_DUMMY
      PrintDebug("  Isolated sector %d contains a thing\n", i);
#     endif

      notdummy[i] = TRUE;
      continue;
    }

    // must be a dummy sector
    count++;

    lev_sectors[i]->is_dummy = TRUE;

#   if DEBUG_DUMMY
    PrintDebug("  Sector %d is DUMMY\n", i);
#   endif
  }

  if (count > 0)
  {
      PrintVerbose("Ignoring %d dummy sectors\n", count);
  }

  UtilFree(bboxes);
  UtilFree(notdummy);
}


/* ----- vertex routines ------------------------------- */

static void VertexAddWallTip(vertex_t *vert, float_g dx, float_g dy,
  sector_t *left, sector_t *right)
{
  wall_tip_t *tip = NewWallTip();
  wall_tip_t *after;

  tip->angle = UtilComputeAngle(dx, dy);
  tip->left  = left;
  tip->right = right;

  // find the correct place (order is increasing angle)
  for (after=vert->tip_set; after && after->next; after=after->next)
  { }

  while (after && tip->angle + ANG_EPSILON < after->angle) 
    after = after->prev;
  
  // link it in
  tip->next = after ? after->next : vert->tip_set;
  tip->prev = after;

  if (after)
  {
    if (after->next)
      after->next->prev = tip;
    
    after->next = tip;
  }
  else
  {
    if (vert->tip_set)
      vert->tip_set->prev = tip;
    
    vert->tip_set = tip;
  }
}

void CalculateWallTips(void)
{
  int i;

  DisplayTicker();

  for (i=0; i < num_linedefs; i++)
  {
    linedef_t *line = lev_linedefs[i];

    float_g x1 = line->start->x;
    float_g y1 = line->start->y;
    float_g x2 = line->end->x;
    float_g y2 = line->end->y;

    sector_t *left  = (line->left)  ? line->left->sector  : NULL;
    sector_t *right = (line->right) ? line->right->sector : NULL;
    
    VertexAddWallTip(line->start, x2-x1, y2-y1, left, right);
    VertexAddWallTip(line->end,   x1-x2, y1-y2, right, left);
  }
 
# if DEBUG_WALLTIPS
  for (i=0; i < num_vertices; i++)
  {
    vertex_t *vert = LookupVertex(i);
    wall_tip_t *tip;

    PrintDebug("WallTips for vertex %d:\n", i);

    for (tip=vert->tip_set; tip; tip=tip->next)
    {
      PrintDebug("  Angle=%1.1f left=%d right=%d\n", tip->angle,
        tip->left ? tip->left->index : -1,
        tip->right ? tip->right->index : -1);
    }
  }
# endif
}

//
// NewVertexFromSplitSeg
//
vertex_t *NewVertexFromSplitSeg(seg_t *seg, float_g x, float_g y)
{
  vertex_t *vert = NewVertex();

  vert->x = x;
  vert->y = y;

  vert->ref_count = seg->partner ? 4 : 2;

  if (lev_doing_gl && (!cur_info->v1_vert || !lev_doing_normal))
  {
    vert->index = num_gl_vert | 0x8000;
    num_gl_vert++;
  }
  else
  {
    vert->index = num_normal_vert;
    num_normal_vert++;
  }

  // compute wall_tip info

  VertexAddWallTip(vert, -seg->pdx, -seg->pdy, seg->sector, 
      seg->partner ? seg->partner->sector : NULL);

  VertexAddWallTip(vert, seg->pdx, seg->pdy,
      seg->partner ? seg->partner->sector : NULL, seg->sector);

  // create a duplex vertex if needed

  if (lev_doing_normal && lev_doing_gl && !cur_info->v1_vert)
  {
    vert->normal_dup = NewVertex();

    vert->normal_dup->x = x;
    vert->normal_dup->y = y;
    vert->normal_dup->ref_count = vert->ref_count;

    vert->normal_dup->index = num_normal_vert;
    num_normal_vert++;
  }

  return vert;
}

//
// NewVertexDegenerate
//
vertex_t *NewVertexDegenerate(vertex_t *start, vertex_t *end)
{
  float_g dx = end->x - start->x;
  float_g dy = end->y - start->y;

  float_g dlen = UtilComputeDist(dx, dy);

  vertex_t *vert = NewVertex();

  vert->ref_count = start->ref_count;

  if (lev_doing_normal)
  {
    vert->index = num_normal_vert;
    num_normal_vert++;
  }
  else
  {
    vert->index = num_gl_vert | 0x8000;
    num_gl_vert++;
  }

  // compute new coordinates

  vert->x = start->x;
  vert->y = start->x;

  if (dlen == 0)
    InternalError("NewVertexDegenerate: bad delta !");

  dx /= dlen;
  dy /= dlen;

  while ((int)vert->x == (int)start->x && 
         (int)vert->y == (int)start->y)
  {
    vert->x += dx;
    vert->y += dy;
  }

  return vert;
}

//
// VertexCheckOpen
//
int VertexCheckOpen(vertex_t *vert, float_g dx, float_g dy,
    sector_t ** left_sec, sector_t ** right_sec)
{
  wall_tip_t *tip;

  angle_g angle = UtilComputeAngle(dx, dy);

  *left_sec = *right_sec = NULL;

  // first check whether there's a wall_tip that lies in the exact
  // direction of the given direction (which is relative to the
  // vertex).

  for (tip=vert->tip_set; tip; tip=tip->next)
  {
    if (fabs(tip->angle - angle) < ANG_EPSILON)
    {
      // yes, found one

      *left_sec  = tip->left;
      *right_sec = tip->right;

      return FALSE;
    }
  }

  // OK, now just find the first wall_tip whose angle is greater than
  // the angle we're interested in.  Therefore we'll be on the RIGHT
  // side of that wall_tip.

  for (tip=vert->tip_set; tip; tip=tip->next)
  {
    if (angle + ANG_EPSILON < tip->angle)
    {
      // found it
      *left_sec = *right_sec = tip->right;

      return (tip->right != NULL);
    }

    if (! tip->next)
    {
      // no more tips, thus we must be on the LEFT side of the tip
      // with the largest angle.

      *left_sec = *right_sec = tip->left;

      return (tip->left != NULL);
    }
  }
  
  InternalError("Vertex %d has no tips !", vert->index);
  return FALSE;
}

//
// VertexCheckMultiSectors
//
static int VertexCheckMultiSectors(vertex_t *vert)
{
  sector_t *sec = NULL;

  wall_tip_t *tip;

  for (tip = vert->tip_set; tip; tip=tip->next)
  {
    if (tip->left)
    {
      if (! sec)
        sec = tip->left;
      else if (sec != tip->left)
        return TRUE;
    }

    if (tip->right)
    {
      if (! sec)
        sec = tip->right;
      else if (sec != tip->right)
        return TRUE;
    }
  }

  return FALSE;  // zero or one sector
}

