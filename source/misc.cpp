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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define MAXARGVS        100

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

#ifndef DJGPP
static void M_FindResponseFile(void);
#endif

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int 	myargc;
char**	myargv;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char		va_buffer[1024];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	M_InitArgs
//
//==========================================================================

void M_InitArgs(int argc, char **argv)
{
	//	Save args
	myargc = argc;
    myargv = argv;

#ifndef DJGPP
	M_FindResponseFile();
#endif
}

//==========================================================================
//
//	M_FindResponseFile
//
// 	Find a Response File. We don't do this in DJGPP because it does this
// in startup code.
//
//==========================================================================

#ifndef DJGPP
static void M_FindResponseFile(void)
{
    int         i;
	
    for (i = 1;i < myargc;i++)
	  if (myargv[i][0] == '@')
	  {
	    FILE *  handle;
	    int     size;
	    int     k;
	    int     index;
	    int    	indexinfile;
	    char    *infile;
	    char    *file;
	    char    *moreargs[20];
	    char    *firstargv;
			
	    // READ THE RESPONSE FILE INTO MEMORY
	    handle = fopen(&myargv[i][1], "rb");
	    if (!handle)
	    {
			printf("\nNo such response file %s!", &myargv[i][1]);
			exit(1);
	    }
	    dprintf("Found response file %s!\n", &myargv[i][1]);
	    fseek(handle, 0, SEEK_END);
	    size = ftell(handle);
	    fseek(handle, 0, SEEK_SET);
	    file = (char*)malloc(size);
	    fread(file, size, 1, handle);
	    fclose(handle);
			
	    // KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG
	    for (index = 0,k = i+1; k < myargc; k++)
			moreargs[index++] = myargv[k];
			
	    firstargv = myargv[0];
	    myargv = (char**)malloc(sizeof(char *) * MAXARGVS);
	    memset(myargv, 0, sizeof(char *) * MAXARGVS);
	    myargv[0] = firstargv;
			
	    infile = file;
	    indexinfile = k = 0;
	    indexinfile++;  // SKIP PAST ARGV[0] (KEEP IT)
	    do
	    {
			myargv[indexinfile++] = infile+k;
			while(k < size &&
		      ((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
		    k++;
			*(infile+k) = 0;
			while(k < size &&
		      ((*(infile+k)<= ' ') || (*(infile+k)>'z')))
		    k++;
	    } while(k < size);
			
	    for (k = 0;k < index;k++)
			myargv[indexinfile++] = moreargs[k];
	    myargc = indexinfile;
	
	    // DISPLAY ARGS
		if (M_CheckParm("-debug"))
		{
		    dprintf("%d command-line args:\n",myargc);
		    for (k=1;k<myargc;k++)
				dprintf("%s\n",myargv[k]);
		}
	    break;
	  }
}
#endif

//==========================================================================
//
//  M_CheckParm
//
//  Checks for the given parameter in the program's command line arguments.
//  Returns the argument number (1 to argc-1) or 0 if not present
//
//==========================================================================

int M_CheckParm(const char *check)
{
    int		i;

    for (i = 1;i<myargc;i++)
    {
		if (!stricmp(check, myargv[i]) )
	    	return i;
    }

    return 0;
}

//==========================================================================
//
//	Endianess handling, swapping 16bit and 32bit.
//	WAD files are stored little endian.
//
//==========================================================================

short (*LittleShort)(short);
short (*BigShort)(short);
int (*LittleLong)(int);
int (*BigLong)(int);
float (*LittleFloat)(float);
float (*BigFloat)(float);

//	Swaping
static short ShortSwap(short x)
{
	return  ((word)x >> 8) |
			((word)x << 8);
}
static short ShortNoSwap(short x)
{
	return x;
}
static int LongSwap(int x)
{
	return 	((dword)x >> 24) |
			(((dword)x >> 8) & 0xff00) |
			(((dword)x << 8) & 0xff0000) |
			((dword)x << 24);
}
static int LongNoSwap(int x)
{
	return x;
}
static float FloatSwap(float x)
{
	union { float f; long l; } a;
	a.f = x;
	a.l = LongSwap(a.l);
	return a.f;
}
static float FloatNoSwap(float x)
{
	return x;
}

void M_InitByteOrder(void)
{
	byte    swaptest[2] = {1, 0};

	// set the byte swapping variables in a portable manner
	if (*(short *)swaptest == 1)
	{
		BigShort = ShortSwap;
		LittleShort = ShortNoSwap;
		BigLong = LongSwap;
		LittleLong = LongNoSwap;
		BigFloat = FloatSwap;
		LittleFloat = FloatNoSwap;
	}
	else
	{
		BigShort = ShortNoSwap;
		LittleShort = ShortSwap;
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
		BigFloat = FloatNoSwap;
		LittleFloat = FloatSwap;
	}
}

//==========================================================================
//
//	M_ReadFile
//
//==========================================================================

int M_ReadFile(const char* name, byte** buffer)
{
    int			handle;
    int			count;
    int			length;
    byte		*buf;
	
    handle = Sys_FileOpenRead(name);
    if (handle == -1)
	{
		Sys_Error("Couldn't open file %s", name);
	}
    length = Sys_FileSize(handle);
    buf = (byte*)Z_Malloc(length + 1, PU_STATIC, NULL);
    count = Sys_FileRead(handle, buf, length);
	buf[length] = 0;
    Sys_FileClose(handle);
	
    if (count < length)
	{
		Sys_Error("Couldn't read file %s", name);
	}
		
    *buffer = buf;
    return length;
}

//==========================================================================
//
//	M_WriteFile
//
//==========================================================================

boolean M_WriteFile(const char* name, const void* source, int length)
{
    int		handle;
    int		count;
	
    handle = Sys_FileOpenWrite(name);

    if (handle == -1)
	{
		return false;
	}

    count = Sys_FileWrite(handle, source, length);
    Sys_FileClose(handle);
	
    if (count < length)
	{
		return false;
	}
		
    return true;
}

//==========================================================================
//
//  M_ValidEpisodeMap
//
//==========================================================================

boolean M_ValidEpisodeMap(int episode, int map)
{
	if (episode < 1 || map < 1 || map > 9)
	{
		return false;
	}
	if (Cvar_Value("shareware"))
	{ // Shareware version checks
		if (episode != 1)
		{
			return false;
		}
	}
	else if (ExtendedWAD)
	{ // Extended version checks
		if (episode == 6)
		{
			if (map > 3)
			{
				return false;
			}
		}
		else if (episode > 5)
		{
			return false;
		}
	}
	else
	{ // Registered version checks
		if (episode == 4)
		{
			if (map != 1)
			{
				return false;
			}
		}
		else if (episode > 3)
		{
			return false;
		}
	}
	return true;
}

//==========================================================================
//
//  superatoi
//
//==========================================================================

int superatoi(const char *s)
{
	int n=0, r=10, x, mul=1;
	const char *c=s;

	for (; *c; c++)
	{
		x = (*c & 223) - 16;

		if (x == -3)
		{
			mul = -mul;
		}
		else if (x == 72 && r == 10)
		{
			n -= (r=n);
			if (!r) r=16;
			if (r<2 || r>36) return -1;
		}
		else
		{
			if (x>10) x-=39;
			if (x >= r) return -1;
			n = (n*r) + x;
		}
	}
	return(mul*n);
}

//==========================================================================
//
//	va
//
//	Very usefull function from QUAKE
//	Does a varargs printf into a temp buffer, so I don't need to have
// varargs versions of all text functions.
//	FIXME: make this buffer size safe someday
//
//==========================================================================

char *va(const char *text, ...)
{
	va_list		args;

	va_start(args, text);
	vsprintf(va_buffer, text, args);
	va_end(args);

	return va_buffer;
}

//==========================================================================
//
//	PassFloat
//
//==========================================================================

int PassFloat(float f)
{
	union
    {
    	float	f;
        int		i;
	} v;

    v.f = f;
    return v.i;
}

#if 0
/*

	All of Quake's data access is through a hierchal file system, but the
contents of the file system can be transparently merged from several sources.

	The "base directory" is the path to the directory holding the quake.exe
and all game directories.  The sys_* files pass this to host_init in
quakeparms_t->basedir.  This can be overridden with the "-basedir" command
line parm to allow code debugging in a different directory.  The base
directory is only used during filesystem initialization.

	The "game directory" is the first tree on the search path and directory
that all generated files (savegames, screenshots, demos, config files) will
be saved to.  This can be overridden with the "-game" command line parameter.
The game directory can never be changed while quake is executing.  This is a
precacution against having a malicious server instruct clients to write files
over areas they shouldn't.

	The "cache directory" is only used during development to save network
bandwidth, especially over ISDN / T1 lines.  If there is a cache directory
specified, when a file is found by the normal search path, it will be
mirrored into the cache directory, then opened there.

FIXME:
	The file "parms.txt" will be read out of the game directory and appended
to the current command line arguments to allow different games to initialize
startup parms differently.  This could be used to add a "-sspeed 22050" for
the high quality sound edition.  Because they are added at the end, they will
not override an explicit setting on the original command line.
	
*/

//============================================================================


/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/

typedef struct searchpath_s
{
	char    filename[MAX_OSPATH];
	pack_t  *pack;          // only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

searchpath_t    *com_searchpaths;

/*
============
COM_WriteFile

The filename will be prefixed by the current game directory
============
*/
void COM_WriteFile (char *filename, void *data, int len)
{
	int             handle;
	char    name[MAX_OSPATH];
	
	sprintf (name, "%s/%s", com_gamedir, filename);

	handle = Sys_FileOpenWrite (name);
	if (handle == -1)
	{
		Sys_Printf ("COM_WriteFile: failed on %s\n", name);
		return;
	}
	
	Sys_Printf ("COM_WriteFile: %s\n", name);
	Sys_FileWrite (handle, data, len);
	Sys_FileClose (handle);
}


/*
============
COM_CreatePath

Only used for CopyFile
============
*/
void    COM_CreatePath (char *path)
{
	char    *ofs;
	
	for (ofs = path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{       // create the directory
			*ofs = 0;
			Sys_mkdir (path);
			*ofs = '/';
		}
	}
}


/*
===========
COM_CopyFile

Copies a file over from the net to the local cache, creating any directories
needed.  This is for the convenience of developers using ISDN from home.
===========
*/
void COM_CopyFile (char *netpath, char *cachepath)
{
	int             in, out;
	int             remaining, count;
	char    buf[4096];
	
	remaining = Sys_FileOpenRead (netpath, &in);            
	COM_CreatePath (cachepath);     // create directories up to the cache file
	out = Sys_FileOpenWrite (cachepath);
	
	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);
		Sys_FileRead (in, buf, count);
		Sys_FileWrite (out, buf, count);
		remaining -= count;
	}

	Sys_FileClose (in);
	Sys_FileClose (out);    
}

/*
===========
COM_FindFile

Finds the file in the search path.
Sets com_filesize and one of handle or file
===========
*/
int COM_FindFile (char *filename, int *handle, FILE **file)
{
	searchpath_t    *search;
	char            netpath[MAX_OSPATH];
	char            cachepath[MAX_OSPATH];
	pack_t          *pak;
	int                     i;
	int                     findtime, cachetime;

	if (file && handle)
		Sys_Error ("COM_FindFile: both handle and file set");
	if (!file && !handle)
		Sys_Error ("COM_FindFile: neither handle or file set");
		
//
// search through the path, one element at a time
//
	search = com_searchpaths;
	if (proghack)
	{	// gross hack to use quake 1 progs with quake 2 maps
		if (!strcmp(filename, "progs.dat"))
			search = search->next;
	}

	for ( ; search ; search = search->next)
	{
	// is the element a pak file?
		if (search->pack)
		{
		// look through all the pak file elements
			pak = search->pack;
			for (i=0 ; i<pak->numfiles ; i++)
				if (!strcmp (pak->files[i].name, filename))
				{       // found it!
					Sys_Printf ("PackFile: %s : %s\n",pak->filename, filename);
					if (handle)
					{
						*handle = pak->handle;
						Sys_FileSeek (pak->handle, pak->files[i].filepos);
					}
					else
					{       // open a new file on the pakfile
						*file = fopen (pak->filename, "rb");
						if (*file)
							fseek (*file, pak->files[i].filepos, SEEK_SET);
					}
					com_filesize = pak->files[i].filelen;
					return com_filesize;
				}
		}
		else
		{               
	// check a file in the directory tree
			if (!static_registered)
			{       // if not a registered version, don't ever go beyond base
				if ( strchr (filename, '/') || strchr (filename,'\\'))
					continue;
			}
			
			sprintf (netpath, "%s/%s",search->filename, filename);
			
			findtime = Sys_FileTime (netpath);
			if (findtime == -1)
				continue;
				
		// see if the file needs to be updated in the cache
			if (!com_cachedir[0])
				strcpy (cachepath, netpath);
			else
			{	
#if defined(_WIN32)
				if ((strlen(netpath) < 2) || (netpath[1] != ':'))
					sprintf (cachepath,"%s%s", com_cachedir, netpath);
				else
					sprintf (cachepath,"%s%s", com_cachedir, netpath+2);
#else
				sprintf (cachepath,"%s%s", com_cachedir, netpath);
#endif

				cachetime = Sys_FileTime (cachepath);
			
				if (cachetime < findtime)
					COM_CopyFile (netpath, cachepath);
				strcpy (netpath, cachepath);
			}	

			Sys_Printf ("FindFile: %s\n",netpath);
			com_filesize = Sys_FileOpenRead (netpath, &i);
			if (handle)
				*handle = i;
			else
			{
				Sys_FileClose (i);
				*file = fopen (netpath, "rb");
			}
			return com_filesize;
		}
		
	}
	
	Sys_Printf ("FindFile: can't find %s\n", filename);
	
	if (handle)
		*handle = -1;
	else
		*file = NULL;
	com_filesize = -1;
	return -1;
}


/*
===========
COM_OpenFile

filename never has a leading slash, but may contain directory walks
returns a handle and a length
it may actually be inside a pak file
===========
*/
int COM_OpenFile (char *filename, int *handle)
{
	return COM_FindFile (filename, handle, NULL);
}

/*
===========
COM_FOpenFile

If the requested file is inside a packfile, a new FILE * will be opened
into the file.
===========
*/
int COM_FOpenFile (char *filename, FILE **file)
{
	return COM_FindFile (filename, NULL, file);
}

/*
============
COM_CloseFile

If it is a pak file handle, don't really close it
============
*/
void COM_CloseFile (int h)
{
	searchpath_t    *s;
	
	for (s = com_searchpaths ; s ; s=s->next)
		if (s->pack && s->pack->handle == h)
			return;
			
	Sys_FileClose (h);
}


/*
============
COM_LoadFile

Filename are reletive to the quake directory.
Allways appends a 0 byte.
============
*/
cache_user_t *loadcache;
byte    *loadbuf;
int             loadsize;
byte *COM_LoadFile (char *path, int usehunk)
{
	int             h;
	byte    *buf;
	char    base[32];
	int             len;

	buf = NULL;     // quiet compiler warning

// look for it in the filesystem or pack files
	len = COM_OpenFile (path, &h);
	if (h == -1)
		return NULL;
	
// extract the filename base name for hunk tag
	COM_FileBase (path, base);
	
	if (usehunk == 1)
		buf = Hunk_AllocName (len+1, base);
	else if (usehunk == 2)
		buf = Hunk_TempAlloc (len+1);
	else if (usehunk == 0)
		buf = Z_Malloc (len+1);
	else if (usehunk == 3)
		buf = Cache_Alloc (loadcache, len+1, base);
	else if (usehunk == 4)
	{
		if (len+1 > loadsize)
			buf = Hunk_TempAlloc (len+1);
		else
			buf = loadbuf;
	}
	else
		Sys_Error ("COM_LoadFile: bad usehunk");

	if (!buf)
		Sys_Error ("COM_LoadFile: not enough space for %s", path);
		
	((byte *)buf)[len] = 0;

	Draw_BeginDisc ();
	Sys_FileRead (h, buf, len);                     
	COM_CloseFile (h);
	Draw_EndDisc ();

	return buf;
}

byte *COM_LoadHunkFile (char *path)
{
	return COM_LoadFile (path, 1);
}

byte *COM_LoadTempFile (char *path)
{
	return COM_LoadFile (path, 2);
}

void COM_LoadCacheFile (char *path, struct cache_user_s *cu)
{
	loadcache = cu;
	COM_LoadFile (path, 3);
}

// uses temp hunk if larger than bufsize
byte *COM_LoadStackFile (char *path, void *buffer, int bufsize)
{
	byte    *buf;
	
	loadbuf = (byte *)buffer;
	loadsize = bufsize;
	buf = COM_LoadFile (path, 4);
	
	return buf;
}

/*
================
COM_AddGameDirectory

Sets com_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ... 
================
*/
void COM_AddGameDirectory (char *dir)
{
	int                             i;
	searchpath_t    *search;
	pack_t                  *pak;
	char                    pakfile[MAX_OSPATH];

	strcpy (com_gamedir, dir);

//
// add the directory to the search path
//
	search = Hunk_Alloc (sizeof(searchpath_t));
	strcpy (search->filename, dir);
	search->next = com_searchpaths;
	com_searchpaths = search;

//
// add any pak files in the format pak0.pak pak1.pak, ...
//
	for (i=0 ; ; i++)
	{
		sprintf (pakfile, "%s/pak%i.pak", dir, i);
		pak = COM_LoadPackFile (pakfile);
		if (!pak)
			break;
		search = Hunk_Alloc (sizeof(searchpath_t));
		search->pack = pak;
		search->next = com_searchpaths;
		com_searchpaths = search;               
	}

//
// add the contents of the parms.txt file to the end of the command line
//

}

/*
================
COM_InitFilesystem
================
*/
void COM_InitFilesystem (void)
{
	int             i, j;
	char    basedir[MAX_OSPATH];
	searchpath_t    *search;

//
// -basedir <path>
// Overrides the system supplied base directory (under GAMENAME)
//
	i = COM_CheckParm ("-basedir");
	if (i && i < com_argc-1)
		strcpy (basedir, com_argv[i+1]);
	else
		strcpy (basedir, host_parms.basedir);

	j = strlen (basedir);

	if (j > 0)
	{
		if ((basedir[j-1] == '\\') || (basedir[j-1] == '/'))
			basedir[j-1] = 0;
	}

//
// -cachedir <path>
// Overrides the system supplied cache directory (NULL or /qcache)
// -cachedir - will disable caching.
//
	i = COM_CheckParm ("-cachedir");
	if (i && i < com_argc-1)
	{
		if (com_argv[i+1][0] == '-')
			com_cachedir[0] = 0;
		else
			strcpy (com_cachedir, com_argv[i+1]);
	}
	else if (host_parms.cachedir)
		strcpy (com_cachedir, host_parms.cachedir);
	else
		com_cachedir[0] = 0;

//
// start up with GAMENAME by default (id1)
//
	COM_AddGameDirectory (va("%s/"GAMENAME, basedir) );

	if (COM_CheckParm ("-rogue"))
		COM_AddGameDirectory (va("%s/rogue", basedir) );
	if (COM_CheckParm ("-hipnotic"))
		COM_AddGameDirectory (va("%s/hipnotic", basedir) );

//
// -game <gamedir>
// Adds basedir/gamedir as an override game
//
	i = COM_CheckParm ("-game");
	if (i && i < com_argc-1)
	{
		com_modified = true;
		COM_AddGameDirectory (va("%s/%s", basedir, com_argv[i+1]));
	}

//
// -path <dir or packfile> [<dir or packfile>] ...
// Fully specifies the exact serach path, overriding the generated one
//
	i = COM_CheckParm ("-path");
	if (i)
	{
		com_modified = true;
		com_searchpaths = NULL;
		while (++i < com_argc)
		{
			if (!com_argv[i] || com_argv[i][0] == '+' || com_argv[i][0] == '-')
				break;
			
			search = Hunk_Alloc (sizeof(searchpath_t));
			if ( !strcmp(COM_FileExtension(com_argv[i]), "pak") )
			{
				search->pack = COM_LoadPackFile (com_argv[i]);
				if (!search->pack)
					Sys_Error ("Couldn't load packfile: %s", com_argv[i]);
			}
			else
				strcpy (search->filename, com_argv[i]);
			search->next = com_searchpaths;
			com_searchpaths = search;
		}
	}

	if (COM_CheckParm ("-proghack"))
		proghack = true;
}

#endif

