#include "i_defs.h"

#include "p_action.h"
#include "vc.h"

#define FUNC(name) \
	void name(mobj_t * object) \
	{ \
		fprintf(cur_file, "%s(); ", #name); \
		if (((state_t *)object)->action_par) \
			fprintf(cur_file, "/* PAR */ "); \
	}
#define START_FUNC(name) \
	void name(mobj_t * object) \
	{ \
		fprintf(cur_file, "%s(", #name);
#define END_FUNC \
		fprintf(cur_file, "); "); \
	}
//				if (s->action_par)
//					fprintf(f, "/* PAR */ ");

// Weapon Action Routine pointers
FUNC(A_Light0)
FUNC(A_Light1)
FUNC(A_Light2)

FUNC(A_WeaponReady)
FUNC(A_WeaponShoot)
FUNC(A_WeaponEject)
FUNC(A_WeaponJump)
FUNC(A_Lower)
FUNC(A_Raise)
FUNC(A_ReFire)
FUNC(A_NoFire)
FUNC(A_NoFireReturn)
FUNC(A_CheckReload)
FUNC(A_SFXWeapon1)
FUNC(A_SFXWeapon2)
FUNC(A_SFXWeapon3)
FUNC(A_WeaponPlaySound)
FUNC(A_WeaponKillSound)
FUNC(A_WeaponTransSet)
FUNC(A_WeaponTransFade)
FUNC(A_WeaponEnableRadTrig)
FUNC(A_WeaponDisableRadTrig)

FUNC(A_SetCrosshair)
FUNC(A_GotTarget)
FUNC(A_GunFlash)
FUNC(A_WeaponKick)

FUNC(A_WeaponShootSA)
FUNC(A_ReFireSA)
FUNC(A_NoFireSA)
FUNC(A_NoFireReturnSA)
FUNC(A_CheckReloadSA)
FUNC(A_GunFlashSA)

// Needed for the bossbrain.
FUNC(A_BrainScream)
FUNC(A_BrainDie)
FUNC(A_BrainSpit)
FUNC(A_CubeSpawn)
FUNC(A_BrainMissileExplode)

// Visibility Actions
FUNC(P_ActTransSet)
FUNC(P_ActTransFade)
FUNC(P_ActTransMore)
FUNC(P_ActTransLess)
FUNC(P_ActTransAlternate)

// Sound Actions
FUNC(P_ActPlaySound)
FUNC(P_ActKillSound)
FUNC(P_ActMakeAmbientSound)
FUNC(P_ActMakeAmbientSoundRandom)
FUNC(P_ActMakeCloseAttemptSound)
FUNC(P_ActMakeDyingSound)
FUNC(P_ActMakeOverKillSound)
FUNC(P_ActMakePainSound)
FUNC(P_ActMakeRangeAttemptSound)
FUNC(P_ActMakeActiveSound)

// Explosion Damage Actions
FUNC(P_ActDamageExplosion)
FUNC(P_ActThrust)

// Stand-by / Looking Actions
FUNC(P_ActStandardLook)
FUNC(P_ActPlayerSupportLook)

// Meander, aimless movement actions.
FUNC(P_ActStandardMeander)
FUNC(P_ActPlayerSupportMeander)

// Chasing Actions
FUNC(P_ActResurrectChase)
FUNC(P_ActStandardChase)
FUNC(P_ActWalkSoundChase)

// Attacking Actions
FUNC(P_ActComboAttack)
FUNC(P_ActMeleeAttack)
FUNC(P_ActRangeAttack)
FUNC(P_ActSpareAttack)
FUNC(P_ActRefireCheck)

// Miscellanous
FUNC(P_ActFaceTarget)
FUNC(P_ActMakeIntoCorpse)
FUNC(P_ActResetSpreadCount)
FUNC(P_ActExplode)
FUNC(P_ActActivateLineType)
FUNC(P_ActEnableRadTrig)
FUNC(P_ActDisableRadTrig)
FUNC(P_ActTouchyRearm)
FUNC(P_ActTouchyDisarm)
FUNC(P_ActBounceRearm)
FUNC(P_ActBounceDisarm)
FUNC(P_ActPathCheck)
FUNC(P_ActPathFollow)
FUNC(P_ActDropItem)
FUNC(P_ActDLightSet)
FUNC(P_ActDLightFade)
FUNC(P_ActDLightRandom)

// Movement actions
FUNC(P_ActFaceDir)
FUNC(P_ActTurnDir)
FUNC(P_ActTurnRandom)
FUNC(P_ActMlookFace)
FUNC(P_ActMlookTurn)
FUNC(P_ActMoveFwd)
FUNC(P_ActMoveRight)
FUNC(P_ActMoveUp)
FUNC(P_ActStopMoving)

// Projectiles
FUNC(P_ActFixedHomingProjectile)
FUNC(P_ActRandomHomingProjectile)
FUNC(P_ActLaunchOrderedSpread)
FUNC(P_ActLaunchRandomSpread)
FUNC(P_ActCreateSmokeTrail)
FUNC(P_ActHomeToSpot)
//boolean_t P_ActLookForTargets)

// Trackers
FUNC(P_ActEffectTracker)
FUNC(P_ActTrackerActive)
FUNC(P_ActTrackerFollow)
FUNC(P_ActTrackerStart)

// Blood and bullet puffs
FUNC(P_ActCheckBlood)
FUNC(P_ActCheckMoving)

FUNC(P_ActJump)
FUNC(A_PlayerScream)
