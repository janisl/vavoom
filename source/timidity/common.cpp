/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   common.c

   */

#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include "timidity.h"

namespace LibTimidity
{

char				current_filename[1024];

/* The paths in this list will be tried whenever we're reading a file */
static PathList*	pathlist = NULL; /* This is a linked list */

/* This is meant to find and open files for reading */
FILE* open_file(const char* name, int decompress, int noise_mode)
{
	if (!name || !(*name))
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "Attempted to open nameless file.");
		return 0;
	}

	/* First try the given name */

	strncpy(current_filename, name, 1023);
	current_filename[1023] = '\0';

	ctl->cmsg(CMSG_INFO, VERB_DEBUG, "Trying to open %s", current_filename);
	FILE* fp = fopen(current_filename, "rb");
	if (fp)
	{
		return fp;
	}

#ifdef ENOENT
	if (noise_mode && (errno != ENOENT))
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: %s", current_filename, strerror(errno));
		return 0;
	}
#endif

	PathList* plp = pathlist;
	if (name[0] != PATH_SEP)
	{
		while (plp)  /* Try along the path then */
		{
			*current_filename = 0;
			int l = strlen(plp->path);
			if (l)
			{
				strcpy(current_filename, plp->path);
				if (current_filename[l - 1] != PATH_SEP)
				{
					strcat(current_filename, PATH_STRING);
				}
			}
			strcat(current_filename, name);
			ctl->cmsg(CMSG_INFO, VERB_DEBUG, "Trying to open %s", current_filename);
			fp = fopen(current_filename, "rb");
			if (fp)
			{
				return fp;
			}
#ifdef ENOENT
			if (noise_mode && (errno != ENOENT))
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: %s", current_filename, strerror(errno));
				return 0;
			}
#endif
			plp=plp->next;
		}
	}

	/* Nothing could be opened. */
	*current_filename = 0;

	if (noise_mode >= 2)
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: %s", name, strerror(errno));
	}

	return 0;
}

/* This closes files opened with open_file */
void close_file(FILE* fp)
{
	fclose(fp);
}

/* This is meant for skipping a few bytes in a file or fifo. */
void skip(FILE* fp, size_t len)
{
	fseek(fp, (long)len, SEEK_CUR);
}

/* This'll allocate memory or die. */
void* safe_malloc(size_t count)
{
	void* p;
	if ((p = malloc(count)))
		return p;
	else
		ctl->cmsg(CMSG_FATAL, VERB_NORMAL, "Sorry. Couldn't malloc %d bytes.", count);

	exit(10);
	return NULL;
}

/* This adds a directory to the path list */
void add_to_pathlist(const char* s)
{
	PathList* plp = (PathList*)safe_malloc(sizeof(PathList));
	strcpy((plp->path = (char*)safe_malloc(strlen(s) + 1)), s);
	plp->next = pathlist;
	pathlist = plp;
}

/* Free memory associated to path list */
void free_pathlist()
{
	PathList* plp = pathlist;
	while (plp)
	{
		if (plp->path)
		{
			free(plp->path);
			plp->path = NULL;
		}
		PathList* next_plp = plp->next;
		free(plp);
		plp = NULL;
		plp = next_plp;
	}
	pathlist = NULL;
}

};
