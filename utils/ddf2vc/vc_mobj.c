#include "i_defs.h"

#include "ddf_main.h"
#include "vc.h"

#ifdef GetClassName
#undef GetClassName
#endif

//	Not included in headers.
extern int num_disabled_mobjinfo;

FILE *cur_file;

static const char *dlight_types[] =
{
	"DLITE_None",
	"DLITE_Constant",
	"DLITE_Linear",
	"DLITE_Quadratic"
};

static damage_t default_damage =
{
	0,      // nominal
	-1,     // linear_max
	-1,     // error
	0,      // delay time
	NULL_LABEL, NULL_LABEL, NULL_LABEL,  // override labels
	false   // no_armour
};
static damage_t default_choke_damage =
{
	6,      // nominal
	14,     // linear_max
	-1,     // error
	2 * 35,      // delay time
	NULL_LABEL, NULL_LABEL, NULL_LABEL,  // override labels
	true   // no_armour
};
static damage_t default_attack_damage =
{
	0,      // nominal
	-1,     // linear_max
	-1,     // error
	0,      // delay time
	NULL_LABEL, NULL_LABEL, NULL_LABEL,  // override labels
	false   // no_armour
};

static const char *GetClassName(const mobjinfo_t *m)
{
	static char name[256];
	int i;

	// Player
	if (m->playernum == 1)
	{
		return "DoomPlayer";
	}

	// Misc stuff
	if (!stricmp(m->ddf.name, "PUFF"))
	{
		return "Puff";
	}
	if (!stricmp(m->ddf.name, "BLOOD"))
	{
		return "Blood";
	}
	if (!stricmp(m->ddf.name, "TELEPORT FLASH"))
	{
		return "TeleportFog";
	}
	if (!stricmp(m->ddf.name, "RESPAWN FLASH"))
	{
		return "ItemRespawnFog";
	}

	// Boss brain stuff
	if (!stricmp(m->ddf.name, "BRAIN DEATH MISSILE"))
	{
		return "BrainDeathMissile";
	}
	if (!stricmp(m->ddf.name, "IMP"))
	{
		return "Imp";
	}
	if (!stricmp(m->ddf.name, "DEMON"))
	{
		return "Demon";
	}
	if (!stricmp(m->ddf.name, "SPECTRE"))
	{
		return "Shadows";
	}
	if (!stricmp(m->ddf.name, "PAIN ELEMENTAL"))
	{
		return "PainElemental";
	}
	if (!stricmp(m->ddf.name, "CACODEMON"))
	{
		return "Cacodemon";
	}
	if (!stricmp(m->ddf.name, "ARCHVILE"))
	{
		return "ArchVile";
	}
	if (!stricmp(m->ddf.name, "REVENANT"))
	{
		return "Revenant";
	}
	if (!stricmp(m->ddf.name, "ARACHNOTRON"))
	{
		return "Arachnotron";
	}
	if (!stricmp(m->ddf.name, "MANCUBUS"))
	{
		return "Mancubus";
	}
	if (!stricmp(m->ddf.name, "HELL KNIGHT"))
	{
		return "HellKnight";
	}
	if (!stricmp(m->ddf.name, "BARON OF HELL"))
	{
		return "BaronOfHell";
	}

	for (i = num_disabled_mobjinfo; i < num_mobjinfo; i++)
	{
		if (mobjinfo[i] == m)
			break;
	}
	sprintf(name, "Thing%d", i);
	return name;
}

const char *SFX(sfx_t *s)
{
	return S_sfx[s->sounds[0]]->ddf.name;
}

void PrintDamage(FILE *f, const char *name, const damage_t *d, const damage_t *def)
{
	if (d->nominal != def->nominal)
		fprintf(f, "\t%s = %d;\n", name, (int)d->nominal);
	if (d->linear_max != def->linear_max)
		fprintf(f, "\t%sMax = %d;\n", name, (int)d->linear_max);
	if (d->error != def->error)
		fprintf(f, "\t%sError = %d;\n", name, (int)d->error);
	if (d->delay!= def->delay)
		fprintf(f, "\t%sDelay = %1.1f;\n", name, d->delay / 35.0f);
/*  // override labels for various states, if the object being damaged
  // has such a state then it is used instead of the normal ones
  // (PAIN, DEATH, OVERKILL).  Defaults to NULL.
  struct label_offset_s
  {
    const char *label;
    int offset;
  }
  pain, death, overkill;
	if (d->)
		fprintf(f, "\t%s = ;\n", d->);
*/
	if (d->no_armour != def->no_armour)
		fprintf(f, "\t%sNoArmour = true;\n", name);
}

void PrintAttack(FILE *f, const attacktype_t *a, const char *Name)
{
	static const char *AttackTypes[] =
	{
		"ATK_Projectile",
		"ATK_Spawner",
		"ATK_TripleSpawner",
		"ATK_Spreader",
		"ATK_RandomSpread",
		"ATK_Shot",
		"ATK_Tracker",
		"ATK_CloseCombat",
		"ATK_ShootToSpot",
		"ATK_SkullFly",
		"ATK_SmartProjectile",
		"ATK_Spray",
	};
	char tmp[128];

	if (!a)
	{
		return;
	}
	fprintf(f, "\t// %s\n", a->ddf.name);
	fprintf(f, "\t%s.AttackStyle = %s;\n", Name, AttackTypes[a->attackstyle]);
	if (a->flags & AF_TraceSmoke)
		fprintf(f, "\t%s.bTraceSmoke = true;\n", Name);
	if (a->flags & AF_KillFailedSpawn)
		fprintf(f, "\t%s.bKillFailedSpawn = true;\n", Name);
	if (a->flags & AF_PrestepSpawn)
		fprintf(f, "\t%s.bPrestepSpawn = true;\n", Name);
	if (a->flags & AF_SpawnTelefrags)
		fprintf(f, "\t%s.bSpawnTelefrags = true;\n", Name);
	if (a->flags & AF_NeedSight)
		fprintf(f, "\t%s.bNeedSight = true;\n", Name);
	if (a->flags & AF_FaceTarget)
		fprintf(f, "\t%s.bFaceTarget = true;\n", Name);
	if (a->flags & AF_Player)
		fprintf(f, "\t%s.bPlayer = true;\n", Name);
	if (a->flags & AF_ForceAim)
		fprintf(f, "\t%s.bForceAim = true;\n", Name);
	if (a->flags & AF_AngledSpawn)
		fprintf(f, "\t%s.bAngledSpawn = true;\n", Name);
	if (a->flags & AF_NoTriggerLines)
		fprintf(f, "\t%s.bNoTriggerLines = true;\n", Name);
	if (a->initsound)
		fprintf(f, "\t%s.InitSound = \'%s\';\n", Name, SFX(a->initsound));
	if (a->sound)
		fprintf(f, "\t%s.Sound = \'%s\';\n", Name, SFX(a->sound));
	if (a->accuracy_slope)
		fprintf(f, "\t%s.AccuracySlope = %1.1f;\n", Name, a->accuracy_slope);
	if (a->accuracy_angle)
		fprintf(f, "\t%s.AccuracyAngle = %1.1f;\n", Name, (double)a->accuracy_angle * 90.0 / (double)ANG90);
	if (a->xoffset)
		fprintf(f, "\t%s.XOffset = %1.1f;\n", Name, a->xoffset);
	if (a->yoffset)
		fprintf(f, "\t%s.YOffset = %1.1f;\n", Name, a->yoffset);
	if (a->angle_offset)
		fprintf(f, "\t%s.AngleOffset = %1.1f;\n", Name, (double)a->angle_offset * 90.0 / (double)ANG90);
	if (a->slope_offset)
		fprintf(f, "\t%s.SlopeOffset = %1.1f;\n", Name, a->slope_offset);
	if (a->assault_speed)
		fprintf(f, "\t%s.AssaultSpeed = %1.1f;\n", Name, a->assault_speed);
	if (a->height)
		fprintf(f, "\t%s.Height = %1.1f;\n", Name, a->height);
	if (a->range)
		fprintf(f, "\t%s.Range = %1.1f;\n", Name, a->range);
	if (a->count)
		fprintf(f, "\t%s.Count = %d;\n", Name, a->count);
	if (a->tooclose)
		fprintf(f, "\t%s.TooClose = %d;\n", Name, a->tooclose);
	sprintf(tmp, "%s.Damage", Name);
	PrintDamage(f, tmp, &a->damage, &default_attack_damage);
	if (a->attack_class)
		fprintf(f, "\t%s.AttackClass = 0x%08x;\n", Name, a->attack_class);
	if (a->objinitstate)
		fprintf(f, "\t%s.ObjInitState = \'S_%d\';\t// %s\n", Name, a->objinitstate, a->objinitstate_ref);
	if (a->notracechance)
		fprintf(f, "\t%s.NoTraceChance = %1.2f;\n", Name, a->notracechance);
	if (a->keepfirechance)
		fprintf(f, "\t%s.KeepFireChance = %1.2f;\n", Name, a->keepfirechance);
	if (a->atk_mobj)
		fprintf(f, "\t%s.AtkMobj = %s;\n", Name, GetClassName(a->atk_mobj));
	if (a->spawnedobj)
		fprintf(f, "\t%s.SpawnedObj = %s;\t// %s\n", Name, GetClassName(a->spawnedobj), a->spawnedobj_ref);
	if (a->puff)
		fprintf(f, "\t%s.Puff = %s;\t// %s\n", Name, GetClassName(a->puff), a->puff_ref);
}

const char *GetKeyName(int mask)
{
	switch (mask)
	{
	case 0x0001: return "it_bluecard";
	case 0x0002: return "it_yellowcard";
	case 0x0004: return "it_redcard";
/*
  KF_GreenCard   = (1 << 3),
*/
	case 0x0010: return "it_blueskull";
	case 0x0020: return "it_yellowskull";
	case 0x0040: return "it_redskull";
/*
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
*/
	}
	return "";
}
const char *PowerNames[] = {
	"pw_invulnerability",
	"pw_strength",
	"pw_invisibility",
	"pw_ironfeet",
	"",
	"pw_infrared",

	"0",//PW_Jetpack
	"0",//PW_NightVision
	"0",//PW_Scuba
};

static void WritePickupBenefits(FILE *f, benefit_t *list)
{
	benefit_t *b;

	for (b = list; b; b = b->next)
	{
		switch (b->type)
		{
		case BENEFIT_None:
			break;
		case BENEFIT_Ammo:
			fprintf(f, "\tif (Toucher.GiveAmmo(%s, %d))\n", GetAmmoName(b->subtype), (int)b->amount);
			fprintf(f, "\t\tret = true;\n");
			break;
		case BENEFIT_AmmoLimit:
			fprintf(f, "\tif (Toucher.Player->MaxAmmo[%s] < %d)\n", GetAmmoName(b->subtype), (int)b->amount);
			fprintf(f, "\t{\n");
			fprintf(f, "\t\tToucher.Player->MaxAmmo[%s] = %d;\n", GetAmmoName(b->subtype), (int)b->amount);
			fprintf(f, "\t\tret = true;\n");
			fprintf(f, "\t}\n");
			break;
		case BENEFIT_Weapon:
			fprintf(f, "\tif (Toucher.GiveWeapon(%d, bDropped))\n", b->subtype - num_disabled_weapons);
			fprintf(f, "\t\tret = true;\n");
			break;
		case BENEFIT_Key:
			fprintf(f, "\tif (Toucher.GiveCard(%s))\n", GetKeyName(b->subtype));
			fprintf(f, "\t\tret = true;\n");
			break;
		case BENEFIT_Health:
			fprintf(f, "\tif (Toucher.GiveBody(%d, %d))\n", (int)b->amount, (int)b->limit);
			fprintf(f, "\t\tret = true;\n");
			break;
		case BENEFIT_Armour:
			fprintf(f, "\tif (Toucher.GiveArmor2(%d, %d, %d))\n", b->subtype + 1, (int)b->amount, (int)b->limit);
			fprintf(f, "\t\tret = true;\n");
			break;
		case BENEFIT_Powerup:
			if (b->subtype == PW_AllMap)
				fprintf(f, "\tif (Toucher.GiveItem(IT_ALL_MAP))\n");
			else
				fprintf(f, "\tif (Toucher.GivePower2(%s, %1.1f, %1.1f))\n", PowerNames[b->subtype], b->amount, b->limit);
			fprintf(f, "\t\tret = true;\n");
			break;
		}
	}
}
void WriteInitialBenefits(FILE *f, benefit_t *list)
{
	benefit_t *b;

	for (b = list; b; b = b->next)
	{
		switch (b->type)
		{
		case BENEFIT_None:
			break;
		case BENEFIT_Ammo:
			fprintf(f, "\tplayer->Ammo[%s] = %d;\n", GetAmmoName(b->subtype), (int)b->amount);
			break;
		case BENEFIT_AmmoLimit:
			fprintf(f, "\tplayer->MaxAmmo[%s] = %d;\n", GetAmmoName(b->subtype), (int)b->amount);
			break;
		case BENEFIT_Weapon:
			fprintf(f, "\tplayer->WeaponOwned[%d] = true;\n", b->subtype);
			break;
		case BENEFIT_Key:
			fprintf(f, "\tplayer->Keys |= 1 << %s\n", GetKeyName(b->subtype));
			break;
		default:
			fprintf(stderr, "Unsupported initial benefit\n");
		}
	}
}

void VC_WriteMobjs(void)
{
	int i;
	int j;
	FILE *f;
	char fname[256];

	sprintf(fname, "%s/things.vc", progsdir);
	I_Printf("Writing %s\n", fname);
	f = fopen(fname, "w");
	cur_file = f;
	fprintf(f, "// Forward declarations.\n");
	for (i = num_disabled_mobjinfo; i < num_mobjinfo; i++)
	{
		fprintf(f, "class %s;\n", GetClassName(mobjinfo[i]));
	}
	fprintf(f, "\n");
	for (i = num_disabled_mobjinfo; i < num_mobjinfo; i++)
	{
		mobjinfo_t *m = mobjinfo[i];

		fprintf(f, "//**************************************************************************\n");
		fprintf(f, "//\n");
		fprintf(f, "// %s\n", m->ddf.name);
		fprintf(f, "//\n");
		fprintf(f, "//**************************************************************************\n");
		if (m->playernum == 1)
			fprintf(f, "\nclass %s:PlayerPawn\n", GetClassName(m));
		else
			fprintf(f, "\nclass %s:Actor\n", GetClassName(m));
		if (m->ddf.number)
			fprintf(f, "\t__mobjinfo__(%d)\n", m->ddf.number);
		fprintf(f, "{\n\n");

		if (m->lose_benefits || m->pickup_benefits)
		{
			fprintf(f, "boolean HandlePickup(Actor Toucher)\n");
			fprintf(f, "{\n");
			fprintf(f, "\tboolean ret = false;\n");
			fprintf(f, "\n");
			//WritePickupBenefits(f, m->lose_benefits);
			WritePickupBenefits(f, m->pickup_benefits);
			fprintf(f, "\treturn ret;\n");
			fprintf(f, "}\n");
			fprintf(f, "\n");
		}

		if (m->first_state || m->last_state)
		{
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
		if (m->res_state)
			fprintf(f, "\tResState = S_%d;\n", m->res_state);
		if (m->meander_state)
			fprintf(f, "\tMeanderState = S_%d;\n", m->meander_state);
		if (m->bounce_state)
			fprintf(f, "\tBounceState = S_%d;\n", m->bounce_state);
		if (m->touch_state)
			fprintf(f, "\tTouchState = S_%d;\n", m->touch_state);
		if (m->jump_state)
			fprintf(f, "\tJumpState = S_%d;\n", m->jump_state);
		if (m->gib_state)
			fprintf(f, "\tGibState = S_%d;\n", m->gib_state);

		if (m->reactiontime)
			fprintf(f, "\tReactionCount = %d;\n", m->reactiontime);
		if (m->painchance)
			fprintf(f, "\tPainChance = %f;\n", m->painchance);
		if (m->spawnhealth != 1000)
		{
			fprintf(f, "\tHealth = %d;\n", (int)m->spawnhealth);
			fprintf(f, "\tGibsHealth = -%d;\n", (int)m->spawnhealth);
		}
		if (m->speed)
		{
			if (m->extendedflags & EF_MONSTER)
				fprintf(f, "\tStepSpeed = %1.1f;\n", m->speed);
			else
				fprintf(f, "\tSpeed = %1.1f;\n", m->speed * 35);
		}
		if (m->float_speed != 2.0)
			fprintf(f, "\tFloatSpeed = %1.1f;\n", m->float_speed);
		if (m->radius)
			fprintf(f, "\tRadius = %1.1f;\n", m->radius);
		if (m->height)
			fprintf(f, "\tHeight = %1.1f;\n", m->height);
		if (m->step_size != 24.0)
			fprintf(f, "\tMaxStepHeight = %1.1f;\n", m->step_size);
		if (m->mass != 100)
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
		if (m->extendedflags & EF_LESSVIS)
			fprintf(f, "\tbLessVis = true;\n");
		if (m->extendedflags & EF_NORESPAWN)
			fprintf(f, "\tbNoRespawn = true;\n");
		if (m->extendedflags & EF_NOGRAVKILL)
			fprintf(f, "\tbNoGravKill = true;\n");
		if (m->extendedflags & EF_DISLOYALTYPE)
			fprintf(f, "\tbDisloyalType = true;\n");
		if (m->extendedflags & EF_OWNATTACKHURTS)
			fprintf(f, "\tbOwnAttackHurts = true;\n");
		if (m->extendedflags & EF_FIRSTCHECK)
			fprintf(f, "\tbFirstCheck = true;\n");
		if (m->extendedflags & EF_NOTRACE)
			fprintf(f, "\tbNoTrace = true;\n");
		if (m->extendedflags & EF_TRIGGERHAPPY)
			fprintf(f, "\tbTriggerHappy = true;\n");
		if (m->extendedflags & EF_NEVERTARGET)
			fprintf(f, "\tbNeverTarget = true;\n");
		if (m->extendedflags & EF_NOGRUDGE)
			fprintf(f, "\tbNoGrudge = true;\n");
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
		if (m->extendedflags & EF_MONSTER)
			fprintf(f, "\tbMonster = true;\n");
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

		PrintDamage(f, "ExplodeDamage", &m->damage, &default_damage);
		if (m->pickup_message)
			fprintf(f, "\tPickupMessage = \"%s\";\n", DDF_LanguageLookup(m->pickup_message));

		if (m->castorder)
			fprintf(f, "\tCastOrder = %d;\n", m->castorder);
		if (m->respawntime != 30 * 35)
			fprintf(f, "\tRespawnTime = %1.1f;\n", m->respawntime / 35.0);
		if (m->translucency != 1.0)
			fprintf(f, "\tTranslucency = %d;\n", (int)((1.0 - m->translucency) * 100));
		if (m->minatkchance)
			fprintf(f, "\tMissileChance = %1.1f;\n", 256 - m->minatkchance * 256);
/*  const colourmap_t *palremap;*/

		if (m->jump_delay != 35)
			fprintf(f, "\tJumpDelay = %1.1f;\n", m->jump_delay / 35.0);
		if (m->jumpheight)
			fprintf(f, "\tJumpHeight = %1.1f;\n", m->jumpheight);
		if (m->crouchheight)
			fprintf(f, "\tCrouchHeight = %1.1f;\n", m->crouchheight);
		if (m->viewheight != 0.75)
			fprintf(f, "\tViewHeight = %1.2f;\n", m->viewheight);
		if (m->shotheight != 0.64f)
			fprintf(f, "\tShotHeight = %f;\n", m->shotheight);
		if (m->maxfall)
			fprintf(f, "\tMaxFall = %1.1f;\n", m->maxfall);
		if (m->fast != 1.0)
			fprintf(f, "\tFast = %1.1f;\n", m->fast);
		if (m->xscale != 1.0)
			fprintf(f, "\tXScale = %1.1f;\n", m->xscale);
		if (m->yscale != 1.0)
			fprintf(f, "\tYScale = %1.1f;\n", m->yscale);
		if (m->bounce_speed != 0.5)
			fprintf(f, "\tBounceSpeed = %1.1f;\n", m->bounce_speed);
		if (m->bounce_up != 0.5)
			fprintf(f, "\tBounceUp = %1.1f;\n", m->bounce_up);
		if (m->sight_slope != 16)
			fprintf(f, "\tSightSlope = %1.1f;\n", m->sight_slope);
		if (m->sight_angle != ANG90)
			fprintf(f, "\tSightAngle = %1.1f;\n", ((double)m->sight_angle) * 180.0 / (double)ANG180);
		if (m->ride_friction != 0.7f)
			fprintf(f, "\tRideFriction = %f;\n", m->ride_friction);
		if (m->shadow_trans != 0.5)
			fprintf(f, "\tShadowTrans = %1.2f;\n", m->shadow_trans);

		if (m->seesound)
			fprintf(f, "\tSightSound = '%s';\n", SFX(m->seesound));
		if (m->attacksound)
			fprintf(f, "\tAttackSound = '%s';\n", SFX(m->attacksound));
		if (m->painsound)
			fprintf(f, "\tPainSound = '%s';\n", SFX(m->painsound));
		if (m->deathsound)
			fprintf(f, "\tDeathSound = '%s';\n", SFX(m->deathsound));
		if (m->overkill_sound)
			fprintf(f, "\tOverkillSound = '%s';\n", SFX(m->overkill_sound));
		if (m->activesound)
			fprintf(f, "\tActiveSound = '%s';\n", SFX(m->activesound));
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

		if (m->fuse)
			fprintf(f, "\tFuse = %d;\n", m->fuse);
		if (m->side)
			fprintf(f, "\tSide = 0x%08x;\n", m->side);
		if (m->playernum)
			fprintf(f, "\tPlayerNum = %d;\n", m->playernum);

		if (m->lung_capacity != 700)
			fprintf(f, "\tLungCapacity = %1.1f;\n", m->lung_capacity / 35.0);
		if (m->gasp_start != 70)
			fprintf(f, "\tGaspStart = %1.1f;\n", m->gasp_start / 35.0);
		PrintDamage(f, "ChokeDamage", &m->choke_damage, &default_choke_damage);

		if (m->bobbing != 1.0)
			fprintf(f, "\tBobbing = %1.2f;\n", m->bobbing);

		if (m->immunity)
			fprintf(f, "\tImmunity = 0x%08x;\n", m->immunity);

		PrintAttack(f, m->closecombat, "CloseCombat");
		PrintAttack(f, m->rangeattack, "RangeAttack");
		PrintAttack(f, m->spareattack, "SpareAttack");

		if (m->halo.height != -1)
			fprintf(f, "\tHaloHeight = %1.1f;\n", m->halo.height);
		if (m->halo.size != 32)
			fprintf(f, "\tHaloSize = %1.1f;\n", m->halo.size);
		if (m->halo.minsize != -1)
			fprintf(f, "\tHaloMinSize = %1.1f;\n", m->halo.minsize);
		if (m->halo.maxsize != -1)
			fprintf(f, "\tHaloMaxSize = %1.1f;\n", m->halo.maxsize);
		if (m->halo.translucency != 0.5)
			fprintf(f, "\tHaloTranslucency = %1.2f;\n", m->halo.translucency);
		if (m->halo.colour != 0x00ffffff)
			fprintf(f, "\tHaloColor = 0x%08x;\n", m->halo.colour);
		if (m->halo.graphic[0])
			fprintf(f, "\tHaloGraphic = \"%s\";\n", m->halo.graphic);

		if (m->dlight.type)
			fprintf(f, "\tDLightType = %s;\n", dlight_types[m->dlight.type]);
		if (m->dlight.intensity != 32)
			fprintf(f, "\tDLightIntensity = %d;\n", m->dlight.intensity);
		if (m->dlight.colour != 0x00ffffff)
			fprintf(f, "\tDLightColor = 0x%08x;\n", m->dlight.colour);
		if (m->dlight.height != 0.5)
			fprintf(f, "\tDLightHeight = %1.2f;\n", m->dlight.height);

		if (m->dropitem)
			fprintf(f, "\tDropItem = %s; // %s\n", GetClassName(m->dropitem), m->dropitem_ref);

		if (m->blood && m->blood_ref)
			fprintf(f, "\tBloodType = %s; // %s\n", GetClassName(m->blood), m->blood_ref);

		if (m->respawneffect && m->respawneffect_ref)
			fprintf(f, "\tRespawnEffect = %s; // %s\n", GetClassName(m->respawneffect), m->respawneffect_ref);

		if (m->spitspot)
			fprintf(f, "\tSpitSpot = %s; // %s\n", GetClassName(m->spitspot), m->spitspot_ref);

		fprintf(f, "}\n");
		fprintf(f, "\n}\n");
		fprintf(f, "\n");
	}
	fclose(f);
}

