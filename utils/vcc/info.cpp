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
//**	Copyright (C) 1999-2002 J∆nis Legzdi∑˝
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

#include "vcc.h"

// MACROS ------------------------------------------------------------------

#define MAX_STATES				(4 * 1024)
#define MAX_SPRITE_NAMES		1024
#define MAX_MODELS				1024
#define MAX_MOBJ_TYPES			1024

// TYPES -------------------------------------------------------------------

struct state_t
{
	int		sprite;
	int		frame;
	int		model_index;
	int		model_frame;
	float	time;
	int		statenum;
	int		nextstate;
	float	misc1;
    float	misc2;
	int		function;
};

struct mobjinfo_t
{
    int		doomednum;
	int		class_id;
};

struct compstate_t
{
	FName Name;
	FName NextName;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int				num_sprite_names;
static FName			*sprite_names;

static int				num_models;
static FName			*models;

static int				num_states;
static state_t			*states = NULL;
static compstate_t		*compstates = NULL;

static mobjinfo_t		*mobj_info = NULL;
static int				num_mobj_types;

static int				gv_num_sprite_names;
static int				gv_sprite_names;
static int				gv_num_models;
static int				gv_models;
static int				gv_num_states;
static int				gv_states;
static int				gv_num_mobj_info;
static int				gv_mobj_info;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	InitInfoTables
//
//==========================================================================

void InitInfoTables(void)
{
	sprite_names = new FName[MAX_SPRITE_NAMES];
	memset(sprite_names, 0, MAX_SPRITE_NAMES * 4);
	num_sprite_names = 0;

	models = new FName[MAX_MODELS];
	memset(models, 0, MAX_MODELS * 4);
	num_models = 1; // 0 indicates no-model

	states = new state_t[MAX_STATES];
	memset(states, 0, MAX_STATES * sizeof(state_t));
	compstates = new compstate_t[MAX_STATES];
	memset(compstates, 0, MAX_STATES * sizeof(compstate_t));
	num_states = 0;

	mobj_info = new mobjinfo_t[MAX_MOBJ_TYPES];
	memset(mobj_info, 0, MAX_MOBJ_TYPES * sizeof(mobjinfo_t));
	num_mobj_types = 0;

	globaldefs[numglobaldefs].Name = "num_sprite_names";
	globaldefs[numglobaldefs].type = &type_int;
	globaldefs[numglobaldefs].ofs = 0;
	gv_num_sprite_names = numglobaldefs++;

	globaldefs[numglobaldefs].Name = "sprite_names";
	globaldefs[numglobaldefs].type = MakeArrayType(&type_name, -1);
	globaldefs[numglobaldefs].ofs = 0;
	gv_sprite_names = numglobaldefs++;

	globaldefs[numglobaldefs].Name = "num_models";
	globaldefs[numglobaldefs].type = &type_int;
	globaldefs[numglobaldefs].ofs = 0;
	gv_num_models = numglobaldefs++;

	globaldefs[numglobaldefs].Name = "models";
	globaldefs[numglobaldefs].type = MakeArrayType(&type_name, -1);
	globaldefs[numglobaldefs].ofs = 0;
	gv_models = numglobaldefs++;

	globaldefs[numglobaldefs].Name = "num_states";
	globaldefs[numglobaldefs].type = &type_int;
	globaldefs[numglobaldefs].ofs = 0;
	gv_num_states = numglobaldefs++;

	globaldefs[numglobaldefs].Name = "states";
	globaldefs[numglobaldefs].type = MakeArrayType(&type_state, -1);
	globaldefs[numglobaldefs].ofs = 0;
	gv_states = numglobaldefs++;

	globaldefs[numglobaldefs].Name = "num_mobj_types";
	globaldefs[numglobaldefs].type = &type_int;
	globaldefs[numglobaldefs].ofs = 0;
	gv_num_mobj_info = numglobaldefs++;

	globaldefs[numglobaldefs].Name = "mobjinfo";
	globaldefs[numglobaldefs].type = MakeArrayType(&type_mobjinfo, -1);
	globaldefs[numglobaldefs].ofs = 0;
	gv_mobj_info = numglobaldefs++;
}

//==========================================================================
//
//	ParseStates
//
//==========================================================================

void ParseStates(TType *class_type)
{
	int i;

	if (!class_type && TK_Check(PU_LPAREN))
	{
		class_type = CheckForType();
		if (!class_type || class_type->type != ev_class)
		{
			ParseError("Class name expected");
		}
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		if (num_states >= MAX_STATES)
		{
			ERR_Exit(ERR_NONE, true, "States overflow.");
		}

		state_t &s = states[num_states];
		compstate_t &cs = compstates[num_states];

		//	St∆vokıa identifik∆tors
		if (tk_Token != TK_IDENTIFIER)
		{
			ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
		}
		cs.Name = tk_Name;
		AddConstant(tk_Name, num_states);
		TK_NextToken();
		TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
		//	Nummurs
		s.statenum = num_states;
		//	Spraita v∆rds
		if (tk_Token != TK_NAME)
		{
			ERR_Exit(ERR_NONE, true, "Sprite name expected");
		}
		if (tk_Name != NAME_None)
		{
			if (strlen(*tk_Name) != 4)
			{
				ERR_Exit(ERR_NONE, true, "Invalid sprite name");
			}
			for (i = 0; i < num_sprite_names; i++)
			{
		   		if (sprite_names[i] == tk_Name)
				{
				   	break;
				}
			}
			if (i == num_sprite_names)
			{
			   	sprite_names[i] = tk_Name;
				num_sprite_names++;
			}
			s.sprite = i;
		}
		else
		{
			s.sprite = 0;
		}
		TK_NextToken();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Kadrs
		s.frame = EvalConstExpression(ev_int);
		TK_Expect(PU_COMMA, ERR_NONE);
		if (tk_Token == TK_NAME)
		{
			//	Modelis
			for (i = 0; i < num_models; i++)
			{
		   		if (models[i] == tk_Name)
				{
				   	break;
				}
			}
			if (i == num_models)
			{
			   	models[i] = tk_Name;
				num_models++;
			}
			s.model_index = i;
			TK_NextToken();
			TK_Expect(PU_COMMA, ERR_NONE);
			//  Kadrs
			s.model_frame = EvalConstExpression(ev_int);
			TK_Expect(PU_COMMA, ERR_NONE);
		}
		else
		{
			s.model_index = 0;
			s.model_frame = 0;
		}
		//  Taktis
		s.time = ConstFloatExpression();
		TK_Expect(PU_COMMA, ERR_NONE);
#if 0
		//	Funkcija
		if (TK_Check(KW_NULL))
		{
			s.function = 0;
		}
		else
		{
			field_t *field = CheckForField(class_type);
			if (!field || field->type->type != ev_method)
			{
				ERR_Exit(ERR_NONE, true, "Function name expected");
			}
			s.function = field->func_num;
		}
		TK_Expect(PU_COMMA, ERR_NONE);
#endif
		//  N∆ko˝ais st∆voklis
		if (tk_Token != TK_IDENTIFIER)
		{
			ERR_Exit(ERR_NONE, true, NULL);
		}
		cs.NextName = tk_Name;
		TK_NextToken();
		if (TK_Check(PU_COMMA))
		{
			//	Misc1
			s.misc1 = ConstFloatExpression();
			TK_Expect(PU_COMMA, ERR_NONE);
			//	Misc2
			s.misc2 = ConstFloatExpression();
		}
		else
		{
			s.misc1 = 0.0;
			s.misc2 = 0.0;
		}
		TK_Expect(PU_RPAREN, ERR_NONE);
		//	Code
		s.function = ParseStateCode(class_type);

		num_states++;
	}
}

//==========================================================================
//
//	AddToMobjInfo
//
//==========================================================================

void AddToMobjInfo(int Index, int ClassID)
{
	mobj_info[num_mobj_types].doomednum = Index;
	mobj_info[num_mobj_types].class_id = ClassID;
	num_mobj_types++;
}

//==========================================================================
//
//	CheckStates
//
//==========================================================================

static void CheckStates(void)
{
	int		i;
	int		j;

	for (i = 0; i < num_states; i++)
	{
		for (j = 0; j < num_states; j++)
		{
			if (compstates[i].NextName == compstates[j].Name)
			{
				states[i].nextstate = j;
				break;
			}
		}
		if (j == num_states)
		{
			ERR_Exit(ERR_NONE, true, "State named \"%s\" was not defined",
				*compstates[i].NextName);
		}
	}

}

//==========================================================================
//
//  AddInfoData
//
//==========================================================================

static void AddInfoData(int globaldef, void *data, int size, bool names)
{
	globaldefs[globaldef].ofs = numglobals;
	memcpy(&globals[numglobals], data, size);
	if (names)
	{
		memset(globalinfo + numglobals, 4, size / 4);
	}
	numglobals += size / 4;
}

//==========================================================================
//
//  AddInfoTables
//
//==========================================================================

void AddInfoTables(void)
{
	int i;

	CheckStates();

	//  Pievieno spraitu v∆rdus
	AddInfoData(gv_num_sprite_names, &num_sprite_names, 4, false);
	AddInfoData(gv_sprite_names, sprite_names, 4 * num_sprite_names, true);
	//  Pievieno modeıus
	AddInfoData(gv_num_models, &num_models, 4, false);
	AddInfoData(gv_models, models, 4 * num_models, true);
	//	Pievieno st∆vokıu tabulu
	AddInfoData(gv_num_states, &num_states, 4, false);
	for (i = 0; i < num_states; i++)
	{
		if (states[i].function)
		{
			globalinfo[numglobals + i * sizeof(*states) / 4 + 9] = 2;
		}
	}
	AddInfoData(gv_states, states, num_states * sizeof(*states), false);
	//	Pievieno objektu aprakstu tabulu
	AddInfoData(gv_num_mobj_info, &num_mobj_types, 4, false);
	for (i = 0; i < num_mobj_types; i++)
	{
		globalinfo[numglobals + i * sizeof(*mobj_info) / 4 + 1] = 3;
	}
	AddInfoData(gv_mobj_info, mobj_info, num_mobj_types * sizeof(*mobj_info), false);

	dprintf("Num sprite names: %d, num models: %d\n",
				num_sprite_names, num_models);
	dprintf("Num states: %d, num mobj types: %d\n",
				num_states, num_mobj_types);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.16  2002/02/16 16:28:36  dj_jl
//	Added support for bool variables
//
//	Revision 1.15  2002/01/17 18:19:52  dj_jl
//	New style of adding to mobjinfo, some fixes
//	
//	Revision 1.14  2002/01/12 18:06:34  dj_jl
//	New style of state functions, some other changes
//	
//	Revision 1.13  2002/01/11 18:21:49  dj_jl
//	Started to use names in progs
//	
//	Revision 1.12  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//	
//	Revision 1.11  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//	
//	Revision 1.10  2001/12/18 19:09:41  dj_jl
//	Some extra info in progs and other small changes
//	
//	Revision 1.9  2001/12/12 19:22:22  dj_jl
//	Support for method usage as state functions, dynamic cast
//	Added dynamic arrays
//	
//	Revision 1.8  2001/12/01 18:17:09  dj_jl
//	Fixed calling of parent method, speedup
//	
//	Revision 1.7  2001/11/09 14:42:28  dj_jl
//	References, beautification
//	
//	Revision 1.6  2001/10/22 17:28:02  dj_jl
//	Removed mobjinfo index constants
//	
//	Revision 1.5  2001/10/02 17:44:52  dj_jl
//	Some optimizations
//	
//	Revision 1.4  2001/09/27 17:05:57  dj_jl
//	Removed spawn functions, added mobj classes
//	
//	Revision 1.3  2001/08/21 17:52:54  dj_jl
//	Added support for real string pointers, beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
