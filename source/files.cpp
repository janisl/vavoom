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

#include "gamedefs.h"
#include "fs_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

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
static bool				bIwadAdded;
static TArray<VStr>		IWadDirs;
static int				IWadIndex;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	AddGameDir
//
//==========================================================================

static void AddZipFile(VStr ZipName)
{
	//	Add ZIP file.
	VZipFile* Zip = new VZipFile(ZipName);
	SearchPaths.Append(Zip);

	//	Add all WAD files in the root of the ZIP file.
	TArray<VStr> Wads;
	Zip->ListWadFiles(Wads);
	for (int i = 0; i < Wads.Num(); i++)
	{
		VStr GwaName = Wads[i].StripExtension() + ".gwa";
		VStream* WadStrm = Zip->OpenFileRead(Wads[i]);
		VStream* GwaStrm = Zip->OpenFileRead(GwaName);

		//	Decompress WAD and GWA files into a memory stream since
		// reading from ZIP will be very slow.
		size_t Len = WadStrm->TotalSize();
		vuint8* Buf = new vuint8[Len];
		WadStrm->Serialise(Buf, Len);
		delete WadStrm;
		WadStrm = new VMemoryStream(Buf, Len);
		delete[] Buf;

		if (GwaStrm)
		{
			Len = GwaStrm->TotalSize();
			Buf = new vuint8[Len];
			GwaStrm->Serialise(Buf, Len);
			delete GwaStrm;
			GwaStrm = new VMemoryStream(Buf, Len);
			delete[] Buf;
		}

		W_AddFileFromZip(ZipName + ":" + Wads[i], WadStrm,
			ZipName + ":" + GwaName, GwaStrm);
	}
}

//==========================================================================
//
//	AddGameDir
//
//==========================================================================

static int cmpfunc(const void* v1, const void* v2)
{
	return ((VStr*)v1)->StripExtension().ICmp(((VStr*)v2)->StripExtension());
}

static void AddGameDir(const VStr& basedir, const VStr& dir)
{
	guard(AddGameDir);
	//	First add all .pk3 files in that directory.
	if (Sys_OpenDir(basedir + "/" + dir))
	{
		TArray<VStr> ZipFiles;
		for (VStr test = Sys_ReadDir(); test.IsNotEmpty(); test = Sys_ReadDir())
		{
			VStr ext = test.ExtractFileExtension().ToLower();
			if (ext == "pk3")
				ZipFiles.Append(test);
		}
		Sys_CloseDir();
		qsort(ZipFiles.Ptr(), ZipFiles.Num(), sizeof(VStr), cmpfunc);
		for (int i = 0; i < ZipFiles.Num(); i++)
		{
			AddZipFile(basedir + "/" + dir + "/" + ZipFiles[i]);
		}
	}

	//	Then add wad##.wad files.
	VStr gwadir;
	if (fl_savedir.IsNotEmpty() && basedir != fl_savedir)
	{
		gwadir = fl_savedir + "/" + dir;
	}

	for (int i = 0; i < 1024; i++)
	{
		VStr buf = basedir + "/" + dir + "/wad" + i + ".wad";
		if (!Sys_FileExists(buf))
			break;
		W_AddFile(buf, gwadir, false);
	}

	//	Finally add directory itself.
	VFilesDir* info = new VFilesDir(basedir + "/" + dir);
	SearchPaths.Append(info);
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
	AddGameDir(fl_basedir, dir);
	if (fl_savedir.IsNotEmpty())
	{
		AddGameDir(fl_savedir, dir);
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
	if (fl_savedir.IsNotEmpty() && Sys_FileExists(fl_savedir + "/" + MainWad))
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

	if (fl_savedir.IsNotEmpty() && Sys_FileExists(fl_savedir + "/" + name))
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
	VScriptParser* sc = new VScriptParser(UseName, FL_OpenSysFileRead(UseName));
	while (!sc->AtEnd())
	{
		version_t &dst = games.Alloc();
		dst.ParmFound = 0;
		dst.FixVoices = false;
		sc->Expect("game");
		sc->ExpectString();
		dst.GameDir = sc->String;
		if (sc->Check("iwad"))
		{
			sc->ExpectString();
			dst.MainWad = sc->String;
		}
		while (sc->Check("addfile"))
		{
			sc->ExpectString();
			dst.AddFiles.Append(sc->String);
		}
		if (sc->Check("param"))
		{
			sc->ExpectString();
			dst.ParmFound = GArgs.CheckParm(*sc->String);
			if (dst.ParmFound)
			{
				select_game = true;
			}
		}
		if (sc->Check("fixvoices"))
		{
			dst.FixVoices = true;
		}
		sc->Expect("end");
	}
	delete sc;

	for (int gi = games.Num() - 1; gi >= 0; gi--)
	{
		version_t& G = games[gi];
		if (select_game && !G.ParmFound)
		{
			continue;
		}
		if (fl_mainwad.IsNotEmpty())
		{
			if (G.MainWad.IsEmpty() || G.MainWad == fl_mainwad || select_game)
			{
				if (!bIwadAdded)
				{
					IWadIndex = SearchPaths.Num();
					VStr MainWadPath = FindMainWad(fl_mainwad);
					W_AddFile(MainWadPath, fl_savedir, G.FixVoices);
					bIwadAdded = true;
				}
				for (int j = 0; j < G.AddFiles.Num(); j++)
				{
					W_AddFile(fl_basedir + "/" + G.AddFiles[j], fl_savedir,
						false);
				}
				SetupGameDir(G.GameDir);
				return;
			}
			continue;
		}
		if (G.MainWad.IsEmpty())
		{
			continue;
		}

		//	Look for the main wad file.
		VStr MainWadPath = FindMainWad(G.MainWad);
		if (MainWadPath.IsNotEmpty())
		{
			fl_mainwad = G.MainWad;
			if (!bIwadAdded)
			{
				IWadIndex = SearchPaths.Num();
				W_AddFile(MainWadPath, fl_savedir, G.FixVoices);
				bIwadAdded = true;
			}
			for (int j = 0; j < G.AddFiles.Num(); j++)
			{
				VStr FName = FindMainWad(G.AddFiles[j]);
				if (FName.IsEmpty())
				{
					Sys_Error("Required file %s not found", *G.AddFiles[j]);
				}
				W_AddFile(FName, fl_savedir, false);
			}
			SetupGameDir(G.GameDir);
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
//	RenameSprites
//
//==========================================================================

static void RenameSprites()
{
	guard(RenameSprites);
	VStream* Strm = FL_OpenFileRead("sprite_rename.txt");
	if (!Strm)
	{
		return;
	}

	VScriptParser* sc = new VScriptParser("sprite_rename.txt", Strm);
	TArray<VSpriteRename> Renames;
	TArray<VSpriteRename> AlwaysRenames;
	TArray<VLumpRename> LumpRenames;
	TArray<VLumpRename> AlwaysLumpRenames;
	while (!sc->AtEnd())
	{
		bool Always = sc->Check("always");

		if (sc->Check("lump"))
		{
			sc->ExpectString();
			VStr Old = sc->String.ToLower();
			sc->ExpectString();
			VStr New = sc->String.ToLower();
			VLumpRename& R = Always ? AlwaysLumpRenames.Alloc() :
				LumpRenames.Alloc();
			R.Old = *Old;
			R.New = *New;
			continue;
		}

		sc->ExpectString();
		if (sc->String.Length() != 4)
		{
			sc->Error("Sprite name must be 4 chars long");
		}
		VStr Old = sc->String.ToLower();

		sc->ExpectString();
		if (sc->String.Length() != 4)
		{
			sc->Error("Sprite name must be 4 chars long");
		}
		VStr New = sc->String.ToLower();

		VSpriteRename& R = Always ? AlwaysRenames.Alloc() : Renames.Alloc();
		R.Old[0] = Old[0];
		R.Old[1] = Old[1];
		R.Old[2] = Old[2];
		R.Old[3] = Old[3];
		R.New[0] = New[0];
		R.New[1] = New[1];
		R.New[2] = New[2];
		R.New[3] = New[3];
	}
	delete sc;

	bool RenameAll = !!GArgs.CheckParm("-oldsprites");
	for (int i = 0; i < SearchPaths.Num(); i++)
	{
		if (RenameAll || i == IWadIndex)
		{
			SearchPaths[i]->RenameSprites(Renames, LumpRenames);
		}
		SearchPaths[i]->RenameSprites(AlwaysRenames, AlwaysLumpRenames);
	}
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
#if !defined(_WIN32)
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
			VStr Ext = VStr(GArgs[fp]).ExtractFileExtension().ToLower();
			if (Ext == "pk3" || Ext == "zip")
				AddZipFile(GArgs[fp]);
			else
				W_AddFile(GArgs[fp], VStr(), false);
		}
	}

	RenameSprites();
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
	IWadDirs.Clear();
	unguard;
}

//==========================================================================
//
//	FL_FileExists
//
//==========================================================================

bool FL_FileExists(const VStr& fname)
{
	guard(FL_FileExists);
	for (int i = SearchPaths.Num() - 1; i >= 0 ; i--)
	{
		if (SearchPaths[i]->FileExists(fname))
		{
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
//	FL_OpenSysFileRead
//
//==========================================================================

VStream* FL_OpenSysFileRead(const VStr& Name)
{
	guard(FL_OpenSysFileRead);
	FILE *File = fopen(*Name, "rb");
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

	if (fl_savedir.IsNotEmpty())
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
//	VStreamFileReader
//
//==========================================================================

VStreamFileReader::VStreamFileReader(FILE* InFile, FOutputDevice *InError) 
	: File(InFile), Error(InError)
{
	guard(VStreamFileReader::VStreamFileReader);
	fseek(File, 0, SEEK_SET);
	bLoading = true;
	unguard;
}
VStreamFileReader::~VStreamFileReader()
{
	guard(VStreamFileReader::~VStreamFileReader);
	if (File)
		Close();
	unguard;
}
void VStreamFileReader::Seek(int InPos)
{
	guard(VStreamFileReader::Seek);
	if (fseek(File, InPos, SEEK_SET))
	{
		bError = true;
		//Error->Logf("seek Failed %i/%i: %i %i", InPos, Size, Pos, ferror(File) );
	}
	unguard;
}
int VStreamFileReader::Tell()
{
	return ftell(File);
}
int VStreamFileReader::TotalSize()
{
	int CurPos = ftell(File);
	fseek(File, 0, SEEK_END);
	int Size = ftell(File);
	fseek(File, CurPos, SEEK_SET);
	return Size;
}
bool VStreamFileReader::AtEnd()
{
	return Tell() >= TotalSize();
}
bool VStreamFileReader::Close()
{
	guardSlow(VStreamFileReader::Close);
	if (File)
		fclose(File);
	File = NULL;
	return !bError;
	unguardSlow;
}
void VStreamFileReader::Serialise(void* V, int Length)
{
	guardSlow(VStreamFileReader::Serialise);
	if (fread(V, Length, 1, File) != 1)
	{
		bError = true;
		Error->Logf("fread failed: Length=%i Error=%i", Length, ferror(File));
	}
	unguardSlow;
}
