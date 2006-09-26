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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define PROTOCOL_VERSION	10

enum
{
	svc_bad,
	svc_nop,
	svc_disconnect,
	svc_server_info,

	svc_print,
	svc_centre_print,
	svc_stringcmd,
	svc_signonnum,

	svc_set_angles,
	svc_centre_look,
	svc_view_data,
	svc_stats_long,
	svc_stats_short,
	svc_stats_byte,

	svc_time,
	svc_pause,

	svc_spawn_baseline,
	svc_update_mobj,

	svc_side_top,
	svc_side_mid,
	svc_side_bot,
	svc_side_ofs,

	svc_sec_floor,
	svc_sec_ceil,
	svc_sec_update,

	svc_start_sound,
	svc_stop_sound,
	svc_start_seq,
	svc_stop_seq,

	svc_poly_spawn,
	svc_poly_translate,
	svc_poly_update,
	svc_force_lightning,

	svc_intermission,
	svc_skip_intermission,
	svc_finale,

	svc_sec_floor_plane,
	svc_sec_ceil_plane,

	svc_serverinfo,
	svc_userinfo,
	svc_setinfo,

	svc_sprites,
	svc_model,
	svc_skin,

	svc_line_transluc,
	svc_sec_transluc,
	svc_extra_floor,
	svc_swap_planes,

	svc_static_light,
	svc_static_light_rgb,
	svc_sec_light_colour,

	svc_change_sky,
	svc_change_music,
	svc_set_floor_light_sec,
	svc_set_ceil_light_sec,

	svc_set_heightsec,

	svc_class_name,
	svc_stats_string,

	USER_PROTOCOL_START = 128
};

enum
{
	clc_bad,
	clc_nop,
	clc_disconnect,
	clc_move,
	clc_stringcmd,
	clc_player_info,
};

#define MOB_X			0x0001
#define MOB_Y			0x0002
#define MOB_Z			0x0004
#define MOB_ANGLE		0x0008
#define MOB_STATE		0x0010
#define MOB_EFFECTS		0x0020
#define MOB_BIG_NUM		0x0040
#define MOB_MORE_BITS	0x0080
#define MOB_CLASS		0x0100
#define MOB_BIG_CLASS	0x0200
#define MOB_TRANSL		0x0400
#define MOB_TRANSLUC	0x0800
#define MOB_ANGLEP		0x1000
#define MOB_ANGLER		0x2000
#define MOB_BIG_STATE	0x4000
#define MOB_MORE_BITS2	0x8000
#define MOB_WEAPON		0x010000
#define MOB_SPRITE		0x020000
#define MOB_FULL_BRIGHT	0x040000
#define MOB_MODEL		0x080000
#define MOB_SKIN_NUM	0x100000
#define MOB_SKIN_IDX	0x200000

#define SUB_FLOOR		0x0001
#define SUB_CEIL		0x0002
#define SUB_LIGHT		0x0004
#define SUB_FLOOR_X		0x0008
#define SUB_FLOOR_Y		0x0010
#define SUB_CEIL_X		0x0020
#define SUB_CEIL_Y		0x0040
#define SUB_BIG_NUM		0x0080

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------
