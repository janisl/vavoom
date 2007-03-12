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
//**
//**	Template for mapping kays to values.
//**
//**************************************************************************

template<class TK, class TV> class TMap
{
protected:
	struct TPair
	{
		TK			Key;
		TV			Value;
		vint32		HashNext;
	};

	TArray<TPair>	Pairs;
	vint32*			HashTable;
	vint32			HashSize;

	void Rehash()
	{
		guardSlow(TMap::Rehash);
		checkSlow(HashSize >= 16);
		if (HashTable)
		{
			delete[] HashTable;
		}
		HashTable = new vint32[HashSize];
		for (int i = 0; i < HashSize; i++)
		{
			HashTable[i] = -1;
		}
		for (int i = 0; i < Pairs.Num(); i++)
		{
			int Hash = GetTypeHash(Pairs[i].Key) & (HashSize - 1);
			Pairs[i].HashNext = HashTable[Hash];
			HashTable[Hash] = i;
		}
		unguardSlow;
	}
	void Relax()
	{
		guardSlow(TMap::Relax);
		while (HashSize > Pairs.Num() + 16)
		{
			HashSize >>= 1;
		}
		Rehash();
		unguardSlow;
	}

public:
	TMap()
	: HashTable(NULL)
	, HashSize(16)
	{
		guardSlow(TMap::TMap);
		Rehash();
		unguardSlow;
	}
	TMap(TMap& Other)
	: Pairs(Other.Pairs)
	, HashTable(NULL)
	, HashSize(Other.HashSize)
	{
		guardSlow(TMap::TMap);
		Rehash();
		unguardSlow;
	}
	~TMap()
	{
		guardSlow(TMap::~TMap);
		if (HashTable)
		{
			delete[] HashTable;
		}
		HashTable = NULL;
		HashSize = 0;
		unguardSlow;
	}
	TMap& operator = (const TMap& Other)
	{
		guardSlow(TMap::operator=);
		Pairs = Other.Pairs;
		HashSize = Other.HashSize;
		Rehash();
		return *this;
		unguardSlow;
	}
	void Set(TK Key, const TV& Value)
	{
		guardSlow(TMap::Set);
		int HashIndex = GetTypeHash(Key) & (HashSize - 1);
		for (int i = HashTable[HashIndex]; i != -1; i = Pairs[i].HashNext)
		{
			if (Pairs[i].Key == Key)
			{
				Pairs[i].Value = Value;
				return;
			}
		}
		TPair& Pair = Pairs.Alloc();
		Pair.HashNext = HashTable[HashIndex];
		HashTable[HashIndex] = Pairs.Num() - 1;
		Pair.Key = Key;
		Pair.Value = Value;
		if (HashSize * 2 + 16 < Pairs.Num())
		{
			HashSize <<= 1;
			Rehash();
		}
		unguardSlow;
	}
	bool Remove(TK Key)
	{
		guardSlow(TMap::Remove);
		bool Removed = false;
		int HashIndex = GetTypeHash(Key) & (HashSize - 1);
		for (int i = HashTable[HashIndex]; i != -1; i = Pairs[i].HashNext)
		{
			if (Pairs[i].Key == Key)
			{
				Pairs[i].RemoveIndex(i);
				Removed = true;
				break;
			}
		}
		if (Removed)
		{
			Relax();
		}
		return Removed;
		unguardSlow;
	}
	TV* Find(TK Key)
	{
		guardSlow(TMap::Find);
		int HashIndex = GetTypeHash(Key) & (HashSize - 1);
		for (int i = HashTable[HashIndex]; i != -1; i = Pairs[i].HashNext)
		{
			if (Pairs[i].Key == Key)
			{
				return &Pairs[i].Value;
			}
		}
		return NULL;
		unguardSlow;
	}
	const TV* Find(TK Key) const
	{
		guardSlow(TMap::Find);
		int HashIndex = GetTypeHash(Key) & (HashSize - 1);
		for (int i = HashTable[HashIndex]; i != -1; i = Pairs[i].HashNext)
		{
			if (Pairs[i].Key == Key)
			{
				return &Pairs[i].Value;
			}
		}
		return NULL;
		unguardSlow;
	}

	class TIterator
	{
	private:
		TMap& Map;
		vint32 Index;
	public:
		TIterator(TMap& AMap)
		: Map(AMap)
		, Index(0)
		{}
		operator bool() const
		{
			return Index < Map.Pairs.Num();
		}
		void operator ++ ()
		{
			Index++;
		}
		const TK& GetKey() const
		{
			return Map.Pairs[Index].Key;
		}
		const TV& GetValue() const
		{
			return Map.Pairs[Index].Value;
		}
		void RemoveCurrent()
		{
			Map.Pairs.RemoveIndex(Index);
			Map.Relax();
			Index--;
		}
	};

	friend class TIterator;
};
