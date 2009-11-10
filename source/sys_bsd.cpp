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
//**	System driver for DOS, LINUX and UNIX dedicated servers.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "gamedefs.h"
#include "svnrev.h"

#ifdef DJGPP
#include <dpmi.h>
#endif

#ifdef __sun__
#include <sys/filio.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

// MACROS ------------------------------------------------------------------

#ifndef O_BINARY
#define O_BINARY	0
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#ifdef __linux__
jmp_buf __Context::Env;
const char* __Context::ErrToThrow;
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static DIR *current_dir;

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
    struct stat		fileinfo;
    
    if (fstat(handle, &fileinfo) == -1)
    {
		Sys_Error("Error fstating");
    }
    return fileinfo.st_size;
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
	return mkdir(*path, 0777);
}

//==========================================================================
//
//	Sys_OpenDir
//
//==========================================================================

int Sys_OpenDir(const VStr& path)
{
	current_dir = opendir(*path);
	return current_dir != NULL;
}

//==========================================================================
//
//	Sys_ReadDir
//
//==========================================================================

VStr Sys_ReadDir()
{
	struct dirent *de = readdir(current_dir);
	if (de)
	{
		return de->d_name;
	}
	return VStr();
}

//==========================================================================
//
//	Sys_CloseDir
//
//==========================================================================

void Sys_CloseDir()
{
	closedir(current_dir);
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

	return !!S_ISDIR(s.st_mode);
}

//==========================================================================
//
//  Sys_Time
//
//==========================================================================

double Sys_Time()
{
	struct timeval		tp;
	struct timezone		tzp;
	static int			secbase = 0;

	gettimeofday(&tp, &tzp);  

	if (!secbase)
	{
		secbase = tp.tv_sec;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec / 1000000.0;
}

//==========================================================================
//
//	Sys_Sleep
//
//==========================================================================

void Sys_Sleep()
{
	usleep(1);
}

//==========================================================================
//
//	Sys_ConsoleInput
//
//==========================================================================

char *Sys_ConsoleInput()
{
	static char		text[256];
	int				len;
	fd_set			fdset;
	struct timeval	timeout;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset); // stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if (select(1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;

	len = read(0, text, sizeof(text));
	if (len < 1)
		return NULL;
	text[len-1] = 0;    // rip off the /n and terminate

	return text;
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

	va_start(argptr, error);
	vsprintf(buf, error, argptr);
	va_end(argptr);

	throw VavoomError(buf);
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
// 	signal_handler
//
// 	Shuts down system, on error signal
//
//==========================================================================

static void signal_handler(int s)
{
	// Ignore future instances of this signal.
	signal(s, SIG_IGN);

	//	Exit with error message
#ifdef __linux__
	switch (s)
	{
	case SIGABRT:
		__Context::ErrToThrow = "Aborted";
		break;
	case SIGFPE:
		__Context::ErrToThrow = "Floating Point Exception";
		break;
	case SIGILL:
		__Context::ErrToThrow = "Illegal Instruction";
		break;
	case SIGSEGV:
		__Context::ErrToThrow = "Segmentation Violation";
		break;
	case SIGTERM:
		__Context::ErrToThrow = "Terminated";
		break;
	case SIGINT:
		__Context::ErrToThrow = "Interrupted by User";
		break;
	case SIGKILL:
		__Context::ErrToThrow = "Killed";
		break;
	case SIGQUIT:
		__Context::ErrToThrow = "Quited";
		break;
	default:
		__Context::ErrToThrow = "Terminated by signal";
	}
	longjmp(__Context::Env, 1);
#else
	switch (s)
	{
	 case SIGABRT:	throw VavoomError("Abnormal termination triggered by abort call");
	 case SIGFPE:	throw VavoomError("Floating Point Exception");
	 case SIGILL:	throw VavoomError("Illegal Instruction");
	 case SIGINT:	throw VavoomError("Interrupted by User");
	 case SIGSEGV:	throw VavoomError("Segmentation Violation");
	 case SIGTERM:	throw VavoomError("Software termination signal from kill");
#ifdef SIGKILL
	 case SIGKILL:	throw VavoomError("Killed");
#endif
#ifdef SIGQUIT
	 case SIGQUIT:	throw VavoomError("Quited");
#endif
#ifdef SIGNOFP
	 case SIGNOFP:	throw VavoomError("VAVOOM requires a floating-point processor");
#endif
     default:		throw VavoomError("Terminated by signal");
	}
#endif
}

//==========================================================================
//
//	main
//
// 	Main program
//
//==========================================================================

int main(int argc, char** argv)
{
	try
	{
		printf("Vavoom dedicated server "VERSION_TEXT" (r" SVN_REVISION_STRING ")\n");

		GArgs.Init(argc, argv);

		//	Install signal handlers
		signal(SIGABRT, signal_handler);
		signal(SIGFPE,  signal_handler);
		signal(SIGILL,  signal_handler);
		signal(SIGSEGV, signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGINT,  signal_handler);
#ifdef SIGKILL
		signal(SIGKILL, signal_handler);
#endif
#ifdef SIGQUIT
		signal(SIGQUIT, signal_handler);
#endif
#ifdef SIGNOFP
		signal(SIGNOFP, signal_handler);
#endif

		//	Initialise
		Host_Init();

		//	Play game
		while (1)
		{
			Host_Frame();
		}
	}
	catch (VavoomError &e)
	{
		Host_Shutdown();

		dprintf("\n\nERROR: %s\n", e.message);

		fprintf(stderr, "\n%s\n", e.message);

		exit(1);
	}
	catch (...)
	{
		Host_Shutdown();
		dprintf("\n\nExiting due to external exception\n");
		fprintf(stderr, "\nExiting due to external exception\n");
		throw;
	}
}
