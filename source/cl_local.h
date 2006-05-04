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

#ifndef __CL_LOCAL_H__
#define __CL_LOCAL_H__

// HEADER FILES ------------------------------------------------------------

#include "iline.h"		//	Input line widget
#include "drawer.h"

// MACROS ------------------------------------------------------------------

#define	MAX_DLIGHTS		32

// TYPES -------------------------------------------------------------------

struct dlight_t
{
	TVec	origin;		// origin of the light
	float	radius;		// radius - how far light goes
	float	die;		// stop lighting after this time
	float	decay;		// drop this each second
	float	minlight;	// don't add when contributing less
	int		key;		// used to identify owner to reuse the same light
	dword	color;		// for colored lights
};

// Client side Map Object definition.
struct clmobjbase_t
{
	TVec		origin;	// position
	TAVec		angles;	// orientation

	int			spritetype;
    int			sprite;	// used to find patch_t and flip value
    int			frame;	// might be ORed with FF_FULLBRIGHT

	int			model_index;
	int			alias_frame;

    int			translucency;
    int			translation;

	int			effects;
};

class VRootWindow;

class VClientGameBase : public VObject
{
#ifdef ZONE_DEBUG_NEW
#undef new
#endif
	DECLARE_CLASS(VClientGameBase, VObject, 0)
#ifdef ZONE_DEBUG_NEW
#define new ZONE_DEBUG_NEW
#endif

	enum
	{
		CF_LocalServer		= 0x01,
		// used to accelerate or skip a stage
		CF_SkipIntermission	= 0x02,
	};
	vuint32				ClientFlags;

	VClientState*		cl;
	level_t*			level;
	VLevel*				GLevel;

	scores_t*			scores;
	im_t*				im;

	VRootWindow*		GRoot;

	int					sb_height;

	void eventRootWindowCreated()
	{
		clpr.Exec(GetVFunction("RootWindowCreated"), (int)this);
	}
	void eventConnected()
	{
		clpr.Exec(GetVFunction("Connected"), (int)this);
	}
	void eventDisconnected()
	{
		clpr.Exec(GetVFunction("Disconnected"), (int)this);
	}
	void eventDemoPlaybackStarted()
	{
		clpr.Exec(GetVFunction("DemoPlaybackStarted"), (int)this);
	}
	void eventDemoPlaybackStopped()
	{
		clpr.Exec(GetVFunction("DemoPlaybackStopped"), (int)this);
	}
	void eventOnHostEndGame()
	{
		clpr.Exec(GetVFunction("OnHostEndGame"), (int)this);
	}
	void eventOnHostError()
	{
		clpr.Exec(GetVFunction("OnHostError"), (int)this);
	}
	void eventStatusBarStartMap()
	{
		clpr.Exec(GetVFunction("StatusBarStartMap"), (int)this);
	}
	void eventStatusBarDrawer(int sb_type)
	{
		clpr.Exec(GetVFunction("StatusBarDrawer"), (int)this, sb_type);
	}
	void eventStatusBarUpdateWidgets()
	{
		clpr.Exec(GetVFunction("StatusBarUpdateWidgets"), (int)this);
	}
	void eventIintermissionStart()
	{
		clpr.Exec(GetVFunction("IintermissionStart"), (int)this);
	}
	void eventStartFinale()
	{
		clpr.Exec(GetVFunction("StartFinale"), (int)this);
	}
	bool eventFinaleResponder(event_t* event)
	{
		return !!clpr.Exec(GetVFunction("FinaleResponder"), (int)this, (int)event);
	}
	void eventDeactivateMenu()
	{
		clpr.Exec(GetVFunction("DeactivateMenu"), (int)this);
	}
	bool eventMenuResponder(event_t* event)
	{
		return !!clpr.Exec(GetVFunction("MenuResponder"), (int)this, (int)event);
	}
	bool eventMenuActive()
	{
		return !!clpr.Exec(GetVFunction("MenuActive"), (int)this);
	}
	void eventSetMenu(const char* Name)
	{
		clpr.Exec(GetVFunction("SetMenu"), (int)this, (int)Name);
	}
	void eventMessageBoxDrawer()
	{
		clpr.Exec(GetVFunction("MessageBoxDrawer"), (int)this);
	}
	bool eventMessageBoxResponder(event_t* event)
	{
		return !!clpr.Exec(GetVFunction("MessageBoxResponder"), (int)this, (int)event);
	}
	bool eventMessageBoxActive()
	{
		return !!clpr.Exec(GetVFunction("MessageBoxActive"), (int)this);
	}
	void eventDrawViewBorder(int x, int y, int w, int h)
	{
		clpr.Exec(GetVFunction("DrawViewBorder"), (int)this, x, y, w, h);
	}
	bool eventParseServerCommand(int cmd_type)
	{
		return !!clpr.Exec(GetVFunction("ParseServerCommand"), (int)this, cmd_type);
	}
	void eventUpdateParticle(particle_t* p, float DeltaTime)
	{
		clpr.Exec(GetVFunction("UpdateParticle"), (int)this, (int)p, PassFloat(DeltaTime));
	}
	void eventUpdateMobj(VEntity* mobj, int key, float DeltaTime)
	{
		clpr.Exec(GetVFunction("UpdateMobj"), (int)this, (int)mobj, key, PassFloat(DeltaTime));
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

dlight_t *CL_AllocDlight(int key);
void CL_DecayLights(void);

void CL_KeepaliveMessage(void);

bool CL_TraceLine(const TVec &start, const TVec &end);

particle_t *R_NewParticle(void);
void R_AddStaticLight(const TVec &origin, float radius, dword color);

void R_InstallSprite(const char *name, int index);
void R_DrawModelFrame(const TVec &origin, float angle, VModel* model,
	int frame, const char *skin);

VModel* Mod_FindName(const char* name);
void R_PositionWeaponModel(VEntity* wpent, VModel* wpmodel, int frame);

void SCR_SetVirtualScreen(int Width, int Height);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern VEntity**		cl_mobjs;
extern clmobjbase_t*	cl_mo_base;
extern VEntity*			cl_weapon_mobjs[MAXPLAYERS];

extern VClientGameBase*	GClGame;

extern int				VirtualWidth;
extern int				VirtualHeight;

extern float fScaleX;
extern float fScaleY;
extern float fScaleXI;
extern float fScaleYI;

inline subsector_t* CL_PointInSubsector(float x, float y)
{
	return GClLevel->PointInSubsector(TVec(x, y, 0));
}

extern dlight_t			cl_dlights[MAX_DLIGHTS];

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.22  2006/03/12 12:54:48  dj_jl
//	Removed use of bitfields for portability reasons.
//
//	Revision 1.21  2006/03/06 13:05:50  dj_jl
//	Thunbker list in level, client now uses entity class.
//	
//	Revision 1.20  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//	
//	Revision 1.19  2006/02/20 22:52:56  dj_jl
//	Changed client state to a class.
//	
//	Revision 1.18  2006/02/13 18:34:34  dj_jl
//	Moved all server progs global functions to classes.
//	
//	Revision 1.17  2006/02/09 22:35:54  dj_jl
//	Moved all client game code to classes.
//	
//	Revision 1.16  2006/02/05 14:11:00  dj_jl
//	Fixed conflict with Solaris.
//	
//	Revision 1.15  2005/05/03 14:56:58  dj_jl
//	Added support for specifying skin index.
//	
//	Revision 1.14  2004/08/18 18:05:46  dj_jl
//	Support for higher virtual screen resolutions.
//	
//	Revision 1.13  2002/09/07 16:31:50  dj_jl
//	Added Level class.
//	
//	Revision 1.12  2002/08/28 16:42:04  dj_jl
//	Configurable entity limit.
//	
//	Revision 1.11  2002/04/11 16:44:44  dj_jl
//	Got rid of some warnings.
//	
//	Revision 1.10  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.9  2001/12/04 18:16:28  dj_jl
//	Player models and skins handled by server
//	
//	Revision 1.8  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.7  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.6  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.5  2001/08/15 17:24:02  dj_jl
//	Improved object update on packet overflows
//	
//	Revision 1.4  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
