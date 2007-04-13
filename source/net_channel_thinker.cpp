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
		for (VField* F = ThinkerClass->NetFields; F; F = F->NextNetField)
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
		for (int j = 0; j < Class->RepInfos[i].RepFields.Num(); j++)
		{
			Values[Class->RepInfos[i].RepFields[j]->NetIndex] = Val;
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
				Msg << (vuint8)F->NetIndex;
				Msg.WriteInt(i, F->Type.ArrayDim);
				VField::NetSerialiseValue(Msg, Data + F->Ofs + i * InnerSize, IntType);
				VField::CopyFieldValue(Data + F->Ofs + i * InnerSize,
					OldData + F->Ofs + i * InnerSize, IntType);
			}
		}
		else
		{
			Msg << (vuint8)F->NetIndex;
			VField::NetSerialiseValue(Msg, Data + F->Ofs, F->Type);
			VField::CopyFieldValue(Data + F->Ofs, OldData + F->Ofs, F->Type);
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
			LInfo->Game = NULL;
			LInfo->World = NULL;
			GClLevel->LevelInfo = LInfo;
			cl->Level = LInfo;
		}
#endif
		if (Th->IsA(VEntity::StaticClass()))
		{
			VEntity* Ent = (VEntity*)Th;
			Ent->Role = ROLE_DumbProxy;
			Ent->RemoteRole = ROLE_Authority;
		}
		SetThinker(Th);
	}

	while (!Msg.AtEnd())
	{
		int FldIdx = Msg.ReadByte();
		VField* F = NULL;
		for (VField* CF = ThinkerClass->NetFields; CF; CF = CF->NextNetField)
		{
			if (CF->NetIndex == FldIdx)
			{
				F = CF;
				break;
			}
		}
		if (!F)
		{
			Sys_Error("Bad net field %d", FldIdx);
		}
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
