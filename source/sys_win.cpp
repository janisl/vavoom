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

#include "WinLocal.h"
#include "gamedefs.h"
#include <signal.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#ifdef __BORLANDC__
#include <dirent.h>
#endif
#include <sys/stat.h>

// MACROS ------------------------------------------------------------------

#define R_OK	4

#define MINIMUM_HEAP_SIZE	0x800000		//   8 meg
#define MAXIMUM_HEAP_SIZE	0x8000000		// 128 meg

#define PAUSE_SLEEP		50				// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP	20				// sleep time when not focus

#ifndef __BORLANDC__
// POSIX file type test macros.  The parameter is an st_mode value.
#define S_ISDIR(m)  ((m) & S_IFDIR)
#define S_ISCHR(m)  ((m) & S_IFCHR)
#define S_ISBLK(m)  ((m) & S_IFBLK)
#define S_ISREG(m)  ((m) & S_IFREG)
#define S_ISFIFO(m) ((m) & S_IFIFO)
#endif

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern "C" {

void MaskExceptions(void);
void Sys_SetFPCW(void);
void Sys_PushFPCW_SetHigh(void);
void Sys_PopFPCW(void);

} // extern "C"

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

HWND				hwnd;	//	Needed for all DirectX interfaces
HINSTANCE			hInst;	//	Needed for DirectInput

// PRIVATE DATA DEFINITIONS ------------------------------------------------

#ifdef __BORLANDC__
static DIR *current_dir;
#endif

static double		pfreq;
static double		curtime = 0.0;
static double		lastcurtime = 0.0;
static dword		oldtime;
static int			lowshift;

static HANDLE		tevent;

boolean		ActiveApp, Minimized;

static TCvarI		win_priority("win_priority", "0", CVAR_ARCHIVE);
static TCvarI		win_sys_keys("win_sys_keys", "1", CVAR_ARCHIVE);

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
//	Sys_FileTime
//
//	Returns -1 if not present
//
//==========================================================================

int	Sys_FileTime(const char *path)
{
	struct	stat	buf;
	
	if (stat(path, &buf) == -1)
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
	return mkdir(path);
}

//==========================================================================
//
//	Sys_OpenDir
//
//==========================================================================

int Sys_OpenDir(const char *path)
{
#ifdef __BORLANDC__
	current_dir = opendir(path);
	return current_dir != NULL;
#else
	return false;
#endif
}

//==========================================================================
//
//	Sys_ReadDir
//
//==========================================================================

const char *Sys_ReadDir(void)
{
#ifdef __BORLANDC__
	struct dirent *de = readdir(current_dir);
	if (de)
	{
		return de->d_name;
	}
#endif
	return NULL;
}

//==========================================================================
//
//	Sys_CloseDir
//
//==========================================================================

void Sys_CloseDir(void)
{
#ifdef __BORLANDC__
	closedir(current_dir);
#endif
}

//==========================================================================
//
//	Sys_DirExists
//
//==========================================================================

bool Sys_DirExists(const char *path)
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
	DWORD  flOldProtect;

	if (!VirtualProtect((LPVOID)startaddr, length, PAGE_READWRITE, &flOldProtect))
   		Sys_Error("Protection change failed\n");
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
	static int			sametimecount;
	LARGE_INTEGER		PerformanceCount;
	dword				temp, t2;
	double				time;

	Sys_PushFPCW_SetHigh();

	QueryPerformanceCounter(&PerformanceCount);

	temp = ((unsigned int)PerformanceCount.LowPart >> lowshift) |
		   ((unsigned int)PerformanceCount.HighPart << (32 - lowshift));

	// check for turnover or backward time
	if ((temp <= oldtime) && ((oldtime - temp) < 0x10000000))
	{
		oldtime = temp;	// so we can't get stuck
	}
	else
	{
		t2 = temp - oldtime;

		time = (double)t2 * pfreq;
		oldtime = temp;

		curtime += time;

		if (curtime == lastcurtime)
		{
			sametimecount++;

			if (sametimecount > 100000)
			{
				curtime += 1.0;
				sametimecount = 0;
			}
		}
		else
		{
			sametimecount = 0;
		}

		lastcurtime = curtime;
	}

	Sys_PopFPCW();

    return curtime;
}

//==========================================================================
//
//  Sys_InitTime
//
//==========================================================================

static void Sys_InitTime(void)
{
	int		        j;
	LARGE_INTEGER	PerformanceFreq;
	LARGE_INTEGER	PerformanceCount;
	dword			lowpart, highpart;

	if (!QueryPerformanceFrequency(&PerformanceFreq))
		Sys_Error("No hardware timer available");

	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	lowpart = (dword)PerformanceFreq.LowPart;
	highpart = (dword)PerformanceFreq.HighPart;
	lowshift = 0;

	while (highpart || (lowpart > 2000000.0))
	{
		lowshift++;
		lowpart >>= 1;
		lowpart |= (highpart & 1) << 31;
		highpart >>= 1;
	}

	pfreq = 1.0 / (double)lowpart;

	//	Read current time and set old time.
	QueryPerformanceCounter(&PerformanceCount);

	oldtime = ((dword)PerformanceCount.LowPart >> lowshift) |
			  ((dword)PerformanceCount.HighPart << (32 - lowshift));

	//	Set start time
	j = M_CheckParm("-starttime");

	if (j)
	{
		curtime = (double)(atof(myargv[j + 1]));
	}
	else
	{
		curtime = 0.0;
	}

	lastcurtime = curtime;
}

//==========================================================================
//
//	Sys_Shutdown
//
//==========================================================================

void Sys_Shutdown(void)
{
	CoUninitialize();
	ShowCursor(TRUE);
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
	dprintf("==========================================================================\n");
	dprintf("			Shuting down VAVOOM\n");
	dprintf("==========================================================================\n");

    // Shutdown system
	Host_Shutdown();

	if (tevent)
		CloseHandle(tevent);

	// Exit
	SendMessage(hwnd, WM_CLOSE, 0, 0);
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
//	Sys_PageIn
//
//==========================================================================

volatile int sys_checksum;

void Sys_PageIn(void *ptr, int size)
{
	byte	*x;
	int		m, n;

	// touch all the memory to make sure it's there. The 16-page skip is to
	// keep Win 95 from thinking we're trying to page ourselves in (we are
	// doing that, of course, but there's no reason we shouldn't)
	x = (byte *)ptr;

	for (n = 0; n < 4; n++)
	{
		for (m = 0; m < (size - 16 * 0x1000); m += 4)
		{
			sys_checksum += *(int *)&x[m];
			sys_checksum += *(int *)&x[m + 16 * 0x1000];
		}
	}
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
	int			p;
	MEMORYSTATUS	lpBuffer;

	p = M_CheckParm("-maxzone");
	if (p && p < myargc - 1)
	{
		maxzone = (int)(atof(myargv[p + 1]) * 0x100000);
		if (maxzone < MINIMUM_HEAP_SIZE)
			maxzone = MINIMUM_HEAP_SIZE;
		if (maxzone > MAXIMUM_HEAP_SIZE)
			maxzone = MAXIMUM_HEAP_SIZE;
	}

	lpBuffer.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus (&lpBuffer);

	// take the greater of all the available memory or half the total memory,
	// but at least 8 Mb
	heap = lpBuffer.dwAvailPhys;

	if (heap < MINIMUM_HEAP_SIZE)
		heap = MINIMUM_HEAP_SIZE;

	if (heap < int(lpBuffer.dwTotalPhys >> 1))
		heap = lpBuffer.dwTotalPhys >> 1;

	if (heap > maxzone)
		heap = maxzone;

	ptr = malloc(heap);
	if (!ptr)
	{
		Sys_Error("Not enough memory");
	}

	dprintf("  0x%x (%f meg) allocated for zone, ZoneBase: 0x%X\n",
		heap, (float)heap / (float)(1024 * 1024), (int)ptr);

	Sys_PageIn(ptr, heap);

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
//	Sys_ConsoleInput
//
//==========================================================================

char *Sys_ConsoleInput(void)
{
//FIXME
	return NULL;
}

//==========================================================================
//
//	WndProc
//
//==========================================================================

static LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg,
								WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	 case WM_KEYDOWN:
		if (!(lParam & 0x40000000) && wParam == VK_PAUSE)
		{
			IN_KeyEvent(K_PAUSE, true);
		}
		return 0;

	 case WM_KEYUP:
		if (wParam == VK_PAUSE)
		{
			IN_KeyEvent(K_PAUSE, false);
		}
		return 0;

	 case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	 case MM_MCINOTIFY:
		return CD_MessageHandler(hwnd, iMsg, wParam, lParam);

	 case WM_SYSCOMMAND:
		// Check for maximize being hit
		switch (wParam & ~0x0F)
		{
		 case SC_SCREENSAVE:
		 case SC_MONITORPOWER:
			// don't call DefWindowProc() because we don't want to start
			// the screen saver fullscreen
			return 0;
		}
		break;

	 case WM_ACTIVATE:
		ActiveApp = !(LOWORD(wParam) == WA_INACTIVE);
		Minimized = (BOOL)HIWORD(wParam);
		break;

	 case WM_KILLFOCUS:
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
		break;

	 case WM_SETFOCUS:
		if (win_priority)
		{
			SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		}
		else
		{
			SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		}
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

//==========================================================================
//
//	SleepUntilInput
//
//==========================================================================

void SleepUntilInput(int time)
{
	MsgWaitForMultipleObjects(1, &tevent, FALSE, time, QS_ALLINPUT);
}

#ifndef USEASM

void MaskExceptions(void)
{
}

void Sys_SetFPCW(void)
{
}

void Sys_PushFPCW_SetHigh(void)
{
}

void Sys_PopFPCW(void)
{
}

void Sys_LowFPPrecision(void)
{
}

void Sys_HighFPPrecision(void)
{
}

#endif

//==========================================================================
//
//	WinMain
//
// 	Main program
//
//==========================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int iCmdShow)
{
	WNDCLASSEX	wndclass;
	MSG 		msg;
	HACCEL		ghAccel;

	try
	{
		M_InitArgs(__argc, __argv);

		hInst = hInstance;

		//	Create window class
		wndclass.cbSize        = sizeof(wndclass);
		wndclass.style         = 0;
		wndclass.lpfnWndProc   = WndProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInst;
		wndclass.hIcon         = LoadIcon(hInstance, "APPICON");
		wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = "VAVOOM";
		wndclass.hIconSm       = NULL;

		if (!RegisterClassEx(&wndclass))
		{
			MessageBox(NULL, "Failed to register class", "Error", MB_OK);
			return 1;
		}

		//	Create window
		hwnd = CreateWindowEx(WS_EX_APPWINDOW,
			"VAVOOM", "VAVOOM for Windows'95", WS_POPUP,
			0, 0, 2, 2, NULL, NULL, hInst, NULL);
		if (!hwnd)
		{
			MessageBox(NULL, "Couldn't create window", "Error", MB_OK);
			return 1;
		}

		// Make the window visible & update its client area
		ShowWindow(hwnd, iCmdShow);
		UpdateWindow(hwnd);

		//	Initialize COM
		if (FAILED(CoInitialize(NULL)))
		{
			MessageBox(hwnd, "Failed to initialize COM", "Error", MB_OK);
			return 1;
		}

		//	Create event
		tevent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!tevent)
		{
			CoUninitialize();
			MessageBox(hwnd, "Couldn't create event", "Error", MB_OK);
			return 1;
		}

		ghAccel = LoadAccelerators(hInst, "AppAccel");

		ShowCursor(FALSE);

		//	Install signal handlers
		signal(SIGINT,  signal_handler);
		signal(SIGILL,  signal_handler);
		signal(SIGFPE,  signal_handler);
		signal(SIGSEGV, signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGBREAK,signal_handler);
		signal(SIGABRT, signal_handler);

		MaskExceptions();
		Sys_SetFPCW();

		Sys_InitTime();

		Host_Init();
		while (1)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					dprintf("Quit message\n");
					Sys_Quit();
				}
				else if (!win_sys_keys && (msg.message == WM_SYSKEYDOWN ||
					msg.message == WM_SYSKEYUP))
				{
				}
				else if (!TranslateAccelerator(msg.hwnd, ghAccel, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			if (Minimized || !ActiveApp)
			{
				SleepUntilInput(PAUSE_SLEEP);
				continue;
			}

			Host_Frame();
		}
	}
	catch (VavoomError &e)
	{
		Host_Shutdown();

		dprintf("\n\nERROR: %s\n", e.message);
		MessageBox(hwnd, e.message, "Error", MB_OK);

		SendMessage(hwnd, WM_CLOSE, 0, 0);
		return 1;
	}
	catch (...)
	{
		Host_Shutdown();
		dprintf("\n\nExiting due to external exception\n");
		throw;
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2001/12/04 18:11:59  dj_jl
//	Fixes for compiling with MSVC
//
//	Revision 1.9  2001/12/01 17:52:52  dj_jl
//	no message
//	
//	Revision 1.8  2001/11/09 14:19:42  dj_jl
//	Functions for directory listing
//	
//	Revision 1.7  2001/10/08 17:26:17  dj_jl
//	Started to use exceptions
//	
//	Revision 1.6  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.5  2001/09/20 16:27:43  dj_jl
//	Improved zone allocation
//	
//	Revision 1.4  2001/08/29 17:49:36  dj_jl
//	Added file time functions
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
