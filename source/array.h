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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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

template<class T> T Clamp(T val, T low, T high)
{
	return val < low ? low : val > high ? high : val;
}

//
// Base dynamic array.
//
class FArray
{
public:
	void* GetData(void)
	{
		return Data;
	}
	const void* GetData(void) const
	{
		return Data;
	}
	bool IsValidIndex(int i) const
	{
		return i >= 0 && i < ArrayNum;
	}
	int Num(void) const
	{
		return ArrayNum;
	}
	void InsertZeroed(int Index, int Count, int ElementSize, EZoneTag Tag)
	{
		Insert(Index, Count, ElementSize, Tag);
		memset((byte *)Data + Index * ElementSize, 0, Count * ElementSize);
	}
	void Insert(int Index, int Count, int ElementSize, EZoneTag Tag)
	{
		int OldNum = ArrayNum;
		if ((ArrayNum += Count) > ArrayMax)
		{
			ArrayMax = ArrayNum + 3 * ArrayNum / 8 + 32;
			Realloc(ElementSize, Tag);
		}
		memmove((byte *)Data + (Index + Count) * ElementSize,
			(byte *)Data + Index * ElementSize, 
			(OldNum - Index) * ElementSize);
	}
	int Add(int Count, int ElementSize, EZoneTag Tag)
	{
		int Index = ArrayNum;
		if ((ArrayNum += Count) > ArrayMax)
		{
			ArrayMax = ArrayNum + 3 * ArrayNum / 8 + 32;
			Realloc(ElementSize, Tag);
		}

		return Index;
	}
	int AddZeroed(int ElementSize, EZoneTag Tag, int n = 1)
	{
		int Index = Add(n, ElementSize, Tag);
		memset((byte *)Data + Index * ElementSize, 0, n * ElementSize);
		return Index;
	}
	void Shrink(int ElementSize, EZoneTag Tag)
	{
		if (ArrayMax != ArrayNum)
		{
			ArrayMax = ArrayNum;
			Realloc(ElementSize, Tag);
		}
	}
	void Empty(int ElementSize, EZoneTag Tag, int Slack = 0)
	{
		ArrayNum = 0;
		ArrayMax = Slack;
		Realloc(ElementSize, Tag);
	}
	FArray(void) : Data(NULL), ArrayNum(0), ArrayMax(0)
	{}
	FArray(ENoInit)
	{}
	~FArray(void)
	{
		if (Data)
			Z_Free(Data);
		Data = NULL;
		ArrayNum = ArrayMax = 0;
	}
	void Remove(int Index, int Count, int ElementSize, EZoneTag Tag)
	{
		if (ArrayNum > Index + Count)
		{
			memmove((byte *)Data + Index * ElementSize,
				(byte *)Data + (Index + Count) * ElementSize,
				(ArrayNum - (Index + Count)) * ElementSize);
		}
		ArrayNum -= Count;
		if (ArrayNum + ArrayNum / 2 + 32 < ArrayMax)
		{
			ArrayMax = ArrayNum + 3 * ArrayNum / 8 + 32;
			Realloc(ElementSize, Tag);
		}
	}
protected:
	void Realloc(int ElementSize, EZoneTag Tag)
	{
		if (ArrayMax)
		{
			if (Data)
			{
				Z_Resize(&Data, ArrayMax * ElementSize);
			}
			else
			{
				Data = Z_Malloc(ArrayMax * ElementSize, Tag, 0);
			}
		}
		else
		{
			if (Data)
			{
				Z_Free(Data);
				Data = NULL;
			}
		}
	}
	FArray(int InNum, int ElementSize, EZoneTag Tag)
		: Data(NULL), ArrayNum(InNum), ArrayMax(InNum)
	{
		Realloc(ElementSize, Tag);
	}
	void *Data;
	int ArrayNum;
	int ArrayMax;
};

//
// Templated dynamic array.
//
template<class T, EZoneTag Tag = PU_STATIC> class TArray : public FArray
{
public:
	typedef T ElementType;
	TArray(void) : FArray()
	{}
	TArray(int InNum) 
		: FArray(InNum, sizeof(T), Tag)
	{}
	TArray(const TArray& Other)
		: FArray(Other.ArrayNum, sizeof(T), Tag)
	{
		ArrayNum = 0;
		for (int i = 0; i < Other.ArrayNum; i++)
			new (*this)T(Other[i]);
	}
	TArray(ENoInit) : FArray(E_NoInit)
	{}
	~TArray()
	{
		Remove(0, ArrayNum);
	}
	T& operator[](int i)
	{
		return ((T*)Data)[i];
	}
	const T& operator[](int i) const
	{
		return ((T*)Data)[i];
	}
	T Pop()
	{
		T Result = ((T*)Data)[ArrayNum - 1];
		Remove(ArrayNum - 1);
		return Result;
	}
	T& Last(int c = 0)
	{
		return ((T*)Data)[ArrayNum - c - 1];
	}
	const T& Last(int c = 0) const
	{
		return ((T*)Data)[ArrayNum - c - 1];
	}
	void Shrink()
	{
		FArray::Shrink(sizeof(T), Tag);
	}
	bool FindItem(const T& Item, int& Index) const
	{
		for (Index = 0; Index < ArrayNum; Index++)
			if ((*this)[Index] == Item)
				return 1;
		return 0;
	}
	int FindItemIndex(const T& Item) const
	{
		for (int Index = 0; Index < ArrayNum; Index++ )
			if ((*this)[Index] == Item)
				return Index;
		return -1;
	}

	// Add, Insert, Remove, Empty interface.
	int Add(int n = 1)
	{
		return FArray::Add(n, sizeof(T), Tag);
	}
	void Insert(int Index, int Count = 1)
	{
		FArray::Insert(Index, Count, sizeof(T), Tag);
	}
	void InsertZeroed(int Index, int Count = 1)
	{
		FArray::InsertZeroed(Index, Count, sizeof(T), Tag);
	}
	void Remove(int Index, int Count = 1)
	{
		for (int i = Index; i < Index + Count; i++)
			(&(*this)[i])->~T();
		FArray::Remove(Index, Count, sizeof(T), Tag);
	}
	void Empty(int Slack = 0)
	{
		for (int i = 0; i < ArrayNum; i++ )
			(&(*this)[i])->~T();
		FArray::Empty(sizeof(T), Tag, Slack);
	}

	// Functions dependent on Add, Remove.
	TArray& operator = (const TArray& Other)
	{
		if (this != &Other)
		{
			Empty(Other.ArrayNum);
			for(int i = 0; i < Other.ArrayNum; i++ )
				new(*this) T(Other[i]);
		}
		return *this;
	}
	int AddItem(const T& Item)
	{
		int Index = Add();
		(*this)[Index] = Item;
		return Index;
	}
	int AddZeroed(int n = 1)
	{
		return FArray::AddZeroed(sizeof(T), Tag, n);
	}
	int AddUniqueItem(const T& Item)
	{
		for (int Index = 0; Index < ArrayNum; Index++)
			if ((*this)[Index] == Item)
				return Index;
		return AddItem(Item);
	}
	int RemoveItem(const T& Item)
	{
		int OriginalNum = ArrayNum;
		for (int Index = 0; Index < ArrayNum; Index++)
			if ((*this)[Index] == Item)
				Remove(Index--);
		return OriginalNum - ArrayNum;
	}

	// Iterator.
	class TIterator
	{
	public:
		TIterator(TArray<T>& InArray) : Array(InArray), Index(-1) { ++*this; }
		void operator++(void) { ++Index; }
		void RemoveCurrent(void) { Array.Remove(Index--); }
		int GetIndex(void) const { return Index; }
		operator bool(void) const { return Index < Array.Num(); }
		T& operator*(void) const { return Array[Index]; }
		T* operator->(void) const { return &Array[Index]; }
		T& GetCurrent(void) const { return Array[Index]; }
		T& GetPrev(void) const { return Array[Index ? Index - 1 : Array.Num() - 1]; }
		T& GetNext(void) const { return Array[Index < Array.Num() - 1 ? Index + 1 : 0]; }
	private:
		TArray<T>& Array;
		int Index;
	};
};

//
// Array operator news.
//
template <class T, EZoneTag Tag> void* operator new(size_t, TArray<T, Tag>& Array)
{
	int Index = Array.FArray::Add(1, sizeof(T), Tag);
	return &Array[Index];
}
template <class T, EZoneTag Tag> void* operator new(size_t, TArray<T, Tag>& Array, int Index)
{
	Array.FArray::Insert(Index, 1, sizeof(T), Tag);
	return &Array[Index];
}

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//
//	Revision 1.9  2006/02/21 22:31:44  dj_jl
//	Created dynamic string class.
//	
//	Revision 1.8  2004/12/03 16:15:46  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.7  2004/08/21 14:54:06  dj_jl
//	Fix.
//	
//	Revision 1.6  2003/09/26 16:59:23  dj_jl
//	Added zone tag to array
//	
//	Revision 1.5  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.4  2002/01/12 18:03:28  dj_jl
//	Removed zone tag
//	
//	Revision 1.3  2002/01/11 18:24:01  dj_jl
//	Added dynamic strings
//	
//	Revision 1.2  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/12 19:26:40  dj_jl
//	Added dynamic arrays
//	
//**************************************************************************
