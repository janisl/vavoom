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

class VAcsGlobal;

class VWorldInfo : public VObject
{
	DECLARE_CLASS(VWorldInfo, VObject, 0)

	VAcsGlobal*		Acs;

	vuint8			GameSkill;
	float			SkillAmmoFactor;
	float			SkillDoubleAmmoFactor;
	float			SkillDamageFactor;
	float			SkillRespawnTime;
	int				SkillRespawnLimit;
	float			SkillAggressiveness;
	int				SkillSpawnFilter;

	enum
	{
		WIF_SkillFastMonsters		= 0x00000001,
		WIF_SkillDisableCheats		= 0x00000002,
		WIF_SkillEasyBossBrain		= 0x00000004,
		WIF_SkillAutoUseHealth		= 0x00000008,
	};
	vuint32			Flags;

	VWorldInfo();

	void Serialise(VStream& Strm);
	void Destroy();

	void SetSkill(int);

	DECLARE_FUNCTION(SetSkill)
};
