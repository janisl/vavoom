//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: cl_local.h 1598 2006-07-04 22:07:34Z dj_jl $
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

struct VZipFileInfo;

//	A zip file.
class VZipFile : public VSearchPath
{
	VStr			ZipFileName;
	VStream*		FileStream;			//	Source stream of the zipfile
	VZipFileInfo*	Files;
	vuint16			NumFiles;			//	Total number of files
	vuint32			BytesBeforeZipFile;	//	Byte before the zipfile, (>0 for sfx)

	vuint32 SearchCentralDir();

public:
	VZipFile(const VStr&);
	~VZipFile();
	VStr FindFile(const VStr&);
	bool FileExists(const VStr&);
	VStream* OpenFileRead(const VStr&);
	void Close();
	int CheckNumForName(VName, EWadNamespace);
	void ReadFromLump(int, void*, int, int);
	int LumpLength(int);
	VName LumpName(int);
	int IterateNS(int, EWadNamespace);
	void BuildGLNodes(VSearchPath*);
	void BuildPVS(VSearchPath*);
	VStream* CreateLumpReaderNum(int);
};

#endif
