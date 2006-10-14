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
	vuint32	colour;		// for coloured lights
};

// Client side Map Object definition.
struct clmobjbase_t
{
	VClass*		Class;
	VState*		State;
	TVec		origin;	// position
	TAVec		angles;	// orientation
	int			spritetype;
	int			translucency;
	int			translation;
	int			effects;
};

class VRootWindow;

class VClientGameBase : public VObject
{
	DECLARE_CLASS(VClientGameBase, VObject, 0)

	enum
	{
		CF_LocalServer		= 0x01,
		// used to accelerate or skip a stage
		CF_SkipIntermission	= 0x02,
		CF_Paused			= 0x04,
	};
	vuint32				ClientFlags;

	VBasePlayer*		cl;
	level_t*			level;
	VLevel*				GLevel;

	scores_t*			scores;
	im_t*				im;

	VRootWindow*		GRoot;

	int					sb_height;

	int					maxclients;
	int					deathmatch;

	VStr				serverinfo;

	int					intermission;

	int					prev_palette;

	vuint32				prev_cshifts[NUM_CSHIFTS];	// powerups and content types

	float				time;
	float				oldtime;

	void eventRootWindowCreated()
	{
		P_PASS_SELF;
		EV_RET_VOID("RootWindowCreated");
	}
	void eventConnected()
	{
		P_PASS_SELF;
		EV_RET_VOID("Connected");
	}
	void eventDisconnected()
	{
		P_PASS_SELF;
		EV_RET_VOID("Disconnected");
	}
	void eventDemoPlaybackStarted()
	{
		P_PASS_SELF;
		EV_RET_VOID("DemoPlaybackStarted");
	}
	void eventDemoPlaybackStopped()
	{
		P_PASS_SELF;
		EV_RET_VOID("DemoPlaybackStopped");
	}
	void eventOnHostEndGame()
	{
		P_PASS_SELF;
		EV_RET_VOID("OnHostEndGame");
	}
	void eventOnHostError()
	{
		P_PASS_SELF;
		EV_RET_VOID("OnHostError");
	}
	void eventStatusBarStartMap()
	{
		P_PASS_SELF;
		EV_RET_VOID("StatusBarStartMap");
	}
	void eventStatusBarDrawer(int sb_type)
	{
		P_PASS_SELF;
		P_PASS_INT(sb_type);
		EV_RET_VOID("StatusBarDrawer");
	}
	void eventStatusBarUpdateWidgets()
	{
		P_PASS_SELF;
		EV_RET_VOID("StatusBarUpdateWidgets");
	}
	void eventIintermissionStart()
	{
		P_PASS_SELF;
		EV_RET_VOID("IintermissionStart");
	}
	void eventStartFinale(VName FinaleType)
	{
		P_PASS_SELF;
		P_PASS_NAME(FinaleType);
		EV_RET_VOID("StartFinale");
	}
	bool eventFinaleResponder(event_t* event)
	{
		P_PASS_SELF;
		P_PASS_PTR(event);
		EV_RET_BOOL("FinaleResponder");
	}
	void eventDeactivateMenu()
	{
		P_PASS_SELF;
		EV_RET_VOID("DeactivateMenu");
	}
	bool eventMenuResponder(event_t* event)
	{
		P_PASS_SELF;
		P_PASS_PTR(event);
		EV_RET_BOOL("MenuResponder");
	}
	bool eventMenuActive()
	{
		P_PASS_SELF;
		EV_RET_BOOL("MenuActive");
	}
	void eventSetMenu(const VStr& Name)
	{
		P_PASS_SELF;
		P_PASS_STR(Name);
		EV_RET_VOID("SetMenu");
	}
	void eventMessageBoxDrawer()
	{
		P_PASS_SELF;
		EV_RET_VOID("MessageBoxDrawer");
	}
	bool eventMessageBoxResponder(event_t* event)
	{
		P_PASS_SELF;
		P_PASS_PTR(event);
		EV_RET_BOOL("MessageBoxResponder");
	}
	bool eventMessageBoxActive()
	{
		P_PASS_SELF;
		EV_RET_BOOL("MessageBoxActive");
	}
	void eventDrawViewBorder(int x, int y, int w, int h)
	{
		P_PASS_SELF;
		P_PASS_INT(x);
		P_PASS_INT(y);
		P_PASS_INT(w);
		P_PASS_INT(h);
		EV_RET_VOID("DrawViewBorder");
	}
	bool eventParseServerCommand(int cmd_type)
	{
		P_PASS_SELF;
		P_PASS_INT(cmd_type);
		EV_RET_BOOL("ParseServerCommand");
	}
	void eventUpdateParticle(particle_t* p, float DeltaTime)
	{
		P_PASS_SELF;
		P_PASS_PTR(p);
		P_PASS_FLOAT(DeltaTime);
		EV_RET_VOID("UpdateParticle");
	}
	void eventUpdateMobj(VEntity* mobj, int key, float DeltaTime)
	{
		P_PASS_SELF;
		P_PASS_REF(mobj);
		P_PASS_INT(key);
		P_PASS_FLOAT(DeltaTime);
		EV_RET_VOID("UpdateMobj");
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

dlight_t *CL_AllocDlight(int);
void CL_DecayLights();

void CL_KeepaliveMessage();

bool CL_TraceLine(const TVec&, const TVec&);

particle_t *R_NewParticle();
void R_AddStaticLight(const TVec&, float, vuint32);

void R_InstallSprite(const char*, int);
void R_DrawModelFrame(const TVec&, float, VModel*, int, const char*);

VModel* Mod_FindName(const char*);
void R_PositionWeaponModel(VEntity*, VModel*, int);

void SCR_SetVirtualScreen(int, int);

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
