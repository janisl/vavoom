#include "i_defs.h"

#include "ddf_main.h"
#include "vc.h"

//	Not included in headers.
extern int num_disabled_mobjinfo;

FILE *cur_file;

int GetIndex(const mobjinfo_t *m)
{
	int i;

	for (i = num_disabled_mobjinfo; i < num_mobjinfo; i++)
	{
		if (mobjinfo[i] == m)
			return i;
	}
	return 0;
}

const char *SFX(sfx_t *s)
{
	return S_sfx[s->sounds[0]]->ddf.name;
}

void VC_WriteMobjs(void)
{
	int i;
	int j;
	FILE *f;

	f = fopen("../../progs/ddf/things.vc", "w");
	cur_file = f;
	for (i = num_disabled_mobjinfo; i < num_mobjinfo; i++)
	{
		mobjinfo_t *m = mobjinfo[i];

		fprintf(f, "//**************************************************************************\n");
		fprintf(f, "//\n");
		fprintf(f, "// %s\n", m->ddf.name);
		fprintf(f, "//\n");
		fprintf(f, "//**************************************************************************\n");
		fprintf(f, "\nclass Thing%d:DDFActor\n", i);
		if (m->ddf.number)
			fprintf(f, "\t__mobjinfo__(%d)\n", m->ddf.number);
		fprintf(f, "{\n\n");

		fprintf(f, "__states__\n");
		fprintf(f, "{\n");
		for (j = m->first_state; j <= m->last_state; j++)
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

		if (m->spawn_state)
			fprintf(f, "\tSpawnState = S_%d;\n", m->spawn_state);
		if (m->idle_state)
			fprintf(f, "\tIdleState = S_%d;\n", m->idle_state);
		if (m->chase_state)
			fprintf(f, "\tSeeState = S_%d;\n", m->chase_state);
		if (m->pain_state)
			fprintf(f, "\tPainState = S_%d;\n", m->pain_state);
		if (m->missile_state)
			fprintf(f, "\tMissileState = S_%d;\n", m->missile_state);
		if (m->melee_state)
			fprintf(f, "\tMeleeState = S_%d;\n", m->melee_state);
		if (m->death_state)
			fprintf(f, "\tDeathState = S_%d;\n", m->death_state);
		if (m->overkill_state)
			fprintf(f, "\tGibsDeathState = S_%d;\n", m->overkill_state);
		if (m->raise_state)
			fprintf(f, "\tRaiseState = S_%d;\n", m->raise_state);
#if 0
		if (m->res_state)
			fprintf(f, "\tResState = S_%d;\n", m->res_state);
		if (m->meander_state)
			fprintf(f, "\tMeanderState = S_%d;\n", m->meander_state);
		if (m->bounce_state)
			fprintf(f, "\tbounce_state = S_%d;\n", m->bounce_state);
		if (m->touch_state)
			fprintf(f, "\tTouchState = S_%d;\n", m->touch_state);
		if (m->jump_state)
			fprintf(f, "\tJumpState = S_%d;\n", m->jump_state);
		if (m->gib_state)
			fprintf(f, "\tGibState = S_%d;\n", m->gib_state);
#endif

		if (m->reactiontime)
			fprintf(f, "\tReactionCount = %d;\n", m->reactiontime);
		if (m->painchance)
			fprintf(f, "\tPainChance = %d;\n", (int)(m->painchance * 255));
		if (m->spawnhealth)
			fprintf(f, "\tHealth = %d;\n", (int)m->spawnhealth);
		if (m->speed)
		{
			if (m->extendedflags & EF_MONSTER)
				fprintf(f, "\tStepSpeed = %1.1f;\n", m->speed);
			else
				fprintf(f, "\tSpeed = %1.1f;\n", m->speed * 35);
		}
#if 0
		if (m->float_speed)
			fprintf(f, "\tFloatSpeed = %1.1f;\n", m->float_speed);
#endif
		if (m->radius)
			fprintf(f, "\tRadius = %1.1f;\n", m->radius);
		if (m->height)
			fprintf(f, "\tHeight = %1.1f;\n", m->height);
		if (m->step_size)
			fprintf(f, "\tMaxStepHeight = %1.1f;\n", m->step_size);
		if (m->mass)
			fprintf(f, "\tMass = %1.1f;\n", m->mass);

		if (m->flags & MF_FUZZY)
			fprintf(f, "\tTranslucency = 90;\n");

		if (m->flags & MF_SPECIAL)
			fprintf(f, "\tbSpecial = true;\n");
		if (m->flags & MF_SOLID)
			fprintf(f, "\tbSolid = true;\n");
		if (m->flags & MF_SHOOTABLE)
			fprintf(f, "\tbShootable = true;\n");
		if (m->flags & MF_NOSECTOR)
			fprintf(f, "\tbHidden = true;\n");
		if (m->flags & MF_NOBLOCKMAP)
			fprintf(f, "\tbNoBlockmap = true;\n");
		if (m->flags & MF_AMBUSH)
			fprintf(f, "\tbAmbush = true;\n");
		if (m->flags & MF_JUSTHIT)
			fprintf(f, "\tbJustHit = true;\n");
		if (m->flags & MF_JUSTATTACKED)
			fprintf(f, "\tbJustAttacked = true;\n");
		if (m->flags & MF_SPAWNCEILING)
			fprintf(f, "\tbSpawnCeiling = true;\n");
		if (m->flags & MF_NOGRAVITY)
			fprintf(f, "\tbNoGravity = true;\n");
		if (m->flags & MF_DROPOFF)
			fprintf(f, "\tbDropOff = true;\n");
		if (m->flags & MF_PICKUP)
			fprintf(f, "\tbPickUp = true;\n");
		if (m->flags & MF_NOCLIP)
		{
			fprintf(f, "\tbColideWithWorld = false;\n");
			fprintf(f, "\tbColideWithThings = false;\n");
		}
		if (m->flags & MF_SLIDE)
			fprintf(f, "\tbSlide = true;\n");
		if (m->flags & MF_FLOAT)
			fprintf(f, "\tbFloat = true;\n");
		if (m->flags & MF_TELEPORT)
			fprintf(f, "\tbTeleport = true;\n");
		if (m->flags & MF_MISSILE)
			fprintf(f, "\tbMissile = true;\n");
		if (m->flags & MF_DROPPED)
			fprintf(f, "\tbDropped = true;\n");
		if (m->flags & MF_NOBLOOD)
			fprintf(f, "\tbNoBlood = true;\n");
		if (m->flags & MF_CORPSE)
			fprintf(f, "\tbCorpse = true;\n");
		if (m->flags & MF_INFLOAT)
			fprintf(f, "\tbInFloat = true;\n");
		if (m->flags & MF_COUNTKILL)
			fprintf(f, "\tbCountKill = true;\n");
		if (m->flags & MF_COUNTITEM)
			fprintf(f, "\tbCountItem = true;\n");
		if (m->flags & MF_SKULLFLY)
			fprintf(f, "\tbSkullFly = true;\n");
		if (m->flags & MF_NOTDMATCH)
			fprintf(f, "\tbNoDeathmatch = true;\n");
		if (m->flags & MF_STEALTH)
			fprintf(f, "\tbStealth = true;\n");
		if (m->flags & MF_JUSTPICKEDUP)
			fprintf(f, "\tbJustPickedUp = true;\n");
		if (m->flags & MF_TOUCHY)
			fprintf(f, "\tbTouchy = true;\n");

		if (m->extendedflags & EF_BOSSMAN)
			fprintf(f, "\tbBoss = true;\n");
#if 0
		if (m->extendedflags & EF_LESSVIS)
			fprintf(f, "\tbLessVis = true;\n");
#endif
		if (m->extendedflags & EF_NORESPAWN)
			fprintf(f, "\tbNoRespawn = true;\n");
		if (m->extendedflags & EF_NOGRAVKILL)
			fprintf(f, "\tbNoGravKill = true;\n");
#if 0
		if (m->extendedflags & EF_DISLOYALTYPE)
			fprintf(f, "\tbDisloyalType = true;\n");
		if (m->extendedflags & EF_OWNATTACKHURTS)
			fprintf(f, "\tbOwnAttackHurts = true;\n");
		if (m->extendedflags & EF_FIRSTCHECK)
			fprintf(f, "\tbFirstCheck = true;\n");
		if (m->extendedflags & EF_NOTRACE)
			fprintf(f, "\tbNoTrace = true;\n");
#endif
		if (m->extendedflags & EF_TRIGGERHAPPY)
			fprintf(f, "\tbTriggerHappy = true;\n");
		if (m->extendedflags & EF_NEVERTARGET)
			fprintf(f, "\tbNeverTarget = true;\n");
		if (m->extendedflags & EF_NOGRUDGE)
			fprintf(f, "\tbNoGrudge = true;\n");
#if 0
		if (m->extendedflags & EF_DUMMYMOBJ)
			fprintf(f, "\tbDummyMobj = true;\n");
		if (m->extendedflags & EF_NORESURRECT)
			fprintf(f, "\tbNoResurect = true;\n");
		if (m->extendedflags & EF_BOUNCE)
			fprintf(f, "\tbBounce = true;\n");
		if (m->extendedflags & EF_EDGEWALKER)
			fprintf(f, "\tbEdgeWalker = true;\n");
		if (m->extendedflags & EF_GRAVFALL)
			fprintf(f, "\tbGravFall = true;\n");
		if (m->extendedflags & EF_CLIMBABLE)
			fprintf(f, "\tbClimbable = true;\n");
		if (m->extendedflags & EF_WATERWALKER)
			fprintf(f, "\tbWaterWalker = true;\n");
#endif
		if (m->extendedflags & EF_MONSTER)
			fprintf(f, "\tbMonster = true;\n");
#if 0
		if (m->extendedflags & EF_CROSSLINES)
			fprintf(f, "\tbCrossLines = true;\n");
		if (m->extendedflags & EF_NOFRICTION)
			fprintf(f, "\tbNoFriction = true;\n");
		if (m->extendedflags & EF_EXTRA)
			fprintf(f, "\tbExtra = true;\n");
		if (m->extendedflags & EF_JUSTBOUNCED)
			fprintf(f, "\tbJustBounced = true;\n");
		if (m->extendedflags & EF_USABLE)
			fprintf(f, "\tbUsable = true;\n");
		if (m->extendedflags & EF_BLOCKSHOTS)
			fprintf(f, "\tbBlockShots = true;\n");
		if (m->extendedflags & EF_CROUCHING)
			fprintf(f, "\tbCrouching = true;\n");
		if (m->extendedflags & EF_TUNNEL)
			fprintf(f, "\tbTunnel = true;\n");
		if (m->extendedflags & EF_DLIGHT)
			fprintf(f, "\tbDLight = true;\n");
		if (m->extendedflags & EF_GIBBED)
			fprintf(f, "\tbGibbed = true;\n");
#endif

/*  damage_t damage;

  // linked list of losing benefits, or NULL
  benefit_t *lose_benefits;
  
  // linked list of pickup benefits, or NULL
  benefit_t *pickup_benefits;*/

		if (m->pickup_message)
			fprintf(f, "\tPickupMessage = \"%s\";\n", DDF_LanguageLookup(m->pickup_message));

#if 0
/*  // linked list of initial benefits for players, or NULL if none
  benefit_t *initial_benefits;*/

		if (m->castorder)
			fprintf(f, "\tCastOrder = %d;\n", m->castorder);
		if (m->respawntime)
			fprintf(f, "\tRespawnTime = %1.1f;\n", m->respawntime / 35.0);
#endif
		if (m->translucency != 1.0)
			fprintf(f, "\tTranslucency = %d;\n", (int)((1.0 - m->translucency) * 100));
#if 0
		if (m->minatkchance)
			fprintf(f, "\tMinAtkChance = %1.1f;\n", m->minatkchance);
/*  const colourmap_t *palremap;

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
  percent_t shadow_trans;*/
#endif

		if (m->seesound)
			fprintf(f, "\tSightSound = '%s';\n", SFX(m->seesound));
		if (m->attacksound)
			fprintf(f, "\tAttackSound = '%s';\n", SFX(m->attacksound));
		if (m->painsound)
			fprintf(f, "\tPainSound = '%s';\n", SFX(m->painsound));
		if (m->deathsound)
			fprintf(f, "\tDeathSound = '%s';\n", SFX(m->deathsound));
#if 0
		if (m->overkill_sound)
			fprintf(f, "\tOverkillSound = '%s';\n", SFX(m->overkill_sound));
#endif
		if (m->activesound)
			fprintf(f, "\tActiveSound = '%s';\n", SFX(m->activesound));
#if 0
		if (m->walksound)
			fprintf(f, "\tWalkSound = '%s';\n", SFX(m->walksound));
		if (m->jump_sound)
			fprintf(f, "\tJumpSound = '%s';\n", SFX(m->jump_sound));
		if (m->noway_sound)
			fprintf(f, "\tNoWaySound = '%s';\n", SFX(m->noway_sound));
		if (m->oof_sound)
			fprintf(f, "\tOofSound = '%s';\n", SFX(m->oof_sound));
		if (m->gasp_sound)
			fprintf(f, "\tGaspSound = '%s';\n", SFX(m->gasp_sound));

/*  int fuse;
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
*/

		if (m->dropitem)
			fprintf(f, "\tDropItem = Thing%d; // %s\n", GetIndex(m->dropitem), m->dropitem_ref);

		if (m->blood && m->blood_ref)
			fprintf(f, "\tBloodType = Thing%d; // %s\n", GetIndex(m->blood), m->blood_ref);

		if (m->respawneffect && m->respawneffect_ref)
			fprintf(f, "\tRespawnEffect = Thing%d; // %s\n", GetIndex(m->respawneffect), m->respawneffect_ref);

		if (m->spitspot)
			fprintf(f, "\tSpitSpot = Thing%d; // %s\n", GetIndex(m->spitspot), m->spitspot_ref);

#endif
		fprintf(f, "}\n");
		fprintf(f, "\n}\n");
		fprintf(f, "\n");
	}
	fclose(f);
}

