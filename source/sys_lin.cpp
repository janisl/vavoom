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

// HEADER FILES ------------------------------------------------------------

#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <allegro.h>

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define O_BINARY	0

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

#ifdef __i386__
extern "C" {

void Sys_SetFPCW(void);

}
#endif

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
	return mkdir(path, 0777);
}

//==========================================================================
//
//	Sys_MakeCodeWriteable
//
//==========================================================================

void Sys_MakeCodeWriteable(unsigned long startaddr, unsigned long length)
{
	int r;
	unsigned long addr;
	int psize = getpagesize();

	addr = (startaddr & ~(psize-1)) - psize;

	r = mprotect((char*)addr, length + startaddr - addr + psize, 7);

	if (r < 0)
	{
   		Sys_Error("Protection change failed\n");
	}
}

//**************************************************************************
//**
//**	TIME
//**
//**************************************************************************

//==========================================================================
//
//  Sys_Time
//
//==========================================================================

double Sys_Time(void)
{
    timeval		tp;
    struct timezone	tzp;
    static int	secbase = 0;

    gettimeofday(&tp, &tzp);  

    if (!secbase)
    {
        secbase = tp.tv_sec;
        return tp.tv_usec / 1000000.0;
    }

    return (tp.tv_sec - secbase) + tp.tv_usec / 1000000.0;
}

//==========================================================================
//
//	Sys_Shutdown
//
//==========================================================================

void Sys_Shutdown(void)
{
	allegro_exit();
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

#define MAX_STACK_ADDR 40

// __builtin_return_address needs a constant, so this cannot be in a loop

#define handle_stack_address(X) \
	if (continue_stack_trace && ((unsigned long)__builtin_frame_address((X)) != 0L) && ((X) < MAX_STACK_ADDR)) \
	{ \
		stack_addr[(X)]= (unsigned long)__builtin_return_address((X)); \
		dprintf("stack %d %8p frame %d %8p\n", \
			(X), __builtin_return_address((X)), (X), __builtin_frame_address((X))); \
	} \
	else if (continue_stack_trace) \
	{ \
		continue_stack_trace = false; \
	}

static void stack_trace(void)
{
	FILE			*fff;
	int				i;
	static dword	stack_addr[MAX_STACK_ADDR];
	// can we still print entries on the calling stack or have we finished?
	static bool		continue_stack_trace = true;

	// clean the stack addresses if necessary
	for (i = 0; i < MAX_STACK_ADDR; i++)
	{
		stack_addr[i] = (unsigned long)0;
	}

	dprintf("STACK TRACE:\n\n");

	handle_stack_address(0);
	handle_stack_address(1);
	handle_stack_address(2);
	handle_stack_address(3);
	handle_stack_address(4);
	handle_stack_address(5);
	handle_stack_address(6);
	handle_stack_address(7);
	handle_stack_address(8);
	handle_stack_address(9);
	handle_stack_address(10);
	handle_stack_address(11);
	handle_stack_address(12);
	handle_stack_address(13);
	handle_stack_address(14);
	handle_stack_address(15);
	handle_stack_address(16);
	handle_stack_address(17);
	handle_stack_address(18);
	handle_stack_address(19);
	handle_stack_address(20);
	handle_stack_address(21);
	handle_stack_address(22);
	handle_stack_address(23);
	handle_stack_address(24);
	handle_stack_address(25);
	handle_stack_address(26);
	handle_stack_address(27);
	handle_stack_address(28);
	handle_stack_address(29);
	handle_stack_address(30);
	handle_stack_address(31);
	handle_stack_address(32);
	handle_stack_address(33);
	handle_stack_address(34);
	handle_stack_address(35);
	handle_stack_address(36);
	handle_stack_address(37);
	handle_stack_address(38);
	handle_stack_address(39);

	// Give a warning
//	fprintf(stderr, "You suddenly see a gruesome SOFTWARE BUG leap for your throat!\n");

	// Open the non-existing file
	fff = fopen("crash.txt", "w");

	// Invalid file
	if (fff)
	{
		// dump stack frame
		for (i = (MAX_STACK_ADDR - 1); i >= 0 ; i--)
		{
			fprintf(fff,"%x\n", stack_addr[i]);
		}
		fclose(fff);
	}
}

void Sys_Error(const char *error, ...)
{
    va_list		argptr;
	char		buf[1024];

	Host_Shutdown();
	stack_trace();

    // Message last, so it actually prints on the screen
    va_start (argptr,error);
    vsprintf(buf, error, argptr);
    va_end (argptr);

    printf("\n%s\n", buf);
	dprintf("\n\nERROR: %s\n", buf);

    exit(1);
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
	#define MINIMUM_HEAP_SIZE	0x800000		//  8 meg
	#define MAXIMUM_HEAP_SIZE	0x2000000		// 32 meg

	int			heap;
    void*		ptr;
	// Maximum allocated for zone heap (16meg default)
	int			maxzone = 0x1000000;
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

	heap = 0xa00000;

	do
	{
		heap -= 0x10000;                // leave 64k alone
		if (heap > maxzone)
			heap = maxzone;
		ptr = malloc(heap);
	} while (!ptr);

	dprintf("  0x%x (%f meg) allocated for zone, ", heap,
				(float)heap / (float)(1024 * 1024));
	dprintf("ZoneBase: 0x%X\n", (int)ptr);

	if (heap < 0x180000)
		Sys_Error("Insufficient memory!");

	*size = heap;
    return ptr;
}

//==========================================================================
//
//	Sys_ConsoleInput
//
//==========================================================================

char *Sys_ConsoleInput(void)
{
    static char text[256];
    int     len;
	fd_set	fdset;
    struct timeval timeout;

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
// 	signal_handler
//
// 	Shuts down system, on error signal
//
//==========================================================================

static void signal_handler(int s)
{
	signal(s,SIG_IGN);  // Ignore future instances of this signal.

	switch (s)
      {
       case SIGABRT:
            Sys_Error("Aborted");
            break;
       case SIGFPE:
            Sys_Error("Floating Point Exception");
            break;
       case SIGILL:
            Sys_Error("Illegal Instruction");
            break;
       case SIGSEGV:
            Sys_Error("Segmentation Violation");
            break;
       case SIGTERM:
            Sys_Error("Terminated");
            break;
       case SIGINT:
            Sys_Error("Interrupted by User");
            break;
       case SIGKILL:
            Sys_Error("Killed");
            break;
       case SIGQUIT:
            Sys_Error("Quited");
            break;
       default:
            Sys_Error("Terminated by signal");
      }
}

//==========================================================================
//
//	Floating point precision dummies for nonintel procesors
//
//==========================================================================

#ifndef __i386__

extern "C" {

void Sys_LowFPPrecision(void){}
void Sys_HighFPPrecision(void){}

}

#endif

//==========================================================================
//
//	main
//
// 	Main program
//
//==========================================================================

int main(int argc,char** argv)
{
	M_InitArgs(argc, argv);

#ifdef __i386__
	Sys_SetFPCW();
#endif

	allegro_init();

	//	Install signal handlers
   	signal(SIGABRT, signal_handler);
   	signal(SIGFPE,  signal_handler);
   	signal(SIGILL,  signal_handler);
   	signal(SIGSEGV, signal_handler);
   	signal(SIGTERM, signal_handler);
   	signal(SIGINT,  signal_handler);
   	signal(SIGKILL, signal_handler);
   	signal(SIGQUIT, signal_handler);

	Host_Init();

    while (1)
    {
		Host_Frame();
    }
}
END_OF_MAIN()	//	For Allegro

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/08/23 17:42:53  dj_jl
//	Directories now are created with full rights
//
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
