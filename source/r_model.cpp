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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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
// models.c -- model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define	MAX_MOD_KNOWN	256

// TYPES -------------------------------------------------------------------

#pragma pack(1)

struct pcx_t
{
	char			manufacturer;
	char			version;
	char			encoding;
	char			bits_per_pixel;

	unsigned short	xmin;
	unsigned short	ymin;
	unsigned short	xmax;
	unsigned short	ymax;

	unsigned short	hres;
	unsigned short	vres;

	unsigned char	palette[48];

	char			reserved;
	char			color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;

	char			filler[58];
};

struct tgaHeader_t
{
	byte id_length;
	byte pal_type;
	byte img_type;
	word first_color;
	word pal_colors;
	byte pal_entry_size;
	word left;
	word top;
	word width;
	word height;
	byte bpp;
	byte descriptor_bits;
};

#pragma pack()

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static model_t *Mod_LoadModel(model_t *mod);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int				SkinWidth;
int				SkinHeight;
int				SkinBPP;
byte			*SkinData;
rgba_t			SkinPal[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static model_t	mod_known[MAX_MOD_KNOWN];
static int		mod_numknown;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Mod_Extradata
//
//	Caches the data if needed
//
//==========================================================================

void *Mod_Extradata(model_t *mod)
{
	void	*r;
	
	r = mod->data;
	if (r)
		return r;

	Mod_LoadModel(mod);
	
	if (!mod->data)
		Sys_Error("Mod_Extradata: caching failed");
	return mod->data;
}

//==========================================================================
//
//	Mod_FindName
//
//==========================================================================

model_t *Mod_FindName(char *name)
{
	int		i;
	model_t	*mod;

	if (!name[0])
		Sys_Error("Mod_ForName: NULL name");
		
	//
	// search the currently loaded models
	//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!strcmp(mod->name, name))
			break;
	}
			
	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
		{
			Sys_Error ("mod_numknown == MAX_MOD_KNOWN");
		}
		else
			mod_numknown++;
		strcpy(mod->name, name);
	}

	return mod;
}

//==========================================================================
//
//	Mod_SwapAliasModel
//
//==========================================================================

static void Mod_SwapAliasModel(model_t *mod)
{
	int					i, j;
	mmdl_t				*pmodel;
	mstvert_t			*pstverts;
	mtriangle_t			*ptri;
	mframe_t			*pframe;
	int					*pcmds;

	pmodel = (mmdl_t*)mod->data;

	//
	// endian-adjust and swap the data, starting with the alias model header
	//
	for (i = 0; i < (int)sizeof(mmdl_t) / 4; i++)
	{
		((int*)pmodel)[i] = LittleLong(((int*)pmodel)[i]);
	}

	if (pmodel->version != ALIAS_VERSION)
		Sys_Error("%s has wrong version number (%i should be %i)",
			mod->name, pmodel->version, ALIAS_VERSION);

	if (pmodel->numverts <= 0)
		Sys_Error("model %s has no vertices", mod->name);

	if (pmodel->numverts > MAXALIASVERTS)
		Sys_Error("model %s has too many vertices", mod->name);

	if (pmodel->numstverts <= 0)
		Sys_Error("model %s has no texture vertices", mod->name);

	if (pmodel->numstverts > MAXALIASSTVERTS)
		Sys_Error("model %s has too many texture vertices", mod->name);

	if (pmodel->numtris <= 0)
		Sys_Error("model %s has no triangles", mod->name);

	if (pmodel->skinwidth & 0x03)
		Sys_Error("Mod_LoadAliasModel: skinwidth not multiple of 4");

	if (pmodel->numskins < 1)
		Sys_Error("Mod_LoadAliasModel: Invalid # of skins: %d\n", pmodel->numskins);

	if (pmodel->numframes < 1)
		Sys_Error("Mod_LoadAliasModel: Invalid # of frames: %d\n", pmodel->numframes);

	//
	// base s and t vertices
	//
	pstverts = (mstvert_t*)((byte*)pmodel + pmodel->ofsstverts);
	for (i = 0; i < pmodel->numstverts; i++)
	{
		pstverts[i].s = LittleShort(pstverts[i].s);
		pstverts[i].t = LittleShort(pstverts[i].t);
	}

	//
	// triangles
	//
	ptri = (mtriangle_t *)((byte*)pmodel + pmodel->ofstris);
	for (i = 0; i < pmodel->numtris; i++)
	{
		for (j = 0; j < 3; j++)
		{
			ptri[i].vertindex[j] = LittleShort(ptri[i].vertindex[j]);
			ptri[i].stvertindex[j] = LittleShort(ptri[i].stvertindex[j]);
		}
	}

	//
	// frames
	//
	pframe = (mframe_t *)((byte*)pmodel + pmodel->ofsframes);
	for (i = 0; i < pmodel->numframes; i++)
	{
		pframe->scale[0] = LittleFloat(pframe->scale[0]);
		pframe->scale[1] = LittleFloat(pframe->scale[1]);
		pframe->scale[2] = LittleFloat(pframe->scale[2]);
		pframe->scale_origin[0] = LittleFloat(pframe->scale_origin[0]);
		pframe->scale_origin[1] = LittleFloat(pframe->scale_origin[1]);
		pframe->scale_origin[2] = LittleFloat(pframe->scale_origin[2]);
		pframe = (mframe_t*)((byte*)pframe + pmodel->framesize);
	}

	//
	// commands
	//
	pcmds = (int*)((byte*)pmodel + pmodel->ofscmds);
	for (i = 0; i < pmodel->numcmds; i++)
	{
		pcmds[i] = LittleLong(pcmds[i]);
	}
}

//==========================================================================
//
//	Mod_LoadModel
//
//	Loads a model into the cache
//
//==========================================================================

static model_t *Mod_LoadModel(model_t *mod)
{
	if (mod->data)
	{
		return mod;
	}

	//
	// load the file
	//
	if (FL_ReadFile(mod->name, &mod->data, PU_CACHE) < 0)
		Sys_Error("Couldn't load %s", mod->name);
	
	if (LittleLong(*(unsigned *)mod->data) != IDPOLY2HEADER)
		Sys_Error("model %s is not a md2 model", mod->name);

	// swap model
	Mod_SwapAliasModel(mod);

	return mod;
}

//==========================================================================
//
//	LoadPCX
//
//==========================================================================

static void LoadPCX(const char *filename, void **bufptr)
{
	int			c;
	int			bytes_per_line;
	int			x, y;
	char		ch;
	pcx_t		*pcx;
	byte		*data;
	int			size;

	size = FL_ReadFile(filename, (void**)&pcx, PU_HIGH);
	if (size < 0)
		Sys_Error("Couldn't find skin %s", filename);

	if (pcx->bits_per_pixel != 8)
	{
		// we like 8 bit color planes
		Sys_Error("No 8-bit planes\n");
	}
	if (pcx->color_planes != 1)
	{
		Sys_Error("Not 8 bpp\n");
	}

	SkinWidth = LittleShort(pcx->xmax) - LittleShort(pcx->xmin) + 1;
	SkinHeight = LittleShort(pcx->ymax) - LittleShort(pcx->ymin) + 1;
	SkinBPP = 8;

	bytes_per_line = pcx->bytes_per_line;

	SkinData = (byte*)Z_Malloc(SkinWidth * SkinHeight, PU_STATIC, bufptr);

	data = (byte *)(pcx + 1);

	for (y = 0; y < SkinHeight; y++)
	{
		// decompress RLE encoded PCX data
		x = 0;

		while (x < bytes_per_line)
		{
			ch = *data++;
			if ((ch & 0xC0) == 0xC0)
			{
				c = (ch & 0x3F);
				ch = *data++;
			}
			else
			{
				c = 1;
			}

			while (c--)
			{
				if (x < SkinWidth)
					SkinData[y * SkinWidth + x] = ch;
				x++;
			}
		}
	}

	if (*data == 12)
	{
		data++;
	}
	else
	{
		data = (byte *)pcx + size - 768;
	}

	for (c = 0; c < 256; c++)
	{
		SkinPal[c].r = *data++;
		SkinPal[c].g = *data++;
		SkinPal[c].b = *data++;
		SkinPal[c].a = 255;
	}

	Z_Free(pcx);
}

//==========================================================================
//
//	LoadTGA
//
//==========================================================================

static void LoadTGA(const char *filename, void **bufptr)
{
	tgaHeader_t *hdr;
	byte *data;
	int col;
	int count;
	int c;

	if (FL_ReadFile(filename, (void**)&hdr, PU_HIGH) < 0)
		Sys_Error("Couldn't find skin %s", filename);

	SkinWidth = LittleShort(hdr->width);
	SkinHeight = LittleShort(hdr->height);

	data = (byte*)(hdr + 1) + hdr->id_length;

	for (int i = 0; i < hdr->pal_colors; i++)
	{
		switch (hdr->pal_entry_size)
		{
		case 16:
			col = *(word *)data;
			SkinPal[i].r = (col & 0x1F) << 3;
			SkinPal[i].g = ((col >> 5) & 0x1F) << 3;
			SkinPal[i].b = ((col >> 10) & 0x1F) << 3;
			SkinPal[i].a = 255;
			break;
		case 24:
			SkinPal[i].b = data[0];
			SkinPal[i].g = data[1];
			SkinPal[i].r = data[2];
			SkinPal[i].a = 255;
			break;
		case 32:
			SkinPal[i].b = data[0];
			SkinPal[i].g = data[1];
			SkinPal[i].r = data[2];
			SkinPal[i].a = data[3];
			break;
		}
		data += (hdr->pal_entry_size >> 3);
	}

	/* Image type:
	 *    0 = no image data
	 *    1 = uncompressed color mapped
	 *    2 = uncompressed true color
	 *    3 = grayscale
	 *    9 = RLE color mapped
	 *   10 = RLE true color
	 *   11 = RLE grayscale
	 */

	if (hdr->img_type == 1 || hdr->img_type == 3 ||
		hdr->img_type == 9 || hdr->img_type == 11)
	{
		SkinBPP = 8;
		SkinData = (byte*)Z_Malloc(SkinWidth * SkinHeight, PU_STATIC, bufptr);
	}
	else
	{
		SkinBPP = 32;
		SkinData = (byte*)Z_Malloc(SkinWidth * SkinHeight * 4, PU_HIGH, NULL);
	}

	if (hdr->img_type == 1 && hdr->bpp == 8 && hdr->pal_type == 1)
	{
		// 8-bit, uncompressed
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			byte *dst = SkinData + yc * SkinWidth;

			memcpy(dst, data, SkinWidth);
			data += SkinWidth;
		}
	}
	else if (hdr->img_type == 2 && hdr->pal_type == 0 && hdr->bpp == 16)
	{
		// 16-bit uncompressed
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(SkinData + yc * SkinWidth * 4);

			for (int x = 0; x < SkinWidth; x++, dst++, data += 2)
			{
				col = *(word *)data;
				dst->r = ((col >> 10) & 0x1F) << 3;
				dst->g = ((col >> 5) & 0x1F) << 3;
				dst->b = (col & 0x1F) << 3;
				dst->a = 255;
			}
		}
	}
	else if (hdr->img_type == 2 && hdr->pal_type == 0 && hdr->bpp == 24)
	{
		// 24-bit uncompressed
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(SkinData + yc * SkinWidth * 4);

			for (int x = 0; x < SkinWidth; x++, dst++, data += 3)
			{
				dst->b = data[0];
				dst->g = data[1];
				dst->r = data[2];
				dst->a = 255;
			}
		}
	}
	else if (hdr->img_type == 2 && hdr->pal_type == 0 && hdr->bpp == 32)
	{
		// 32-bit uncompressed
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(SkinData + yc * SkinWidth * 4);

			for (int x = 0; x < SkinWidth; x++, dst++, data += 4)
			{
				dst->b = data[0];
				dst->g = data[1];
				dst->r = data[2];
				dst->a = data[3];
			}
		}
	}
	else if (hdr->img_type == 3 && hdr->bpp == 8 && hdr->pal_type == 1)
	{
		// Grayscale uncompressed
		for (int i = 0; i < 256; i++)
		{
			SkinPal[i].r = i;
			SkinPal[i].g = i;
			SkinPal[i].b = i;
			SkinPal[i].a = 255;
		}
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			byte *dst = SkinData + yc * SkinWidth;

			memcpy(dst, data, SkinWidth);
			data += SkinWidth;
		}
	}
	else if (hdr->img_type == 9 && hdr->bpp == 8 && hdr->pal_type == 1)
	{
		// 8-bit RLE compressed
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			byte *dst = SkinData + yc * SkinWidth;
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					while (count--)
						*(dst++) = *data;
					data++;
				}
				else
				{
					count++;
					c += count;
					memcpy(dst, data, count);
					data += count;
					dst += count;
				}
			}
			while (c < SkinWidth);
		}
	}
	else if (hdr->img_type == 10 && hdr->pal_type == 0 && hdr->bpp == 16)
	{
		// 16-bit RLE compressed
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(SkinData + yc * SkinWidth * 4);
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					col = *(word *)data;
					while (count--)
					{
						dst->r = ((col >> 10) & 0x1F) << 3;
						dst->g = ((col >> 5) & 0x1F) << 3;
						dst->b = (col & 0x1F) << 3;
						dst->a = 255;
						dst++;
					}
					data += 2;
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						col = *(word *)data;
						dst->r = ((col >> 10) & 0x1F) << 3;
						dst->g = ((col >> 5) & 0x1F) << 3;
						dst->b = (col & 0x1F) << 3;
						dst->a = 255;
						data += 2;
						dst++;
					}
				}
			}
			while (c < SkinWidth);
		}
	}
	else if (hdr->img_type == 10 && hdr->pal_type == 0 && hdr->bpp == 24)
	{
		// 24-bit REL compressed
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(SkinData + yc * SkinWidth * 4);
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					while (count--)
					{
						dst->r = data[2];
						dst->g = data[1];
						dst->b = data[0];
						dst->a = 255;
						dst++;
					}
					data += 3;
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						dst->r = data[2];
						dst->g = data[1];
						dst->b = data[0];
						dst->a = 255;
						data += 3;
						dst++;
					}
				}
			}
			while (c < SkinWidth);
		}
	}
	else if (hdr->img_type == 10 && hdr->pal_type == 0 && hdr->bpp == 32)
	{
		// 32-bit RLE compressed
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			rgba_t *dst = (rgba_t*)(SkinData + yc * SkinWidth * 4);
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					while (count--)
					{
						dst->r = data[2];
						dst->g = data[1];
						dst->b = data[0];
						dst->a = data[3];
						dst++;
					}
					data += 4;
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						dst->r = data[2];
						dst->g = data[1];
						dst->b = data[0];
						dst->a = data[3];
						data += 4;
						dst++;
					}
				}
			}
			while (c < SkinWidth);
		}
	}
	else if (hdr->img_type == 11 && hdr->bpp == 8 && hdr->pal_type == 1)
	{
		// Grayscale RLE compressed
		for (int i = 0; i < 256; i++)
		{
			SkinPal[i].r = i;
			SkinPal[i].g = i;
			SkinPal[i].b = i;
			SkinPal[i].a = 255;
		}
		for (int y = SkinHeight; y; y--)
		{
			int yc = hdr->descriptor_bits & 0x20 ? SkinHeight - y : y - 1;
			byte *dst = SkinData + yc * SkinWidth;
			c = 0;

			do
			{
				count = *data++;
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					while (count--)
						*(dst++) = *data;
					data++;
				}
				else
				{
					count++;
					c += count;
					memcpy(dst, data, count);
					data += count;
					dst += count;
				}
			}
			while (c < SkinWidth);
		}
	}
	else
	{
		Sys_Error("Nonsupported tga format");
	}


	Z_Free(hdr);
}

//==========================================================================
//
//	WriteTGA
//
//==========================================================================

void WriteTGA(char* filename, void* data, int width, int height, int bpp,
	byte* palette, bool bot2top)
{
	ofstream s(filename, ios::out | ios::binary);
	if (!s)
	{
		con << "Couldn't write tga\n";
		return;
	}

	tgaHeader_t hdr;
	hdr.id_length = 0;
	hdr.pal_type = (bpp == 8) ? 1 : 0;
	hdr.img_type = (bpp == 8) ? 1 : 2;
	hdr.first_color = 0;
	hdr.pal_colors = (bpp == 8) ? 256 : 0;
	hdr.pal_entry_size = (bpp == 8) ? 24 : 0;
	hdr.left = 0;
	hdr.top = 0;
	hdr.width = width;
	hdr.height = height;
	hdr.bpp = bpp;
	hdr.descriptor_bits = bot2top ? 0 : 0x20;
	s.write((char *)&hdr, sizeof(hdr));

	if (bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			s.put(palette[i * 3 + 2]);
			s.put(palette[i * 3 + 1]);
			s.put(palette[i * 3]);
		}
	}

	if (bpp == 8)
	{
		s.write((char *)data, width * height);
	}
	else if (bpp == 24)
	{
		rgb_t *src = (rgb_t *)data;
		for (int i = 0; i < width * height; i++, src++)
		{
			s.put(src->b);
			s.put(src->g);
			s.put(src->r);
		}
	}

	s.close();
}

//==========================================================================
//
//	Mod_LoadSkin
//
//==========================================================================

void Mod_LoadSkin(const char *name, void **bufptr)
{
	char ext[8];

	FL_ExtractFileExtension(name, ext);
	if (!strcmp(ext, "pcx"))
	{
		LoadPCX(name, bufptr);
	}
	else if (!strcmp(ext, "tga"))
	{
		LoadTGA(name, bufptr);
	}
	else
	{
		Sys_Error("Unsupported graphics format");
	}
}

//==========================================================================
//
//	R_PositionWeaponModel
//
//==========================================================================

void R_PositionWeaponModel(clmobj_t &wpent, model_t *wpmodel, int frame)
{
	mmdl_t *pmdl = (mmdl_t*)Mod_Extradata(wpmodel);
	if ((frame >= pmdl->numframes) || (frame < 0))
	{
		frame = 0;
	}
	mtriangle_t *ptris = (mtriangle_t*)((byte*)pmdl + pmdl->ofstris);
	mframe_t *pframe = (mframe_t*)((byte*)pmdl + pmdl->ofsframes +
		frame * pmdl->framesize);
	trivertx_t *pverts = (trivertx_t *)(pframe + 1);
	TVec p[3];
	for (int vi = 0; vi < 3; vi++)
	{
		p[vi].x = pverts[ptris[0].vertindex[vi]].v[0] * pframe->scale[0] + pframe->scale_origin[0];
		p[vi].y = pverts[ptris[0].vertindex[vi]].v[1] * pframe->scale[1] + pframe->scale_origin[1];
		p[vi].z = pverts[ptris[0].vertindex[vi]].v[2] * pframe->scale[2] + pframe->scale_origin[2];
	}
	TVec md_forward, md_left, md_up;
	AngleVectors(wpent.angles, md_forward, md_left, md_up);
	md_left = -md_left;
	wpent.origin += md_forward * p[0].x + md_left * p[0].y + md_up * p[0].z;
	TAVec wangles;
	VectorAngles(p[1] - p[0], wangles);
	wpent.angles.yaw = AngleMod(wpent.angles.yaw + wangles.yaw);
	wpent.angles.pitch = AngleMod(wpent.angles.pitch + wangles.pitch);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//
//	Revision 1.3  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.2  2001/09/20 16:24:16  dj_jl
//	Added support for tga skins
//	
//	Revision 1.1  2001/09/06 17:46:37  dj_jl
//	no message
//	
//	Revision 1.4  2001/08/15 17:18:05  dj_jl
//	Removed MAX_SKIN_HEIGHT
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
