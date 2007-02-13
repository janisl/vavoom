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
REGISTER_NAME(GameInfo)
REGISTER_NAME(Entity)
REGISTER_NAME(BasePlayer)
REGISTER_NAME(ViewEntity)
REGISTER_NAME(Acs)
REGISTER_NAME(Button)
REGISTER_NAME(Level)
REGISTER_NAME(GC)
REGISTER_NAME(Window)
REGISTER_NAME(ModalWindow)
REGISTER_NAME(RootWindow)
REGISTER_NAME(ClientGameBase)
REGISTER_NAME(ClientState)

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
REGISTER_NAME(playpal)
REGISTER_NAME(rgbtable)
REGISTER_NAME(colormap)
REGISTER_NAME(fogmap)
REGISTER_NAME(translat)
REGISTER_NAME(sndcurve)
REGISTER_NAME(sndinfo)
REGISTER_NAME(sndseq)
REGISTER_NAME(mapinfo)
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
REGISTER_NAME(clprogs)
REGISTER_NAME(svprogs)
REGISTER_NAME(language)

// Closing -----------------------------------------------------------------

#ifdef REGISTERING_ENUM
		NUM_HARDCODED_NAMES
	};
	#undef REGISTER_NAME
	#undef REGISTERING_ENUM
#endif
