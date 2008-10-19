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

#include "vc_local.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	OLDDEC_Decoration,
	OLDDEC_Breakable,
	OLDDEC_Projectile,
	OLDDEC_Pickup,
};

enum
{
	BOUNCE_None,
	BOUNCE_Doom,
	BOUNCE_Heretic,
	BOUNCE_Hexen
};

enum
{
	PROP_Int,
	PROP_IntConst,
	PROP_IntUnsupported,
	PROP_BitIndex,
	PROP_Float,
	PROP_Speed,
	PROP_Tics,
	PROP_Percent,
	PROP_FloatClamped,
	PROP_FloatClamped2,
	PROP_FloatOpt2,
	PROP_Name,
	PROP_NameLower,
	PROP_Str,
	PROP_StrUnsupported,
	PROP_Class,
	PROP_BoolConst,
	PROP_State,
	PROP_Game,
	PROP_SpawnId,
	PROP_ConversationId,
	PROP_PainChance,
	PROP_DamageFactor,
	PROP_MissileDamage,
	PROP_VSpeed,
	PROP_RenderStyle,
	PROP_Translation,
	PROP_BloodColour,
	PROP_BloodType,
	PROP_StencilColour,
	PROP_Monster,
	PROP_Projectile,
	PROP_ClearFlags,
	PROP_DropItem,
	PROP_States,
	PROP_SkipSuper,
	PROP_Args,
	PROP_PickupMessage,
	PROP_LowMessage,
	PROP_PowerupColour,
	PROP_ColourRange,
	PROP_DamageScreenColour,
	PROP_HexenArmor,
	PROP_StartItem,
};

enum
{
	FLAG_Bool,
	FLAG_Unsupported,
	FLAG_Byte,
	FLAG_Float,
	FLAG_Name,
	FLAG_Class,
	FLAG_NoClip,
};

struct VClassFixup
{
	int			Offset;
	VStr		Name;
	VClass*		ReqParent;
	VClass*		Class;
};

struct VPropDef
{
	vuint8		Type;
	VName		Name;
	VName		PropName;
	VName		Prop2Name;
	union
	{
		int		IConst;
		float	FMin;
	};
	float		FMax;
	VStr		CPrefix;
};

struct VFlagDef
{
	vuint8		Type;
	VName		Name;
	VName		AltName;
	VName		PropName;
	union
	{
		vuint8	BTrue;
		float	FTrue;
	};
	VName		NTrue;
	union
	{
		vuint8	BFalse;
		float	FFalse;
	};
	VName		NFalse;
};

struct VFlagList
{
	TArray<VPropDef>	Props;
	TArray<VFlagDef>	Flags;
	VClass*				Class;
};

//==========================================================================
//
//	VDecorateInvocation
//
//==========================================================================

class VDecorateInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[VMethod::MAX_PARAMS + 1];

	VDecorateInvocation(VName, const TLocation&, int, VExpression**);
	~VDecorateInvocation();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static VExpression* ParseExpressionPriority13(VScriptParser* sc);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TArray<VLineSpecInfo>	LineSpecialInfos;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VPackage*		DecPkg;

static VClass*			ActorClass;
static VClass*			FakeInventoryClass;
static VClass*			InventoryClass;
static VClass*			AmmoClass;
static VClass*			BasicArmorPickupClass;
static VClass*			BasicArmorBonusClass;
static VClass*			HealthClass;
static VClass*			PowerupGiverClass;
static VClass*			PuzzleItemClass;
static VClass*			WeaponClass;
static VClass*			WeaponPieceClass;
static VClass*			PlayerPawnClass;
static VClass*			MorphProjectileClass;

static VMethod*			FuncA_Scream;
static VMethod*			FuncA_NoBlocking;
static VMethod*			FuncA_ScreamAndUnblock;
static VMethod*			FuncA_ActiveSound;
static VMethod*			FuncA_ActiveAndUnblock;
static VMethod*			FuncA_ExplodeParms;
static VMethod*			FuncA_FreezeDeath;
static VMethod*			FuncA_FreezeDeathChunks;

static TArray<VFlagList>	FlagList;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	ParseDecorateDef
//
//==========================================================================

static void ParseDecorateDef(VXmlDocument& Doc)
{
	guard(ParseDecorateDef);
	for (VXmlNode* N = Doc.Root.FindChild("class"); N; N = N->FindNext())
	{
		VStr ClassName = N->GetAttribute("name");
		VFlagList& Lst = FlagList.Alloc();
		Lst.Class = VClass::FindClass(*ClassName);
		for (VXmlNode* PN = N->FirstChild; PN; PN = PN->NextSibling)
		{
			if (PN->Name == "prop_int")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Int;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_int_const")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_IntConst;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
				P.IConst = atoi(*PN->GetAttribute("value"));
			}
			else if (PN->Name == "prop_int_unsupported")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_IntUnsupported;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_bit_index")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_BitIndex;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_float")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Float;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_speed")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Speed;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_tics")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Tics;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_percent")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Percent;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_float_clamped")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_FloatClamped;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
				P.FMin = atof(*PN->GetAttribute("min"));
				P.FMax = atof(*PN->GetAttribute("max"));
			}
			else if (PN->Name == "prop_float_clamped_2")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_FloatClamped2;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
				P.Prop2Name = *PN->GetAttribute("property2");
				P.FMin = atof(*PN->GetAttribute("min"));
				P.FMax = atof(*PN->GetAttribute("max"));
			}
			else if (PN->Name == "prop_float_optional_2")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_FloatOpt2;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
				P.Prop2Name = *PN->GetAttribute("property2");
			}
			else if (PN->Name == "prop_name")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Name;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_name_lower")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_NameLower;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_string")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Str;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_string_unsupported")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_StrUnsupported;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_class")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Class;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
				if (PN->HasAttribute("prefix"))
				{
					P.CPrefix = PN->GetAttribute("prefix");
				}
			}
			else if (PN->Name == "prop_bool_const")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_BoolConst;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
				P.IConst = !PN->GetAttribute("value").ICmp("true");
			}
			else if (PN->Name == "prop_state")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_State;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_game")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Game;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_spawn_id")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_SpawnId;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_conversation_id")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_ConversationId;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_pain_chance")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_PainChance;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_damage_factor")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_DamageFactor;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_missile_damage")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_MissileDamage;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_vspeed")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_VSpeed;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_render_style")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_RenderStyle;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_translation")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Translation;
				P.Name = *PN->GetAttribute("name").ToLower();
				P.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "prop_blood_colour")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_BloodColour;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_blood_type")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_BloodType;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_stencil_colour")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_StencilColour;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_monster")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Monster;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_projectile")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Projectile;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_clear_flags")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_ClearFlags;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_drop_item")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_DropItem;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_states")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_States;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_skip_super")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_SkipSuper;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_args")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_Args;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_pickup_message")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_PickupMessage;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_low_message")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_LowMessage;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_powerup_colour")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_PowerupColour;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_colour_range")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_ColourRange;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_damage_screen_colour")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_DamageScreenColour;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_hexen_armor")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_HexenArmor;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "prop_start_item")
			{
				VPropDef& P = Lst.Props.Alloc();
				P.Type = PROP_StartItem;
				P.Name = *PN->GetAttribute("name").ToLower();
			}
			else if (PN->Name == "flag")
			{
				VFlagDef& F = Lst.Flags.Alloc();
				F.Type = FLAG_Bool;
				F.Name = *PN->GetAttribute("name").ToLower();
				F.AltName = *(ClassName.ToLower() + "." + F.Name);
				F.PropName = *PN->GetAttribute("property");
			}
			else if (PN->Name == "flag_unsupported")
			{
				VFlagDef& F = Lst.Flags.Alloc();
				F.Type = FLAG_Unsupported;
				F.Name = *PN->GetAttribute("name").ToLower();
				F.AltName = *(ClassName.ToLower() + "." + F.Name);
			}
			else if (PN->Name == "flag_byte")
			{
				VFlagDef& F = Lst.Flags.Alloc();
				F.Type = FLAG_Byte;
				F.Name = *PN->GetAttribute("name").ToLower();
				F.AltName = *(ClassName.ToLower() + "." + F.Name);
				F.PropName = *PN->GetAttribute("property");
				F.BTrue = atoi(*PN->GetAttribute("true_value"));
				F.BFalse = atoi(*PN->GetAttribute("false_value"));
			}
			else if (PN->Name == "flag_float")
			{
				VFlagDef& F = Lst.Flags.Alloc();
				F.Type = FLAG_Float;
				F.Name = *PN->GetAttribute("name").ToLower();
				F.AltName = *(ClassName.ToLower() + "." + F.Name);
				F.PropName = *PN->GetAttribute("property");
				F.FTrue = atof(*PN->GetAttribute("true_value"));
				F.FFalse = atof(*PN->GetAttribute("false_value"));
			}
			else if (PN->Name == "flag_name")
			{
				VFlagDef& F = Lst.Flags.Alloc();
				F.Type = FLAG_Name;
				F.Name = *PN->GetAttribute("name").ToLower();
				F.AltName = *(ClassName.ToLower() + "." + F.Name);
				F.PropName = *PN->GetAttribute("property");
				F.NTrue = *PN->GetAttribute("true_value");
				F.NFalse = *PN->GetAttribute("false_value");
			}
			else if (PN->Name == "flag_class")
			{
				VFlagDef& F = Lst.Flags.Alloc();
				F.Type = FLAG_Class;
				F.Name = *PN->GetAttribute("name").ToLower();
				F.AltName = *(ClassName.ToLower() + "." + F.Name);
				F.PropName = *PN->GetAttribute("property");
				F.NTrue = *PN->GetAttribute("true_value");
				F.NFalse = *PN->GetAttribute("false_value");
			}
			else if (PN->Name == "flag_noclip")
			{
				VFlagDef& F = Lst.Flags.Alloc();
				F.Type = FLAG_NoClip;
				F.Name = *PN->GetAttribute("name").ToLower();
				F.AltName = *(ClassName.ToLower() + "." + F.Name);
			}
			else
			{
				Sys_Error("Unknown XML node %s", *PN->Name);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VDecorateSingleName::VDecorateSingleName
//
//==========================================================================

VDecorateSingleName::VDecorateSingleName(const VStr& AName,
	const TLocation& ALoc)
: VExpression(ALoc)
, Name(AName)
{
}

//==========================================================================
//
//	VDecorateSingleName::DoResolve
//
//==========================================================================

VExpression* VDecorateSingleName::DoResolve(VEmitContext& ec)
{
	guard(VDecorateSingleName::DoResolve);
	VName CheckName = va("decorate_%s", *Name.ToLower());
	if (ec.SelfClass)
	{
		VConstant* Const = ec.SelfClass->FindConstant(CheckName);
		if (Const)
		{
			VExpression* e = new VConstantValue(Const, Loc);
			delete this;
			return e->Resolve(ec);
		}

		VProperty* Prop = ec.SelfClass->FindProperty(CheckName);
		if (Prop)
		{
			if (!Prop->GetFunc)
			{
				ParseError(Loc, "Property %s cannot be read", *Name);
				delete this;
				return NULL;
			}
			VExpression* e = new VInvocation(NULL, Prop->GetFunc, NULL,
				false, false, Loc, 0, NULL);
			delete this;
			return e->Resolve(ec);
		}
	}

	CheckName = *Name.ToLower();
	//	Look only for constants defined in DECORATE scripts.
	VConstant* Const = ec.Package->FindConstant(CheckName);
	if (Const)
	{
		VExpression* e = new VConstantValue(Const, Loc);
		delete this;
		return e->Resolve(ec);
	}

	ParseError(Loc, "Illegal expression identifier %s", *Name);
	delete this;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VDecorateSingleName::Emit
//
//==========================================================================

void VDecorateSingleName::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//==========================================================================
//
//	VDecorateSingleName::IsDecorateSingleName
//
//==========================================================================

bool VDecorateSingleName::IsDecorateSingleName() const
{
	return true;
}

//==========================================================================
//
//	VDecorateInvocation::VDecorateInvocation
//
//==========================================================================

VDecorateInvocation::VDecorateInvocation(VName AName, const TLocation& ALoc, int ANumArgs,
	VExpression** AArgs)
: VExpression(ALoc)
, Name(AName)
, NumArgs(ANumArgs)
{
	for (int i = 0; i < NumArgs; i++)
		Args[i] = AArgs[i];
}

//==========================================================================
//
//	VDecorateInvocation::~VDecorateInvocation
//
//==========================================================================

VDecorateInvocation::~VDecorateInvocation()
{
	for (int i = 0; i < NumArgs; i++)
		if (Args[i])
			delete Args[i];
}

//==========================================================================
//
//	VDecorateInvocation::DoResolve
//
//==========================================================================

VExpression* VDecorateInvocation::DoResolve(VEmitContext& ec)
{
	guard(VDecorateInvocation::DoResolve);
	if (ec.SelfClass)
	{
		//	First try with decorate_ prefix, then without.
		VMethod* M = ec.SelfClass->FindMethod(va("decorate_%s", *Name));
		if (!M)
		{
			M = ec.SelfClass->FindMethod(Name);
		}
		if (M)
		{
			if (M->Flags & FUNC_Iterator)
			{
				ParseError(Loc, "Iterator methods can only be used in foreach statements");
				delete this;
				return NULL;
			}
			VExpression* e = new VInvocation(NULL, M, NULL,
				false, false, Loc, NumArgs, Args);
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}
	}

	ParseError(Loc, "Unknown method %s", *Name);
	delete this;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VDecorateInvocation::Emit
//
//==========================================================================

void VDecorateInvocation::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//==========================================================================
//
//	GetClassFieldFloat
//
//==========================================================================

static float GetClassFieldFloat(VClass* Class, VName FieldName)
{
	guard(GetClassFieldFloat);
	VField* F = Class->FindFieldChecked(FieldName);
	float* Ptr = (float*)(Class->Defaults + F->Ofs);
	return *Ptr;
	unguard;
}

//==========================================================================
//
//	GetClassFieldVec
//
//==========================================================================

static TVec GetClassFieldVec(VClass* Class, VName FieldName)
{
	guard(GetClassFieldVec);
	VField* F = Class->FindFieldChecked(FieldName);
	TVec* Ptr = (TVec*)(Class->Defaults + F->Ofs);
	return *Ptr;
	unguard;
}

//==========================================================================
//
//	GetClassDropItems
//
//==========================================================================

static TArray<VDropItemInfo>& GetClassDropItems(VClass* Class)
{
	guard(GetClassDropItems);
	VField* F = Class->FindFieldChecked("DropItemList");
	return *(TArray<VDropItemInfo>*)(Class->Defaults + F->Ofs);
	unguard;
}

//==========================================================================
//
//	GetClassDamageFactors
//
//==========================================================================

static TArray<VDamageFactor>& GetClassDamageFactors(VClass* Class)
{
	guard(GetClassDamageFactors);
	VField* F = Class->FindFieldChecked("DamageFactors");
	return *(TArray<VDamageFactor>*)(Class->Defaults + F->Ofs);
	unguard;
}

//==========================================================================
//
//	GetClassPainChances
//
//==========================================================================

static TArray<VPainChanceInfo>& GetClassPainChances(VClass* Class)
{
	guard(GetClassPainChances);
	VField* F = Class->FindFieldChecked("PainChances");
	return *(TArray<VPainChanceInfo>*)(Class->Defaults + F->Ofs);
	unguard;
}

//==========================================================================
//
//	SetClassFieldInt
//
//==========================================================================

static void SetClassFieldInt(VClass* Class, VName FieldName, int Value,
	int Idx = 0)
{
	guard(SetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	vint32* Ptr = (vint32*)(Class->Defaults + F->Ofs);
	Ptr[Idx] = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldByte
//
//==========================================================================

static void SetClassFieldByte(VClass* Class, VName FieldName, vuint8 Value)
{
	guard(SetClassFieldByte);
	VField* F = Class->FindFieldChecked(FieldName);
	vuint8* Ptr = Class->Defaults + F->Ofs;
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldFloat
//
//==========================================================================

static void SetClassFieldFloat(VClass* Class, VName FieldName, float Value,
	int Idx = 0)
{
	guard(SetClassFieldFloat);
	VField* F = Class->FindFieldChecked(FieldName);
	float* Ptr = (float*)(Class->Defaults + F->Ofs);
	Ptr[Idx] = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldBool
//
//==========================================================================

static void SetClassFieldBool(VClass* Class, VName FieldName, int Value)
{
	guard(SetClassFieldBool);
	VField* F = Class->FindFieldChecked(FieldName);
	vuint32* Ptr = (vuint32*)(Class->Defaults + F->Ofs);
	if (Value)
		*Ptr |= F->Type.BitMask;
	else
		*Ptr &= ~F->Type.BitMask;
	unguard;
}

//==========================================================================
//
//	SetClassFieldName
//
//==========================================================================

static void SetClassFieldName(VClass* Class, VName FieldName, VName Value)
{
	guard(SetClassFieldName);
	VField* F = Class->FindFieldChecked(FieldName);
	VName* Ptr = (VName*)(Class->Defaults + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldStr
//
//==========================================================================

static void SetClassFieldStr(VClass* Class, VName FieldName,
	const VStr& Value)
{
	guard(SetClassFieldStr);
	VField* F = Class->FindFieldChecked(FieldName);
	VStr* Ptr = (VStr*)(Class->Defaults + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldVec
//
//==========================================================================

static void SetClassFieldVec(VClass* Class, VName FieldName,
	const TVec& Value)
{
	guard(SetClassFieldVec);
	VField* F = Class->FindFieldChecked(FieldName);
	TVec* Ptr = (TVec*)(Class->Defaults + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetFieldByte
//
//==========================================================================

static void SetFieldByte(VObject* Obj, VName FieldName, vuint8 Value)
{
	guard(SetFieldByte);
	VField* F = Obj->GetClass()->FindFieldChecked(FieldName);
	vuint8* Ptr = (vuint8*)Obj + F->Ofs;
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetFieldFloat
//
//==========================================================================

static void SetFieldFloat(VObject* Obj, VName FieldName, float Value,
	int Idx = 0)
{
	guard(SetFieldFloat);
	VField* F = Obj->GetClass()->FindFieldChecked(FieldName);
	float* Ptr = (float*)((vuint8*)Obj + F->Ofs);
	Ptr[Idx] = Value;
	unguard;
}

//==========================================================================
//
//	SetFieldBool
//
//==========================================================================

static void SetFieldBool(VObject* Obj, VName FieldName, int Value)
{
	guard(SetFieldBool);
	VField* F = Obj->GetClass()->FindFieldChecked(FieldName);
	vuint32* Ptr = (vuint32*)((vuint8*)Obj + F->Ofs);
	if (Value)
		*Ptr |= F->Type.BitMask;
	else
		*Ptr &= ~F->Type.BitMask;
	unguard;
}

//==========================================================================
//
//	SetFieldName
//
//==========================================================================

static void SetFieldName(VObject* Obj, VName FieldName, VName Value)
{
	guard(SetFieldName);
	VField* F = Obj->GetClass()->FindFieldChecked(FieldName);
	VName* Ptr = (VName*)((vuint8*) + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetFieldClass
//
//==========================================================================

static void SetFieldClass(VObject* Obj, VName FieldName, VClass* Value)
{
	guard(SetFieldClass);
	VField* F = Obj->GetClass()->FindFieldChecked(FieldName);
	VClass** Ptr = (VClass**)((vuint8*) + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	AddClassFixup
//
//==========================================================================

static void AddClassFixup(VClass* Class, VName FieldName,
	const VStr& ClassName, TArray<VClassFixup>& ClassFixups)
{
	guard(AddClassFixup);
	VField* F = Class->FindFieldChecked(FieldName);
	VClassFixup& CF = ClassFixups.Alloc();
	CF.Offset = F->Ofs;
	CF.Name = ClassName;
	CF.ReqParent = F->Type.Class;
	CF.Class = Class;
	unguard;
}

//==========================================================================
//
//	SkipBlock
//
//==========================================================================

static void SkipBlock(VScriptParser* sc, int Level)
{
	while (!sc->AtEnd() && Level > 0)
	{
		if (sc->Check("{"))
		{
			Level++;
		}
		else if (sc->Check("}"))
		{
			Level--;
		}
		else
		{
			sc->GetString();
		}
	}
}

//==========================================================================
//
//	ParseMethodCall
//
//==========================================================================

static VExpression* ParseMethodCall(VScriptParser* sc, VName Name,
	TLocation Loc)
{
	guard(ParseMethodCall);
	VExpression* Args[VMethod::MAX_PARAMS + 1];
	int NumArgs = 0;
	if (!sc->Check(")"))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13(sc);
			if (NumArgs == VMethod::MAX_PARAMS)
				ParseError(sc->GetLoc(), "Too many arguments");
			else
				NumArgs++;
		} while (sc->Check(","));
		sc->Expect(")");
	}
	return new VDecorateInvocation(Name, Loc, NumArgs, Args);
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority0
//
//==========================================================================

static VExpression* ParseExpressionPriority0(VScriptParser* sc)
{
	guard(ParseExpressionPriority0);
	TLocation l = sc->GetLoc();
	if (sc->CheckNumber())
	{
		vint32 Val = sc->Number;
		return new VIntLiteral(Val, l);
	}

	if (sc->CheckFloat())
	{
		float Val = sc->Float;
		return new VFloatLiteral(Val, l);
	}

	if (sc->CheckQuotedString())
	{
		int Val = DecPkg->FindString(*sc->String);
		return new VStringLiteral(Val, l);
	}

	if (sc->Check("false"))
	{
		return new VIntLiteral(0, l);
	}

	if (sc->Check("true"))
	{
		return new VIntLiteral(1, l);
	}

	if (sc->Check("("))
	{
		VExpression* op = ParseExpressionPriority13(sc);
		if (!op)
		{
			ParseError(l, "Expression expected");
		}
		sc->Expect(")");
		return op;
	}

	if (sc->CheckIdentifier())
	{
		VStr Name = sc->String;
		//	Skip random generator ID.
		if ((!Name.ICmp("random") || !Name.ICmp("random2")) && sc->Check("["))
		{
			sc->ExpectString();
			sc->Expect("]");
		}
		if (sc->Check("("))
		{
			return ParseMethodCall(sc, *Name.ToLower(), l);
		}
		return new VDecorateSingleName(Name, l);
	}

	return NULL;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority1
//
//==========================================================================

static VExpression* ParseExpressionPriority1(VScriptParser* sc)
{
	guard(ParseExpressionPriority1);
	return ParseExpressionPriority0(sc);
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority2
//
//==========================================================================

static VExpression* ParseExpressionPriority2(VScriptParser* sc)
{
	guard(ParseExpressionPriority2);
	VExpression*	op;

	TLocation l = sc->GetLoc();

	if (sc->Check("+"))
	{
		op = ParseExpressionPriority2(sc);
		return new VUnary(VUnary::Plus, op, l);
	}

	if (sc->Check("-"))
	{
		op = ParseExpressionPriority2(sc);
		return new VUnary(VUnary::Minus, op, l);
	}

	if (sc->Check("!"))
	{
		op = ParseExpressionPriority2(sc);
		return new VUnary(VUnary::Not, op, l);
	}

	if (sc->Check("~"))
	{
		op = ParseExpressionPriority2(sc);
		return new VUnary(VUnary::BitInvert, op, l);
	}

	return ParseExpressionPriority1(sc);
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority3
//
//==========================================================================

static VExpression* ParseExpressionPriority3(VScriptParser* sc)
{
	guard(ParseExpressionPriority3);
	VExpression* op1 = ParseExpressionPriority2(sc);
	if (!op1)
	{
		return NULL;
	}
	bool done = false;
	do
	{
		TLocation l = sc->GetLoc();
		if (sc->Check("*"))
		{
			VExpression* op2 = ParseExpressionPriority2(sc);
			op1 = new VBinary(VBinary::Multiply, op1, op2, l);
		}
		else if (sc->Check("/"))
		{
			VExpression* op2 = ParseExpressionPriority2(sc);
			op1 = new VBinary(VBinary::Divide, op1, op2, l);
		}
		else if (sc->Check("%"))
		{
			VExpression* op2 = ParseExpressionPriority2(sc);
			op1 = new VBinary(VBinary::Modulus, op1, op2, l);
		}
		else
		{
			done = true;
		}
	}
	while (!done);
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority4
//
//==========================================================================

static VExpression* ParseExpressionPriority4(VScriptParser* sc)
{
	guard(ParseExpressionPriority4);
	VExpression* op1 = ParseExpressionPriority3(sc);
	if (!op1)
	{
		return NULL;
	}
	bool done = false;
	do
	{
		TLocation l = sc->GetLoc();
		if (sc->Check("+"))
		{
			VExpression* op2 = ParseExpressionPriority3(sc);
			op1 = new VBinary(VBinary::Add, op1, op2, l);
		}
		else if (sc->Check("-"))
		{
			VExpression* op2 = ParseExpressionPriority3(sc);
			op1 = new VBinary(VBinary::Subtract, op1, op2, l);
		}
		else
		{
			done = true;
		}
	}
	while (!done);
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority5
//
//==========================================================================

static VExpression* ParseExpressionPriority5(VScriptParser* sc)
{
	guard(ParseExpressionPriority5);
	VExpression* op1 = ParseExpressionPriority4(sc);
	if (!op1)
	{
		return NULL;
	}
	bool done = false;
	do
	{
		TLocation l = sc->GetLoc();
		if (sc->Check("<<"))
		{
			VExpression* op2 = ParseExpressionPriority4(sc);
			op1 = new VBinary(VBinary::LShift, op1, op2, l);
		}
		else if (sc->Check(">>"))
		{
			VExpression* op2 = ParseExpressionPriority4(sc);
			op1 = new VBinary(VBinary::RShift, op1, op2, l);
		}
		else
		{
			done = true;
		}
	}
	while (!done);
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority6
//
//==========================================================================

static VExpression* ParseExpressionPriority6(VScriptParser* sc)
{
	guard(ParseExpressionPriority6);
	VExpression* op1 = ParseExpressionPriority5(sc);
	if (!op1)
	{
		return NULL;
	}
	bool done = false;
	do
	{
		TLocation l = sc->GetLoc();
		if (sc->Check("<"))
		{
			VExpression* op2 = ParseExpressionPriority5(sc);
			op1 = new VBinary(VBinary::Less, op1, op2, l);
		}
		else if (sc->Check("<="))
		{
			VExpression* op2 = ParseExpressionPriority5(sc);
			op1 = new VBinary(VBinary::LessEquals, op1, op2, l);
		}
		else if (sc->Check(">"))
		{
			VExpression* op2 = ParseExpressionPriority5(sc);
			op1 = new VBinary(VBinary::Greater, op1, op2, l);
		}
		else if (sc->Check(">="))
		{
			VExpression* op2 = ParseExpressionPriority5(sc);
			op1 = new VBinary(VBinary::GreaterEquals, op1, op2, l);
		}
		else
		{
			done = true;
		}
	}
	while (!done);
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority7
//
//==========================================================================

static VExpression* ParseExpressionPriority7(VScriptParser* sc)
{
	guard(ParseExpressionPriority7);
	VExpression* op1 = ParseExpressionPriority6(sc);
	if (!op1)
	{
		return NULL;
	}
	bool done = false;
	do
	{
		TLocation l = sc->GetLoc();
		if (sc->Check("=="))
		{
			VExpression* op2 = ParseExpressionPriority6(sc);
			op1 = new VBinary(VBinary::Equals, op1, op2, l);
		}
		else if (sc->Check("!="))
		{
			VExpression* op2 = ParseExpressionPriority6(sc);
			op1 = new VBinary(VBinary::NotEquals, op1, op2, l);
		}
		else
		{
			done = true;
		}
	} while (!done);
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority8
//
//==========================================================================

static VExpression* ParseExpressionPriority8(VScriptParser* sc)
{
	guard(ParseExpressionPriority8);
	VExpression* op1 = ParseExpressionPriority7(sc);
	if (!op1)
	{
		return NULL;
	}
	TLocation l = sc->GetLoc();
	while (sc->Check("&"))
	{
		VExpression* op2 = ParseExpressionPriority7(sc);
		op1 = new VBinary(VBinary::And, op1, op2, l);
		l = sc->GetLoc();
	}
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority9
//
//==========================================================================

static VExpression* ParseExpressionPriority9(VScriptParser* sc)
{
	guard(ParseExpressionPriority9);
	VExpression* op1 = ParseExpressionPriority8(sc);
	if (!op1)
	{
		return NULL;
	}
	TLocation l = sc->GetLoc();
	while (sc->Check("^"))
	{
		VExpression* op2 = ParseExpressionPriority8(sc);
		op1 = new VBinary(VBinary::XOr, op1, op2, l);
		l = sc->GetLoc();
	}
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority10
//
//==========================================================================

static VExpression* ParseExpressionPriority10(VScriptParser* sc)
{
	guard(ParseExpressionPriority10);
	VExpression* op1 = ParseExpressionPriority9(sc);
	if (!op1)
	{
		return NULL;
	}
	TLocation l = sc->GetLoc();
	while (sc->Check("|"))
	{
		VExpression* op2 = ParseExpressionPriority9(sc);
		op1 = new VBinary(VBinary::Or, op1, op2, l);
		l = sc->GetLoc();
	}
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority11
//
//==========================================================================

static VExpression* ParseExpressionPriority11(VScriptParser* sc)
{
	guard(ParseExpressionPriority11);
	VExpression* op1 = ParseExpressionPriority10(sc);
	if (!op1)
	{
		return NULL;
	}
	TLocation l = sc->GetLoc();
	while (sc->Check("&&"))
	{
		VExpression* op2 = ParseExpressionPriority10(sc);
		op1 = new VBinaryLogical(VBinaryLogical::And, op1, op2, l);
		l = sc->GetLoc();
	}
	return op1;
	unguard;
}

//==========================================================================
//
//	ParseExpressionPriority12
//
//==========================================================================

static VExpression* ParseExpressionPriority12(VScriptParser* sc)
{
	guard(ParseExpressionPriority12);
	VExpression* op1 = ParseExpressionPriority11(sc);
	if (!op1)
	{
		return NULL;
	}
	TLocation l = sc->GetLoc();
	while (sc->Check("||"))
	{
		VExpression* op2 = ParseExpressionPriority11(sc);
		op1 = new VBinaryLogical(VBinaryLogical::Or, op1, op2, l);
		l = sc->GetLoc();
	}
	return op1;
	unguard;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority13
//
//==========================================================================

static VExpression* ParseExpressionPriority13(VScriptParser* sc)
{
	guard(ParseExpressionPriority13);
	VExpression* op = ParseExpressionPriority12(sc);
	if (!op)
	{
		return NULL;
	}
	TLocation l = sc->GetLoc();
	if (sc->Check("?"))
	{
		VExpression* op1 = ParseExpressionPriority13(sc);
		sc->Expect(":");
		VExpression* op2 = ParseExpressionPriority13(sc);
		op = new VConditional(op, op1, op2, l);
	}
	return op;
	unguard;
}

//==========================================================================
//
//	ParseExpression
//
//==========================================================================

static VExpression* ParseExpression(VScriptParser* sc)
{
	guard(ParseExpression);
	return ParseExpressionPriority13(sc);
	unguard;
}

//==========================================================================
//
//	ParseConst
//
//==========================================================================

static void ParseConst(VScriptParser* sc)
{
	guard(ParseConst);
	sc->SetCMode(true);
	sc->Expect("int");
	sc->ExpectString();
	TLocation Loc = sc->GetLoc();
	VStr Name = sc->String.ToLower();
	sc->Expect("=");

	VExpression* Expr = ParseExpression(sc);
	if (!Expr)
	{
		sc->Error("Constant value expected");
	}
	else
	{
		VEmitContext ec(DecPkg);
		Expr = Expr->Resolve(ec);
		if (Expr)
		{
			int Val = Expr->GetIntConst();
			delete Expr;
			VConstant* C = new VConstant(*Name, DecPkg, Loc);
			C->Type = TYPE_Int;
			C->Value = Val;
		}
	}
	sc->Expect(";");
	sc->SetCMode(false);
	unguard;
}

//==========================================================================
//
//	ParseAction
//
//==========================================================================

static void ParseAction(VScriptParser* sc, VClass* Class)
{
	guard(ParseAction);
	sc->Expect("native");
	//	Find the method. First try with decorate_ prefix, then without.
	sc->ExpectIdentifier();
	VMethod* M = Class->FindMethod(va("decorate_%s", *sc->String));
	if (!M)
	{
		M = Class->FindMethod(*sc->String);
	}
	if (!M)
	{
		sc->Error(va("Method %s not found in class %s", *sc->String,
			Class->GetName()));
	}
	if (M->ReturnType.Type != TYPE_Void)
	{
		sc->Error(va("State action %s doesn't return void", *sc->String));
	}
	VDecorateStateAction& A = Class->DecorateStateActions.Alloc();
	A.Name = *sc->String.ToLower();
	A.Method = M;
	//	Skip arguments, right now I don't care bout them.
	sc->Expect("(");
	while (!sc->Check(")"))
	{
		sc->ExpectString();
	}
	sc->Expect(";");
	unguard;
}

//==========================================================================
//
//	ParseClass
//
//==========================================================================

static void ParseClass(VScriptParser* sc)
{
	guard(ParseClass);
	sc->SetCMode(true);
	//	Get class name and find the class.
	sc->ExpectString();
	VClass* Class = VClass::FindClass(*sc->String);
	if (!Class)
	{
		sc->Error("Class not found");
	}
	//	I don't care about parent class name because in Vavoom it can be
	// different
	sc->Expect("extends");
	sc->ExpectString();
	sc->Expect("native");
	sc->Expect("{");
	while (!sc->Check("}"))
	{
		if (sc->Check("action"))
		{
			ParseAction(sc, Class);
		}
		else
		{
			sc->Error("Unknown class property");
		}
	}
	sc->SetCMode(false);
	unguard;
}

//==========================================================================
//
//	ParseEnum
//
//==========================================================================

static void ParseEnum(VScriptParser* sc)
{
	guard(ParseEnum);
	GCon->Logf("Enum");
	sc->Expect("{");
	SkipBlock(sc, 1);
	unguard;
}

//==========================================================================
//
//	ParseFlag
//
//==========================================================================

static bool ParseFlag(VScriptParser* sc, VClass* Class, bool Value,
	TArray<VClassFixup>& ClassFixups)
{
	guard(ParseFlag);
	//	Get full name of the flag.
	sc->ExpectIdentifier();
	VStr Flag = sc->String;
	while (sc->Check("."))
	{
		sc->ExpectIdentifier();
		Flag += ".";
		Flag += sc->String;
	}
	VName FlagName(*Flag.ToLower());

	for (int j = 0; j < FlagList.Num(); j++)
	{
		if (!Class->IsChildOf(FlagList[j].Class))
		{
			continue;
		}
		const TArray<VFlagDef>& Lst = FlagList[j].Flags;
		for (int i = 0; i < Lst.Num(); i++)
		{
			if (FlagName == Lst[i].Name || FlagName == Lst[i].AltName)
			{
				switch (Lst[i].Type)
				{
				case FLAG_Bool:
					SetClassFieldBool(Class, Lst[i].PropName, Value);
					break;
				case FLAG_Unsupported:
					GCon->Logf("Unsupported flag %s in %s", *Flag,
						Class->GetName());
					break;
				case FLAG_Byte:
					SetClassFieldByte(Class, Lst[i].PropName, Value ?
						Lst[i].BTrue : Lst[i].BFalse);
					break;
				case FLAG_Float:
					SetClassFieldFloat(Class, Lst[i].PropName, Value ?
						Lst[i].FTrue : Lst[i].FFalse);
					break;
				case FLAG_Name:
					SetClassFieldName(Class, Lst[i].PropName, Value ?
						Lst[i].NTrue : Lst[i].NFalse);
					break;
				case FLAG_Class:
					AddClassFixup(Class, Lst[i].PropName, Value ?
						*Lst[i].NTrue : *Lst[i].NFalse, ClassFixups);
					break;
				case FLAG_NoClip:
					SetClassFieldBool(Class, "bColideWithThings", !Value);
					SetClassFieldBool(Class, "bColideWithWorld", !Value);
					break;
				}
				return true;
			}
		}
	}
	sc->Error(va("Unknown flag %s", *Flag));
	return false;
	unguard;
}

//==========================================================================
//
//	ParseStateString
//
//==========================================================================

static VStr ParseStateString(VScriptParser* sc)
{
	guard(ParseStateString);
	VStr		StateStr;

	if (!sc->CheckQuotedString())
	{
		sc->ExpectIdentifier();
	}
	StateStr = sc->String;

	if (sc->Check("::"))
	{
		sc->ExpectIdentifier();
		StateStr += "::";
		StateStr += sc->String;
	}

	if (sc->Check("."))
	{
		sc->ExpectIdentifier();
		StateStr += ".";
		StateStr += sc->String;
	}

	return StateStr;
	unguard;
}

//==========================================================================
//
//	ParseStates
//
//==========================================================================

static bool ParseStates(VScriptParser* sc, VClass* Class,
	TArray<VState*>& States)
{
	guard(ParseStates);
	VState* PrevState = NULL;
	VState* LastState = NULL;
	VState* LoopStart = NULL;
	int NewLabelsStart = Class->StateLabelDefs.Num();

	sc->Expect("{");
	//	Disable escape sequences in states.
	sc->SetEscape(false);
	while (!sc->Check("}"))
	{
		TLocation TmpLoc = sc->GetLoc();
		VStr TmpName = ParseStateString(sc);

		//	Goto command.
		if (!TmpName.ICmp("Goto"))
		{
			VName GotoLabel = *ParseStateString(sc);
			int GotoOffset = 0;
			if (sc->Check("+"))
			{
				sc->ExpectNumber();
				GotoOffset = sc->Number;
			}

			if (!LastState && NewLabelsStart == Class->StateLabelDefs.Num())
			{
				sc->Error("Goto before first state");
			}
			if (LastState)
			{
				LastState->GotoLabel = GotoLabel;
				LastState->GotoOffset = GotoOffset;
			}
			for (int i = NewLabelsStart; i < Class->StateLabelDefs.Num(); i++)
			{
				Class->StateLabelDefs[i].GotoLabel = GotoLabel;
				Class->StateLabelDefs[i].GotoOffset = GotoOffset;
			}
			NewLabelsStart = Class->StateLabelDefs.Num();
			PrevState = NULL;
			continue;
		}

		//	Stop command.
		if (!TmpName.ICmp("Stop"))
		{
			if (!LastState && NewLabelsStart == Class->StateLabelDefs.Num())
			{
				sc->Error("Stop before first state");
				continue;
			}
			if (LastState)
			{
				LastState->NextState = NULL;
			}
			for (int i = NewLabelsStart; i < Class->StateLabelDefs.Num(); i++)
			{
				Class->StateLabelDefs[i].State = NULL;
			}
			NewLabelsStart = Class->StateLabelDefs.Num();
			PrevState = NULL;
			continue;
		}

		//	Wait command.
		if (!TmpName.ICmp("Wait") || !TmpName.ICmp("Fail"))
		{
			if (!LastState)
			{
				sc->Error(va("%s before first state", *TmpName));
				continue;
			}
			LastState->NextState = LastState;
			PrevState = NULL;
			continue;
		}

		//	Loop command.
		if (!TmpName.ICmp("Loop"))
		{
			if (!LastState)
			{
				sc->Error("Loop before first state");
				continue;
			}
			LastState->NextState = LoopStart;
			PrevState = NULL;
			continue;
		}

		//	Check for label.
		if (sc->Check(":"))
		{
			LastState = NULL;
			VStateLabelDef& Lbl = Class->StateLabelDefs.Alloc();
			Lbl.Loc = TmpLoc;
			Lbl.Name = TmpName;
			continue;
		}

		VState* State = new VState(va("S_%d", States.Num()), Class, TmpLoc);
		States.Append(State);

		//	Sprite name
		if (TmpName.Length() != 4)
		{
			sc->Error("Invalid sprite name");
		}
		State->SpriteName = *TmpName.ToLower();

		//  Frame
		sc->ExpectString();
		char FChar = VStr::ToUpper(sc->String[0]);
		if (FChar < 'A' || FChar > ']')
		{
			sc->Error("Frames must be A-Z, [, \\ or ]");
		}
		State->Frame = FChar - 'A';
		VStr FramesString = sc->String;

		//  Tics
		sc->ExpectNumberWithSign();
		if (sc->Number < 0)
		{
			State->Time = sc->Number;
		}
		else
		{
			State->Time = float(sc->Number) / 35.0;
		}

		bool NeedsUnget = true;
		while (sc->GetString() && !sc->Crossed)
		{
			//	Check for bright parameter.
			if (!sc->String.ICmp("Bright"))
			{
				State->Frame |= VState::FF_FULLBRIGHT;
				continue;
			}

			//	Check for offsets.
			if (!sc->String.ICmp("Offset"))
			{
				sc->Expect("(");
				sc->ExpectNumberWithSign();
				State->Misc1 = sc->Number;
				sc->Expect(",");
				sc->ExpectNumberWithSign();
				State->Misc2 = sc->Number;
				sc->Expect(")");
				continue;
			}

			//	Get function name and parse arguments.
			VStr FuncName = sc->String;
			VStr FuncNameLower = sc->String.ToLower();
			VExpression* Args[VMethod::MAX_PARAMS + 1];
			int NumArgs = 0;
			if (sc->Check("("))
			{
				if (!sc->Check(")"))
				{
					do
					{
						Args[NumArgs] = ParseExpressionPriority13(sc);
						if (NumArgs == VMethod::MAX_PARAMS)
							ParseError(sc->GetLoc(), "Too many arguments");
						else
							NumArgs++;
					} while (sc->Check(","));
					sc->Expect(")");
				}
			}

			//	Find the state action method. First check action specials, then
			// state actions.
			VMethod* Func = NULL;
			for (int i = 0; i < LineSpecialInfos.Num(); i++)
			{
				if (LineSpecialInfos[i].Name == FuncNameLower)
				{
					Func = Class->FindMethodChecked("A_ExecActionSpecial");
					if (NumArgs > 5)
					{
						sc->Error("Too many arguments");
					}
					else
					{
						//	Add missing arguments.
						while (NumArgs < 5)
						{
							Args[NumArgs] = new VIntLiteral(0, sc->GetLoc());
							NumArgs++;
						}
						//	Add action special number argument.
						Args[5] = new VIntLiteral(LineSpecialInfos[i].Number,
							sc->GetLoc());
						NumArgs++;
					}
					break;
				}
			}
			if (!Func)
			{
				VDecorateStateAction* Act = Class->FindDecorateStateAction(
					*FuncNameLower);
				Func = Act ? Act->Method : NULL;
			}
			if (!Func)
			{
				GCon->Logf("Unknown state action %s in %s", *FuncName, Class->GetName());
			}
			else if (Func->NumParams || NumArgs)
			{
				VInvocation* Expr = new VInvocation(NULL, Func, NULL,
					false, false, sc->GetLoc(), NumArgs, Args);
				Expr->CallerState = State;
				Expr->MultiFrameState = FramesString.Length() > 1;
				VExpressionStatement* Stmt = new VExpressionStatement(Expr);
				VMethod* M = new VMethod(NAME_None, Class, sc->GetLoc());
				M->Flags = FUNC_Final;
				M->ReturnType = TYPE_Void;
				M->Statement = Stmt;
				M->ParamsSize = 1;
				Class->AddMethod(M);
				State->Function = M;
			}
			else
			{
				State->Function = Func;
			}

			//	If state function is not assigned, it means something is wrong.
			// In that case we need to free argument expressions.
			if (!State->Function)
			{
				for (int i = 0; i < NumArgs; i++)
				{
					if (Args[i])
					{
						delete Args[i];
					}
				}
			}
			NeedsUnget = false;
			break;
		}
		if (NeedsUnget)
		{
			sc->UnGet();
		}

		//	Link previous state.
		if (PrevState)
		{
			PrevState->NextState = State;
		}

		//	Assign state to the labels.
		for (int i = NewLabelsStart; i < Class->StateLabelDefs.Num(); i++)
		{
			Class->StateLabelDefs[i].State = State;
			LoopStart = State;
		}
		NewLabelsStart = Class->StateLabelDefs.Num();
		PrevState = State;
		LastState = State;

		for (size_t i = 1; i < FramesString.Length(); i++)
		{
			char FChar = VStr::ToUpper(FramesString[i]);
			if (FChar < 'A' || FChar > ']')
			{
				sc->Error("Frames must be A-Z, [, \\ or ]");
			}

			//	Create a new state.
			VState* s2 = new VState(va("S_%d", States.Num()), Class,
				sc->GetLoc());
			States.Append(s2);
			s2->SpriteName = State->SpriteName;
			s2->Frame = (State->Frame & VState::FF_FULLBRIGHT) | (FChar - 'A');
			s2->Time = State->Time;
			s2->Misc1 = State->Misc1;
			s2->Misc2 = State->Misc2;
			s2->Function = State->Function;

			//	Link previous state.
			PrevState->NextState = s2;
			PrevState = s2;
			LastState = s2;
		}
	}
	//	Re-enable escape sequences.
	sc->SetEscape(true);
	return true;
	unguard;
}

//==========================================================================
//
//	ParseParentState
//
//	This is for compatibility with old WADs.
//
//==========================================================================

static void ParseParentState(VScriptParser* sc, VClass* Class,
	const char* LblName)
{
	guard(ParseParentState);
	TLocation TmpLoc = sc->GetLoc();
	VState* State;
	//	If there's a string token on next line, it gets eaten. Is this a bug?
	if (sc->GetString() && !sc->Crossed)
	{
		sc->UnGet();
		if (sc->Check("0"))
		{
			State = NULL;
		}
		else if (sc->Check("parent"))
		{
			//	Find state in parent class.
			sc->ExpectString();
			VStateLabel* SLbl = Class->ParentClass->FindStateLabel(*sc->String);
			State = SLbl ? SLbl->State : NULL;

			//	Check for offset.
			int Offs = 0;
			if (sc->Check("+"))
			{
				sc->ExpectNumber();
				Offs = sc->Number;
			}

			if (!State && Offs)
			{
				sc->Error(va("Attempt to get invalid state from actor %s",
					Class->GetSuperClass()->GetName()));
			}
			else if (State)
			{
				State = State->GetPlus(Offs, true);
			}
		}
		else
		{
			sc->Error("Invalid state assignment");
		}
	}
	else
	{
		State = NULL;
	}

	VStateLabelDef& Lbl = Class->StateLabelDefs.Alloc();
	Lbl.Loc = TmpLoc;
	Lbl.Name = LblName;
	Lbl.State = State;
	unguard;
}

//==========================================================================
//
//	ParseActor
//
//==========================================================================

static void ParseActor(VScriptParser* sc, TArray<VClassFixup>& ClassFixups)
{
	guard(ParseActor);
	//	Parse actor name. In order to allow dots in actor names, this is done
	// in non-C mode, so we have to do a little bit more complex parsing.
	sc->ExpectString();
	VStr NameStr;
	VStr ParentStr;
	int ColonPos = sc->String.IndexOf(':');
	if (ColonPos >= 0)
	{
		//	There's a colon inside, so plit up the string.
		NameStr = VStr(sc->String, 0, ColonPos);
		ParentStr = VStr(sc->String, ColonPos + 1, sc->String.Length() -
			ColonPos - 1);
	}
	else
	{
		NameStr = sc->String;
	}

	if (VClass::FindClassNoCase(*sc->String))
	{
		sc->Error(va("Redeclared class %s", *sc->String));
	}

	if (ColonPos < 0)
	{
		//	There's no colon, check if next string starts with it.
		sc->ExpectString();
		if (sc->String[0] == ':')
		{
			ColonPos = 0;
			ParentStr = VStr(sc->String, 1, sc->String.Length() - 1);
		}
		else
		{
			sc->UnGet();
		}
	}

	//	If we got colon but no parent class name, then get it.
	if (ColonPos >= 0 && !ParentStr)
	{
		sc->ExpectString();
		ParentStr = sc->String;
	}

	VClass* ParentClass = ActorClass;
	if (ParentStr)
	{
		ParentClass = VClass::FindClassNoCase(*ParentStr);
		if (!ParentClass)
		{
			sc->Error(va("Parent class %s not found", *ParentStr));
		}
		if (!ParentClass->IsChildOf(ActorClass))
		{
			sc->Error(va("Parent class %s is not an actor class", *ParentStr));
		}
	}

	VClass* Class = ParentClass->CreateDerivedClass(*NameStr, DecPkg,
		sc->GetLoc());
	DecPkg->ParsedClasses.Append(Class);

	if (ParentClass)
	{
		//	Copy class fixups of the parent class.
		for (int i = 0; i < ClassFixups.Num(); i++)
		{
			VClassFixup& CF = ClassFixups[i];
			if (CF.Class == ParentClass)
			{
				VClassFixup& NewCF = ClassFixups.Alloc();
				NewCF.Offset = CF.Offset;
				NewCF.Name = CF.Name;
				NewCF.ReqParent = CF.ReqParent;
				NewCF.Class = Class;
			}
		}
	}

	VClass* ReplaceeClass = NULL;
	if (sc->Check("replaces"))
	{
		sc->ExpectString();
		ReplaceeClass = VClass::FindClassNoCase(*sc->String);
		if (!ReplaceeClass)
		{
			sc->Error(va("Replaced class %s not found", *sc->String));
		}
		if (!ReplaceeClass->IsChildOf(ActorClass))
		{
			sc->Error(va("Replaced class %s is not an actor class", *sc->String));
		}
	}

	//	Time to switch to the C mode.
	sc->SetCMode(true);

	int GameFilter = 0;
	int DoomEdNum = -1;
	int SpawnNum = -1;
	TArray<VState*> States;
	bool DropItemsDefined = false;

	if (sc->CheckNumber())
	{
		if (sc->Number < -1 || sc->Number > 32767)
		{
			sc->Error("DoomEdNum is out of range [-1, 32767]");
		}
		DoomEdNum = sc->Number;
	}

	sc->Expect("{");
	while (!sc->Check("}"))
	{
		if (sc->Check("+"))
		{
			if (!ParseFlag(sc, Class, true, ClassFixups))
			{
				return;
			}
			continue;
		}
		if (sc->Check("-"))
		{
			if (!ParseFlag(sc, Class, false, ClassFixups))
			{
				return;
			}
			continue;
		}

		if (sc->Check("action"))
		{
			ParseAction(sc, Class);
			continue;
		}

		//	Get full name of the property.
		sc->ExpectIdentifier();
		VStr Prop = sc->String;
		while (sc->Check("."))
		{
			sc->ExpectIdentifier();
			Prop += ".";
			Prop += sc->String;
		}
		VName PropName = *Prop.ToLower();

		bool FoundProp = false;
		for (int j = 0; j < FlagList.Num() && !FoundProp; j++)
		{
			if (!Class->IsChildOf(FlagList[j].Class))
			{
				continue;
			}
			for (int i = 0; i < FlagList[j].Props.Num(); i++)
			{
				VPropDef& P = FlagList[j].Props[i];
				if (PropName != P.Name)
				{
					continue;
				}
				switch (P.Type)
				{
				case PROP_Int:
					sc->ExpectNumberWithSign();
					SetClassFieldInt(Class, P.PropName, sc->Number);
					break;
				case PROP_IntConst:
					SetClassFieldInt(Class, P.PropName, P.IConst);
					break;
				case PROP_IntUnsupported:
					//FIXME
					sc->ExpectNumber();
					GCon->Logf("Property %s in %s is not yet supported", *Prop, Class->GetName());
					break;
				case PROP_BitIndex:
					sc->ExpectNumber();
					SetClassFieldInt(Class, P.PropName, 1 << (sc->Number - 1));
					break;
				case PROP_Float:
					sc->ExpectFloatWithSign();
					SetClassFieldFloat(Class, P.PropName, sc->Float);
					break;
				case PROP_Speed:
					sc->ExpectFloatWithSign();
					SetClassFieldFloat(Class, P.PropName, sc->Float * 35.0);
					break;
				case PROP_Tics:
					sc->ExpectNumberWithSign();
					SetClassFieldFloat(Class, P.PropName, sc->Number / 35.0);
					break;
				case PROP_Percent:
					sc->ExpectFloat();
					SetClassFieldFloat(Class, P.PropName, MID(0, sc->Float, 100) / 100.0);
					break;
				case PROP_FloatClamped:
					sc->ExpectFloatWithSign();
					SetClassFieldFloat(Class, P.PropName, MID(P.FMin, sc->Float, P.FMax));
					break;
				case PROP_FloatClamped2:
					sc->ExpectFloatWithSign();
					SetClassFieldFloat(Class, P.PropName, MID(P.FMin, sc->Float, P.FMax));
					SetClassFieldFloat(Class, P.Prop2Name, MID(P.FMin, sc->Float, P.FMax));
					break;
				case PROP_FloatOpt2:
					sc->ExpectFloat();
					SetClassFieldFloat(Class, P.PropName, sc->Float);
					SetClassFieldFloat(Class, P.Prop2Name, sc->Float);
					if (sc->Check(","))
					{
						sc->ExpectFloat();
						SetClassFieldFloat(Class, P.Prop2Name, sc->Float);
					}
					else if (sc->CheckFloat())
					{
						SetClassFieldFloat(Class, P.Prop2Name, sc->Float);
					}
					break;
				case PROP_Name:
					sc->ExpectString();
					SetClassFieldName(Class, P.PropName, *sc->String);
					break;
				case PROP_NameLower:
					sc->ExpectString();
					SetClassFieldName(Class, P.PropName, *sc->String.ToLower());
					break;
				case PROP_Str:
					sc->ExpectString();
					SetClassFieldStr(Class, P.PropName, sc->String);
					break;
				case PROP_StrUnsupported:
					//FIXME
					sc->ExpectString();
					GCon->Logf("Property %s in %s is not yet supported", *Prop, Class->GetName());
					break;
				case PROP_Class:
					sc->ExpectString();
					AddClassFixup(Class, P.PropName, P.CPrefix + sc->String, ClassFixups);
					break;
				case PROP_BoolConst:
					SetClassFieldBool(Class, P.PropName, P.IConst);
					break;
				case PROP_State:
					ParseParentState(sc, Class, *P.PropName);
					break;
				case PROP_Game:
					if (sc->Check("Doom"))
					{
						GameFilter |= GAME_Doom;
					}
					else if (sc->Check("Heretic"))
					{
						GameFilter |= GAME_Heretic;
					}
					else if (sc->Check("Hexen"))
					{
						GameFilter |= GAME_Hexen;
					}
					else if (sc->Check("Strife"))
					{
						GameFilter |= GAME_Strife;
					}
					else if (sc->Check("Raven"))
					{
						GameFilter |= GAME_Raven;
					}
					else if (sc->Check("Any"))
					{
						GameFilter |= GAME_Any;
					}
					else if (GameFilter)
					{
						sc->Error("Unknown game filter");
					}
					break;
				case PROP_SpawnId:
					sc->ExpectNumber();
					SpawnNum = sc->Number;
					break;
				case PROP_ConversationId:
					sc->ExpectNumber();
					SetClassFieldInt(Class, "ConversationID", sc->Number);
					if (sc->Check(","))
					{
						sc->ExpectNumberWithSign();
						sc->Expect(",");
						sc->ExpectNumberWithSign();
					}
					break;
				case PROP_PainChance:
					if (sc->CheckNumber())
					{
						SetClassFieldFloat(Class, "PainChance", float(sc->Number) / 256.0);
					}
					else
					{
						sc->ExpectString();
						VName DamageType = sc->String.ICmp("Normal") ? NAME_None :
							VName(*sc->String);
						sc->Expect(",");
						sc->ExpectNumber();
		
						//	Check pain chances array for replacements.
						TArray<VPainChanceInfo> PainChances = GetClassPainChances(Class);
						VPainChanceInfo* PC = NULL;
						for (int i = 0; i < PainChances.Num(); i++)
						{
							if (PainChances[i].DamageType == DamageType)
							{
								PC = &PainChances[i];
								break;
							}
						}
						if (!PC)
						{
							PC = &PainChances.Alloc();
							PC->DamageType = DamageType;
						}
						PC->Chance = float(sc->Number) / 256.0;
					}
					break;
				case PROP_DamageFactor:
				{
					sc->ExpectString();
					VName DamageType = !sc->String.ICmp("Normal") ? NAME_None :
						VName(*sc->String);
					sc->Expect(",");
					sc->ExpectFloat();
		
					//	Check damage factors array for replacements.
					TArray<VDamageFactor> DamageFactors = GetClassDamageFactors(Class);
					VDamageFactor* DF = NULL;
					for (int i = 0; i < DamageFactors.Num(); i++)
					{
						if (DamageFactors[i].DamageType == DamageType)
						{
							DF = &DamageFactors[i];
							break;
						}
					}
					if (!DF)
					{
						DF = &DamageFactors.Alloc();
						DF->DamageType = DamageType;
					}
					DF->Factor = sc->Float;
					break;
				}
				case PROP_MissileDamage:
					if (sc->Check("("))
					{
						VExpression* Expr = ParseExpression(sc);
						if (!Expr)
						{
							ParseError(sc->GetLoc(), "Damage expression expected");
						}
						else
						{
							VMethod* M = new VMethod("GetMissileDamage", Class, sc->GetLoc());
							M->ReturnTypeExpr = new VTypeExpr(TYPE_Int, sc->GetLoc());
							M->ReturnType = TYPE_Int;
							M->NumParams = 2;
							M->Params[0].Name = "Mask";
							M->Params[0].Loc = sc->GetLoc();
							M->Params[0].TypeExpr = new VTypeExpr(TYPE_Int, sc->GetLoc());
							M->Params[1].Name = "Add";
							M->Params[1].Loc = sc->GetLoc();
							M->Params[1].TypeExpr = new VTypeExpr(TYPE_Int, sc->GetLoc());
							M->Statement = new VReturn(Expr, sc->GetLoc());
							Class->AddMethod(M);
							M->Define();
						}
						sc->Expect(")");
					}
					else
					{
						sc->ExpectNumber();
						SetClassFieldInt(Class, "MissileDamage", sc->Number);
					}
					break;
				case PROP_VSpeed:
				{
					sc->ExpectFloatWithSign();
					TVec Val = GetClassFieldVec(Class, "Velocity");
					Val.z = sc->Float * 35.0;
					SetClassFieldVec(Class, "Velocity", Val);
					break;
				}
				case PROP_RenderStyle:
				{
					int RenderStyle = 0;
					if (sc->Check("None"))
					{
						RenderStyle = STYLE_None;
					}
					else if (sc->Check("Normal"))
					{
						RenderStyle = STYLE_Normal;
					}
					else if (sc->Check("Fuzzy"))
					{
						RenderStyle = STYLE_Fuzzy;
					}
					else if (sc->Check("SoulTrans"))
					{
						RenderStyle = STYLE_SoulTrans;
					}
					else if (sc->Check("OptFuzzy"))
					{
						RenderStyle = STYLE_OptFuzzy;
					}
					else if (sc->Check("Translucent"))
					{
						RenderStyle = STYLE_Translucent;
					}
					else if (sc->Check("Add"))
					{
						RenderStyle = STYLE_Add;
					}
					else if (sc->Check("Stencil"))
					{
						//FIXME
						GCon->Logf("Render style Stencil in %s is not yet supported", Class->GetName());
					}
					else
					{
						sc->Error("Bad render style");
					}
					SetClassFieldByte(Class, P.PropName, RenderStyle);
					break;
				}
				case PROP_Translation:
					SetClassFieldInt(Class, P.PropName,
						R_ParseDecorateTranslation(sc,
						GameFilter & GAME_Strife ? 7 : 3));
					break;
				case PROP_BloodColour:
				{
					vuint32 Col;
					if (sc->CheckNumber())
					{
						int r = MID(0, sc->Number, 255);
						sc->Check(",");
						sc->ExpectNumber();
						int g = MID(0, sc->Number, 255);
						sc->Check(",");
						sc->ExpectNumber();
						int b = MID(0, sc->Number, 255);
						Col = 0xff000000 | (r << 16) | (g << 8) | b;
					}
					else
					{
						sc->ExpectString();
						Col = M_ParseColour(sc->String);
					}
					SetClassFieldInt(Class, "BloodColour", Col);
					SetClassFieldInt(Class, "BloodTranslation",
						R_GetBloodTranslation(Col));
					break;
				}
				case PROP_BloodType:
					sc->ExpectString();
					AddClassFixup(Class, "BloodType", sc->String, ClassFixups);
					if (sc->Check(","))
					{
						sc->ExpectString();
					}
					AddClassFixup(Class, "BloodSplatterType", sc->String, ClassFixups);
					if (sc->Check(","))
					{
						sc->ExpectString();
					}
					AddClassFixup(Class, "AxeBloodType", sc->String, ClassFixups);
					break;
				case PROP_StencilColour:
					//FIXME
					if (sc->CheckNumber())
					{
						sc->ExpectNumber();
						sc->ExpectNumber();
					}
					else
					{
						sc->ExpectString();
					}
					GCon->Logf("Property StencilColor in %s is not yet supported", Class->GetName());
					break;
				case PROP_Monster:
					SetClassFieldBool(Class, "bShootable", true);
					SetClassFieldBool(Class, "bCountKill", true);
					SetClassFieldBool(Class, "bSolid", true);
					SetClassFieldBool(Class, "bActivatePushWall", true);
					SetClassFieldBool(Class, "bActivateMCross", true);
					SetClassFieldBool(Class, "bPassMobj", true);
					SetClassFieldBool(Class, "bMonster", true);
					SetClassFieldBool(Class, "bCanUseWalls", true);
					break;
				case PROP_Projectile:
					SetClassFieldBool(Class, "bNoBlockmap", true);
					SetClassFieldBool(Class, "bNoGravity", true);
					SetClassFieldBool(Class, "bDropOff", true);
					SetClassFieldBool(Class, "bMissile", true);
					SetClassFieldBool(Class, "bActivateImpact", true);
					SetClassFieldBool(Class, "bActivatePCross", true);
					SetClassFieldBool(Class, "bNoTeleport", true);
					if (GGameInfo->Flags & VGameInfo::GIF_DefaultBloodSplatter)
					{
						SetClassFieldBool(Class, "bBloodSplatter", true);
					}
					break;
				case PROP_ClearFlags:
					for (int j = 0; j < FlagList.Num(); j++)
					{
						if (FlagList[j].Class != ActorClass)
						{
							continue;
						}
						for (int i = 0; i < FlagList[j].Flags.Num(); i++)
						{
							switch (FlagList[j].Flags[i].Type)
							{
							case FLAG_Bool:
								SetClassFieldBool(Class, FlagList[j].Flags[i].PropName, false);
								break;
							}
						}
					}
					SetClassFieldByte(Class, "BounceType", BOUNCE_None);
					SetClassFieldBool(Class, "bColideWithThings", true);
					SetClassFieldBool(Class, "bColideWithWorld", true);
					SetClassFieldBool(Class, "bPickUp", false);
					break;
				case PROP_DropItem:
				{
					if (!DropItemsDefined)
					{
						GetClassDropItems(Class).Clear();
						DropItemsDefined = true;
					}
					sc->ExpectString();
					VDropItemInfo DI;
					DI.TypeName = *sc->String;
					DI.Type = NULL;
					DI.Amount = 0;
					DI.Chance = 1.0;
					bool HaveChance = false;
					if (sc->Check(","))
					{
						sc->ExpectNumber();
						HaveChance = true;
					}
					else
					{
						HaveChance = sc->CheckNumber();
					}
					if (HaveChance)
					{
						DI.Chance = float(sc->Number) / 255.0;
						if (sc->Check(","))
						{
							sc->ExpectNumber();
							DI.Amount = sc->Number;
						}
						else if (sc->CheckNumber())
						{
							DI.Amount = sc->Number;
						}
					}
					GetClassDropItems(Class).Insert(0, DI);
					break;
				}
				case PROP_States:
					if (!ParseStates(sc, Class, States))
					{
						return;
					}
					break;
				case PROP_SkipSuper:
				{
					//	Preserve items that should not be copied
					TArray<VDamageFactor> DamageFactors = GetClassDamageFactors(Class);
					TArray<VPainChanceInfo> PainChances = GetClassPainChances(Class);
					//	Copy default properties.
					ActorClass->CopyObject(ActorClass->Defaults, Class->Defaults);
					//	Copy state labels
					Class->StateLabels = ActorClass->StateLabels;
					Class->ClassFlags |= CLASS_SkipSuperStateLabels;
					//	Drop items are reset back to the list of the parent class
					GetClassDropItems(Class) = GetClassDropItems(Class->ParentClass);
					//	Restore items that should not be copied
					GetClassDamageFactors(Class) = DamageFactors;
					GetClassPainChances(Class) = PainChances;
					break;
				}
				case PROP_Args:
					for (int i = 0; i < 5; i++)
					{
						sc->ExpectNumber();
						SetClassFieldInt(Class, "Args", sc->Number, i);
						if (i < 4 && !sc->Check(","))
						{
							break;
						}
					}
					SetClassFieldBool(Class, "bArgsDefined", true);
					break;
				case PROP_PickupMessage:
				{
					sc->ExpectString();
					VStr Msg = sc->String;
					int Filter = 0;
					if (!Msg.ICmp("Doom"))
					{
						Filter = GAME_Doom;
					}
					else if (!Msg.ICmp("Heretic"))
					{
						Filter = GAME_Heretic;
					}
					else if (!Msg.ICmp("Hexen"))
					{
						Filter = GAME_Hexen;
					}
					else if (!Msg.ICmp("Raven"))
					{
						Filter = GAME_Raven;
					}
					else if (!Msg.ICmp("Strife"))
					{
						Filter = GAME_Strife;
					}
					if (Filter && sc->Check(","))
					{
						sc->ExpectString();
						if (GGameInfo->GameFilterFlag & Filter)
						{
							SetClassFieldStr(Class, "PickupMessage", sc->String);
						}
					}
					else
					{
						SetClassFieldStr(Class, "PickupMessage", Msg);
					}
					break;
				}
				case PROP_LowMessage:
					sc->ExpectNumber();
					SetClassFieldInt(Class, "LowHealth", sc->Number);
					sc->Expect(",");
					sc->ExpectString();
					SetClassFieldStr(Class, "LowHealthMessage", sc->String);
					break;
				case PROP_PowerupColour:
					if (sc->Check("InverseMap"))
					{
						SetClassFieldInt(Class, "BlendColour", 0x00123456);
					}
					else if (sc->Check("GoldMap"))
					{
						SetClassFieldInt(Class, "BlendColour", 0x00123457);
					}
					else if (sc->Check("RedMap"))
					{
						SetClassFieldInt(Class, "BlendColour", 0x00123458);
					}
					else if (sc->Check("GreenMap"))
					{
						SetClassFieldInt(Class, "BlendColour", 0x00123459);
					}
					else
					{
						vuint32 Col;
						if (sc->CheckNumber())
						{
							int r = MID(0, sc->Number, 255);
							sc->Check(",");
							sc->ExpectNumber();
							int g = MID(0, sc->Number, 255);
							sc->Check(",");
							sc->ExpectNumber();
							int b = MID(0, sc->Number, 255);
							Col = (r << 16) | (g << 8) | b;
						}
						else
						{
							sc->ExpectString();
							Col = M_ParseColour(sc->String);
						}
						sc->Check(",");
						sc->ExpectFloat();
						int a = MID(0, (int)(sc->Float * 255), 255);
						Col |= a << 24;
						SetClassFieldInt(Class, "BlendColour", Col);
					}
					break;
				case PROP_ColourRange:
					sc->ExpectNumber();
					SetClassFieldInt(Class, "TranslStart", sc->Number);
					sc->Check(",");
					sc->ExpectNumber();
					SetClassFieldInt(Class, "TranslEnd", sc->Number);
					break;
				case PROP_DamageScreenColour:
				{
					//	First number is ignored. Is it a bug?
					int Col;
					if (sc->CheckNumber())
					{
						sc->ExpectNumber();
						int r = MID(sc->Number, 0, 255);
						sc->Check(",");
						sc->ExpectNumber();
						int g = MID(sc->Number, 0, 255);
						sc->Check(",");
						sc->ExpectNumber();
						int b = MID(sc->Number, 0, 255);
						Col = 0xff000000 | (r << 16) | (g << 8) | b;
					}
					else
					{
						sc->ExpectString();
						Col = M_ParseColour(sc->String);
					}
					SetClassFieldInt(Class, "DamageScreenColour", Col);
					break;
				}
				case PROP_HexenArmor:
					sc->ExpectFloat();
					SetClassFieldFloat(Class, "HexenArmor", sc->Float, 0);
					sc->Expect(",");
					sc->ExpectFloat();
					SetClassFieldFloat(Class, "HexenArmor", sc->Float, 1);
					sc->Expect(",");
					sc->ExpectFloat();
					SetClassFieldFloat(Class, "HexenArmor", sc->Float, 2);
					sc->Expect(",");
					sc->ExpectFloat();
					SetClassFieldFloat(Class, "HexenArmor", sc->Float, 3);
					sc->Expect(",");
					sc->ExpectFloat();
					SetClassFieldFloat(Class, "HexenArmor", sc->Float, 4);
					break;
				case PROP_StartItem:
				{
					if (!DropItemsDefined)
					{
						GetClassDropItems(Class).Clear();
						DropItemsDefined = true;
					}
					sc->ExpectString();
					VDropItemInfo DI;
					DI.TypeName = *sc->String;
					DI.Type = NULL;
					DI.Amount = 0;
					DI.Chance = 1.0;
					if (sc->Check(","))
					{
						sc->ExpectNumber();
						DI.Amount = sc->Number;
					}
					else if (sc->CheckNumber())
					{
						DI.Amount = sc->Number;
					}
					GetClassDropItems(Class).Insert(0, DI);
					break;
				}
				}
				FoundProp = true;
				break;
			}
		}
		if (FoundProp)
		{
			continue;
		}

		sc->Error(va("Unknown property \"%s\"", *Prop));
	}

	sc->SetCMode(false);

	Class->EmitStateLabels();

	//	Set up linked list of states.
	if (States.Num())
	{
		Class->States = States[0];
		for (int i = 0; i < States.Num() - 1; i++)
		{
			States[i]->Next = States[i + 1];
		}

		for (int i = 0; i < States.Num(); i++)
		{
			if (States[i]->GotoLabel != NAME_None)
			{
				States[i]->NextState = Class->ResolveStateLabel(
					States[i]->Loc, States[i]->GotoLabel, States[i]->GotoOffset);
			}
		}
	}

	if (DoomEdNum > 0)
	{
		mobjinfo_t& MI = VClass::GMobjInfos.Alloc();
		MI.Class = Class;
		MI.DoomEdNum = DoomEdNum;
		MI.GameFilter = GameFilter;
	}
	if (SpawnNum > 0)
	{
		mobjinfo_t& SI = VClass::GScriptIds.Alloc();
		SI.Class = Class;
		SI.DoomEdNum = SpawnNum;
		SI.GameFilter = GameFilter;
	}

	if (ReplaceeClass)
	{
		ReplaceeClass->Replacement = Class;
		Class->Replacee = ReplaceeClass;
	}
	unguard;
}

//==========================================================================
//
//	ParseOldDecStates
//
//==========================================================================

static void ParseOldDecStates(VScriptParser* sc, TArray<VState*>& States,
	VClass* Class)
{
	guard(ParseOldDecStates);
	TArray<VStr> Tokens;
	sc->String.Split(",\t\r\n", Tokens);
	for (int TokIdx = 0; TokIdx < Tokens.Num(); TokIdx++)
	{
		const char* pFrame = *Tokens[TokIdx];
		int DurColon = Tokens[TokIdx].IndexOf(':');
		float Duration = 4;
		if (DurColon >= 0)
		{
			Duration = atoi(pFrame);
			pFrame = *Tokens[TokIdx] + DurColon + 1;
		}

		bool GotState = false;
		while (*pFrame)
		{
			if (*pFrame == ' ')
			{
			}
			else if (*pFrame == '*')
			{
				if (!GotState)
				{
					sc->Error("* must come after a frame");
				}
				States[States.Num() - 1]->Frame |= VState::FF_FULLBRIGHT;
			}
			else if (*pFrame < 'A' || *pFrame > ']')
			{
				sc->Error("Frames must be A-Z, [, \\, or ]");
			}
			else
			{
				GotState = true;
				VState* State = new VState(va("S_%d", States.Num()), Class,
					sc->GetLoc());
				States.Append(State);
				State->Frame = *pFrame - 'A';
				State->Time = Duration >= 0 ? float(Duration) / 35.0 : -1.0;
			}
			pFrame++;
		}
	}
	unguard;
}

//==========================================================================
//
//	ParseOldDecoration
//
//==========================================================================

static void ParseOldDecoration(VScriptParser* sc, int Type)
{
	guard(ParseOldDecoration);
	//	Get name of the class.
	sc->ExpectString();
	VName ClassName = *sc->String;

	//	Create class.
	VClass* Class = Type == OLDDEC_Pickup ?
		FakeInventoryClass->CreateDerivedClass(ClassName, DecPkg,
		sc->GetLoc()) :
		ActorClass->CreateDerivedClass(ClassName, DecPkg, sc->GetLoc());
	DecPkg->ParsedClasses.Append(Class);
	if (Type == OLDDEC_Breakable)
	{
		SetClassFieldBool(Class, "bShootable", true);
	}
	if (Type == OLDDEC_Projectile)
	{
		SetClassFieldBool(Class, "bMissile", true);
		SetClassFieldBool(Class, "bDropOff", true);
	}

	//	Parse game filters.
	int GameFilter = 0;
	while (!sc->Check("{"))
	{
		if (sc->Check("Doom"))
		{
			GameFilter |= GAME_Doom;
		}
		else if (sc->Check("Heretic"))
		{
			GameFilter |= GAME_Heretic;
		}
		else if (sc->Check("Hexen"))
		{
			GameFilter |= GAME_Hexen;
		}
		else if (sc->Check("Strife"))
		{
			GameFilter |= GAME_Strife;
		}
		else if (sc->Check("Raven"))
		{
			GameFilter |= GAME_Raven;
		}
		else if (sc->Check("Any"))
		{
			GameFilter |= GAME_Any;
		}
		else if (GameFilter)
		{
			sc->Error("Unknown game filter");
		}
		else
		{
			sc->Error("Unknown identifier");
		}
	}

	int DoomEdNum = -1;
	int SpawnNum = -1;
	VName Sprite("tnt1");
	VName DeathSprite(NAME_None);
	TArray<VState*> States;
	int SpawnStart = 0;
	int SpawnEnd = 0;
	int DeathStart = 0;
	int DeathEnd = 0;
	bool DiesAway = false;
	bool SolidOnDeath = false;
	float DeathHeight = 0.0;
	int BurnStart = 0;
	int BurnEnd = 0;
	bool BurnsAway = false;
	bool SolidOnBurn = false;
	float BurnHeight = 0.0;
	int IceStart = 0;
	int IceEnd = 0;
	bool GenericIceDeath = false;
	bool Explosive = false;

	while (!sc->Check("}"))
	{
		if (sc->Check("DoomEdNum"))
		{
			sc->ExpectNumber();
			if (sc->Number < -1 || sc->Number > 32767)
			{
				sc->Error("DoomEdNum is out of range [-1, 32767]");
			}
			DoomEdNum = sc->Number;
		}
		else if (sc->Check("SpawnNum"))
		{
			sc->ExpectNumber();
			if (sc->Number < 0 || sc->Number > 255)
			{
				sc->Error("SpawnNum is out of range [0, 255]");
			}
			SpawnNum = sc->Number;
		}

		//	Spawn state
		else if (sc->Check("Sprite"))
		{
			sc->ExpectString();
			if (sc->String.Length() != 4)
			{
				sc->Error("Sprite name must be 4 characters long");
			}
			Sprite = *sc->String.ToLower();
		}
		else if (sc->Check("Frames"))
		{
			sc->ExpectString();
			SpawnStart = States.Num();
			ParseOldDecStates(sc, States, Class);
			SpawnEnd = States.Num();
		}

		//	Death states
		else if ((Type == OLDDEC_Breakable || Type == OLDDEC_Projectile) &&
			sc->Check("DeathSprite"))
		{
			sc->ExpectString();
			if (sc->String.Length() != 4)
			{
				sc->Error("Sprite name must be 4 characters long");
			}
			DeathSprite = *sc->String.ToLower();
		}
		else if ((Type == OLDDEC_Breakable || Type == OLDDEC_Projectile) &&
			sc->Check("DeathFrames"))
		{
			sc->ExpectString();
			DeathStart = States.Num();
			ParseOldDecStates(sc, States, Class);
			DeathEnd = States.Num();
		}
		else if (Type == OLDDEC_Breakable && sc->Check("DiesAway"))
		{
			DiesAway = true;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnDeathFrames"))
		{
			sc->ExpectString();
			BurnStart = States.Num();
			ParseOldDecStates(sc, States, Class);
			BurnEnd = States.Num();
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnsAway"))
		{
			BurnsAway = true;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("IceDeathFrames"))
		{
			sc->ExpectString();
			IceStart = States.Num();
			ParseOldDecStates(sc, States, Class);

			//	Make a copy of the last state for A_FreezeDeathChunks
			VState* State = new VState(va("S_%d", States.Num()), Class,
				sc->GetLoc());
			States.Append(State);
			State->Frame = States[States.Num() - 2]->Frame;

			IceEnd = States.Num();
		}
		else if (Type == OLDDEC_Breakable && sc->Check("GenericIceDeath"))
		{
			GenericIceDeath = true;
		}

		//	Misc properties
		else if (sc->Check("Radius"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Radius", sc->Float);
		}
		else if (sc->Check("Height"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Height", sc->Float);
		}
		else if (sc->Check("Mass"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Mass", sc->Float);
		}
		else if (sc->Check("Scale"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "ScaleX", sc->Float);
			SetClassFieldFloat(Class, "ScaleY", sc->Float);
		}
		else if (sc->Check("Alpha"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Alpha", MID(0.0, sc->Float, 1.0));
		}
		else if (sc->Check("RenderStyle"))
		{
			int RenderStyle = 0;
			if (sc->Check("STYLE_None"))
			{
				RenderStyle = STYLE_None;
			}
			else if (sc->Check("STYLE_Normal"))
			{
				RenderStyle = STYLE_Normal;
			}
			else if (sc->Check("STYLE_Fuzzy"))
			{
				RenderStyle = STYLE_Fuzzy;
			}
			else if (sc->Check("STYLE_SoulTrans"))
			{
				RenderStyle = STYLE_SoulTrans;
			}
			else if (sc->Check("STYLE_OptFuzzy"))
			{
				RenderStyle = STYLE_OptFuzzy;
			}
			else if (sc->Check("STYLE_Translucent"))
			{
				RenderStyle = STYLE_Translucent;
			}
			else if (sc->Check("STYLE_Add"))
			{
				RenderStyle = STYLE_Add;
			}
			else
			{
				sc->Error("Bad render style");
			}
			SetClassFieldByte(Class, "RenderStyle", RenderStyle);
		}
		else if (sc->Check("Translation1"))
		{
			sc->ExpectNumber();
			if (sc->Number < 0 || sc->Number > 2)
			{
				sc->Error("Translation1 is out of range [0, 2]");
			}
			SetClassFieldInt(Class, "Translation", (TRANSL_Standard <<
				TRANSL_TYPE_SHIFT) + sc->Number);
		}
		else if (sc->Check("Translation2"))
		{
			sc->ExpectNumber();
			if (sc->Number < 0 || sc->Number > MAX_LEVEL_TRANSLATIONS)
			{
				sc->Error(va("Translation2 is out of range [0, %d]",
					MAX_LEVEL_TRANSLATIONS));
			}
			SetClassFieldInt(Class, "Translation", (TRANSL_Level <<
				TRANSL_TYPE_SHIFT) + sc->Number);
		}

		//	Breakable decoration properties.
		else if (Type == OLDDEC_Breakable && sc->Check("Health"))
		{
			sc->ExpectNumber();
			SetClassFieldInt(Class, "Health", sc->Number);
		}
		else if (Type == OLDDEC_Breakable && sc->Check("DeathHeight"))
		{
			sc->ExpectFloat();
			DeathHeight = sc->Float;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnHeight"))
		{
			sc->ExpectFloat();
			BurnHeight = sc->Float;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("SolidOnDeath"))
		{
			SolidOnDeath = true;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("SolidOnBurn"))
		{
			SolidOnBurn = true;
		}
		else if ((Type == OLDDEC_Breakable || Type == OLDDEC_Projectile) &&
			sc->Check("DeathSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "DeathSound", *sc->String);
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnDeathSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "ActiveSound", *sc->String);
		}

		//	Projectile properties
		else if (Type == OLDDEC_Projectile && sc->Check("Speed"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Speed", sc->Float * 35.0);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("Damage"))
		{
			sc->ExpectNumber();
			SetClassFieldFloat(Class, "MissileDamage", sc->Number);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("DamageType"))
		{
			if (sc->Check("Normal"))
			{
				SetClassFieldName(Class, "DamageType", NAME_None);
			}
			else
			{
				sc->ExpectString();
				SetClassFieldName(Class, "DamageType", *sc->String);
			}
		}
		else if (Type == OLDDEC_Projectile && sc->Check("SpawnSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "SightSound", *sc->String);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("ExplosionRadius"))
		{
			sc->ExpectNumber();
			SetClassFieldFloat(Class, "ExplosionRadius", sc->Number);
			Explosive = true;
		}
		else if (Type == OLDDEC_Projectile && sc->Check("ExplosionDamage"))
		{
			sc->ExpectNumber();
			SetClassFieldFloat(Class, "ExplosionDamage", sc->Number);
			Explosive = true;
		}
		else if (Type == OLDDEC_Projectile && sc->Check("DoNotHurtShooter"))
		{
			SetClassFieldBool(Class, "bExplosionDontHurtSelf", true);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("DoomBounce"))
		{
			SetClassFieldByte(Class, "BounceType", BOUNCE_Doom);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("HereticBounce"))
		{
			SetClassFieldByte(Class, "BounceType", BOUNCE_Heretic);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("HexenBounce"))
		{
			SetClassFieldByte(Class, "BounceType", BOUNCE_Hexen);
		}

		//	Pickup properties
		else if (Type == OLDDEC_Pickup && sc->Check("PickupMessage"))
		{
			sc->ExpectString();
			SetClassFieldStr(Class, "PickupMessage", sc->String);
		}
		else if (Type == OLDDEC_Pickup && sc->Check("PickupSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "PickupSound", *sc->String);
		}
		else if (Type == OLDDEC_Pickup && sc->Check("Respawns"))
		{
			SetClassFieldBool(Class, "bRespawns", true);
		}

		//	Compatibility flags
		else if (sc->Check("LowGravity"))
		{
			SetClassFieldFloat(Class, "Gravity", 0.125);
		}
		else if (sc->Check("FireDamage"))
		{
			SetClassFieldName(Class, "DamageType", "Fire");
		}

		//	Flags
		else if (sc->Check("Solid"))
		{
			SetClassFieldBool(Class, "bSolid", true);
		}
		else if (sc->Check("NoSector"))
		{
			SetClassFieldBool(Class, "bNoSector", true);
		}
		else if (sc->Check("NoBlockmap"))
		{
			SetClassFieldBool(Class, "bNoBlockmap", true);
		}
		else if (sc->Check("SpawnCeiling"))
		{
			SetClassFieldBool(Class, "bSpawnCeiling", true);
		}
		else if (sc->Check("NoGravity"))
		{
			SetClassFieldBool(Class, "bNoGravity", true);
		}
		else if (sc->Check("Shadow"))
		{
			GCon->Logf("Shadow flag is not currently supported");
		}
		else if (sc->Check("NoBlood"))
		{
			SetClassFieldBool(Class, "bNoBlood", true);
		}
		else if (sc->Check("CountItem"))
		{
			SetClassFieldBool(Class, "bCountItem", true);
		}
		else if (sc->Check("WindThrust"))
		{
			SetClassFieldBool(Class, "bWindThrust", true);
		}
		else if (sc->Check("FloorClip"))
		{
			SetClassFieldBool(Class, "bFloorClip", true);
		}
		else if (sc->Check("SpawnFloat"))
		{
			SetClassFieldBool(Class, "bSpawnFloat", true);
		}
		else if (sc->Check("NoTeleport"))
		{
			SetClassFieldBool(Class, "bNoTeleport", true);
		}
		else if (sc->Check("Ripper"))
		{
			SetClassFieldBool(Class, "bRip", true);
		}
		else if (sc->Check("Pushable"))
		{
			SetClassFieldBool(Class, "bPushable", true);
		}
		else if (sc->Check("SlidesOnWalls"))
		{
			SetClassFieldBool(Class, "bSlide", true);
		}
		else if (sc->Check("CanPass"))
		{
			SetClassFieldBool(Class, "bPassMobj", true);
		}
		else if (sc->Check("CannotPush"))
		{
			SetClassFieldBool(Class, "bCannotPush", true);
		}
		else if (sc->Check("ThruGhost"))
		{
			SetClassFieldBool(Class, "bThruGhost", true);
		}
		else if (sc->Check("NoDamageThrust"))
		{
			SetClassFieldBool(Class, "bNoDamageThrust", true);
		}
		else if (sc->Check("Telestomp"))
		{
			SetClassFieldBool(Class, "bTelestomp", true);
		}
		else if (sc->Check("FloatBob"))
		{
			SetClassFieldBool(Class, "bFloatBob", true);
		}
		else if (sc->Check("ActivateImpact"))
		{
			SetClassFieldBool(Class, "bActivateImpact", true);
		}
		else if (sc->Check("CanPushWalls"))
		{
			SetClassFieldBool(Class, "bActivatePushWall", true);
		}
		else if (sc->Check("ActivateMCross"))
		{
			SetClassFieldBool(Class, "bActivateMCross", true);
		}
		else if (sc->Check("ActivatePCross"))
		{
			SetClassFieldBool(Class, "bActivatePCross", true);
		}
		else if (sc->Check("Reflective"))
		{
			SetClassFieldBool(Class, "bReflective", true);
		}
		else if (sc->Check("FloorHugger"))
		{
			SetClassFieldBool(Class, "bIgnoreFloorStep", true);
		}
		else if (sc->Check("CeilingHugger"))
		{
			SetClassFieldBool(Class, "bIgnoreCeilingStep", true);
		}
		else if (sc->Check("DontSplash"))
		{
			SetClassFieldBool(Class, "bNoSplash", true);
		}
		else
		{
			Sys_Error("Unknown property %s", *sc->String);
		}
	}

	if (SpawnEnd == 0)
	{
		sc->Error(va("%s has no Frames definition", *ClassName));
	}
	if (Type == OLDDEC_Breakable && DeathEnd == 0)
	{
		sc->Error(va("%s has no DeathFrames definition", *ClassName));
	}
	if (GenericIceDeath && IceEnd != 0)
	{
		sc->Error("IceDeathFrames and GenericIceDeath are mutually exclusive");
	}

	if (DoomEdNum > 0)
	{
		mobjinfo_t& MI = VClass::GMobjInfos.Alloc();
		MI.Class = Class;
		MI.DoomEdNum = DoomEdNum;
		MI.GameFilter = GameFilter;
	}
	if (SpawnNum > 0)
	{
		mobjinfo_t& SI = VClass::GScriptIds.Alloc();
		SI.Class = Class;
		SI.DoomEdNum = SpawnNum;
		SI.GameFilter = GameFilter;
	}

	//	Set up linked list of states.
	Class->States = States[0];
	for (int i = 0; i < States.Num() - 1; i++)
	{
		States[i]->Next = States[i + 1];
	}

	//	Set up default sprite for all states.
	for (int i = 0; i < States.Num(); i++)
	{
		States[i]->SpriteName = Sprite;
	}
	//	Set death sprite if it's defined.
	if (DeathSprite != NAME_None && DeathEnd != 0)
	{
		for (int i = DeathStart; i < DeathEnd; i++)
		{
			States[i]->SpriteName = DeathSprite;
		}
	}

	//	Set up links of spawn states.
	if (SpawnEnd - SpawnStart == 1)
	{
		States[SpawnStart]->Time = -1.0;
	}
	else
	{
		for (int i = SpawnStart; i < SpawnEnd - 1; i++)
		{
			States[i]->NextState = States[i + 1];
		}
		States[SpawnEnd - 1]->NextState = States[SpawnStart];
	}
	Class->SetStateLabel("Spawn", States[SpawnStart]);

	//	Set up links of death states.
	if (DeathEnd != 0)
	{
		for (int i = DeathStart; i < DeathEnd - 1; i++)
		{
			States[i]->NextState = States[i + 1];
		}
		if (!DiesAway && Type != OLDDEC_Projectile)
		{
			States[DeathEnd - 1]->Time = -1.0;
		}
		if (Type == OLDDEC_Projectile)
		{
			if (Explosive)
			{
				States[DeathStart]->Function = FuncA_ExplodeParms;
			}
		}
		else
		{
			//	First death state plays death sound, second makes it
			// non-blocking unless it should stay solid.
			States[DeathStart]->Function = FuncA_Scream;
			if (!SolidOnDeath)
			{
				if (DeathEnd - DeathStart > 1)
				{
					States[DeathStart + 1]->Function = FuncA_NoBlocking;
				}
				else
				{
					States[DeathStart]->Function = FuncA_ScreamAndUnblock;
				}
			}

			if (!DeathHeight)
			{
				DeathHeight = GetClassFieldFloat(Class, "Height");
			}
			SetClassFieldFloat(Class, "DeathHeight", DeathHeight);
		}

		Class->SetStateLabel("Death", States[DeathStart]);
	}

	//	Set up links of burn death states.
	if (BurnEnd != 0)
	{
		for (int i = BurnStart; i < BurnEnd - 1; i++)
		{
			States[i]->NextState = States[i + 1];
		}
		if (!BurnsAway)
		{
			States[BurnEnd - 1]->Time = -1.0;
		}
		//	First death state plays active sound, second makes it
		// non-blocking unless it should stay solid.
		States[BurnStart]->Function = FuncA_ActiveSound;
		if (!SolidOnBurn)
		{
			if (BurnEnd - BurnStart > 1)
			{
				States[BurnStart + 1]->Function = FuncA_NoBlocking;
			}
			else
			{
				States[BurnStart]->Function = FuncA_ActiveAndUnblock;
			}
		}

		if (!BurnHeight)
		{
			BurnHeight = GetClassFieldFloat(Class, "Height");
		}
		SetClassFieldFloat(Class, "BurnHeight", BurnHeight);

		TArray<VName> Names;
		Names.Append("Death");
		Names.Append("Fire");
		Class->SetStateLabel(Names, States[BurnStart]);
	}

	//	Set up links of ice death states.
	if (IceEnd != 0)
	{
		for (int i = IceStart; i < IceEnd - 1; i++)
		{
			States[i]->NextState = States[i + 1];
		}

		States[IceEnd - 2]->Time = 5.0 / 35.0;
		States[IceEnd - 2]->Function = FuncA_FreezeDeath;

		States[IceEnd - 1]->NextState = States[IceEnd - 1];
		States[IceEnd - 1]->Time = 1.0 / 35.0;
		States[IceEnd - 1]->Function = FuncA_FreezeDeathChunks;

		TArray<VName> Names;
		Names.Append("Death");
		Names.Append("Ice");
		Class->SetStateLabel(Names, States[IceStart]);
	}
	else if (GenericIceDeath)
	{
		VStateLabel* Lbl = Class->FindStateLabel("GenericIceDeath");
		TArray<VName> Names;
		Names.Append("Death");
		Names.Append("Ice");
		Class->SetStateLabel(Names, Lbl ? Lbl->State : NULL);
	}
	unguard;
}

//==========================================================================
//
//	ParseDecorate
//
//==========================================================================

static void ParseDecorate(VScriptParser* sc, TArray<VClassFixup>& ClassFixups)
{
	guard(ParseDecorate);
	while (!sc->AtEnd())
	{
		if (sc->Check("#include"))
		{
			sc->ExpectString();
			int Lump = W_CheckNumForFileName(sc->String);
			//	Check WAD lump only if it's no longer than 8 characters and
			// has no path separator.
			if (Lump < 0 && sc->String.Length() <= 8 &&
				sc->String.IndexOf('/') < 0)
			{
				Lump = W_CheckNumForName(VName(*sc->String, VName::AddLower8));
			}
			if (Lump < 0)
			{
				sc->Error(va("Lump %s not found", *sc->String));
			}
			ParseDecorate(new VScriptParser(sc->String,
				W_CreateLumpReaderNum(Lump)), ClassFixups);
		}
		else if (sc->Check("const"))
		{
			ParseConst(sc);
		}
		else if (sc->Check("enum"))
		{
			ParseEnum(sc);
		}
		else if (sc->Check("class"))
		{
			ParseClass(sc);
		}
		else if (sc->Check("actor"))
		{
			ParseActor(sc, ClassFixups);
		}
		else if (sc->Check("breakable"))
		{
			ParseOldDecoration(sc, OLDDEC_Breakable);
		}
		else if (sc->Check("pickup"))
		{
			ParseOldDecoration(sc, OLDDEC_Pickup);
		}
		else if (sc->Check("projectile"))
		{
			ParseOldDecoration(sc, OLDDEC_Projectile);
		}
		else
		{
			ParseOldDecoration(sc, OLDDEC_Decoration);
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	ReadLineSpecialInfos
//
//==========================================================================

void ReadLineSpecialInfos()
{
	guard(ReadLineSpecialInfos);
	VStream* Strm = FL_OpenFileRead("line_specials.txt");
	check(Strm);
	VScriptParser* sc = new VScriptParser("line_specials.txt", Strm);
	while (!sc->AtEnd())
	{
		VLineSpecInfo& I = LineSpecialInfos.Alloc();
		sc->ExpectNumber();
		I.Number = sc->Number;
		sc->ExpectString();
		I.Name = sc->String.ToLower();
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	ProcessDecorateScripts
//
//==========================================================================

void ProcessDecorateScripts()
{
	guard(ProcessDecorateScripts);
	GCon->Logf(NAME_Init, "Parsing DECORATE definition files");
	for (int Lump = W_IterateFile(-1, "vavoom_decorate_defs.xml"); Lump != -1;
		Lump = W_IterateFile(Lump, "vavoom_decorate_defs.xml"))
	{
		VStream* Strm = W_CreateLumpReaderNum(Lump);
		check(Strm);
		VXmlDocument* Doc = new VXmlDocument();
		Doc->Parse(*Strm, "vavoom_decorate_defs.xml");
		delete Strm;
		ParseDecorateDef(*Doc);
		delete Doc;
	}

	GCon->Logf(NAME_Init, "Processing DECORATE scripts");

	DecPkg = new VPackage(NAME_decorate);

	//	Find classes.
	ActorClass = VClass::FindClass("Actor");
	FakeInventoryClass = VClass::FindClass("FakeInventory");
	InventoryClass = VClass::FindClass("Inventory");
	AmmoClass = VClass::FindClass("Ammo");
	BasicArmorPickupClass = VClass::FindClass("BasicArmorPickup");
	BasicArmorBonusClass = VClass::FindClass("BasicArmorBonus");
	HealthClass = VClass::FindClass("Health");
	PowerupGiverClass = VClass::FindClass("PowerupGiver");
	PuzzleItemClass = VClass::FindClass("PuzzleItem");
	WeaponClass = VClass::FindClass("Weapon");
	WeaponPieceClass = VClass::FindClass("WeaponPiece");
	PlayerPawnClass = VClass::FindClass("PlayerPawn");
	MorphProjectileClass = VClass::FindClass("MorphProjectile");

	//	Find methods used by old style decorations.
	FuncA_Scream = ActorClass->FindMethodChecked("A_Scream");
	FuncA_NoBlocking = ActorClass->FindMethodChecked("A_NoBlocking");
	FuncA_ScreamAndUnblock = ActorClass->FindMethodChecked("A_ScreamAndUnblock");
	FuncA_ActiveSound = ActorClass->FindMethodChecked("A_ActiveSound");
	FuncA_ActiveAndUnblock = ActorClass->FindMethodChecked("A_ActiveAndUnblock");
	FuncA_ExplodeParms = ActorClass->FindMethodChecked("A_ExplodeParms");
	FuncA_FreezeDeath = ActorClass->FindMethodChecked("A_FreezeDeath");
	FuncA_FreezeDeathChunks = ActorClass->FindMethodChecked("A_FreezeDeathChunks");

	//	Parse scripts.
	TArray<VClassFixup> ClassFixups;
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_decorate)
		{
			ParseDecorate(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)), ClassFixups);
		}
	}

	//	Make sure all import classes were defined.
	if (VMemberBase::GDecorateClassImports.Num())
	{
		for (int i = 0; i < VMemberBase::GDecorateClassImports.Num(); i++)
		{
			GCon->Logf("Undefined DECORATE class %s",
				VMemberBase::GDecorateClassImports[i]->GetName());
		}
		Sys_Error("Not all DECORATE class imports were defined");
	}

	GCon->Logf(NAME_Init, "Post-procesing");

	//	Set class properties.
	for (int i = 0; i < ClassFixups.Num(); i++)
	{
		VClassFixup& CF = ClassFixups[i];
		check(CF.ReqParent);
		if (!CF.Name.ICmp("None"))
		{
			*(VClass**)(CF.Class->Defaults + CF.Offset) = NULL;
		}
		else
		{
			VClass* C = VClass::FindClassNoCase(*CF.Name);
			if (!C)
			{
				GCon->Logf("No such class %s", *CF.Name);
			}
			else if (!C->IsChildOf(CF.ReqParent))
			{
				GCon->Logf("Class %s is not a descendant of %s",
					*CF.Name, CF.ReqParent->GetName());
			}
			else
			{
				*(VClass**)(CF.Class->Defaults + CF.Offset) = C;
			}
		}
	}
	for (int i = 0; i < DecPkg->ParsedClasses.Num(); i++)
	{
		TArray<VDropItemInfo>& List = GetClassDropItems(DecPkg->ParsedClasses[i]);
		for (int j = 0; j < List.Num(); j++)
		{
			VDropItemInfo& DI = List[j];
			if (DI.TypeName == NAME_None)
			{
				continue;
			}
			VClass* C = VClass::FindClassNoCase(*DI.TypeName);
			if (!C)
			{
				GCon->Logf("No such class %s", *DI.TypeName);
			}
			else if (!C->IsChildOf(ActorClass))
			{
				GCon->Logf("Class %s is not an actor class", *DI.TypeName);
			}
			else
			{
				DI.Type = C;
			}
		}
	}

	//	Emit code.
	for (int i = 0; i < DecPkg->ParsedClasses.Num(); i++)
	{
		DecPkg->ParsedClasses[i]->DecorateEmit();
	}
	//	Compile and set up for execution.
	for (int i = 0; i < DecPkg->ParsedClasses.Num(); i++)
	{
		DecPkg->ParsedClasses[i]->DecoratePostLoad();
	}

	if (NumErrors)
	{
		BailOut();
	}

	VClass::StaticReinitStatesLookup();

	TLocation::ClearSourceFiles();
	unguard;
}

//==========================================================================
//
//	ShutdownDecorate
//
//==========================================================================

void ShutdownDecorate()
{
	guard(ShutdownDecorate);
	FlagList.Clear();
	LineSpecialInfos.Clear();
	unguard;
}

//==========================================================================
//
//	VEntity::SetDecorateFlag
//
//==========================================================================

void VEntity::SetDecorateFlag(const VStr& Flag, bool Value)
{
	guard(VEntity::SetDecorateFlag);
	VName FlagName(*Flag.ToLower());
	for (int j = 0; j < FlagList.Num(); j++)
	{
		if (!IsA(FlagList[j].Class))
		{
			continue;
		}
		const TArray<VFlagDef>& Lst = FlagList[j].Flags;
		for (int i = 0; i < Lst.Num(); i++)
		{
			if (FlagName == Lst[i].Name || FlagName == Lst[i].AltName)
			{
				switch (Lst[i].Type)
				{
				case FLAG_Bool:
					SetFieldBool(this, Lst[i].PropName, Value);
					break;
				case FLAG_Unsupported:
					GCon->Logf("Unsupported flag %s in %s", *Flag,
						GetClass()->GetName());
					break;
				case FLAG_Byte:
					SetFieldByte(this, Lst[i].PropName, Value ?
						Lst[i].BTrue : Lst[i].BFalse);
					break;
				case FLAG_Float:
					SetFieldFloat(this, Lst[i].PropName, Value ?
						Lst[i].FTrue : Lst[i].FFalse);
					break;
				case FLAG_Name:
					SetFieldName(this, Lst[i].PropName, Value ?
						Lst[i].NTrue : Lst[i].NFalse);
					break;
				case FLAG_Class:
					SetFieldClass(this, Lst[i].PropName, Value ?
						Lst[i].NTrue != NAME_None ? VClass::FindClass(*Lst[i].NTrue) : NULL :
						Lst[i].NFalse != NAME_None ? VClass::FindClass(*Lst[i].NFalse) : NULL);
					break;
				case FLAG_NoClip:
					SetFieldBool(this, "bColideWithThings", !Value);
					SetFieldBool(this, "bColideWithWorld", !Value);
					break;
				}
				return;
			}
		}
	}
	GCon->Logf("Unknown flag %s", *Flag);
	unguard;
}
