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
#include "fwaddefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct lumpinfo_t
{
	VName			Name;
	vint32			Position;
	vint32			Size;
	EWadNamespace	Namespace;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWadFile::VWadFile
//
//==========================================================================

VWadFile::VWadFile()
: Stream(NULL)
, NumLumps(0)
, LumpInfo(NULL)
{
}

//==========================================================================
//
//	VWadFile::~VWadFile
//
//==========================================================================

VWadFile::~VWadFile()
{
	Close();
}

//==========================================================================
//
//	VWadFile::Open
//
//==========================================================================

void VWadFile::Open(const VStr& FileName, const VStr& AGwaDir, bool FixVoices,
	VStream* InStream)
{
	guard(VWadFile::Open);
	wadinfo_t		header;
	lumpinfo_t*		lump_p;
	int				i;
	int				length;
	filelump_t*		fileinfo;
	filelump_t*		fi_p;

	Name = FileName;
	GwaDir = AGwaDir;

	if (InStream)
	{
		Stream = InStream;
	}
	else
	{
		// open the file and add to directory
		Stream = FL_OpenSysFileRead(FileName);
		if (!Stream)
		{
			Sys_Error("Couldn't open %s", *FileName);
		}
	}
	GCon->Logf(NAME_Init, "adding %s", *FileName);

	// WAD file
	Stream->Serialise(&header, sizeof(header));
	if (VStr::NCmp(header.identification, "IWAD", 4))
	{
		// Homebrew levels?
		if (VStr::NCmp(header.identification, "PWAD", 4))
		{
			Sys_Error ("Wad file %s doesn't have IWAD "
		 		"or PWAD id\n", *FileName);
		}
	}
	header.numlumps = LittleLong(header.numlumps);
	header.infotableofs = LittleLong(header.infotableofs);
	NumLumps = header.numlumps;
	//	Moved here to make static data less fragmented
	LumpInfo = new lumpinfo_t[NumLumps];
	length = header.numlumps * sizeof(filelump_t);
	fi_p = fileinfo = (filelump_t*)Z_Malloc(length);
	Stream->Seek(header.infotableofs);
	Stream->Serialise(fileinfo, length);

	// Fill in lumpinfo
	lump_p = LumpInfo;

	for (i = 0; i < NumLumps; i++, lump_p++, fileinfo++)
	{
		lump_p->Name = VName(fileinfo->name, VName::AddLower8);
		lump_p->Position = LittleLong(fileinfo->filepos);
		lump_p->Size = LittleLong(fileinfo->size);
		lump_p->Namespace = WADNS_Global;
	}
	
	Z_Free(fi_p);

	//	Set up namespaces.
	InitNamespaces();

	if (FixVoices)
	{
		FixVoiceNamespaces();
	}
	unguard;
}

//==========================================================================
//
//	VWadFile::OpenSingleLump
//
//==========================================================================

void VWadFile::OpenSingleLump(const VStr& FileName)
{
	guard(VWadFile::OpenSingleLump);
	// open the file and add to directory
	Stream = FL_OpenSysFileRead(FileName);
	if (!Stream)
	{
		Sys_Error("Couldn't open %s", *FileName);
	}
	GCon->Logf(NAME_Init, "adding %s", *FileName);

	Name = FileName;
	GwaDir = VStr();

	// single lump file
	NumLumps = 1;
	LumpInfo = new lumpinfo_t[1];

	// Fill in lumpinfo
	LumpInfo->Name = VName(*FileName.ExtractFileBase(), VName::AddLower8);
	LumpInfo->Position = 0;
	LumpInfo->Size = Stream->TotalSize();
	LumpInfo->Namespace = WADNS_Global;
	unguard;
}

//==========================================================================
//
//	VWadFile::Close
//
//==========================================================================

void VWadFile::Close()
{
	guard(VWadFile::Close);
	if (LumpInfo)
	{
		delete[] LumpInfo;
		LumpInfo = NULL;
	}
	NumLumps = 0;
	Name.Clean();
	GwaDir.Clean();
	if (Stream)
	{
		delete Stream;
		Stream = NULL;
	}
	unguard;
}

//==========================================================================
//
//  VWadFile::CheckNumForName
//
//  Returns -1 if name not found.
//
//==========================================================================

int VWadFile::CheckNumForName(VName LumpName, EWadNamespace InNS)
{
	guard(VWadFile::CheckNumForName);
	//	Special ZIP-file namespaces in WAD file are in global namespace.
	EWadNamespace NS = InNS;
	if (NS > WADNS_ZipSpecial)
	{
		NS = WADNS_Global;
	}

	for (int i = NumLumps - 1; i >= 0; i--)
	{
		if (LumpInfo[i].Namespace == NS && LumpInfo[i].Name == LumpName)
		{
			return i;
		}
	}

	// Not found.
	return -1;
	unguard;
}

//==========================================================================
//
//  VWadFile::ReadFromLump
//
//  Loads part of the lump into the given buffer.
//
//==========================================================================

void VWadFile::ReadFromLump(int lump, void* dest, int pos, int size)
{
	guard(VWadFile::ReadFromLump);
	if ((vuint32)lump >= (vuint32)NumLumps)
	{
		Sys_Error("VWadFile::ReadFromLump: %i >= numlumps", lump);
	}

	lumpinfo_t &l = LumpInfo[lump];

	if (pos >= l.Size)
	{
		return;
	}

	Stream->Seek(l.Position + pos);
	Stream->Serialise(dest, size);
	unguard;
}

//==========================================================================
//
//  VWadFile::InitNamespaces
//
//==========================================================================

void VWadFile::InitNamespaces()
{
	guard(VWadFile::InitNamespaces);
	InitNamespace(WADNS_Sprites, NAME_s_start, NAME_s_end, NAME_ss_start, NAME_ss_end);
	InitNamespace(WADNS_Flats, NAME_f_start, NAME_f_end, NAME_ff_start, NAME_ff_end);
	InitNamespace(WADNS_ColourMaps, NAME_c_start, NAME_c_end, NAME_cc_start, NAME_cc_end);
	InitNamespace(WADNS_ACSLibrary, NAME_a_start, NAME_a_end, NAME_aa_start, NAME_aa_end);
	InitNamespace(WADNS_NewTextures, NAME_tx_start, NAME_tx_end);
	InitNamespace(WADNS_Voices, NAME_v_start, NAME_v_end, NAME_vv_start, NAME_vv_end);
	InitNamespace(WADNS_HiResTextures, NAME_hi_start, NAME_hi_end);
	unguard;
}

//==========================================================================
//
//  VWadFile::InitNamespace
//
//==========================================================================

void VWadFile::InitNamespace(EWadNamespace NS, VName Start, VName End,
	VName AltStart, VName AltEnd)
{
	guard(VWadFile::InitNamespace);
	bool InNS = false;
	for (int i = 0; i < NumLumps; i++)
	{
		lumpinfo_t& L = LumpInfo[i];

		//	Skip if lump is already in other namespace.
		if (L.Namespace != WADNS_Global)
			continue;

		if (InNS)
		{
			//	Check for ending marker.
			if (L.Name == End || (AltEnd != NAME_None && L.Name == AltEnd))
			{
				InNS = false;
			}
			else
			{
				L.Namespace = NS;
			}
		}
		else
		{
			//	Check for starting marker.
			if (L.Name == Start || (AltStart != NAME_None && L.Name == AltStart))
			{
				InNS = true;
			}
		}
	}
	unguard;
}

//==========================================================================
//
//  VWadFile::FixVoiceNamespaces
//
//==========================================================================

void VWadFile::FixVoiceNamespaces()
{
	guard(VWadFile::FixVoiceNamespaces);
	for (int i = 0; i < NumLumps; i++)
	{
		lumpinfo_t& L = LumpInfo[i];

		//	Skip if lump is already in other namespace.
		if (L.Namespace != WADNS_Global)
			continue;

		const char* LName = *L.Name;
		if (LName[0] == 'v' && LName[1] == 'o' && LName[2] == 'c' &&
			LName[3] >= '0' && LName[3] <= '9' &&
			(LName[4] == 0 || (LName[4] >= '0' && LName[4] <= '9' &&
			(LName[5] == 0 || (LName[5] >= '0' && LName[5] <= '9' &&
			(LName[6] == 0 || (LName[6] >= '0' && LName[6] <= '9' &&
			(LName[7] == 0 || (LName[7] >= '0' && LName[7] <= '9')))))))))
		{
			L.Namespace = WADNS_Voices;
		}
	}
	unguard;
}

//==========================================================================
//
//  VWadFile::LumpLength
//
//  Returns the buffer size needed to load the given lump.
//
//==========================================================================

int VWadFile::LumpLength(int lump)
{
	guard(VWadFile::LumpLength);
	return LumpInfo[lump].Size;
	unguard;
}

//==========================================================================
//
//  VWadFile::LumpName
//
//==========================================================================

VName VWadFile::LumpName(int lump)
{
	guard(VWadFile::LumpName);
	if (lump >= NumLumps)
	{
		return NAME_None;
	}
	return LumpInfo[lump].Name;
	unguard;
}

//==========================================================================
//
//  VWadFile::IterateNS
//
//==========================================================================

int VWadFile::IterateNS(int Start, EWadNamespace NS)
{
	guard(VWadFile::IterateNS);
	for (int li = Start; li < NumLumps; li++)
	{
		if (LumpInfo[li].Namespace == NS)
		{
			return li;
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VWadFile::BuildGLNodes
//
//==========================================================================

void VWadFile::BuildGLNodes(VSearchPath* GlWad)
{
	guard(VWadFile::BuildGLNodes);
#ifdef CLIENT
	VStr gwaname;
	if (GwaDir)
	{
		FL_CreatePath(GwaDir);
		gwaname = GwaDir + "/" + Name.ExtractFileName();
	}
	else
		gwaname = Name;
	gwaname = gwaname.StripExtension() + ".gwa";

	// Build GL nodes
	if (!GLBSP_BuildNodes(*Name, *gwaname))
	{
		Sys_Error("Node build failed");
	}

	// Build PVS
	GLVis_BuildPVS(*Name, *gwaname);

	// Add GWA file
	((VWadFile*)GlWad)->Open(gwaname, VStr(), false, NULL);
#endif
	unguard;
}

//==========================================================================
//
//	VWadFile::BuildPVS
//
//==========================================================================

void VWadFile::BuildPVS(VSearchPath* BaseWad)
{
	guard(VWadFile::BuildPVS);
#ifdef CLIENT
	VStr name = ((VWadFile*)BaseWad)->Name;

	VStr glname = Name;

	// Close old file
	Close();

	// Build PVS
	GLVis_BuildPVS(*name, BaseWad != this ? *glname : NULL);

	// Add GWA file
	Open(glname, VStr(), false, NULL);
#endif
	unguard;
}

//==========================================================================
//
//  VWadFile::CreateLumpReaderNum
//
//==========================================================================

VStream* VWadFile::CreateLumpReaderNum(int lump)
{
	guard(VWadFile::CreateLumpReaderNum);
	check((vuint32)lump < (vuint32)NumLumps);
	lumpinfo_t &l = LumpInfo[lump];

	// read the lump in
	void* ptr = Z_Malloc(l.Size);
	if (l.Size)
	{
		Stream->Seek(l.Position);
		Stream->Serialise(ptr, l.Size);
	}
	

	//	Create stream.
	VStream* S = new VMemoryStream(ptr, l.Size);
	Z_Free(ptr);
	return S;
	unguard;
}

//==========================================================================
//
//	VWadFile::RenameSprites
//
//==========================================================================

void VWadFile::RenameSprites(const TArray<VSpriteRename>& A)
{
	guard(VWadFile::RenameSprites);
	for (int i = 0; i < NumLumps; i++)
	{
		lumpinfo_t& L = LumpInfo[i];
		if (L.Namespace != WADNS_Sprites)
		{
			continue;
		}
		for (int j = 0; j < A.Num(); j++)
		{
			if ((*L.Name)[0] != A[j].Old[0] || (*L.Name)[1] != A[j].Old[1] ||
				(*L.Name)[2] != A[j].Old[2] || (*L.Name)[3] != A[j].Old[3])
			{
				continue;
			}
			char NewName[12];
			VStr::Cpy(NewName, *L.Name);
			NewName[0] = A[j].New[0];
			NewName[1] = A[j].New[1];
			NewName[2] = A[j].New[2];
			NewName[3] = A[j].New[3];
			L.Name = NewName;
		}
	}
	unguard;
}

int VWadFile::CheckNumForFileName(VStr)
{
	return -1;
}

bool VWadFile::FileExists(const VStr&)
{
	return false;
}

VStream* VWadFile::OpenFileRead(const VStr&)
{
	return NULL;
}
