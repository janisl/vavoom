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
//**	
//**	Switches, buttons. Two-state animation. Exits.
//**
//**************************************************************************

//==================================================================
//
//      CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
//==================================================================

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

#define	MAXBUTTONS	(4 * MAXPLAYERS)	// 4 buttons each player at once, max.
#define BUTTONTIME	35					// 1 second

// TYPES -------------------------------------------------------------------

typedef enum
{
	SWTCH_TOP,
	SWTCH_MIDDLE,
	SWTCH_BOTTOM
} bwhere_e;

typedef struct
{
    line_t*		line;
    bwhere_e	where;
    int			btexture;
    int			btimer;
} button_t;

struct terrainType_t
{
	int			pic;
	int			type;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

//	Switches
static int				numswitches;
static int				*switchlist;
//	Switch sounds
static int				*switch_sound;
//	Buttons
static button_t			buttonlist[MAXBUTTONS];

//	Terrain types, maby not right place for them
static terrainType_t	*terrainTypes;
static int				numTerrainTypes;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	P_InitSwitchList
//
//	Only called at game initialization.
//
//==========================================================================

void P_InitSwitchList(void)
{
    int			index;
	int			t1;
	int			t2;

    index = 0;
	switchlist = (int*)Z_StrMalloc(1);
	switch_sound = (int*)Z_StrMalloc(1);
	SC_Open("switches");
	while (SC_GetString())
	{
		t1 = R_CheckTextureNumForName(sc_String);

		SC_MustGetString();
		t2 = R_CheckTextureNumForName(sc_String);

		SC_MustGetString();

    	if ((t1 < 0) || (t2 < 0))
		{
			continue;
		}
		Z_Resize((void**)&switch_sound, (index + 2) * 2);
		Z_Resize((void**)&switchlist, (index + 2) * 4);
		switch_sound[index / 2] = S_GetSoundID(sc_String);
    	switchlist[index++] = t1;
    	switchlist[index++] = t2;
	}
	SC_Close();
   	numswitches = index / 2;
}

//==========================================================================
//
//  P_ClearButtons
//
//==========================================================================

void P_ClearButtons(void)
{
	int		i;

    for (i = 0;i < MAXBUTTONS;i++)
		memset(&buttonlist[i], 0, sizeof(button_t));
}

//==========================================================================
//
//  P_StartButton
//
//	Start a button counting down till it turns off.
//
//==========================================================================

static void P_StartButton(line_t* line, bwhere_e w, int texture, int time)
{
    int		i;
    
    // See if button is already pressed
    for (i = 0;i < MAXBUTTONS;i++)
    {
		if (buttonlist[i].btimer && buttonlist[i].line == line)
		{
		    return;
		}
    }
    
	for (i = 0;i < MAXBUTTONS;i++)
	{
		if (!buttonlist[i].btimer)
		{
			buttonlist[i].line = line;
			buttonlist[i].where = w;
			buttonlist[i].btexture = texture;
			buttonlist[i].btimer = time;
			return;
		}
	}

	Sys_Error("P_StartButton: no button slots left!");
}

//==========================================================================
//
//  P_ChangeSwitchTexture
//
//	Function that changes wall texture.
//	Tell it if switch is ok to use again (1=yes, it's a button).
//
//==========================================================================

void P_ChangeSwitchTexture(line_t* line, int useAgain)
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     i;

    texTop = level.sides[line->sidenum[0]].toptexture;
    texMid = level.sides[line->sidenum[0]].midtexture;
    texBot = level.sides[line->sidenum[0]].bottomtexture;

    for (i = 0;i < numswitches*2;i++)
    {
		if (switchlist[i] == texTop)
		{
			SV_SectorStartSound(line->frontsector,
				switch_sound[i / 2], 0, 127);
			SV_SetLineTexture(line->sidenum[0], 0, switchlist[i ^ 1]);

		    if (useAgain)
			{
				P_StartButton(line, SWTCH_TOP, switchlist[i], BUTTONTIME);
			}
		    return;
		}
		else if (switchlist[i] == texMid)
	    {
			SV_SectorStartSound(line->frontsector,
				switch_sound[i / 2], 0, 127);
			SV_SetLineTexture(line->sidenum[0], 1, switchlist[i ^ 1]);

			if (useAgain)
			{
				P_StartButton(line, SWTCH_MIDDLE, switchlist[i], BUTTONTIME);
			}
			return;
	    }
	    else if (switchlist[i] == texBot)
		{
			SV_SectorStartSound(line->frontsector,
				switch_sound[i / 2], 0, 127);
			SV_SetLineTexture(line->sidenum[0], 2, switchlist[i ^ 1]);

		    if (useAgain)
			{
				P_StartButton(line, SWTCH_BOTTOM, switchlist[i], BUTTONTIME);
			}
		    return;
		}
    }
}

//==========================================================================
//
//	P_UpdateButtons
//
//==========================================================================

void P_UpdateButtons(void)
{
	int 	i;

    //	DO BUTTONS
    for (i = 0; i < MAXBUTTONS; i++)
	{
		if (buttonlist[i].btimer)
		{
		    buttonlist[i].btimer--;
		    if (!buttonlist[i].btimer)
		    {
				switch (buttonlist[i].where)
				{
				 case SWTCH_TOP:
					SV_SetLineTexture(buttonlist[i].line->sidenum[0], 0, buttonlist[i].btexture);
				    break;
		    
				 case SWTCH_MIDDLE:
					SV_SetLineTexture(buttonlist[i].line->sidenum[0], 1, buttonlist[i].btexture);
				    break;
		    
				 case SWTCH_BOTTOM:
					SV_SetLineTexture(buttonlist[i].line->sidenum[0], 2, buttonlist[i].btexture);
				    break;
				}
				memset(&buttonlist[i], 0, sizeof(button_t));
		    }
		}
	}
}

//**************************************************************************
//
//	Terrain types
//
//**************************************************************************

//==========================================================================
//
// P_InitTerrainTypes
//
//==========================================================================

void P_InitTerrainTypes(void)
{
	SC_Open("TERRAINS");
	terrainTypes = (terrainType_t*)Z_Malloc(1);
	numTerrainTypes = 0;
	while (SC_GetString())
	{
		int		pic;

		pic = R_CheckFlatNumForName(sc_String);
		if (pic == -1)
		{
			pic = R_CheckTextureNumForName(sc_String);
		}
		SC_MustGetNumber();
		if (pic != -1)
		{
			numTerrainTypes++;
			Z_Resize((void**)&terrainTypes, numTerrainTypes * sizeof(terrainType_t));
			terrainTypes[numTerrainTypes - 1].pic = pic;
			terrainTypes[numTerrainTypes - 1].type = sc_Number;
		}
	}
	SC_Close();
}

//==========================================================================
//
//	SV_TerrainType
//
//==========================================================================

int SV_TerrainType(int pic)
{
	for (int i = 0; i < numTerrainTypes; i++)
	{
		if (terrainTypes[i].pic == pic)
		{
			return terrainTypes[i].type;
		}
	}
	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/10/09 17:20:42  dj_jl
//	Fixed switch sounds
//
//	Revision 1.4  2001/08/29 17:55:42  dj_jl
//	Added sound channels
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
