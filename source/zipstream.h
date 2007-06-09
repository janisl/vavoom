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

#ifndef _ZIPSTREAM_H
#define _ZIPSTREAM_H

#include <zlib.h>

class VZipStreamReader : public VStream
{
private:
	enum { BUFFER_SIZE = 16384 };

	VStream*		SrcStream;
	Bytef			Buffer[BUFFER_SIZE];
	z_stream		ZStream;
	bool			Initialised;
	vuint32			UncompressedSize;

public:
	VZipStreamReader(VStream*, vuint32 = 0xffffffff);
	~VZipStreamReader();
	void Serialise(void*, int);
	void Seek(int);
	int Tell();
	int TotalSize();
	bool AtEnd();
	bool Close();
};

#endif
