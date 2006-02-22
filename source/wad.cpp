//**************************************************************************
//**
//**    ##   ##    ##    ##   ##   ####     ####   ###     ###
//**    ##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**     ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**     ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**      ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**       #    ##    ##    #      ####     ####   ##       ##
//**
//**    $Id$
//**
//**    Copyright (C) 1999-2002 JÆnis Legzdi·ý
//**
//**    This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**    This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************
//**	
//**    Handles WAD file header, directory, lump I/O.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "fwaddefs.h"

// MACROS ------------------------------------------------------------------

#define MAX_WAD_FILES			32

#define GET_LUMP_FILE(num)		wad_files[num >> 16]
#define FILE_INDEX(num)			(num >> 16)
#define LUMP_INDEX(num)			(num & 0xffff)
#define MAKE_HANDLE(wi, num)	((wi << 16) + num)

// TYPES -------------------------------------------------------------------

struct lumpinfo_t
{
	char			name[12];
	int				position;
	int				size;
	EWadNamespace	Namespace;
};

class WadFile
{
public:
	char Name[MAX_OSPATH];
	int Handle;
	int NumLumps;
	lumpinfo_t *LumpInfo;	// Location of each lump on disk.
	void **LumpCache;
	const char* GwaDir;

	WadFile(void) : Handle(-1),
		NumLumps(0), LumpInfo(NULL), LumpCache(NULL)
	{
	}
	void Open(const char *filename, const char* AGwaDir, bool FixVoices);
	void OpenSingleLump(const char *filename);
	void CloseFile(void);
	bool CanClose(void);
	void Close(void);
	int CheckNumForName(const char* name, EWadNamespace NS);
	void ReadLump(int lump, void* dest);
	void ReadFromLump(int lump, void* dest, int pos, int size);
	void* CacheLumpNum(int lump, int tag);
	void InitNamespaces();
	void FixVoiceNamespaces();
	void InitNamespace(EWadNamespace NS, const char* Start, const char* End,
		bool AltMarkers);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

bool GLBSP_BuildNodes(const char *name, const char* gwafile);
void GLVis_BuildPVS(const char *srcfile, const char* gwafile);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int				AuxiliaryIndex;

static int				num_wad_files = 0;
static WadFile			wad_files[MAX_WAD_FILES];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  W_CleanupName
//
//==========================================================================

void W_CleanupName(const char *src, char *dst)
{
	guard(W_CleanupName);
	int i;
	for (i = 0; i < 8 && src[i]; i++)
	{
		dst[i] = toupper(src[i]);
	}
	for (; i < 9; i++)
	{
		dst[i] = 0;
	}
	unguard;
}

//==========================================================================
//
//	WadFile::Open
//
//==========================================================================

void WadFile::Open(const char *filename, const char* AGwaDir, bool FixVoices)
{
	guard(WadFile::Open);
	wadinfo_t		header;
	lumpinfo_t*		lump_p;
	int				i;
	int				length;
	filelump_t*		fileinfo;
	filelump_t*		fi_p;

	strcpy(Name, filename);
	GwaDir = AGwaDir;

	// open the file and add to directory
	Handle = Sys_FileOpenRead(filename);
	if (Handle == -1)
	{
		Sys_Error("Couldn't open %s", filename);
	}
	GCon->Logf(NAME_Init, "adding %s", filename);

	// WAD file
	Sys_FileRead(Handle, &header, sizeof(header));
	if (strncmp(header.identification, "IWAD", 4))
	{
		// Homebrew levels?
		if (strncmp(header.identification, "PWAD", 4))
		{
			Sys_Error ("Wad file %s doesn't have IWAD "
		 		"or PWAD id\n", filename);
		}
	}
	header.numlumps = LittleLong(header.numlumps);
	header.infotableofs = LittleLong(header.infotableofs);
	NumLumps = header.numlumps;
	//	Moved here to make static data less fragmented
	LumpInfo = Z_New<lumpinfo_t>(NumLumps);
	length = header.numlumps * sizeof(filelump_t);
	fi_p = fileinfo = (filelump_t*)Z_Malloc(length, PU_STATIC, 0);
	Sys_FileSeek(Handle, header.infotableofs);
	Sys_FileRead(Handle, fileinfo, length);

	// Fill in lumpinfo
	lump_p = LumpInfo;

	for (i = 0; i < NumLumps; i++, lump_p++, fileinfo++)
	{
		W_CleanupName(fileinfo->name, lump_p->name);
		lump_p->position = LittleLong(fileinfo->filepos);
		lump_p->size = LittleLong(fileinfo->size);
		lump_p->Namespace = WADNS_Global;
	}
	
	Z_Free(fi_p);

	//	Set up namespaces.
	InitNamespaces();

	if (FixVoices)
	{
		FixVoiceNamespaces();
	}

	// set up caching
	LumpCache = Z_CNew<void*>(NumLumps);
	unguard;
}

//==========================================================================
//
//	WadFile::OpenSingleLump
//
//==========================================================================

void WadFile::OpenSingleLump(const char *filename)
{
	guard(WadFile::OpenSingleLump);
	// open the file and add to directory
	Handle = Sys_FileOpenRead(filename);
	if (Handle == -1)
	{
		Sys_Error("Couldn't open %s", filename);
	}
	GCon->Logf(NAME_Init, "adding %s", filename);

	strcpy(Name, filename);
	GwaDir = NULL;

	// single lump file
	NumLumps = 1;
	LumpInfo = Z_New<lumpinfo_t>();

	// Fill in lumpinfo
	FL_ExtractFileBase(filename, LumpInfo->name);
	W_CleanupName(LumpInfo->name, LumpInfo->name);
	LumpInfo->position = 0;
	LumpInfo->size = Sys_FileSize(Handle);
	
	// set up caching
	LumpCache = Z_CNew<void*>();
	unguard;
}

//==========================================================================
//
//	WadFile::CloseFile
//
//==========================================================================

void WadFile::CloseFile()
{
	guard(WadFile::CloseFile);
	if (Handle > 0)
	{
		Sys_FileClose(Handle);
		Handle = -1;
	}
	unguard;
}

//==========================================================================
//
//	WadFile::CanClose
//
//==========================================================================

bool WadFile::CanClose()
{
	guard(WadFile::CanClose);
	for (int i = 0; i < NumLumps; i++)
	{
		if (LumpCache[i])
		{
			return false;
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	WadFile::Close
//
//==========================================================================

void WadFile::Close()
{
	guard(WadFile::Close);
	if (LumpCache)
	{
		for (int i = 0; i < NumLumps; i++)
		{
			if (LumpCache[i])
			{
				Z_Free(LumpCache[i]);
			}
		}
		Z_Free(LumpCache);
		LumpCache = NULL;
	}
	if (LumpInfo)
	{
		Z_Free(LumpInfo);
		LumpInfo = NULL;
	}
	NumLumps = 0;
	Name[0] = 0;
	CloseFile();
	unguard;
}

//==========================================================================
//
//  W_AddFile
//
//  All files are optional, but at least one file must be found (PWAD, if
// all required lumps are present). Files with a .wad extension are wadlink
// files with multiple lumps. Other files are single lumps with the base
// filename for the lump name.
//
//==========================================================================

void W_AddFile(const char *filename, const char* gwadir, bool FixVoices)
{
	guard(W_AddFile);
	int wadtime;
	char ext[8];

	wadtime = Sys_FileTime(filename);
	if (wadtime == -1)
	{
		Sys_Error("Required file %s doesn't exist", filename);
	}

	FL_ExtractFileExtension(filename, ext);
	if (stricmp(ext, "wad") && stricmp(ext, "gwa"))
	{
		wad_files[num_wad_files].OpenSingleLump(filename);
	}
	else
	{
		wad_files[num_wad_files].Open(filename, gwadir, FixVoices);
	}

	num_wad_files++;

	if (!stricmp(ext, "wad"))
	{
		char gl_name[1024];

		bool FoundGwa = false;
		if (gwadir)
		{
			strcpy(gl_name, gwadir);
			strcat(gl_name, "/");
			FL_ExtractFileName(filename, gl_name + strlen(gl_name));
			FL_StripExtension(gl_name);
			strcat(gl_name, ".gwa");
			if (Sys_FileTime(gl_name) >= wadtime)
			{
				W_AddFile(gl_name, NULL, false);
				FoundGwa = true;
			}
		}

		if (!FoundGwa)
		{
			strcpy(gl_name, filename);
			FL_StripExtension(gl_name);
			strcat(gl_name, ".gwa");
			if (Sys_FileTime(gl_name) >= wadtime)
			{
				W_AddFile(gl_name, NULL, false);
			}
			else
			{
				//	Leave empty slot for GWA file
				num_wad_files++;
			}
		}
	}
	unguard;
}

//==========================================================================
//
//  W_OpenAuxiliary
//
//==========================================================================

void W_OpenAuxiliary(const char *filename)
{
	guard(W_OpenAuxiliary);
	W_CloseAuxiliary();

	AuxiliaryIndex = num_wad_files;

	W_AddFile(filename, NULL, false);
	unguard;
}

//==========================================================================
//
//  W_CloseAuxiliary
//
//==========================================================================

void W_CloseAuxiliary()
{
	guard(W_CloseAuxiliary);
	if (AuxiliaryIndex)
	{
		wad_files[AuxiliaryIndex].Close();
		wad_files[AuxiliaryIndex + 1].Close();
		num_wad_files = AuxiliaryIndex;
		AuxiliaryIndex = 0;
	}
	unguard;
}

//==========================================================================
//
//  W_CloseAuxiliaryFile
//
//  WARNING: W_CloseAuxiliary() must be called before any further
// auxiliary lump processing.
//
//==========================================================================

void W_CloseAuxiliaryFile()
{
	guard(W_CloseAuxiliaryFile);
	wad_files[AuxiliaryIndex].CloseFile();
	wad_files[AuxiliaryIndex + 1].CloseFile();
	unguard;
}

#ifdef CLIENT

//==========================================================================
//
//	W_BuildGLNodes
//
//==========================================================================

void W_BuildGLNodes(int lump)
{
	guard(W_BuildGLNodes);
	int fi = FILE_INDEX(lump);

	char gwaname[MAX_OSPATH];
	if (wad_files[fi].GwaDir)
	{
		FL_CreatePath(wad_files[fi].GwaDir);
		strcpy(gwaname, wad_files[fi].GwaDir);
		strcat(gwaname, "/");
		FL_ExtractFileName(wad_files[fi].Name, gwaname + strlen(gwaname));
	}
	else
		strcpy(gwaname, wad_files[fi].Name);
	FL_StripExtension(gwaname);
	strcat(gwaname, ".gwa");

	// Build GL nodes
	if (!GLBSP_BuildNodes(wad_files[fi].Name, gwaname))
	{
		Sys_Error("Node build failed");
	}

	// Build PVS
	GLVis_BuildPVS(wad_files[fi].Name, gwaname);

	// Add GWA file
	wad_files[fi + 1].Open(gwaname, NULL, false);
	unguard;
}

//==========================================================================
//
//	W_BuildPVS
//
//==========================================================================

void W_BuildPVS(int lump, int gllump)
{
	guard(W_BuildPVS);
	int fi = FILE_INDEX(lump);
	char name[MAX_OSPATH];
	strcpy(name, wad_files[fi].Name);

	int glfi = FILE_INDEX(gllump);
	char glname[MAX_OSPATH];
	strcpy(glname, wad_files[glfi].Name);

	if (!wad_files[glfi].CanClose())
	{
		GCon->Logf("Can't close %s, some lumps are in use", glname);
		GCon->Log("PVS build not performed");
		return;
	}

	// Close old file
	wad_files[glfi].Close();

	// Build PVS
	GLVis_BuildPVS(name, fi != glfi ? glname : NULL);

	// Add GWA file
	wad_files[glfi].Open(glname, NULL, false);
	unguard;
}

#endif

//==========================================================================
//
//  WadFile::CheckNumForName
//
//  Returns -1 if name not found.
//
//==========================================================================

int WadFile::CheckNumForName(const char* name, EWadNamespace NS)
{
	guard(WadFile::CheckNumForName);
	for (int i = NumLumps - 1; i >= 0; i--)
	{
		if (LumpInfo[i].Namespace == NS && !strcmp(name, LumpInfo[i].name))
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
//  W_CheckNumForName
//
//  Returns -1 if name not found.
//
//==========================================================================

int W_CheckNumForName(const char* name, EWadNamespace NS)
{
	guard(W_CheckNumForName);
	char cleaned[12];
	W_CleanupName(name, cleaned);
	for (int wi = num_wad_files - 1; wi >= 0; wi--)
	{
		int i = wad_files[wi].CheckNumForName(cleaned, NS);
		if (i >= 0)
		{
			return MAKE_HANDLE(wi, i);
		}
	}

	// Not found.
	return -1;
	unguard;
}

//==========================================================================
//
//  W_GetNumForName
//
//  Calls W_CheckNumForName, but bombs out if not found.
//
//==========================================================================

int W_GetNumForName(const char* name, EWadNamespace NS)
{
	guard(W_GetNumForName);
	int	i;

	i = W_CheckNumForName(name, NS);
	
	if (i == -1)
	{
		Sys_Error("W_GetNumForName: %s not found!", name);
	}

	return i;
	unguard;
}

//==========================================================================
//
//  W_LumpLength
//
//  Returns the buffer size needed to load the given lump.
//
//==========================================================================

int W_LumpLength(int lump)
{
	guard(W_LumpLength);
	if (FILE_INDEX(lump) >= num_wad_files)
	{
		Sys_Error("W_LumpLength: %i >= num_wad_files", FILE_INDEX(lump));
	}
	WadFile &w = GET_LUMP_FILE(lump);
	int lumpindex = LUMP_INDEX(lump);
	return w.LumpInfo[lumpindex].size;
	unguard;
}

//==========================================================================
//
//  W_LumpName
//
//==========================================================================

const char *W_LumpName(int lump)
{
	guard(W_LumpName);
	static char empty_string[4] = "";
	if (FILE_INDEX(lump) >= num_wad_files)
	{
		return empty_string;
	}
	WadFile &w = GET_LUMP_FILE(lump);
	int lumpindex = LUMP_INDEX(lump);
	if (lumpindex >= w.NumLumps)
	{
		return empty_string;
	}
	return w.LumpInfo[lumpindex].name;
	unguard;
}

//==========================================================================
//
//  WadFile::ReadLump
//
//  Loads the lump into the given buffer, which must be >= W_LumpLength().
//
//==========================================================================

void WadFile::ReadLump(int lump, void* dest)
{
	guard(WadFile::ReadLump);
	if ((dword)lump >= (dword)NumLumps)
	{
		Sys_Error("WadFile::ReadLump: %i >= numlumps", lump);
	}

	lumpinfo_t &l = LumpInfo[lump];

	Sys_FileSeek(Handle, l.position);
	int c = Sys_FileRead(Handle, dest, l.size);

	if (c < l.size)
	{
		Sys_Error("W_ReadLump: only read %i of %i on lump %i",
			c, l.size, lump);
	}
	unguard;
}

//==========================================================================
//
//  W_ReadLump
//
//==========================================================================

void W_ReadLump(int lump, void* dest)
{
	guard(W_ReadLump);
	if (FILE_INDEX(lump) >= num_wad_files)
	{
		Sys_Error("W_ReadLump: %i >= num_wad_files", FILE_INDEX(lump));
	}

	WadFile &w = GET_LUMP_FILE(lump);
	w.ReadLump(LUMP_INDEX(lump), dest);
	unguard;
}

//==========================================================================
//
//  WadFile::ReadFromLump
//
//  Loads part of the lump into the given buffer.
//
//==========================================================================

void WadFile::ReadFromLump(int lump, void* dest, int pos, int size)
{
	guard(WadFile::ReadFromLump);
	if ((dword)lump >= (dword)NumLumps)
	{
		Sys_Error("WadFile::ReadFromLump: %i >= numlumps", lump);
	}

	lumpinfo_t &l = LumpInfo[lump];

	if (pos >= l.size)
	{
		return;
	}

	Sys_FileSeek(Handle, l.position + pos);
	Sys_FileRead(Handle, dest, size);
	unguard;
}

//==========================================================================
//
//  W_ReadFromLump
//
//==========================================================================

void W_ReadFromLump(int lump, void* dest, int pos, int size)
{
	guard(W_ReadFromLump);
	if (FILE_INDEX(lump) >= num_wad_files)
	{
		Sys_Error("W_ReadFromLump: %i >= num_wad_files", FILE_INDEX(lump));
	}

	WadFile &w = GET_LUMP_FILE(lump);
	w.ReadFromLump(LUMP_INDEX(lump), dest, pos, size);
	unguard;
}

//==========================================================================
//
//  WadFile::CacheLumpNum
//
//==========================================================================

void* WadFile::CacheLumpNum(int lump, int tag)
{
	guard(WadFile::CacheLumpNum);
	if ((unsigned)lump >= (unsigned)NumLumps)
	{
		Sys_Error("W_CacheLumpNum: %i >= numlumps", lump);
	}
		
	if (!LumpCache[lump])
	{
		// read the lump in
//		byte *ptr = (byte*)Z_Malloc(W_LumpLength(lump) + 1, tag, &lumpcache[lump]);
		byte *ptr = (byte*)Z_Malloc(LumpInfo[lump].size + 1, tag, &LumpCache[lump]);
		ReadLump(lump, LumpCache[lump]);
//		ptr[W_LumpLength(lump)] = 0;
		ptr[LumpInfo[lump].size] = 0;
	}
	else
	{
		Z_ChangeTag(LumpCache[lump], tag);
	}
	
	return LumpCache[lump];
	unguard;
}

//==========================================================================
//
//  W_CacheLumpNum
//
//==========================================================================

void* W_CacheLumpNum(int lump, int tag)
{
	guard(W_CacheLumpNum);
	if (FILE_INDEX(lump) >= num_wad_files)
	{
		Sys_Error("W_CacheLumpNum: %i >= num_wad_files", FILE_INDEX(lump));
	}

	WadFile &w = GET_LUMP_FILE(lump);
	int lumpindex = LUMP_INDEX(lump);
	return w.CacheLumpNum(lumpindex, tag);
	unguard;
}

//==========================================================================
//
//  W_CacheLumpName
//
//==========================================================================

void* W_CacheLumpName(const char* name,int tag, EWadNamespace NS)
{
	guard(W_CacheLumpName);
	return W_CacheLumpNum(W_GetNumForName(name, NS), tag);
	unguard;
}

//==========================================================================
//
//	VStreamLumpReader
//
//==========================================================================

class VStreamLumpReader : public VStream
{
public:
	VStreamLumpReader(byte* InData, int InSize)
		: Data(InData), Pos(0), Size(InSize)
	{
		bLoading = true;
	}
	~VStreamLumpReader(void)
	{
		if (Data)
			Close();
	}
	void Serialise(void* V, int Length)
	{
		if (Length > Size - Pos)
		{
			bError = true;
		}
		memcpy(V, Data + Pos, Length);
		Pos += Length;
	}
	int Tell(void)
	{
		return Pos;
	}
	int TotalSize(void)
	{
		return Size;
	}
	bool AtEnd(void)
	{
		return Pos >= Size;
	}
	void Seek(int InPos)
	{
		Pos = InPos;
	}
	bool Close(void)
	{
		Z_Free(Data);
		Data = NULL;
		return !bError;
	}

protected:
	byte *Data;
	int Pos;
	int Size;
};

//==========================================================================
//
//  W_CreateLumpReader
//
//==========================================================================

VStream* W_CreateLumpReader(int lump)
{
	guard(W_CreateLumpReader);
	return new VStreamLumpReader((byte *)W_CacheLumpNum(lump, PU_STATIC),
		W_LumpLength(lump));
	unguard;
}

//==========================================================================
//
//  W_CreateLumpReader
//
//==========================================================================

VStream* W_CreateLumpReader(const char* name, EWadNamespace NS)
{
	guard(W_CreateLumpReader);
	return W_CreateLumpReader(W_GetNumForName(name, NS));
	unguard;
}

//==========================================================================
//
//  W_ForEachLump
//
//==========================================================================

bool W_ForEachLump(bool (*func)(int, const char*, int, EWadNamespace))
{
	guard(W_ForEachLump);
	for (int wi = 0; wi < num_wad_files; wi++)
	{
		WadFile &w = wad_files[wi];
		for (int i = 0; i < w.NumLumps; i++)
		{
			if (!func(MAKE_HANDLE(wi, i), w.LumpInfo[i].name, 
				w.LumpInfo[i].size, w.LumpInfo[i].Namespace))
			{
				return false;
			}
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//  W_IterateNS
//
//==========================================================================

int W_IterateNS(int Prev, EWadNamespace NS)
{
	guard(W_IterateNS);
	int wi = FILE_INDEX((Prev + 1));
	int li = LUMP_INDEX((Prev + 1));
	for (; wi < num_wad_files; wi++, li = 0)
	{
		WadFile &w = wad_files[wi];
		for (; li < w.NumLumps; li++)
		{
			if (w.LumpInfo[li].Namespace == NS)
			{
				return MAKE_HANDLE(wi, li);
			}
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//  WadFile::InitNamespaces
//
//==========================================================================

void WadFile::InitNamespaces()
{
	guard(WadFile::InitNamespaces);
	InitNamespace(WADNS_Sprites, "S_START", "S_END", true);
	InitNamespace(WADNS_Flats, "F_START", "F_END", true);
	InitNamespace(WADNS_ColorMaps, "C_START", "C_END", true);
	InitNamespace(WADNS_ACSLibrary, "A_START", "A_END", true);
	InitNamespace(WADNS_NewTextures, "TX_START", "TX_END", false);
	InitNamespace(WADNS_Voices, "V_START", "V_END", true);
	InitNamespace(WADNS_HiResTextures, "H_START", "H_END", true);
	InitNamespace(WADNS_Progs, "PR_START", "PR_END", false);
	unguard;
}

//==========================================================================
//
//  WadFile::InitNamespace
//
//==========================================================================

void WadFile::InitNamespace(EWadNamespace NS, const char* Start,
	const char* End, bool AltMarkers)
{
	guard(WadFile::InitNamespace);
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
			if (!strcmp(L.name, End) || (AltMarkers && 
				L.name[0] == End[0] && !strcmp(L.name + 1, End)))
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
			if (!strcmp(L.name, Start) || (AltMarkers &&
				L.name[0] == Start[0] && !strcmp(L.name + 1, Start)))
			{
				InNS = true;
			}
		}
	}
	unguard;
}

//==========================================================================
//
//  WadFile::FixVoiceNamespaces
//
//==========================================================================

void WadFile::FixVoiceNamespaces()
{
	guard(WadFile::FixVoiceNamespaces);
	for (int i = 0; i < NumLumps; i++)
	{
		lumpinfo_t& L = LumpInfo[i];

		//	Skip if lump is already in other namespace.
		if (L.Namespace != WADNS_Global)
			continue;

		if (L.name[0] == 'V' && L.name[1] == 'O' && L.name[2] == 'C' &&
			L.name[3] >= '0' && L.name[3] <= '9' &&
			(L.name[4] == 0 || (L.name[4] >= '0' && L.name[4] <= '9' &&
			(L.name[5] == 0 || (L.name[5] >= '0' && L.name[5] <= '9' &&
			(L.name[6] == 0 || (L.name[6] >= '0' && L.name[6] <= '9' &&
			(L.name[7] == 0 || (L.name[7] >= '0' && L.name[7] <= '9')))))))))
		{
			L.Namespace = WADNS_Voices;
		}
	}
	unguard;
}

//==========================================================================
//
//  W_Profile
//
//==========================================================================

#if 0
void W_Profile(void)
{
	static int	info[2500][10];
	static int	profilecount = 0;
	int			i;
	memblock_t*	block;
	void*		ptr;
	char		ch;
	FILE*		f;
	int			j;
	char		name[16];
	
	sprintf(name,"jl/waddump%d.txt", profilecount);
	
	for (i = 0; i < numlumps; i++)
	{	
		ptr = lumpcache[i];
		if (!ptr)
		{
			ch = ' ';
			continue;
		}
		else
		{
			block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
			if (block->tag < PU_PURGELEVEL)
				ch = 'S';
			else
				ch = 'P';
		}
		info[i][profilecount] = ch;
	}
	profilecount++;

	f = fopen(name, "w");
	name[8] = 0;

	for (i=0 ; i<numlumps ; i++)
	{
		memcpy (name,lumpinfo[i].name,8);

		for (j=0 ; j<8 ; j++)
			if (!name[j])
				break;

		for ( ; j<8 ; j++)
			name[j] = ' ';

		fprintf (f,"%i %s %i ", i, name, lumpinfo[i].prev);

//		for (j=0 ; j<profilecount ; j++)
//			fprintf (f,"    %c",info[i][j]);

		fprintf (f,"\n");
	}
	fclose (f);
}
#endif

//**************************************************************************
//
//  $Log$
//  Revision 1.23  2006/02/22 20:33:51  dj_jl
//  Created stream class.
//
//  Revision 1.22  2006/01/29 20:41:30  dj_jl
//  On Unix systems use ~/.vavoom for generated files.
//
//  Revision 1.21  2005/11/24 20:07:36  dj_jl
//  Aded namespace for progs.
//
//  Revision 1.20  2005/11/06 15:28:40  dj_jl
//  Some cleanup.
//
//  Revision 1.19  2005/11/05 15:49:14  dj_jl
//  Putting Strife shareware voices in correct namespace.
//
//  Revision 1.18  2005/11/05 14:57:36  dj_jl
//  Putting Strife shareware voices in correct namespace.
//
//  Revision 1.17  2005/10/20 22:22:39  dj_jl
//  Fixed double destruction of reader archive.
//
//  Revision 1.16  2005/10/18 20:53:04  dj_jl
//  Implemented basic support for streamed music.
//
//  Revision 1.15  2005/05/26 16:55:43  dj_jl
//  New lump namespace iterator
//
//  Revision 1.14  2004/12/03 16:15:47  dj_jl
//  Implemented support for extended ACS format scripts, functions, libraries and more.
//
//  Revision 1.13  2004/11/23 12:43:10  dj_jl
//  Wad file lump namespaces.
//
//  Revision 1.12  2003/03/08 12:08:05  dj_jl
//  Beautification.
//
//  Revision 1.11  2002/07/23 16:29:56  dj_jl
//  Replaced console streams with output device class.
//
//  Revision 1.10  2002/05/18 16:56:35  dj_jl
//  Added FArchive and FOutputDevice classes.
//
//  Revision 1.9  2002/01/07 12:16:43  dj_jl
//  Changed copyright year
//
//  Revision 1.8  2001/10/08 17:34:57  dj_jl
//  A lots of small changes and cleanups
//
//  Revision 1.7  2001/09/25 17:07:06  dj_jl
//  Safe PVS build
//
//  Revision 1.6  2001/09/14 16:51:46  dj_jl
//  Object oriented wad files, added dynamic build of GWA file
//
//  Revision 1.5  2001/08/30 17:42:31  dj_jl
//  Using file times
//
//  Revision 1.4  2001/08/21 17:50:17  dj_jl
//  Made W_UseAuxiliary safe
//
//  Revision 1.3  2001/07/31 17:16:31  dj_jl
//  Just moved Log to the end of file
//
//  Revision 1.2  2001/07/27 14:27:54  dj_jl
//  Update with Id-s and Log-s, some fixes
//
//**************************************************************************
