//----------------------------------------------------------------------------
//  EDGE Data Definition File Code (Main)
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2001  The EDGE Team.
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------

#ifndef __DDF_MAIN__
#define __DDF_MAIN__

#include "dm_defs.h"
#include "e_think.h"
#include "m_fixed.h"


#define DEBUG_DDF  0

struct image_s;


typedef struct ddf_base_s
{
  // name of this DDF entry.  For non-numbered ddf entries (lines,
  // sectors and playlists) this can be NULL, otherwise it must be
  // valid.
  char *name;

  // number of this DDF entry.  Zero means "not numbered".
  // Negative values not allowed (covert them to zero).
  int number;

  // CRC of the important parts of this DDF entry, i.e. anything that
  // has a significant effect on gameplay.  Doesn't include the entry
  // name or number.
  unsigned long crc;
}
ddf_base_t;

#define DDF_BASE_NIL  { "", 0, 0 }


// percentage type.  Ranges from 0.0 - 1.0
typedef float_t percent_t;

#define PERCENT_MAKE(val)  ((val) / 100.0)
#define PERCENT_2_FLOAT(perc)  (perc)


//-------------------------------------------------------------------------
//-----------------------  THING STATE STUFF   ----------------------------
//-------------------------------------------------------------------------

#define S_NULL 0

typedef int statenum_t;
typedef int spritenum_t;

struct mobj_s;

// State Struct
typedef struct
{
  // sprite ref
  int sprite;

  // frame ref
  short frame;
 
  // brightness
  short bright;
 
  // duration in tics
  long tics;

  // label for state, or NULL
  const char *label;

  // routine to be performed
  void (* action)(struct mobj_s * object);

  // parameter for routine, or NULL
  void *action_par;

  // next state ref.  S_NULL means "remove me".
  int nextstate;

  // jump state ref.  S_NULL means remove.
  int jumpstate;
}
state_t;

// ------------------------------------------------------------------
// -------------------------------SFX--------------------------------
// ------------------------------------------------------------------

// -KM- 1998/10/29
typedef struct
{
  int num;
  int sounds[1]; // -ACB- 1999/11/06 Zero based array is not ANSI compliant
                 // -AJA- I'm also relying on the [1] within sfxinfo_t.
}
sfx_t;

// Bastard SFX are sounds that are hardcoded into the
// code.  They should be removed if at all possible

typedef struct
{
  sfx_t *s;
  char name[8];
}
bastard_sfx_t;

extern bastard_sfx_t bastard_sfx[];

#define sfx_swtchn bastard_sfx[0].s
#define sfx_tink bastard_sfx[1].s
#define sfx_radio bastard_sfx[2].s
#define sfx_oof bastard_sfx[3].s
#define sfx_pstop bastard_sfx[4].s
#define sfx_stnmov bastard_sfx[5].s
#define sfx_pistol bastard_sfx[6].s
#define sfx_swtchx bastard_sfx[7].s
#define sfx_jpmove bastard_sfx[8].s
#define sfx_jpidle bastard_sfx[9].s
#define sfx_jprise bastard_sfx[10].s
#define sfx_jpdown bastard_sfx[11].s
#define sfx_jpflow bastard_sfx[12].s

#define sfx_None (sfx_t*) NULL

// ------------------------------------------------------------------
// ------------------- COLOURMAP & PALETTE STUFF --------------------
// ------------------------------------------------------------------

// -AJA- 1999/07/09: colmap.ddf structures.

// RGB 8:8:8
typedef unsigned int rgbcol_t;

typedef enum
{
  // don't apply gun-flash type effects (looks silly for fog)
  COLSP_NoFlash  = 0x0001
}
colourspecial_e;

typedef struct colmapcache_s
{
  // the pointer returned by Z_New.
  char *baseptr;

  // the actual pointer. This one is aligned.
  void *data;

  int validcount;
  int bpp;
  
  // colour for GL renderer
  rgbcol_t gl_colour;
}
colmapcache_t;

typedef struct colourmap_s
{
  ddf_base_t ddf;

  char lump_name[10];

  int start;
  int length;

  colourspecial_e special;

  colmapcache_t cache;
}
colourmap_t;

// ------------------------------------------------------------------
// ------------------------BENEFIT TYPES-----------------------------
// ------------------------------------------------------------------

typedef enum
{
  BENEFIT_None = 0,
  BENEFIT_Ammo,
  BENEFIT_AmmoLimit,
  BENEFIT_Weapon,
  BENEFIT_Key,
  BENEFIT_Health,
  BENEFIT_Armour,
  BENEFIT_Powerup
}
benefit_type_e;

// Ammunition types defined.
typedef enum
{
  // Only used for P_SelectNewWeapon()
  AM_DontCare = -2,
  
  // Unlimited for chainsaw / fist.
  AM_NoAmmo = -1,
  
  // Pistol / chaingun ammo.
  AM_Bullet,
  
  // Shotgun / double barreled shotgun.
  AM_Shell,
  
  // Missile launcher.
  AM_Rocket,

  // Plasma rifle, BFG.
  AM_Cell,

  // New ammo types
  AM_Pellet,
  AM_Nail,
  AM_Grenade,
  AM_Gas,

  // -AJA- Note: Savegame code relies on NUMAMMO == 8.
  NUMAMMO
}
ammotype_t;

typedef enum
{
  // weakest armour, saves 33% of damage
  ARMOUR_Green = 0,

  // better armour, saves 50% of damage
  ARMOUR_Blue,

  // good armour, saves 75% of damage.  (not in Doom)
  ARMOUR_Yellow,

  // the best armour, saves 90% of damage.  (not in Doom)
  ARMOUR_Red,
  
  // -AJA- Note: Savegame code relies on NUMARMOUR == 4.
  NUMARMOUR
}
armour_type_e;

typedef short armour_set_t;  // one bit per armour

// Power up artifacts.
//
// -MH- 1998/06/17  Jet Pack Added
// -ACB- 1998/07/15 NightVision Added

typedef enum
{
  PW_Invulnerable = 0,
  PW_Berserk,
  PW_PartInvis,
  PW_AcidSuit,
  PW_AllMap,
  PW_Infrared,

  // extra powerups (not in Doom)
  PW_Jetpack,
  PW_NightVision,
  PW_Scuba,

  PW_Unused9,
  PW_Unused10,
  PW_Unused11,

  PW_Unused12,
  PW_Unused13,
  PW_Unused14,
  PW_Unused15,

  // -AJA- Note: Savegame code relies on NUMPOWERS == 16.
  NUMPOWERS
}
power_type_e;

// -AJA- 2000/02/27: added this new structure 
typedef struct benefit_s
{
  // next in linked list
  struct benefit_s *next;

  // type of benefit (ammo, ammo-limit, weapon, key, health, armour,
  // or powerup).
  benefit_type_e type;
  
  // sub-type (specific type of ammo, weapon, key or powerup).  For
  // armour this is the class, for health it is unused.
  int subtype;

  // amount of benefit (e.g. quantity of ammo or health).  For weapons
  // and keys, this is a boolean value: 1 to give, 0 to ignore.  For
  // powerups, it is number of seconds the powerup lasts.
  float_t amount;

  // for health, armour and powerups, don't make the new value go
  // higher than this (if it is already higher, prefer not to pickup
  // the object).
  float_t limit;
}
benefit_t;

// -AJA- 2000/07/23: added this structure
typedef struct condition_check_s
{
  // next in linked list (order is unimportant)
  struct condition_check_s *next;

  // negate the condition
  boolean_t negate;

  enum
  {
    // dummy condition, used if parsing failed
    COND_NONE = 0,
      
    // object must have health
    COND_Health,

    // player must have armour (subtype is ARMOUR_* value)
    COND_Armour,

    // player must have a key (subtype is KF_* value).
    COND_Key,

    // player must have a weapon (subtype is slot number).
    COND_Weapon,

    // player must have a powerup (subtype is PW_* value).
    COND_Powerup,

    // player must have ammo (subtype is AM_* value)
    COND_Ammo,

    // player must be jumping
    COND_Jumping,

    // player must be crouching
    COND_Crouching,

    // object must be swimming (i.e. in water)
    COND_Swimming,

    // player must be attacking (holding fire down)
    COND_Attacking,

    // player must be rampaging (holding fire a long time)
    COND_Rampaging,

    // player must be using (holding space down)
    COND_Using
  }
  cond_type;

  // sub-type (specific type of ammo, weapon, key, powerup).  Not used
  // for health, jumping, crouching, etc.
  int subtype;

  // required amount of health, armour or ammo,   Not used for
  // weapon, key, powerup, jumping, crouching, etc.
  float_t amount;
}
condition_check_t;

// ------------------------------------------------------------------
// --------------------MOVING OBJECT INFORMATION---------------------
// ------------------------------------------------------------------

// improved damage support
typedef struct damage_s
{
  // nominal damage amount (required)
  float_t nominal;

  // used for DAMAGE.MAX: when this is > 0, the damage is random
  // between nominal and linear_max, where each value has equal
  // probability.
  float_t linear_max;
  
  // used for DAMAGE.ERROR: when this is > 0, the damage is the
  // nominal value +/- this error amount, with a bell-shaped
  // distribution (values near the nominal are much more likely than
  // values at the outer extreme).
  float_t error;

  // delay (in terms of tics) between damage application, e.g. 34
  // would be once every second.  Only used for slime/crush damage.
  int delay;

  // override labels for various states, if the object being damaged
  // has such a state then it is used instead of the normal ones
  // (PAIN, DEATH, OVERKILL).  Defaults to NULL.
  struct label_offset_s
  {
    const char *label;
    int offset;
  }
  pain, death, overkill;

  // this flag says that the damage is unaffected by the player's
  // armour -- and vice versa.
  boolean_t no_armour;
}
damage_t;

#define NULL_LABEL  { NULL, 0 }

#define DAMAGE_COMPUTE(var,dam)  \
    do {  \
      (var) = (dam)->nominal;  \
      \
      if ((dam)->error > 0)  \
        (var) += (dam)->error * P_RandomNegPos() / 255.0;  \
      else if ((dam)->linear_max > 0)  \
        (var) += ((dam)->linear_max - (var)) * P_Random() / 255.0;  \
      \
      if ((var) < 0) (var) = 0;  \
    } while (0)

// a bitset is a set of named bits, from `A' to `Z'.
typedef int bitset_t;

#define BITSET_EMPTY  0
#define BITSET_FULL   0x7FFFFFFF
#define BITSET_MAKE(ch)  (1 << ((ch) - 'A'))

typedef enum
{
  // dynamic lighting disabled
  DLITE_None,

  // lighting is constant (regardless of distance)
  DLITE_Constant,

  // lighting is proportional to 1 / distance
  DLITE_Linear,

  // lighting is proportional to 1 / (distance^2)
  DLITE_Quadratic
}
dlight_type_e;

// -KM- 1998/10/29 sfx_t SFXs
// -KM- 1998/11/25 Added weapons, changed invisibility + accuracy.
typedef struct mobjinfo_s
{
  ddf_base_t ddf;

  // range of states used
  int first_state;
  int last_state;
  
  int spawn_state;
  int idle_state;
  int chase_state;
  int pain_state;
  int missile_state;
  int melee_state;
  int death_state;
  int overkill_state;
  int raise_state;
  int res_state;
  int meander_state;
  int bounce_state;
  int touch_state;
  int jump_state;
  int gib_state;

  int reactiontime;
  percent_t painchance;
  float_t spawnhealth;
  float_t speed;
  float_t float_speed;
  float_t radius;
  float_t height;
  float_t step_size;
  float_t mass;

  int flags;
  int extendedflags;
  damage_t damage;

  // linked list of losing benefits, or NULL
  benefit_t *lose_benefits;
  
  // linked list of pickup benefits, or NULL
  benefit_t *pickup_benefits;

  // pickup message, a reference to languages.ldf
  char *pickup_message;

  // linked list of initial benefits for players, or NULL if none
  benefit_t *initial_benefits;

  int castorder;
  int respawntime;
  percent_t translucency;
  percent_t minatkchance;
  const colourmap_t *palremap;

  int jump_delay;
  float_t jumpheight;
  float_t crouchheight;
  percent_t viewheight;
  percent_t shotheight;
  float_t maxfall;
  float_t fast;
  float_t xscale;
  float_t yscale;
  float_t bounce_speed;
  float_t bounce_up;
  float_t sight_slope;
  angle_t sight_angle;
  float_t ride_friction;
  percent_t shadow_trans;

  sfx_t *seesound;
  sfx_t *attacksound;
  sfx_t *painsound;
  sfx_t *deathsound;
  sfx_t *overkill_sound;
  sfx_t *activesound;
  sfx_t *walksound;
  sfx_t *jump_sound;
  sfx_t *noway_sound;
  sfx_t *oof_sound;
  sfx_t *gasp_sound;

  int fuse;
  bitset_t side;
  int playernum;

  // breathing support: lung_capacity is how many tics we can last
  // underwater.  gasp_start is how long underwater before we gasp
  // when leaving it.  Damage and choking interval is in choke_damage.
  int lung_capacity;
  int gasp_start;
  damage_t choke_damage;

  // controls how much the player bobs when walking.
  percent_t bobbing;

  // what attack classes we are immune to (usually none).
  bitset_t immunity;

  const struct attacktype_s *closecombat;
  const struct attacktype_s *rangeattack;
  const struct attacktype_s *spareattack;

  // halo information (height < 0 disables halo)
  struct haloinfo_s
  {
    float_t height;
    float_t size, minsize, maxsize;
    percent_t translucency;
    rgbcol_t colour;
    char graphic[10];
  }
  halo;

  // dynamic light info
  struct dlightinfo_s
  {
    dlight_type_e type;
    int intensity;
    rgbcol_t colour;
    percent_t height;
  }
  dlight;

  // item to drop (or NULL).  The mobjinfo pointer is only valid after
  // DDF_MobjCleanUp() has been called.
  const struct mobjinfo_s *dropitem;
  const char *dropitem_ref;

  // blood object (or NULL).  The mobjinfo pointer is only valid after
  // DDF_MobjCleanUp() has been called.
  const struct mobjinfo_s *blood;
  const char *blood_ref;
  
  // respawn effect object (or NULL).  The mobjinfo pointer is only
  // valid after DDF_MobjCleanUp() has been called.
  const struct mobjinfo_s *respawneffect;
  const char *respawneffect_ref;
  
  // spot type for the `SHOOT_TO_SPOT' attack (or NULL).  The mobjinfo
  // pointer is only valid after DDF_MobjCleanUp() has been called.
  const struct mobjinfo_s *spitspot;
  const char *spitspot_ref;
}
mobjinfo_t;

// ------------------------------------------------------------------
// --------------------ATTACK TYPE STRUCTURES------------------------
// ------------------------------------------------------------------

// -KM- 1998/11/25 Added BFG SPRAY attack type.
typedef enum
{
  ATK_PROJECTILE,
  ATK_SPAWNER,
  ATK_TRIPLESPAWNER,
  ATK_SPREADER,
  ATK_RANDOMSPREAD,
  ATK_SHOT,
  ATK_TRACKER,
  ATK_CLOSECOMBAT,
  ATK_SHOOTTOSPOT,
  ATK_SKULLFLY,
  ATK_SMARTPROJECTILE,
  ATK_SPRAY,
  NUMATKCLASS
}
attackstyle_e;

typedef enum
{
  AF_TraceSmoke      = 1,
  AF_KillFailedSpawn = 2,
  AF_PrestepSpawn    = 4,
  AF_SpawnTelefrags  = 8,
  AF_NeedSight       = 16,
  AF_FaceTarget      = 32,
  AF_Player          = 64,
  AF_ForceAim        = 128,
  AF_AngledSpawn     = 256,
  AF_NoTriggerLines  = 512
}
attackflags_e;

typedef struct attacktype_s
{
  ddf_base_t ddf;

  attackstyle_e attackstyle;
  attackflags_e flags;
  sfx_t *initsound;
  sfx_t *sound;
  float_t accuracy_slope;
  angle_t accuracy_angle;
  float_t xoffset;
  float_t yoffset;
  angle_t angle_offset;  // -AJA- 1999/09/10.
  float_t slope_offset;  //
  float_t assault_speed;
  float_t height;
  float_t range;
  int count;
  int tooclose;
  damage_t damage;

  // class of the attack.
  bitset_t attack_class;
 
  // object init state.  The integer value only becomes valid after
  // DDF_AttackCleanUp() has been called.
  int objinitstate;
  const char *objinitstate_ref;
  
  percent_t notracechance;
  percent_t keepfirechance;

  // the MOBJ that is integrated with this attack, or NULL
  const struct mobjinfo_s *atk_mobj;

  // spawned object (for spawners).  The mobjinfo pointer only becomes
  // valid after DDF_AttackCleanUp().  Can be NULL.
  const struct mobjinfo_s *spawnedobj;
  const char *spawnedobj_ref;
  
  // puff object.  The mobjinfo pointer only becomes valid after
  // DDF_AttackCleanUp() has been called.  Can be NULL.
  const struct mobjinfo_s *puff;
  const char *puff_ref;
}
attacktype_t;

// ------------------------------------------------------------------
// -----------------------WEAPON HANDLING----------------------------
// ------------------------------------------------------------------

// -AJA- 2000/01/12: Weapon special flags
typedef enum
{
  // monsters cannot hear this weapon (doesn't wake them up)
  WPSP_SilentToMonsters = 0x0001
}
weapon_flag_e;

// Weapon info: sprite frames, ammunition use.
typedef struct weaponinfo_s
{
  // Weapon's name, etc...
  ddf_base_t ddf;

  // Attack type used.
  struct attacktype_s *attack;
  
  // Type of ammo this weapon uses.
  ammotype_t ammo;
  
  // Ammo used per shot.
  int ammopershot;
  
  // Amount of shots in a clip
  int clip;
  
  // If true, this is an automatic weapon.  If false it is semiautomatic.
  boolean_t autofire;
  
  // Amount of kick this weapon gives
  float_t kick;
  
  // Second attack type.
  struct attacktype_s *sa_attack;
  
  // Type of ammo for second attack.
  ammotype_t sa_ammo;
  
  // Ammo used per second attack shot.
  int sa_ammopershot;

  // Amount of shots in a second attack clip
  int sa_clip;
  
  // Second attack is automatic ?
  boolean_t sa_autofire;
  
  // range of states used
  int first_state;
  int last_state;
  
  // State to use when raising the weapon
  int up_state;
  
  // State to use when lowering the weapon (if changing weapon)
  int down_state;
  
  // State that the weapon is ready to fire in.
  int ready_state;
  
  // State showing the weapon 'firing'
  int attack_state;
  
  // State showing the weapon being reloaded
  int reload_state;

  // State showing the muzzle flash
  int flash_state;
  
  // State showing the second attack firing
  int sa_attack_state;
  
  // State showing the second attack reloading
  int sa_reload_state;

  // State showing the second attack muzzle flash
  int sa_flash_state;
  
  // Crosshair states
  int crosshair;
  
  // State showing viewfinder when zoomed.  Can be zero
  int zoom_state;

  // The player gets this weapon on spawn.  (Fist + Pistol)
  boolean_t autogive;
  
  // This weapon gives feedback on hit (chainsaw)
  boolean_t feedback;
  
  // This weapon upgrades a previous one. (Berserk -> Fist)
  int upgraded_weap;
 
  // This affects how it will be selected if out of ammo.  Also
  // determines the cycling order when on the same key.  Dangerous
  // weapons are not auto-selected when out of ammo.
  int priority;
  boolean_t dangerous;
 
  // Attack type for the WEAPON_EJECT code pointer.
  struct attacktype_s *eject_attack;
  
  // Sounds.
  // Played at the start of every readystate
  sfx_t *idle;
  
  // Played while the trigger is held (chainsaw)
  sfx_t *engaged;
  
  // Played while the trigger is held and it is pointed at a target.
  sfx_t *hit;
  
  // Played when the weapon is selected
  sfx_t *start;
  
  // Misc sounds
  sfx_t *sound1;
  sfx_t *sound2;
  sfx_t *sound3;
  
  // This close combat weapon should not push the target away (chainsaw)
  boolean_t nothrust;
  
  // which number key this weapon is bound to, or -1 for none
  int bind_key;
  
  // -AJA- 2000/01/12: weapon special flags
  weapon_flag_e special_flags;

  // -AJA- 2000/03/18: when > 0, this weapon can zoom
  angle_t zoom_fov;

  // -AJA- 2000/05/23: weapon loses accuracy when refired.
  boolean_t refire_inacc;

  // -AJA- 2000/10/20: show current clip in status bar (not total)
  boolean_t show_clip;

  // controls for weapon bob (up & down) and sway (left & right).
  // Given as percentages in DDF.
  percent_t bobbing;
  percent_t swaying;
}
weaponinfo_t;

// -KM- 1998/11/25 Dynamic number of choices, 10 keys.
typedef struct
{
  int numchoices;
  weaponinfo_t ** choices;
}
weaponkey_t;

// ------------------------------------------------------------------
// --------------------------ANIMATIONS------------------------------
// ------------------------------------------------------------------

//
// source animation definition
//
// -KM- 98/07/31 Anims.ddf
//
typedef struct
{
  ddf_base_t ddf;

  // true for textures, false for flats
  boolean_t istexture;
  
  // first and last names in TEXTURE1/2 lump
  char endname[10];
  char startname[10];

  // how many 1/35s ticks each frame lasts
  int speed;
}
animdef_t;

//
// SWITCHES
//
#define BUTTONTIME  35

typedef struct switchcache_s
{
  const struct image_s *image[2];
}
switchcache_t;

typedef struct switchlist_s
{
  ddf_base_t ddf;

  char name1[10];
  char name2[10];

  // sound effects for this switch
  sfx_t *on_sfx;
  sfx_t *off_sfx;

  // how many ticks the button image lasts
  int time;

  switchcache_t cache;
}
switchlist_t;

// ------------------------------------------------------------------
// ---------------MAP STRUCTURES AND CONFIGURATION-------------------
// ------------------------------------------------------------------

// -KM- 1998/11/25 Added generalised Finale type.
typedef struct
{
  // Text
  char *text;
  char text_back[10];
  char text_flat[10];
  float_t text_speed;
  unsigned int text_wait;

  // Pic
  unsigned int numpics;
  unsigned int picwait;
  char *pics;

  // Cast
  boolean_t docast;

  // Bunny
  boolean_t dobunny;

  // Music
  int music;
}
finale_t;

typedef enum
{
   MPF_Jumping       = 0x1,
   MPF_Mlook         = 0x2,
   MPF_Translucency  = 0x4,
   MPF_Cheats        = 0x8,
   MPF_ItemRespawn   = 0x10,
   MPF_FastParm      = 0x20,     // Fast Monsters
   MPF_ResRespawn    = 0x40,     // Resurrect Monsters (else Teleport)
   MPF_StretchSky    = 0x80,
   MPF_True3D        = 0x100,    // True 3D Gameplay
   MPF_Stomp         = 0x200,    // Monsters can stomp players
   MPF_MoreBlood     = 0x400,    // Make a bloody mess
   MPF_Respawn       = 0x800,
   MPF_AutoAim       = 0x1000,
   MPF_AutoAimMlook  = 0x2000,
   MPF_ResetPlayer   = 0x4000,   // Force player back to square #1
   MPF_Extras        = 0x8000,
   MPF_LimitZoom     = 0x10000,  // Limit zoom to certain weapons
   MPF_Shadows       = 0x20000,
   MPF_Halos         = 0x40000,
   MPF_Crouching     = 0x80000,
   MPF_Kicking       = 0x100000, // Weapon recoil
   MPF_BoomCompat    = 0x200000
}
mapsettings_t;

typedef enum
{
  // standard Doom shading
  LMODEL_Doom = 0,

  // Doom shading without the brighter N/S, darker E/W walls
  LMODEL_Doomish = 1,

  // flat lighting (no shading at all)
  LMODEL_Flat = 2,

  // vertex lighting
  LMODEL_Vertex = 3
}
lighting_model_e;

typedef enum
{
  // standard Doom intermission stats
  WISTYLE_Doom = 0,

  // no stats at all
  WISTYLE_None = 1
}
intermission_style_e;

typedef struct mapstuff_s
{
  ddf_base_t ddf;

  // next in the list
  struct mapstuff_s *next;

  // level description, a reference to languages.ldf
  char *description;
  
  char namegraphic[10];
  char lump[10];
  char sky[10];
  char surround[10];
  int music;

  int partime;
  char *episode_name;

  // flags come in two flavours: "force on" and "force off".  When not
  // forced, then the user is allowed to control it (not applicable to
  // all the flags, e.g. RESET_PLAYER).
  mapsettings_t force_on;
  mapsettings_t force_off;

  // name of the next normal level
  char nextmapname[10];

  // name of the secret level
  char secretmapname[10];

  // -KM- 1998/11/25 All lines with this trigger will be activated at
  // the level start. (MAP07)
  int autotag;

  // -AJA- 2000/08/23: lighting model & intermission style
  lighting_model_e lighting;
  intermission_style_e wistyle;

  // -KM- 1998/11/25 Generalised finales.
  finale_t f[2];
}
mapstuff_t;

// ------------------------------------------------------------------
// -------------------------INTERMISSIONS----------------------------
// ------------------------------------------------------------------

typedef enum
{
  WI_NORMAL,
  WI_LEVEL
}
animtype_t;

typedef struct
{
  int x;
  int y;
}
point_t;

typedef struct
{
  // Tics on this frame
  int tics;

  // Position on screen where this goes
  point_t pos;

  // Name of pic to display.
  char pic[10];

  // cached image
  const struct image_s *image;
}
wi_frame_t;

typedef struct wi_anim_s
{
  animtype_t type;
  char level[10];

  int numframes;
  wi_frame_t *frames;

  // Countdown number of tics
  int count;
  int frameon;
}
wi_anim_t;

typedef struct
{
  point_t pos;
  char name[10];
}
mappos_t;

typedef struct wi_map_s
{
  ddf_base_t ddf;

  wi_anim_t *anims;
  int numanims;

  mappos_t *mappos;
  boolean_t *mapdone;
  int nummaps;

  char background[10];
  char splatpic[10];
  char yah[2][10];

  // -AJA- 1999/10/22: background cameras.
  char bg_camera[32];

  int music;
  sfx_t *percent;
  sfx_t *done;
  sfx_t *endmap;
  sfx_t *nextmap;
  sfx_t *accel_snd;
  sfx_t *frag_snd;

  char firstmap[10];
  char namegraphic[10];
  char **titlepics;
  int numtitlepics;
  int titlemusic;
  int titletics;
}
wi_map_t;

// ------------------------------------------------------------------
// ---------------------------LANGUAGES------------------------------
// ------------------------------------------------------------------

typedef struct langref_s
{
  const char *refname;
  const char *string;

  struct langref_s *next;
}
langref_t;

// -AJA- 2000/04/16: new container, one for each language

typedef struct language_s
{
  ddf_base_t ddf;

  langref_t *refs;
}
language_t;


// ------------------------------------------------------------------
// ------------------------LINEDEF TYPES-----------------------------
// ------------------------------------------------------------------

// Triggers (What the line triggers)
typedef enum
{
  line_none,
  line_shootable,
  line_walkable,
  line_pushable,
  line_manual,   // same as pushable, but ignore any tag
  line_Any
}
trigger_e;

// Triggers (What object types can cause the line to be triggered)
typedef enum
{
  trig_player  = 1,
  trig_monster = 2,
  trig_other   = 4
}
trigacttype_e;

// Height Info Reference
typedef enum
{
  REF_Absolute = 0,  // Absolute from current position
  REF_Current,       // Measure from current sector height
  REF_Surrounding,   // Measure from surrounding heights
  REF_LowestLoTexture,

  // additive flags
  REF_MASK    = 0x00FF,
  REF_CEILING = 0x0100,   // otherwise floor
  REF_HIGHEST = 0x0200,   // otherwise lowest
  REF_NEXT    = 0x0400,   // otherwise absolute
  REF_INCLUDE = 0x0800,   // otherwise excludes self
}
heightref_e;

// Light Specials
typedef enum
{
  LITE_None,

  // set light to new level instantly
  LITE_Set,

  // fade light to new level over time
  LITE_Fade,

  // flicker like a fire
  LITE_FireFlicker,

  // smoothly fade between bright and dark, continously
  LITE_Glow,

  // blink randomly between bright and dark
  LITE_Flash,

  // blink between bright and dark, alternating
  LITE_Strobe
}
litetype_e;

// Movement type
typedef enum
{
  mov_undefined,
  mov_Once,
  mov_MoveWaitReturn,
  mov_Continuous,
  mov_Plat,
  mov_Stairs,
  mov_Stop
}
movetype_e;

// Security type: requires certain key
typedef enum
{
  KF_NONE = 0,

  // keep card/skull together, for easy SKCK check
  KF_BlueCard    = (1 << 0),
  KF_YellowCard  = (1 << 1),
  KF_RedCard     = (1 << 2),
  KF_GreenCard   = (1 << 3),

  KF_BlueSkull   = (1 << 4),
  KF_YellowSkull = (1 << 5),
  KF_RedSkull    = (1 << 6),
  KF_GreenSkull  = (1 << 7),

  // -AJA- 2001/06/30: ten new keys (these + Green ones)
  KF_GoldKey     = (1 << 8),
  KF_SilverKey   = (1 << 9),
  KF_BrassKey    = (1 << 10),
  KF_CopperKey   = (1 << 11),
  KF_SteelKey    = (1 << 12),
  KF_WoodenKey   = (1 << 13),
  KF_FireKey     = (1 << 14),
  KF_WaterKey    = (1 << 15),

  // this is a special flag value that indicates that _all_ of the
  // keys in the bitfield must be held.  Normally we require _any_ of
  // the keys in the bitfield to be held.
  //
  KF_STRICTLY_ALL = (1 << 16),

  // Boom compatibility: don't care if card or skull
  KF_BOOM_SKCK = (1 << 17),

  // mask of actual key bits
  KF_CARDS  = 0x000F,
  KF_SKULLS = 0x00F0,
  KF_MASK = 0xFFFF
}
keys_e;

#define EXPAND_KEYS(set)  ((set) |  \
    (((set) & KF_CARDS) << 4) | (((set) & KF_SKULLS) >> 4))


// -AJA- 1999/06/21: extra floor types

typedef enum
{
  // keeps the value from being zero
  EXFL_Present = 0x0001,

  // floor is thick, has sides.  When clear: surface only
  EXFL_Thick = 0x0002,

  // floor is liquid, i.e. non-solid.  When clear: solid
  EXFL_Liquid = 0x0004,

  // can monsters see through this extrafloor ?
  EXFL_SeeThrough = 0x0010,

  // things with the WATERWALKER tag will not fall through.
  // Also, certain player sounds (pain, death) can be overridden when
  // in a water region.  Scope for other "waterish" effects...
  // 
  EXFL_Water = 0x0020,

  // the region properties will "flood" all lower regions (unless it
  // finds another flooder).
  // 
  EXFL_Flooder = 0x0040,

  // the properties (lighting etc..) below are not transferred from
  // the dummy sector, they'll be the same as the above region.
  // 
  EXFL_NoShade = 0x0080,

  // take the side texture for THICK floors from the upper part of the
  // sidedef where the thick floor is drawn (instead of tagging line).
  // 
  EXFL_SideUpper = 0x0100,

  // like above, but use the lower part.
  EXFL_SideLower = 0x0200,

  // Boom compatibility flag (for linetype 242)
  EXFL_BoomTex = 0x0400
}
extrafloor_type_e;

#define EF_DEF_THIN    (EXFL_Present | 0)
#define EF_DEF_THICK   (EXFL_Present | EXFL_Thick)
#define EF_DEF_LIQUID  (EXFL_Present | EXFL_Liquid)

typedef enum
{
  EFCTL_None = 0,

  // remove an extra floor
  EFCTL_Remove
}
extrafloor_control_e;

typedef struct extrafloor_info_s
{
  extrafloor_type_e type;
  extrafloor_control_e control;
}
extrafloor_info_t;

// -AJA- 1999/07/12: teleporter special flags.
typedef enum
{
  TELSP_SameDir = 0x0001,
  TELSP_SameHeight = 0x0002,
  TELSP_SameSpeed = 0x0004,
  TELSP_SameOffset = 0x0008,
  TELSP_Rotate = 0x0010
}
teleportspecial_e;

#define TELSP_Preserve  \
    (TELSP_SameDir | TELSP_SameHeight | TELSP_SameSpeed)

//
// Moving Planes (Ceilings, floors and doors)
//
typedef struct moving_plane_s
{
  // Type of floor: raise/lower/etc
  movetype_e type;

  // True for a ceiling, false for a floor
  boolean_t is_ceiling;
  boolean_t crush;

  // How fast the plane moves.
  float_t speed_up;
  float_t speed_down;

  // This refers to what the dest. height refers to.
  heightref_e destref;

  // Destination height.
  float_t dest;

  // -AJA- 2001/05/28: This specifies the other height used.
  heightref_e otherref;
  float_t other;

  // Floor texture to change to.
  char tex[10];

  // PLAT/DOOR Specific: Time to wait before returning.
  int wait;
  int prewait;

  // Up/Down/Stop sfx
  sfx_t *sfxstart, *sfxup, *sfxdown, *sfxstop;

  // Scrolling. -AJA- 2000/04/16
  angle_t scroll_angle;
  float_t scroll_speed;
}
moving_plane_t;

//
// Elevator structure
//
// This is essentially used the same way as linking the floor and
// ceiling together, but I don't intend on hacking the code for that,
// so we do it the more informative way.
//
// -ACB- 2001/01/11 Added as elevator sector
//
typedef struct elevator_sector_s
{
  // Type of floor: raise/lower/etc
  movetype_e type;
  
  // How fast the elevator moves.
  float_t speed_up;
  float_t speed_down;

  // Wait times.
  int wait;       
  int prewait;

  // Up/Down/Stop sfx
  sfx_t *sfxstart, *sfxup, *sfxdown, *sfxstop;  
}
elevator_sector_t;

typedef enum
{
  // not a slider
  SLIDE_None = 0,

  // door slides left (when looking at the right side)
  SLIDE_Left,

  // door slides right (when looking at the right side)
  SLIDE_Right,

  // door opens from middle
  SLIDE_Center
}
slidetype_e;

//
// Thin Sliding Doors
//
// -AJA- 2000/08/05: added this.
//
typedef struct sliding_door_s
{
  // type of slider, normally SLIDE_None
  slidetype_e type;

  // how fast it opens/closes
  float_t speed;

  // time to wait before returning (in tics).  Note: door stays open
  // after the last activation.
  int wait;

  // whether or not the texture can be seen through
  boolean_t see_through;

  // how far it actually opens (usually 100%)
  percent_t distance;

  // sound effects.
  sfx_t *sfx_start;
  sfx_t *sfx_open;
  sfx_t *sfx_close;
  sfx_t *sfx_stop;
}
sliding_door_t;

// DONUT SPECIFIC
typedef struct donut_s
{
  // Do Donut?
  boolean_t dodonut;

  // SFX for inner donut parts
  sfx_t *d_sfxin, *d_sfxinstop;

  // SFX for outer donut parts
  sfx_t *d_sfxout, *d_sfxoutstop;
}
donut_t;

typedef struct teleport_point_s
{
  // If true, teleport activator
  boolean_t teleport;

  // effect object spawned when going in...
  const mobjinfo_t *inspawnobj;
  const char *inspawnobj_ref;

  // effect object spawned when going out...
  const mobjinfo_t *outspawnobj;
  const char *outspawnobj_ref;

  // Teleport delay
  int delay;

  // Special flags.
  teleportspecial_e special;
}
teleport_point_t;

// -KM- 1998/09/27 Generalisation of light types for lines + sectors
typedef struct
{
  litetype_e type;

  // light level to change to (for SET and FADE)
  int level;

  // chance value for FLASH type
  percent_t chance;
  
  // time remaining dark and bright, in tics
  int darktime;
  int brighttime;

  // synchronisation time, in tics
  int sync;

  // stepping used for FADE and GLOW types
  int step;
}
lighttype_t;

typedef enum
{
  EXIT_None = 0,
  EXIT_Normal,
  EXIT_Secret
}
exittype_e;

typedef enum
{
  // make tagged lines (inclusive) 50% translucent
  LINEFX_Translucency = 0x0001,

  // make tagged walls (inclusive) scroll using vector
  LINEFX_VectorScroll = 0x0002,

  // make source line scroll using sidedef offsets
  LINEFX_OffsetScroll = 0x0004,

  // experimental: tagged walls (inclusive) scaling & skewing
  LINEFX_Scale = 0x0010,
  LINEFX_Skew  = 0x0020,

  // experimental: transfer properties to tagged walls (incl)
  LINEFX_LightWall = 0x0040,

  // experimental: make tagged lines (exclusive) non-blocking
  LINEFX_UnblockThings = 0x0100,

  // experimental: make tagged lines (incl) block bullets/missiles
  LINEFX_BlockShots = 0x0200,

  // experimental: make tagged lines (incl) block monster sight
  LINEFX_BlockSight = 0x0400
}
line_effect_type_e;

typedef enum
{
  // transfer sector lighting to tagged floors/ceilings
  SECTFX_LightFloor   = 0x0001,
  SECTFX_LightCeiling = 0x0002,

  // make tagged floors/ceilings scroll
  SECTFX_ScrollFloor   = 0x0004,
  SECTFX_ScrollCeiling = 0x0008,

  // push things on tagged floor
  SECTFX_PushThings = 0x0010,

  // restore light/scroll/push in tagged floors/ceilings
  SECTFX_ResetFloor   = 0x0040,
  SECTFX_ResetCeiling = 0x0080,

  // experimental: set floor/ceiling texture scale
  SECTFX_ScaleFloor   = 0x0100,
  SECTFX_ScaleCeiling = 0x0200,

  // experimental: align floor/ceiling texture to line
  SECTFX_AlignFloor   = 0x0400,
  SECTFX_AlignCeiling = 0x0800
}
sector_effect_type_e;

// -AJA- 1999/10/12: Generalised scrolling parts of walls.
typedef enum
{
  SCPT_RightUpper  = 0x0001,
  SCPT_RightMiddle = 0x0002,
  SCPT_RightLower  = 0x0004,

  SCPT_LeftUpper  = 0x0010,
  SCPT_LeftMiddle = 0x0020,
  SCPT_LeftLower  = 0x0040,

  SCPT_LeftRevX   = 0x0100,
  SCPT_LeftRevY   = 0x0200
}
scroll_part_e;

#define SCPT_RIGHT  (SCPT_RightUpper | SCPT_RightMiddle | SCPT_RightLower)
#define SCPT_LEFT   (SCPT_LeftUpper | SCPT_LeftMiddle | SCPT_LeftLower)

// -AJA- 2001/01/23: Implementation of layered skies.
typedef enum
{
  TILESKY_None = 0,
  TILESKY_Flat,
  TILESKY_Texture
}
tilesky_type_e;

#define MAX_TILESKY  4

typedef struct tilesky_info_s
{
  tilesky_type_e type;

  // which layer, lower numbers are further away
  int layer;
  
  // how many times the image should tile
  int number;

  // dimensions for the sky sphere
  float_t squish;
  float_t offset;
}
tilesky_info_t;

typedef struct ladder_info_s
{
  // height of ladder itself.  Zero or negative disables.  Bottom of
  // ladder comes from Y_OFFSET on the linedef.
  float_t height;
}
ladder_info_t;

// -AJA- 1999/10/24: Reimplemented when_appear_e type.
typedef enum
{
  WNAP_SkillBits = 0x001F,

  WNAP_Single = 0x0100,
  WNAP_Coop = 0x0200,
  WNAP_DeathMatch = 0x0400
}
when_appear_e;

#define DEFAULT_APPEAR  (0xFFFF)

// -AJA- 1999/12/07: Linedef special flags
typedef enum
{
  // player must be able to vertically reach this linedef to press it
  LINSP_MustReach = 0x0001,

  // don't change the texture on other linedefs with the same tag
  LINSP_SwitchSeparate = 0x0002
}
line_special_e;

// Linedef structure itself
typedef struct linedeftype_s
{
  // Line's name, number, etc...
  ddf_base_t ddf;

  // Linedef will change to this.
  int newtrignum;

  // Determines whether line is shootable/walkable/pushable
  trigger_e type;

  // Determines whether line is acted on by monsters/players/projectiles
  trigacttype_e obj;

  // Keys required to use
  keys_e keys;

  // Number of times this line can be triggered. -1 = Any amount
  int count;

  // Special sector type to change to.  Used to turn off acid
  int specialtype;

  // Crush.  If true, players will be crushed.  If false, obj will stop(/return)
  boolean_t crush;

  // Floor
  moving_plane_t f;

  // Ceiling
  moving_plane_t c;

  // Elevator (moving sector) -ACB- 2001/01/11
  elevator_sector_t e;

  // Donut 
  donut_t d;

  // Slider
  sliding_door_t s;

  // Tile Skies
  tilesky_info_t sky;
  
  // -AJA- 2001/03/10: ladder linetypes
  ladder_info_t ladder;

  // Teleport
  teleport_point_t t;

  // LIGHT SPECIFIC
  // Things may be added here; start strobing/flashing glowing lights.
  lighttype_t l;

  // EXIT SPECIFIC
  exittype_e e_exit;

  // SCROLLER SPECIFIC
  float_t s_xspeed;
  float_t s_yspeed;
  scroll_part_e scroll_parts;

  // -ACB- 1998/09/11 Message handling
  char *failedmessage;

  // Colourmap changing
  // -AJA- 1999/07/09: Now uses colmap.ddf
  const colourmap_t *use_colourmap;

  // Property Transfers
  float_t gravity;
  float_t friction;
  float_t viscosity;
  float_t drag;

  // Ambient sound transfer
  sfx_t *ambient_sfx;

  // Activation sound (overrides the switch sound)
  sfx_t *activate_sfx;

  int music;

  // Automatically trigger this line at level start ?
  boolean_t autoline;

  // Activation only possible from right side of line
  boolean_t singlesided;

  // -AJA- 1999/06/21: Extra floor handling
  extrafloor_info_t ef;

  // -AJA- 1999/06/30: TRANSLUCENT MID-TEXTURES
  percent_t translucency;

  // -AJA- 1999/10/24: Appearance control.
  when_appear_e appear;

  // -AJA- 1999/12/07: line special flags
  line_special_e special_flags;

  // -AJA- 2000/01/09: enable (if +1) or disable (if -1) all radius
  //       triggers with the same tag as the linedef.
  int trigger_effect;

  // -AJA- 2000/09/28: BOOM compatibility fields (and more !).
  line_effect_type_e line_effect;
  scroll_part_e line_parts;
  sector_effect_type_e sector_effect;
}
linedeftype_t;

// -AJA- 1999/11/25: Sector special flags
typedef enum
{
  // apply damage whenever in whole region (not just touching floor)
  SECSP_WholeRegion = 0x0001,

  // goes with above: damage is proportional to how deep you're in
  // Also affects pushing sectors.
  SECSP_Proportional = 0x0002,

  // push _all_ things, including NOGRAVITY ones
  SECSP_PushAll = 0x0008,

  // the push force is constant, regardless of the mass
  SECSP_PushConstant = 0x0010,

  // breathing support: this sector contains no air.
  SECSP_AirLess = 0x0020,

  // player can swim in this sector
  SECSP_Swimming = 0x0040
}
sector_flag_e;

// -KM- 1998/09/27 Sectors.ddf stuff
typedef struct
{
  // Sector's name, number, etc...
  ddf_base_t ddf;

  // This sector gives you secret count
  boolean_t secret;

  // Gravity
  float_t gravity;
  float_t friction;
  float_t viscosity;
  float_t drag;

  boolean_t crush;

  // Movement
  moving_plane_t f, c;

  // Elevator: Added -ACB- 2001/01/11
  elevator_sector_t e;

  // Lighting
  lighttype_t l;

  // Slime
  damage_t damage;

  // -AJA- 1999/11/25: sector special flags
  sector_flag_e special_flags;

  // Exit.  Also disables god mode.
  exittype_e e_exit;

  // Colourmap changing
  // -AJA- 1999/07/09: Now uses colmap.ddf
  const colourmap_t *use_colourmap;

  // SFX
  sfx_t *ambient_sfx;

  // -AJA- 1999/10/24: Appearance control.
  when_appear_e appear;

  // -AJA- 2000/01/02: DDF-itisation of crushers.
  int crush_time;
  float_t crush_damage;

  // -AJA- 2000/04/16: Pushing (fixed direction).
  float_t push_speed;
  float_t push_zspeed;
  angle_t push_angle;
}
specialsector_t;

// ----------------------------------------------------------------
// -------------------------MUSIC PLAYLIST-------------------------
// ----------------------------------------------------------------

typedef enum
{
  MUS_UNKNOWN   = 0,
  MUS_CD        = 1,
  MUS_MIDI      = 2,
  MUS_MUS       = 3,
  MUS_MP3       = 4,
  ENDOFMUSTYPES = 5
}
musictype_t;

typedef enum
{
  MUSINF_UNKNOWN   = 0,
  MUSINF_TRACK     = 1,
  MUSINF_LUMP      = 2,
  MUSINF_FILE      = 3,
  ENDOFMUSINFTYPES = 4
}
musicinftype_t;

typedef struct playlist_s
{
  ddf_base_t ddf;
  musictype_t type;
  musicinftype_t infotype;
  char *info;
}
playlist_t;

// ----------------------------------------------------------------
// ------------------------ SOUND EFFECTS ------------------------
// ----------------------------------------------------------------

//
// -ACB- 1999/10/06 Removed usefulness and priv_data. Usefulness was
//                  removed because I felt no need for sound caching
//                  in a basic system.  Priv_data was removed as this
//                  should be held by only the system specific code.
//                  Lumpnum got booted for the same reason as above.

typedef struct sfxinfo_s
{
  // sound's name, etc..
  ddf_base_t ddf;

  // full sound lump name
  char lump_name[10];

  // sfxinfo ID number
  // -AJA- Changed to a sfx_t.  It serves two purposes: (a) hold the
  //       sound ID, like before, (b) better memory usage, as we don't
  //       need to allocate a new sfx_t for non-wildcard sounds.
  sfx_t normal;

  // Sfx singularity (only one at a time), or 0 if not singular
  int singularity;

  // Sfx priority
  int priority;

  // volume of sound, 255 is normal, lower is quieter.
  percent_t volume;

  // -KM- 1998/09/01  Looping: for non NULL origins
  boolean_t looping;

  // -AJA- 2000/04/19: Prefer to play the whole sound rather than
  //       chopping it off with a new sound.
  boolean_t precious;
 
  // distance limit, if the hearer is further away than `max_distance'
  // then the this sound won't be played at all.
  float_t max_distance;
 
  // for the cache's circular linked list. NULL if not cached.
  struct sfxinfo_s *next, *prev;
}
sfxinfo_t;

// ------------------------------------------------------------------
// -------------------------EXTERNALISATIONS-------------------------
// ------------------------------------------------------------------

extern state_t *states;
extern int num_states;

extern animdef_t ** animdefs;
extern int numanimdefs;

extern attacktype_t ** ddf_attacks;
extern int num_ddf_attacks;

extern linedeftype_t ** ddf_linetypes;
extern int num_ddf_linetypes;

extern specialsector_t ** ddf_sectors;
extern int num_ddf_sectors;

extern wi_map_t ** wi_maps;
extern int num_wi_maps;

extern mapstuff_t ** level_maps;
extern int num_level_maps;
extern const mapstuff_t *currentmap;
extern const mapstuff_t *nextmap;

extern const moving_plane_t donut_floor;
extern const linedeftype_t template_line;
extern linedeftype_t *specialLineDefs[211];

extern language_t ** languages;
extern int num_languages;
extern int cur_lang_index;

extern mobjinfo_t ** mobjinfo;
extern int num_mobjinfo;

// the complete set of sound effects
extern sfxinfo_t ** S_sfx;
extern int numsfx;

extern switchlist_t ** alph_switches;
extern int num_alph_switches;

// -KM- 1998/11/25 Dynamic number of weapons, always 10 weapon keys.
extern weaponinfo_t ** weaponinfo;
extern int numweapons;
extern int num_disabled_weapons;
extern weaponkey_t weaponkey[10];

void DDF_MainInit(void);
boolean_t DDF_MainCleanUp(void);
boolean_t DDF_MainParseCondition(const char *str, condition_check_t *cond);

int DDF_CompareName(const char *A, const char *B);
sfx_t *DDF_SfxLookupSound(const char *name);
sfxinfo_t *DDF_SfxSelect(const sfx_t *sound_id);
const char *DDF_LanguageLookup(const char *refname);
boolean_t DDF_LanguageValidRef(const char *refname);

void DDF_MobjGetBenefit(const char *info, void *storage);
const mobjinfo_t *DDF_MobjLookup(const char *refname);
const mobjinfo_t *DDF_MobjLookupNum(int number);
const mobjinfo_t *DDF_MobjLookupCast(int castnum);
const mobjinfo_t *DDF_MobjLookupPlayer(int playernum);

const colourmap_t *DDF_ColmapLookup(const char *name);
const wi_map_t *DDF_GameLookup(const char *name);
const playlist_t *DDF_MusicLookupNum(int number);
const mapstuff_t *DDF_LevelMapLookup(const char *refname);
const linedeftype_t *DDF_LineLookupNum(int number);
const specialsector_t *DDF_SectorLookupNum(int number);
int DDF_WeaponLookup(const char *name);

struct sector_s;

// -AJA- 2001/05/29: BOOM generalised line/sector type support.
void DDF_LineClearGeneralised(void);
void DDF_SectorClearGeneralised(void);

// -KM- 1998/12/16 If you have a ddf file to add, call
//  this, passing the data and the size of the data in
//  memory.
void DDF_ReadAnims(void *data, int size);
void DDF_ReadAtks(void *data, int size);
void DDF_ReadColourMaps(void *data, int size);
void DDF_ReadGames(void *data, int size);
void DDF_ReadLangs(void *data, int size);
void DDF_ReadLevels(void *data, int size);
void DDF_ReadLines(void *data, int size);
void DDF_ReadMusicPlaylist(void *data, int size);
void DDF_ReadSectors(void *data, int size);
void DDF_ReadSFX(void *data, int size);
void DDF_ReadSW(void *data, int size);
void DDF_ReadThings(void *data, int size);
void DDF_ReadWeapons(void *data, int size);

#endif
