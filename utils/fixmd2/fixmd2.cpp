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
	if (model->ident != IDPOLY2HEADER)
		Error("Not a model");
	if (model->version != ALIAS_VERSION)
		Error("Bad version");
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
	strncpy(skin->name, name, sizeof(skin->name));
}

//==========================================================================
//
//	MoveSTVerts
//
//==========================================================================

static void MoveSTVerts(const char *str)
{
	int			olds1, oldt1;
	int			olds2, oldt2;
	int			news, newt;
	char		*sp;
	mstvert_t	*vert;
	int			i;

	olds1 = strtol(str, &sp, 0);
	if (*sp != ',')
		Error("Bad syntax");
	sp++;
	oldt1 = strtol(sp, &sp, 0);
	if (*sp != ',')
		Error("Bad syntax");
	sp++;
	olds2 = strtol(sp, &sp, 0);
	if (*sp != ',')
		Error("Bad syntax");
	sp++;
	oldt2 = strtol(sp, &sp, 0);
	if (*sp != ',')
		Error("Bad syntax");
	sp++;
	news = strtol(sp, &sp, 0);
	if (*sp != ',')
		Error("Bad syntax");
	sp++;
	newt = strtol(sp, &sp, 0);
	if (*sp)
		Error("Bad syntax");

	vert = (mstvert_t*)((byte*)model + model->ofsstverts);
	for (i = 0; i < model->numstverts; i++)
	{
		if (vert[i].s >= olds1 && vert[i].t >= oldt1 &&
			vert[i].s < olds2 && vert[i].t < oldt2)
		{
			vert[i].s = vert[i].s - olds1 + news;
			vert[i].t = vert[i].t - oldt1 + newt;
		}
	}
	//FIXME process GL commands
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
		else if (argv[i][0] == 'i')
		{
			ScaleModel(1.0 / atof(argv[i] + 1));
		}
		else if (argv[i][0] == 'x')
		{
			ShiftModel(atof(argv[i] + 1), 0, 0);
		}
		else if (argv[i][0] == 'y')
		{
			ShiftModel(0, atof(argv[i] + 1), 0);
		}
		else if (argv[i][0] == 'z')
		{
			ShiftModel(0, 0, atof(argv[i] + 1));
		}
		else if (argv[i][0] == '/')
		{
			FixModelSkin(argv[i] + 1);
		}
		else if (argv[i][0] == 'm')
		{
			MoveSTVerts(argv[i] + 1);
		}
		else
		{
			Error("Bad args\n");
		}
	}
	WriteModel();
	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/09/24 17:29:38  dj_jl
//	Beautification
//
//	Revision 1.4  2001/08/04 17:38:19  dj_jl
//	Added moving of texture vertexes
//
//	Revision 1.3  2001/07/31 17:03:38  dj_jl
//	Added checks for model file
//
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
