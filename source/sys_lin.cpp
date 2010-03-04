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

// HEADER FILES ------------------------------------------------------------

#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <allegro.h>

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern "C" {

void MaskExceptions();
void Sys_SetFPCW();
void Sys_PushFPCW_SetHigh();
void Sys_PopFPCW();

} // extern "C"

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#ifdef USE_GUARD_SIGNAL_CONTEXT
jmp_buf __Context::Env;
const char* __Context::ErrToThrow;
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static DIR *current_dir;

// CODE --------------------------------------------------------------------

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
//	Sys_Sleep
//
//==========================================================================

void Sys_Sleep()
{
	usleep(1);
}

//==========================================================================
//
//	Sys_Shutdown
//
//==========================================================================

void Sys_Shutdown()
{
	allegro_exit();
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

static void PutEndText(const char* text)
{
	int i, j;
	int att = -1;
	int nlflag = 0;
	char *col;

	//	If option -noendtxt is set, don't print the text.
	if (GArgs.CheckParm("-noendtxt"))
		return;

	//	If the xterm has more then 80 columns we need to add nl's
	col = getenv("COLUMNS");
	if (col)
	{
		if (atoi(col) > 80)
			nlflag++;
	}
	else
		nlflag++;

	/* print 80x25 text and deal with the attributes too */
	for (i = 1; i <= 80 * 25; i++, text += 2)
	{
		//	Attribute first
		j = (byte)text[1];
		//	Attribute changed?
		if (j != att)
		{
			static const char map[] = "04261537";
			//	Save current attribute
			att = j;
			//	Set new attribute: bright, foreground, background
			// (we don't have bright background)
			printf("\033[0;%s3%c;4%cm", (j & 0x88) ? "1;" : "", map[j & 7],
				map[(j & 0x70) >> 4]);
		}

		//	Now the text.
		if (*text < 32)
			putchar('.');
		else
			putchar(*text);

		//	Do we need a nl?
		if (nlflag && !(i % 80))
		{
			att = 0;
			puts("\033[0m");
		}
	}
	//	All attributes off
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

void Sys_Quit(const char* EndText)
{
	// Shutdown system
	Host_Shutdown();

	// Throw the end text at the screen
	if (EndText)
	{
		PutEndText(EndText);
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
		stack_addr[(X)]= __builtin_return_address((X)); \
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
	static void*	stack_addr[MAX_STACK_ADDR];
	// can we still print entries on the calling stack or have we finished?
	static bool		continue_stack_trace = true;

	// clean the stack addresses if necessary
	for (i = 0; i < MAX_STACK_ADDR; i++)
	{
		stack_addr[i] = 0;
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
	//fprintf(stderr, "You suddenly see a gruesome SOFTWARE BUG leap for your throat!\n");

	// Open the non-existing file
	fff = fopen("crash.txt", "w");

	// Invalid file
	if (fff)
	{
		// dump stack frame
		for (i = (MAX_STACK_ADDR - 1); i >= 0 ; i--)
		{
			fprintf(fff,"%8p\n", stack_addr[i]);
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

	throw VavoomError(buf);
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
#ifdef USE_GUARD_SIGNAL_CONTEXT
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

#if !USE_ASM_I386

void MaskExceptions()
{
}

void Sys_SetFPCW()
{
}

void Sys_PushFPCW_SetHigh()
{
}

void Sys_PopFPCW()
{
}

void Sys_LowFPPrecision()
{
}

void Sys_HighFPPrecision()
{
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
		GArgs.Init(argc, argv);

		Sys_SetFPCW();

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
END_OF_MAIN()	//	For Allegro
