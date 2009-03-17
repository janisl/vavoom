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

#ifndef __FS_LOCAL_H__
#define __FS_LOCAL_H__

//==========================================================================
//	VSpriteRename
//==========================================================================

struct VSpriteRename
{
	char	Old[4];
	char	New[4];
};

struct VLumpRename
{
	VName	Old;
	VName	New;
};

//==========================================================================
//	VSearchPath
//==========================================================================

class VSearchPath
{
public:
	virtual ~VSearchPath();
	virtual bool FileExists(const VStr&) = 0;
	virtual VStream* OpenFileRead(const VStr&) = 0;
	virtual void Close() = 0;
	virtual int CheckNumForName(VName, EWadNamespace) = 0;
	virtual int CheckNumForFileName(VStr) = 0;
	virtual void ReadFromLump(int, void*, int, int) = 0;
	virtual int LumpLength(int) = 0;
	virtual VName LumpName(int) = 0;
	virtual int IterateNS(int, EWadNamespace) = 0;
	virtual void BuildGLNodes(VSearchPath*) = 0;
	virtual void BuildPVS(VSearchPath*) = 0;
	virtual VStream* CreateLumpReaderNum(int) = 0;
	virtual void RenameSprites(const TArray<VSpriteRename>&,
		const TArray<VLumpRename>&) = 0;
};

//==========================================================================
//	VFilesDir
//==========================================================================

class VFilesDir : public VSearchPath
{
private:
	VStr			Path;
	TArray<VStr>	CachedFiles;

public:
	VFilesDir(const VStr& aPath)
	: Path(aPath)
	{}
	bool FileExists(const VStr&);
	VStream* OpenFileRead(const VStr&);
	void Close();
	int CheckNumForName(VName, EWadNamespace);
	int CheckNumForFileName(VStr);
	void ReadFromLump(int, void*, int, int);
	int LumpLength(int);
	VName LumpName(int);
	int IterateNS(int, EWadNamespace);
	void BuildGLNodes(VSearchPath*);
	void BuildPVS(VSearchPath*);
	VStream* CreateLumpReaderNum(int);
	void RenameSprites(const TArray<VSpriteRename>&,
		const TArray<VLumpRename>&);
};

//==========================================================================
//	VWadFile
//==========================================================================

struct lumpinfo_t;

class VWadFile : public VSearchPath
{
private:
	VStr			Name;
	VStream*		Stream;
	int				NumLumps;
	lumpinfo_t*		LumpInfo;	// Location of each lump on disk.
	VStr			GwaDir;

	void InitNamespaces();
	void FixVoiceNamespaces();
	void InitNamespace(EWadNamespace NS, VName Start, VName End,
		VName AltStart = NAME_None, VName AltEnd = NAME_None);

public:
	VWadFile();
	~VWadFile();
	void Open(const VStr&, const VStr&, bool, VStream*);
	void OpenSingleLump(const VStr& FileName);
	void Close();
	int CheckNumForName(VName LumpName, EWadNamespace NS);
	int CheckNumForFileName(VStr);
	void ReadFromLump(int lump, void* dest, int pos, int size);
	int LumpLength(int);
	VName LumpName(int);
	int IterateNS(int, EWadNamespace);
	void BuildGLNodes(VSearchPath*);
	void BuildPVS(VSearchPath*);
	bool FileExists(const VStr&);
	VStream* OpenFileRead(const VStr&);
	VStream* CreateLumpReaderNum(int);
	void RenameSprites(const TArray<VSpriteRename>&,
		const TArray<VLumpRename>&);
};

//==========================================================================
//	VZipFile
//==========================================================================

struct VZipFileInfo;

//	A zip file.
class VZipFile : public VSearchPath
{
private:
	VStr			ZipFileName;
	VStream*		FileStream;			//	Source stream of the zipfile
	VZipFileInfo*	Files;
	vuint16			NumFiles;			//	Total number of files
	vuint32			BytesBeforeZipFile;	//	Byte before the zipfile, (>0 for sfx)

	vuint32 SearchCentralDir();
	static int FileCmpFunc(const void*, const void*);

public:
	VZipFile(const VStr&);
	~VZipFile();
	bool FileExists(const VStr&);
	VStream* OpenFileRead(const VStr&);
	void Close();
	int CheckNumForName(VName, EWadNamespace);
	int CheckNumForFileName(VStr);
	void ReadFromLump(int, void*, int, int);
	int LumpLength(int);
	VName LumpName(int);
	int IterateNS(int, EWadNamespace);
	VStream* CreateLumpReaderNum(int);
	void RenameSprites(const TArray<VSpriteRename>&,
		const TArray<VLumpRename>&);

	void BuildGLNodes(VSearchPath*);
	void BuildPVS(VSearchPath*);

	void ListWadFiles(TArray<VStr>&);
};

//==========================================================================
//	VStreamFileReader
//==========================================================================

class VStreamFileReader : public VStream
{
public:
	VStreamFileReader(FILE*, FOutputDevice*);
	~VStreamFileReader();
	void Seek(int InPos);
	int Tell();
	int TotalSize();
	bool AtEnd();
	bool Close();
	void Serialise(void* V, int Length);

protected:
	FILE *File;
	FOutputDevice *Error;
};

void W_AddFileFromZip(const VStr&, VStream*, const VStr&, VStream*);

bool GLBSP_BuildNodes(const char *name, const char* gwafile);
void GLVis_BuildPVS(const char *srcfile, const char* gwafile);

extern TArray<VSearchPath*>	SearchPaths;

#endif
