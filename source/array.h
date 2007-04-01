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
//**	Dynamic array template.
//**
//**************************************************************************

template<class T> class TArray
{
public:
	TArray()
	: ArrData(0)
	{
		Clear();
	}
	TArray(ENoInit)
	{}
	TArray(const TArray<T>& Other)
	: ArrData(0)
	{
		*this = Other;
	}
	~TArray()
	{
		Clear();
	}

	void Clear()
	{
		if (ArrData)
		{
			delete[] ArrData;
		}
		ArrData = NULL;
		ArrNum = 0;
		ArrSize = 0;
	}
	int Num() const
	{
		return ArrNum;
	}
	int NumAllocated() const
	{
		return ArrSize;
	}
	T* Ptr()
	{
		return ArrData;
	}
	const T* Ptr() const
	{
		return ArrData;
	}

	void Resize(int NewSize)
	{
		check(NewSize >= 0);

		if (NewSize <= 0)
		{
			Clear();
			return;
		}

		if (NewSize == ArrSize)
		{
			return;
		}
		T* OldData = ArrData;
		ArrSize = NewSize;
		if (ArrNum > NewSize)
		{
			ArrNum = NewSize;
		}

		ArrData = new T[ArrSize];
		for (int i = 0; i < ArrNum; i++)
		{
			ArrData[i] = OldData[i];
		}

		if (OldData)
		{
			delete[] OldData;
		}
	}
	void SetNum(int NewNum, bool bResize = true)
	{
		check(NewNum >= 0);
		if (bResize || NewNum > ArrSize)
		{
			Resize(NewNum);
		}
		ArrNum = NewNum;
	}
	void Condense()
	{
		Resize(ArrNum);
	}

	TArray<T>& operator=(const TArray<T>& Other)
	{
		Clear();

		ArrNum = Other.ArrNum;
		ArrSize = Other.ArrSize;
		if (ArrSize)
		{
			ArrData = new T[ArrSize];
			for (int i = 0; i < ArrNum; i++)
			{
				ArrData[i] = Other.ArrData[i];
			}
		}
		return *this;
	}
	T& operator[](int Index)
	{
		check(Index >= 0);
		check(Index < ArrNum);
		return ArrData[Index];
	}
	const T& operator[](int Index) const
	{
		check(Index >= 0);
		check(Index < ArrNum);
		return ArrData[Index];
	}

	int Append(const T& Item)
	{
		if (ArrNum == ArrSize)
		{
			Resize(ArrSize + ArrSize * 3 / 8 + 32);
		}
		ArrData[ArrNum] = Item;
		ArrNum++;
		return ArrNum - 1;
	}
	T& Alloc()
	{
		if (ArrNum == ArrSize)
		{
			Resize(ArrSize + ArrSize * 3 / 8 + 32);
		}

		return ArrData[ArrNum++];
	}
	bool RemoveIndex(int Index)
	{
		check(ArrData != NULL);
		check(Index >= 0);
		check(Index < ArrNum);
	
		if (Index < 0 || Index >= ArrNum)
		{
			return false;
		}
	
		ArrNum--;
		for (int i = Index; i < ArrNum; i++)
		{
			ArrData[i] = ArrData[i + 1];
		}
	
		return true;
	}
	void Remove(const T& Item)
	{
		for (int i = 0; i < ArrNum; i++)
		{
			if (ArrData[i] == Item)
			{
				RemoveIndex(i--);
			}
		}
	}

private:
	int ArrNum;
	int ArrSize;
	T* ArrData;
};
