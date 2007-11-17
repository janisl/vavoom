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
//**
//**	Header file registering global hardcoded Vavoom names.
//**
//**************************************************************************

// Macros ------------------------------------------------------------------

// Define a message as an enumeration.
#ifndef REGISTER_NAME
	#define REGISTER_NAME(name)	NAME_##name,
	#define REGISTERING_ENUM
	enum EName {
#endif

// Hardcoded names ---------------------------------------------------------

// Special zero value, meaning no name.
REGISTER_NAME(None)

// Log messages.
REGISTER_NAME(Log)
REGISTER_NAME(Init)
REGISTER_NAME(Dev)
REGISTER_NAME(DevNet)

//	Native class names.
REGISTER_NAME(Object)
REGISTER_NAME(Thinker)
REGISTER_NAME(LevelInfo)
REGISTER_NAME(WorldInfo)
REGISTER_NAME(GameInfo)
REGISTER_NAME(Entity)
REGISTER_NAME(BasePlayer)
REGISTER_NAME(PlayerReplicationInfo)
REGISTER_NAME(ViewEntity)
REGISTER_NAME(Acs)
REGISTER_NAME(Button)
REGISTER_NAME(Level)
REGISTER_NAME(Widget)
REGISTER_NAME(RootWidget)
REGISTER_NAME(ActorDisplayWindow)
REGISTER_NAME(ClientGameBase)
REGISTER_NAME(ClientState)

//	Special struct names
REGISTER_NAME(TVec)
REGISTER_NAME(TAVec)

//	Thinker events
REGISTER_NAME(Tick)
REGISTER_NAME(ClientTick)

//	Entity events
REGISTER_NAME(OnMapSpawn)
REGISTER_NAME(BeginPlay)
REGISTER_NAME(Destroyed)
REGISTER_NAME(Touch)
REGISTER_NAME(BlockedByLine)
REGISTER_NAME(ApplyFriction)
REGISTER_NAME(PushLine)
REGISTER_NAME(HandleFloorclip)
REGISTER_NAME(CrossSpecialLine)
REGISTER_NAME(SectorChanged)
REGISTER_NAME(RoughCheckThing)
REGISTER_NAME(GiveInventory)
REGISTER_NAME(TakeInventory)
REGISTER_NAME(CheckInventory)
REGISTER_NAME(GetSigilPieces)
REGISTER_NAME(MoveThing)
REGISTER_NAME(GetStateTime)

//	LevelInfo events
REGISTER_NAME(SpawnSpecials)
REGISTER_NAME(UpdateSpecials)
REGISTER_NAME(AfterUnarchiveThinkers)
REGISTER_NAME(FindLine)
REGISTER_NAME(PolyThrustMobj)
REGISTER_NAME(TagBusy)
REGISTER_NAME(PolyBusy)
REGISTER_NAME(ThingCount)
REGISTER_NAME(FindMobjFromTID)
REGISTER_NAME(ExecuteActionSpecial)
REGISTER_NAME(EV_ThingProjectile)
REGISTER_NAME(StartPlaneWatcher)
REGISTER_NAME(SpawnMapThing)
REGISTER_NAME(UpdateParticle)

//	BasePlayer events
REGISTER_NAME(PutClientIntoServer)
REGISTER_NAME(SpawnClient)
REGISTER_NAME(NetGameReborn)
REGISTER_NAME(DisconnectClient)
REGISTER_NAME(UserinfoChanged)
REGISTER_NAME(PlayerExitMap)
REGISTER_NAME(PlayerTick)
REGISTER_NAME(SetViewPos)
REGISTER_NAME(Cheat_God)
REGISTER_NAME(Cheat_NoClip)
REGISTER_NAME(Cheat_Gimme)
REGISTER_NAME(Cheat_KillAll)
REGISTER_NAME(Cheat_Morph)
REGISTER_NAME(Cheat_NoWeapons)
REGISTER_NAME(Cheat_Class)
REGISTER_NAME(ClientStartSound)
REGISTER_NAME(ClientStopSound)
REGISTER_NAME(ClientStartSequence)
REGISTER_NAME(ClientAddSequenceChoice)
REGISTER_NAME(ClientStopSequence)
REGISTER_NAME(ClientForceLightning)
REGISTER_NAME(ClientPrint)
REGISTER_NAME(ClientCentrePrint)
REGISTER_NAME(ClientSetAngles)
REGISTER_NAME(ClientIntermission)
REGISTER_NAME(ClientPause)
REGISTER_NAME(ClientSkipIntermission)
REGISTER_NAME(ClientFinale)
REGISTER_NAME(ClientChangeMusic)
REGISTER_NAME(ClientSetServerInfo)
REGISTER_NAME(ServerImpulse)
REGISTER_NAME(ServerSetUserInfo)

//	ClientGameBase events
REGISTER_NAME(RootWindowCreated)
REGISTER_NAME(Connected)
REGISTER_NAME(Disconnected)
REGISTER_NAME(DemoPlaybackStarted)
REGISTER_NAME(DemoPlaybackStopped)
REGISTER_NAME(OnHostEndGame)
REGISTER_NAME(OnHostError)
REGISTER_NAME(StatusBarStartMap)
REGISTER_NAME(StatusBarDrawer)
REGISTER_NAME(StatusBarUpdateWidgets)
REGISTER_NAME(IintermissionStart)
REGISTER_NAME(StartFinale)
REGISTER_NAME(FinaleResponder)
REGISTER_NAME(DeactivateMenu)
REGISTER_NAME(MenuResponder)
REGISTER_NAME(MenuActive)
REGISTER_NAME(SetMenu)
REGISTER_NAME(MessageBoxDrawer)
REGISTER_NAME(MessageBoxResponder)
REGISTER_NAME(MessageBoxActive)
REGISTER_NAME(DrawViewBorder)

//	Widget events
REGISTER_NAME(OnCreate)
REGISTER_NAME(OnDestroy)
REGISTER_NAME(OnChildAdded)
REGISTER_NAME(OnChildRemoved)
REGISTER_NAME(OnConfigurationChanged)
REGISTER_NAME(OnVisibilityChanged)
REGISTER_NAME(OnEnableChanged)
REGISTER_NAME(OnFocusableChanged)
REGISTER_NAME(OnFocusReceived)
REGISTER_NAME(OnFocusLost)
REGISTER_NAME(OnDraw)
REGISTER_NAME(OnPostDraw)
REGISTER_NAME(OnKeyDown)
REGISTER_NAME(OnKeyUp)
REGISTER_NAME(OnMouseMove)
REGISTER_NAME(OnMouseEnter)
REGISTER_NAME(OnMouseLeave)
REGISTER_NAME(OnMouseDown)
REGISTER_NAME(OnMouseUp)
REGISTER_NAME(OnMouseClick)
REGISTER_NAME(OnMMouseClick)
REGISTER_NAME(OnRMouseClick)

//	Lump names
REGISTER_NAME(s_start)
REGISTER_NAME(s_end)
REGISTER_NAME(ss_start)
REGISTER_NAME(ss_end)
REGISTER_NAME(f_start)
REGISTER_NAME(f_end)
REGISTER_NAME(ff_start)
REGISTER_NAME(ff_end)
REGISTER_NAME(c_start)
REGISTER_NAME(c_end)
REGISTER_NAME(cc_start)
REGISTER_NAME(cc_end)
REGISTER_NAME(a_start)
REGISTER_NAME(a_end)
REGISTER_NAME(aa_start)
REGISTER_NAME(aa_end)
REGISTER_NAME(tx_start)
REGISTER_NAME(tx_end)
REGISTER_NAME(v_start)
REGISTER_NAME(v_end)
REGISTER_NAME(vv_start)
REGISTER_NAME(vv_end)
REGISTER_NAME(hi_start)
REGISTER_NAME(hi_end)
REGISTER_NAME(pr_start)
REGISTER_NAME(pr_end)
REGISTER_NAME(pnames)
REGISTER_NAME(texture1)
REGISTER_NAME(texture2)
REGISTER_NAME(f_sky)
REGISTER_NAME(f_sky001)
REGISTER_NAME(f_sky1)
REGISTER_NAME(autopage)
REGISTER_NAME(animated)
REGISTER_NAME(switches)
REGISTER_NAME(animdefs)
REGISTER_NAME(hirestex)
REGISTER_NAME(skyboxes)
REGISTER_NAME(loadacs)
REGISTER_NAME(playpal)
REGISTER_NAME(colormap)
REGISTER_NAME(fogmap)
REGISTER_NAME(x11r6rgb)
REGISTER_NAME(translat)
REGISTER_NAME(sndcurve)
REGISTER_NAME(sndinfo)
REGISTER_NAME(sndseq)
REGISTER_NAME(mapinfo)
REGISTER_NAME(terrain)
REGISTER_NAME(lockdefs)
REGISTER_NAME(behavior)
REGISTER_NAME(gl_level)
REGISTER_NAME(gl_pvs)
REGISTER_NAME(endoom)
REGISTER_NAME(endtext)
REGISTER_NAME(endstrf)
REGISTER_NAME(teleicon)
REGISTER_NAME(saveicon)
REGISTER_NAME(loadicon)
REGISTER_NAME(ammnum0)
REGISTER_NAME(stcfn033)
REGISTER_NAME(textcolo)
REGISTER_NAME(clprogs)
REGISTER_NAME(svprogs)
REGISTER_NAME(language)

//	Standard font names
REGISTER_NAME(smallfont)
REGISTER_NAME(smallfont2)
REGISTER_NAME(bigfont)
REGISTER_NAME(consolefont)

// Closing -----------------------------------------------------------------

#ifdef REGISTERING_ENUM
		NUM_HARDCODED_NAMES
	};
	#undef REGISTER_NAME
	#undef REGISTERING_ENUM
#endif
