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

class VRootWidget;
struct VModel;

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct dlight_t
{
	TVec		origin;		// origin of the light
	float		radius;		// radius - how far light goes
	float		die;		// stop lighting after this time
	float		decay;		// drop this each second
	float		minlight;	// don't add when contributing less
	VThinker*	Owner;		// used to identify owner to reuse the same light
	vuint32		colour;		// for coloured lights
};

struct im_t
{
	VName		LeaveMap;
	vint32		LeaveCluster;
	VStr		LeaveName;
	VName		LeaveTitlePatch;
	VName		ExitPic;

	VName		EnterMap;
	vint32		EnterCluster;
	VStr		EnterName;
	VName		EnterTitlePatch;
	VName		EnterPic;

	VName		InterMusic;

	VStr		Text;
	VName		TextFlat;
	VName		TextPic;
	VName		TextMusic;
	vint32		TextCDTrack;
	vint32		TextCDId;

	enum
	{
		IMF_TextIsLump		= 0x01,
	};
	vint32		IMFlags;
};

class VClientGameBase : public VObject
{
	DECLARE_CLASS(VClientGameBase, VObject, 0)

	enum
	{
		CF_LocalServer		= 0x01,
		// used to accelerate or skip a stage
		CF_SkipIntermission	= 0x02,
	};
	vuint32				ClientFlags;

	VGameInfo*			Game;
	VBasePlayer*		cl;
	VLevel*				GLevel;

	im_t				im;

	VRootWidget*		GRoot;

	int					sb_height;

	int					maxclients;
	int					deathmatch;

	VStr				serverinfo;

	int					intermission;

	VClientGameBase()
	: serverinfo(E_NoInit)
	{}

	void eventRootWindowCreated()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_RootWindowCreated);
	}
	void eventConnected()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_Connected);
	}
	void eventDisconnected()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_Disconnected);
	}
	void eventDemoPlaybackStarted()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_DemoPlaybackStarted);
	}
	void eventDemoPlaybackStopped()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_DemoPlaybackStopped);
	}
	void eventOnHostEndGame()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_OnHostEndGame);
	}
	void eventOnHostError()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_OnHostError);
	}
	void eventStatusBarStartMap()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_StatusBarStartMap);
	}
	void eventStatusBarDrawer(int sb_type)
	{
		P_PASS_SELF;
		P_PASS_INT(sb_type);
		EV_RET_VOID(NAME_StatusBarDrawer);
	}
	void eventStatusBarUpdateWidgets(float DeltaTime)
	{
		P_PASS_SELF;
		P_PASS_FLOAT(DeltaTime);
		EV_RET_VOID(NAME_StatusBarUpdateWidgets);
	}
	void eventIintermissionStart()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_IintermissionStart);
	}
	void eventStartFinale(VName FinaleType)
	{
		P_PASS_SELF;
		P_PASS_NAME(FinaleType);
		EV_RET_VOID(NAME_StartFinale);
	}
	bool eventFinaleResponder(event_t* event)
	{
		P_PASS_SELF;
		P_PASS_PTR(event);
		EV_RET_BOOL(NAME_FinaleResponder);
	}
	void eventDeactivateMenu()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_DeactivateMenu);
	}
	bool eventMenuResponder(event_t* event)
	{
		P_PASS_SELF;
		P_PASS_PTR(event);
		EV_RET_BOOL(NAME_MenuResponder);
	}
	bool eventMenuActive()
	{
		P_PASS_SELF;
		EV_RET_BOOL(NAME_MenuActive);
	}
	void eventSetMenu(const VStr& Name)
	{
		P_PASS_SELF;
		P_PASS_STR(Name);
		EV_RET_VOID(NAME_SetMenu);
	}
	void eventMessageBoxDrawer()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_MessageBoxDrawer);
	}
	bool eventMessageBoxResponder(event_t* event)
	{
		P_PASS_SELF;
		P_PASS_PTR(event);
		EV_RET_BOOL(NAME_MessageBoxResponder);
	}
	bool eventMessageBoxActive()
	{
		P_PASS_SELF;
		EV_RET_BOOL(NAME_MessageBoxActive);
	}
	void eventDrawViewBorder(int x, int y, int w, int h)
	{
		P_PASS_SELF;
		P_PASS_INT(x);
		P_PASS_INT(y);
		P_PASS_INT(w);
		P_PASS_INT(h);
		EV_RET_VOID(NAME_DrawViewBorder);
	}
	void eventAddNotifyMessage(const VStr& Str)
	{
		P_PASS_SELF;
		P_PASS_STR(Str);
		EV_RET_VOID(NAME_AddNotifyMessage);
	}
	void eventAddCentreMessage(const VStr& Msg)
	{
		P_PASS_SELF;
		P_PASS_STR(Msg);
		EV_RET_VOID(NAME_AddCentreMessage);
	}
	void eventAddHudMessage(const VStr& Message, VName Font, int Type, int Id,
		int Colour, const VStr& ColourName, float x, float y, int HudWidth,
		int HudHeight, float HoldTime, float Time1, float Time2)
	{
		P_PASS_SELF;
		P_PASS_STR(Message);
		P_PASS_NAME(Font);
		P_PASS_INT(Type);
		P_PASS_INT(Id);
		P_PASS_INT(Colour);
		P_PASS_STR(ColourName);
		P_PASS_FLOAT(x);
		P_PASS_FLOAT(y);
		P_PASS_INT(HudWidth);
		P_PASS_INT(HudHeight);
		P_PASS_FLOAT(HoldTime);
		P_PASS_FLOAT(Time1);
		P_PASS_FLOAT(Time2);
		EV_RET_VOID(NAME_AddHudMessage);
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void CL_DecayLights();

void CL_KeepaliveMessage();
void CL_ParseServerInfo(class VMessageIn& msg);
void CL_ReadFromServerInfo();
void CL_StopRecording();

void R_DrawModelFrame(const TVec&, float, VModel*, int, int, const char*, int,
	int, int, float);

VModel* Mod_FindName(const VStr&);

void SCR_SetVirtualScreen(int, int);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern VClientGameBase*		GClGame;

extern int					VirtualWidth;
extern int					VirtualHeight;

extern float				fScaleX;
extern float				fScaleY;
extern float				fScaleXI;
extern float				fScaleYI;

extern bool					UserInfoSent;

#endif
