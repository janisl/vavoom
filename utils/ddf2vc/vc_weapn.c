#include "i_defs.h"

#include "ddf_main.h"
#include "vc.h"

//	Not included in headers.
extern int num_disabled_mobjinfo;

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
	char fname[256];

	sprintf(fname, "%s/wpninfo.vc", progsdir);
	I_Printf("Writing %s\n", fname);
	f = fopen(fname, "w");
	cur_file = f;
	for (i = num_disabled_weapons; i < numweapons; i++)
	{
		fprintf(f, "class Weapon%d;\n", i);
	}
	fprintf(f, "\nclassid WeaponClasses[NUMWEAPONS] = {\n");
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

	sprintf(fname, "%s/weapons.vc", progsdir);
	I_Printf("Writing %s\n", fname);
	f = fopen(fname, "w");
	cur_file = f;
	for (i = num_disabled_weapons; i < numweapons; i++)
	{
		weaponinfo_t *w = weaponinfo[i];

		fprintf(f, "//**************************************************************************\n");
		fprintf(f, "//\n");
		fprintf(f, "//\t%s\n", w->ddf.name);
		fprintf(f, "//\n");
		fprintf(f, "//**************************************************************************\n");
		fprintf(f, "\nclass Weapon%d:Weapon\n{\n\n", i);

		if (w->first_state || w->last_state)
		{
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
					fprintf(f, "S_%d)", s->nextstate);
				else
					fprintf(f, "S_NULL)");
				if (s->action)
					s->action((mobj_t *)s);
				else
					fprintf(f, " { ");
				fprintf(f, "}\n");
			}
			fprintf(f, "}\n");
			fprintf(f, "\n");
		}

		fprintf(f, "defaultproperties\n");
		fprintf(f, "{\n");

		PrintAttack(f, w->attack, "Attack");
		fprintf(f, "\tAmmo = %s;\n", GetAmmoName(w->ammo));
		if (w->ammopershot)
			fprintf(f, "\tAmmoPerShot = %d;\n", w->ammopershot);
		if (w->clip != 1)
			fprintf(f, "\tClip = %d;\n", w->clip);
		if (w->autofire)
			fprintf(f, "\tbAutoFire = true;\n");
		if (w->kick)
			fprintf(f, "\tKick = %f;\n", w->kick);

		PrintAttack(f, w->sa_attack, "SAAttack");
		if (w->sa_ammo != AM_NoAmmo)
			fprintf(f, "\tSAAmmo = %s;\n", GetAmmoName(w->sa_ammo));
		if (w->sa_ammopershot)
			fprintf(f, "\tSAAmmoPerShot = %d;\n", w->sa_ammopershot);
		if (w->sa_clip != 1)
			fprintf(f, "\tSAClip = %d;\n", w->sa_clip);
		if (w->sa_autofire)
			fprintf(f, "\tbSAAutoFire = true;\n");

		if (w->up_state)
			fprintf(f, "\tUpState = S_%d;\n", w->up_state);
		if (w->down_state)
			fprintf(f, "\tDownState = S_%d;\n", w->down_state);
		if (w->ready_state)
			fprintf(f, "\tReadyState = S_%d;\n", w->ready_state);
		if (w->attack_state)
			fprintf(f, "\tAttackState = S_%d;\n", w->attack_state);
		if (w->reload_state)
			fprintf(f, "\tReloadState = S_%d;\n", w->reload_state);
		if (w->flash_state)
			fprintf(f, "\tFlashState = S_%d;\n", w->flash_state);
		if (w->sa_attack_state)
			fprintf(f, "\tSAAttackState = S_%d;\n", w->sa_attack_state);
		if (w->sa_reload_state)
			fprintf(f, "\tSAReloadState = S_%d;\n", w->sa_reload_state);
		if (w->sa_flash_state)
			fprintf(f, "\tSAFlashState = S_%d;\n", w->sa_flash_state);
		if (w->crosshair)
			fprintf(f, "\tCrosshair = %d;\n", w->crosshair);
		if (w->zoom_state)
			fprintf(f, "\tZoomState = S_%d;\n", w->zoom_state);
		if (w->feedback)
			fprintf(f, "\tbFeedBack = true;\n");
		if (w->upgraded_weap != -1)
			fprintf(f, "\tUpgradedWeap = %d;\n", w->upgraded_weap);
		if (w->priority)
			fprintf(f, "\tPriority = %d;\n", w->priority);
		if (w->dangerous)
			fprintf(f, "\tbDangerous = true;\n");
		PrintAttack(f, w->eject_attack, "EjectAttack");
		if (w->idle)
			fprintf(f, "\tIdleSound = \'%s\';\n", SFX(w->idle));
		if (w->engaged)
			fprintf(f, "\tEngagedSound = \'%s\';\n", SFX(w->engaged));
		if (w->hit)
			fprintf(f, "\tHitSound = \'%s\';\n", SFX(w->hit));
		if (w->start)
			fprintf(f, "\tStartSound = \'%s\';\n", SFX(w->start));
		if (w->sound1)
			fprintf(f, "\tSound1 = \'%s\';\n", SFX(w->sound1));
		if (w->sound2)
			fprintf(f, "\tSound2 = \'%s\';\n", SFX(w->sound2));
		if (w->sound3)
			fprintf(f, "\tSound3 = \'%s\';\n", SFX(w->sound3));
		if (w->nothrust)
			fprintf(f, "\tbNoThrust = true;\n");
		if (w->bind_key != -1)
			fprintf(f, "\tBindKey = %d;\n", w->bind_key);
		if (w->special_flags & WPSP_SilentToMonsters)
			fprintf(f, "\tbSilentToMonsters = true;\n");
		if (w->zoom_fov)
			fprintf(f, "\tZoomFov = %1.1f;\n", (double)w->zoom_fov * 90.0 / (double)ANG90);
		if (w->refire_inacc)
			fprintf(f, "\tbRefireInacc = true;\n");
		if (w->show_clip)
			fprintf(f, "\tbShowClip = true;\n");
		if (w->bobbing != 1.0f)
			fprintf(f, "\tBobbing = %1.2f;\n", w->bobbing);
		if (w->swaying != 1.0f)
			fprintf(f, "\tSwaying = %1.2f;\n", w->swaying);
		fprintf(f, "}\n");
		fprintf(f, "\n}\n\n");
	}

	fprintf(f, "//==========================================================================\n");
    fprintf(f, "//\n");
    fprintf(f, "//\tSetInitialWeapons\n");
    fprintf(f, "//\n");
    fprintf(f, "//==========================================================================\n");
    fprintf(f, "\n");
	fprintf(f, "void SetInitialWeapons(player_t *player)\n");
	fprintf(f, "{\n");
	for (i = num_disabled_mobjinfo; i < num_mobjinfo; i++)
	{
		if (mobjinfo[i]->playernum == 1)
		{
			WriteInitialBenefits(f, mobjinfo[i]->initial_benefits);
			break;
		}
	}
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

