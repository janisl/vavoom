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
//**
//**	This file includes code from ZDoom, copyright 1998-2004 Randy Heit,
//**  all rights reserved, with the following licence:
//**
//** Redistribution and use in source and binary forms, with or without
//** modification, are permitted provided that the following conditions
//** are met:
//**
//** 1. Redistributions of source code must retain the above copyright
//**    notice, this list of conditions and the following disclaimer.
//** 2. Redistributions in binary form must reproduce the above copyright
//**    notice, this list of conditions and the following disclaimer in the
//**    documentation and/or other materials provided with the distribution.
//** 3. The name of the author may not be used to endorse or promote products
//**    derived from this software without specific prior written permission.
//**
//** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
//** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
//** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
//** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
#include "p_acs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

//
//	Internal engine limits
//
enum
{
	MAX_ACS_SCRIPT_VARS	= 20,
	MAX_ACS_MAP_VARS	= 128,
};

enum EAcsFormat
{
	ACS_Old,
	ACS_Enhanced,
	ACS_LittleEnhanced,
	ACS_Unknown
};

//	Script flags.
enum
{
	SCRIPTF_Net = 0x0001	//	Safe to "puke" in multiplayer.
};

struct VAcsHeader
{
	char		Marker[4];
	vint32		InfoOffset;
	vint32		Code;
};

struct VAcsInfo
{
	vuint16		Number;
	vuint8		Type;
	vuint8		ArgCount;
	vuint8*		Address;
	vuint16		Flags;
	vuint16		VarCount;
	VAcs*		RunningScript;
};

struct VAcsFunction
{
	vuint8		ArgCount;
	vuint8		LocalCount;
	vuint8		HasReturnValue;
	vuint8		ImportNum;
	vuint32		Address;
};

//
//	A action code scripts object module - level's BEHAVIOR lump or library.
//
class VAcsObject
{
private:
	friend class VAcsLevel;

	struct VArrayInfo
	{
		vint32		Size;
		vint32*		Data;
	};

	EAcsFormat			Format;

	vint32				LumpNum;
	vint32				LibraryID;

	vint32				DataSize;
	vuint8*				Data;

	vuint8*				Chunks;

	vint32				NumScripts;
	VAcsInfo*			Scripts;

	VAcsFunction*		Functions;
	vint32				NumFunctions;

	vint32				NumStrings;
	char**				Strings;

	vint32				MapVarStore[MAX_ACS_MAP_VARS];

	vint32				NumArrays;
	VArrayInfo*			ArrayStore;
	vint32				NumTotalArrays;
	VArrayInfo**		Arrays;

	TArray<VAcsObject*>	Imports;

	void LoadOldObject();
	void LoadEnhancedObject();
	void UnencryptStrings();
	int FindFunctionName(const char* Name) const;
	int FindMapVarName(const char* Name) const;
	int FindMapArray(const char* Name) const;
	int FindStringInChunk(vuint8* Chunk, const char* Name) const;
	vuint8* FindChunk(const char* id) const;
	vuint8* NextChunk(vuint8* prev) const;
	void Serialise(VStream& Strm);
	void StartTypedACScripts(int Type);

public:
	VAcsLevel*			Level;
	vint32*				MapVars[MAX_ACS_MAP_VARS];

	VAcsObject(VAcsLevel* ALevel, int Lump);
	~VAcsObject();

	vuint8* OffsetToPtr(int);
	int PtrToOffset(vuint8*);
	EAcsFormat GetFormat() const
	{
		return Format;
	}
	int GetNumScripts() const
	{
		return NumScripts;
	}
	VAcsInfo& GetScriptInfo(int i)
	{
		return Scripts[i];
	}
	const char* GetString(int i) const
	{
		return Strings[i];
	}
	int GetLibraryID() const
	{
		return LibraryID;
	}
	VAcsInfo* FindScript(int Number) const;
	VAcsFunction* GetFunction(int funcnum, VAcsObject*& Object);
	int GetArrayVal(int ArrayIdx, int Index);
	void SetArrayVal(int ArrayIdx, int Index, int Value);
};

struct VAcsCallReturn
{
	int				ReturnAddress;
	VAcsFunction*	ReturnFunction;
	VAcsObject*		ReturnObject;
	vuint8			bDiscardResult;
	vuint8			Pad[3];
};

class VAcs : public VThinker
{
	DECLARE_CLASS(VAcs, VThinker, 0)
	NO_DEFAULT_CONSTRUCTOR(VAcs)

	enum
	{
		ASTE_Running,
		ASTE_Suspended,
		ASTE_WaitingForTag,
		ASTE_WaitingForPoly,
		ASTE_WaitingForScriptStart,
		ASTE_WaitingForScript,
		ASTE_Terminating
	};

	VEntity*		Activator;
	line_t*			line;
	vint32 			side;
	vint32 			number;
	VAcsInfo*		info;
	vuint8			State;
	float			DelayTime;
	vint32			WaitValue;
	vint32*			LocalVars;
	vuint8*			InstructionPointer;
	VAcsObject*		ActiveObject;

	void Destroy();
	void Serialise(VStream&);
	int RunScript(float);
	void Tick(float);

private:
	enum { ACS_STACK_DEPTH		= 4096 };

	enum EScriptAction
	{
		SCRIPT_Continue,
		SCRIPT_Stop,
		SCRIPT_Terminate,
	};

	//
	//	Constants used by scripts.
	//

	enum EGameMode
	{
		GAME_SINGLE_PLAYER,
		GAME_NET_COOPERATIVE,
		GAME_NET_DEATHMATCH,
		GAME_TITLE_MAP
	};

	enum ETexturePosition
	{
		TEXTURE_TOP,
		TEXTURE_MIDDLE,
		TEXTURE_BOTTOM
	};

	enum
	{
		BLOCK_NOTHING,
		BLOCK_CREATURES,
		BLOCK_EVERYTHING,
		BLOCK_RAILING
	};

	enum
	{
		LEVELINFO_PAR_TIME,
		LEVELINFO_CLUSTERNUM,
		LEVELINFO_LEVELNUM,
		LEVELINFO_TOTAL_SECRETS,
		LEVELINFO_FOUND_SECRETS,
		LEVELINFO_TOTAL_ITEMS,
		LEVELINFO_FOUND_ITEMS,
		LEVELINFO_TOTAL_MONSTERS,
		LEVELINFO_KILLED_MONSTERS,
		LEVELINFO_SUCK_TIME
	};

	//	Flags for ReplaceTextures
	enum
	{
		NOT_BOTTOM			= 1,
		NOT_MIDDLE			= 2,
		NOT_TOP				= 4,
		NOT_FLOOR			= 8,
		NOT_CEILING			= 16,
	};

	const char* GetStr(int Index)
	{
		return ActiveObject->Level->GetString(Index);
	}

	VEntity* EntityFromTID(int TID, VEntity* Default)
	{
		if (!TID)
		{
			return Default;
		}
		else
		{
			int search = -1;
			return Level->eventFindMobjFromTID(TID, &search);
		}
	}
	int FindSectorFromTag(int, int);
	void GiveInventory(VEntity*, const char*, int);
	void TakeInventory(VEntity*, const char*, int);
	int CheckInventory(VEntity*, const char*);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, Acs)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAcsObject::VAcsObject
//
//==========================================================================

VAcsObject::VAcsObject(VAcsLevel* ALevel, int Lump)
: Level(ALevel)
{
	guard(VAcsObject::VAcsObject);
	Format = ACS_Unknown;
	LumpNum = Lump;
	LibraryID = 0;
	DataSize = 0;
	Data = NULL;
	Chunks = NULL;
	NumScripts = 0;
	Scripts = NULL;
	NumFunctions = 0;
	Functions = NULL;
	NumStrings = 0;
	Strings = NULL;
	NumArrays = 0;
	ArrayStore = NULL;
	NumTotalArrays = 0;
	Arrays = NULL;
	memset(MapVarStore, 0, sizeof(MapVarStore));

	if (Lump < 0)
    {
		return;
    }
	if (W_LumpLength(Lump) < (int)sizeof(VAcsHeader))
    {
		GCon->Log("Behavior lump too small");
		return;
    }

	VStream* Strm = W_CreateLumpReaderNum(Lump);
	Data = new vuint8[Strm->TotalSize()];
	Strm->Serialise(Data, Strm->TotalSize());
	delete Strm;
	VAcsHeader* header = (VAcsHeader*)Data;

	//	Check header.
	if (header->Marker[0] != 'A' || header->Marker[1] != 'C' ||
		header->Marker[2] != 'S')
	{
		return;
	}
	//	Determine format.
	switch (header->Marker[3])
	{
	case 0:
		Format = ACS_Old;
		break;
	case 'E':
		Format = ACS_Enhanced;
		break;
	case 'e':
		Format = ACS_LittleEnhanced;
		break;
	default:
		return;
	}

	DataSize = W_LumpLength(Lump);

	if (Format == ACS_Old)
	{
		vuint32 dirofs = LittleLong(header->InfoOffset);
		vuint8* pretag = Data + dirofs - 4;

		Chunks = Data + DataSize;
		//	Check for redesigned ACSE/ACSe
		if (dirofs >= 6 * 4 && pretag[0] == 'A' &&
			pretag[1] == 'C' && pretag[2] == 'S' &&
			(pretag[3] == 'e' || pretag[3] == 'E'))
		{
			Format = (pretag[3] == 'e') ? ACS_LittleEnhanced : ACS_Enhanced;
			Chunks = Data + LittleLong(*(int*)(Data + dirofs - 8));
			//	Forget about the compatibility cruft at the end of the lump
			DataSize = dirofs - 8;
		}
	}
	else
	{
		Chunks = Data + LittleLong(header->InfoOffset);
	}

	if (Format == ACS_Old)
	{
		LoadOldObject();
	}
	else
	{
		LoadEnhancedObject();
	}
	unguard;
}

//==========================================================================
//
//	VAcsObject::~VAcsObject
//
//==========================================================================

VAcsObject::~VAcsObject()
{
	guard(VAcsObject::~VAcsObject);
	delete[] Scripts;
	delete[] Strings;
	for (int i = 0; i < NumArrays; i++)
		delete[] ArrayStore[i].Data;
	if (ArrayStore)
		delete[] ArrayStore;
	if (Arrays)
		delete[] Arrays;
	delete[] Data;
	unguard;
}

//==========================================================================
//
//	VAcsObject::LoadOldObject
//
//==========================================================================

void VAcsObject::LoadOldObject()
{
	guard(VAcsObject::LoadOldObject);
	int i;
	int *buffer;
	VAcsInfo *info;
	VAcsHeader *header;

	header = (VAcsHeader*)Data;

	//	Load script info.
	buffer = (int*)(Data + LittleLong(header->InfoOffset));
	NumScripts = LittleLong(*buffer++);
	if (NumScripts == 0)
	{
		//	Empty behavior lump
		return;
	}
	Scripts = new VAcsInfo[NumScripts];
	memset(Scripts, 0, NumScripts * sizeof(VAcsInfo));
	for (i = 0, info = Scripts; i < NumScripts; i++, info++)
	{
		info->Number = LittleLong(*buffer) % 1000;
		info->Type = LittleLong(*buffer) / 1000;
		buffer++;
		info->Address = OffsetToPtr(LittleLong(*buffer++));
		info->ArgCount = LittleLong(*buffer++);
		info->Flags = 0;
		info->VarCount = MAX_ACS_SCRIPT_VARS;
	}

	//	Load strings.
	NumStrings = LittleLong(*buffer++);
	Strings = new char*[NumStrings];
	for (i = 0; i < NumStrings; i++)
	{
		Strings[i] = (char*)Data + LittleLong(buffer[i]);
	}

	//	Set up map vars.
	memset(MapVarStore, 0, sizeof(MapVarStore));
	for (i = 0; i < MAX_ACS_MAP_VARS; i++)
	{
		MapVars[i] = &MapVarStore[i];
	}

	//	Add to loaded objects.
	LibraryID = Level->LoadedObjects.Append(this) << 16;
	unguard;
}

//==========================================================================
//
//	VAcsObject::LoadEnhancedObject
//
//==========================================================================

void VAcsObject::LoadEnhancedObject()
{
	guard(VAcsObject::LoadEnhancedObject);
	int i;
	int *buffer;
	VAcsInfo *info;

	//	Load scripts.
	buffer = (int*)FindChunk("SPTR");
	if (Data[3] != 0)
	{
		NumScripts = LittleLong(buffer[1]) / 12;
		Scripts = new VAcsInfo[NumScripts];
		memset(Scripts, 0, NumScripts * sizeof(VAcsInfo));
		buffer += 2;

		for (i = 0, info = Scripts; i < NumScripts; i++, info++)
		{
			info->Number = LittleShort(*(short*)buffer);
			info->Type = LittleShort(((short*)buffer)[1]);
			buffer++;
			info->Address = OffsetToPtr(LittleLong(*buffer++));
			info->ArgCount = LittleLong(*buffer++);
			info->Flags = 0;
			info->VarCount = MAX_ACS_SCRIPT_VARS;
		}
	}
	else
	{
		NumScripts = LittleLong(buffer[1]) / 8;
		Scripts = new VAcsInfo[NumScripts];
		memset(Scripts, 0, NumScripts * sizeof(VAcsInfo));
		buffer += 2;

		for (i = 0, info = Scripts; i < NumScripts; i++, info++)
		{
			info->Number = LittleShort(*(short*)buffer);
			info->Type = ((vuint8*)buffer)[2];
			info->ArgCount = ((vuint8*)buffer)[3];
			buffer++;
			info->Address = OffsetToPtr(LittleLong(*buffer++));
			info->Flags = 0;
			info->VarCount = MAX_ACS_SCRIPT_VARS;
		}
	}

	//	Load script flags.
	buffer = (int*)FindChunk("SFLG");
	if (buffer)
	{
		int count = LittleLong(buffer[1]) / 4;
		buffer += 2;
		for (int i = 0; i < count; i++, buffer++)
		{
			VAcsInfo* info = FindScript(LittleShort(((word*)buffer)[0]));
			if (info)
			{
				info->Flags = LittleShort(((word*)buffer)[1]);
			}
		}
	}

	//	Load script var counts
	buffer = (int*)FindChunk("SVCT");
	if (buffer)
	{
		int count = LittleLong(buffer[1]) / 4;
		buffer += 2;
		for (i = 0; i < count; i++, buffer++)
		{
			VAcsInfo* info = FindScript(LittleShort(((word*)buffer)[0]));
			if (info)
			{
				info->VarCount = LittleShort(((word*)buffer)[1]);
				//	Make sure it's at least 3 so in SpawnScript we can safely
				// assign args to first 3 variables.
				if (info->VarCount < 3)
					info->VarCount = 3;
			}
		}
	}

	//	Load functions.
	buffer = (int*)FindChunk("FUNC");
	if (buffer)
	{
		NumFunctions = LittleLong(buffer[1]) / 8;
		Functions = (VAcsFunction*)(buffer + 2);
		for (i = 0; i < NumFunctions; i++)
			Functions[i].Address = LittleLong(Functions[i].Address);
	}

	//	Unencrypt strings.
	UnencryptStrings();

	//	A temporary hack.
	buffer = (int*)FindChunk("STRL");
	if (buffer)
	{
		buffer += 2;
		NumStrings = LittleLong(buffer[1]);
		Strings = new char*[NumStrings];
		for(i = 0; i < NumStrings; i++)
		{
			Strings[i] = (char*)buffer + LittleLong(buffer[i + 3]);
		}
	}

	//	Initialise this object's map variable pointers to defaults. They can
	// be changed later once the imported modules are loaded.
	for (i = 0; i < MAX_ACS_MAP_VARS; i++)
	{
		MapVars[i] = &MapVarStore[i];
	}

	//	Initialise this object's map variables.
	memset(MapVarStore, 0, sizeof(MapVarStore));
	buffer = (int*)FindChunk("MINI");
	while (buffer)
	{
		int numvars = LittleLong(buffer[1]) / 4 - 1;
		int firstvar = LittleLong(buffer[2]);
		for (i = 0; i < numvars; i++)
		{
			MapVarStore[firstvar + i] = LittleLong(buffer[3 + i]);
		}
		buffer = (int*)NextChunk((vuint8*)buffer);
	}

	//	Create arrays.
	buffer = (int*)FindChunk("ARAY");
	if (buffer)
	{
		NumArrays = LittleLong(buffer[1]) / 8;
		ArrayStore = new VArrayInfo[NumArrays];
		memset(ArrayStore, 0, sizeof(*ArrayStore) * NumArrays);
		for (i = 0; i < NumArrays; ++i)
		{
			MapVarStore[LittleLong(buffer[2 + i * 2])] = i;
			ArrayStore[i].Size = LittleLong(buffer[3 + i * 2]);
			ArrayStore[i].Data = new vint32[ArrayStore[i].Size];
			memset(ArrayStore[i].Data, 0, ArrayStore[i].Size * sizeof(vint32));
		}
	}

	//	Initialise arrays.
	buffer = (int*)FindChunk("AINI");
	while (buffer)
	{
		int arraynum = MapVarStore[LittleLong(buffer[2])];
		if ((unsigned)arraynum < (unsigned)NumArrays)
		{
			int initsize = (LittleLong(buffer[1]) - 4) / 4;
			if (initsize > ArrayStore[arraynum].Size)
				initsize = ArrayStore[arraynum].Size;
			int *elems = ArrayStore[arraynum].Data;
			for (i = 0; i < initsize; i++)
			{
				elems[i] = LittleLong(buffer[3 + i]);
			}
		}
		buffer = (int*)NextChunk((vuint8*)buffer);
	}

	//	Start setting up array pointers.
	NumTotalArrays = NumArrays;
	buffer = (int*)FindChunk("AIMP");
	if (buffer)
	{
		NumTotalArrays += LittleLong(buffer[2]);
	}
	if (NumTotalArrays)
	{
		Arrays = new VArrayInfo*[NumTotalArrays];
		for (i = 0; i < NumArrays; ++i)
		{
			Arrays[i] = &ArrayStore[i];
		}
	}

	//	Now that everything is set up, record this object as being among
	// the loaded objects. We need to do this before resolving any imports,
	// because an import might (indirectly) need to resolve exports in this
	// module. The only things that can be exported are functions and map
	// variables, which must already be present if they're exported, so this
	// is okay.
	LibraryID = Level->LoadedObjects.Append(this) << 16;

	//	Tag the library ID to any map variables that are initialised with
	// strings.
	if (LibraryID)
	{
		buffer = (int*)FindChunk("MSTR");
		if (buffer)
		{
			for (i = 0; i < LittleLong(buffer[1]) / 4; i++)
			{
				MapVarStore[LittleLong(buffer[i + 2])] |= LibraryID;
			}
		}

		buffer = (int*)FindChunk("ASTR");
		if (buffer)
		{
			for (i = 0; i < LittleLong(buffer[1]) / 4; i++)
			{
				int arraynum = MapVarStore[LittleLong(buffer[i + 2])];
				if ((unsigned)arraynum < (unsigned)NumArrays)
				{
					int *elems = ArrayStore[arraynum].Data;
					for (int j = ArrayStore[arraynum].Size; j > 0; j--, elems++)
					{
						*elems |= LibraryID;
					}
				}
			}
		}
	}

	//	Library loading.
	buffer = (int*)FindChunk("LOAD");
	if (buffer)
	{
		const char* const parse = (char*)&buffer[2];
		for (i = 0; i < LittleLong(buffer[1]); i++)
		{
			if (parse[i])
			{
				VAcsObject* Object = NULL;
				int Lump = W_CheckNumForName(VName(&parse[i],
					VName::AddLower8), WADNS_ACSLibrary);
				if (Lump < 0)
				{
					GCon->Logf("Could not find ACS library %s.", &parse[i]);
				}
				else
				{
					Object = Level->LoadObject(Lump);
				}
				Imports.Append(Object);
				do ; while (parse[++i]);
			}
		}

		//	Go through each imported object in order and resolve all
		// imported functions and map variables.
		for (i = 0; i < Imports.Num(); i++)
		{
			VAcsObject* lib = Imports[i];
			int j;

			if (!lib)
				continue;

			// Resolve functions
			buffer = (int*)FindChunk("FNAM");
			for (j = 0; j < NumFunctions; j++)
			{
				VAcsFunction *func = &Functions[j];
				if (func->Address != 0 || func->ImportNum != 0)
					continue;

				int libfunc = lib->FindFunctionName((char*)(buffer + 2) +
					LittleLong(buffer[3 + j]));
				if (libfunc < 0)
					continue;

				VAcsFunction* realfunc = &lib->Functions[libfunc];
				//	Make sure that the library really defines this
				// function. It might simply be importing it itself.
				if (realfunc->Address == 0 || realfunc->ImportNum != 0)
					continue;

				func->Address = libfunc;
				func->ImportNum = i + 1;
				if (realfunc->ArgCount != func->ArgCount)
				{
					GCon->Logf("Function %s in %s has %d arguments. "
						"%s expects it to have %d.",
						(char *)(buffer + 2) + LittleLong(buffer[3 + j]),
						*W_LumpName(lib->LumpNum), realfunc->ArgCount,
						*W_LumpName(LumpNum), func->ArgCount);
					Format = ACS_Unknown;
				}
				//	The next two properties do not effect code compatibility,
				// so it is okay for them to be different in the imported
				// module than they are in this one, as long as we make sure
				// to use the real values.
				func->LocalCount = realfunc->LocalCount;
				func->HasReturnValue = realfunc->HasReturnValue;
			}

			//	Resolve map variables.
			buffer = (int*)FindChunk("MIMP");
			if (buffer)
			{
				char* parse = (char*)&buffer[2];
				for (j = 0; j < LittleLong(buffer[1]); j++)
				{
					int varNum = LittleLong(*(int*)&parse[j]);
					j += 4;
					int impNum = lib->FindMapVarName(&parse[j]);
					if (impNum >= 0)
					{
						MapVars[varNum] = &lib->MapVarStore[impNum];
					}
					do ; while (parse[++j]);
				}
			}

			// Resolve arrays
			if (NumTotalArrays > NumArrays)
			{
				buffer = (int*)FindChunk("AIMP");
				char* parse = (char*)&buffer[3];
				for (j = 0; j < LittleLong(buffer[2]); j++)
				{
					int varNum = LittleLong(*(int*)parse);
					parse += 4;
					int expectedSize = LittleLong(*(int*)parse);
					parse += 4;
					int impNum = lib->FindMapArray(parse);
					if (impNum >= 0)
					{
						Arrays[NumArrays + j] = &lib->ArrayStore[impNum];
						MapVarStore[varNum] = NumArrays + j;
						if (lib->ArrayStore[impNum].Size != expectedSize)
						{
							Format = ACS_Unknown;
							GCon->Logf("The array %s in %s has %ld elements, "
								"but %s expects it to only have %ld.",
								parse, *W_LumpName(lib->LumpNum),
								lib->ArrayStore[impNum].Size,
								*W_LumpName(LumpNum), expectedSize);
						}
					}
					do ; while (*++parse);
					++parse;
				}
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VAcsObject::UnencryptStrings
//
//==========================================================================

void VAcsObject::UnencryptStrings()
{
	guard(VAcsObject::UnencryptStrings);
	vuint8 *prevchunk = NULL;
	vuint32* chunk = (vuint32*)FindChunk("STRE");
	while (chunk)
	{
		for (int strnum = 0; strnum < LittleLong(chunk[3]); strnum++)
		{
			int ofs = LittleLong(chunk[5 + strnum]);
			vuint8* data = (vuint8*)chunk + ofs + 8;
			vuint8 last;
			int p = (vuint8)(ofs * 157135);
			int i = 0;
			do
			{
				last = (data[i] ^= (vuint8)(p + (i >> 1)));
				i++;
			} while (last != 0);
		}
		prevchunk = (vuint8*)chunk;
		chunk = (vuint32*)NextChunk((vuint8*)chunk);
		prevchunk[3] = 'L';
	}
	if (prevchunk)
	{
		prevchunk[3] = 'L';
	}
	unguard;
}

//==========================================================================
//
//	VAcsObject::FindFunctionName
//
//==========================================================================

int VAcsObject::FindFunctionName(const char* Name) const
{
	guard(VAcsObject::FindFunctionName);
	return FindStringInChunk(FindChunk("FNAM"), Name);
	unguard;
}

//==========================================================================
//
//	VAcsObject::FindMapVarName
//
//==========================================================================

int VAcsObject::FindMapVarName(const char* Name) const
{
	guard(VAcsObject::FindMapVarName);
	return FindStringInChunk(FindChunk("MEXP"), Name);
	unguard;
}

//==========================================================================
//
//	VAcsObject::FindMapArray
//
//==========================================================================

int VAcsObject::FindMapArray(const char* Name) const
{
	guard(VAcsObject::FindMapArray);
	int var = FindMapVarName(Name);
	if (var >= 0)
	{
		return MapVarStore[var];
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VAcsObject::FindStringInChunk
//
//==========================================================================

int VAcsObject::FindStringInChunk(vuint8* Chunk, const char* Name) const
{
	guard(VAcsObject::FindStringInChunk);
	if (Chunk)
	{
		int count = LittleLong(((int*)Chunk)[2]);
		for (int i = 0; i < count; ++i)
		{
			if (!VStr::ICmp(Name, (char*)(Chunk + 8) +
				LittleLong(((int*)Chunk)[3 + i])))
			{
				return i;
			}
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VAcsObject::FindChunk
//
//==========================================================================

vuint8* VAcsObject::FindChunk(const char* id) const
{
	guard(VAcsObject::FindChunk);
	vuint8* chunk = Chunks;
	while (chunk && chunk < Data + DataSize)
	{
		if (*(int*)chunk == *(int*)id)
		{
			return chunk;
		}
		chunk = chunk + LittleLong(((int*)chunk)[1]) + 8;
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VAcsObject::NextChunk
//
//==========================================================================

vuint8* VAcsObject::NextChunk(vuint8* prev) const
{
	guard(VAcsObject::NextChunk);
	int id = *(int*)prev;
	vuint8* chunk = prev + LittleLong(((int*)prev)[1]) + 8;
	while (chunk && chunk < Data + DataSize)
	{
		if (*(int*)chunk == id)
		{
			return chunk;
		}
		chunk = chunk + LittleLong(((int*)chunk)[1]) + 8;
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VAcsObject::Serialise
//
//==========================================================================

void VAcsObject::Serialise(VStream& Strm)
{
	guard(VAcsObject::Serialise);
	for (int i = 0; i < NumScripts; i++)
	{
		Strm << Scripts[i].RunningScript;
	}
	for (int i = 0; i < MAX_ACS_MAP_VARS; i++)
	{
		Strm << STRM_INDEX(MapVarStore[i]);
	}
	unguard;
}

//==========================================================================
//
//	VAcsObject::OffsetToPtr
//
//==========================================================================

vuint8* VAcsObject::OffsetToPtr(int Offs)
{
	if (Offs < 0 || Offs >= DataSize)
		Host_Error("Bad offset in ACS file");
	return Data + Offs;
}

//==========================================================================
//
//	VAcsObject::PtrToOffset
//
//==========================================================================

int VAcsObject::PtrToOffset(vuint8* Ptr)
{
	return Ptr - Data;
}

//==========================================================================
//
//	VAcsObject::FindScript
//
//==========================================================================

VAcsInfo* VAcsObject::FindScript(int Number) const
{
	guard(VAcsObject::FindScript);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].Number == Number)
		{
			return Scripts + i;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VAcsObject::GetFunction
//
//==========================================================================

VAcsFunction* VAcsObject::GetFunction(int funcnum,
	VAcsObject*& Object)
{
	guard(VAcsObject::GetFunction);
	if ((unsigned)funcnum >= (unsigned)NumFunctions)
	{
		return NULL;
	}
	VAcsFunction* Func = Functions + funcnum;
	if (Func->ImportNum)
	{
		return Imports[Func->ImportNum - 1]->GetFunction(Func->Address,
			Object);
	}
	Object = this;
	return Func;
	unguard;
}

//==========================================================================
//
//	VAcsObject::GetArrayVal
//
//==========================================================================

int VAcsObject::GetArrayVal(int ArrayIdx, int Index)
{
	guard(VAcsObject::GetArrayVal);
	if ((unsigned)ArrayIdx >= (unsigned)NumTotalArrays)
		return 0;
	if ((unsigned)Index >= (unsigned)Arrays[ArrayIdx]->Size)
		return 0;
	return Arrays[ArrayIdx]->Data[Index];
	unguard;
}

//==========================================================================
//
//	VAcsObject::SetArrayVal
//
//==========================================================================

void VAcsObject::SetArrayVal(int ArrayIdx, int Index, int Value)
{
	guard(VAcsObject::SetArrayVal);
	if ((unsigned)ArrayIdx >= (unsigned)NumTotalArrays)
		return;
	if ((unsigned)Index >= (unsigned)Arrays[ArrayIdx]->Size)
		return;
	Arrays[ArrayIdx]->Data[Index] = Value;
	unguard;
}

//==========================================================================
//
//	VAcsObject::StartTypedACScripts
//
//==========================================================================

void VAcsObject::StartTypedACScripts(int Type)
{
	guard(VAcsObject::StartTypedACScripts);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].Type == Type)
		{
			// Auto-activate
			Level->SpawnScript(&Scripts[i], this, NULL, NULL, 0, 0, 0, 0,
				true, true);
		}
	}
	unguard;
}

//==========================================================================
//
//	VAcsLevel::VAcsLevel
//
//==========================================================================

VAcsLevel::VAcsLevel(VLevel* ALevel)
: XLevel(ALevel)
{
}

//==========================================================================
//
//	VAcsLevel::~VAcsLevel
//
//==========================================================================

VAcsLevel::~VAcsLevel()
{
	guard(VAcsLevel::~VAcsLevel);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		delete LoadedObjects[i];
	}
	LoadedObjects.Clear();
	unguard;
}

//==========================================================================
//
//	VAcsLevel::LoadObject
//
//==========================================================================

VAcsObject* VAcsLevel::LoadObject(int Lump)
{
	guard(VAcsLevel::LoadObject);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		if (LoadedObjects[i]->LumpNum == Lump)
		{
			return LoadedObjects[i];
		}
	}
	return new VAcsObject(this, Lump);
	unguard;
}

//==========================================================================
//
//	VAcsLevel::FindScript
//
//==========================================================================

VAcsInfo* VAcsLevel::FindScript(int Number, VAcsObject*& Object)
{
	guard(VAcsLevel::FindScript);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		VAcsInfo* Found = LoadedObjects[i]->FindScript(Number);
		if (Found)
		{
			Object = LoadedObjects[i];
			return Found;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VAcsLevel::GetString
//
//==========================================================================

const char* VAcsLevel::GetString(int Index)
{
	guard(VAcsLevel::GetString);
	int ObjIdx = Index >> 16;
	if (ObjIdx >= LoadedObjects.Num())
	{
		return NULL;
	}
	return LoadedObjects[ObjIdx]->GetString(Index & 0xffff);
	unguard;
}

//==========================================================================
//
//	VAcsLevel::GetObject
//
//==========================================================================

VAcsObject* VAcsLevel::GetObject(int Index)
{
	guard(VAcsLevel::GetObject);
	if ((unsigned)Index >= (unsigned)LoadedObjects.Num())
	{
		return NULL;
	}
	return LoadedObjects[Index];
	unguard;
}

//==========================================================================
//
//	VAcsLevel::StartTypedACScripts
//
//==========================================================================

void VAcsLevel::StartTypedACScripts(int Type)
{
	guard(VAcsLevel::StartTypedACScripts);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->StartTypedACScripts(Type);
	}
	unguard;
}

//==========================================================================
//
//	VAcsLevel::Serialise
//
//==========================================================================

void VAcsLevel::Serialise(VStream& Strm)
{
	guard(VAcsLevel::Serialise);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->Serialise(Strm);
	}
	unguard;
}

//==========================================================================
//
//	VAcsLevel::AddToACSStore
//
//==========================================================================

bool VAcsLevel::AddToACSStore(int Type, VName Map, int Number, int Arg1,
	int Arg2, int Arg3, VEntity* Activator)
{
	guard(VAcsLevel::AddToACSStore);
	VAcsStore& S = XLevel->WorldInfo->Acs->Store.Alloc();
	S.Map = Map;
	S.Type = Type;
	S.PlayerNum = Activator && Activator->Player ?
		SV_GetPlayerNum(Activator->Player) : -1;
	S.Script = Number;
	S.Args[0] = Arg1;
	S.Args[1] = Arg2;
	S.Args[2] = Arg3;
	return true;
	unguard;
}

//==========================================================================
//
//	VAcsLevel::CheckAcsStore
//
//	Scans the ACS store and executes all scripts belonging to the current
// map.
//
//==========================================================================

void VAcsLevel::CheckAcsStore()
{
	guard(VAcsLevel::CheckAcsStore);
	for (int i = XLevel->WorldInfo->Acs->Store.Num() - 1; i >= 0; i--)
	{
		VAcsStore* store = &XLevel->WorldInfo->Acs->Store[i];
		if (store->Map != XLevel->MapName)
		{
			continue;
		}

		VAcsObject* Object;
		VAcsInfo* Info = FindScript(store->Script, Object);
		if (!Info)
		{
			//	Script not found
			GCon->Logf(NAME_Dev, "Start ACS ERROR: Unknown script %d", store->Script);
		}
		else
		{
			switch (store->Type)
			{
			case VAcsStore::Start:
			case VAcsStore::StartAlways:
				SpawnScript(Info, Object, store->PlayerNum >= 0 &&
					GGameInfo->Players[store->PlayerNum] &&
					(GGameInfo->Players[store->PlayerNum]->PlayerFlags &
					VBasePlayer::PF_Spawned) ?
					GGameInfo->Players[store->PlayerNum]->MO : NULL, NULL, 0,
					store->Args[0], store->Args[1], store->Args[2],
					store->Type == VAcsStore::StartAlways, true);
				break;

			case VAcsStore::Terminate:
				if (!Info->RunningScript ||
					Info->RunningScript->State == VAcs::ASTE_Terminating)
				{
					//	States that disallow termination
					break;
				}
				Info->RunningScript->State = VAcs::ASTE_Terminating;
				break;

			case VAcsStore::Suspend:
				if (!Info->RunningScript ||
					Info->RunningScript->State == VAcs::ASTE_Suspended ||
					Info->RunningScript->State == VAcs::ASTE_Terminating)
				{
					// States that disallow suspension
					break;
				}
				Info->RunningScript->State = VAcs::ASTE_Suspended;
				break;
			}
		}
		XLevel->WorldInfo->Acs->Store.RemoveIndex(i);
	}
	unguard;
}

//==========================================================================
//
//	VAcsLevel::Start
//
//==========================================================================

bool VAcsLevel::Start(int Number, int MapNum, int Arg1, int Arg2, int Arg3,
	VEntity* Activator, line_t* Line, int Side, bool Always, bool WantResult)
{
	guard(VAcsLevel::Start);
	if (MapNum)
	{
		VName Map = P_GetMapNameByLevelNum(MapNum);
		if (Map != NAME_None && Map != XLevel->MapName)
		{
			// Add to the script store
			return AddToACSStore(Always ? VAcsStore::StartAlways :
				VAcsStore::Start, Map, Number, Arg1, Arg2, Arg3, Activator);
		}
	}

	VAcsObject* Object;
	VAcsInfo* Info = FindScript(Number, Object);
	if (!Info)
	{
		//	Script not found
		GCon->Logf(NAME_Dev, "Start ACS ERROR: Unknown script %d", Number);
		return false;
	}
	VAcs* script = SpawnScript(Info, Object, Activator, Line, Side, Arg1,
		Arg2, Arg3, Always, false);
	if (WantResult)
	{
		return !!script->RunScript(host_frametime);
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VAcsLevel::Terminate
//
//==========================================================================

bool VAcsLevel::Terminate(int Number, int MapNum)
{
	guard(VAcsLevel::Terminate);
	if (MapNum)
	{
		VName Map = P_GetMapNameByLevelNum(MapNum);
		if (Map != NAME_None && Map != XLevel->MapName)
		{
			// Add to the script store
			return AddToACSStore(VAcsStore::Terminate, Map, Number, 0, 0, 0, 0);
		}
	}

	VAcsObject* Object;
	VAcsInfo* Info = FindScript(Number, Object);
	if (!Info)
	{
		//	Script not found
		return false;
	}
	if (!Info->RunningScript ||
		Info->RunningScript->State == VAcs::ASTE_Terminating)
	{
		//	States that disallow termination
		return false;
	}
	Info->RunningScript->State = VAcs::ASTE_Terminating;
	return true;
	unguard;
}

//==========================================================================
//
//	VAcsLevel::Suspend
//
//==========================================================================

bool VAcsLevel::Suspend(int Number, int MapNum)
{
	guard(VAcsLevel::Suspend);
	if (MapNum)
	{
		VName Map = P_GetMapNameByLevelNum(MapNum);
		if (Map != NAME_None && Map != XLevel->MapName)
		{
			// Add to the script store
			return AddToACSStore(VAcsStore::Suspend, Map, Number, 0, 0, 0, 0);
		}
	}

	VAcsObject* Object;
	VAcsInfo* Info = FindScript(Number, Object);
	if (!Info)
	{
		//	Script not found.
		return false;
	}
	if (!Info->RunningScript ||
		Info->RunningScript->State == VAcs::ASTE_Suspended ||
		Info->RunningScript->State == VAcs::ASTE_Terminating)
	{
		// States that disallow suspension
		return false;
	}
	Info->RunningScript->State = VAcs::ASTE_Suspended;
	return true;
	unguard;
}

//==========================================================================
//
//	VAcsLevel::SpawnScript
//
//==========================================================================

VAcs* VAcsLevel::SpawnScript(VAcsInfo* Info, VAcsObject* Object,
	VEntity* Activator, line_t* Line, int Side, int Arg1, int Arg2, int Arg3,
	bool Always, bool Delayed)
{
	guard(VAcsLevel::SpawnScript);
	if (!Always && Info->RunningScript)
	{
		if (Info->RunningScript->State == VAcs::ASTE_Suspended)
		{
			//	Resume a suspended script
			Info->RunningScript->State = VAcs::ASTE_Running;
		}
		//	Script is already executing
		return Info->RunningScript;
	}

	VAcs* script = (VAcs*)XLevel->SpawnThinker(VAcs::StaticClass());
	script->info = Info;
	script->number = Info->Number;
	script->InstructionPointer = Info->Address;
	script->State = VAcs::ASTE_Running;
	script->ActiveObject = Object;
	script->Activator = Activator;
	script->line = Line;
	script->side = Side;
	script->LocalVars = new vint32[Info->VarCount];
	script->LocalVars[0] = Arg1;
	script->LocalVars[1] = Arg2;
	script->LocalVars[2] = Arg3;
	memset(script->LocalVars + Info->ArgCount, 0,
		(Info->VarCount - Info->ArgCount) * 4);
	if (Delayed)
	{
		//	World objects are allotted 1 second for initialization.
		script->DelayTime = 1.0;
	}
	if (!Always)
	{
		Info->RunningScript = script;
	}
	return script;
	unguard;
}

//==========================================================================
//
//	VAcsGrowingArray::VAcsGrowingArray
//
//==========================================================================

VAcsGrowingArray::VAcsGrowingArray()
: Size(0)
, Data(NULL)
{
}

//==========================================================================
//
//	VAcsGrowingArray::Redim
//
//==========================================================================

void VAcsGrowingArray::Redim(int NewSize)
{
	guard(VAcsGrowingArray::Redim);
	if (!NewSize && Data)
	{
		delete[] Data;
		Data = NULL;
	}
	else if (NewSize)
	{
		int* Temp = Data;
		Data = new int[NewSize];
		if (Temp)
		{
			memcpy(Data, Temp, Min(Size, NewSize) * sizeof(int));
			delete[] Temp;
		}
		//	Clear newly allocated elements.
		if (NewSize > Size)
		{
			memset(Data + Size, 0, (NewSize - Size) * sizeof(int));
		}
	}
	Size = NewSize;
	unguard;
}

//==========================================================================
//
//	VAcsGrowingArray::SetElemVal
//
//==========================================================================

void VAcsGrowingArray::SetElemVal(int Index, int Value)
{
	guard(VAcsGrowingArray::SetElemVal);
	if (Index >= Size)
	{
		Redim(Index + 1);
	}
	Data[Index] = Value;
	unguard;
}

//==========================================================================
//
//	VAcsGrowingArray::GetElemVal
//
//==========================================================================

int VAcsGrowingArray::GetElemVal(int Index)
{
	guard(VAcsGrowingArray::GetElemVal);
	if ((unsigned)Index >= (unsigned)Size)
		return 0;
	return Data[Index];
	unguard;
}

//==========================================================================
//
//	VAcsGrowingArray::Serialise
//
//==========================================================================

void VAcsGrowingArray::Serialise(VStream& Strm)
{
	guard(VAcsGrowingArray::Serialise);
	if (Strm.IsLoading())
	{
		int NewSize;
		Strm << STRM_INDEX(NewSize);
		Redim(NewSize);
	}
	else
	{
		Strm << STRM_INDEX(Size);
	}
	for (int i = 0; i < Size; i++)
	{
		Strm << STRM_INDEX(Data[i]);
	}
	unguard;
}

//==========================================================================
//
//	VAcs::Destroy
//
//==========================================================================

void VAcs::Destroy()
{
	guard(VAcs::Destroy);
	if (LocalVars)
	{
		delete[] LocalVars;
	}
	unguard;
}

//==========================================================================
//
//	VAcs::Serialise
//
//==========================================================================

void VAcs::Serialise(VStream& Strm)
{
	guard(VAcs::Serialise);
	vint32 TmpInt;

	Super::Serialise(Strm);
	Strm << Activator;
	if (Strm.IsLoading())
	{
		Strm << STRM_INDEX(TmpInt);
		line = TmpInt == -1 ? NULL : &XLevel->Lines[TmpInt];
	}
	else
	{
		TmpInt = line ? line - XLevel->Lines : -1;
		Strm << STRM_INDEX(TmpInt);
	}
	Strm << side
		<< number
		<< State
		<< DelayTime
		<< STRM_INDEX(WaitValue);
	if (Strm.IsLoading())
	{
		Strm << STRM_INDEX(TmpInt);
		ActiveObject = XLevel->Acs->GetObject(TmpInt);
		Strm << STRM_INDEX(TmpInt);
		InstructionPointer = ActiveObject->OffsetToPtr(TmpInt);
		info = ActiveObject->FindScript(number);
		LocalVars = new vint32[info->VarCount];
	}
	else
	{
		TmpInt = ActiveObject->GetLibraryID() >> 16;
		Strm << STRM_INDEX(TmpInt);
		TmpInt = ActiveObject->PtrToOffset(InstructionPointer);
		Strm << STRM_INDEX(TmpInt);
	}
	for (int i = 0; i < info->VarCount; i++)
	{
		Strm << LocalVars[i];
	}
	unguard;
}

//==========================================================================
//
//	VAcs::Tick
//
//==========================================================================

void VAcs::Tick(float DeltaTime)
{
	guard(VAcs::Tick);
	RunScript(DeltaTime);
	unguard;
}

//==========================================================================
//
//	VAcs::RunScript
//
//==========================================================================

#define STUB(cmd)	GCon->Log("Executing unimplemented ACS PCODE " #cmd);

#ifdef __GNUC__
#define USE_COMPUTED_GOTO 1
#endif

#if USE_COMPUTED_GOTO
#define ACSVM_SWITCH(op)	goto *vm_labels[op];
#define ACSVM_CASE(x)		Lbl_ ## x:
#define ACSVM_BREAK \
	if (fmt == ACS_LittleEnhanced) \
	{ \
		cmd = *ip; \
		if (cmd >= 240) \
		{ \
			cmd = 240 + ((cmd - 240) << 8) + ip[1]; \
			ip += 2; \
		} \
		else \
		{ \
			ip++; \
		} \
	} \
	else \
	{ \
		cmd = READ_INT32(ip); \
		ip += 4; \
	} \
	if ((vuint32)cmd >= PCODE_COMMAND_COUNT) \
	{ \
		goto LblDefault; \
	} \
	goto *vm_labels[cmd];
#define ACSVM_BREAK_STOP	goto LblFuncStop;
#define ACSVM_DEFAULT		LblDefault:
#else
#define ACSVM_SWITCH(op)	switch (cmd)
#define ACSVM_CASE(op)		case op:
#define ACSVM_BREAK			break
#define ACSVM_BREAK_STOP	break
#define ACSVM_DEFAULT		default:
#endif

#define READ_INT32(p)		((p)[0] | ((p)[1] << 8) | ((p)[2] << 16) | ((p)[3] << 24))
#define READ_BYTE_OR_INT32	(fmt == ACS_LittleEnhanced ? *ip : READ_INT32(ip))
#define INC_BYTE_OR_INT32	if (fmt == ACS_LittleEnhanced) ip++; else ip += 4

int VAcs::RunScript(float DeltaTime)
{
	guard(VAcs::RunScript);
	VAcsObject* WaitObject;
	if (State == ASTE_Terminating)
	{
		if (info->RunningScript == this)
		{
			info->RunningScript = NULL;
		}
		DestroyThinker();
		return 1;
	}
	if (State == ASTE_WaitingForTag && !Level->eventTagBusy(WaitValue))
	{
		State = ASTE_Running;
	}
	if (State == ASTE_WaitingForPoly && !Level->eventPolyBusy(WaitValue))
	{
		State = ASTE_Running;
	}
	if (State == ASTE_WaitingForScriptStart &&
		XLevel->Acs->FindScript(WaitValue, WaitObject) &&
		XLevel->Acs->FindScript(WaitValue, WaitObject)->RunningScript)
	{
		State = ASTE_WaitingForScript;
	}
	if (State == ASTE_WaitingForScript &&
		!XLevel->Acs->FindScript(WaitValue, WaitObject)->RunningScript)
	{
		State = ASTE_Running;
	}
	if (State != ASTE_Running)
	{
		return 1;
	}
	if (DelayTime)
	{
		DelayTime -= DeltaTime;
		if (DelayTime < 0)
			DelayTime = 0;
		return 1;
	}

	//	Shortcuts
	int* WorldVars = Level->World->Acs->WorldVars;
	int* GlobalVars = Level->World->Acs->GlobalVars;
	VAcsGrowingArray* WorldArrays = Level->World->Acs->WorldArrays;
	VAcsGrowingArray* GlobalArrays = Level->World->Acs->GlobalArrays;

	VStr PrintStr;
	vint32 resultValue = 1;
	vint32 stack[ACS_STACK_DEPTH];
	vint32* optstart = NULL;
	vint32* locals = LocalVars;
	VAcsFunction* activeFunction = NULL;
	EAcsFormat fmt = ActiveObject->GetFormat();
	int action = SCRIPT_Continue;
	vuint8* ip = InstructionPointer;
	vint32* sp = stack;
	do
	{
		vint32 cmd;

#if USE_COMPUTED_GOTO
		static void* vm_labels[] = {
#define DECLARE_PCD(name)	&&Lbl_PCD_ ## name
#include "p_acs.h"
		0 };
#endif

		if (fmt == ACS_LittleEnhanced)
		{
			cmd = *ip;
			if (cmd >= 240)
			{
				cmd = 240 + ((cmd - 240) << 8) + ip[1];
				ip += 2;
			}
			else
			{
				ip++;
			}
		}
		else
		{
			cmd = READ_INT32(ip);
			ip += 4;
		}

		ACSVM_SWITCH(cmd)
		{
		//	Standard P-Code commands.
		ACSVM_CASE(PCD_Nop)
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Terminate)
			action = SCRIPT_Terminate;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_Suspend)
			State = ASTE_Suspended;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_PushNumber)
			*sp = READ_INT32(ip);
			ip += 4;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec1)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, sp[-1], 0, 0, 0, 0,
					line, side, Activator);
				sp--;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec2)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, sp[-2], sp[-1], 0,
					0, 0, line, side, Activator);
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec3)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, sp[-3], sp[-2],
					sp[-1], 0, 0, line, side, Activator);
				sp -= 3;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec4)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, sp[-4], sp[-3],
					sp[-2], sp[-1], 0, line, side, Activator);
				sp -= 4;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec5)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, sp[-5], sp[-4],
					sp[-3], sp[-2], sp[-1], line, side, Activator);
				sp -= 5;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec1Direct)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, READ_INT32(ip), 0,
					0, 0, 0, line, side, Activator);
				ip += 4;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec2Direct)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, READ_INT32(ip),
					READ_INT32(ip + 4), 0, 0, 0, line, side, Activator);
				ip += 8;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec3Direct)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, READ_INT32(ip),
					READ_INT32(ip + 4), READ_INT32(ip + 8), 0, 0, line, side,
					Activator);
				ip += 12;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec4Direct)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, READ_INT32(ip),
					READ_INT32(ip + 4), READ_INT32(ip + 8),
					READ_INT32(ip + 12), 0, line, side, Activator);
				ip += 16;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec5Direct)
			{
				int special = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				Level->eventExecuteActionSpecial(special, READ_INT32(ip),
					READ_INT32(ip + 4), READ_INT32(ip + 8),
					READ_INT32(ip + 12), READ_INT32(ip + 16), line, side,
					Activator);
				ip += 20;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Add)
			sp[-2] += sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Subtract)
			sp[-2] -= sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Multiply)
			sp[-2] *= sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Divide)
			sp[-2] /= sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Modulus)
			sp[-2] %= sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EQ)
			sp[-2] = sp[-2] == sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_NE)
			sp[-2] = sp[-2] != sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LT)
			sp[-2] = sp[-2] < sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GT)
			sp[-2] = sp[-2] > sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LE)
			sp[-2] = sp[-2] <= sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GE)
			sp[-2] = sp[-2] >= sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AssignScriptVar)
			locals[READ_BYTE_OR_INT32] = sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AssignMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] = sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AssignWorldVar)
			WorldVars[READ_BYTE_OR_INT32] = sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushScriptVar)
			*sp = locals[READ_BYTE_OR_INT32];
			INC_BYTE_OR_INT32;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushMapVar)
			*sp = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
			INC_BYTE_OR_INT32;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushWorldVar)
			*sp = WorldVars[READ_BYTE_OR_INT32];
			INC_BYTE_OR_INT32;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AddScriptVar)
			locals[READ_BYTE_OR_INT32] += sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AddMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] += sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AddWorldVar)
			WorldVars[READ_BYTE_OR_INT32] += sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SubScriptVar)
			locals[READ_BYTE_OR_INT32] -= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SubMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] -= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SubWorldVar)
			WorldVars[READ_BYTE_OR_INT32] -= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MulScriptVar)
			locals[READ_BYTE_OR_INT32] *= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MulMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] *= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MulWorldVar)
			WorldVars[READ_BYTE_OR_INT32] *= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DivScriptVar)
			locals[READ_BYTE_OR_INT32] /= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DivMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] /= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DivWorldVar)
			WorldVars[READ_BYTE_OR_INT32] /= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ModScriptVar)
			locals[READ_BYTE_OR_INT32] %= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ModMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] %= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ModWorldVar)
			WorldVars[READ_BYTE_OR_INT32] %= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IncScriptVar)
			locals[READ_BYTE_OR_INT32]++;
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IncMapVar)
			(*ActiveObject->MapVars[READ_BYTE_OR_INT32])++;
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IncWorldVar)
			WorldVars[READ_BYTE_OR_INT32]++;
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DecScriptVar)
			locals[READ_BYTE_OR_INT32]--;
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DecMapVar)
			(*ActiveObject->MapVars[READ_BYTE_OR_INT32])--;
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DecWorldVar)
			WorldVars[READ_BYTE_OR_INT32]--;
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Goto)
			ip = ActiveObject->OffsetToPtr(READ_INT32(ip));
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IfGoto)
			if (sp[-1])
			{
				ip = ActiveObject->OffsetToPtr(READ_INT32(ip));
			}
			else
			{
				ip += 4;
			}
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Drop)
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Delay)
			DelayTime = float(sp[-1]) / 35.0;
			sp--;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_DelayDirect)
			DelayTime = float(READ_INT32(ip)) / 35.0;
			ip += 4;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_Random)
			sp[-2] = sp[-2] + (vint32)(Random() * (sp[-1] - sp[-2] + 1));
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RandomDirect)
			*sp = READ_INT32(ip) + (vint32)(Random() * (READ_INT32(ip + 4) -
				READ_INT32(ip) + 1));
			ip += 8;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ThingCount)
			sp[-2] = Level->eventThingCount(sp[-2], sp[-1]);
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ThingCountDirect)
			*sp = Level->eventThingCount(READ_INT32(ip), READ_INT32(ip + 4));
			ip += 8;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TagWait)
			WaitValue = sp[-1];
			State = ASTE_WaitingForTag;
			sp--;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_TagWaitDirect)
			WaitValue = READ_INT32(ip);
			State = ASTE_WaitingForTag;
			ip += 4;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_PolyWait)
			WaitValue = sp[-1];
			State = ASTE_WaitingForPoly;
			sp--;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_PolyWaitDirect)
			WaitValue = READ_INT32(ip);
			State = ASTE_WaitingForPoly;
			ip += 4;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_ChangeFloor)
			{
				int Flat = GTextureManager.NumForName(VName(GetStr(sp[-1]),
					VName::AddLower8), TEXTYPE_Flat, true, true);
				for  (int Idx = FindSectorFromTag(sp[-2], -1); Idx >= 0;
					Idx = FindSectorFromTag(sp[-2], Idx))
				{
					XLevel->Sectors[Idx].floor.pic = Flat;
				}
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ChangeFloorDirect)
			{
				int Tag = READ_INT32(ip);
				int Flat = GTextureManager.NumForName(VName(GetStr(
					READ_INT32(ip + 4)), VName::AddLower8), TEXTYPE_Flat,
					true, true);
				ip += 8;
				for (int Idx = FindSectorFromTag(Tag, -1); Idx >= 0;
					Idx = FindSectorFromTag(Tag, Idx))
				{
					XLevel->Sectors[Idx].floor.pic = Flat;
				}
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ChangeCeiling)
			{
				int Flat = GTextureManager.NumForName(VName(GetStr(sp[-1]),
					VName::AddLower8), TEXTYPE_Flat, true, true);
				for  (int Idx = FindSectorFromTag(sp[-2], -1); Idx >= 0;
					Idx = FindSectorFromTag(sp[-2], Idx))
				{
					XLevel->Sectors[Idx].ceiling.pic = Flat;
				}
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ChangeCeilingDirect)
			{
				int Tag = READ_INT32(ip);
				int Flat = GTextureManager.NumForName(VName(GetStr(
					READ_INT32(ip + 4)), VName::AddLower8), TEXTYPE_Flat,
					true, true);
				ip += 8;
				for (int Idx = FindSectorFromTag(Tag, -1); Idx >= 0;
					Idx = FindSectorFromTag(Tag, Idx))
				{
					XLevel->Sectors[Idx].ceiling.pic = Flat;
				}
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Restart)
			ip = info->Address;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndLogical)
			sp[-2] = sp[-2] && sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrLogical)
			sp[-2] = sp[-2] || sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndBitwise)
			sp[-2] = sp[-2] & sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrBitwise)
			sp[-2] = sp[-2] | sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EorBitwise)
			sp[-2] = sp[-2] ^ sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_NegateLogical)
			sp[-1] = !sp[-1];
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LShift)
			sp[-2] = sp[-2] << sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RShift)
			sp[-2] = sp[-2] >> sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_UnaryMinus)
			sp[-1] = -sp[-1];
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IfNotGoto)
			if (!sp[-1])
			{
				ip = ActiveObject->OffsetToPtr(READ_INT32(ip));
			}
			else
			{
				ip += 4;
			}
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LineSide)
			*sp = side;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ScriptWait)
			WaitValue = sp[-1];
			if (!XLevel->Acs->FindScript(WaitValue, WaitObject) ||
				!XLevel->Acs->FindScript(WaitValue, WaitObject)->RunningScript)
			{
				State = ASTE_WaitingForScriptStart;
			}
			else
			{
				State = ASTE_WaitingForScript;
			}
			sp--;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_ScriptWaitDirect)
			WaitValue = READ_INT32(ip);
			if (!XLevel->Acs->FindScript(WaitValue, WaitObject) ||
				!XLevel->Acs->FindScript(WaitValue, WaitObject)->RunningScript)
			{
				State = ASTE_WaitingForScriptStart;
			}
			else
			{
				State = ASTE_WaitingForScript;
			}
			ip += 4;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_ClearLineSpecial)
			if (line)
			{
				line->special = 0;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CaseGoto)
			if (sp[-1] == READ_INT32(ip))
			{
				ip = ActiveObject->OffsetToPtr(READ_INT32(ip + 4));
				sp--;
			}
			else
			{
				ip += 8;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_BeginPrint)
			PrintStr.Clean();
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EndPrint)
			PrintStr = PrintStr.EvalEscapeSequences();
			if (Activator && Activator->EntityFlags & VEntity::EF_IsPlayer)
			{
				Activator->Player->CentrePrintf(*PrintStr);
			}
			else
			{
				BroadcastCentrePrint(*PrintStr);
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintString)
			PrintStr += GetStr(sp[-1]);
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintNumber)
			PrintStr +=  VStr(sp[-1]);
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintCharacter)
			PrintStr += (char)sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerCount)
			sp[0] = 0;
			for (int i = 0; i < MAXPLAYERS; i++)
			{
				if (Level->Game->Players[i])
					sp[0]++;
			}
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GameType)
			if (netgame == false)
			{
				*sp = GAME_SINGLE_PLAYER;
			}
			else if (deathmatch)
			{
				*sp = GAME_NET_DEATHMATCH;
			}
			else
			{
				*sp = GAME_NET_COOPERATIVE;
			}
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GameSkill)
			*sp = gameskill;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Timer)
			*sp = XLevel->TicTime;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SectorSound)
			Level->SectorStartSound(line ? line->frontsector : NULL,
				GSoundManager->GetSoundID(GetStr(sp[-2])), 0, sp[-1] / 127.0,
				1.0);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AmbientSound)
			StartSound(TVec(0, 0, 0), 0, GSoundManager->GetSoundID(
				GetStr(sp[-2])), 0, sp[-1] / 127.0, 0.0);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SoundSequence)
			Level->SectorStartSequence(line ? line->frontsector : NULL,
				GetStr(sp[-1]), 0);
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetLineTexture)
			{
				int Tex = GTextureManager.NumForName(VName(GetStr(sp[-1]),
					VName::AddLower8), TEXTYPE_Wall, true, true);
				int searcher = -1;
				for (line_t *line = Level->eventFindLine(sp[-4], &searcher);
					line != NULL; line = Level->eventFindLine(sp[-4], &searcher))
				{
					if (sp[-2] == TEXTURE_MIDDLE)
					{
						GLevel->Sides[line->sidenum[sp[-3]]].midtexture = Tex;
					}
					else if (sp[-2] == TEXTURE_BOTTOM)
					{
						GLevel->Sides[line->sidenum[sp[-3]]].bottomtexture = Tex;
					}
					else
					{
						// TEXTURE_TOP
						GLevel->Sides[line->sidenum[sp[-3]]].toptexture = Tex;
					}
				}
				sp -= 4;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetLineBlocking)
			{
				int searcher = -1;
				for (line_t* line = Level->eventFindLine(sp[-2], &searcher);
					line != NULL; line = Level->eventFindLine(sp[-2], &searcher))
				{
					switch (sp[-1])
					{
					case BLOCK_NOTHING:
						line->flags &= ~(ML_BLOCKING | ML_BLOCKEVERYTHING | ML_RAILING);
						break;
					case BLOCK_CREATURES:
					default:
						line->flags &= ~(ML_BLOCKEVERYTHING | ML_RAILING);
						line->flags |= ML_BLOCKING;
						break;
					case BLOCK_EVERYTHING:
						line->flags &= ~ML_RAILING;
						line->flags |= ML_BLOCKING | ML_BLOCKEVERYTHING;
						break;
					case BLOCK_RAILING:
						line->flags &= ~ML_BLOCKEVERYTHING;
						line->flags |= ML_BLOCKING | ML_RAILING;
						break;
					}
				}
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetLineSpecial)
			{
				int searcher = -1;
				for (line_t* line = Level->eventFindLine(sp[-7], &searcher);
					line != NULL; line = Level->eventFindLine(sp[-7], &searcher))
				{
					line->special = sp[-6];
					line->arg1 = sp[-5];
					line->arg2 = sp[-4];
					line->arg3 = sp[-3];
					line->arg4 = sp[-2];
					line->arg5 = sp[-1];
				}
				sp -= 7;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ThingSound)
			{
				VName sound = GetStr(sp[-2]);
				int searcher = -1;
				for (VEntity* mobj = Level->eventFindMobjFromTID(sp[-3], &searcher);
					mobj != NULL; mobj = Level->eventFindMobjFromTID(sp[-3], &searcher))
				{
					mobj->StartSound(sound, 0, sp[-1] / 127.0, 1.0);
				}
				sp -= 3;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EndPrintBold)
			//FIXME yellow message
			PrintStr = PrintStr.EvalEscapeSequences();
			BroadcastCentrePrint(*PrintStr);
			ACSVM_BREAK;

		//	Extended P-Code commands.
		ACSVM_CASE(PCD_ActivatorSound)
			if (Activator)
			{
				Activator->StartSound(GetStr(sp[-2]), 0, sp[-1] / 127.0, 1.0);
			}
			else
			{
				StartSound(TVec(0, 0, 0), 0, GSoundManager->GetSoundID(
					GetStr(sp[-2])), 0, sp[-1] / 127.0, 1.0);
			}
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LocalAmbientSound)
			if (Activator)
			{
				Activator->StartLocalSound(GetStr(sp[-2]), 0, sp[-1] / 127.0,
					1.0);
			}
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetLineMonsterBlocking)
			{
				int searcher = -1;
				for (line_t* line = Level->eventFindLine(sp[-2], &searcher);
					line != NULL; line = Level->eventFindLine(sp[-2], &searcher))
				{
					if (sp[-1])
						line->flags |= ML_BLOCKMONSTERS;
					else
						line->flags &= ~ML_BLOCKMONSTERS;
				}
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerHealth)
			if (Activator)
			{
				*sp = Activator->Health;
			}
			else
			{
				*sp = 0;
			}
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerArmorPoints)
			STUB(PCD_PlayerArmorPoints)
			//FIXME implement this
			if (Activator && Activator->Player)
			{
				*sp = 0;
			}
			else
			{
				*sp = 0;
			}
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerFrags)
			if (Activator && Activator->Player)
			{
				*sp = Activator->Player->Frags;
			}
			else
			{
				*sp = 0;
			}
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintName)
			{
				VBasePlayer* Plr;
				if (sp[-1] <= 0 || sp[-1] > MAXPLAYERS)
				{
					Plr = Activator ? Activator->Player : NULL;
				}
				else
				{
					Plr = Level->Game->Players[sp[-1] - 1];
				}
				if (Plr && (Plr->PlayerFlags & VBasePlayer::PF_Spawned))
				{
					PrintStr += Plr->PlayerName;
				}
				else if (Plr && !(Plr->PlayerFlags & VBasePlayer::PF_Spawned))
				{
					PrintStr += VStr("Player ") + VStr(sp[-1]);
				}
				else if (Activator)
				{
					PrintStr += Activator->GetClass()->GetName();
				}
				else
				{
					PrintStr += "Unknown";
				}
				sp--;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MusicChange)
			SV_ChangeMusic(GetStr(sp[-2]));
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SinglePlayer)
			sp[-1] = !netgame;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_FixedMul)
			sp[-2] = vint32((double)sp[-2] / (double)0x10000 * (double)sp[-1]);
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_FixedDiv)
			sp[-2] = vint32((double)sp[-2] / (double)sp[-1] * (double)0x10000);
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetGravity)
			Level->Gravity = ((float)sp[-1] / (float)0x10000) *
				DEFAULT_GRAVITY / 800.0;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetGravityDirect)
			Level->Gravity = ((float)READ_INT32(ip) / (float)0x10000) *
				DEFAULT_GRAVITY / 800.0;
			ip += 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetAirControl)
			STUB(PCD_SetAirControl)
			//FIXME implement this
			//sp[-1] - air control, fixed point
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetAirControlDirect)
			STUB(PCD_SetAirControlDirect)
			//FIXME implement this
			//READ_INT32(ip) - air control, fixed point
			ip += 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ClearInventory)
			STUB(PCD_ClearInventory)
			//FIXME implement this
			if (Activator)
			{
			}
			else
			{
				for (int i = 0; i < MAXPLAYERS; i++)
				{
				}
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GiveInventory)
			STUB(PCD_GiveInventory)
			GiveInventory(Activator, GetStr(sp[-2]), sp[-1]);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GiveInventoryDirect)
			STUB(PCD_GiveInventoryDirect)
			GiveInventory(Activator, GetStr(READ_INT32(ip)),
				READ_INT32(ip + 4));
			ip += 8;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TakeInventory)
			STUB(PCD_TakeInventory)
			TakeInventory(Activator, GetStr(sp[-2]), sp[-1]);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TakeInventoryDirect)
			STUB(PCD_TakeInventoryDirect)
			TakeInventory(Activator, GetStr(READ_INT32(ip)),
				READ_INT32(ip + 4));
			ip += 8;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CheckInventory)
			STUB(PCD_CheckInventory)
			sp[-1] = CheckInventory(Activator, GetStr(sp[-1]));
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CheckInventoryDirect)
			STUB(PCD_CheckInventoryDirect)
			*sp = CheckInventory(Activator, GetStr(READ_INT32(ip)));
			ip += 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Spawn)
			STUB(PCD_Spawn)
			//FIXME implement this
			//sp[-6] - type name, string
			//sp[-5] - x, fixed point
			//sp[-4] - y, fixed point
			//sp[-3] - z, fixed point
			//sp[-2] - TID
			//sp[-1] - angle, 256 as full circle
			//Pushes result.
			sp[-6] = 0;
			sp -= 5;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SpawnDirect)
			STUB(PCD_SpawnDirect)
			//FIXME implement this
			//READ_INT32(ip) - type name, string
			//READ_INT32(ip + 4) - x, fixed point
			//READ_INT32(ip + 8) - y, fixed point
			//READ_INT32(ip + 12) - z, fixed point
			//READ_INT32(ip + 16) - TID
			//READ_INT32(ip + 20) - angle, 256 as full circle
			//Pushes result
			*sp = 0;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SpawnSpot)
			STUB(PCD_SpawnSpot)
			//FIXME implement this
			//sp[-4] - type name, string
			//sp[-3] - TID of the spot
			//sp[-2] - TID
			//sp[-1] - angle, 256 as full circle
			//Pushes result
			sp[-4] = 0;
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SpawnSpotDirect)
			STUB(PCD_SpawnSpotDirect)
			//FIXME implement this
			//READ_INT32(ip) - type name, string.
			//READ_INT32(ip + 4) - TID of the spot
			//READ_INT32(ip + 8) - TID
			//READ_INT32(ip + 12) - angle, 256 as full circle
			//Pushes result
			*sp = 0;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetMusic)
			SV_ChangeMusic(GetStr(sp[-3]));
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetMusicDirect)
			SV_ChangeMusic(GetStr(READ_INT32(ip)));
			ip += 12;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LocalSetMusic)
			if (Activator && Activator->EntityFlags & VEntity::EF_IsPlayer)
			{
				SV_ChangeLocalMusic(Activator->Player, GetStr(sp[-3]));
			}
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LocalSetMusicDirect)
			if (Activator && Activator->EntityFlags & VEntity::EF_IsPlayer)
			{
				SV_ChangeLocalMusic(Activator->Player, GetStr(READ_INT32(ip)));
			}
			ip += 12;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintFixed)
			PrintStr += VStr(float(sp[-1]) / float(0x10000));
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintLocalised)
			PrintStr += GLanguage[GetStr(sp[-1])];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MoreHudMessage)
			PrintStr = PrintStr.EvalEscapeSequences();
			optstart = NULL;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OptHudMessage)
			optstart = sp;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EndHudMessage)
		ACSVM_CASE(PCD_EndHudMessageBold)
			STUB(PCD_EndHudMessage or PCD_EndHudMessageBold)
			//FIXME implement this
			if (!optstart)
			{
				optstart = sp;
			}
			//optstart[-6] - type
			//optstart[-5] - ID
			//optstart[-4] - colour
			//optstart[-3] - x, fixed point
			//optstart[-2] - y, fixed point
			//optstart[-1] - hold time, fixed point
			if (cmd != PCD_EndHudMessageBold &&
				Activator && Activator->EntityFlags & VEntity::EF_IsPlayer)
			{
				Activator->Player->Printf(*PrintStr);
			}
			else
			{
				BroadcastCentrePrint(*PrintStr);
			}
			sp = optstart - 6;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetFont)
			STUB(PCD_SetFont)
			//FIXME implement this
			//sp[-1] - font name, string
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetFontDirect)
			STUB(PCD_SetFontDirect)
			//FIXME implement this
			//READ_INT32(ip) - font name, string
			ip += 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushByte)
			*sp = *ip;
			sp++;
			ip++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec1DirectB)
			Level->eventExecuteActionSpecial(ip[0], ip[1], 0, 0, 0, 0, line,
				side, Activator);
			ip += 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec2DirectB)
			Level->eventExecuteActionSpecial(ip[0], ip[1], ip[2], 0, 0, 0,
				line, side, Activator);
			ip += 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec3DirectB)
			Level->eventExecuteActionSpecial(ip[0], ip[1], ip[2], ip[3], 0,
				0, line, side, Activator);
			ip += 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec4DirectB)
			Level->eventExecuteActionSpecial(ip[0], ip[1], ip[2], ip[3],
				ip[4], 0, line, side, Activator);
			ip += 5;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec5DirectB)
			Level->eventExecuteActionSpecial(ip[0], ip[1], ip[2], ip[3],
				ip[4], ip[5], line, side, Activator);
			ip += 6;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DelayDirectB)
			DelayTime = float(*ip) / 35.0;
			ip++;
			action = SCRIPT_Stop;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_RandomDirectB)
			*sp = ip[0] + (vint32)(Random() * (ip[1] - ip[0] + 1));
			ip += 2;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushBytes)
			for (int i = 0; i < ip[0]; i++)
				sp[i] = ip[i + 1];
			sp += ip[0];
			ip += ip[0] + 1;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Push2Bytes)
			sp[0] = ip[0];
			sp[1] = ip[1];
			ip += 2;
			sp += 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Push3Bytes)
			sp[0] = ip[0];
			sp[1] = ip[1];
			sp[2] = ip[2];
			ip += 3;
			sp += 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Push4Bytes)
			sp[0] = ip[0];
			sp[1] = ip[1];
			sp[2] = ip[2];
			sp[3] = ip[3];
			ip += 4;
			sp += 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Push5Bytes)
			sp[0] = ip[0];
			sp[1] = ip[1];
			sp[2] = ip[2];
			sp[3] = ip[3];
			sp[4] = ip[4];
			ip += 5;
			sp += 5;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetThingSpecial)
			{
				int searcher = -1;
				for (VEntity* Ent = Level->eventFindMobjFromTID(sp[-7], &searcher);
					Ent; Ent = Level->eventFindMobjFromTID(sp[-7], &searcher))
				{
					Ent->Special = sp[-6];
					Ent->Args[0] = sp[-5];
					Ent->Args[1] = sp[-4];
					Ent->Args[2] = sp[-3];
					Ent->Args[3] = sp[-2];
					Ent->Args[4] = sp[-1];
				}
				sp -= 7;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AssignGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] = sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushGlobalVar)
			*sp = GlobalVars[READ_BYTE_OR_INT32];
			INC_BYTE_OR_INT32;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AddGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] += sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SubGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] -= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MulGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] *= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DivGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] /= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ModGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] %= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IncGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32]++;
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DecGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32]--;
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_FadeTo)
			STUB(PCD_FadeTo)
			//FIXME implement this
			//sp[-5] - r
			//sp[-4] - g
			//sp[-3] - b
			//sp[-2] - a
			//sp[-1] - time, fixed point
			sp -= 5;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_FadeRange)
			STUB(PCD_FadeRange)
			//FIXME implement this
			//sp[-9] - r1
			//sp[-8] - g1
			//sp[-7] - b1
			//sp[-6] - a1
			//sp[-5] - r2
			//sp[-4] - g2
			//sp[-3] - b2
			//sp[-2] - a2
			//sp[-1] - time, fixed point
			sp -= 9;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CancelFade)
			STUB(PCD_CancelFade)
			//FIXME implement this
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayMovie)
			STUB(PCD_PlayMovie)
			//FIXME implement this
			//sp[-1] - movie name, string
			//Pushes result
			sp[-1] = 0;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetFloorTrigger)
			Level->eventStartPlaneWatcher(Activator, line, side, false,
				sp[-8], sp[-7], sp[-6], sp[-5], sp[-4], sp[-3], sp[-2],
				sp[-1]);
			sp -= 8;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetCeilingTrigger)
			Level->eventStartPlaneWatcher(Activator, line, side, true,
				sp[-8], sp[-7], sp[-6], sp[-5], sp[-4], sp[-3], sp[-2],
				sp[-1]);
			sp -= 8;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetActorX)
			{
				VEntity* Ent = EntityFromTID(sp[-1], Activator);
				if (!Ent)
				{
					sp[-1] = 0;
				}
				else
				{
					sp[-1] = vint32(Ent->Origin.x * 0x10000);
				}
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetActorY)
			{
				VEntity* Ent = EntityFromTID(sp[-1], Activator);
				if (!Ent)
				{
					sp[-1] = 0;
				}
				else
				{
					sp[-1] = vint32(Ent->Origin.y * 0x10000);
				}
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetActorZ)
			{
				VEntity* Ent = EntityFromTID(sp[-1], Activator);
				if (!Ent)
				{
					sp[-1] = 0;
				}
				else
				{
					sp[-1] = vint32(Ent->Origin.z * 0x10000);
				}
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_StartTranslation)
			STUB(PCD_StartTranslation)
			//FIXME implement this
			//sp[-1] - index
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TranslationRange1)
			STUB(PCD_TranslationRange1)
			//FIXME implement this
			//sp[-4] - start
			//sp[-3] - end
			//sp[-2] - pal1
			//sp[-1] - pal2
			sp -= 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TranslationRange2)
			STUB(PCD_TranslationRange2)
			//FIXME implement this
			//sp[-8] - start
			//sp[-7] - end
			//sp[-6] - r1
			//sp[-5] - g1
			//sp[-4] - b1
			//sp[-3] - r2
			//sp[-2] - g2
			//sp[-1] - b2
			sp -= 8;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EndTranslation)
			STUB(PCD_EndTranslation)
			//FIXME implement this
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Call)
		ACSVM_CASE(PCD_CallDiscard)
			{
				int funcnum;
				int i;
				VAcsObject* object = ActiveObject;

				funcnum = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				VAcsFunction* func = ActiveObject->GetFunction(funcnum,
					object);
				if (!func)
				{
					GCon->Logf("Function %d in script %d out of range",
						funcnum, number);
					action = SCRIPT_Terminate;
					ACSVM_BREAK_STOP;
				}
				if ((sp - stack) + func->LocalCount + 64 > ACS_STACK_DEPTH)
				{
					// 64 is the margin for the function's working space
					GCon->Logf("Out of stack space in script %d", number);
					action = SCRIPT_Terminate;
					ACSVM_BREAK_STOP;
				}
				//	The function's first argument is also its first local
				// variable.
				locals = sp - func->ArgCount;
				//	Make space on the stack for any other variables the
				// function uses.
				for (i = 0; i < func->LocalCount; i++)
				{
					sp[i] = 0;
				}
				sp += i;
				((VAcsCallReturn*)sp)->ReturnAddress =
					ActiveObject->PtrToOffset(ip);
				((VAcsCallReturn*)sp)->ReturnFunction = activeFunction;
				((VAcsCallReturn*)sp)->ReturnObject = ActiveObject;
				((VAcsCallReturn*)sp)->bDiscardResult = (cmd == PCD_CallDiscard);
				sp += sizeof(VAcsCallReturn) / sizeof(vint32);
				ActiveObject = object;
				ip = ActiveObject->OffsetToPtr(func->Address);
				activeFunction = func;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ReturnVoid)
		ACSVM_CASE(PCD_ReturnVal)
			{
				int value;
				VAcsCallReturn* retState;

				if (cmd == PCD_ReturnVal)
				{
					value = sp[-1];
					sp--;
				}
				else
				{
					value = 0;
				}
				sp -= sizeof(VAcsCallReturn) / sizeof(vint32);
				retState = (VAcsCallReturn*)sp;
				sp -= activeFunction->ArgCount + activeFunction->LocalCount;
				ActiveObject = retState->ReturnObject;
				activeFunction = retState->ReturnFunction;
				ip = ActiveObject->OffsetToPtr(retState->ReturnAddress);
				fmt = ActiveObject->GetFormat();
				if (!activeFunction)
				{
					locals = LocalVars;
				}
				else
				{
					locals = sp - activeFunction->ArgCount -
						activeFunction->LocalCount - sizeof(VAcsCallReturn) /
						sizeof(vint32);
				}
				if (!retState->bDiscardResult)
				{
					*sp = value;
					sp++;
				}
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushMapArray)
			sp[-1] = ActiveObject->GetArrayVal(*ActiveObject->MapVars[
				READ_BYTE_OR_INT32], sp[-1]);
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AssignMapArray)
			ActiveObject->SetArrayVal(*ActiveObject->MapVars[
				READ_BYTE_OR_INT32], sp[-2], sp[-1]);
			INC_BYTE_OR_INT32;
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AddMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) + sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SubMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) - sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MulMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) * sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DivMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) / sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ModMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) % sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IncMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-1],
					ActiveObject->GetArrayVal(ANum, sp[-1]) + 1);
				INC_BYTE_OR_INT32;
				sp--;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DecMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-1],
					ActiveObject->GetArrayVal(ANum, sp[-1]) - 1);
				INC_BYTE_OR_INT32;
				sp--;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Dup)
			*sp = sp[-1];
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Swap)
			{
				int tmp = sp[-2];
				sp[-2] = sp[-1];
				sp[-1] = tmp;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Sin)
			sp[-1] = vint32(msin(float(sp[-1]) * 360.0 / 0x10000) * 0x10000);
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Cos)
			sp[-1] = vint32(mcos(float(sp[-1]) * 360.0 / 0x10000) * 0x10000);
			ACSVM_BREAK;

		ACSVM_CASE(PCD_VectorAngle)
			sp[-2] = vint32(matan(float(sp[-1]) / float(0x10000),
				float(sp[-2]) / float(0x10000)) / 360.0 * 0x10000);
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CheckWeapon)
			STUB(PCD_CheckWeapon)
			//FIXME implement this
			//sp[-1] - weapon name, string
			//Pushes result
			sp[-1] = 0;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetWeapon)
			STUB(PCD_SetWeapon)
			//FIXME implement this
			//sp[-1] - weapon name, string
			//Pushes result
			sp[-1] = 0;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TagString)
			sp[-1] |= ActiveObject->GetLibraryID();
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushWorldArray)
			sp[-1] = WorldArrays[READ_BYTE_OR_INT32].GetElemVal(sp[-1]);
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AssignWorldArray)
			WorldArrays[READ_BYTE_OR_INT32].SetElemVal(sp[-2], sp[-1]);
			INC_BYTE_OR_INT32;
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AddWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) + sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SubWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) - sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MulWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) * sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DivWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) / sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ModWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) % sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IncWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-1],
					WorldArrays[ANum].GetElemVal(sp[-1]) + 1);
				INC_BYTE_OR_INT32;
				sp--;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DecWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-1],
					WorldArrays[ANum].GetElemVal(sp[-1]) - 1);
				INC_BYTE_OR_INT32;
				sp--;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PushGlobalArray)
			sp[-1] = GlobalArrays[READ_BYTE_OR_INT32].GetElemVal(sp[-1]);
			INC_BYTE_OR_INT32;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AssignGlobalArray)
			GlobalArrays[READ_BYTE_OR_INT32].SetElemVal(sp[-2], sp[-1]);
			INC_BYTE_OR_INT32;
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AddGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) + sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SubGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) - sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_MulGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) * sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DivGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) / sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ModGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) % sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_IncGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-1],
					GlobalArrays[ANum].GetElemVal(sp[-1]) + 1);
				INC_BYTE_OR_INT32;
				sp--;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_DecGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-1],
					GlobalArrays[ANum].GetElemVal(sp[-1]) - 1);
				INC_BYTE_OR_INT32;
				sp--;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetMarineWeapon)
			STUB(PCD_SetMarineWeapon)
			//FIXME implement this
			//sp[-2] - TID
			//sp[-1] - weapon name, string
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetActorProperty)
			STUB(PCD_SetActorProperty)
			//FIXME implement this
			//sp[-3] - TID
			//sp[-2] - property
			//sp[-1] - value
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetActorProperty)
			STUB(PCD_GetActorProperty)
			//FIXME implement this
			//sp[-2] - TID
			//sp[-1] - property
			//Pushes result
			sp[-2] = 0;
			sp -= 1;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerNumber)
			*sp = Activator && (Activator->EntityFlags & VEntity::EF_IsPlayer) ?
				SV_GetPlayerNum(Activator->Player) : -1;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ActivatorTID)
			*sp = Activator ? Activator->TID : 0;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetMarineSprite)
			STUB(PCD_SetMarineSprite)
			//FIXME implement this
			//sp[-2] - TID
			//sp[-1] - class name, string
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetScreenWidth)
			*sp = 640;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetScreenHeight)
			*sp = 480;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ThingProjectile2)
			Level->eventEV_ThingProjectile(sp[-7], sp[-6], sp[-5], sp[-4],
				sp[-3], sp[-2], sp[-1]);
			sp -= 7;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_StrLen)
			sp[-1] = VStr::Utf8Length(GetStr(sp[-1]));
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetHudSize)
			STUB(PCD_SetHudSize)
			//FIXME implement this
			//sp[-3] - hud width, abs-ed
			//sp[-2] - hud height, abs-ed
			//sp[-1] - cover status bar
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetCvar)
			sp[-1] = VCvar::GetInt(GetStr(sp[-1]));
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CaseGotoSorted)
			//	The count and jump table are 4-byte aligned.
			if (ActiveObject->PtrToOffset(ip) & 3)
			{
				ip += 4 - (ActiveObject->PtrToOffset(ip) & 3);
			}
			{
				int numcases = READ_INT32(ip);
				int min = 0, max = numcases - 1;
				while (min <= max)
				{
					int mid = (min + max) / 2;
					int caseval = READ_INT32(ip + 4 + mid * 8);
					if (caseval == sp[-1])
					{
						ip = ActiveObject->OffsetToPtr(READ_INT32(ip + 8 + mid * 8));
						sp--;
						ACSVM_BREAK;
					}
					else if (caseval < sp[-1])
					{
						min = mid + 1;
					}
					else
					{
						max = mid - 1;
					}
				}
				if (min > max)
				{
					// The case was not found, so go to the next instruction.
					ip += 4 + numcases * 8;
				}
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetResultValue)
			resultValue = sp[-1];
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetLineRowOffset)
			*sp = line ? (vint32)XLevel->Sides[line->sidenum[0]].rowoffset : 0;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetActorFloorZ)
			{
				VEntity* Ent = EntityFromTID(sp[-1], Activator);
				sp[-1] = Ent ? vint32(Ent->FloorZ * 0x10000) : 0;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetActorAngle)
			{
				VEntity* Ent = EntityFromTID(sp[-1], Activator);
				sp[-1] = Ent ? vint32(Ent->Angles.yaw * 0x10000 / 360) &
					0xffff : 0;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetSectorFloorZ)
			{
				int SNum = FindSectorFromTag(sp[-3], -1);
				sp[-3] = SNum >= 0 ? vint32(XLevel->Sectors[SNum].floor.
					GetPointZ(sp[-2], sp[-1]) * 0x10000) : 0;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetSectorCeilingZ)
			{
				int SNum = FindSectorFromTag(sp[-3], -1);
				sp[-3] = SNum >= 0 ? vint32(XLevel->Sectors[SNum].ceiling.
					GetPointZ(sp[-2], sp[-1]) * 0x10000) : 0;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSpec5Result)
			sp[-5] = Level->eventExecuteActionSpecial(READ_BYTE_OR_INT32,
				sp[-5], sp[-4], sp[-3], sp[-2], sp[-1], line, side,
				Activator);
			INC_BYTE_OR_INT32;
			sp -= 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetSigilPieces)
			*sp = Activator ? Activator->eventGetSigilPieces() : 0;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetLevelInfo)
			switch (sp[-1])
			{
			case LEVELINFO_PAR_TIME:
				sp[-1] = Level->ParTime;
				break;
			case LEVELINFO_SUCK_TIME:
				sp[-1] = Level->SuckTime;
				break;
			case LEVELINFO_CLUSTERNUM:
				sp[-1] = Level->Cluster;
				break;
			case LEVELINFO_LEVELNUM:
				sp[-1] = Level->LevelNum;
				break;
			case LEVELINFO_TOTAL_SECRETS:
				sp[-1] = Level->TotalSecret;
				break;
			case LEVELINFO_FOUND_SECRETS:
				sp[-1] = Level->CurrentSecret;
				break;
			case LEVELINFO_TOTAL_ITEMS:
				sp[-1] = Level->TotalItems;
				break;
			case LEVELINFO_FOUND_ITEMS:
				sp[-1] = Level->CurrentItems;
				break;
			case LEVELINFO_TOTAL_MONSTERS:
				sp[-1] = Level->TotalKills;
				break;
			case LEVELINFO_KILLED_MONSTERS:
				sp[-1] = Level->CurrentKills;
				break;
			default:
				sp[-1] = 0;
				break;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ChangeSky)
			SV_ChangeSky(GetStr(sp[-2]), GetStr(sp[-1]));
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerInGame)
			sp[-1] = (sp[-1] < 0 || sp[-1] >= MAXPLAYERS) ? false :
				(Level->Game->Players[sp[-1]] && (Level->Game->Players[
				sp[-1]]->PlayerFlags & VBasePlayer::PF_Spawned));
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerIsBot)
			sp[-1] = (sp[-1] < 0 || sp[-1] >= MAXPLAYERS) ? false :
				Level->Game->Players[sp[-1]] && Level->Game->Players[
				sp[-1]]->PlayerFlags & VBasePlayer::PF_Spawned &&
				Level->Game->Players[sp[-1]]->PlayerFlags &
				VBasePlayer::PF_IsBot;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetCameraToTexture)
			STUB(PCD_SetCameraToTexture)
			//sp[-3] - TID
			//sp[-2] - Texture
			//sp[-1] - FOV
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EndLog)
			PrintStr = PrintStr.EvalEscapeSequences();
			GCon->Log(PrintStr);
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetAmmoCapacity)
			STUB(PCD_GetAmmoCapacity)
			//sp[-1] - Type name
			//Pushes result
			sp[-1] = 0;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetAmmoCapacity)
			STUB(PCD_SetAmmoCapacity)
			//sp[-2] - Type name
			//sp[-1] - Amount
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintMapCharArray)
			{
				int ANum = *ActiveObject->MapVars[sp[-1]];
				int Idx = sp[-2];
				for (int c = ActiveObject->GetArrayVal(ANum, Idx); c;
					c = ActiveObject->GetArrayVal(ANum, Idx))
				{
					PrintStr += (char)c;
					Idx++;
				}
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintWorldCharArray)
			{
				int ANum = *ActiveObject->MapVars[sp[-1]];
				int Idx = sp[-2];
				for (int c = WorldArrays[ANum].GetElemVal(Idx); c;
					c = WorldArrays[ANum].GetElemVal(Idx))
				{
					PrintStr += (char)c;
					Idx++;
				}
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PrintGlobalCharArray)
			{
				int ANum = *ActiveObject->MapVars[sp[-1]];
				int Idx = sp[-2];
				for (int c = GlobalArrays[ANum].GetElemVal(Idx); c;
					c = GlobalArrays[ANum].GetElemVal(Idx))
				{
					PrintStr += (char)c;
					Idx++;
				}
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetActorAngle)
			{
				int searcher = -1;
				for (VEntity* Ent = Level->eventFindMobjFromTID(sp[-2], &searcher);
					Ent; Ent = Level->eventFindMobjFromTID(sp[-2], &searcher))
				{
					Ent->Angles.yaw = (float)(sp[-1] & 0xffff) * 360.0 / (float)0x10000;
				}
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SpawnProjectile)
			STUB(PCD_SpawnProjectile)
			//sp[-7] - TID
			//sp[-6] - Type name
			//sp[-5] - angle
			//sp[-4] - speed
			//sp[-3] - vspeed
			//sp[-2] - Gravity
			//sp[-1] - New TID
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetSectorLightLevel)
			{
				int SNum = FindSectorFromTag(sp[-1], -1);
				sp[-1] = SNum >= 0 ? XLevel->Sectors[SNum].params.lightlevel : 0;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetActorCeilingZ)
			{
				VEntity* Ent = EntityFromTID(sp[-1], Activator);
				sp[-1] = Ent ? vint32(Ent->CeilingZ * 0x10000) : 0;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetActorPosition)
			{
				VEntity* Ent = EntityFromTID(sp[-5], Activator);
				sp[-5] = Ent ? Ent->eventMoveThing(TVec(
					(float)sp[-4] / (float)0x10000,
					(float)sp[-3] / (float)0x10000,
					(float)sp[-2] / (float)0x10000), !!sp[-1]) : 0;
				sp -= 4;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ClearActorInventory)
			STUB(PCD_ClearActorInventory)
			//sp[-1] - TID
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GiveActorInventory)
			STUB(PCD_GiveActorInventory)
			//sp[-3] - TID
			//sp[-2] - Item name
			//sp[-1] - Count
			GiveInventory(EntityFromTID(sp[-3], NULL), GetStr(sp[-2]), sp[-1]);
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TakeActorInventory)
			STUB(PCD_TakeActorInventory)
			//sp[-3] - TID
			//sp[-2] - Item name
			//sp[-1] - Count
			TakeInventory(EntityFromTID(sp[-3], NULL), GetStr(sp[-2]), sp[-1]);
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CheckActorInventory)
			STUB(PCD_CheckActorInventory)
			//sp[-2] - TID
			//sp[-1] - Item name
			sp[-2] = CheckInventory(EntityFromTID(sp[-2], NULL), GetStr(sp[-1]));
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ThingCountName)
			STUB(PCD_ThingCountName)
			//sp[-2] - Type name
			//sp[-1] - TID
			//Pushes result.
			sp[-2] = 0;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SpawnSpotFacing)
			STUB(PCD_SpawnSpotFacing)
			//sp[-3] - Type name
			//sp[-2] - Spot TID
			//sp[-1] - New TID
			//Pushes result.
			sp[-3] = 0;
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerClass)
			if (sp[-1] < 0 || sp[-1] >= MAXPLAYERS || !Level->Game->Players[sp[-1]] ||
				!(Level->Game->Players[sp[-1]]->PlayerFlags & VBasePlayer::PF_Spawned))
			{
				sp[-1] = -1;
			}
			else
			{
				sp[-1] = Level->Game->Players[sp[-1]]->PClass;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndScriptVar)
			locals[READ_BYTE_OR_INT32] &= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] &= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndWorldVar)
			WorldVars[READ_BYTE_OR_INT32] &= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] &= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) & sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) & sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AndGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) & sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EOrScriptVar)
			locals[READ_BYTE_OR_INT32] ^= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EOrMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] ^= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EOrWorldVar)
			WorldVars[READ_BYTE_OR_INT32] ^= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EOrGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] ^= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EOrMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) ^ sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EOrWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) ^ sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EOrGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) ^ sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrScriptVar)
			locals[READ_BYTE_OR_INT32] |= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] |= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrWorldVar)
			WorldVars[READ_BYTE_OR_INT32] |= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] |= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) | sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) | sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_OrGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) | sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSScriptVar)
			locals[READ_BYTE_OR_INT32] <<= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] <<= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSWorldVar)
			WorldVars[READ_BYTE_OR_INT32] <<= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] <<= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) << sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) << sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LSGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) << sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RSScriptVar)
			locals[READ_BYTE_OR_INT32] >>= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RSMapVar)
			*ActiveObject->MapVars[READ_BYTE_OR_INT32] >>= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RSWorldVar)
			WorldVars[READ_BYTE_OR_INT32] >>= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RSGlobalVar)
			GlobalVars[READ_BYTE_OR_INT32] >>= sp[-1];
			INC_BYTE_OR_INT32;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RSMapArray)
			{
				int ANum = *ActiveObject->MapVars[READ_BYTE_OR_INT32];
				ActiveObject->SetArrayVal(ANum, sp[-2],
					ActiveObject->GetArrayVal(ANum, sp[-2]) >> sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RSWorldArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				WorldArrays[ANum].SetElemVal(sp[-2],
					WorldArrays[ANum].GetElemVal(sp[-2]) >> sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_RSGlobalArray)
			{
				int ANum = READ_BYTE_OR_INT32;
				GlobalArrays[ANum].SetElemVal(sp[-2],
					GlobalArrays[ANum].GetElemVal(sp[-2]) >> sp[-1]);
				INC_BYTE_OR_INT32;
				sp -= 2;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetPlayerInfo)
			STUB(PCD_GetPlayerInfo)
			//sp[-2] - Player num
			//sp[-1] - Info type
			//Pushes result.
			sp[-2] = -1;
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ChangeLevel)
			STUB(PCD_ChangeLevel)
			//sp[-4] - Level name
			//sp[-3] - Position
			//sp[-2] - Skill
			//sp[-1] - Flags
			sp -= 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SectorDamage)
			STUB(PCD_SectorDamage)
			//sp[-5] - Tag
			//sp[-4] - Amount
			//sp[-3] - Damage type
			//sp[-2] - Protection inventory class name
			//sp[-1] - Flags
			sp -= 5;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ReplaceTextures)
			if (~sp[-1] & (NOT_TOP | NOT_MIDDLE | NOT_BOTTOM))
			{
				int FromTex = GTextureManager.NumForName(VName(GetStr(sp[-3]),
					VName::AddLower8), TEXTYPE_Wall, true);
				int ToTex = GTextureManager.NumForName(VName(GetStr(sp[-2]),
					VName::AddLower8), TEXTYPE_Wall, true);
				for (int i = 0; i < XLevel->NumSides; i++)
				{
					if (!(sp[-1] & NOT_TOP) &&
						XLevel->Sides[i].toptexture == FromTex)
					{
						XLevel->Sides[i].toptexture = ToTex;
					}
					if (!(sp[-1] & NOT_MIDDLE) &&
						XLevel->Sides[i].midtexture == FromTex)
					{
						XLevel->Sides[i].midtexture = ToTex;
					}
					if (!(sp[-1] & NOT_BOTTOM) &&
						XLevel->Sides[i].bottomtexture == FromTex)
					{
						XLevel->Sides[i].bottomtexture = ToTex;
					}
				}
			}
			if (~sp[-1] & (NOT_FLOOR | NOT_CEILING))
			{
				int FromTex = GTextureManager.NumForName(VName(GetStr(sp[-3]),
					VName::AddLower8), TEXTYPE_Flat, true);
				int ToTex = GTextureManager.NumForName(VName(GetStr(sp[-2]),
					VName::AddLower8), TEXTYPE_Flat, true);
				for (int i = 0; i < XLevel->NumSectors; i++)
				{
					if (!(sp[-1] & NOT_FLOOR) &&
						XLevel->Sectors[i].floor.pic == FromTex)
					{
						XLevel->Sectors[i].floor.pic = ToTex;
					}
					if (!(sp[-1] & NOT_CEILING) &&
						XLevel->Sectors[i].ceiling.pic == FromTex)
					{
						XLevel->Sectors[i].ceiling.pic = ToTex;
					}
				}
			}
			sp -= 3;
			ACSVM_BREAK;

		//	These p-codes are not supported. They will terminate script.
		ACSVM_CASE(PCD_PlayerBlueSkull)
		ACSVM_CASE(PCD_PlayerRedSkull)
		ACSVM_CASE(PCD_PlayerYellowSkull)
		ACSVM_CASE(PCD_PlayerMasterSkull)
		ACSVM_CASE(PCD_PlayerBlueCard)
		ACSVM_CASE(PCD_PlayerRedCard)
		ACSVM_CASE(PCD_PlayerYellowCard)
		ACSVM_CASE(PCD_PlayerMasterCard)
		ACSVM_CASE(PCD_PlayerBlackSkull)
		ACSVM_CASE(PCD_PlayerSilverSkull)
		ACSVM_CASE(PCD_PlayerGoldSkull)
		ACSVM_CASE(PCD_PlayerBlackCard)
		ACSVM_CASE(PCD_PlayerSilverCard)
		ACSVM_CASE(PCD_PlayerOnTeam)
		ACSVM_CASE(PCD_PlayerTeam)
		ACSVM_CASE(PCD_PlayerExpert)
		ACSVM_CASE(PCD_BlueTeamCount)
		ACSVM_CASE(PCD_RedTeamCount)
		ACSVM_CASE(PCD_BlueTeamScore)
		ACSVM_CASE(PCD_RedTeamScore)
		ACSVM_CASE(PCD_IsOneFlagCTF)
		ACSVM_CASE(PCD_LSpec6)
		ACSVM_CASE(PCD_LSpec6Direct)
		ACSVM_CASE(PCD_Team2FragPoints)
		ACSVM_CASE(PCD_ConsoleCommand)
		ACSVM_CASE(PCD_SetStyle)
		ACSVM_CASE(PCD_SetStyleDirect)
		ACSVM_CASE(PCD_WriteToIni)
		ACSVM_CASE(PCD_GetFromIni)
		ACSVM_CASE(PCD_GrabInput)
		ACSVM_CASE(PCD_SetMousePointer)
		ACSVM_CASE(PCD_MoveMousePointer)
			GCon->Logf(NAME_Dev, "Unsupported ACS p-code %d", cmd);
			action = SCRIPT_Terminate;
			ACSVM_BREAK_STOP;

		ACSVM_DEFAULT
			Host_Error("Illegal ACS opcode %d", cmd);
		}
	} while  (action == SCRIPT_Continue);
#if USE_COMPUTED_GOTO
LblFuncStop:
#endif
	InstructionPointer = ip;
	if (action == SCRIPT_Terminate)
	{
		if (info->RunningScript == this)
		{
			info->RunningScript = NULL;
		}
		DestroyThinker();
	}
	return resultValue;
	unguard;
}

//==========================================================================
//
//  FindSectorFromTag
//
//	RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
//==========================================================================

int VAcs::FindSectorFromTag(int tag, int start)
{
	guard(VAcs::FindSectorFromTag);
	for (int i = start + 1; i < XLevel->NumSectors; i++)
		if (XLevel->Sectors[i].tag == tag)
			return i;
	return -1;
	unguard;
}

//============================================================================
//
//	VAcs::GiveInventory
//
//============================================================================

void VAcs::GiveInventory(VEntity* Activator, const char* AType, int Amount)
{
	guard(VAcs::GiveInventory);
	if (Amount <= 0)
	{
		return;
	}
	const char* Type = AType;
	if (VStr::Cmp(Type, "Armor") == 0)
	{
		Type = "BasicArmor";
	}
	if (Activator)
	{
		Activator->eventGiveInventory(Type, Amount);
	}
	else
	{
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (Level->Game->Players[i] &&
				Level->Game->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned)
				Level->Game->Players[i]->MO->eventGiveInventory(Type, Amount);
		}
	}
	return;
	unguard;
}

//============================================================================
//
//	VAcs::TakeInventory
//
//============================================================================

void VAcs::TakeInventory(VEntity* Activator, const char* AType, int Amount)
{
	guard(VAcs::TakeInventory);
	if (Amount <= 0)
	{
		return;
	}
	const char* Type = AType;
	if (VStr::Cmp(Type, "Armor") == 0)
	{
		Type = "BasicArmor";
	}
	if (Activator)
	{
		Activator->eventTakeInventory(Type, Amount);
	}
	else
	{
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (Level->Game->Players[i] &&
				Level->Game->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned)
				Level->Game->Players[i]->MO->eventTakeInventory(Type, Amount);
		}
	}
	unguard;
}

//============================================================================
//
//	VAcs::CheckInventory
//
//============================================================================

int VAcs::CheckInventory(VEntity* Activator, const char* AType)
{
	guard(VAcs::CheckInventory);
	if (!Activator)
		return 0;

	const char* Type = AType;
	if (VStr::Cmp(Type, "Armor") == 0)
	{
		Type = "BasicArmor";
	}
	else if (!VStr::Cmp(Type, "Health"))
	{
		return Activator->Health;
	}
	return Activator->eventCheckInventory(Type);
	unguard;
}

//==========================================================================
//
//	VAcsGlobal::VAcsGlobal
//
//==========================================================================

VAcsGlobal::VAcsGlobal()
{
	memset(WorldVars, 0, sizeof(WorldVars));
	memset(GlobalVars, 0, sizeof(GlobalVars));
}

//==========================================================================
//
//	VAcsGlobal::Serialise
//
//==========================================================================

void VAcsGlobal::Serialise(VStream& Strm)
{
	guard(VAcsGlobal::Serialise);
	for (int i = 0; i < MAX_ACS_WORLD_VARS; i++)
	{
		Strm << STRM_INDEX(WorldVars[i]);
	}
	for (int i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
	{
		Strm << STRM_INDEX(GlobalVars[i]);
	}
	for (int i = 0; i < MAX_ACS_WORLD_VARS; i++)
	{
		WorldArrays[i].Serialise(Strm);
	}
	for (int i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
	{
		GlobalArrays[i].Serialise(Strm);
	}

	vint32 NumAcsStore = Store.Num();
	Strm << STRM_INDEX(NumAcsStore);
	if (Strm.IsLoading())
	{
		Store.SetNum(NumAcsStore);
	}
	for (int i = 0; i < NumAcsStore; i++)
	{
		Strm << Store[i].Map
			<< Store[i].Type
			<< Store[i].PlayerNum
			<< STRM_INDEX(Store[i].Script)
			<< STRM_INDEX(Store[i].Args[0])
			<< STRM_INDEX(Store[i].Args[1])
			<< STRM_INDEX(Store[i].Args[2]);
	}
	unguard;
}

//==========================================================================
//
//	Script ACS methods
//
//==========================================================================

IMPLEMENT_FUNCTION(VLevel, StartACS)
{
	P_GET_BOOL(WantResult);
	P_GET_BOOL(Always);
	P_GET_INT(side);
	P_GET_PTR(line_t, line);
	P_GET_REF(VEntity, activator);
	P_GET_INT(arg3);
	P_GET_INT(arg2);
	P_GET_INT(arg1);
	P_GET_INT(map);
	P_GET_INT(num);
	P_GET_SELF;
	RET_BOOL(Self->Acs->Start(num, map, arg1, arg2, arg3, activator, line,
		side, Always, WantResult));
}

IMPLEMENT_FUNCTION(VLevel, SuspendACS)
{
	P_GET_INT(map);
	P_GET_INT(number);
	P_GET_SELF;
	RET_BOOL(Self->Acs->Suspend(number, map));
}

IMPLEMENT_FUNCTION(VLevel, TerminateACS)
{
	P_GET_INT(map);
	P_GET_INT(number);
	P_GET_SELF;
	RET_BOOL(Self->Acs->Terminate(number, map));
}
