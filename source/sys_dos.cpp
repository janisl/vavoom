//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

#include <fcntl.h>
#include <unistd.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/exceptn.h>
#include <sys/farptr.h>
#include <go32.h>
#include <signal.h>
#include <conio.h>
#include <allegro.h>

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

//#define PROGS_PROFILE

#define MINIMUM_HEAP_SIZE	0x800000		//  8 meg
#define MAXIMUM_HEAP_SIZE	0x2000000		// 32 meg

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern "C" {

void	__djgpp_traceback_exit(int _sig) __attribute__((noreturn));

void Sys_SetFPCW(void);
void Sys_PushFPCW_SetHigh(void);
void Sys_PopFPCW(void);

}

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

static void Sys_InitTime(void)
{
	int			j;
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
	j = M_CheckParm("-starttime");

	if (j)
	{
		curtime = (double)(atof(myargv[j + 1]));
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

double Sys_Time(void)
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

char *Sys_ConsoleInput(void)
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

    // Throw the end text at the screen
	if (W_CheckNumForName("ENDOOM") >= 0)
    {
	    puttext(1, 1, 80, 25, W_CacheLumpName("ENDOOM", PU_CACHE));
	    gotoxy(1, 24);
	}
    else if (W_CheckNumForName("ENDTEXT") >= 0)
    {
	    puttext(1, 1, 80, 25, W_CacheLumpName("ENDTEXT", PU_CACHE));
		gotoxy(1, 24);
	}
    else if (W_CheckNumForName("ENDSTRF") >= 0)
    {
	    puttext(1, 1, 80, 25, W_CacheLumpName("ENDSTRF", PU_CACHE));
		gotoxy(1, 24);
	}
    else
    {
		printf("\nHexen: Beyound Heretic");
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

	Host_Shutdown();

    // Message last, so it actually prints on the screen
    va_start(argptr, error);
    vsprintf(buf, error, argptr);
    va_end(argptr);

	dprintf("\n\nERROR: %s\n", buf);

    fprintf(stderr, "\n%s\n", buf);

	if (M_CheckParm("-RHIDE") || M_CheckParm("-debug"))
		__djgpp_traceback_exit(SIGABRT);
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
	int			heap;
    void*		ptr;
	// Maximum allocated for zone heap (16meg default)
	int			maxzone = 0x1000000;
	int			p;

	if (M_CheckParm("-opengl"))
	{
		maxzone = 0x800000;
	}
	p = M_CheckParm("-maxzone");
	if (p && p < myargc - 1)
    {
		maxzone = (int)(atof(myargv[p + 1]) * 0x100000);
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
	signal(s,SIG_IGN);

    //	Exit with error message
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
       case SIGINT:
            Sys_Error("Interrupted by User");
            break;
       case SIGSEGV:
            Sys_Error("Segmentation Violation");
            break;
       case SIGTERM:
            Sys_Error("Terminated");
            break;
       case SIGKILL:
            Sys_Error("Killed");
            break;
       case SIGQUIT:
            Sys_Error("Quited");
            break;
       case SIGNOFP:
            Sys_Error("VAVOOM requires a floating-point processor");
            break;
       default:
            Sys_Error("Terminated by signal");
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

extern "C" void PR_Profile2(void);

int main(int argc,char** argv)
{
	M_InitArgs(argc, argv);

	//	Startup Allegro
    allegro_init();

#ifdef PROGS_PROFILE
	install_timer();
	install_int(PR_Profile2, 1);
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

    return 0;
}
