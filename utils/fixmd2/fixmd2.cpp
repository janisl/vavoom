//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
#include "scrlib.h"
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
static float		sparms[3];
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

static void ScaleModel(void)
{
	mframe_t *frame = (mframe_t*)((byte*)model + model->ofsframes);
	for (int i = 0; i < model->numframes; i++)
	{
		frame->scale[0] *= sparms[0];
		frame->scale[1] *= sparms[1];
		frame->scale[2] *= sparms[2];
		frame = (mframe_t*)((byte*)frame + model->framesize);
	}
}

//==========================================================================
//
//	ShiftModel
//
//==========================================================================

static void ShiftModel(void)
{
	mframe_t *frame = (mframe_t*)((byte*)model + model->ofsframes);
	for (int i = 0; i < model->numframes; i++)
	{
		frame->scale_origin[0] += sparms[0];
		frame->scale_origin[1] += sparms[1];
		frame->scale_origin[2] += sparms[2];
		frame = (mframe_t*)((byte*)frame + model->framesize);
	}
}

//==========================================================================
//
//	FixModelSkin
//
//==========================================================================

static void FixModelSkin(void)
{
	mskin_t *skin = (mskin_t*)((byte*)model + model->ofsskins);
	strcpy(skin->name, sc_String);
}

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "Usage: fixmd2 <script>\n";
		return -1;
	}
	SC_Open(argv[1]);
	while (SC_GetString())
	{
		strcpy(filename, sc_String);
		LoadModel();
		do
		{
			SC_MustGetString();
			if (SC_Compare("scale"))
			{
				SC_MustGetFloat();
				sparms[0] = sc_Float;
				SC_MustGetFloat();
				sparms[1] = sc_Float;
				SC_MustGetFloat();
				sparms[2] = sc_Float;
				ScaleModel();
			}
			else if (SC_Compare("shift"))
			{
				SC_MustGetFloat();
				sparms[0] = sc_Float;
				SC_MustGetFloat();
				sparms[1] = sc_Float;
				SC_MustGetFloat();
				sparms[2] = sc_Float;
				ShiftModel();
			}
			else if (SC_Compare("skin"))
			{
				SC_MustGetString();
				FixModelSkin();
			}
			else if (!SC_Compare("end"))
			{
				SC_ScriptError("Bad command");
			}
		}
		while (!SC_Compare("end"));
		WriteModel();
	}
	SC_Close();
	return 0;
}
