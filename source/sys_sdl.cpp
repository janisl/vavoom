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

#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <locale.h>
#include <iconv.h>
#include <langinfo.h>
#include <SDL.h>

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
//	Sys_FileTime
//
//	Returns -1 if not present
//
//==========================================================================

int	Sys_FileTime(const char *path)
{
	struct stat		buf;
	
	if (stat(path,&buf) == -1)
		return -1;
	
	return buf.st_mtime;
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
//	Sys_OpenDir
//
//==========================================================================

int Sys_OpenDir(const char *path)
{
	current_dir = opendir(path);
	return current_dir != NULL;
}

//==========================================================================
//
//	Sys_ReadDir
//
//==========================================================================

const char* Sys_ReadDir()
{
	struct dirent *de = readdir(current_dir);
	if (de)
	{
		return de->d_name;
	}
	return NULL;
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

bool Sys_DirExists(const char* path)
{
	struct stat s;
	
	if (stat(path, &s) == -1)
		return false;
	
	return !!S_ISDIR(s.st_mode);
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

double Sys_Time()
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

void Sys_Shutdown()
{
}

//==========================================================================
//
//	PutEndText
//
//	Function to write the Doom end message text
//
//	Copyright (C) 1998 by Udo Munk <udo@umserver.umnet.de>
//
//	This code is provided AS IS and there are no guarantees, none.
//	Feel free to share and modify.
//
//==========================================================================

static void PutEndText(const char *name)
{
	int i, j;
	int att = -1;
	int nlflag = 0;
	char *text;
	char *col;
	iconv_t cd;

	/* if option -noendtxt is set, don't print the text */
	if (M_CheckParm("-noendtxt"))
		return;

	/* if the xterm has more then 80 columns we need to add nl's */
	col = getenv("COLUMNS");
	if (col)
	{
		if (atoi(col) > 80)
			nlflag++;
	}

	/* get the lump with the text */
	text = (char*)W_CacheLumpName(name, PU_CACHE);

	setlocale(LC_CTYPE, "");
	cd = iconv_open(va("%s//TRANSLIT", nl_langinfo(CODESET)), "cp437");

	/* print 80x25 text and deal with the attributes too */
	for (i = 1; i <= 80 * 25; i++, text += 2)
	{
		/* attribute first */
		/* attribute changed? */
		j = (byte)text[1];
		if (j != att)
		{
			static const char map[] = "04261537";
			/* save current attribute */
			att = j;
			/* set new attribute: bright, foreground, background
			 * (we don't have bright background) */
			printf("\033[0;%s3%c;4%cm", (j & 0x88) ? "1;" : "", map[j & 7],
				map[(j & 0x70) >> 4]);
		}

		/* now the text */
		if (*text >= 0 && *text < 32)
			putchar('.');
		else
		{
			char buf[8];
			char *in = text;
			char *out = buf;
			size_t inbytes = 1;
			size_t outbytes = 8;
			iconv(cd, &in, &inbytes, &out, &outbytes);
			fwrite(buf, 1, out - buf, stdout);
		}

		/* do we need a nl? */
		if (nlflag && !(i % 80))
		{
			att = 0;
			puts("\033[0m");
		}
	}
	/* all attributes off */
	printf("\033[0m");

	if (nlflag)
		printf("\n");
}

//==========================================================================
//
// 	Sys_Quit
//
// 	Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
//==========================================================================

void Sys_Quit()
{
	// Shutdown system
	Host_Shutdown();

	SDL_Quit();

	// Throw the end text at the screen
	if (W_CheckNumForName("ENDOOM") >= 0)
	{
		PutEndText("ENDOOM");
	}
	else if (W_CheckNumForName("ENDTEXT") >= 0)
	{
		PutEndText("ENDTEXT");
	}
	else if (W_CheckNumForName("ENDSTRF") >= 0)
	{
		PutEndText("ENDSTRF");
	}
	else
	{
		printf("\nHexen: Beyound Heretic\n");
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

static void stack_trace()
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
	va_list argptr;
	char buf[1024];

	va_start(argptr,error);
	vsprintf(buf, error, argptr);
	va_end(argptr);

	dprintf("Sys_Error: %s", buf);
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
#define MINIMUM_HEAP_SIZE	0x800000		//   8 meg
#define MAXIMUM_HEAP_SIZE	0x8000000		// 128 meg

	int			heap;
	void*		ptr;
	// Maximum allocated for zone heap (64meg default)
	int			maxzone = 0x4000000;
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

	heap = maxzone + 0x10000;

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

char *Sys_ConsoleInput()
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
	 case SIGKILL:	throw VavoomError("Killed");
	 case SIGQUIT:	throw VavoomError("Quited");
     default:		throw VavoomError("Terminated by signal");
	}
#endif
}

//==========================================================================
//
//	Floating point precision dummies for nonintel procesors
//
//==========================================================================

#ifndef __i386__

extern "C" {

void Sys_LowFPPrecision(){}
void Sys_HighFPPrecision(){}

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
	try
	{
		M_InitArgs(argc, argv);

#ifdef __i386__
		Sys_SetFPCW();
#endif

		// if( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 )
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			Sys_Error("SDL_InitSubSystem(): %s\n",SDL_GetError());
		}

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
	catch (VavoomError &e)
	{
		Host_Shutdown();
		stack_trace();

		printf("\n%s\n", e.message);
		dprintf("\n\nERROR: %s\n", e.message);

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

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2005/05/26 16:59:21  dj_jl
//	Initial heap size
//
//	Revision 1.6  2005/04/28 07:16:16  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.5  2004/12/27 12:23:17  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.4  2004/10/11 06:53:16  dj_jl
//	In end text characters below space are replaced with dots.
//	
//	Revision 1.3  2003/10/22 06:15:00  dj_jl
//	Safer handling of signals in Linux
//	
//	Revision 1.2  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2002/01/03 18:39:42  dj_jl
//	Added SDL port
//	
//**************************************************************************
