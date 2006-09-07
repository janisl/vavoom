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

#define ACS_STACK_DEPTH		4096

// TYPES -------------------------------------------------------------------

enum EACSFormat
{
	ACS_Old,
	ACS_Enhanced,
	ACS_LittleEnhanced,
	ACS_Unknown
};

enum EScriptAction
{
	SCRIPT_CONTINUE,
	SCRIPT_STOP,
	SCRIPT_TERMINATE,
};

enum EGameMode
{
	GAME_SINGLE_PLAYER,
	GAME_NET_COOPERATIVE,
	GAME_NET_DEATHMATCH
};

enum ETexturePosition
{
	TEXTURE_TOP,
	TEXTURE_MIDDLE,
	TEXTURE_BOTTOM
};

//	Script flags.
enum
{
	SCRIPTF_Net = 0x0001	//	Safe to "puke" in multiplayer.
};

enum aste_t
{
	ASTE_INACTIVE,
	ASTE_RUNNING,
	ASTE_SUSPENDED,
	ASTE_WAITINGFORTAG,
	ASTE_WAITINGFORPOLY,
	ASTE_WAITINGFORSCRIPT,
	ASTE_TERMINATING
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
	LEVELINFO_KILLED_MONSTERS
};

struct acsHeader_t
{
	char	marker[4];
	int		infoOffset;
	int		code;
};

struct acsInfo_t
{
	vuint16		number;
	vuint8		type;
	vuint8		argCount;
	vuint8*		Address;
	vuint16		Flags;
	vuint16		VarCount;
	vuint8		state;
	vint32		waitValue;
};

struct FACScriptFunction
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
class FACScriptsObject
{
private:
	struct FArrayInfo
	{
		vint32		Size;
		vint32*		Data;
	};

	EACSFormat			Format;

	int					LumpNum;
	int					LibraryID;

	int					DataSize;
	byte*				Data;

	byte*				Chunks;

	int					NumScripts;
	acsInfo_t*			Scripts;

	FACScriptFunction*	Functions;
	int					NumFunctions;

	int					NumStrings;
	char**				Strings;

	int					MapVarStore[MAX_ACS_MAP_VARS];

	int					NumArrays;
	FArrayInfo*			ArrayStore;
	int					NumTotalArrays;
	FArrayInfo**		Arrays;

	TArray<FACScriptsObject*>	Imports;

	static TArray<FACScriptsObject*>	LoadedObjects;

	void LoadOldObject();
	void LoadEnhancedObject();
	void UnencryptStrings();
	int FindFunctionName(const char* Name) const;
	int FindMapVarName(const char* Name) const;
	int FindMapArray(const char* Name) const;
	int FindStringInChunk(byte* Chunk, const char* Name) const;
	byte* FindChunk(const char* id) const;
	byte* NextChunk(byte* prev) const;
	void Serialise(VStream& Strm);
	void StartTypedACScripts(int Type);
	void ScriptFinished(int number);

public:
	int*				MapVars[MAX_ACS_MAP_VARS];

	FACScriptsObject(int Lump);
	~FACScriptsObject();

	vuint8* OffsetToPtr(int);
	int PtrToOffset(vuint8*);
	EACSFormat GetFormat() const
	{
		return Format;
	}
	int GetNumScripts() const
	{
		return NumScripts;
	}
	acsInfo_t& GetScriptInfo(int i)
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
	acsInfo_t* FindScript(int Number) const;
	FACScriptFunction* GetFunction(int funcnum, FACScriptsObject*& Object);
	int GetArrayVal(int ArrayIdx, int Index);
	void SetArrayVal(int ArrayIdx, int Index, int Value);

	static FACScriptsObject* StaticLoadObject(int Lump);
	static void StaticUnloadObjects();
	static acsInfo_t* StaticFindScript(int Number, FACScriptsObject*& Object);
	static const char* StaticGetString(int Index);
	static FACScriptsObject* StaticGetObject(int Index);
	static void StaticStartTypedACScripts(int Type);
	static void StaticSerialise(VStream& Strm);
	static void StaticScriptFinished(int number);
};

struct CallReturn
{
	int					ReturnAddress;
	FACScriptFunction*	ReturnFunction;
	FACScriptsObject*	ReturnObject;
	byte				bDiscardResult;
	byte				Pad[3];
};

class VACS : public VThinker
{
	DECLARE_CLASS(VACS, VThinker, 0)
	NO_DEFAULT_CONSTRUCTOR(VACS)

	VEntity*			Activator;
	line_t*				line;
	vint32 				side;
	vint32 				number;
	acsInfo_t*			info;
	float				DelayTime;
	vint32*				LocalVars;
	vuint8*				InstructionPointer;
	FACScriptsObject*	ActiveObject;

	void Destroy();
	void Serialise(VStream&);
	int RunScript(float);
	void Tick(float);

private:
	const char* GetStr(int Index)
	{
		return FACScriptsObject::StaticGetString(Index);
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

static bool AddToACSStore(const char* map, int number, int arg1, int arg2,
	int arg3);
static VACS* SpawnScript(acsInfo_t* Info, FACScriptsObject* Object,
	VEntity* Activator, line_t* Line, int Side, int Arg1, int Arg2, int Arg3,
	bool Delayed);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int WorldVars[MAX_ACS_WORLD_VARS];
int GlobalVars[MAX_ACS_GLOBAL_VARS];
FACSGrowingArray WorldArrays[MAX_ACS_WORLD_VARS];
FACSGrowingArray GlobalArrays[MAX_ACS_GLOBAL_VARS];
acsstore_t ACSStore[MAX_ACS_STORE+1]; // +1 for termination marker

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, ACS)

TArray<FACScriptsObject*>	FACScriptsObject::LoadedObjects;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FACScriptsObject::FACScriptsObject
//
//==========================================================================

FACScriptsObject::FACScriptsObject(int Lump)
{
	guard(FACScriptsObject::FACScriptsObject);
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
	if (W_LumpLength(Lump) < (int)sizeof(acsHeader_t))
    {
		GCon->Log("Behavior lump too small");
		return;
    }

	VStream* Strm = W_CreateLumpReaderNum(Lump);
	Data = new vuint8[Strm->TotalSize()];
	Strm->Serialise(Data, Strm->TotalSize());
	delete Strm;
	acsHeader_t* header = (acsHeader_t*)Data;

	//	Check header.
	if (header->marker[0] != 'A' || header->marker[1] != 'C' ||
		header->marker[2] != 'S')
	{
		return;
	}
	//	Determine format.
	switch (header->marker[3])
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
		vuint32 dirofs = LittleLong(header->infoOffset);
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
		Chunks = Data + LittleLong(header->infoOffset);
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
//	FACScriptsObject::~FACScriptsObject
//
//==========================================================================

FACScriptsObject::~FACScriptsObject()
{
	guard(FACScriptsObject::~FACScriptsObject);
	delete[] Scripts;
	delete[] Strings;
	for (int i = 0; i < NumArrays; i++)
		delete[] ArrayStore[i].Data;
	if (ArrayStore)
		delete[] ArrayStore;
	if (Arrays)
		delete[] Arrays;
	Z_Free(Data);
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::LoadOldObject
//
//==========================================================================

void FACScriptsObject::LoadOldObject()
{
	guard(FACScriptsObject::LoadOldObject);
	int i;
	int *buffer;
	acsInfo_t *info;
	acsHeader_t *header;

	header = (acsHeader_t*)Data;

	//	Load script info.
	buffer = (int*)(Data + LittleLong(header->infoOffset));
	NumScripts = LittleLong(*buffer++);
	if (NumScripts == 0)
	{
		//	Empty behavior lump
		return;
	}
	Scripts = new acsInfo_t[NumScripts];
	memset(Scripts, 0, NumScripts * sizeof(acsInfo_t));
	for (i = 0, info = Scripts; i < NumScripts; i++, info++)
	{
		info->number = LittleLong(*buffer) % 1000;
		info->type = LittleLong(*buffer) / 1000;
		buffer++;
		info->Address = OffsetToPtr(LittleLong(*buffer++));
		info->argCount = LittleLong(*buffer++);
		info->Flags = 0;
		info->VarCount = MAX_ACS_SCRIPT_VARS;
		info->state = ASTE_INACTIVE;
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
	LibraryID = LoadedObjects.Append(this) << 16;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::LoadEnhancedObject
//
//==========================================================================

void FACScriptsObject::LoadEnhancedObject()
{
	guard(FACScriptsObject::LoadEnhancedObject);
	int i;
	int *buffer;
	acsInfo_t *info;

	//	Load scripts.
	buffer = (int*)FindChunk("SPTR");
	if (Data[3] != 0)
	{
		NumScripts = LittleLong(buffer[1]) / 12;
		Scripts = new acsInfo_t[NumScripts];
		memset(Scripts, 0, NumScripts * sizeof(acsInfo_t));
		buffer += 2;

		for (i = 0, info = Scripts; i < NumScripts; i++, info++)
		{
			info->number = LittleShort(*(short*)buffer);
			info->type = LittleShort(((short*)buffer)[1]);
			buffer++;
			info->Address = OffsetToPtr(LittleLong(*buffer++));
			info->argCount = LittleLong(*buffer++);
			info->Flags = 0;
			info->VarCount = MAX_ACS_SCRIPT_VARS;
			info->state = ASTE_INACTIVE;
		}
	}
	else
	{
		NumScripts = LittleLong(buffer[1]) / 8;
		Scripts = new acsInfo_t[NumScripts];
		memset(Scripts, 0, NumScripts * sizeof(acsInfo_t));
		buffer += 2;

		for (i = 0, info = Scripts; i < NumScripts; i++, info++)
		{
			info->number = LittleShort(*(short*)buffer);
			info->type = ((byte*)buffer)[2];
			info->argCount = ((byte*)buffer)[3];
			buffer++;
			info->Address = OffsetToPtr(LittleLong(*buffer++));
			info->Flags = 0;
			info->VarCount = MAX_ACS_SCRIPT_VARS;
			info->state = ASTE_INACTIVE;
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
			acsInfo_t* info = FindScript(LittleShort(((word*)buffer)[0]));
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
			acsInfo_t* info = FindScript(LittleShort(((word*)buffer)[0]));
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
		Functions = (FACScriptFunction*)(buffer + 2);
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

	//	Initialize this object's map variable pointers to defaults. They can
	// be changed later once the imported modules are loaded.
	for (i = 0; i < MAX_ACS_MAP_VARS; i++)
	{
		MapVars[i] = &MapVarStore[i];
	}

	//	Initialize this object's map variables.
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
		buffer = (int*)NextChunk((byte*)buffer);
	}

	//	Create arrays.
	buffer = (int*)FindChunk("ARAY");
	if (buffer)
	{
		NumArrays = LittleLong(buffer[1]) / 8;
		ArrayStore = new FArrayInfo[NumArrays];
		memset(ArrayStore, 0, sizeof(*ArrayStore) * NumArrays);
		for (i = 0; i < NumArrays; ++i)
		{
			MapVarStore[LittleLong(buffer[2 + i * 2])] = i;
			ArrayStore[i].Size = LittleLong(buffer[3 + i * 2]);
			ArrayStore[i].Data = new vint32[ArrayStore[i].Size];
			memset(ArrayStore[i].Data, 0, ArrayStore[i].Size * sizeof(vint32));
		}
	}

	//	Initialize arrays.
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
		buffer = (int*)NextChunk((byte*)buffer);
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
		Arrays = new FArrayInfo*[NumTotalArrays];
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
	LibraryID = LoadedObjects.Append(this) << 16;

	//	Tag the library ID to any map variables that are initialized with
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
				FACScriptsObject* Object = NULL;
				int Lump = W_CheckNumForName(VName(&parse[i],
					VName::AddLower8), WADNS_ACSLibrary);
				if (Lump < 0)
				{
					GCon->Logf("Could not find ACS library %s.", &parse[i]);
				}
				else
				{
					Object = StaticLoadObject(Lump);
				}
				Imports.Append(Object);
				do ; while (parse[++i]);
			}
		}

		//	Go through each imported object in order and resolve all
		// imported functions and map variables.
		for (i = 0; i < Imports.Num(); i++)
		{
			FACScriptsObject* lib = Imports[i];
			int j;

			if (!lib)
				continue;

			// Resolve functions
			buffer = (int*)FindChunk("FNAM");
			for (j = 0; j < NumFunctions; j++)
			{
				FACScriptFunction *func = &Functions[j];
				if (func->Address != 0 || func->ImportNum != 0)
					continue;

				int libfunc = lib->FindFunctionName((char*)(buffer + 2) +
					LittleLong(buffer[3 + j]));
				if (libfunc < 0)
					continue;

				FACScriptFunction* realfunc = &lib->Functions[libfunc];
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
//	FACScriptsObject::UnencryptStrings
//
//==========================================================================

void FACScriptsObject::UnencryptStrings()
{
	guard(FACScriptsObject::UnencryptStrings);
	byte *prevchunk = NULL;
	vuint32* chunk = (vuint32*)FindChunk("STRE");
	while (chunk)
	{
		for (int strnum = 0; strnum < LittleLong(chunk[3]); strnum++)
		{
			int ofs = LittleLong(chunk[5 + strnum]);
			byte* data = (byte*)chunk + ofs + 8;
			byte last;
			int p = (byte)(ofs * 157135);
			int i = 0;
			do
			{
				last = (data[i] ^= (byte)(p + (i >> 1)));
				i++;
			} while (last != 0);
		}
		prevchunk = (byte*)chunk;
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
//	FACScriptsObject::FindFunctionName
//
//==========================================================================

int FACScriptsObject::FindFunctionName(const char* Name) const
{
	guard(FACScriptsObject::FindFunctionName);
	return FindStringInChunk(FindChunk("FNAM"), Name);
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::FindMapVarName
//
//==========================================================================

int FACScriptsObject::FindMapVarName(const char* Name) const
{
	guard(FACScriptsObject::FindMapVarName);
	return FindStringInChunk(FindChunk("MEXP"), Name);
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::FindMapArray
//
//==========================================================================

int FACScriptsObject::FindMapArray(const char* Name) const
{
	guard(FACScriptsObject::FindMapArray);
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
//	FACScriptsObject::FindStringInChunk
//
//==========================================================================

int FACScriptsObject::FindStringInChunk(byte* Chunk, const char* Name) const
{
	guard(FACScriptsObject::FindStringInChunk);
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
//	FACScriptsObject::FindChunk
//
//==========================================================================

byte* FACScriptsObject::FindChunk(const char* id) const
{
	guard(FACScriptsObject::FindChunk);
	byte* chunk = Chunks;
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
//	FACScriptsObject::NextChunk
//
//==========================================================================

byte* FACScriptsObject::NextChunk(byte* prev) const
{
	guard(FACScriptsObject::NextChunk);
	int id = *(int*)prev;
	byte* chunk = prev + LittleLong(((int*)prev)[1]) + 8;
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
//	FACScriptsObject::Serialise
//
//==========================================================================

void FACScriptsObject::Serialise(VStream& Strm)
{
	guard(FACScriptsObject::Serialise);
	for (int i = 0; i < NumScripts; i++)
	{
		Strm << Scripts[i].state;
		Strm << STRM_INDEX(Scripts[i].waitValue);
	}
	for (int i = 0; i < MAX_ACS_MAP_VARS; i++)
	{
		Strm << STRM_INDEX(MapVarStore[i]);
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::OffsetToPtr
//
//==========================================================================

vuint8* FACScriptsObject::OffsetToPtr(int Offs)
{
	if (Offs < 0 || Offs >= DataSize)
		Host_Error("Bad offset in ACS file");
	return Data + Offs;
}

//==========================================================================
//
//	FACScriptsObject::PtrToOffset
//
//==========================================================================

int FACScriptsObject::PtrToOffset(vuint8* Ptr)
{
	return Ptr - Data;
}

//==========================================================================
//
//	FACScriptsObject::FindScript
//
//==========================================================================

acsInfo_t* FACScriptsObject::FindScript(int Number) const
{
	guard(FACScriptsObject::FindScript);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].number == Number)
		{
			return Scripts + i;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::GetFunction
//
//==========================================================================

FACScriptFunction* FACScriptsObject::GetFunction(int funcnum,
	FACScriptsObject*& Object)
{
	guard(FACScriptsObject::GetFunction);
	if ((unsigned)funcnum >= (unsigned)NumFunctions)
	{
		return NULL;
	}
	FACScriptFunction* Func = Functions + funcnum;
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
//	FACScriptsObject::GetArrayVal
//
//==========================================================================

int FACScriptsObject::GetArrayVal(int ArrayIdx, int Index)
{
	guard(FACScriptsObject::GetArrayVal);
	if ((unsigned)ArrayIdx >= (unsigned)NumTotalArrays)
		return 0;
	if ((unsigned)Index >= (unsigned)Arrays[ArrayIdx]->Size)
		return 0;
	return Arrays[ArrayIdx]->Data[Index];
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::SetArrayVal
//
//==========================================================================

void FACScriptsObject::SetArrayVal(int ArrayIdx, int Index, int Value)
{
	guard(FACScriptsObject::SetArrayVal);
	if ((unsigned)ArrayIdx >= (unsigned)NumTotalArrays)
		return;
	if ((unsigned)Index >= (unsigned)Arrays[ArrayIdx]->Size)
		return;
	Arrays[ArrayIdx]->Data[Index] = Value;
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StartTypedACScripts
//
//==========================================================================

void FACScriptsObject::StartTypedACScripts(int Type)
{
	guard(FACScriptsObject::StartTypedACScripts);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].type == Type)
		{
			// Auto-activate
			SpawnScript(&Scripts[i], this, NULL, NULL, 0, 0, 0, 0, true);
		}
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticLoadObject
//
//==========================================================================

FACScriptsObject* FACScriptsObject::StaticLoadObject(int Lump)
{
	guard(FACScriptsObject::StaticLoadObject);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		if (LoadedObjects[i]->LumpNum == Lump)
		{
			return LoadedObjects[i];
		}
	}
	return new FACScriptsObject(Lump);
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticUnloadObjects
//
//==========================================================================

void FACScriptsObject::StaticUnloadObjects()
{
	guard(FACScriptsObject::StaticUnloadObjects);
	for (int i = 0; i < LoadedObjects.Num(); i++)
		delete LoadedObjects[i];
	LoadedObjects.Clear();
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticFindScript
//
//==========================================================================

acsInfo_t* FACScriptsObject::StaticFindScript(int Number, FACScriptsObject*& Object)
{
	guard(FACScriptsObject::StaticFindScript);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		acsInfo_t* Found = LoadedObjects[i]->FindScript(Number);
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
//	FACScriptsObject::StaticGetString
//
//==========================================================================

const char* FACScriptsObject::StaticGetString(int Index)
{
	guard(FACScriptsObject::StaticGetString);
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
//	FACScriptsObject::StaticGetObject
//
//==========================================================================

FACScriptsObject* FACScriptsObject::StaticGetObject(int Index)
{
	guard(FACScriptsObject::StaticGetObject);
	if ((unsigned)Index >= (unsigned)LoadedObjects.Num())
	{
		return NULL;
	}
	return LoadedObjects[Index];
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticStartTypedACScripts
//
//==========================================================================

void FACScriptsObject::StaticStartTypedACScripts(int Type)
{
	guard(FACScriptsObject::StaticStartTypedACScripts);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->StartTypedACScripts(Type);
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticSerialise
//
//==========================================================================

void FACScriptsObject::StaticSerialise(VStream& Strm)
{
	guard(FACScriptsObject::StaticSerialise);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->Serialise(Strm);
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::ScriptFinished
//
//==========================================================================

void FACScriptsObject::ScriptFinished(int number)
{
	guard(FACScriptsObject::ScriptFinished);
	for (int i = 0; i < NumScripts; i++)
	{
		if (Scripts[i].state == ASTE_WAITINGFORSCRIPT &&
			Scripts[i].waitValue == number)
		{
			Scripts[i].state = ASTE_RUNNING;
		}
	}
	unguard;
}

//==========================================================================
//
//	FACScriptsObject::StaticScriptFinished
//
//==========================================================================

void FACScriptsObject::StaticScriptFinished(int number)
{
	guard(FACScriptsObject::StaticScriptFinished);
	for (int i = 0; i < LoadedObjects.Num(); i++)
	{
		LoadedObjects[i]->ScriptFinished(number);
	}
	unguard;
}

//==========================================================================
//
//	FACSGrowingArray::Redim
//
//==========================================================================

void FACSGrowingArray::Redim(int NewSize)
{
	guard(FACSGrowingArray::Redim);
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
//	FACSGrowingArray::SetElemVal
//
//==========================================================================

void FACSGrowingArray::SetElemVal(int Index, int Value)
{
	guard(FACSGrowingArray::SetElemVal);
	if (Index >= Size)
	{
		Redim(Index + 1);
	}
	Data[Index] = Value;
	unguard;
}

//==========================================================================
//
//	FACSGrowingArray::GetElemVal
//
//==========================================================================

int FACSGrowingArray::GetElemVal(int Index)
{
	guard(FACSGrowingArray::GetElemVal);
	if ((unsigned)Index >= (unsigned)Size)
		return 0;
	return Data[Index];
	unguard;
}

//==========================================================================
//
//	FACSGrowingArray::Serialise
//
//==========================================================================

void FACSGrowingArray::Serialise(VStream& Strm)
{
	guard(FACSGrowingArray::Serialise);
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
//	VACS::Destroy
//
//==========================================================================

void VACS::Destroy()
{
	guard(VACS::Destroy);
	if (LocalVars)
	{
		delete[] LocalVars;
	}
	unguard;
}

//==========================================================================
//
//	VACS::Serialise
//
//==========================================================================

void VACS::Serialise(VStream& Strm)
{
	guard(VACS::Serialise);
	vint32 TmpInt;

	Super::Serialise(Strm);
	Strm.SerialiseReference(*(VObject**)&Activator, VEntity::StaticClass());
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
		<< DelayTime;
	if (Strm.IsLoading())
	{
		Strm << STRM_INDEX(TmpInt);
		ActiveObject = FACScriptsObject::StaticGetObject(TmpInt);
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

void VACS::Tick(float DeltaTime)
{
	guard(VACS::Tick);
	RunScript(DeltaTime);
	unguard;
}

//==========================================================================
//
//	VACS::RunScript
//
//==========================================================================

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

int VACS::RunScript(float DeltaTime)
{
	guard(VACS::RunScript);
	if (info->state == ASTE_TERMINATING)
	{
		info->state = ASTE_INACTIVE;
		FACScriptsObject::StaticScriptFinished(number);
		SetFlags(_OF_DelayedDestroy);
		return 1;
	}
	if (info->state == ASTE_WAITINGFORTAG &&
		!Level->eventTagBusy(info->waitValue))
	{
		info->state = ASTE_RUNNING;
	}
	if (info->state == ASTE_WAITINGFORPOLY &&
		!Level->eventPolyBusy(info->waitValue))
	{
		info->state = ASTE_RUNNING;
	}
	if (info->state != ASTE_RUNNING)
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

	VStr PrintStr;
	vint32 resultValue = 1;
	vint32 stack[ACS_STACK_DEPTH];
	vint32* optstart = NULL;
	vint32* locals = LocalVars;
	FACScriptFunction* activeFunction = NULL;
	EACSFormat fmt = ActiveObject->GetFormat();
	int action = SCRIPT_CONTINUE;
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
			action = SCRIPT_TERMINATE;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_Suspend)
			info->state = ASTE_SUSPENDED;
			action = SCRIPT_STOP;
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
			action = SCRIPT_STOP;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_DelayDirect)
			DelayTime = float(READ_INT32(ip)) / 35.0;
			ip += 4;
			action = SCRIPT_STOP;
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
			info->waitValue = sp[-1];
			info->state = ASTE_WAITINGFORTAG;
			sp--;
			action = SCRIPT_STOP;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_TagWaitDirect)
			info->waitValue = READ_INT32(ip);
			info->state = ASTE_WAITINGFORTAG;
			ip += 4;
			action = SCRIPT_STOP;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_PolyWait)
			info->waitValue = sp[-1];
			info->state = ASTE_WAITINGFORPOLY;
			sp--;
			action = SCRIPT_STOP;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_PolyWaitDirect)
			info->waitValue = READ_INT32(ip);
			info->state = ASTE_WAITINGFORPOLY;
			ip += 4;
			action = SCRIPT_STOP;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_ChangeFloor)
			{
				int Flat = GTextureManager.NumForName(VName(GetStr(sp[-1]),
					VName::AddLower8), TEXTYPE_Flat, true, true);
				for  (int Idx = FindSectorFromTag(sp[-2], -1); Idx >= 0;
					Idx = FindSectorFromTag(sp[-2], Idx))
				{
					SV_SetFloorPic(Idx, Flat);
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
					SV_SetFloorPic(Idx, Flat);
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
					SV_SetCeilPic(Idx, Flat);
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
					SV_SetCeilPic(Idx, Flat);
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
			info->waitValue = sp[-1];
			info->state = ASTE_WAITINGFORSCRIPT;
			sp--;
			action = SCRIPT_STOP;
			ACSVM_BREAK_STOP;

		ACSVM_CASE(PCD_ScriptWaitDirect)
			info->waitValue = READ_INT32(ip);
			info->state = ASTE_WAITINGFORSCRIPT;
			ip += 4;
			action = SCRIPT_STOP;
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
				SV_ClientCenterPrintf(Activator->Player, *PrintStr);
			}
			else
			{
				SV_BroadcastCentrePrintf(*PrintStr);
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
				if (GGameInfo->Players[i])
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
			*sp = level.tictime;
			sp++;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SectorSound)
			SV_SectorStartSound(line ? line->frontsector : NULL,
				GSoundManager->GetSoundID(GetStr(sp[-2])), 0, sp[-1]);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_AmbientSound)
			SV_StartSound(NULL, GSoundManager->GetSoundID(GetStr(sp[-2])),
				0, sp[-1]);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SoundSequence)
			SV_SectorStartSequence(line ? line->frontsector : NULL,
				GetStr(sp[-1]));
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
					SV_SetLineTexture(line->sidenum[sp[-3]], sp[-2], Tex);
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
					if (sp[-1])
						line->flags |= ML_BLOCKING;
					else
						line->flags &= ~ML_BLOCKING;
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
				int sound = GSoundManager->GetSoundID(GetStr(sp[-2]));
				int searcher = -1;
				for (VEntity* mobj = Level->eventFindMobjFromTID(sp[-3], &searcher);
					mobj != NULL; mobj = Level->eventFindMobjFromTID(sp[-3], &searcher))
				{
					SV_StartSound(mobj, sound, 0, sp[-1]);
				}
				sp -= 3;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_EndPrintBold)
			//FIXME yellow message
			PrintStr = PrintStr.EvalEscapeSequences();
			SV_BroadcastCentrePrintf(*PrintStr);
			ACSVM_BREAK;

		//	Extended P-Code commands.
		ACSVM_CASE(PCD_ActivatorSound)
			SV_StartSound(Activator, GSoundManager->GetSoundID(GetStr(
				sp[-2])), 0, sp[-1]);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_LocalAmbientSound)
			SV_StartLocalSound(Activator, GSoundManager->GetSoundID(GetStr(
				sp[-2])), 0, sp[-1]);
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
					Plr = GGameInfo->Players[sp[-1] - 1];
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
			//FIXME implement this
			//sp[-1] - air control, fixed point
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetAirControlDirect)
			//FIXME implement this
			//READ_INT32(ip) - air control, fixed point
			ip += 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ClearInventory)
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
			GiveInventory(Activator, GetStr(sp[-2]), sp[-1]);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GiveInventoryDirect)
			GiveInventory(Activator, GetStr(READ_INT32(ip)),
				READ_INT32(ip + 4));
			ip += 8;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TakeInventory)
			TakeInventory(Activator, GetStr(sp[-2]), sp[-1]);
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TakeInventoryDirect)
			TakeInventory(Activator, GetStr(READ_INT32(ip)),
				READ_INT32(ip + 4));
			ip += 8;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CheckInventory)
			sp[-1] = CheckInventory(Activator, GetStr(sp[-1]));
			ACSVM_BREAK;

		ACSVM_CASE(PCD_CheckInventoryDirect)
			*sp = CheckInventory(Activator, GetStr(READ_INT32(ip)));
			ip += 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Spawn)
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
			//FIXME print localised string.
			PrintStr += GetStr(sp[-1]);
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
				SV_ClientPrintf(Activator->Player, *PrintStr);
			}
			else
			{
				SV_BroadcastCentrePrintf(*PrintStr);
			}
			sp = optstart - 6;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetFont)
			//FIXME implement this
			//sp[-1] - font name, string
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetFontDirect)
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
			action = SCRIPT_STOP;
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
			//FIXME implement this
			//sp[-5] - r
			//sp[-4] - g
			//sp[-3] - b
			//sp[-2] - a
			//sp[-1] - time, fixed point
			sp -= 5;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_FadeRange)
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
			//FIXME implement this
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayMovie)
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
			//FIXME implement this
			//sp[-1] - index
			sp--;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TranslationRange1)
			//FIXME implement this
			//sp[-4] - start
			//sp[-3] - end
			//sp[-2] - pal1
			//sp[-1] - pal2
			sp -= 4;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_TranslationRange2)
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
			//FIXME implement this
			ACSVM_BREAK;

		ACSVM_CASE(PCD_Call)
		ACSVM_CASE(PCD_CallDiscard)
			{
				int funcnum;
				int i;
				FACScriptsObject* object = ActiveObject;

				funcnum = READ_BYTE_OR_INT32;
				INC_BYTE_OR_INT32;
				FACScriptFunction* func = ActiveObject->GetFunction(funcnum,
					object);
				if (!func)
				{
					GCon->Logf("Function %d in script %d out of range",
						funcnum, number);
					action = SCRIPT_TERMINATE;
					ACSVM_BREAK_STOP;
				}
				if ((sp - stack) + func->LocalCount + 64 > ACS_STACK_DEPTH)
				{
					// 64 is the margin for the function's working space
					GCon->Logf("Out of stack space in script %d", number);
					action = SCRIPT_TERMINATE;
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
				((CallReturn*)sp)->ReturnAddress =
					ActiveObject->PtrToOffset(ip);
				((CallReturn*)sp)->ReturnFunction = activeFunction;
				((CallReturn*)sp)->ReturnObject = ActiveObject;
				((CallReturn*)sp)->bDiscardResult = (cmd == PCD_CallDiscard);
				sp += sizeof(CallReturn) / sizeof(vint32);
				ip = ActiveObject->OffsetToPtr(func->Address);
				ActiveObject = object;
				activeFunction = func;
			}
			ACSVM_BREAK;

		ACSVM_CASE(PCD_ReturnVoid)
		ACSVM_CASE(PCD_ReturnVal)
			{
				int value;
				CallReturn* retState;

				if (cmd == PCD_ReturnVal)
				{
					value = sp[-1];
					sp--;
				}
				else
				{
					value = 0;
				}
				sp -= sizeof(CallReturn) / sizeof(vint32);
				retState = (CallReturn*)sp;
				ip = ActiveObject->OffsetToPtr(retState->ReturnAddress);
				sp -= activeFunction->ArgCount + activeFunction->LocalCount;
				activeFunction = retState->ReturnFunction;
				ActiveObject = retState->ReturnObject;
				fmt = ActiveObject->GetFormat();
				if (!activeFunction)
				{
					locals = LocalVars;
				}
				else
				{
					locals = sp - activeFunction->ArgCount -
						activeFunction->LocalCount - sizeof(CallReturn) /
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
			//FIXME implement this
			//sp[-1] - weapon name, string
			//Pushes result
			sp[-1] = 0;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetWeapon)
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
			//FIXME implement this
			//sp[-2] - TID
			//sp[-1] - weapon name, string
			sp -= 2;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_SetActorProperty)
			//FIXME implement this
			//sp[-3] - TID
			//sp[-2] - property
			//sp[-1] - value
			sp -= 3;
			ACSVM_BREAK;

		ACSVM_CASE(PCD_GetActorProperty)
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
				sp[-1] = level.partime;
				break;
			case LEVELINFO_CLUSTERNUM:
				sp[-1] = level.cluster;
				break;
			case LEVELINFO_LEVELNUM:
				sp[-1] = level.levelnum;
				break;
			case LEVELINFO_TOTAL_SECRETS:
				sp[-1] = level.totalsecret;
				break;
			case LEVELINFO_FOUND_SECRETS:
				sp[-1] = level.currentsecret;
				break;
			case LEVELINFO_TOTAL_ITEMS:
				sp[-1] = level.totalitems;
				break;
			case LEVELINFO_FOUND_ITEMS:
				sp[-1] = level.currentitems;
				break;
			case LEVELINFO_TOTAL_MONSTERS:
				sp[-1] = level.totalkills;
				break;
			case LEVELINFO_KILLED_MONSTERS:
				sp[-1] = level.currentkills;
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
				(GGameInfo->Players[sp[-1]] && (GGameInfo->Players[
				sp[-1]]->PlayerFlags & VBasePlayer::PF_Spawned));
			ACSVM_BREAK;

		ACSVM_CASE(PCD_PlayerIsBot)
			sp[-1] = (sp[-1] < 0 || sp[-1] >= MAXPLAYERS) ? false :
				GGameInfo->Players[sp[-1]] && GGameInfo->Players[
				sp[-1]]->PlayerFlags & VBasePlayer::PF_Spawned &&
				GGameInfo->Players[sp[-1]]->PlayerFlags &
				VBasePlayer::PF_IsBot;
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
			GCon->Logf(NAME_Dev, "Unsupported ACS p-code %d", cmd);
			action = SCRIPT_TERMINATE;
			ACSVM_BREAK_STOP;

		ACSVM_DEFAULT
			Host_Error("Illegal ACS opcode %d", cmd);
		}
	} while  (action == SCRIPT_CONTINUE);
#if USE_COMPUTED_GOTO
LblFuncStop:
#endif
	InstructionPointer = ip;
	if (action == SCRIPT_TERMINATE)
	{
		info->state = ASTE_INACTIVE;
		FACScriptsObject::StaticScriptFinished(number);
		SetFlags(_OF_DelayedDestroy);
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

int VACS::FindSectorFromTag(int tag, int start)
{
	guard(VACS::FindSectorFromTag);
	for (int i = start + 1; i < XLevel->NumSectors; i++)
		if (XLevel->Sectors[i].tag == tag)
			return i;
	return -1;
	unguard;
}

//============================================================================
//
//	VACS::GiveInventory
//
//============================================================================

void VACS::GiveInventory(VEntity* Activator, const char* AType, int Amount)
{
	guard(VACS::GiveInventory);
	if (Amount <= 0)
	{
		return;
	}
	const char* Type = AType;
	if (VStr::Cmp(Type, "Armor") == 0)
	{
		Type = "BasicArmor";
	}
	else if (Activator)
	{
		Activator->eventGiveInventory(Type, Amount);
	}
	else
	{
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (GGameInfo->Players[i] &&
				GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned)
				GGameInfo->Players[i]->MO->eventGiveInventory(Type, Amount);
		}
	}
	return;
	unguard;
}

//============================================================================
//
//	VACS::TakeInventory
//
//============================================================================

void VACS::TakeInventory(VEntity* Activator, const char* AType, int Amount)
{
	guard(VACS::TakeInventory);
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
			if (GGameInfo->Players[i] &&
				GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned)
				GGameInfo->Players[i]->MO->eventTakeInventory(Type, Amount);
		}
	}
	unguard;
}

//============================================================================
//
//	VACS::CheckInventory
//
//============================================================================

int VACS::CheckInventory(VEntity* Activator, const char* AType)
{
	guard(VACS::CheckInventory);
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
// P_LoadACScripts
//
//==========================================================================

void P_LoadACScripts(int Lump)
{
	guard(P_LoadACScripts);
	if (Lump < 0)
	{
		return;
	}

	FACScriptsObject::StaticLoadObject(Lump);
	unguard;
}

//==========================================================================
//
//	P_UnloadACScripts
//
//==========================================================================

void P_UnloadACScripts()
{
	guard(P_UnloadACScripts);
	FACScriptsObject::StaticUnloadObjects();
	unguard;
}

//==========================================================================
//
//	P_StartTypedACScripts
//
//==========================================================================

void P_StartTypedACScripts(int Type)
{
	FACScriptsObject::StaticStartTypedACScripts(Type);
}

//==========================================================================
//
// P_CheckACSStore
//
// Scans the ACS store and executes all scripts belonging to the current
// map.
//
//==========================================================================

void P_CheckACSStore()
{
	guard(P_CheckACSStore);
	acsstore_t *store;

	for (store = ACSStore; store->map[0] != 0; store++)
	{
		if (!VStr::ICmp(store->map, *level.MapName))
		{
			FACScriptsObject* Object;
			acsInfo_t* info = FACScriptsObject::StaticFindScript(store->script, Object);
			if (info)
			{
				if (info->state == ASTE_SUSPENDED)
				{
					//	Resume a suspended script
					info->state = ASTE_RUNNING;
				}
				else if (info->state == ASTE_INACTIVE)
				{
					SpawnScript(info, Object, NULL, NULL, 0, store->args[0],
						store->args[1], store->args[2], true);
				}
			}
			else
			{
				//	Script not found.
				GCon->Logf(NAME_Dev, "P_CheckACSStore: Unknown script %d",
					store->script);
			}
			VStr::Cpy(store->map, "-");
		}
	}
	unguard;
}

//==========================================================================
//
//	P_StartACS
//
//==========================================================================

bool P_StartACS(int number, int map_num, int arg1, int arg2, int arg3,
	VEntity *activator, line_t *line, int side, bool Always, bool WantResult)
{
	guard(P_StartACS);
	char map[12] = "";
	FACScriptsObject* Object;

	if (map_num)
	{
		VStr::Cpy(map, SV_GetMapName(map_num));
	}

	if (map[0] && VStr::ICmp(map, *level.MapName))
	{
		// Add to the script store
		return AddToACSStore(map, number, arg1, arg2, arg3);
	}
	acsInfo_t* info = FACScriptsObject::StaticFindScript(number, Object);
	if (!info)
	{
		//	Script not found
		GCon->Logf(NAME_Dev, "P_StartACS ERROR: Unknown script %d", number);
		return false;
	}
	if (!Always)
	{
		if (info->state == ASTE_SUSPENDED)
		{
			//	Resume a suspended script
			info->state = ASTE_RUNNING;
			return true;
		}
		if (info->state != ASTE_INACTIVE)
		{
			//	Script is already executing
			return false;
		}
	}
	VACS* script = SpawnScript(info, Object, activator, line, side, arg1,
		arg2, arg3, false);
	if (WantResult)
	{
		return script->RunScript(host_frametime);
	}
	return true;
	unguard;
}

//==========================================================================
//
// AddToACSStore
//
//==========================================================================

static bool AddToACSStore(const char *map, int number, int arg1, int arg2,
	int arg3)
{
	int i;
	int index;

	index = -1;
	for (i = 0; ACSStore[i].map[0]; i++)
	{
		if (ACSStore[i].script == number && !VStr::Cmp(ACSStore[i].map, map))
		{
			// Don't allow duplicates
			return false;
		}
		if (index == -1 && ACSStore[i].map[0] == '-')
		{
			// Remember first empty slot
			index = i;
		}
	}
	if (index == -1)
	{
		// Append required
		if (i == MAX_ACS_STORE)
		{
			Sys_Error("AddToACSStore: MAX_ACS_STORE (%d) exceeded.",
				MAX_ACS_STORE);
		}
		index = i;
		ACSStore[index + 1].map[0] = 0;
	}
	VStr::Cpy(ACSStore[index].map, map);
	ACSStore[index].script = number;
	ACSStore[index].args[0] = arg1;
	ACSStore[index].args[1] = arg2;
	ACSStore[index].args[2] = arg3;
	return true;
}

//==========================================================================
//
// P_TerminateACS
//
//==========================================================================

bool P_TerminateACS(int number, int)
{
	guard(P_TerminateACS);
	acsInfo_t* info;
	FACScriptsObject* Object;

	info = FACScriptsObject::StaticFindScript(number, Object);
	if (!info)
	{
		//	Script not found
		return false;
	}
	if (info->state == ASTE_INACTIVE || info->state == ASTE_TERMINATING)
	{
		//	States that disallow termination
		return false;
	}
	info->state = ASTE_TERMINATING;
	return true;
	unguard;
}

//==========================================================================
//
// P_SuspendACS
//
//==========================================================================

bool P_SuspendACS(int number, int)
{
	guard(P_SuspendACS);
	acsInfo_t* info;
	FACScriptsObject* Object;

	info = FACScriptsObject::StaticFindScript(number, Object);
	if (!info)
	{
		//	Script not found.
		return false;
	}
	if (info->state == ASTE_INACTIVE || info->state == ASTE_SUSPENDED ||
		info->state == ASTE_TERMINATING)
	{
		// States that disallow suspension
		return false;
	}
	info->state = ASTE_SUSPENDED;
	return true;
	unguard;
}

//==========================================================================
//
//	P_ACSInitNewGame
//
//==========================================================================

void P_ACSInitNewGame()
{
	guard(P_ACSInitNewGame);
	memset(WorldVars, 0, sizeof(WorldVars));
	memset(GlobalVars, 0, sizeof(GlobalVars));
	memset(ACSStore, 0, sizeof(ACSStore));
	for (int i = 0; i < MAX_ACS_WORLD_VARS; i++)
		WorldArrays[i].Redim(0);
	for (int i = 0; i < MAX_ACS_GLOBAL_VARS; i++)
		GlobalArrays[i].Redim(0);
	unguard;
}

//==========================================================================
//
//	P_SerialiseScripts
//
//==========================================================================

void P_SerialiseScripts(VStream& Strm)
{
	FACScriptsObject::StaticSerialise(Strm);
}

//==========================================================================
//
//	SpawnScript
//
//==========================================================================

static VACS* SpawnScript(acsInfo_t* Info, FACScriptsObject* Object,
	VEntity* Activator, line_t* Line, int Side, int Arg1, int Arg2, int Arg3,
	bool Delayed)
{
	VACS* script = Spawn<VACS>();
	GLevel->AddThinker(script);
	script->info = Info;
	script->number = Info->number;
	script->InstructionPointer = Info->Address;
	script->ActiveObject = Object;
	script->Activator = Activator;
	script->line = Line;
	script->side = Side;
	script->LocalVars = new vint32[Info->VarCount];
	script->LocalVars[0] = Arg1;
	script->LocalVars[1] = Arg2;
	script->LocalVars[2] = Arg3;
	memset(script->LocalVars + Info->argCount, 0,
		(Info->VarCount - Info->argCount) * 4);
	if (Delayed)
	{
		//	World objects are allotted 1 second for initialization.
		script->DelayTime = 1.0;
	}
	Info->state = ASTE_RUNNING;
	return script;
}
