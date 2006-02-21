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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define MAXWADFILES 	20

// TYPES -------------------------------------------------------------------

struct search_path_t
{
	VStr			Path;
	search_path_t*	Next;
};

struct version_t
{
	VStr			MainWad;
	VStr			GameDir;
	TArray<VStr>	AddFiles;
	int				ParmFound;
	bool			FixVoices;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void SetupGameDir(const char *dirname);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool	fl_devmode = false;
char	fl_basedir[MAX_OSPATH];
char	fl_savedir[MAX_OSPATH];
char	fl_gamedir[MAX_OSPATH];
char	fl_mainwad[MAX_OSPATH];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static search_path_t*	searchpaths;

const char*				wadfiles[MAXWADFILES];
const char*				gwadirs[MAXWADFILES];
static bool				fl_fixvoices;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FL_AddFile
//
//==========================================================================

void FL_AddFile(const char *file, const char* gwadir)
{
	guard(FL_AddFile);
	int i = 0;
	while (wadfiles[i])
	{
		i++;
	}
	char* newfile = (char*)Z_Malloc(strlen(file) + 1, PU_STATIC, 0);
	strcpy(newfile, file);
	wadfiles[i] = newfile;
	if (gwadir)
	{
		char* newdir = (char*)Z_Malloc(strlen(gwadir) + 1, PU_STATIC, 0);
		strcpy(newdir, gwadir);
		gwadirs[i] = newdir;
	}
	unguard;
}

//==========================================================================
//
//	AddGameDir
//
//==========================================================================

static void AddGameDir(const char *dir)
{
	guard(AddGameDir);
	search_path_t	*info;

	info = (search_path_t*)Z_StrCalloc(sizeof(*info));
	info->Path = VStr(fl_basedir) + "/" + dir;
	info->Next = searchpaths;
	searchpaths = info;

	const char* gwadir = NULL;
	if (fl_savedir[0])
	{
		info = (search_path_t*)Z_StrCalloc(sizeof(*info));
		info->Path = VStr(fl_savedir) + "/" + dir;
		info->Next = searchpaths;
		searchpaths = info;
		gwadir = *info->Path;
	}

	for (int i = 0; i < 1024; i++)
	{
		char	buf[128];

		sprintf(buf, "%s/%s/wad%d.wad", fl_basedir, dir, i);
		if (!Sys_FileExists(buf))
			break;
		FL_AddFile(buf, gwadir);
	}

	if (fl_savedir[0])
	{
		for (int i = 0; i < 1024; i++)
		{
			char	buf[128];
	
			sprintf(buf, "%s/%s/wad%d.wad", fl_savedir, dir, i);
			if (!Sys_FileExists(buf))
				break;
			FL_AddFile(buf, NULL);
		}
	}

	strcpy(fl_gamedir, dir);
	unguard;
}

//==========================================================================
//
//	ParseBase
//
//==========================================================================

static void ParseBase(const char *name)
{
	guard(ParseBase);
	TArray<version_t>	games;
	bool				select_game;
	char				UseName[MAX_OSPATH];

	if (fl_savedir[0] && Sys_FileExists(va("%s/%s", fl_savedir, name)))
	{
		sprintf(UseName, "%s/%s", fl_savedir, name);
	}
	else if (Sys_FileExists(va("%s/%s", fl_basedir, name)))
	{
		sprintf(UseName, "%s/%s", fl_basedir, name);
	}
	else
	{
		return;
	}

	select_game = false;
	SC_OpenFile(UseName);
	while (SC_GetString())
	{
		version_t &dst = *new(games, 0) version_t;
		dst.ParmFound = 0;
		dst.FixVoices = false;
		if (!SC_Compare("game"))
		{
			SC_ScriptError(NULL);
		}
		SC_MustGetString();
		dst.GameDir = sc_String;
		SC_MustGetString();
		if (SC_Compare("iwad"))
		{
			SC_MustGetString();
			dst.MainWad = sc_String;
			SC_MustGetString();
		}
		while (SC_Compare("addfile"))
		{
			SC_MustGetString();
			new(dst.AddFiles) VStr(sc_String);
			SC_MustGetString();
		}
		if (SC_Compare("param"))
		{
			SC_MustGetString();
			dst.ParmFound = M_CheckParm(sc_String);
			if (dst.ParmFound)
			{
				select_game = true;
			}
			SC_MustGetString();
		}
		if (SC_Compare("fixvoices"))
		{
			dst.FixVoices = true;
			SC_MustGetString();
		}
		if (!SC_Compare("end"))
		{
			SC_ScriptError(NULL);
		}
	}
	SC_Close();

	for (TArray<version_t>::TIterator GIt(games); GIt; ++GIt)
	{
		if (select_game && !GIt->ParmFound)
		{
			continue;
		}
		if (fl_mainwad[0])
		{
			if (!GIt->MainWad || GIt->MainWad == fl_mainwad)
			{
				for (TArray<VStr>::TIterator It(GIt->AddFiles); It; ++It)
				{
					FL_AddFile(va("%s/%s", fl_basedir, **It),
						fl_savedir[0] ? fl_savedir : NULL);
				}
				SetupGameDir(*GIt->GameDir);
				fl_fixvoices = GIt->FixVoices;
				return;
			}
			continue;
		}
		if (!GIt->MainWad)
		{
			continue;
		}

		//	First look in the save directory.
		if (fl_savedir[0] && Sys_FileExists(va("%s/%s", fl_savedir, *GIt->MainWad)))
		{
			strcpy(fl_mainwad, *GIt->MainWad);
			FL_AddFile(va("%s/%s", fl_savedir, fl_mainwad), NULL);
			for (TArray<VStr>::TIterator It(GIt->AddFiles); It; ++It)
			{
				FL_AddFile(va("%s/%s", fl_savedir, **It), NULL);
			}
			SetupGameDir(*GIt->GameDir);
			fl_fixvoices = GIt->FixVoices;
			return;
		}

		//	Then in base directory.
		if (Sys_FileExists(va("%s/%s", fl_basedir, *GIt->MainWad)))
		{
			strcpy(fl_mainwad, *GIt->MainWad);
			FL_AddFile(va("%s/%s", fl_basedir, fl_mainwad),
				fl_savedir[0] ? fl_savedir : NULL);
			for (TArray<VStr>::TIterator It(GIt->AddFiles); It; ++It)
			{
				FL_AddFile(va("%s/%s", fl_basedir, **It),
					fl_savedir[0] ? fl_savedir : NULL);
			}
			SetupGameDir(*GIt->GameDir);
			fl_fixvoices = GIt->FixVoices;
			return;
		}
	}

	if (select_game)
		Sys_Error("Main wad file not found.");
	else
		Sys_Error("Game mode indeterminate.");
	unguard;
}

//==========================================================================
//
//	SetupGameDir
//
//==========================================================================

static void SetupGameDir(const char *dirname)
{
	guard(SetupGameDir);
	char		tmp[256];

	sprintf(tmp, "%s/base.txt", dirname);
	ParseBase(tmp);
	AddGameDir(dirname);
	unguard;
}

//==========================================================================
//
//	FL_Init
//
//==========================================================================

void FL_Init()
{
	guard(FL_Init);
	int p;

	//	Set up base directory (main data files).
	strcpy(fl_basedir, ".");
	p = M_CheckParm("-basedir");
	if (p && p < myargc - 1)
	{
		strcpy(fl_basedir, myargv[p + 1]);
	}

	//	Set up save directory (files written by engine).
	p = M_CheckParm("-savedir");
	if (p && p < myargc - 1)
	{
		strcpy(fl_savedir, myargv[p + 1]);
	}
#if defined(__unix__) && !defined(DJGPP) && !defined(_WIN32)
	else
	{
		const char* HomeDir = getenv("HOME");
		if (HomeDir)
		{
			sprintf(fl_savedir, "%s/.vavoom", HomeDir);
		}
	}
#endif

	AddGameDir("basev");

	p = M_CheckParm("-iwad");
	if (p && p < myargc - 1)
	{
		strcpy(fl_mainwad, myargv[p + 1]);
		FL_AddFile(fl_mainwad, NULL);
	}

	p =	M_CheckParm("-devgame");
	if (p && p < myargc - 1)
	{
		fl_devmode = true;
	}
	else
	{
		p =	M_CheckParm("-game");
	}

	if (p && p < myargc - 1)
	{
		SetupGameDir(myargv[p + 1]);
	}
	else
	{
		ParseBase("basev/games.txt");
#ifdef DEVELOPER
		//  I need progs to be loaded from files
		fl_devmode = true;
#endif
	}

	p = M_CheckParm("-file");
	if (p)
	{
		while (++p != myargc && myargv[p][0] != '-' && myargv[p][0] != '+')
		{
			FL_AddFile(myargv[p], NULL);
		}
	}

	const char** filenames = wadfiles;
	const char** gwanames = gwadirs;
	// open all the files, load headers, and count lumps
	for ( ; *filenames ; filenames++, gwanames++)
	{
		W_AddFile(*filenames, *gwanames, fl_fixvoices);
	}
	unguard;
}

//==========================================================================
//
//	FL_FindFile
//
//==========================================================================

bool FL_FindFile(const char *fname, char *dest)
{
	guard(FL_FindFile);
	for (search_path_t* search = searchpaths; search; search = search->Next)
	{
		VStr tmp = search->Path + "/" + fname;
		if (Sys_FileExists(*tmp))
		{
			if (dest)
			{
				strcpy(dest, *tmp);
			}
			return true;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	FL_CreatePath
//
//==========================================================================

void FL_CreatePath(const char* Path)
{
	guard(FL_CreatePath);
	char* Temp = (char*)Z_Malloc(strlen(Path) + 1);
	strcpy(Temp, Path);
	for (size_t i = 3; i <= strlen(Temp); i++)
	{
		if (Temp[i] == '/' || Temp[i] == '\\' || Temp[i] == 0)
		{
			char Save = Temp[i];
			Temp[i] = 0;
			if (!Sys_FileExists(Temp))
				Sys_CreateDirectory(Temp);
			Temp[i] = Save;
		}
	}
	Z_Free(Temp);
	unguard;
}

//==========================================================================
//
//	FL_CreateFilePath
//
//==========================================================================

static void FL_CreateFilePath(const char* Path)
{
	guard(FL_CreateFilePath);
	char* Temp = (char*)Z_Malloc(strlen(Path) + 1);
	FL_ExtractFilePath(Path, Temp);
	FL_CreatePath(Temp);
	Z_Free(Temp);
	unguard;
}

//==========================================================================
//
//	FL_ReadFile
//
//==========================================================================

int FL_ReadFile(const char* name, void** buffer, int tag)
{
	guard(FL_ReadFile);
	int			handle;
	int			count;
	int			length;
	byte		*buf;
	char		realname[MAX_OSPATH];

	if (!FL_FindFile(name, realname))
	{
		return -1;
	}

	handle = Sys_FileOpenRead(realname);
	if (handle == -1)
	{
		Sys_Error("Couldn't open file %s", realname);
	}
	length = Sys_FileSize(handle);
	buf = (byte*)Z_Malloc(length + 1, tag, buffer);
	count = Sys_FileRead(handle, buf, length);
	buf[length] = 0;
	Sys_FileClose(handle);
	
	if (count < length)
	{
		Sys_Error("Couldn't read file %s", realname);
	}
		
	return length;
	unguard;
}

//==========================================================================
//
//	FL_WriteFile
//
//==========================================================================

bool FL_WriteFile(const char* name, const void* source, int length)
{
	guard(FL_WriteFile);
	int		handle;
	int		count;
	const char*		RealName;

	if (fl_savedir[0])
	{
		RealName = va("%s/%s/%s", fl_savedir, fl_gamedir, name);
	}
	else
	{
		RealName = va("%s/%s/%s", fl_basedir, fl_gamedir, name);
	}
	FL_CreateFilePath(RealName);
	handle = Sys_FileOpenWrite(RealName);

	if (handle == -1)
	{
		return false;
	}

	count = Sys_FileWrite(handle, source, length);
	Sys_FileClose(handle);

	if (count < length)
	{
		return false;
	}

	return true;
	unguard;
}

//==========================================================================
//
//	FL_DefaultPath
//
//==========================================================================

void FL_DefaultPath(char *path, const char *basepath)
{
	guard(FL_DefaultPath);
	char    temp[128];

	if (path[0] == '/')
	{
		return;                   // absolute path location
	}
	strcpy(temp, path);
	strcpy(path, basepath);
	strcat(path, temp);
	unguard;
}

//==========================================================================
//
//	FL_DefaultExtension
//
//==========================================================================

void FL_DefaultExtension(char *path, const char *extension)
{
	guard(FL_DefaultExtension);
	char    *src;

	//
	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
        {
			return;                 // it has an extension
		}
		src--;
	}

	strcat(path, extension);
	unguard;
}

//==========================================================================
//
//	FL_StripFilename
//
//==========================================================================

void FL_StripFilename(char *path)
{
	guard(FL_StripFilename);
	int             length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != '/')
	{
		length--;
	}
	path[length] = 0;
	unguard;
}

//==========================================================================
//
//	FL_StripExtension
//
//==========================================================================

void FL_StripExtension(char *path)
{
	guard(FL_StripExtension);
	char *search;

	search = path + strlen(path) - 1;
	while (*search != '/' && search != path)
	{
		if (*search == '.')
		{
			*search = 0;
			return;
		}
		search--;
	}
	unguard;
}

//==========================================================================
//
//	FL_ExtractFilePath
//
//==========================================================================

void FL_ExtractFilePath(const char *path, char *dest)
{
	guard(FL_ExtractFilePath);
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a \ or the start
	//
	while (src != path && *(src-1) != '/' && *(src-1) != '\\')
		src--;

	memcpy(dest, path, src - path);
	dest[src - path] = 0;
	unguard;
}

//==========================================================================
//
//	FL_ExtractFileName
//
//==========================================================================

void FL_ExtractFileName(const char *path, char *dest)
{
	guard(FL_ExtractFileName);
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a \ or the start
	//
	while (src != path && *(src-1) != '/' && *(src-1) != '\\')
		src--;

	strcpy(dest, src);
	unguard;
}

//==========================================================================
//
//	FL_ExtractFileBase
//
//==========================================================================

void FL_ExtractFileBase(const char *path, char *dest)
{
	guard(FL_ExtractFileBase);
#if 0
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a \ or the start
	//
	while (src != path && *(src-1) != '/')
		src--;

	while (*src && *src != '.')
	{
		*dest++ = toupper(*src++);
	}
	*dest = 0;
#else
	int		i;
    int		length;

	i = strlen(path) - 1;
    
    // back up until a \ or the start
    while (i && path[i - 1] != '\\' && path[i - 1] != '/')
    {
		i--;
    }
    
    // copy up to eight characters
    memset(dest, 0, 8);
    length = 0;
    
    while (path[i] && path[i] != '.')
    {
		if (++length == 9)
	    	Sys_Error("Filename base of %s >8 chars", path);

		*dest++ = toupper((int)path[i]);
		i++;
    }
#endif
	unguard;
}

//==========================================================================
//
//	FL_ExtractFileExtension
//
//==========================================================================

void FL_ExtractFileExtension(const char *path, char *dest)
{
	guard(FL_ExtractFileExtension);
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a . or the start
	//
	while (src != path && *(src-1) != '.')
		src--;
	if (src == path)
	{
		*dest = 0;	// no extension
		return;
	}

	strcpy(dest, src);
	unguard;
}

//==========================================================================
//
//	FArchiveFileReader
//
//==========================================================================

class FArchiveFileReader : public FArchive
{
public:
	FArchiveFileReader(FILE* InFile, FOutputDevice *InError) 
		: File(InFile), Error(InError)
	{
		guard(FArchiveFileReader::FArchiveFileReader);
		fseek(File, 0, SEEK_SET);
		ArIsLoading = true;
		ArIsPersistent = true;
		unguard;
	}
	~FArchiveFileReader()
	{
		guard(FArchiveFileReader::~FArchiveFileReader);
		if (File)
			Close();
		unguard;
	}
	void Seek(int InPos)
	{
		//guard(FArchiveFileReader::Seek);
		if (fseek(File, InPos, SEEK_SET))
		{
			ArIsError = true;
			//Error->Logf("seek Failed %i/%i: %i %i", InPos, Size, Pos, ferror(File) );
		}
		//unguard;
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
	bool Close()
	{
		guardSlow(FArchiveFileReader::Close);
		if (File)
			fclose(File);
		File = NULL;
		return !ArIsError;
		unguardSlow;
	}
	void Serialise(void* V, int Length)
	{
		guardSlow(FArchiveFileReader::Serialise);
		if (fread(V, Length, 1, File) != 1)
		{
			ArIsError = true;
			Error->Logf("fread failed: Length=%i Error=%i", Length, ferror(File));
		}
		unguardSlow;
	}
protected:
	FILE *File;
	FOutputDevice *Error;
};

FArchive* FL_OpenFileRead(const char *Name)
{
	guard(FL_OpenFileRead);
	char TmpName[256];

	if (!FL_FindFile(Name, TmpName))
	{
		return NULL;
	}
	FILE *File = fopen(TmpName, "rb");
	if (!File)
	{
		return NULL;
	}
	return new FArchiveFileReader(File, GCon);
	unguard;
}

//==========================================================================
//
//	FArchiveFileWriter
//
//==========================================================================

class FArchiveFileWriter : public FArchive
{
public:
	FArchiveFileWriter(FILE *InFile, FOutputDevice *InError) 
		: File(InFile), Error(InError)
	{
		guard(FArchiveFileWriter::FArchiveFileReader);
		ArIsSaving = true;
		ArIsPersistent = true;
		unguard;
	}
	~FArchiveFileWriter()
	{
		guard(FArchiveFileWriter::~FArchiveFileWriter);
		if (File)
			Close();
		unguard;
	}
	void Seek(int InPos)
	{
		//guard(FArchiveFileWriter::Seek);
		if (fseek(File, InPos, SEEK_SET))
		{
			ArIsError = true;
			//Error->Logf( TEXT("seek Failed %i/%i: %i %i"), InPos, Size, Pos, ferror(File) );
		}
		//unguard;
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
	bool Close()
	{
		guardSlow(FArchiveFileWriter::Close);
		if (File && fclose(File))
		{
			ArIsError = true;
			Error->Logf("fclose failed");
		}
		File = NULL;
		return !ArIsError;
		unguardSlow;
	}
	void Serialise(void* V, int Length)
	{
		guardSlow(FArchiveFileWriter::Serialise);
		if (fwrite(V, Length, 1, File) != 1)
		{
			ArIsError = true;
			Error->Logf("fwrite failed: Length=%i Error=%i", Length, ferror(File));
		}
		unguardSlow;
	}
	void Flush()
	{
		if (fflush(File))
		{
			ArIsError = true;
			Error->Logf("WriteFailed");
		}
	}
protected:
	FILE *File;
	FOutputDevice *Error;
};

FArchive* FL_OpenFileWrite(const char *Name)
{
	guard(FL_OpenFileWrite);
	char TmpName[1024];

	if (fl_savedir[0])
		sprintf(TmpName, "%s/%s/%s", fl_savedir, fl_gamedir, Name);
	else
		sprintf(TmpName, "%s/%s/%s", fl_basedir, fl_gamedir, Name);
	FL_CreateFilePath(TmpName);
	FILE *File = fopen(TmpName, "wb");
	if (!File)
	{
		return NULL;
	}
	return new FArchiveFileWriter(File, GCon);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.20  2006/02/21 22:31:44  dj_jl
//	Created dynamic string class.
//
//	Revision 1.19  2006/02/06 21:46:10  dj_jl
//	Fixed missing validity check, added guard macros.
//	
//	Revision 1.18  2006/01/29 20:41:30  dj_jl
//	On Unix systems use ~/.vavoom for generated files.
//	
//	Revision 1.17  2005/11/05 14:57:36  dj_jl
//	Putting Strife shareware voices in correct namespace.
//	
//	Revision 1.16  2004/12/03 16:15:46  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.15  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.14  2002/11/16 17:13:09  dj_jl
//	Some compatibility changes.
//	
//	Revision 1.13  2002/08/24 14:52:16  dj_jl
//	Fixed screenshots.
//	
//	Revision 1.12  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.11  2002/02/22 18:09:49  dj_jl
//	Some improvements, beautification.
//	
//	Revision 1.10  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.9  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.8  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.7  2001/08/31 17:21:01  dj_jl
//	Finished base game script
//	
//	Revision 1.6  2001/08/30 17:46:21  dj_jl
//	Removed game dependency
//	
//	Revision 1.5  2001/08/21 17:40:54  dj_jl
//	Added devgame mode
//	
//	Revision 1.4  2001/08/04 17:26:59  dj_jl
//	Removed shareware / ExtendedWAD from engine
//	Added support for script base.txt in game directory
//	
//	Revision 1.3  2001/07/31 17:08:37  dj_jl
//	Reworking filesystem
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
