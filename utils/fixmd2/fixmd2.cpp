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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:55  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "cmdlib.h"
#include "fmd2defs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char			filename[256];
static mmdl_t		*model;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	LoadModel
//
//==========================================================================

static void LoadModel(void)
{
	LoadFile(filename, (void**)&model);
}

//==========================================================================
//
//	WriteModel
//
//==========================================================================

static void WriteModel(void)
{
	FILE *f = fopen(filename, "wb");
	fwrite(model, 1, model->ofsend, f);
	fclose(f);
}

//==========================================================================
//
//	ScaleModel
//
//==========================================================================

static void ScaleModel(double scale)
{
	mframe_t *frame = (mframe_t*)((byte*)model + model->ofsframes);
	for (int i = 0; i < model->numframes; i++)
	{
		frame->scale[0] *= scale;
		frame->scale[1] *= scale;
		frame->scale[2] *= scale;
		frame->scale_origin[0] *= scale;
		frame->scale_origin[1] *= scale;
		frame->scale_origin[2] *= scale;
		frame = (mframe_t*)((byte*)frame + model->framesize);
	}
}

//==========================================================================
//
//	ShiftModel
//
//==========================================================================

static void ShiftModel(double x, double y, double z)
{
	mframe_t *frame = (mframe_t*)((byte*)model + model->ofsframes);
	for (int i = 0; i < model->numframes; i++)
	{
		frame->scale_origin[0] += x;
		frame->scale_origin[1] += y;
		frame->scale_origin[2] += z;
		frame = (mframe_t*)((byte*)frame + model->framesize);
	}
}

//==========================================================================
//
//	FixModelSkin
//
//==========================================================================

static void FixModelSkin(const char *name)
{
	mskin_t *skin = (mskin_t*)((byte*)model + model->ofsskins);
	strcpy(skin->name, name);
}

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cerr << "Usage: fixmd2 <model> [options]\n";
		cerr << "Options are:\n";
		cerr << "  s<scale>  - scale model by scale\n";
		cerr << "  i<iscale> - scale model by 1/iscale\n";
		cerr << "  x<shift>  - shift origin on x\n";
		cerr << "  y<shift>  - shift origin on y\n";
		cerr << "  z<shift>  - shift origin on z\n";
		cerr << "  /<name>   - set skin\n";
		return -1;
	}
	strcpy(filename, argv[1]);
	LoadModel();
	for (int i = 2; i < argc; i++)
	{
		if (argv[i][0] == 's')
		{
			ScaleModel(atof(argv[i] + 1));
		}
		if (argv[i][0] == 'i')
		{
			ScaleModel(1.0 / atof(argv[i] + 1));
		}
		if (argv[i][0] == 'x')
		{
			ShiftModel(atof(argv[i] + 1), 0, 0);
		}
		if (argv[i][0] == 'y')
		{
			ShiftModel(0, atof(argv[i] + 1), 0);
		}
		if (argv[i][0] == 'z')
		{
			ShiftModel(0, 0, atof(argv[i] + 1));
		}
		if (argv[i][0] == '/')
		{
			FixModelSkin(argv[i] + 1);
		}
	}
	WriteModel();
	return 0;
}
