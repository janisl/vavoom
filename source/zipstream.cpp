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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "zipstream.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VZipStreamReader::VZipStreamReader
//
//==========================================================================

VZipStreamReader::VZipStreamReader(VStream* ASrcStream,
	vuint32 AUncompressedSize)
: SrcStream(ASrcStream)
, Initialised(false)
, UncompressedSize(AUncompressedSize)
{
	guard(VZipStreamReader::VZipStreamReader);
	//	Initialise zip stream structure.
	ZStream.total_out = 0;
	ZStream.zalloc = (alloc_func)0;
	ZStream.zfree = (free_func)0;
	ZStream.opaque = (voidpf)0;

	//	Read in some initial data.
	vint32 BytesToRead = BUFFER_SIZE;
	if (BytesToRead > SrcStream->TotalSize())
		BytesToRead = SrcStream->TotalSize();
	SrcStream->Seek(0);
	SrcStream->Serialise(Buffer, BytesToRead);
	if (SrcStream->IsError())
	{
		bError = true;
		return;
	}
	ZStream.next_in = Buffer;
	ZStream.avail_in = BytesToRead;

	//	Open zip stream.
	int err = inflateInit(&ZStream);
	if (err != Z_OK)
	{
		bError = true;
		GCon->Log("Failed to initialise inflate ZStream");
		return;
	}

	Initialised = true;
	bLoading = true;
	unguard;
}

//==========================================================================
//
//	VZipStreamReader::~VZipStreamReader
//
//==========================================================================

VZipStreamReader::~VZipStreamReader()
{
	guard(VZipStreamReader::~VZipStreamReader);
	Close();
	unguard;
}

//==========================================================================
//
//	VZipStreamReader::Serialise
//
//==========================================================================

void VZipStreamReader::Serialise(void* V, int Length)
{
	guard(VZipStreamReader::Serialise);
	if (bError)
	{
		//	Don't read anything from already broken stream.
		return;
	}
	if (SrcStream->IsError())
	{
		return;
	}

	if (Length == 0)
	{
		return;
	}

	ZStream.next_out = (Bytef*)V;
	ZStream.avail_out = Length;

	int BytesRead = 0;
	while (ZStream.avail_out > 0)
	{
		if (ZStream.avail_in == 0)
		{
			if (SrcStream->AtEnd())
			{
				break;
			}
			vint32 BytesToRead = BUFFER_SIZE;
			if (BytesToRead > SrcStream->TotalSize() - SrcStream->Tell())
				BytesToRead = SrcStream->TotalSize() - SrcStream->Tell();
			SrcStream->Serialise(Buffer, BytesToRead);
			if (SrcStream->IsError())
			{
				bError = true;
				return;
			}
			ZStream.next_in = Buffer;
			ZStream.avail_in = BytesToRead;
		}

		vuint32 TotalOutBefore = ZStream.total_out;
		int err = inflate(&ZStream, Z_SYNC_FLUSH);
		if (err >= 0 && ZStream.msg != NULL)
		{
			bError = true;
			GCon->Logf("Decompression failed: %s", ZStream.msg);
			return;
		}
		vuint32 TotalOutAfter = ZStream.total_out;
		BytesRead += TotalOutAfter - TotalOutBefore;

		if (err != Z_OK)
			break;
	}

	if (BytesRead != Length)
	{
		bError = true;
		GCon->Logf("Only read %d of %d bytes", BytesRead, Length);
	}
	unguard;
}

//==========================================================================
//
//	VZipStreamReader::Seek
//
//==========================================================================

void VZipStreamReader::Seek(int InPos)
{
	guard(VZipStreamReader::Seek);
	check(InPos >= 0);
	check(InPos <= (int)UncompressedSize);

	if (UncompressedSize == 0xffffffff)
	{
		Sys_Error("Seek on zip ZStream with unknown total size");
	}

	if (bError)
	{
		return;
	}

	//	If seeking backwards, reset input ZStream to the begining of the file.
	if (InPos < Tell())
	{
		check(Initialised);
		inflateEnd(&ZStream);
		memset(&ZStream, 0, sizeof(ZStream));
		verify(inflateInit2(&ZStream, -MAX_WBITS) == Z_OK);
		SrcStream->Seek(0);
	}

	//	Read data into a temporary buffer untill we reach needed position.
	int ToSkip = InPos - Tell();
	while (ToSkip > 0)
	{
		int Count = ToSkip > 1024 ? 1024 : ToSkip;
		ToSkip -= Count;
		vuint8 TmpBuf[1024];
		Serialise(TmpBuf, Count);
	}
	unguard;
}

//==========================================================================
//
//	VZipStreamReader::Tell
//
//==========================================================================

int VZipStreamReader::Tell()
{
	return ZStream.total_out;
}

//==========================================================================
//
//	VZipStreamReader::TotalSize
//
//==========================================================================

int VZipStreamReader::TotalSize()
{
	if (UncompressedSize == 0xffffffff)
	{
		Sys_Error("TotalSize on zip ZStream with unknown total size");
	}
	return UncompressedSize;
}

//==========================================================================
//
//	VZipStreamReader::AtEnd
//
//==========================================================================

bool VZipStreamReader::AtEnd()
{
	return ZStream.avail_in == 0 && SrcStream->AtEnd();
}

//==========================================================================
//
//	VZipStreamReader::Close
//
//==========================================================================

bool VZipStreamReader::Close()
{
	guard(VZipStreamReader::Close);
	if (Initialised)
	{
		inflateEnd(&ZStream);
	}
	Initialised = false;
	return !bError;
	unguard;
}
