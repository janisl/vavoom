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
//	VThinkerChannel::VThinkerChannel
//
//==========================================================================

VThinkerChannel::VThinkerChannel(VNetConnection* AConnection, vint32 AIndex,
	vuint8 AOpenedLocally)
: VChannel(AConnection, CHANNEL_Thinker, AIndex, AOpenedLocally)
, Thinker(NULL)
, ThinkerClass(NULL)
, OldData(NULL)
, NewObj(false)
, UpdatedThisFrame(false)
, FieldCondValues(NULL)
{
}

//==========================================================================
//
//	VThinkerChannel::~VThinkerChannel
//
//==========================================================================

VThinkerChannel::~VThinkerChannel()
{
	guard(VThinkerChannel::~VThinkerChannel);
	//	Mark channel as closing to prevent sending a message.
	Closing = true;

	//	If this is a client version of entity, destriy it.
	if (Thinker && !OpenedLocally)
	{
		Thinker->XLevel->RemoveThinker(Thinker);
		Thinker->ConditionalDestroy();
	}
	SetThinker(NULL);
	unguard;
}

//==========================================================================
//
//	VThinkerChannel::SetThinker
//
//==========================================================================

void VThinkerChannel::SetThinker(VThinker* AThinker)
{
	guard(VThinkerChannel::SetThinker);
	if (Thinker)
	{
		Connection->ThinkerChannels.Remove(Thinker);
		if (OldData)
		{
			for (VField* F = ThinkerClass->NetFields; F; F = F->NextNetField)
			{
				VField::DestructField(OldData + F->Ofs, F->Type);
			}
			delete[] OldData;
			OldData = NULL;
		}
		if (FieldCondValues)
		{
			delete[] FieldCondValues;
			FieldCondValues = NULL;
		}
	}

	Thinker = AThinker;

	if (Thinker)
	{
		ThinkerClass = Thinker->GetClass();
		if (OpenedLocally)
		{
			VThinker* Def = (VThinker*)ThinkerClass->Defaults;
			OldData = new vuint8[ThinkerClass->ClassSize];
			memset(OldData, 0, ThinkerClass->ClassSize);
			for (VField* F = ThinkerClass->NetFields; F; F = F->NextNetField)
			{
				VField::CopyFieldValue((vuint8*)Def + F->Ofs, OldData + F->Ofs,
					F->Type);
			}
			FieldCondValues = new vuint8[ThinkerClass->NumNetFields];
		}
		NewObj = true;
		Connection->ThinkerChannels.Set(Thinker, this);
	}
	unguard;
}

//==========================================================================
//
//	VThinkerChannel::EvalCondValues
//
//==========================================================================

void VThinkerChannel::EvalCondValues(VObject* Obj, VClass* Class, vuint8* Values)
{
	guard(VThinkerChannel::EvalCondValues);
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
//	VThinkerChannel::Update
//
//==========================================================================

void VThinkerChannel::Update()
{
	guard(VThinkerChannel::Update);
	if (Closing)
	{
		return;
	}

	EvalCondValues(Thinker, Thinker->GetClass(), FieldCondValues);
	vuint8* Data = (vuint8*)Thinker;

	VMessageOut Msg(this);
	Msg.bReliable = true;

	if (NewObj)
	{
		Msg.bOpen = true;
		Msg.WriteInt(Thinker->GetClass()->NetId, VMemberBase::GNetClassLookup.Num());
		NewObj = false;
		Thinker->ThinkerFlags |= VThinker::TF_NetInitial;
	}

	TAVec SavedAngles;
	VEntity* Ent = Cast<VEntity>(Thinker);
	if (Ent)
	{
		SavedAngles = Ent->Angles;
		if (Ent->EntityFlags & VEntity::EF_IsPlayer)
		{
			//	Clear look angles, because they must not affect model orientation
			Ent->Angles.pitch = 0;
			Ent->Angles.roll = 0;
		}
	}
	else
	{
		//	Shut up compiler warnings.
		SavedAngles.yaw = 0;
		SavedAngles.pitch = 0;
		SavedAngles.roll = 0;
	}

	for (VField* F = Thinker->GetClass()->NetFields; F; F = F->NextNetField)
	{
		if (!FieldCondValues[F->NetIndex])
		{
			continue;
		}

		//	Set up pointer to the value and do swapping for the role fields.
		vuint8* FieldValue = Data + F->Ofs;
		if (F == Connection->Context->RoleField)
		{
			FieldValue = Data + Connection->Context->RemoteRoleField->Ofs;
		}
		else if (F == Connection->Context->RemoteRoleField)
		{
			FieldValue = Data + Connection->Context->RoleField->Ofs;
		}

		if (VField::IdenticalValue(FieldValue, OldData + F->Ofs, F->Type))
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
				if (VField::IdenticalValue(FieldValue + i * InnerSize,
					OldData + F->Ofs + i * InnerSize, IntType))
				{
					continue;
				}
				Msg.WriteInt(F->NetIndex, Thinker->GetClass()->NumNetFields);
				Msg.WriteInt(i, F->Type.ArrayDim);
				VField::NetSerialiseValue(Msg, FieldValue + i * InnerSize, IntType);
				VField::CopyFieldValue(FieldValue + i * InnerSize,
					OldData + F->Ofs + i * InnerSize, IntType);
			}
		}
		else
		{
			Msg.WriteInt(F->NetIndex, Thinker->GetClass()->NumNetFields);
			VField::NetSerialiseValue(Msg, FieldValue, F->Type);
			VField::CopyFieldValue(FieldValue, OldData + F->Ofs, F->Type);
		}
	}

	if (Ent && (Ent->EntityFlags & VEntity::EF_IsPlayer))
	{
		Ent->Angles = SavedAngles;
	}
	UpdatedThisFrame = true;

	if (Msg.GetNumBits())
	{
		SendMessage(&Msg);
	}

	//	Clear temporary networking flags.
	Thinker->ThinkerFlags &= ~VThinker::TF_NetInitial;
	unguard;
}

//==========================================================================
//
//	VThinkerChannel::ParsePacket
//
//==========================================================================

void VThinkerChannel::ParsePacket(VMessageIn& Msg)
{
	guard(VThinkerChannel::ParsePacket);
	if (Msg.bOpen)
	{
		int ci = Msg.ReadInt(VMemberBase::GNetClassLookup.Num());
		VClass* C = VMemberBase::GNetClassLookup[ci];
	
		VThinker* Th = Connection->Context->GetLevel()->SpawnThinker(C);
#ifdef CLIENT
		if (Th->IsA(VLevelInfo::StaticClass()))
		{
			VLevelInfo* LInfo = (VLevelInfo*)Th;
			LInfo->Level = LInfo;
			GClLevel->LevelInfo = LInfo;
			cl->Level = LInfo;
			cls.signon = 1;
		}
#endif
		SetThinker(Th);
	}

	VEntity* Ent = Cast<VEntity>(Thinker);
	if (Ent)
	{
		Ent->UnlinkFromWorld();
	}
	while (!Msg.AtEnd())
	{
		int FldIdx = Msg.ReadInt(Thinker->GetClass()->NumNetFields);
		VField* F = NULL;
		for (VField* CF = ThinkerClass->NetFields; CF; CF = CF->NextNetField)
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
				VField::NetSerialiseValue(Msg, (vuint8*)Thinker + F->Ofs +
					Idx * IntType.GetSize(), IntType);
			}
			else
			{
				VField::NetSerialiseValue(Msg, (vuint8*)Thinker + F->Ofs, F->Type);
			}
			continue;
		}

		VMethod* Func = NULL;
		for (VMethod* CM = Thinker->GetClass()->NetMethods; CM; CM = CM->NextNetMethod)
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
			memset(pr_stackPtr, 0, Func->ParamsSize * sizeof(VStack));
			//	Push self pointer
			PR_PushPtr(Thinker);
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
					pr_stackPtr->p = NULL;
					VField::NetSerialiseValue(Msg, (vuint8*)&pr_stackPtr->p, Func->ParamTypes[i]);
					pr_stackPtr++;
					break;
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
	if (Ent)
	{
		Ent->LinkToWorld();
	}
	unguard;
}

//==========================================================================
//
//	VThinkerChannel::Close
//
//==========================================================================

void VThinkerChannel::Close()
{
	guard(VThinkerChannel::Close);
	VChannel::Close();
	SetThinker(NULL);
	unguard;
}
