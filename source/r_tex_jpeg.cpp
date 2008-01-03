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
#include "r_tex.h"

#ifdef CLIENT
extern "C"
{
#include <jpeglib.h>
}
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

#ifdef CLIENT
struct VJpegClientData
{
	VStream*	Strm;
	JOCTET		Buffer[4096];
};
#endif

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VJpegTexture::Create
//
//==========================================================================

VTexture* VJpegTexture::Create(VStream& Strm, int LumpNum)
{
	guard(VJpegTexture::Create);
	if (Strm.TotalSize() < 11)
	{
		//	File is too small.
		return NULL;
	}

	vuint8 Buf[8];

	//	Check header.
	Strm.Seek(0);
	Strm.Serialise(Buf, 2);
	if (Buf[0] != 0xff || Buf[1] != 0xd8)
	{
		return NULL;
	}

	//	Find SOFn marker to get the image dimensions.
	int Len;
	do
	{
		if (Strm.TotalSize() - Strm.Tell() < 4)
		{
			return NULL;
		}

		//	Read marker.
		Strm.Serialise(Buf, 2);
		if (Buf[0] != 0xff)
		{
			//	Missing identifier of a marker.
			return NULL;
		}

		//	Skip padded 0xff-s
		while (Buf[1] == 0xff)
		{
			if (Strm.TotalSize() - Strm.Tell() < 3)
			{
				return NULL;
			}
			Strm.Serialise(Buf + 1, 1);
		}

		//	Read length
		Strm.Serialise(Buf + 2, 2);
		Len = Buf[3] + (Buf[2] << 8);
		if (Len < 2)
		{
			return NULL;
		}
		if (Strm.Tell() + Len - 2 >= Strm.TotalSize())
		{
			return NULL;
		}

		//	If it's not a SOFn marker, then skip it.
		if (Buf[1] != 0xc0 && Buf[1] != 0xc1 && Buf[1] != 0xc2)
		{
			Strm.Seek(Strm.Tell() + Len - 2);
		}
	}
	while (Buf[1] != 0xc0 && Buf[1] != 0xc1 && Buf[1] != 0xc2);

	if (Len < 7)
	{
		return NULL;
	}
	Strm.Serialise(Buf, 5);
	vint32 Width = Buf[4] + (Buf[3] << 8);
	vint32 Height = Buf[2] + (Buf[1] << 8);
	return new VJpegTexture(LumpNum, Width, Height);
	unguard;
}

//==========================================================================
//
//	VJpegTexture::VJpegTexture
//
//==========================================================================

VJpegTexture::VJpegTexture(int ALumpNum, int AWidth, int AHeight)
: LumpNum(ALumpNum)
, Pixels(0)
{
	Name = W_LumpName(LumpNum);
	Width = AWidth;
	Height = AHeight;
}

//==========================================================================
//
//	VJpegTexture::~VJpegTexture
//
//==========================================================================

VJpegTexture::~VJpegTexture()
{
	guard(VJpegTexture::~VJpegTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VJpegTexture::GetPixels
//
//==========================================================================

#ifdef CLIENT
static void my_init_source(j_decompress_ptr cinfo)
{
	cinfo->src->next_input_byte = NULL;
	cinfo->src->bytes_in_buffer = 0;
}

static boolean my_fill_input_buffer(j_decompress_ptr cinfo)
{
	VJpegClientData* cdata = (VJpegClientData*)cinfo->client_data;
	if (cdata->Strm->AtEnd())
	{
		//	Insert a fake EOI marker.
		cdata->Buffer[0] = 0xff;
		cdata->Buffer[1] = JPEG_EOI;
		cinfo->src->next_input_byte = cdata->Buffer;
		cinfo->src->bytes_in_buffer = 2;
		return FALSE;
	}

	int Count = 4096;
	if (Count > cdata->Strm->TotalSize() - cdata->Strm->Tell())
	{
		Count = cdata->Strm->TotalSize() - cdata->Strm->Tell();
	}
	cdata->Strm->Serialise(cdata->Buffer, Count);
	cinfo->src->next_input_byte = cdata->Buffer;
	cinfo->src->bytes_in_buffer = Count;
	return TRUE;
}

static void my_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	if (num_bytes <= 0)
	{
		return;
	}
	if ((long)cinfo->src->bytes_in_buffer > num_bytes)
	{
		cinfo->src->bytes_in_buffer -= num_bytes;
		cinfo->src->next_input_byte += num_bytes;
	}
	else
	{
		VJpegClientData* cdata = (VJpegClientData*)cinfo->client_data;
		int Pos = cdata->Strm->Tell() + num_bytes - cinfo->src->bytes_in_buffer;
		if (Pos > cdata->Strm->TotalSize())
		{
			Pos = cdata->Strm->TotalSize();
		}
		cdata->Strm->Seek(Pos);
		cinfo->src->bytes_in_buffer = 0;
	}
}

static void my_term_source(j_decompress_ptr)
{
}

static void my_error_exit(j_common_ptr cinfo)
{
	(*cinfo->err->output_message)(cinfo);
	throw -1;
}

static void my_output_message(j_common_ptr cinfo)
{
	char Msg[JMSG_LENGTH_MAX];
	cinfo->err->format_message(cinfo, Msg);
	GCon->Log(Msg);
}
#endif

vuint8* VJpegTexture::GetPixels()
{
	guard(VJpegTexture::GetPixels);
#ifdef CLIENT
	//	If we already have loaded pixels, return them.
	if (Pixels)
	{
		return Pixels;
	}

	Format = TEXFMT_RGBA;
	Pixels = new vuint8[Width * Height * 4];
	memset(Pixels, 0, Width * Height * 4);

	jpeg_decompress_struct	cinfo;
	jpeg_source_mgr			smgr;
	jpeg_error_mgr			jerr;
	VJpegClientData			cdata;

	//	Open stream.
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);
	check(Strm);

	try
	{
		//	Set up the JPEG error routines.
		cinfo.err = jpeg_std_error(&jerr);
		jerr.error_exit = my_error_exit;
		jerr.output_message = my_output_message;

		//	Set client data pointer
		cinfo.client_data = &cdata;
		cdata.Strm = Strm;

		//	Initialise the JPEG decompression object.
		jpeg_create_decompress(&cinfo);

		//	Specify data source
		smgr.init_source = my_init_source;
		smgr.fill_input_buffer = my_fill_input_buffer;
		smgr.skip_input_data = my_skip_input_data;
		smgr.resync_to_restart = jpeg_resync_to_restart;
		smgr.term_source = my_term_source;
		cinfo.src = &smgr;

		//	Read file parameters with jpeg_read_header()
		jpeg_read_header(&cinfo, TRUE);

		if (!((cinfo.out_color_space == JCS_RGB && cinfo.num_components == 3) ||
			(cinfo.out_color_space == JCS_CMYK && cinfo.num_components == 4) ||
			(cinfo.out_color_space == JCS_GRAYSCALE && cinfo.num_components == 1)))
		{
			GCon->Log("Unsupported JPEG file format");
			throw -1;
		}

		//	Start decompressor.
		jpeg_start_decompress(&cinfo);

		//	JSAMPLEs per row in output buffer
		int row_stride = cinfo.output_width * cinfo.output_components;
		//	Make a one-row-high sample array that will go away when done with image.
		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo,
			JPOOL_IMAGE, row_stride, 1);

		//	Read image
		vuint8* pDst = Pixels;
		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);
			JOCTET* pSrc = buffer[0];
			switch (cinfo.out_color_space)
			{
			case JCS_RGB:
				for (int x = 0; x < Width; x++)
				{
					pDst[0] = pSrc[0];
					pDst[1] = pSrc[1];
					pDst[2] = pSrc[2];
					pDst[3] = 0xff;
					pSrc += 3;
					pDst += 4;
				}
				break;

			case JCS_GRAYSCALE:
				for (int x = 0; x < Width; x++)
				{
					pDst[0] = pSrc[0];
					pDst[1] = pSrc[0];
					pDst[2] = pSrc[0];
					pDst[3] = 0xff;
					pSrc++;
					pDst += 4;
				}
				break;

			case JCS_CMYK:
				for (int x = 0; x < Width; x++)
				{
					pDst[0] = (255 - pSrc[0]) * (255 - pSrc[3]) / 255;
					pDst[1] = (255 - pSrc[1]) * (255 - pSrc[3]) / 255;
					pDst[2] = (255 - pSrc[2]) * (255 - pSrc[3]) / 255;
					pDst[3] = 0xff;
					pSrc += 4;
					pDst += 4;
				}
				break;

			default:
				break;
			}
		}

		//	Finish decompression.
		jpeg_finish_decompress(&cinfo);
	}
	catch (int)
	{
	}

	//	Release JPEG decompression object
	jpeg_destroy_decompress(&cinfo);

	//	Free memory.
	delete Strm;
	return Pixels;
#else
	Sys_Error("ReadPixels on dedicated server");
	return NULL;
#endif
	unguard;
}

//==========================================================================
//
//	VJpegTexture::Unload
//
//==========================================================================

void VJpegTexture::Unload()
{
	guard(VJpegTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}
