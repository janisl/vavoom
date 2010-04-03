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

#include <time.h>
#include <signal.h>
#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VVccLog : public VLogListener
{
public:
	void Serialise(const char* Text, EName Event)
	{
		dprintf(Text);
	}
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void SignalHandler(int s);
static void Init();
static void ProcessArgs(int ArgCount, char **ArgVector);
static void OpenDebugFile(const VStr& name);
static void DumpAsm();
static void PC_Init();
static void PC_DumpAsm(char*);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VStr			SourceFileName;
static VStr			ObjectFileName;

static VPackage*	CurrentPackage;

static int			num_dump_asm;
static char*		dump_asm_names[1024];
static bool			DebugMode;
static FILE*		DebugFile;

static VLexer		Lex;
static VVccLog		VccLog;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char **argv)
{
	try
	{
		GLog.AddListener(&VccLog);

		int starttime;
		int endtime;

		M_InitByteOrder();

		signal(SIGSEGV, SignalHandler);

		starttime = time(0);
		Init();
		ProcessArgs(argc, argv);

		Lex.OpenSource(SourceFileName);
		VParser Parser(Lex, CurrentPackage);
		Parser.Parse();
		int parsetime = time(0);
		dprintf("Parsed in %02d:%02d\n",
			(parsetime - starttime) / 60, (parsetime - starttime) % 60);
		CurrentPackage->Emit();
		int compiletime = time(0);
		dprintf("Compiled in %02d:%02d\n",
			(compiletime - parsetime) / 60, (compiletime - parsetime) % 60);
		CurrentPackage->WriteObject(*ObjectFileName);
		DumpAsm();
		VName::StaticExit();
		endtime = time(0);
		dprintf("Wrote in %02d:%02d\n",
			(endtime - compiletime) / 60, (endtime - compiletime) % 60);
		dprintf("Time elapsed: %02d:%02d\n",
			(endtime - starttime) / 60, (endtime - starttime) % 60);
		VMemberBase::StaticExit();
	}
	catch (VException& e)
	{
		FatalError(e.What());
	}
	return 0;
}

//==========================================================================
//
// 	signal_handler
//
// 	Shuts down system, on error signal
//
//==========================================================================

static void SignalHandler(int s)
{
	switch (s)
	{
	case SIGSEGV:
		FatalError("%s:%d Segmentation Violation", *Lex.Location.GetSource(),
			Lex.Location.GetLine());
	}
}

//==========================================================================
//
// 	Init
//
//==========================================================================

static void Init()
{
	DebugMode = false;
	DebugFile = NULL;
	num_dump_asm = 0;
	VName::StaticInit();
	VMemberBase::StaticInit();
	PC_Init();
}

//==========================================================================
//
//	DisplayUsage
//
//==========================================================================

static void DisplayUsage()
{
	//	Print usage.
	printf("\n");
	printf("VCC Version 1.%d. Copyright (c)2000-2001 by JL.          ("__DATE__" "__TIME__")\n", PROG_VERSION);
	printf("Usage: vcc [options] source[.c] [object[.dat]]\n");
	printf("    -d<file>     Output debugging information into specified file\n");
	printf("    -a<function> Output function's ASM statements into debug file\n");
	printf("    -D<name>           Define macro\n");
	printf("    -I<directory>      Include files directory\n");
	printf("    -P<directory>      Package import files directory\n");
	exit(1);
}

//==========================================================================
//
// 	ProcessArgs
//
//==========================================================================

static void ProcessArgs(int ArgCount, char **ArgVector)
{
	int		i;
	int		count;
	char*	text;
	char	option;

	count = 0;
	for (i = 1; i < ArgCount; i++)
	{
		text = ArgVector[i];
		if (*text == '-')
		{
			text++;
			if (*text == 0)
			{
				DisplayUsage();
			}
			option = *text++;
			switch (option)
			{
			case 'd':
				DebugMode = true;
				if (*text)
				{
					OpenDebugFile(text);
				}
				break;
			case 'a':
				if (!*text)
				{
					DisplayUsage();
				}
				dump_asm_names[num_dump_asm++] = text;
				break;
			case 'I':
				Lex.AddIncludePath(text);
				break;
			case 'D':
				Lex.AddDefine(text);
				break;
			case 'P':
				VMemberBase::StaticAddPackagePath(text);
				break;
			default:
				DisplayUsage();
				break;
			}
			continue;
		}
		count++;
		switch(count)
		{
		case 1:
			SourceFileName = VStr(text).DefaultExtension(".vc");
			break;
		case 2:
			ObjectFileName = VStr(text).DefaultExtension(".dat");
			break;
		default:
			DisplayUsage();
			break;
		}
	}
	if (count == 0)
	{
		DisplayUsage();
	}
	if (count == 1)
	{
		ObjectFileName = SourceFileName.StripExtension() + ".dat";
	}
	if (!DebugFile)
	{
		VStr DbgFileName;
		DbgFileName = ObjectFileName.StripExtension() + ".txt";
		OpenDebugFile(DbgFileName);
		DebugMode = true;
	}
	SourceFileName = SourceFileName.FixFileSlashes();
	ObjectFileName = ObjectFileName.FixFileSlashes();
	dprintf("Main source file: %s\n", *SourceFileName);
	dprintf("  Resulting file: %s\n", *ObjectFileName);
}

//==========================================================================
//
// 	OpenDebugFile
//
//==========================================================================

static void OpenDebugFile(const VStr& name)
{
	DebugFile = fopen(*name, "w");
	if (!DebugFile)
	{
		FatalError("Can\'t open debug file \"%s\".", *name);
	}
}

//==========================================================================
//
//	DumpAsm
//
//==========================================================================

static void DumpAsm()
{
	for (int i = 0; i < num_dump_asm; i++)
	{
		PC_DumpAsm(dump_asm_names[i]);
	}
}

//==========================================================================
//
//	dprintf
//
//==========================================================================

int dprintf(const char *text, ...)
{
	va_list		argPtr;

	if (!DebugMode)
	{
		return 0;
	}
	FILE* fp = DebugFile? DebugFile : stdout;
	va_start(argPtr, text);
	int ret = vfprintf(fp, text, argPtr);
	va_end(argPtr);
	fflush(fp);
	return ret;
}

//==========================================================================
//
//	PC_Init
//
//==========================================================================

static void PC_Init()
{
	CurrentPackage = new VPackage();
}

//==========================================================================
//
//  PC_DumpAsm
//
//==========================================================================

static void PC_DumpAsm(char* name)
{
	int		i;
	char	buf[1024];
	char	*cname;
	char	*fname;

	VStr::Cpy(buf, name);
	if (strstr(buf, "."))
	{
		cname = buf;
		fname = strstr(buf, ".") + 1;
		fname[-1] = 0;
	}
	else
	{
		dprintf("Dump ASM: Bad name %s\n", name);
		return;
	}
	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->MemberType == MEMBER_Method &&
			!VStr::Cmp(cname, *VMemberBase::GMembers[i]->Outer->Name) &&
			!VStr::Cmp(fname, *VMemberBase::GMembers[i]->Name))
		{
			((VMethod*)VMemberBase::GMembers[i])->DumpAsm();
			return;
		}
	}
	dprintf("Dump ASM: %s not found!\n", name);
}

//==========================================================================
//
//	Malloc
//
//==========================================================================

void* Malloc(size_t size)
{
	if (!size)
	{
		return NULL;
	}

	void *ptr = malloc(size);
	if (!ptr)
	{
		FatalError("Couldn't alloc %d bytes", (int)size);
	}
	memset(ptr, 0, size);
	return ptr;
}

//==========================================================================
//
//	Free
//
//==========================================================================

void Free(void* ptr)
{
	if (ptr)
	{
		free(ptr);
	}
}

//==========================================================================
//
//	va
//
//	Very usefull function from QUAKE
//
//==========================================================================

char *va(const char *text, ...)
{
	va_list		args;
	static char	va_buffer[1024];

	va_start(args, text);
	vsprintf(va_buffer, text, args);
	va_end(args);

	return va_buffer;
}

//==========================================================================
//
//	VFileReader
//
//==========================================================================

class VFileReader : public VStream
{
private:
	FILE*				File;

public:
	VFileReader(FILE* InFile)
	: File(InFile)
	{
		bLoading = true;
	}
	~VFileReader()
	{
		fclose(File);
	}

	//	Stream interface.
	void Serialise(void* V, int Length)
	{
		if (fread(V, Length, 1, File) != 1)
		{
			bError = true;
		}
	}
	void Seek(int InPos)
	{
		if (fseek(File, InPos, SEEK_SET))
		{
			bError = true;
		}
	}
	int Tell()
	{
		return ftell(File);
	}
	int TotalSize()
	{
		int CurPos = ftell(File);
		fseek(File, 0, SEEK_END);
		int Size = ftell(File);
		fseek(File, CurPos, SEEK_SET);
		return Size;
	}
	bool AtEnd()
	{
		return !!feof(File);
	}
	void Flush()
	{
		if (fflush(File))
		{
			bError = true;
		}
	}
	bool Close()
	{
		return !bError;
	}
};


//==========================================================================
//
//	OpenFile
//
//==========================================================================

VStream* OpenFile(const VStr& Name)
{
	FILE* File = fopen(*Name, "rb");
	if (!File)
	{
		return NULL;
	}
	return new VFileReader(File);
}
