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
//**	DOOM selection menu, options, episode etc.
//**	Sliders and icons. Kinda widget stuff.
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_ShutdownServer(boolean crash);
void CL_Disconnect(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean	slistSorted;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char			m_return_reason[32];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//**************************************************************************
//
//	Console functions
//
//**************************************************************************

COMMAND(menu_main)
{
	clpr.Exec("menu_main");
}


//==========================================================================
//
//	HelpMenu
//
//==========================================================================

COMMAND(HelpMenu)
{
    clpr.Exec("menu_help");
}

//==========================================================================
//
//  SaveMenu
//
//==========================================================================

COMMAND(SaveMenu)
{
    clpr.Exec("menu_save");
}

//==========================================================================
//
//  LoadMenu
//
//==========================================================================

COMMAND(LoadMenu)
{
    clpr.Exec("menu_load");
}

//==========================================================================
//
//	OptionsMenu
//
//==========================================================================

COMMAND(OptionsMenu)
{
    clpr.Exec("menu_options");
}

//==========================================================================
//
//	ResolutionMenu
//
//==========================================================================

COMMAND(ResolutionMenu)
{
	clpr.Exec("menu_resolution");
}

//==========================================================================
//
//  QuitConfirm
//
//==========================================================================

COMMAND(QuitConfirm)
{
	S_StartSoundName("Chat");
    clpr.Exec("menu_quit");
}

//**************************************************************************
//
//	General routines
//
//**************************************************************************

//==========================================================================
//
//	MN_Init
//
//==========================================================================

void MN_Init(void)
{
#ifdef SERVER
	clpr.SetGlobal("local_server", 1);
#else
	clpr.SetGlobal("local_server", 0);
#endif
}

//==========================================================================
//
//  MN_ActivateMenu
//
//==========================================================================

void MN_ActivateMenu(void)
{
    // intro might call this repeatedly
    if (!MN_Active())
	{
		clpr.Exec("menu_main");
	}
}

//==========================================================================
//
//	MN_DeactivateMenu
//
//==========================================================================

void MN_DeactivateMenu(void)
{
	clpr.Exec("MN_DeactivateMenu");
}

//==========================================================================
//
//  MN_Responder
//
//==========================================================================

boolean MN_Responder(event_t* event)
{
    // Pop-up menu?
    if (!MN_Active() && event->type == ev_keydown && !C_Active() &&
		(cls.state != ca_connected || cls.demoplayback) &&
		event->data1 != '`' && (event->data1 < K_F1 || event->data1 > K_F12))
	{
		MN_ActivateMenu();
		return true;
	}

	return clpr.Exec("MN_Responder", (int)event);
}

//==========================================================================
//
//	MN_Drawer
//
//==========================================================================

void MN_Drawer(void)
{
	clpr.SetGlobal("frametime", PassFloat(host_frametime));
	clpr.Exec("MN_Drawer");
}

//==========================================================================
//
//	Server list menu
//
//==========================================================================

void StartSearch(void)
{
	slistSilent = true;
	slistLocal = false;
	NET_Slist();
}

struct slist_t
{
	boolean		inProgress;
	int			count;
	hostcache_t	cache[HOSTCACHESIZE];
	char		return_reason[32];
};

void GetSlist(slist_t *slist)
{
	int		i, j;

	if (!slistSorted)
	{
		if (hostCacheCount > 1)
		{
			hostcache_t temp;
			for (i = 0; i < hostCacheCount; i++)
				for (j = i + 1; j < hostCacheCount; j++)
					if (strcmp(hostcache[j].name, hostcache[i].name) < 0)
					{
						memcpy(&temp, &hostcache[j], sizeof(hostcache_t));
						memcpy(&hostcache[j], &hostcache[i], sizeof(hostcache_t));
						memcpy(&hostcache[i], &temp, sizeof(hostcache_t));
					}
		}
		slistSorted = true;
		memset(m_return_reason, 0, sizeof(m_return_reason));
	}

	slist->inProgress = slistInProgress;
	slist->count = hostCacheCount;
	memcpy(slist->cache, hostcache, sizeof(hostcache));
	strcpy(slist->return_reason, m_return_reason);
}

boolean MN_Active(void)
{
	return clpr.Exec("MN_Active");
}

//==========================================================================
//
//	MB_Active
//
//==========================================================================

boolean MB_Active(void)
{
	return clpr.Exec("MB_Active");
}

//==========================================================================
//
//	MB_Drawer
//
//==========================================================================

void MB_Drawer(void)
{
	clpr.Exec("MB_Drawer");
}

//==========================================================================
//
//  MB_Responder
//
//==========================================================================

boolean MB_Responder(event_t *event)
{
	return clpr.Exec("MB_Responder", (int)event);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/08/30 17:39:51  dj_jl
//	Moved view border and message box to progs
//
//	Revision 1.4  2001/08/07 16:49:26  dj_jl
//	Added C_Active
//	
//	Revision 1.3  2001/07/31 17:09:34  dj_jl
//	Removed seting shareware and ExtendedWAD in progs
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
