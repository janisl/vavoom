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
REGISTER_NAME(ACS)
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
REGISTER_NAME(h_start)
REGISTER_NAME(h_end)
REGISTER_NAME(hh_start)
REGISTER_NAME(hh_end)
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
REGISTER_NAME(transp10)
REGISTER_NAME(transp20)
REGISTER_NAME(transp30)
REGISTER_NAME(transp40)
REGISTER_NAME(transp50)
REGISTER_NAME(consmap)
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

// Closing -----------------------------------------------------------------

#ifdef REGISTERING_ENUM
		NUM_HARDCODED_NAMES
	};
	#undef REGISTER_NAME
	#undef REGISTERING_ENUM
#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.12  2006/03/02 23:24:35  dj_jl
//	Wad lump names stored as names.
//
//	Revision 1.11  2006/02/25 17:12:38  dj_jl
//	Added missing implementation of the player class.
//	
//	Revision 1.10  2006/02/20 22:52:56  dj_jl
//	Changed client state to a class.
//	
//	Revision 1.9  2006/02/09 22:35:54  dj_jl
//	Moved all client game code to classes.
//	
//	Revision 1.8  2005/12/27 22:24:00  dj_jl
//	Created level info class, moved action special handling to it.
//	
//	Revision 1.7  2005/11/22 19:10:38  dj_jl
//	Cleaned up a bit.
//	
//	Revision 1.6  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//	
//	Revision 1.5  2004/08/21 17:22:15  dj_jl
//	Changed rendering driver declaration.
//	
//	Revision 1.4  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.3  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/18 18:57:11  dj_jl
//	Added global name subsystem
//	
//**************************************************************************
