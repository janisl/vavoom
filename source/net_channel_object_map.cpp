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
//	VObjectMapChannel::VObjectMapChannel
//
//==========================================================================

VObjectMapChannel::VObjectMapChannel(VNetConnection* AConnection,
	vint32 AIndex, vuint8 AOpenedLocally)
: VChannel(AConnection, CHANNEL_ObjectMap, AIndex, AOpenedLocally)
, CurrName(0)
, CurrClass(1)
{
}

//==========================================================================
//
//	VObjectMapChannel::~VObjectMapChannel
//
//==========================================================================

VObjectMapChannel::~VObjectMapChannel()
{
	Connection->ObjMapSent = true;
}

//==========================================================================
//
//	VObjectMapChannel::Tick
//
//==========================================================================

void VObjectMapChannel::Tick()
{
	guard(VObjectMapChannel::Tick);
	VChannel::Tick();
	if (OpenedLocally)
	{
		Update();
	}
	unguard;
}

//==========================================================================
//
//	VObjectMapChannel::Update
//
//==========================================================================

void VObjectMapChannel::Update()
{
	guard(VObjectMapChannel::Update);
	int Cnt = 0;
	for (VMessageOut* M = OutMsg; M; M = M->Next)
	{
		Cnt++;
	}
	if (Cnt >= 10)
	{
		return;
	}

	VMessageOut Msg(this);
	Msg.bReliable = true;

	if (!CurrName)
	{
		//	Opening message, send number of names.
		Msg.bOpen = true;
		vint32 NumNames = Connection->ObjMap->NameLookup.Num();
		Msg << NumNames;
		vint32 NumClasses = Connection->ObjMap->ClassLookup.Num();
		Msg << NumClasses;
	}

	//	Send names while we have anything to send.
	while (CurrName < Connection->ObjMap->NameLookup.Num())
	{
		VNameEntry* E = VName::GetEntry(CurrName);
		int Len = VStr::Length(E->Name);
		//	Send message if this name will not fit
		if (Msg.GetNumBytes() + 1 + Len > OUT_MESSAGE_SIZE / 8)
		{
			SendMessage(&Msg);
			int Cnt = 0;
			for (VMessageOut* M = OutMsg; M; M = M->Next)
			{
				Cnt++;
			}
			if (Cnt >= 10)
			{
				return;
			}
			Msg = VMessageOut(this);
			Msg.bReliable = true;
		}
		Msg.WriteInt(Len, NAME_SIZE);
		Msg.Serialise(E->Name, Len);
		CurrName++;
	}

	//	Send classes while we have anything to send.
	while (CurrClass < Connection->ObjMap->ClassLookup.Num())
	{
		//	Send message if this class will not fit
		if (Msg.GetNumBytes() + 4 > OUT_MESSAGE_SIZE / 8)
		{
			SendMessage(&Msg);
			int Cnt = 0;
			for (VMessageOut* M = OutMsg; M; M = M->Next)
			{
				Cnt++;
			}
			if (Cnt >= 10)
			{
				return;
			}
			Msg = VMessageOut(this);
			Msg.bReliable = true;
		}
		VName Name = Connection->ObjMap->ClassLookup[CurrClass]->GetVName();
		Connection->ObjMap->SerialiseName(Msg, Name);
		CurrClass++;
	}

	//	This is the last message.
	Msg.bClose = true;
	SendMessage(&Msg);
	unguard;
}

//==========================================================================
//
//	VObjectMapChannel::ParsePacket
//
//==========================================================================

void VObjectMapChannel::ParsePacket(VMessageIn& Msg)
{
	guard(VObjectMapChannel::ParsePacket);
	if (Msg.bOpen)
	{
		//	Opening message, read number of names.
		vint32 NumNames;
		Msg << NumNames;
		Connection->ObjMap->NameLookup.SetNum(NumNames);
		vint32 NumClasses;
		Msg << NumClasses;
		Connection->ObjMap->ClassLookup.SetNum(NumClasses);
		Connection->ObjMap->ClassLookup[0] = NULL;
	}

	//	Read names.
	while (!Msg.AtEnd() && CurrName < Connection->ObjMap->NameLookup.Num())
	{
		int Len = Msg.ReadInt(NAME_SIZE);
		char Buf[NAME_SIZE + 1];
		Msg.Serialise(Buf, Len);
		Buf[Len] = 0;
		VName Name = Buf;
		Connection->ObjMap->NameLookup[CurrName] = Name;
		while (Connection->ObjMap->NameMap.Num() <= Name.GetIndex())
		{
			Connection->ObjMap->NameMap.Append(-1);
		}
		Connection->ObjMap->NameMap[Name.GetIndex()] = CurrName;
		CurrName++;
	}

	//	Read classes.
	while (!Msg.AtEnd() && CurrClass < Connection->ObjMap->ClassLookup.Num())
	{
		VName Name;
		Connection->ObjMap->SerialiseName(Msg, Name);
		VClass* C = VMemberBase::StaticFindClass(Name);
		check(C);
		Connection->ObjMap->ClassLookup[CurrClass] = C;
		Connection->ObjMap->ClassMap.Set(C, CurrClass);
		CurrClass++;
	}
	unguard;
}
