#include "i_defs.h"

#include "ddf_main.h"
#include "vc.h"

char *AmmoName[] =
{
	"am_noammo",
	"am_clip",
	"am_shell",
	"am_misl",
	"am_cell",
	"am_pellet",
	"am_nail",
	"am_grenade",
	"am_gas",
};
char *GetAmmoName(int num)
{
	return AmmoName[num + 1];
}

void VC_WriteWeapons(void)
{
	int i;
	int j;
	FILE *f;
	int best;

	f = fopen("../../progs/ddf/wpninfo.vc", "w");
	cur_file = f;
	for (i = num_disabled_weapons; i < numweapons; i++)
	{
		fprintf(f, "class Weapon%d;\n", i);
	}
	fprintf(f, "\nclassid WeaponClasses[] = {\n");
	for (i = num_disabled_weapons; i < numweapons; i++)
	{
		fprintf(f, "\tWeapon%d,\n", i);
	}
	fprintf(f, "};\n\nint weapon_ammo_type[] = {\n");
	for (i = num_disabled_weapons; i < numweapons; i++)
	{
		fprintf(f, "\t%s,\n", GetAmmoName(weaponinfo[i]->ammo));
	}
	fprintf(f, "};\n");
	fclose(f);

	f = fopen("../../progs/ddf/weapons.vc", "w");
	cur_file = f;
	for (i = num_disabled_weapons; i < numweapons; i++)
	{
		weaponinfo_t *w = weaponinfo[i];

		fprintf(f, "//**************************************************************************\n");
		fprintf(f, "//\n");
		fprintf(f, "//\t%s\n", w->ddf.name);
		fprintf(f, "//\n");
		fprintf(f, "//**************************************************************************\n");
		fprintf(f, "\nclass Weapon%d:DDFWeapon\n{\n\n", i);

		fprintf(f, "__states__\n");
		fprintf(f, "{\n");
		for (j = w->first_state; j <= w->last_state; j++)
		{
			state_t *s = &states[j];

			if (s->label)
				fprintf(f, "\t// %s\n", s->label);
			fprintf(f, "\tS_%d('%s', %d", j, (char *)s->sprite, s->frame);
			if (s->bright)
				fprintf(f, " | FF_FULLBRIGHT");
			if (s->tics < 0)
				fprintf(f, ", -1.0, ");
			else
				fprintf(f, ", %d.0 / 35.0, ", (int)s->tics);
			if (s->nextstate)
				fprintf(f, "S_%d) { ", s->nextstate);
			else
				fprintf(f, "S_NULL) { ");
			if (s->action)
				s->action((mobj_t *)s);
			fprintf(f, "}\n");
		}
		fprintf(f, "}\n");
		fprintf(f, "\n");

		fprintf(f, "defaultproperties\n");
		fprintf(f, "{\n");

/*
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
*/
		fprintf(f, "\tAmmo = %s;\n", GetAmmoName(w->ammo));

/*
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
*/

		if (w->up_state)
			fprintf(f, "\tUpState = S_%d;\n", w->up_state);
		if (w->down_state)
			fprintf(f, "\tDownState = S_%d;\n", w->down_state);
		if (w->ready_state)
			fprintf(f, "\tReadyState = S_%d;\n", w->ready_state);
		if (w->attack_state)
			fprintf(f, "\tAttackState = S_%d;\n", w->attack_state);
/*
  // State showing the weapon being reloaded
  int reload_state;
*/
		if (w->flash_state)
			fprintf(f, "\tFlashState = S_%d;\n", w->flash_state);
/*
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
*/
		fprintf(f, "}\n");
		fprintf(f, "\n}\n\n");
	}

	fprintf(f, "void SetInitialWeapons(player_t *player)\n");
	fprintf(f, "{\n");
	best = 0;
	for (i = num_disabled_weapons; i < numweapons; i++)
	{
		weaponinfo_t *w = weaponinfo[i];
		if (w->autogive)
		{
			fprintf(f, "\tplayer->WeaponOwned[%d] = true;\n", i);
			best = i;
		}
	}
	fprintf(f, "\tSetWeapon(player, %d);\n", best);
	fprintf(f, "}\n");
	fclose(f);
}

