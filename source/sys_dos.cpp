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

#include <fcntl.h>
#include <unistd.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/exceptn.h>
#include <sys/farptr.h>
#include <go32.h>
#include <signal.h>
#include <conio.h>
#include <dirent.h>
#include <allegro.h>

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

//#define PROGS_PROFILE

#define MINIMUM_HEAP_SIZE	0x800000		//   8 meg
#define MAXIMUM_HEAP_SIZE	0x8000000		// 128 meg

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern "C" {

void	__djgpp_traceback_exit(int _sig) __attribute__((noreturn));

void Sys_SetFPCW();
void Sys_PushFPCW_SetHigh();
void Sys_PopFPCW();

}

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

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
//	Sys_MakeCodeWriteable
//
//==========================================================================

void Sys_MakeCodeWriteable(unsigned long, unsigned long)
{
	// it's always writeable
}

//**************************************************************************
//**
//**	TIME
//**
//**************************************************************************

static double	curtime = 0.0;
static double	oldtime = 0.0;

//==========================================================================
//
//  Sys_InitTime
//
//==========================================================================

static void Sys_InitTime()
{
    int			r;
    dword		t;
    dword		tick;

	//
    // set system timer to mode 2 for the Sys_Time() function
	//
	outportb(0x43, 0x34);
	outportb(0x40, 0);
	outportb(0x40, 0);

	//
	//	Init old time
	//
	t = _farpeekw(_dos_ds, 0x46c) * 65536;

    outportb(0x43, 0); // latch time
    r = inportb(0x40);
    r |= inportb(0x40) << 8;
    r = (r - 1) & 0xffff;

    tick = _farpeekw(_dos_ds, 0x46c) * 65536;
    if ((tick != t) && (r & 0x8000))
		t = tick;

	oldtime = (double) (t + (65536 - r)) / 1193200.0;

	//
	//	Set start time
	//
	const char* p = GArgs.CheckValue("-starttime");

	if (p)
	{
		curtime = (double)atof(p);
	}
	else
	{
		curtime = 0.0;
	}
}

//==========================================================================
//
//  Sys_Time
//
//==========================================================================

double Sys_Time()
{
    int				r;
    unsigned		t, tick;
	double			ft, time;

	Sys_PushFPCW_SetHigh();

	t = _farpeekw(_dos_ds, 0x46c) * 65536;

    outportb(0x43, 0); // latch time
    r = inportb(0x40);
    r |= inportb(0x40) << 8;
    r = (r - 1) & 0xffff;

    tick = _farpeekw(_dos_ds, 0x46c) * 65536;
    if ((tick != t) && (r & 0x8000))
		t = tick;

	ft = (double) (t + (65536 - r)) / 1193200.0;
	time = ft - oldtime;
	oldtime = ft;

	if (time < 0)
	{
		if (time > -3000.0)
			time = 0.0;
		else
			time += 3600.0;
	}

	curtime += time;

	Sys_PopFPCW();

    return curtime;
}

//==========================================================================
//
//	Sys_ConsoleInput
//
//==========================================================================

char *Sys_ConsoleInput()
{
	static char	text[256];
	static int	len = 0;
	char		ch;

	if (!kbhit())
		return NULL;

	ch = getche();

	switch (ch)
	{
		case '\r':
			putch('\n');
			if (len)
			{
				text[len] = 0;
				len = 0;
				return text;
			}
			break;

		case '\b':
			putch(' ');
			if (len)
			{
				len--;
				putch('\b');
			}
			break;

		default:
			text[len] = ch;
			len = (len + 1) & 0xff;
			break;
	}

	return NULL;
}

//==========================================================================
//
//	Sys_Shutdown
//
//==========================================================================

void Sys_Shutdown()
{
	allegro_exit();

#ifdef PROGS_PROFILE
	svpr.DumpProfile();
	clpr.DumpProfile();
#endif
}

//==========================================================================
//
// 	Sys_Quit
//
// 	Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
//==========================================================================

void Sys_Quit(const char* EndText)
{
	// Shutdown system
	Host_Shutdown();

	// Throw the end text at the screen
	if (EndText)
	{
		puttext(1, 1, 80, 25, EndText);
		gotoxy(1, 24);
	}

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

	va_start(argptr, error);
	vsprintf(buf, error, argptr);
	va_end(argptr);

	throw VavoomError(buf);
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
	// Maximum allocated for zone heap (64meg default)
	int			maxzone = 0x4000000;

	const char* p = GArgs.CheckValue("-mem");
	if (p)
	{
		heap = (int)(atof(p) * 0x100000);
		ptr = malloc(heap);
		if (!ptr)
		{
			Sys_Error("Couldn't alloc %d bytes", heap);
		}
	}
	else
	{
		p = GArgs.CheckValue("-maxzone");
		if (p)
	    {
			maxzone = (int)(atof(p) * 0x100000);
			if (maxzone < MINIMUM_HEAP_SIZE)
				maxzone = MINIMUM_HEAP_SIZE;
			if (maxzone > MAXIMUM_HEAP_SIZE)
				maxzone = MAXIMUM_HEAP_SIZE;
		}

		// 	Get available memory size
		heap = _go32_dpmi_remaining_physical_memory();

		do
		{
			heap -= 0x10000;                // leave 64k alone
			if (heap > maxzone)
				heap = maxzone;
			ptr = malloc(heap);
		} while (!ptr);
	}

	dprintf("0x%x (%f meg) allocated for zone, Zone base 0x%X\n",
		heap, (float)heap / (float)(1024 * 1024), (int)ptr);

	if (heap < 0x180000)
		Sys_Error("Insufficient DPMI memory!");

	*size = heap;
    return ptr;
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
#ifdef DEVELOPER
	// Ignore future instances of this signal.
	signal(s, SIG_IGN);

	//	Print signal type in debug file
	switch (s)
	{
	case SIGABRT:
		dprintf("Aborted\n");
		break;
	case SIGFPE:
		dprintf("Floating Point Exception\n");
		break;
	case SIGILL:
		dprintf("Illegal Instruction\n");
		break;
	case SIGINT:
		dprintf("Interrupted by User\n");
		break;
	case SIGSEGV:
		dprintf("Segmentation Violation\n");
		break;
	case SIGTERM:
		dprintf("Terminated\n");
		break;
	case SIGKILL:
		dprintf("Killed\n");
		break;
	case SIGQUIT:
		dprintf("Quited\n");
		break;
	case SIGNOFP:
		dprintf("VAVOOM requires a floating-point processor\n");
		break;
	default:
		dprintf("Terminated by signal\n");
	}

	//	Shutdown game
	Host_Shutdown();

	//	Set default signal handlers
	__djgpp_exception_toggle();

	//	Exit with default signal handler (with traceback)
	raise(s);

	//	In a case if default signal handler doesn't exit from program
	exit(1);
#else
	// Ignore future instances of this signal.
	signal(s, SIG_IGN);

	//	Exit with error message
	switch (s)
	{
	case SIGABRT:
		throw VavoomError("Aborted");
	case SIGFPE:
		throw VavoomError("Floating Point Exception");
	case SIGILL:
		throw VavoomError("Illegal Instruction");
	case SIGINT:
		throw VavoomError("Interrupted by User");
	case SIGSEGV:
		throw VavoomError("Segmentation Violation");
	case SIGTERM:
		throw VavoomError("Terminated");
	case SIGKILL:
		throw VavoomError("Killed");
	case SIGQUIT:
		throw VavoomError("Quited");
	case SIGNOFP:
		throw VavoomError("VAVOOM requires a floating-point processor");
	default:
		throw VavoomError("Terminated by signal");
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

extern "C" void PR_Profile2();

int main(int argc,char** argv)
{
	try
	{
		GArgs.Init(argc, argv);

		//	Startup Allegro
    	allegro_init();

#ifdef PROGS_PROFILE
		install_timer();
		install_int_ex(PR_Profile2, 1);
#endif

		//	Install signal handlers overriding Allegro handlers
   		signal(SIGABRT, signal_handler);
	   	signal(SIGFPE,  signal_handler);
   		signal(SIGILL,  signal_handler);
	   	signal(SIGSEGV, signal_handler);
   		signal(SIGTERM, signal_handler);
	   	signal(SIGINT,  signal_handler);
   		signal(SIGKILL, signal_handler);
	   	signal(SIGQUIT, signal_handler);
   		signal(SIGNOFP, signal_handler);

		Sys_SetFPCW();
		Sys_InitTime();

		//	Initialize
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

		if (GArgs.CheckParm("-RHIDE") || GArgs.CheckParm("-debug"))
			__djgpp_traceback_exit(SIGABRT);
		exit(1);
	}
	catch (...)
	{
		Host_Shutdown();
		dprintf("\n\nExiting due to external exception\n");
		fprintf(stderr, "\nExiting due to external exception\n");
		throw;
	}

    return 0;
}
