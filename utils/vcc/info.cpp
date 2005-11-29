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

// HEADER FILES ------------------------------------------------------------

#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct state_t
{
	int		sprite;
	int		frame;
	int		model_index;
	int		model_frame;
	float	time;
	int		nextstate;
	int		function;
	FName	statename;
};

struct mobjinfo_t
{
    int		doomednum;
	int		class_id;
};

struct compstate_t
{
	FName NextName;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<FName>		sprite_names;

static TArray<FName>		models;

static TArray<state_t>		states;
static TArray<compstate_t>	compstates;

static TArray<mobjinfo_t>	mobj_info;

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

void InitInfoTables()
{
	//	For some strange reason when compiling with gcc, data field contains
	// some garbage already at the start of the program.
	memset(&mobj_info, 0, sizeof(mobj_info));

	sprite_names.Empty(64);
	models.Empty(64);
	states.Empty(1024);
	compstates.Empty(1024);
	mobj_info.Empty(128);
	models.AddItem(NAME_None);	// 0 indicates no-model

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
//	FindState
//
//==========================================================================

static int FindState(FName StateName)
{
	for (TArray<state_t>::TIterator It(states); It; ++It)
	{
		if (It->statename == StateName)
		{
			return It.GetIndex();
		}
	}
	ParseError("No such state %s", *StateName);
	return 0;
}

//==========================================================================
//
//	ParseStates
//
//==========================================================================

void ParseStates(TClass* InClass)
{
	int i;

	if (!InClass && TK_Check(PU_LPAREN))
	{
		InClass = CheckForClass();
		if (!InClass)
		{
			ParseError("Class name expected");
		}
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		state_t &s = *new(states) state_t;
		memset(&s, 0, sizeof(s));
		compstate_t &cs = *new(compstates) compstate_t;
		memset(&cs, 0, sizeof(cs));

		//	State identifier
		if (tk_Token != TK_IDENTIFIER)
		{
			ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
		}
		s.statename = tk_Name;
		AddConstant(tk_Name, states.Num() - 1);
		TK_NextToken();
		TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
		//	Sprite name
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
			for (i = 0; i < sprite_names.Num(); i++)
			{
		   		if (sprite_names[i] == tk_Name)
				{
				   	break;
				}
			}
			if (i == sprite_names.Num())
			{
			   	i = sprite_names.AddItem(tk_Name);
			}
			s.sprite = i;
		}
		else
		{
			s.sprite = 0;
		}
		TK_NextToken();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Frame
		s.frame = EvalConstExpression(ev_int);
		TK_Expect(PU_COMMA, ERR_NONE);
		if (tk_Token == TK_NAME)
		{
			//	Model
			for (i = 0; i < models.Num(); i++)
			{
		   		if (models[i] == tk_Name)
				{
				   	break;
				}
			}
			if (i == models.Num())
			{
			   	i = models.AddItem(tk_Name);
			}
			s.model_index = i;
			TK_NextToken();
			TK_Expect(PU_COMMA, ERR_NONE);
			//  Frame
			s.model_frame = EvalConstExpression(ev_int);
			TK_Expect(PU_COMMA, ERR_NONE);
		}
		else
		{
			s.model_index = 0;
			s.model_frame = 0;
		}
		//  Tics
		s.time = ConstFloatExpression();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Next state
		if (tk_Token != TK_IDENTIFIER)
		{
			ERR_Exit(ERR_NONE, true, NULL);
		}
		cs.NextName = tk_Name;
		TK_NextToken();
		TK_Expect(PU_RPAREN, ERR_NONE);
		//	Code
		s.function = ParseStateCode(InClass);
		functions[s.function].Name = va("%s_func", *s.statename);
	}
}

//==========================================================================
//
//	AddToMobjInfo
//
//==========================================================================

void AddToMobjInfo(int Index, int ClassID)
{
	int i = mobj_info.Add();
	mobj_info[i].doomednum = Index;
	mobj_info[i].class_id = ClassID;
}

//==========================================================================
//
//	SkipStates
//
//==========================================================================

void SkipStates(TClass* InClass)
{
	if (!InClass && TK_Check(PU_LPAREN))
	{
		InClass = CheckForClass();
		if (!InClass)
		{
			ParseError("Class name expected");
		}
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		state_t &s = states[FindState(tk_Name)];

		//	State identifier
		if (tk_Token != TK_IDENTIFIER)
		{
			ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
		}
		TK_NextToken();
		TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
		//	Sprite name
		TK_NextToken();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Frame
		EvalConstExpression(ev_int);
		TK_Expect(PU_COMMA, ERR_NONE);
		if (tk_Token == TK_NAME)
		{
			TK_NextToken();
			TK_Expect(PU_COMMA, ERR_NONE);
			//  Frame
			EvalConstExpression(ev_int);
			TK_Expect(PU_COMMA, ERR_NONE);
		}
		//  Tics
		ConstFloatExpression();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Next state
		if (tk_Token != TK_IDENTIFIER)
		{
			ERR_Exit(ERR_NONE, true, NULL);
		}
		TK_NextToken();
		TK_Expect(PU_RPAREN, ERR_NONE);
		//	Code
		CompileStateCode(InClass, s.function);
	}
}

//==========================================================================
//
//	CheckStates
//
//==========================================================================

static void CheckStates()
{
	int		i;
	int		j;

	for (i = 0; i < states.Num(); i++)
	{
		for (j = 0; j < states.Num(); j++)
		{
			if (compstates[i].NextName == states[j].statename)
			{
				states[i].nextstate = j;
				break;
			}
		}
		if (j == states.Num())
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
		memset(globalinfo + numglobals, GLOBALTYPE_Name, size / 4);
	}
	numglobals += size / 4;
}

//==========================================================================
//
//  AddInfoTables
//
//==========================================================================

void AddInfoTables()
{
	int i;

	CheckStates();

	//  Add sprite names
	i = sprite_names.Num();
	AddInfoData(gv_num_sprite_names, &i, 4, false);
	AddInfoData(gv_sprite_names, sprite_names.GetData(), 4 * sprite_names.Num(), true);
	//  Add models
	i = models.Num();
	AddInfoData(gv_num_models, &i, 4, false);
	AddInfoData(gv_models, models.GetData(), 4 * models.Num(), true);
	//	Add state table
	i = states.Num();
	AddInfoData(gv_num_states, &i, 4, false);
	for (i = 0; i < states.Num(); i++)
	{
		if (states[i].function)
		{
			globalinfo[numglobals + i * sizeof(state_t) / 4 +
				STRUCT_OFFSET(state_t, function) / 4] = GLOBALTYPE_Function;
		}
		globalinfo[numglobals + i * sizeof(state_t) / 4 +
			STRUCT_OFFSET(state_t, statename) / 4] = GLOBALTYPE_Name;
	}
	AddInfoData(gv_states, states.GetData(), states.Num() * sizeof(state_t), false);
	//	Add object description table
	i = mobj_info.Num();
	AddInfoData(gv_num_mobj_info, &i, 4, false);
	for (i = 0; i < mobj_info.Num(); i++)
	{
		globalinfo[numglobals + i * sizeof(mobjinfo_t) / 4 + 1] = GLOBALTYPE_Class;
	}
	AddInfoData(gv_mobj_info, mobj_info.GetData(), mobj_info.Num() * sizeof(mobjinfo_t), false);

	dprintf("Num sprite names: %d, num models: %d\n",
				sprite_names.Num(), models.Num());
	dprintf("Num states: %d, num mobj types: %d\n",
				states.Num(), mobj_info.Num());
}

//**************************************************************************
//
//	$Log$
//	Revision 1.25  2005/11/29 19:31:43  dj_jl
//	Class and struct classes, removed namespaces, beautification.
//
//	Revision 1.24  2005/11/24 20:42:05  dj_jl
//	Renamed opcodes, cleanup and improvements.
//	
//	Revision 1.23  2005/04/28 07:00:40  dj_jl
//	Temporary fix for crash with optimisations.
//	
//	Revision 1.22  2003/10/22 06:42:55  dj_jl
//	Added function name
//	
//	Revision 1.21  2003/03/08 12:47:52  dj_jl
//	Code cleanup.
//	
//	Revision 1.20  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//	
//	Revision 1.19  2002/07/20 14:53:34  dj_jl
//	Switched to dynamic arrays.
//	
//	Revision 1.18  2002/06/14 15:33:45  dj_jl
//	Some fixes.
//	
//	Revision 1.17  2002/02/22 18:11:53  dj_jl
//	Removed misc fields from states.
//	
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
