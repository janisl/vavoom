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

//
//	VThinker
//
//	Doubly linked list of actors and other special elements of a level.
//
class VThinker : public VObject
{
	DECLARE_CLASS(VThinker, VObject, 0)

	VLevel*			XLevel;		//	Level object.
	VLevelInfo*		Level;		//	Level info object.

	VThinker*		Prev;
	VThinker*		Next;

	enum
	{
		TF_AlwaysRelevant			= 0x00000001,
		TF_NetInitial				= 0x00000002,
	};
	vuint32			ThinkerFlags;

	static int		FIndex_Tick;

	VThinker();

	//	VObject interface
	void Destroy();
	void Serialise(VStream&);

	//	VThinker interface
	virtual void Tick(float);
	virtual void DestroyThinker();

	void StartSound(const TVec&, vint32, vint32, vint32, float, float);
	void StopSound(vint32, vint32);
	void StartSoundSequence(const TVec&, vint32, VName, vint32);
	void AddSoundSequenceChoice(vint32, VName);
	void StopSoundSequence(vint32);

	void BroadcastPrintf(const char*, ...);
	void BroadcastCentrePrintf(const char*, ...);

	DECLARE_FUNCTION(Spawn)
	DECLARE_FUNCTION(Destroy)
	DECLARE_FUNCTION(NextThinker)

	//	Print functions
	DECLARE_FUNCTION(bprint)

	DECLARE_FUNCTION(AllocDlight)
	DECLARE_FUNCTION(NewParticle)
};

template <class T> class TThinkerIterator
{
private:
	VThinker* Th;
	void GetNext()
	{
		while (Th && (!Th->IsA(T::StaticClass()) ||
			(Th->GetFlags() & _OF_DelayedDestroy)))
		{
			Th = Th->Next;
		}
	}
public:
	TThinkerIterator(const VLevel* Level)
	{
		Th = Level->ThinkerHead;
		GetNext();
	}
	operator bool()
	{
		return Th != NULL;
	}
	void operator ++()
	{
		if (Th)
		{
			Th = Th->Next;
			GetNext();
		}
	}
	T* operator ->()
	{
		return (T*)Th;
	}
	T* operator *()
	{
		return (T*)Th;
	}
};
