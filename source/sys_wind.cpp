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

// HEADER FILES ------------------------------------------------------------

#include <signal.h>
#include <fcntl.h>
#define ftime fucked_ftime
#include <io.h>
#undef ftime
#include <direct.h>
#include <conio.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include "winlocal.h"	
#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define R_OK	4

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static HANDLE			dir_handle;
static WIN32_FIND_DATA	dir_buf;
static bool				dir_already_got;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  Sys_FileOpenRead
//
//==========================================================================

int Sys_FileOpenRead(const VStr& filename)
{
	return open(*filename, O_RDONLY | O_BINARY);
}

//==========================================================================
//
//  Sys_FileOpenWrite
//
//==========================================================================

int Sys_FileOpenWrite(const VStr& filename)
{
	return open(*filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
}

//==========================================================================
//
//	Sys_FileRead
//
//==========================================================================

int Sys_FileRead(int handle, void* buf, int size)
{
	return read(handle, buf, size);
}

//==========================================================================
//
//	Sys_FileWrite
//
//==========================================================================

int Sys_FileWrite(int handle, const void* buf, int size)
{
	return write(handle, buf, size);
}

//==========================================================================
//
//	Sys_FileSize
//
//==========================================================================

int Sys_FileSize(int handle)
{
    return filelength(handle);
}

//==========================================================================
//
//	Sys_FileSeek
//
//==========================================================================

int Sys_FileSeek(int handle, int offset)
{
	return lseek(handle, offset, SEEK_SET);
}

//==========================================================================
//
//	Sys_FileClose
//
//==========================================================================

int Sys_FileClose(int handle)
{
	return close(handle);
}

//==========================================================================
//
//	Sys_FileExists
//
//==========================================================================

int Sys_FileExists(const VStr& filename)
{
    return !access(*filename, R_OK);
}

//==========================================================================
//
//	Sys_FileTime
//
//	Returns -1 if not present
//
//==========================================================================

int	Sys_FileTime(const VStr& path)
{
	struct	stat	buf;
	
	if (stat(*path, &buf) == -1)
		return -1;
	
	return buf.st_mtime;
}

//==========================================================================
//
//	Sys_CreateDirectory
//
//==========================================================================

int Sys_CreateDirectory(const VStr& path)
{
	return mkdir(*path);
}

//==========================================================================
//
//	Sys_OpenDir
//
//==========================================================================

int Sys_OpenDir(const VStr& dirname)
{
	dir_handle = FindFirstFile(va("%s/*.*", *dirname), &dir_buf);
	if (dir_handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	dir_already_got = true;
	return true;
}

//==========================================================================
//
//	Sys_ReadDir
//
//==========================================================================

VStr Sys_ReadDir()
{
	if (!dir_already_got)
	{
		if (FindNextFile(dir_handle, &dir_buf) != TRUE)
		{
			return VStr();
		}
	}
	dir_already_got = false;
	return dir_buf.cFileName;
}

//==========================================================================
//
//	Sys_CloseDir
//
//==========================================================================

void Sys_CloseDir()
{
    FindClose(dir_handle);
}

//==========================================================================
//
//	Sys_DirExists
//
//==========================================================================

bool Sys_DirExists(const VStr& path)
{
	struct stat s;
	
	if (stat(*path, &s) == -1)
		return false;
	
	return !!(s.st_mode & S_IFDIR);
}

//==========================================================================
//
//	Sys_Shutdown
//
//==========================================================================

void Sys_Shutdown()
{
}

//==========================================================================
//
// 	Sys_Quit
//
// 	Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
//==========================================================================

void Sys_Quit(const char*)
{
	// Shutdown system
	Host_Shutdown();

	// Exit
	exit(0);
}

//==========================================================================
//
// 	Sys_Error
//
//	Exits game and displays error message.
//
//==========================================================================

void Sys_Error(const char *error, ...)
{
	va_list argptr;
	char buf[1024];

	va_start(argptr,error);
	vsprintf(buf, error, argptr);
	va_end(argptr);

	throw VavoomError(buf);
}

//==========================================================================
//
// 	signal_handler
//
// 	Shuts down system, on error signal
//
//==========================================================================

void signal_handler(int s)
{
	signal(s, SIG_IGN);  // Ignore future instances of this signal.

	switch (s)
	{
	 case SIGINT:	throw VavoomError("Interrupted by User");
	 case SIGILL:	throw VavoomError("Illegal Instruction");
	 case SIGFPE:	throw VavoomError("Floating Point Exception");
	 case SIGSEGV:	throw VavoomError("Segmentation Violation");
	 case SIGTERM:	throw VavoomError("Software termination signal from kill");
	 case SIGBREAK:	throw VavoomError("Ctrl-Break sequence");
	 case SIGABRT:	throw VavoomError("Abnormal termination triggered by abort call");
     default:		throw VavoomError("Terminated by signal");
    }
}

//==========================================================================
//
//	Sys_Time
//
//==========================================================================

double Sys_Time()
{
	double t;
    struct timeb tstruct;
	static int	starttime;

	ftime(&tstruct);

	if (!starttime)
		starttime = tstruct.time;
	t = (tstruct.time - starttime) + tstruct.millitm * 0.001;
	
	return t;
}

//==========================================================================
//
//	Sys_ConsoleInput
//
//==========================================================================

char *Sys_ConsoleInput()
{
	static char	text[256];
	static int		len;
	int		c;

	// read a line out
	while (kbhit())
	{
		c = getch();
		putch(c);
		if (c == '\r')
		{
			text[len] = 0;
			putch('\n');
			len = 0;
			return text;
		}
		if (c == 8)
		{
			if (len)
			{
				putch(' ');
				putch(c);
				len--;
				text[len] = 0;
			}
			continue;
		}
		text[len] = c;
		len++;
		text[len] = 0;
		if (len == sizeof(text))
			len = 0;
	}

	return NULL;
}

//==========================================================================
//
//	main
//
// 	Main program
//
//==========================================================================

int main(int argc, char **argv)
{
	try
	{
		printf("Vavoom dedicated server "VERSION_TEXT"\n");

		GArgs.Init(argc, argv);

		//Install signal handler
		signal(SIGINT,  signal_handler);
		signal(SIGILL,  signal_handler);
		signal(SIGFPE,  signal_handler);
		signal(SIGSEGV, signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGBREAK,signal_handler);
		signal(SIGABRT, signal_handler);

		Host_Init();
		while (1)
		{
			Host_Frame();
		}
	}
	catch (VavoomError &e)
	{
		Host_Shutdown();

		dprintf("\n\nERROR: %s\n", e.message);
		fprintf(stderr, "%s\n", e.message);

		return 1;
	}
	catch (...)
	{
		Host_Shutdown();
		dprintf("\n\nExiting due to external exception\n");
		fprintf(stderr, "\nExiting due to external exception\n");
		throw;
	}
}
