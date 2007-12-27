//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: files.cpp 1649 2006-08-10 18:29:09Z dj_jl $
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
//**	Based on sources from zlib with following notice:
//**
//**	Copyright (C) 1998-2004 Gilles Vollant
//**
//**	This software is provided 'as-is', without any express or implied
//**  warranty.  In no event will the authors be held liable for any damages
//**  arising from the use of this software.
//**
//**	Permission is granted to anyone to use this software for any purpose,
//**  including commercial applications, and to alter it and redistribute it
//**  freely, subject to the following restrictions:
//**
//**	1. The origin of this software must not be misrepresented; you must
//**  not claim that you wrote the original software. If you use this
//**  software in a product, an acknowledgment in the product documentation
//**  would be appreciated but is not required.
//**	2. Altered source versions must be plainly marked as such, and must
//**  not be misrepresented as being the original software.
//**	3. This notice may not be removed or altered from any source
//**  distribution.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "fs_local.h"
#include <zlib.h>

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	SIZECENTRALDIRITEM = 0x2e,
	SIZEZIPLOCALHEADER = 0x1e
};

//	VZipFileInfo contain information about a file in the zipfile
struct VZipFileInfo
{
	VStr		Name;					//	Name of the file
	vuint16		flag;					//	General purpose bit flag
	vuint16		compression_method;		//	Compression method
	vuint32		crc;					//	Crc-32
	vuint32		compressed_size;		//	Compressed size
	vuint32		uncompressed_size;		//	Uncompressed size
	vuint16		size_filename;			//	Filename length
	vuint32		offset_curfile;			//	Relative offset of local header

	//	For WAD-like access.
	VName		LumpName;
	vint32		LumpNamespace;
};

class VZipFileReader : public VStream
{
private:
	enum { UNZ_BUFSIZE = 16384 };

	VStream*			FileStream;		//	Source stream of the zipfile
	const VZipFileInfo&	Info;			//	Info about the file we are reading
	FOutputDevice*		Error;

	Bytef		ReadBuffer[UNZ_BUFSIZE];//	Internal buffer for compressed data
	z_stream	stream;					//	ZLib stream structure for inflate

	vuint32		pos_in_zipfile;			//	Position in byte on the zipfile
	vuint32		start_pos;				//	Initial position, for restart
	bool		stream_initialised;		//	Flag set if stream structure is initialised

	vuint32		Crc32;					//	Crc32 of all data uncompressed
	vuint32		rest_read_compressed;	//	Number of byte to be decompressed
	vuint32		rest_read_uncompressed;	//	Number of byte to be obtained after decomp

	bool CheckCurrentFileCoherencyHeader(vuint32*, vuint32);

public:
	VZipFileReader(VStream*, vuint32, const VZipFileInfo&, FOutputDevice*);
	~VZipFileReader();
	void Serialise(void*, int);
	void Seek(int);
	int Tell();
	int TotalSize();
	bool AtEnd();
	bool Close();
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
//	VZipFile::VZipFile
//
//==========================================================================

VZipFile::VZipFile(const VStr& zipfile)
: ZipFileName(zipfile)
, Files(NULL)
, NumFiles(0)
{
	guard(VZipFile::VZipFile);
	GCon->Logf(NAME_Init, "Adding %s", *ZipFileName);

	FileStream = FL_OpenSysFileRead(ZipFileName);
	check(FileStream);

	vuint32 central_pos = SearchCentralDir();
	check(central_pos);

	FileStream->Seek(central_pos);

	vuint32 Signature;
	vuint16 number_disk;		//	Number of the current dist, used for
								// spaning ZIP, unsupported, always 0
	vuint16 number_disk_with_CD;//	Number the the disk with central dir, used
								// for spaning ZIP, unsupported, always 0
	vuint16 number_entry_CD;	//	Total number of entries in
								// the central dir
								// (same than number_entry on nospan)
	vuint16 size_comment;		//	Size of the global comment of the zipfile

	*FileStream
		//	The signature, already checked
		<< Signature
		//	Number of this disk
		<< number_disk
		//	Number of the disk with the start of the central directory
		<< number_disk_with_CD
		//	Total number of entries in the central dir on this disk
		<< NumFiles
		//	Total number of entries in the central dir
		<< number_entry_CD;

	check(number_entry_CD == NumFiles);
	check(number_disk_with_CD == 0);
	check(number_disk == 0);

	vuint32		size_central_dir;	//	Size of the central directory
	vuint32		offset_central_dir;	//	Offset of start of central directory with
									// respect to the starting disk number

	*FileStream
		<< size_central_dir
		<< offset_central_dir
		<< size_comment;

	check(central_pos >= offset_central_dir + size_central_dir);

	BytesBeforeZipFile = central_pos - (offset_central_dir + size_central_dir);

	Files = new VZipFileInfo[NumFiles];

	//	Set the current file of the zipfile to the first file.
	vuint32 pos_in_central_dir = offset_central_dir;
	for (int i = 0; i < NumFiles; i++)
	{
		VZipFileInfo& file_info = Files[i];

		FileStream->Seek(pos_in_central_dir + BytesBeforeZipFile);

		vuint32 Magic;
		vuint16 version;		//	Version made by
		vuint16 version_needed;	//	Version needed to extract
		vuint32 dosDate;		//	Last mod file date in Dos fmt
		vuint16 size_file_extra;//	Extra field length
		vuint16 size_file_comment;	//	File comment length
		vuint16 disk_num_start;	//	Disk number start
		vuint16 internal_fa;	//	Internal file attributes
		vuint32 external_fa;	//	External file attributes

		/* we check the magic */
		*FileStream
			<< Magic
			<< version
			<< version_needed
			<< file_info.flag
			<< file_info.compression_method
			<< dosDate
			<< file_info.crc
			<< file_info.compressed_size
			<< file_info.uncompressed_size
			<< file_info.size_filename
			<< size_file_extra
			<< size_file_comment
			<< disk_num_start
			<< internal_fa
			<< external_fa
			<< file_info.offset_curfile;

		check(Magic == 0x02014b50);

		char* filename_inzip = new char[file_info.size_filename + 1];
		filename_inzip[file_info.size_filename] = '\0';
		FileStream->Serialise(filename_inzip, file_info.size_filename);
		Files[i].Name = VStr(filename_inzip).ToLower().FixFileSlashes();
		delete[] filename_inzip;

		//	Set up lump name for WAD-like access.
		VStr LumpName = Files[i].Name.ExtractFileName().StripExtension();

		//	Map some directories to WAD namespaces.
		Files[i].LumpNamespace =
			Files[i].Name.StartsWith("sprites/") ? WADNS_Sprites :
			Files[i].Name.StartsWith("flats/") ? WADNS_Flats :
			Files[i].Name.StartsWith("colormaps/") ? WADNS_ColourMaps :
			Files[i].Name.StartsWith("acs/") ? WADNS_ACSLibrary :
			Files[i].Name.StartsWith("textures/") ? WADNS_NewTextures :
			Files[i].Name.StartsWith("voices/") ? WADNS_Voices :
			Files[i].Name.StartsWith("hires/") ? WADNS_HiResTextures :
			Files[i].Name.StartsWith("progs/") ? WADNS_Progs :
			Files[i].Name.StartsWith("patches/") ? WADNS_Patches :
			Files[i].Name.StartsWith("graphics/") ? WADNS_Graphics :
			Files[i].Name.StartsWith("sounds/") ? WADNS_Sounds :
			Files[i].Name.StartsWith("music/") ? WADNS_Music :
			Files[i].Name.IndexOf('/') == -1 ? WADNS_Global : -1;

		//	Anything from other directories won't be accessed as lump.
		if (Files[i].LumpNamespace == -1)
			LumpName = VStr();

		//	For sprites \ is a valid frame character but is not allowed to
		// be in a file name, so we do a little mapping here.
		if (Files[i].LumpNamespace == WADNS_Sprites)
		{
			for (size_t ni = 0; ni < LumpName.Length(); ni++)
			{
				if (LumpName[ni] == '^')
				{
					LumpName[ni] = '\\';
				}
			}
		}

		//	Final lump name;
		Files[i].LumpName = VName(*LumpName, VName::AddLower8);

		//	Set the current file of the zipfile to the next file.
		pos_in_central_dir += SIZECENTRALDIRITEM + file_info.size_filename +
			size_file_extra + size_file_comment;
	}

	//	Sort files alphabetically.
	qsort(Files, NumFiles, sizeof(VZipFileInfo), FileCmpFunc);
	unguard;
}

//==========================================================================
//
//	VZipFile::~VZipFile
//
//==========================================================================

VZipFile::~VZipFile()
{
	guard(VZipFile::~VZipFile);
	Close();
	unguard;
}

//==========================================================================
//
//	VZipFile::SearchCentralDir
//
//	Locate the Central directory of a zipfile (at the end, just before
// the global comment)
//
//==========================================================================

vuint32 VZipFile::SearchCentralDir()
{
	enum { BUFREADCOMMENT = 0x400 };

	vuint8 buf[BUFREADCOMMENT + 4];
	vuint32 uMaxBack = 0xffff; /* maximum size of global comment */
	vuint32 uPosFound = 0;

	vuint32 uSizeFile = FileStream->TotalSize();

	if (uMaxBack > uSizeFile)
		uMaxBack = uSizeFile;

	vuint32 uBackRead = 4;
	while (uBackRead < uMaxBack)
	{
		if (uBackRead + BUFREADCOMMENT > uMaxBack)
			uBackRead = uMaxBack;
		else
			uBackRead += BUFREADCOMMENT;
		vuint32 uReadPos = uSizeFile - uBackRead;

		vuint32 uReadSize = ((BUFREADCOMMENT + 4) < (uSizeFile - uReadPos)) ?
			(BUFREADCOMMENT + 4) : (uSizeFile - uReadPos);
		FileStream->Seek(uReadPos);
		FileStream->Serialise(buf, uReadSize);

		for (int i = (int)uReadSize - 3; i-- > 0;)
			if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) &&
				((*(buf + i + 2)) == 0x05) && ((*(buf + i + 3)) == 0x06))
			{
				uPosFound = uReadPos + i;
				break;
			}

		if (uPosFound != 0)
			break;
	}
	return uPosFound;
}

//==========================================================================
//
//	VZipFile::FileExists
//
//==========================================================================

int VZipFile::FileCmpFunc(const void* v1, const void* v2)
{
	return ((VZipFileInfo*)v1)->Name.ICmp(((VZipFileInfo*)v2)->Name);
}

//==========================================================================
//
//	VZipFile::FileExists
//
//==========================================================================

bool VZipFile::FileExists(const VStr& FName)
{
	guard(VZipFile::FileExists);
	for (int i = 0; i < NumFiles; i++)
	{
		if (Files[i].Name == FName)
		{
			return true;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VZipFile::OpenFileRead
//
//==========================================================================

VStream* VZipFile::OpenFileRead(const VStr& FName)
{
	guard(VZipFile::OpenFileRead);
	for (int i = 0; i < NumFiles; i++)
	{
		if (Files[i].Name == FName)
		{
			return new VZipFileReader(FileStream, BytesBeforeZipFile,
				Files[i], GCon);
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VZipFile::Close
//
//==========================================================================

void VZipFile::Close()
{
	guard(VZipFile::Close);
	if (Files)
	{
		delete[] Files;
		Files = NULL;
	}
	if (FileStream)
	{
		delete FileStream;
		FileStream = 0;
	}
	unguard;
}

//==========================================================================
//
//	VZipFile::CheckNumForName
//
//==========================================================================

int VZipFile::CheckNumForName(VName LumpName, EWadNamespace NS)
{
	guard(VZipFile::CheckNumForName);
	for (int i = NumFiles - 1; i >= 0; i--)
	{
		if (Files[i].LumpNamespace == NS && Files[i].LumpName == LumpName)
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
//	VZipFile::CheckNumForFileName
//
//==========================================================================

int VZipFile::CheckNumForFileName(VStr Name)
{
	guard(VZipFile::CheckNumForFileName);
	VStr CheckName = Name.ToLower();
	for (int i = NumFiles - 1; i >= 0; i--)
	{
		if (Files[i].Name == CheckName)
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
//	VZipFile::ReadFromLump
//
//==========================================================================

void VZipFile::ReadFromLump(int Lump, void* Dest, int Pos, int Size)
{
	guard(VZipFile::ReadFromLump);
	check(Lump >= 0);
	check(Lump < NumFiles);
	VStream* Strm = CreateLumpReaderNum(Lump);
	Strm->Seek(Pos);
	Strm->Serialise(Dest, Size);
	delete Strm;
	unguard;
}

//==========================================================================
//
//	VZipFile::LumpLength
//
//==========================================================================

int VZipFile::LumpLength(int Lump)
{
	if (Lump >= NumFiles)
	{
		return 0;
	}
	return Files[Lump].uncompressed_size;
}

//==========================================================================
//
//	VZipFile::LumpName
//
//==========================================================================

VName VZipFile::LumpName(int Lump)
{
	if (Lump >= NumFiles)
	{
		return NAME_None;
	}
	return Files[Lump].LumpName;
}

//==========================================================================
//
//	VZipFile::IterateNS
//
//==========================================================================

int VZipFile::IterateNS(int Start, EWadNamespace NS)
{
	guard(VZipFile::IterateNS);
	for (int li = Start; li < NumFiles; li++)
	{
		if (Files[li].LumpNamespace == NS)
		{
			return li;
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VZipFile::CreateLumpReaderNum
//
//==========================================================================

VStream* VZipFile::CreateLumpReaderNum(int Lump)
{
	guard(VZipFile::CreateLumpReaderNum);
	check(Lump >= 0);
	check(Lump < NumFiles);
	return new VZipFileReader(FileStream, BytesBeforeZipFile, Files[Lump],
		GCon);
	unguard;
}

void VZipFile::BuildGLNodes(VSearchPath*)
{
}
void VZipFile::BuildPVS(VSearchPath*)
{
}

//==========================================================================
//
//	VZipFile::ListWadFiles
//
//==========================================================================

void VZipFile::ListWadFiles(TArray<VStr>& List)
{
	guard(VZipFile::ListWadFiles);
	for (int i = 0; i < NumFiles; i++)
	{
		//	Only .wad files.
		if (!Files[i].Name.EndsWith(".wad"))
			continue;
		//	Don't add WAD files in subdirectories
		if (Files[i].Name.IndexOf('/') != -1)
			continue;
		List.Append(Files[i].Name);
	}
	unguard;
}

//==========================================================================
//
//	VZipFileReader::VZipFileReader
//
//==========================================================================

VZipFileReader::VZipFileReader(VStream* InStream, vuint32 BytesBeforeZipFile,
	const VZipFileInfo& aInfo, FOutputDevice *InError) 
: FileStream(InStream)
, Info(aInfo)
, Error(InError)
{
	guard(VZipFileReader::VZipFileReader);
	//	Open the file in the zip
	vuint32 iSizeVar;
	if (!CheckCurrentFileCoherencyHeader(&iSizeVar, BytesBeforeZipFile))
	{
		bError = true;
		return;
	}

	stream_initialised = false;

	if (Info.compression_method != 0 && Info.compression_method != Z_DEFLATED)
	{
		bError = true;
		Error->Logf("Compression method %d is not supported",
			Info.compression_method);
		return;
	}

	Crc32 = 0;

	stream.total_out = 0;

	if (Info.compression_method == Z_DEFLATED)
	{
		stream.zalloc = (alloc_func)0;
		stream.zfree = (free_func)0;
		stream.opaque = (voidpf)0;
		stream.next_in = (Bytef*)0;
		stream.avail_in = 0;

		int err = inflateInit2(&stream, -MAX_WBITS);
		if (err != Z_OK)
		{
			/* windowBits is passed < 0 to tell that there is no zlib header.
			* Note that in this case inflate *requires* an extra "dummy" byte
			* after the compressed stream in order to complete decompression and
			* return Z_STREAM_END.
			* In unzip, i don't wait absolutely Z_STREAM_END because I known the
			* size of both compressed and uncompressed data
			*/
			bError = true;
			Error->Log("Failed to initialise inflate stream");
			return;
		}
		stream_initialised = true;
	}
	rest_read_compressed = Info.compressed_size;
	rest_read_uncompressed = Info.uncompressed_size;

	pos_in_zipfile = Info.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar +
		BytesBeforeZipFile;
	start_pos = pos_in_zipfile;

	stream.avail_in = 0;
	bLoading = true;
	unguard;
}

//==========================================================================
//
//	VZipFileReader::~VZipFileReader
//
//==========================================================================

VZipFileReader::~VZipFileReader()
{
	guard(VZipFileReader::~VZipFileReader);
	Close();
	unguard;
}

//==========================================================================
//
//	VZipFileReader::CheckCurrentFileCoherencyHeader
//
//	Read the local header of the current zipfile
//	Check the coherency of the local header and info in the end of central
// directory about this file
//	Store in *piSizeVar the size of extra info in local header
// (filename and size of extra field data)
//
//==========================================================================

bool VZipFileReader::CheckCurrentFileCoherencyHeader(vuint32* piSizeVar,
	vuint32 byte_before_the_zipfile)
{
	guard(VZipFileReader::CheckCurrentFileCoherencyHeader);
	vuint32 Magic, DateTime, Crc, ComprSize, UncomprSize;
	vuint16 Version, Flags, ComprMethod, FileNameSize, ExtraFieldSize;

	*piSizeVar = 0;

	FileStream->Seek(Info.offset_curfile + byte_before_the_zipfile);

	*FileStream
		<< Magic
		<< Version
		<< Flags
		<< ComprMethod
		<< DateTime
		<< Crc
		<< ComprSize
		<< UncomprSize
		<< FileNameSize
		<< ExtraFieldSize;

	if (Magic != 0x04034b50)
	{
		Error->Log("Bad file magic");
		return false;
	}

	if (ComprMethod != Info.compression_method)
	{
		Error->Log("Compression method doesn\'t match");
		return false;
	}

	if ((Crc != Info.crc) && ((Flags & 8) == 0))
	{
		Error->Log("CRC doesn\'t match");
		return false;
	}

	if ((ComprSize != Info.compressed_size) && ((Flags & 8) == 0))
	{
		Error->Log("Compressed size doesn\'t match");
		return false;
	}

	if ((UncomprSize != Info.uncompressed_size) && ((Flags & 8) == 0))
	{
		Error->Log("Uncompressed size doesn\'t match");
		return false;
	}

	if ((FileNameSize != Info.size_filename))
	{
		Error->Log("File name length doesn\'t match");
		return false;
	}

	*piSizeVar += FileNameSize + ExtraFieldSize;

	return true;
	unguard;
}

//==========================================================================
//
//	VZipFileReader::Serialise
//
//==========================================================================

void VZipFileReader::Serialise(void* V, int Length)
{
	guard(VZipFileReader::Serialise);
	if (bError)
	{
		//	Don't read anything from already broken stream.
		return;
	}
	if (FileStream->IsError())
	{
		return;
	}

	if (Length == 0)
		return;

	stream.next_out = (Bytef*)V;
	stream.avail_out = Length;

	if ((vuint32)Length > rest_read_uncompressed)
		stream.avail_out = rest_read_uncompressed;

	int iRead = 0;
	while (stream.avail_out > 0)
	{
		if (stream.avail_in == 0 && rest_read_compressed > 0)
		{
			vuint32 uReadThis = UNZ_BUFSIZE;
			if (rest_read_compressed < uReadThis)
				uReadThis = rest_read_compressed;
			FileStream->Seek(pos_in_zipfile);
			FileStream->Serialise(ReadBuffer, uReadThis);
			if (FileStream->IsError())
			{
				Error->Log("Failed to read from zip file");
				return;
			}

			pos_in_zipfile += uReadThis;

			rest_read_compressed -= uReadThis;

			stream.next_in = ReadBuffer;
			stream.avail_in = uReadThis;
		}

		if (Info.compression_method == 0)
		{
			if (stream.avail_in == 0 && rest_read_compressed == 0)
				break;

			int uDoCopy;
			if (stream.avail_out < stream.avail_in)
				uDoCopy = stream.avail_out;
			else
				uDoCopy = stream.avail_in;

			for (int i = 0; i < uDoCopy; i++)
				*(stream.next_out + i) = *(stream.next_in + i);

			Crc32 = crc32(Crc32, stream.next_out, uDoCopy);
			rest_read_uncompressed -= uDoCopy;
			stream.avail_in -= uDoCopy;
			stream.avail_out -= uDoCopy;
			stream.next_out += uDoCopy;
			stream.next_in += uDoCopy;
			stream.total_out += uDoCopy;
			iRead += uDoCopy;
		}
		else
		{
			int flush = Z_SYNC_FLUSH;

			uLong uTotalOutBefore = stream.total_out;
			const Bytef* bufBefore = stream.next_out;

			int err = inflate(&stream, flush);
			if (err >= 0 && stream.msg != NULL)
			{
				bError = true;
				Error->Logf("Decompression failed: %s", stream.msg);
				return;
			}

			uLong uTotalOutAfter = stream.total_out;
			vuint32 uOutThis = uTotalOutAfter - uTotalOutBefore;

			Crc32 = crc32(Crc32, bufBefore, (uInt)uOutThis);

			rest_read_uncompressed -= uOutThis;

			iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

			if (err != Z_OK)
				break;
		}
	}

	if (iRead != Length)
	{
		bError = true;
		Error->Logf("Only read %d of %d bytes", iRead, Length);
	}
	unguard;
}

//==========================================================================
//
//	VZipFileReader::Seek
//
//==========================================================================

void VZipFileReader::Seek(int InPos)
{
	guard(VZipFileReader::Seek);
	check(InPos >= 0);
	check(InPos <= (int)Info.uncompressed_size);

	if (bError)
		return;

	//	If seeking backwards, reset input stream to the begining of the file.
	if (InPos < Tell())
	{
		if (Info.compression_method == Z_DEFLATED)
		{
			check(stream_initialised);
			inflateEnd(&stream);
			memset(&stream, 0, sizeof(stream));
			verify(inflateInit2(&stream, -MAX_WBITS) == Z_OK);
		}
		else
		{
			memset(&stream, 0, sizeof(stream));
		}
		Crc32 = 0;
		rest_read_compressed = Info.compressed_size;
		rest_read_uncompressed = Info.uncompressed_size;
		pos_in_zipfile = start_pos;
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
//	VZipFileReader::Tell
//
//==========================================================================

int VZipFileReader::Tell()
{
	return stream.total_out;
}

//==========================================================================
//
//	VZipFileReader::TotalSize
//
//==========================================================================

int VZipFileReader::TotalSize()
{
	return Info.uncompressed_size;
}

//==========================================================================
//
//	VZipFileReader::AtEnd
//
//==========================================================================

bool VZipFileReader::AtEnd()
{
	return rest_read_uncompressed == 0;
}

//==========================================================================
//
//	VZipFileReader::Close
//
//==========================================================================

bool VZipFileReader::Close()
{
	guard(VZipFileReader::Close);
	if (rest_read_uncompressed == 0)
	{
		if (Crc32 != Info.crc)
		{
			bError = true;
			Error->Log("Bad CRC");
		}
	}

	if (stream_initialised)
	{
		inflateEnd(&stream);
	}
	stream_initialised = false;
	return !bError;
	unguard;
}
