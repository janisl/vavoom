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
//**
//**	System driver for DOS, LINUX and UNIX dedicated servers.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

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

#define MINIMUM_HEAP_SIZE	0x800000		//  8 meg
#define MAXIMUM_HEAP_SIZE	0x2000000		// 32 meg

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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  Sys_FileOpenRead
//
//==========================================================================

int Sys_FileOpenRead(const char* filename)
{
	return open(filename, O_RDONLY | O_BINARY);
}

//==========================================================================
//
//  Sys_FileOpenWrite
//
//==========================================================================

int Sys_FileOpenWrite(const char* filename)
{
	return open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
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

int Sys_FileExists(const char* filename)
{
    return !access(filename, R_OK);
}

//==========================================================================
//
//	Sys_CreateDirectory
//
//==========================================================================

int Sys_CreateDirectory(const char* path)
{
	return mkdir(path, S_IWUSR);
}

//==========================================================================
//
//  Sys_Time
//
//==========================================================================

double Sys_Time(void)
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
//	Sys_ConsoleInput
//
//==========================================================================

char *Sys_ConsoleInput(void)
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
//	Sys_ZoneBase
//
// 	Called by startup code to get the ammount of memory to malloc for the
// zone management.
//
//==========================================================================

void* Sys_ZoneBase(int* size)
{
	int			heap;
    void*		ptr;
	// Maximum allocated for zone heap (13meg default)
	int			maxzone = 0xd00000;
	int			p;

	p = M_CheckParm("-maxzone");
	if (p && p < myargc - 1)
    {
		maxzone = (int)(atof(myargv[p + 1]) * 0x100000);
		if (maxzone < MINIMUM_HEAP_SIZE)
			maxzone = MINIMUM_HEAP_SIZE;
		if (maxzone > MAXIMUM_HEAP_SIZE)
			maxzone = MAXIMUM_HEAP_SIZE;
	}

#ifdef DJGPP
	// 	Get available memory size
	heap = _go32_dpmi_remaining_physical_memory();
#else
	heap = 0xa00000;
#endif

	do
	{
		heap -= 0x10000;                // leave 64k alone
		if (heap > maxzone)
			heap = maxzone;
		ptr = malloc(heap);
	} while (!ptr);

	dprintf("0x%x (%f meg) allocated for zone, Zone base 0x%X\n",
		heap, (float)heap / (float)(1024 * 1024), (int)ptr);

	if (heap < 0x180000)
		Sys_Error("Insufficient memory!");

	*size = heap;
    return ptr;
}

//==========================================================================
//
// 	Sys_Quit
//
// 	Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
//==========================================================================

void Sys_Quit(void)
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
    va_list		argptr;
	char		buf[1024];

	Host_Shutdown();

    // Message last, so it actually prints on the screen
    va_start(argptr, error);
    vsprintf(buf, error, argptr);
    va_end(argptr);

	dprintf("\n\nERROR: %s\n", buf);

    fprintf(stderr, "\n%s\n", buf);

	exit(1);
}

//==========================================================================
//
//	Sys_Shutdown
//
//==========================================================================

void Sys_Shutdown(void)
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
	switch (s)
    {
     case SIGABRT:	Sys_Error("Abnormal termination triggered by abort call");
	 case SIGFPE:	Sys_Error("Floating Point Exception");
	 case SIGILL:	Sys_Error("Illegal Instruction");
	 case SIGINT:	Sys_Error("Interrupted by User");
	 case SIGSEGV:	Sys_Error("Segmentation Violation");
     case SIGTERM:	Sys_Error("Software termination signal from kill");
#ifdef SIGKILL
	 case SIGKILL:	Sys_Error("Killed");
#endif
#ifdef SIGQUIT
	 case SIGQUIT:	Sys_Error("Quited");
#endif
#ifdef SIGNOFP
	 case SIGNOFP:	Sys_Error("VAVOOM requires a floating-point processor");
#endif
     default:		Sys_Error("Terminated by signal");
    }
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
	printf("Vavoom dedicated server "VERSION_TEXT"\n");

	M_InitArgs(argc, argv);

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

	//	Initialize
	Host_Init();

	//	Play game
    while (1)
    {
		Host_Frame();
    }
}

