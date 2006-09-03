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
//**    Copyright (C) 1999-2006 Jānis Legzdiņš
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

#define GET_LUMP_FILE(num)		SearchPaths[num >> 16]
#define FILE_INDEX(num)			(num >> 16)
#define LUMP_INDEX(num)			(num & 0xffff)
#define MAKE_HANDLE(wi, num)	((wi << 16) + num)

// TYPES -------------------------------------------------------------------

struct lumpinfo_t
{
	VName			Name;
	vint32			Position;
	vint32			Size;
	EWadNamespace	Namespace;
};

class VWadFile : public VSearchPath
{
private:
	VStr		Name;
	VStream*	Stream;
	int			NumLumps;
	lumpinfo_t*	LumpInfo;	// Location of each lump on disk.
	VStr		GwaDir;

public:
	VWadFile() : Stream(NULL), NumLumps(0), LumpInfo(NULL)
	{
	}
	~VWadFile()
	{
		Close();
	}
	void Open(const VStr&, const VStr&, bool, VStream*);
	void OpenSingleLump(const VStr& FileName);
	void Close();
	int CheckNumForName(VName LumpName, EWadNamespace NS);
	void ReadFromLump(int lump, void* dest, int pos, int size);
	void InitNamespaces();
	void FixVoiceNamespaces();
	void InitNamespace(EWadNamespace NS, VName Start, VName End,
		VName AltStart = NAME_None, VName AltEnd = NAME_None);
	int LumpLength(int);
	VName LumpName(int);
	int IterateNS(int, EWadNamespace);
	void BuildGLNodes(VSearchPath*);
	void BuildPVS(VSearchPath*);
	VStr FindFile(const VStr&);
	bool FileExists(const VStr&);
	VStream* OpenFileRead(const VStr&);
	VStream* CreateLumpReaderNum(int);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

bool GLBSP_BuildNodes(const char *name, const char* gwafile);
void GLVis_BuildPVS(const char *srcfile, const char* gwafile);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern TArray<VStr>			wadfiles;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int					AuxiliaryIndex;

// CODE --------------------------------------------------------------------

VSearchPath::~VSearchPath()
{
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

void W_AddFile(const VStr& FileName, const VStr& GwaDir, bool FixVoices)
{
	guard(W_AddFile);
	int wadtime;

	wadtime = Sys_FileTime(FileName);
	if (wadtime == -1)
	{
		Sys_Error("Required file %s doesn't exist", *FileName);
	}

	wadfiles.Append(FileName);

	VStr ext = FileName.ExtractFileExtension().ToLower();
	VWadFile* Wad = new VWadFile;
	if (ext != "wad" && ext != "gwa")
	{
		Wad->OpenSingleLump(FileName);
	}
	else
	{
		Wad->Open(FileName, GwaDir, FixVoices, NULL);
	}
	SearchPaths.Append(Wad);

	if (ext == "wad")
	{
		VStr gl_name;

		bool FoundGwa = false;
		if (GwaDir)
		{
			gl_name = GwaDir + "/" +
				FileName.ExtractFileName().StripExtension() + ".gwa";
			if (Sys_FileTime(gl_name) >= wadtime)
			{
				W_AddFile(gl_name, VStr(), false);
				FoundGwa = true;
			}
		}

		if (!FoundGwa)
		{
			gl_name = FileName.StripExtension() + ".gwa";
			if (Sys_FileTime(gl_name) >= wadtime)
			{
				W_AddFile(gl_name, VStr(), false);
			}
			else
			{
				//	Leave empty slot for GWA file
				SearchPaths.Append(new VWadFile);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//  W_AddFileFromZip
//
//==========================================================================

void W_AddFileFromZip(const VStr& WadName, VStream* WadStrm,
	const VStr& GwaName, VStream* GwaStrm)
{
	guard(W_AddFileFromZip);
	//	Add WAD file.
	wadfiles.Append(WadName);
	VWadFile* Wad = new VWadFile;
	Wad->Open(WadName, VStr(), false, WadStrm);
	SearchPaths.Append(Wad);

	if (GwaStrm)
	{
		//	Add GWA file
		wadfiles.Append(GwaName);
		VWadFile* Gwa = new VWadFile;
		Gwa->Open(GwaName, VStr(), false, GwaStrm);
		SearchPaths.Append(Gwa);
	}
	else
	{
		//	Leave empty slot for GWA file
		SearchPaths.Append(new VWadFile);
	}
	unguard;
}

//==========================================================================
//
//  W_OpenAuxiliary
//
//==========================================================================

void W_OpenAuxiliary(const VStr& FileName)
{
	guard(W_OpenAuxiliary);
	W_CloseAuxiliary();

	AuxiliaryIndex = SearchPaths.Num();

	W_AddFile(FileName, VStr(), false);
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
		SearchPaths[AuxiliaryIndex]->Close();
		SearchPaths[AuxiliaryIndex + 1]->Close();
		delete SearchPaths[AuxiliaryIndex];
		delete SearchPaths[AuxiliaryIndex + 1];
		SearchPaths.SetNum(AuxiliaryIndex);
		AuxiliaryIndex = 0;
	}
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
	SearchPaths[FILE_INDEX(lump)]->BuildGLNodes(SearchPaths[FILE_INDEX(lump) + 1]);
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
	SearchPaths[FILE_INDEX(gllump)]->BuildPVS(SearchPaths[FILE_INDEX(lump)]);
	unguard;
}

#endif

//==========================================================================
//
//  W_CheckNumForName
//
//  Returns -1 if name not found.
//
//==========================================================================

int W_CheckNumForName(VName Name, EWadNamespace NS)
{
	guard(W_CheckNumForName);
	for (int wi = SearchPaths.Num() - 1; wi >= 0; wi--)
	{
		int i = SearchPaths[wi]->CheckNumForName(Name, NS);
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

int W_GetNumForName(VName Name, EWadNamespace NS)
{
	guard(W_GetNumForName);
	int i = W_CheckNumForName(Name, NS);
	if (i == -1)
	{
		Sys_Error("W_GetNumForName: %s not found!", *Name);
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
	if (FILE_INDEX(lump) >= SearchPaths.Num())
	{
		Sys_Error("W_LumpLength: %i >= num_wad_files", FILE_INDEX(lump));
	}
	VSearchPath* w = GET_LUMP_FILE(lump);
	int lumpindex = LUMP_INDEX(lump);
	return w->LumpLength(lumpindex);
	unguard;
}

//==========================================================================
//
//  W_LumpName
//
//==========================================================================

VName W_LumpName(int lump)
{
	guard(W_LumpName);
	if (FILE_INDEX(lump) >= SearchPaths.Num())
	{
		return NAME_None;
	}
	VSearchPath* w = GET_LUMP_FILE(lump);
	int lumpindex = LUMP_INDEX(lump);
	return w->LumpName(lumpindex);
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
	if (FILE_INDEX(lump) >= SearchPaths.Num())
	{
		Sys_Error("W_ReadFromLump: %i >= num_wad_files", FILE_INDEX(lump));
	}

	VSearchPath* w = GET_LUMP_FILE(lump);
	w->ReadFromLump(LUMP_INDEX(lump), dest, pos, size);
	unguard;
}

//==========================================================================
//
//  W_CreateLumpReaderNum
//
//==========================================================================

VStream* W_CreateLumpReaderNum(int lump)
{
	guard(W_CreateLumpReaderNum);
	return GET_LUMP_FILE(lump)->CreateLumpReaderNum(LUMP_INDEX(lump));
	unguard;
}

//==========================================================================
//
//  W_CreateLumpReaderName
//
//==========================================================================

VStream* W_CreateLumpReaderName(VName Name, EWadNamespace NS)
{
	guard(W_CreateLumpReaderName);
	return W_CreateLumpReaderNum(W_GetNumForName(Name, NS));
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
	for (; wi < SearchPaths.Num(); wi++, li = 0)
	{
		li = SearchPaths[wi]->IterateNS(li, NS);
		if (li != -1)
		{
			return MAKE_HANDLE(wi, li);
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//  W_Profile
//
//==========================================================================

#if 0
void W_Profile()
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

//==========================================================================
//
//  W_Shutdown
//
//==========================================================================

void W_Shutdown()
{
	guard(W_Shutdown);
	for (int i = 0; i < SearchPaths.Num(); i++)
	{
		delete SearchPaths[i];
	}
	SearchPaths.Clear();
	unguard;
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
	InitNamespace(WADNS_ColorMaps, NAME_c_start, NAME_c_end, NAME_cc_start, NAME_cc_end);
	InitNamespace(WADNS_ACSLibrary, NAME_a_start, NAME_a_end, NAME_aa_start, NAME_aa_end);
	InitNamespace(WADNS_NewTextures, NAME_tx_start, NAME_tx_end);
	InitNamespace(WADNS_Voices, NAME_v_start, NAME_v_end, NAME_vv_start, NAME_vv_end);
	InitNamespace(WADNS_HiResTextures, NAME_h_start, NAME_h_end, NAME_hh_start, NAME_hh_end);
	InitNamespace(WADNS_Progs, NAME_pr_start, NAME_pr_end);
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
	Stream->Seek(l.Position);
	Stream->Serialise(ptr, l.Size);

	//	Create stream.
	VStream* S = new VMemoryStream(ptr, l.Size);
	Z_Free(ptr);
	return S;
	unguard;
}

VStr VWadFile::FindFile(const VStr&)
{
	return VStr();
}

bool VWadFile::FileExists(const VStr&)
{
	return false;
}

VStream* VWadFile::OpenFileRead(const VStr&)
{
	return NULL;
}
