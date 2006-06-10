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

class VFilesDir : public VSearchPath
{
private:
	VStr			Path;

public:
	VFilesDir(const VStr& aPath)
	: Path(aPath)
	{}
	VStr FindFile(const VStr&);
	VStream* OpenFileRead(const VStr&);
	void Close();
	int CheckNumForName(VName, EWadNamespace);
	void ReadLump(int, void*);
	void ReadFromLump(int, void*, int, int);
	void* CacheLumpNum(int);
	int LumpLength(int);
	VName LumpName(int);
	int IterateNS(int, EWadNamespace);
	void BuildGLNodes(VSearchPath*);
	void BuildPVS(VSearchPath*);
	VStream* CreateLumpReaderNum(int);
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

TArray<VSearchPath*>	SearchPaths;

TArray<VStr>			wadfiles;
TArray<VStr>			gwadirs;
static bool				fl_fixvoices;
static TArray<VStr>		IWadDirs;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FL_AddFile
//
//==========================================================================

void FL_AddFile(const VStr& file, const VStr& gwadir)
{
	guard(FL_AddFile);
	wadfiles.Append(file);
	gwadirs.Append(gwadir);
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
	VFilesDir	*info;

	info = new VFilesDir(fl_basedir + "/" + dir);
	SearchPaths.Append(info);

	VStr gwadir;
	if (fl_savedir)
	{
		info = new VFilesDir(fl_savedir + "/" + dir);
		SearchPaths.Append(info);
		gwadir = fl_savedir + "/" + dir;
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
//	FindMainWad
//
//==========================================================================

static VStr FindMainWad(VStr MainWad)
{
	//	First check in IWAD directories.
	for (int i = 0; i < IWadDirs.Num(); i++)
	{
		if (Sys_FileExists(IWadDirs[i] + "/" + MainWad))
		{
			return IWadDirs[i] + "/" + MainWad;
		}
	}

	//	Then look in the save directory.
	if (fl_savedir && Sys_FileExists(fl_savedir + "/" + MainWad))
	{
		return fl_savedir + "/" + MainWad;
	}

	//	Finally in base directory.
	if (Sys_FileExists(fl_basedir + "/" + MainWad))
	{
		return fl_basedir + "/" + MainWad;
	}

	return VStr();
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
		version_t &dst = games.Alloc();
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
			dst.AddFiles.Append(sc_String);
			SC_MustGetString();
		}
		if (SC_Compare("param"))
		{
			SC_MustGetString();
			dst.ParmFound = GArgs.CheckParm(sc_String);
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

	for (int gi = games.Num() - 1; gi >= 0; gi--)
	{
		version_t& G = games[gi];
		if (select_game && !G.ParmFound)
		{
			continue;
		}
		if (fl_mainwad)
		{
			if (!G.MainWad || G.MainWad == fl_mainwad)
			{
				for (int j = 0; j < G.AddFiles.Num(); j++)
				{
					FL_AddFile(fl_basedir + "/" + G.AddFiles[j], fl_savedir);
				}
				SetupGameDir(G.GameDir);
				fl_fixvoices = G.FixVoices;
				return;
			}
			continue;
		}
		if (!G.MainWad)
		{
			continue;
		}

		//	Look for the main wad file.
		VStr MainWadPath = FindMainWad(G.MainWad);
		if (MainWadPath)
		{
			fl_mainwad = G.MainWad;
			FL_AddFile(MainWadPath, fl_savedir);
			for (int j = 0; j < G.AddFiles.Num(); j++)
			{
				VStr FName = FindMainWad(G.AddFiles[j]);
				if (!FName)
				{
					Sys_Error("Required file %s not found", *G.AddFiles[j]);
				}
				FL_AddFile(FName, fl_savedir);
			}
			SetupGameDir(G.GameDir);
			fl_fixvoices = G.FixVoices;
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
	const char* p;

	//	Set up base directory (main data files).
	fl_basedir = ".";
	p = GArgs.CheckValue("-basedir");
	if (p)
	{
		fl_basedir = p;
	}

	//	Set up save directory (files written by engine).
	p = GArgs.CheckValue("-savedir");
	if (p)
	{
		fl_savedir = p;
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

	//	Set up additional directories where to look for IWAD files.
	int iwp = GArgs.CheckParm("-iwaddir");
	if (iwp)
	{
		while (++iwp != GArgs.Count() && GArgs[iwp][0] != '-' && GArgs[iwp][0] != '+')
		{
			IWadDirs.Append(GArgs[iwp]);
		}
	}

	AddGameDir("basev/common");

	p = GArgs.CheckValue("-iwad");
	if (p)
	{
		fl_mainwad = p;
		FL_AddFile(fl_mainwad, VStr());
	}

	p = GArgs.CheckValue("-devgame");
	if (p)
	{
		fl_devmode = true;
	}
	else
	{
		p = GArgs.CheckValue("-game");
	}

	if (p)
	{
		SetupGameDir(p);
	}
	else
	{
		ParseBase("basev/games.txt");
#ifdef DEVELOPER
		//  I need progs to be loaded from files
		fl_devmode = true;
#endif
	}

	int fp = GArgs.CheckParm("-file");
	if (fp)
	{
		while (++fp != GArgs.Count() && GArgs[fp][0] != '-' && GArgs[fp][0] != '+')
		{
			FL_AddFile(GArgs[fp], VStr());
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
//	FL_Shutdown
//
//==========================================================================

void FL_Shutdown()
{
	guard(FL_Shutdown);
	for (int i = 0; i < SearchPaths.Num(); i++)
	{
		delete SearchPaths[i];
	}
	SearchPaths.Clear();
	fl_basedir.Clean();
	fl_savedir.Clean();
	fl_gamedir.Clean();
	fl_mainwad.Clean();
	wadfiles.Clear();
	gwadirs.Clear();
	IWadDirs.Clear();
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
	for (int i = SearchPaths.Num() - 1; i >= 0 ; i--)
	{
		VStr tmp = SearchPaths[i]->FindFile(fname);
		if (tmp)
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
	for (size_t i = 3; i <= Temp.Length(); i++)
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
	for (int i = SearchPaths.Num() - 1; i >= 0; i--)
	{
		VStream* Strm = SearchPaths[i]->OpenFileRead(Name);
		if (Strm)
		{
			return Strm;
		}
	}
	return NULL;
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

//==========================================================================
//
//	VFilesDir::FindFile
//
//==========================================================================

VStr VFilesDir::FindFile(const VStr& fname)
{
	guard(VFilesDir::FindFile);
	VStr tmp = Path + "/" + fname;
	if (Sys_FileExists(tmp))
	{
		return tmp;
	}
	return VStr();
	unguard;
}

//==========================================================================
//
//	VFilesDir::OpenFileRead
//
//==========================================================================

VStream* VFilesDir::OpenFileRead(const VStr& Name)
{
	guard(FL_OpenFileRead);
	VStr TmpName = Path + "/" + Name;
	if (!Sys_FileExists(TmpName))
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

void VFilesDir::Close()
{
}
int VFilesDir::CheckNumForName(VName, EWadNamespace)
{
	return -1;
}
void VFilesDir::ReadLump(int, void*)
{
	Sys_Error("ReadLump on directory");
}
void VFilesDir::ReadFromLump(int, void*, int, int)
{
	Sys_Error("ReadFromLump on directory");
}
void* VFilesDir::CacheLumpNum(int)
{
	Sys_Error("CacheLumpNum on directory");
	return NULL;
}
int VFilesDir::LumpLength(int)
{
	return 0;
}
VName VFilesDir::LumpName(int)
{
	return NAME_None;
}
int VFilesDir::IterateNS(int, EWadNamespace)
{
	return -1;
}
void VFilesDir::BuildGLNodes(VSearchPath*)
{
	Sys_Error("BuildGLNodes on directory");
}
void VFilesDir::BuildPVS(VSearchPath*)
{
	Sys_Error("BuildPVS on directory");
}
VStream* VFilesDir::CreateLumpReaderNum(int)
{
	return NULL;
}
