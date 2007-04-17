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
#include "network.h"
#include "progdefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VPlayerChannel::VPlayerChannel
//
//==========================================================================

VPlayerChannel::VPlayerChannel(VNetConnection* AConnection, vint32 AIndex,
	vuint8 AOpenedLocally)
: VChannel(AConnection, CHANNEL_Player, AIndex, AOpenedLocally)
, Plr(NULL)
, OldData(NULL)
, NewObj(false)
, FieldCondValues(NULL)
{
}

//==========================================================================
//
//	VPlayerChannel::~VPlayerChannel
//
//==========================================================================

VPlayerChannel::~VPlayerChannel()
{
	SetPlayer(NULL);
}

//==========================================================================
//
//	VPlayerChannel::SetPlayer
//
//==========================================================================

void VPlayerChannel::SetPlayer(VBasePlayer* APlr)
{
	guard(VPlayerChannel::SetPlayer);
	if (Plr)
	{
		for (VField* F = Plr->GetClass()->NetFields; F; F = F->NextNetField)
		{
			VField::CleanField(OldData + F->Ofs, F->Type);
		}
		if (OldData)
		{
			delete[] OldData;
			OldData = NULL;
		}
		if (FieldCondValues)
		{
			delete[] FieldCondValues;
			FieldCondValues = NULL;
		}
	}

	Plr = APlr;

	if (Plr)
	{
		VBasePlayer* Def = (VBasePlayer*)Plr->GetClass()->Defaults;
		OldData = new vuint8[Plr->GetClass()->ClassSize];
		memset(OldData, 0, Plr->GetClass()->ClassSize);
		for (VField* F = Plr->GetClass()->NetFields; F; F = F->NextNetField)
		{
			VField::CopyFieldValue((vuint8*)Def + F->Ofs, OldData + F->Ofs,
				F->Type);
		}
		FieldCondValues = new vuint8[Plr->GetClass()->NumNetFields];
		NewObj = true;
	}
	unguard;
}

//==========================================================================
//
//	VPlayerChannel::EvalCondValues
//
//==========================================================================

void VPlayerChannel::EvalCondValues(VObject* Obj, VClass* Class, vuint8* Values)
{
	guard(VPlayerChannel::EvalCondValues);
	if (Class->GetSuperClass())
	{
		EvalCondValues(Obj, Class->GetSuperClass(), Values);
	}
	for (int i = 0; i < Class->RepInfos.Num(); i++)
	{
		P_PASS_REF(Obj);
		bool Val = !!VObject::ExecuteFunction(Class->RepInfos[i].Cond).i;
		for (int j = 0; j < Class->RepInfos[i].RepMembers.Num(); j++)
		{
			if (Class->RepInfos[i].RepMembers[j]->MemberType != MEMBER_Field)
				continue;
			Values[((VField*)Class->RepInfos[i].RepMembers[j])->NetIndex] = Val;
		}
	}
	unguard;
}

//==========================================================================
//
//	VPlayerChannel::Update
//
//==========================================================================

void VPlayerChannel::Update()
{
	guard(VPlayerChannel::Update);
	EvalCondValues(Plr, Plr->GetClass(), FieldCondValues);

	VMessageOut Msg(this);
	Msg.bReliable = true;
	vuint8* Data = (vuint8*)Plr;
	for (VField* F = Plr->GetClass()->NetFields; F; F = F->NextNetField)
	{
		if (!FieldCondValues[F->NetIndex])
		{
			continue;
		}
		if (VField::IdenticalValue(Data + F->Ofs, OldData + F->Ofs, F->Type))
		{
			continue;
		}
		if (F->Type.Type == ev_array)
		{
			VField::FType IntType = F->Type;
			IntType.Type = F->Type.ArrayInnerType;
			int InnerSize = IntType.GetSize();
			for (int i = 0; i < F->Type.ArrayDim; i++)
			{
				if (VField::IdenticalValue(Data + F->Ofs + i * InnerSize,
					OldData + F->Ofs + i * InnerSize, IntType))
				{
					continue;
				}
				Msg.WriteInt(F->NetIndex, Plr->GetClass()->NumNetFields);
				Msg.WriteInt(i, F->Type.ArrayDim);
				VField::NetSerialiseValue(Msg, Data + F->Ofs + i * InnerSize, IntType);
				VField::CopyFieldValue(Data + F->Ofs + i * InnerSize,
					OldData + F->Ofs + i * InnerSize, IntType);
			}
		}
		else
		{
			Msg.WriteInt(F->NetIndex, Plr->GetClass()->NumNetFields);
			VField::NetSerialiseValue(Msg, Data + F->Ofs, F->Type);
			VField::CopyFieldValue(Data + F->Ofs, OldData + F->Ofs, F->Type);
		}
	}

	if (Msg.GetNumBits())
	{
		SendMessage(&Msg);
	}
	unguard;
}

//==========================================================================
//
//	VPlayerChannel::ParsePacket
//
//==========================================================================

void VPlayerChannel::ParsePacket(VMessageIn& Msg)
{
	guard(VPlayerChannel::ParsePacket);
	while (!Msg.AtEnd())
	{
		int FldIdx = Msg.ReadInt(Plr->GetClass()->NumNetFields);
		VField* F = NULL;
		for (VField* CF = Plr->GetClass()->NetFields; CF; CF = CF->NextNetField)
		{
			if (CF->NetIndex == FldIdx)
			{
				F = CF;
				break;
			}
		}
		if (F)
		{
			if (F->Type.Type == ev_array)
			{
				int Idx = Msg.ReadInt(F->Type.ArrayDim);
				VField::FType IntType = F->Type;
				IntType.Type = F->Type.ArrayInnerType;
				VField::NetSerialiseValue(Msg, (vuint8*)Plr + F->Ofs +
					Idx * IntType.GetSize(), IntType);
			}
			else
			{
				VField::NetSerialiseValue(Msg, (vuint8*)Plr + F->Ofs, F->Type);
			}
			continue;
		}

		VMethod* Func = NULL;
		for (VMethod* CM = Plr->GetClass()->NetMethods; CM; CM = CM->NextNetMethod)
		{
			if (CM->NetIndex == FldIdx)
			{
				Func = CM;
				break;
			}
		}
		if (Func)
		{
			guard(RPC);
			//	Push self pointer
			PR_PushPtr(Plr);
			//	Get arguments
			for (int i = 0; i < Func->NumParams; i++)
			{
				switch (Func->ParamTypes[i].Type)
				{
				case ev_int:
				case ev_name:
				case ev_bool:
					VField::NetSerialiseValue(Msg, (vuint8*)&pr_stackPtr->i, Func->ParamTypes[i]);
					pr_stackPtr++;
					break;
				case ev_float:
					VField::NetSerialiseValue(Msg, (vuint8*)&pr_stackPtr->f, Func->ParamTypes[i]);
					pr_stackPtr++;
					break;
				case ev_string:
				case ev_pointer:
				case ev_reference:
				case ev_class:
				case ev_state:
					VField::NetSerialiseValue(Msg, (vuint8*)&pr_stackPtr->p, Func->ParamTypes[i]);
					pr_stackPtr++;
					break;
				case ev_vector:
					{
						TVec Vec;
						VField::NetSerialiseValue(Msg, (vuint8*)&Vec, Func->ParamTypes[i]);
						PR_Pushv(Vec);
					}
					break;
				default:
					Sys_Error("Bad method argument type %d", Func->ParamTypes[i].Type);
				}
			}
			//	Execute it
			VObject::ExecuteFunction(Func);
			//	If it returns a vector, pop the rest of values
			if (Func->ReturnType.Type == ev_vector)
			{
				PR_Pop();
				PR_Pop();
			}
			unguard;
			continue;
		}

		Sys_Error("Bad net field %d", FldIdx);
	}
	unguard;
}
