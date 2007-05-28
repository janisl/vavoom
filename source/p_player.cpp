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

#include "gamedefs.h"
#include "progdefs.h"
#include "network.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, BasePlayer)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	EvalCond
//
//==========================================================================

static bool EvalCond(VObject* Obj, VClass* Class, VMethod* M)
{
	guard(EvalCond);
	for (int i = 0; i < Class->RepInfos.Num(); i++)
	{
		for (int j = 0; j < Class->RepInfos[i].RepMembers.Num(); j++)
		{
			if (Class->RepInfos[i].RepMembers[j]->MemberType != MEMBER_Method)
				continue;
			if (Class->RepInfos[i].RepMembers[j]->Name != M->Name)
				continue;
			P_PASS_REF(Obj);
			return !!VObject::ExecuteFunction(Class->RepInfos[i].Cond).i;
		}
	}
	if (Class->GetSuperClass())
	{
		return EvalCond(Obj, Class->GetSuperClass(), M);
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::ExecuteNetMethod
//
//==========================================================================

bool VBasePlayer::ExecuteNetMethod(VMethod* Func)
{
	guard(VBasePlayer::ExecuteNetMethod);
	if (host_standalone)
	{
		return false;
	}

	if (!EvalCond(this, GetClass(), Func))
	{
		return false;
	}

	VMessageOut Msg(Net->Channels[CHANIDX_Player]);
	Msg.bReliable = !!(Func->Flags & FUNC_NetReliable);

	Msg.WriteInt(Func->NetIndex, GetClass()->NumNetFields);

	//	Serialise arguments
	guard(SerialiseArguments);
	VStack* Param = pr_stackPtr - Func->ParamsSize + 1;	//	Skip self
	for (int i = 0; i < Func->NumParams; i++)
	{
		switch (Func->ParamTypes[i].Type)
		{
		case TYPE_Int:
		case TYPE_Byte:
		case TYPE_Bool:
		case TYPE_Name:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->i, Func->ParamTypes[i]);
			Param++;
			break;
		case TYPE_Float:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->f, Func->ParamTypes[i]);
			Param++;
			break;
		case TYPE_String:
		case TYPE_Pointer:
		case TYPE_Reference:
		case TYPE_Class:
		case TYPE_State:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->p, Func->ParamTypes[i]);
			Param++;
			break;
		case TYPE_Vector:
			{
				TVec Vec;
				Vec.x = Param[0].f;
				Vec.y = Param[1].f;
				Vec.z = Param[2].f;
				VField::NetSerialiseValue(Msg, (vuint8*)&Vec, Func->ParamTypes[i]);
				Param += 3;
			}
			break;
		default:
			Sys_Error("Bad method argument type %d", Func->ParamTypes[i].Type);
		}
	}
	unguard;

	//	Send it.
	Net->Channels[CHANIDX_Player]->SendMessage(&Msg);

	//	Clean up parameters
	guard(CleanUp);
	VStack* Param = pr_stackPtr - Func->ParamsSize + 1;	//	Skip self
	for (int i = 0; i < Func->NumParams; i++)
	{
		switch (Func->ParamTypes[i].Type)
		{
		case TYPE_Int:
		case TYPE_Byte:
		case TYPE_Bool:
		case TYPE_Float:
		case TYPE_Name:
		case TYPE_Pointer:
		case TYPE_Reference:
		case TYPE_Class:
		case TYPE_State:
			Param++;
			break;
		case TYPE_String:
			((VStr*)&Param->p)->Clean();
			Param++;
			break;
		case TYPE_Vector:
			Param += 3;
			break;
		default:
			Sys_Error("Bad method argument type %d", Func->ParamTypes[i].Type);
		}
	}
	pr_stackPtr -= Func->ParamsSize;
	unguard;

	//	Push null return value
	guard(RetVal);
	switch (Func->ReturnType.Type)
	{
	case TYPE_Void:
		break;
	case TYPE_Int:
	case TYPE_Byte:
	case TYPE_Bool:
	case TYPE_Name:
		PR_Push(0);
		break;
	case TYPE_Float:
		PR_Pushf(0);
		break;
	case TYPE_String:
		PR_PushStr(VStr());
		break;
	case TYPE_Pointer:
	case TYPE_Reference:
	case TYPE_Class:
	case TYPE_State:
		PR_PushPtr(NULL);
		break;
	case TYPE_Vector:
		PR_Pushf(0);
		PR_Pushf(0);
		PR_Pushf(0);
		break;
	default:
		Sys_Error("Bad return value type");
	}
	unguard;

	//	It's been handled here.
	return true;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::Printf
//
//==========================================================================

void VBasePlayer::Printf(const char *s, ...)
{
	guard(VBasePlayer::Printf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	eventClientPrint(buf);
	unguard;
}

//==========================================================================
//
//	VBasePlayer::CentrePrintf
//
//==========================================================================

void VBasePlayer::CentrePrintf(const char *s, ...)
{
	guard(VBasePlayer::CentrePrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	eventClientCentrePrint(buf);
	unguard;
}

//===========================================================================
//
//  VBasePlayer::SetViewState
//
//===========================================================================

void VBasePlayer::SetViewState(int position, VState* stnum)
{
	guard(VBasePlayer::SetViewState);
	VViewState& VSt = ViewStates[position];
	VState *state = stnum;
	do
	{
		if (!state)
		{
			// Object removed itself.
			VSt.State = NULL;
			VSt.StateTime = -1;
			break;
		}
		VSt.State = state;
		VSt.StateTime = state->Time;	// could be 0
		if (state->Misc1)
		{
			VSt.SX = state->Misc1;
		}
		if (state->Misc2)
		{
			VSt.SY = state->Misc2;
		}
		// Call action routine.
		if (state->Function)
		{
			P_PASS_REF(ViewEnt);
			ExecuteFunction(state->Function);
			if (!VSt.State)
			{
				break;
			}
		}
		state = VSt.State->NextState;
	}
	while (!VSt.StateTime);	// An initial state of 0 could cycle through.
	unguard;
}

//==========================================================================
//
//	VBasePlayer::AdvanceViewStates
//
//==========================================================================

void VBasePlayer::AdvanceViewStates(float deltaTime)
{
	for (int i = 0; i < NUMPSPRITES; i++)
	{
		VViewState& St = ViewStates[i];
		// a null state means not active
		if (St.State)
		{
			// drop tic count and possibly change state
			// a -1 tic count never changes
			if (St.StateTime != -1.0)
			{
				St.StateTime -= deltaTime;
				if (St.StateTime <= 0.0)
				{
					St.StateTime = 0.0;
					SetViewState(i, St.State->NextState);
				}
			}
		}
	}
}

//==========================================================================
//
//	Natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VBasePlayer, cprint)
{
	VStr msg = PF_FormatString();
	P_GET_SELF;
	Self->Printf(*msg);
}

IMPLEMENT_FUNCTION(VBasePlayer, centreprint)
{
	VStr msg = PF_FormatString();
	P_GET_SELF;
	Self->CentrePrintf(*msg);
}

IMPLEMENT_FUNCTION(VBasePlayer, GetPlayerNum)
{
	P_GET_SELF;
	RET_INT(SV_GetPlayerNum(Self));
}

IMPLEMENT_FUNCTION(VBasePlayer, ClearPlayer)
{
	P_GET_SELF;

	Self->PClass = 0;
	Self->ForwardMove = 0;
	Self->SideMove = 0;
	Self->FlyMove = 0;
	Self->Buttons = 0;
	Self->Impulse = 0;
	Self->MO = NULL;
	Self->PlayerState = 0;
	Self->ViewOrg = TVec(0, 0, 0);
	Self->PlayerFlags &= ~VBasePlayer::PF_FixAngle;
	Self->Health = 0;
	Self->Items = 0;
	Self->PlayerFlags &= ~VBasePlayer::PF_AttackDown;
	Self->PlayerFlags &= ~VBasePlayer::PF_UseDown;
	Self->ExtraLight = 0;
	Self->FixedColourmap = 0;
	memset(Self->CShifts, 0, sizeof(Self->CShifts));
	Self->PSpriteSY = 0;

	vuint8* Def = Self->GetClass()->Defaults;
	for (VField* F = Self->GetClass()->Fields; F; F = F->Next)
	{
		VField::CopyFieldValue(Def + F->Ofs, (vuint8*)Self + F->Ofs, F->Type);
	}
}

IMPLEMENT_FUNCTION(VBasePlayer, SetViewState)
{
	P_GET_PTR(VState, stnum);
	P_GET_INT(position);
	P_GET_SELF;
	Self->SetViewState(position, stnum);
}

IMPLEMENT_FUNCTION(VBasePlayer, AdvanceViewStates)
{
	P_GET_FLOAT(deltaTime);
	P_GET_SELF;
	Self->AdvanceViewStates(deltaTime);
}
