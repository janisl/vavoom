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

static void SetupGameDir(const VStr& dirname);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool	fl_devmode = false;
VStr	fl_basedir;
VStr	fl_savedir;
VStr	fl_gamedir;
VStr	fl_mainwad;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static search_path_t*	searchpaths;

TArray<VStr>			wadfiles;
TArray<VStr>			gwadirs;
static bool				fl_fixvoices;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FL_AddFile
//
//==========================================================================

void FL_AddFile(const VStr& file, const VStr& gwadir)
{
	guard(FL_AddFile);
	wadfiles.AddItem(file);
	gwadirs.AddItem(gwadir);
	unguard;
}

//==========================================================================
//
//	AddGameDir
//
//==========================================================================

static void AddGameDir(const VStr& dir)
{
	guard(AddGameDir);
	search_path_t	*info;

	info = (search_path_t*)Z_StrCalloc(sizeof(*info));
	info->Path = fl_basedir + "/" + dir;
	info->Next = searchpaths;
	searchpaths = info;

	VStr gwadir;
	if (fl_savedir)
	{
		info = (search_path_t*)Z_StrCalloc(sizeof(*info));
		info->Path = fl_savedir + "/" + dir;
		info->Next = searchpaths;
		searchpaths = info;
		gwadir = info->Path;
	}

	for (int i = 0; i < 1024; i++)
	{
		VStr buf = fl_basedir + "/" + dir + "/wad" + i + ".wad";
		if (!Sys_FileExists(buf))
			break;
		FL_AddFile(buf, gwadir);
	}

	if (fl_savedir)
	{
		for (int i = 0; i < 1024; i++)
		{
			VStr buf = fl_savedir + "/" + dir + "/wad" + i + ".wad";
			if (!Sys_FileExists(buf))
				break;
			FL_AddFile(buf, VStr());
		}
	}

	fl_gamedir = dir;
	unguard;
}

//==========================================================================
//
//	ParseBase
//
//==========================================================================

static void ParseBase(const VStr& name)
{
	guard(ParseBase);
	TArray<version_t>	games;
	bool				select_game;
	VStr				UseName;

	if (fl_savedir && Sys_FileExists(fl_savedir + "/" + name))
	{
		UseName = fl_savedir + "/" + name;
	}
	else if (Sys_FileExists(fl_basedir + "/" + name))
	{
		UseName = fl_basedir + "/" + name;
	}
	else
	{
		return;
	}

	select_game = false;
	SC_OpenFile(*UseName);
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
		if (fl_mainwad)
		{
			if (!GIt->MainWad || GIt->MainWad == fl_mainwad)
			{
				for (TArray<VStr>::TIterator It(GIt->AddFiles); It; ++It)
				{
					FL_AddFile(fl_basedir + "/" + *It, fl_savedir);
				}
				SetupGameDir(GIt->GameDir);
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
		if (fl_savedir && Sys_FileExists(fl_savedir + "/" + GIt->MainWad))
		{
			fl_mainwad = GIt->MainWad;
			FL_AddFile(fl_savedir + "/" + fl_mainwad, VStr());
			for (TArray<VStr>::TIterator It(GIt->AddFiles); It; ++It)
			{
				FL_AddFile(fl_savedir + "/" + *It, VStr());
			}
			SetupGameDir(GIt->GameDir);
			fl_fixvoices = GIt->FixVoices;
			return;
		}

		//	Then in base directory.
		if (Sys_FileExists(fl_basedir + "/" + GIt->MainWad))
		{
			fl_mainwad = GIt->MainWad;
			FL_AddFile(fl_basedir + "/" + fl_mainwad, fl_savedir);
			for (TArray<VStr>::TIterator It(GIt->AddFiles); It; ++It)
			{
				FL_AddFile(fl_basedir + "/" + *It, fl_savedir);
			}
			SetupGameDir(GIt->GameDir);
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

static void SetupGameDir(const VStr& dirname)
{
	guard(SetupGameDir);
	ParseBase(dirname + "/base.txt");
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
	fl_basedir = ".";
	p = M_CheckParm("-basedir");
	if (p && p < myargc - 1)
	{
		fl_basedir = myargv[p + 1];
	}

	//	Set up save directory (files written by engine).
	p = M_CheckParm("-savedir");
	if (p && p < myargc - 1)
	{
		fl_savedir = myargv[p + 1];
	}
#if defined(__unix__) && !defined(DJGPP) && !defined(_WIN32)
	else
	{
		const char* HomeDir = getenv("HOME");
		if (HomeDir)
		{
			fl_savedir = VStr(HomeDir) + "/.vavoom";
		}
	}
#endif

	AddGameDir("basev/common");

	p = M_CheckParm("-iwad");
	if (p && p < myargc - 1)
	{
		fl_mainwad = myargv[p + 1];
		FL_AddFile(fl_mainwad, VStr());
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
			FL_AddFile(myargv[p], VStr());
		}
	}

	// open all the files, load headers, and count lumps
	for (int i = 0; i < wadfiles.Num(); i++)
	{
		W_AddFile(wadfiles[i], gwadirs[i], fl_fixvoices);
	}
	unguard;
}

//==========================================================================
//
//	FL_FindFile
//
//==========================================================================

VStr FL_FindFile(const VStr& fname)
{
	guard(FL_FindFile);
	for (search_path_t* search = searchpaths; search; search = search->Next)
	{
		VStr tmp = search->Path + "/" + fname;
		if (Sys_FileExists(tmp))
		{
			return tmp;
		}
	}
	return VStr();
	unguard;
}

//==========================================================================
//
//	FL_CreatePath
//
//==========================================================================

void FL_CreatePath(const VStr& Path)
{
	guard(FL_CreatePath);
	VStr Temp = Path;
	for (int i = 3; i <= Temp.Length(); i++)
	{
		if (Temp[i] == '/' || Temp[i] == '\\' || Temp[i] == 0)
		{
			char Save = Temp[i];
			Temp[i] = 0;
			if (!Sys_DirExists(Temp))
				Sys_CreateDirectory(Temp);
			Temp[i] = Save;
		}
	}
	unguard;
}

//==========================================================================
//
//	FL_ReadFile
//
//==========================================================================

int FL_ReadFile(const VStr& name, void** buffer, int tag)
{
	guard(FL_ReadFile);
	int			handle;
	int			count;
	int			length;
	byte		*buf;
	VStr		realname;

	realname = FL_FindFile(name);
	if (!realname)
	{
		return -1;
	}

	handle = Sys_FileOpenRead(realname);
	if (handle == -1)
	{
		Sys_Error("Couldn't open file %s", *realname);
	}
	length = Sys_FileSize(handle);
	buf = (byte*)Z_Malloc(length + 1, tag, buffer);
	count = Sys_FileRead(handle, buf, length);
	buf[length] = 0;
	Sys_FileClose(handle);
	
	if (count < length)
	{
		Sys_Error("Couldn't read file %s", *realname);
	}
		
	return length;
	unguard;
}

//==========================================================================
//
//	FL_WriteFile
//
//==========================================================================

bool FL_WriteFile(const VStr& name, const void* source, int length)
{
	guard(FL_WriteFile);
	int		handle;
	int		count;
	VStr	RealName;

	if (fl_savedir)
	{
		RealName = fl_savedir + "/" + fl_gamedir + "/" + name;
	}
	else
	{
		RealName = fl_basedir + "/" + fl_gamedir + "/" + name;
	}
	FL_CreatePath(RealName.ExtractFilePath());
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
//	VStreamFileReader
//
//==========================================================================

class VStreamFileReader : public VStream
{
public:
	VStreamFileReader(FILE* InFile, FOutputDevice *InError) 
		: File(InFile), Error(InError)
	{
		guard(VStreamFileReader::VStreamFileReader);
		fseek(File, 0, SEEK_SET);
		bLoading = true;
		unguard;
	}
	~VStreamFileReader()
	{
		guard(VStreamFileReader::~VStreamFileReader);
		if (File)
			Close();
		unguard;
	}
	void Seek(int InPos)
	{
		//guard(VStreamFileReader::Seek);
		if (fseek(File, InPos, SEEK_SET))
		{
			bError = true;
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
		guardSlow(VStreamFileReader::Close);
		if (File)
			fclose(File);
		File = NULL;
		return !bError;
		unguardSlow;
	}
	void Serialise(void* V, int Length)
	{
		guardSlow(VStreamFileReader::Serialise);
		if (fread(V, Length, 1, File) != 1)
		{
			bError = true;
			Error->Logf("fread failed: Length=%i Error=%i", Length, ferror(File));
		}
		unguardSlow;
	}
protected:
	FILE *File;
	FOutputDevice *Error;
};

//==========================================================================
//
//	FL_OpenFileRead
//
//==========================================================================

VStream* FL_OpenFileRead(const VStr& Name)
{
	guard(FL_OpenFileRead);
	VStr TmpName = FL_FindFile(Name);
	if (!TmpName)
	{
		return NULL;
	}
	FILE *File = fopen(*TmpName, "rb");
	if (!File)
	{
		return NULL;
	}
	return new VStreamFileReader(File, GCon);
	unguard;
}

//==========================================================================
//
//	VStreamFileWriter
//
//==========================================================================

class VStreamFileWriter : public VStream
{
public:
	VStreamFileWriter(FILE *InFile, FOutputDevice *InError) 
		: File(InFile), Error(InError)
	{
		guard(VStreamFileWriter::VStreamFileReader);
		bLoading = false;
		unguard;
	}
	~VStreamFileWriter()
	{
		guard(VStreamFileWriter::~VStreamFileWriter);
		if (File)
			Close();
		unguard;
	}
	void Seek(int InPos)
	{
		//guard(VStreamFileWriter::Seek);
		if (fseek(File, InPos, SEEK_SET))
		{
			bError = true;
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
		guardSlow(VStreamFileWriter::Close);
		if (File && fclose(File))
		{
			bError = true;
			Error->Logf("fclose failed");
		}
		File = NULL;
		return !bError;
		unguardSlow;
	}
	void Serialise(void* V, int Length)
	{
		guardSlow(VStreamFileWriter::Serialise);
		if (fwrite(V, Length, 1, File) != 1)
		{
			bError = true;
			Error->Logf("fwrite failed: Length=%i Error=%i", Length, ferror(File));
		}
		unguardSlow;
	}
	void Flush()
	{
		if (fflush(File))
		{
			bError = true;
			Error->Logf("WriteFailed");
		}
	}
protected:
	FILE *File;
	FOutputDevice *Error;
};

//==========================================================================
//
//	FL_OpenFileWrite
//
//==========================================================================

VStream* FL_OpenFileWrite(const VStr& Name)
{
	guard(FL_OpenFileWrite);
	VStr TmpName;

	if (fl_savedir)
		TmpName = fl_savedir + "/" + fl_gamedir + "/" + Name;
	else
		TmpName = fl_basedir + "/" + fl_gamedir + "/" + Name;
	FL_CreatePath(TmpName.ExtractFilePath());
	FILE *File = fopen(*TmpName, "wb");
	if (!File)
	{
		return NULL;
	}
	return new VStreamFileWriter(File, GCon);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.23  2006/03/26 13:06:18  dj_jl
//	Implemented support for modular progs.
//
//	Revision 1.22  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//	
//	Revision 1.21  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
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
