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

enum ENoInit			{E_NoInit};

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
	void InsertZeroed(int Index, int Count, int ElementSize)
	{
		Insert(Index, Count, ElementSize);
		memset((byte *)Data + Index * ElementSize, 0, Count * ElementSize);
	}
	void Insert(int Index, int Count, int ElementSize)
	{
		int OldNum = ArrayNum;
		if ((ArrayNum += Count) > ArrayMax)
		{
			ArrayMax = ArrayNum + 3 * ArrayNum / 8 + 32;
			Realloc(ElementSize);
		}
		memmove((byte *)Data + (Index + Count) * ElementSize,
			(byte *)Data + Index * ElementSize, 
			(OldNum - Index) * ElementSize);
	}
	int Add(int Count, int ElementSize)
	{
		int Index = ArrayNum;
		if ((ArrayNum += Count) > ArrayMax)
		{
			ArrayMax = ArrayNum + 3 * ArrayNum / 8 + 32;
			Realloc(ElementSize);
		}

		return Index;
	}
	int AddZeroed(int ElementSize, int n = 1)
	{
		int Index = Add(n, ElementSize);
		memset((byte *)Data + Index * ElementSize, 0, n * ElementSize);
		return Index;
	}
	void Shrink(int ElementSize)
	{
		if (ArrayMax != ArrayNum)
		{
			ArrayMax = ArrayNum;
			Realloc(ElementSize);
		}
	}
	void Empty(int ElementSize, int Slack = 0)
	{
		ArrayNum = 0;
		ArrayMax = Slack;
		Realloc(ElementSize);
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
	/*void CountBytes(FArchive& Ar, int ElementSize)
	{
		Ar.CountBytes(ArrayNum * ElementSize, ArrayMax * ElementSize);
	}*/
	void Remove(int Index, int Count, int ElementSize)
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
			Realloc(ElementSize);
		}
	}
protected:
	void Realloc(int ElementSize)
	{
		if (ArrayMax)
		{
			if (Data)
			{
				Z_Resize(&Data, ArrayMax * ElementSize);
			}
			else
			{
				Data = Z_Malloc(ArrayMax * ElementSize, PU_STRING, 0);
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
	FArray(int InNum, int ElementSize)
		: Data(NULL), ArrayNum(InNum), ArrayMax(InNum)
	{
		Realloc(ElementSize);
	}
	void *Data;
	int ArrayNum;
	int ArrayMax;
};

//
// Templated dynamic array.
//
template<class T> class TArray : public FArray
{
public:
	typedef T ElementType;
	TArray(void) : FArray()
	{}
	TArray(int InNum) 
		: FArray(InNum, sizeof(T))
	{}
	TArray(const TArray& Other)
		: FArray(Other.ArrayNum, sizeof(T))
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
		FArray::Shrink(sizeof(T));
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
		return INDEX_NONE;
	}
	/*friend FArchive& operator<<( FArchive& Ar, TArray& A )
	{
		A.CountBytes( Ar );
		if( sizeof(T)==1 )
		{
			// Serialize simple bytes which require no construction or destruction.
			Ar << AR_INDEX(A.ArrayNum);
			if( Ar.IsLoading() )
			{
				A.ArrayMax = A.ArrayNum;
				A.Realloc( sizeof(T) );
			}
			Ar.Serialize( &A(0), A.Num() );
		}
		else if( Ar.IsLoading() )
		{
			// Load array.
			INT NewNum;
			Ar << AR_INDEX(NewNum);
			A.Empty( NewNum );
			for( INT i=0; i<NewNum; i++ )
				Ar << *new(A)T;
		}
		else
		{
			// Save array.
			Ar << AR_INDEX(A.ArrayNum);
			for( INT i=0; i<A.ArrayNum; i++ )
				Ar << A( i );
		}
		return Ar;
	}*/
	/*void CountBytes( FArchive& Ar )
	{
		FArray::CountBytes( Ar, sizeof(T) );
	}*/

	// Add, Insert, Remove, Empty interface.
	int Add(int n = 1)
	{
		return FArray::Add(n, sizeof(T));
	}
	void Insert(int Index, int Count = 1)
	{
		FArray::Insert(Index, Count, sizeof(T));
	}
	void InsertZeroed(int Index, int Count = 1)
	{
		FArray::InsertZeroed( Index, Count, sizeof(T) );
	}
	void Remove(int Index, int Count = 1)
	{
		for (int i = Index; i < Index + Count; i++)
			(&(*this)[i])->~T();
		FArray::Remove(Index, Count, sizeof(T));
	}
	void Empty(int Slack = 0)
	{
		for (int i = 0; i < ArrayNum; i++ )
			(&(*this)[i])->~T();
		FArray::Empty(sizeof(T), Slack);
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
		return FArray::AddZeroed(sizeof(T), n);
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
template <class T> void* operator new(size_t, TArray<T>& Array)
{
	int Index = Array.FArray::Add(1, sizeof(T));
	return &Array[Index];
}
template <class T> void* operator new(size_t, TArray<T>& Array, int Index)
{
	Array.FArray::Insert(Index, 1, sizeof(T));
	return &Array[Index];
}

//==========================================================================
//
//	Dynamic strings.
//
//==========================================================================

//
// A dynamically sizeable string.
//
class FString : protected TArray<char>
{
public:
	FString() : TArray<char>()
	{}
	FString(const FString& Other) : TArray<char>(Other.ArrayNum)
	{
		if (ArrayNum)
			memcpy(&(*this)[0], &Other[0], ArrayNum * sizeof(char));
	}
	FString(const char* In) : TArray<char>(*In ? (strlen(In) + 1) : 0)
	{
		if (ArrayNum)
			memcpy(&(*this)[0], In, ArrayNum * sizeof(char));
	}
	FString(ENoInit) : TArray<char>(E_NoInit)
	{}
	/*explicit FString( BYTE   Arg, INT Digits=1 );
	explicit FString( SBYTE  Arg, INT Digits=1 );
	explicit FString( _WORD  Arg, INT Digits=1 );
	explicit FString( SWORD  Arg, INT Digits=1 );
	explicit FString( INT    Arg, INT Digits=1 );
	explicit FString( DWORD  Arg, INT Digits=1 );
	explicit FString( FLOAT  Arg, INT Digits=1, INT RightDigits=0, UBOOL LeadZero=1 );
	explicit FString( DOUBLE Arg, INT Digits=1, INT RightDigits=0, UBOOL LeadZero=1 );*/
	FString& operator = (const char* Other)
	{
		if (&(*this)[0] != Other)
		{
			ArrayNum = ArrayMax = *Other ? strlen(Other) + 1 : 0;
			Realloc(sizeof(char));
			if (ArrayNum)
				memcpy(&(*this)[0], Other, ArrayNum * sizeof(char));
		}
		return *this;
	}
	FString& operator = (const FString& Other)
	{
		if (this != &Other)
		{
			ArrayNum = ArrayMax = Other.Num();
			Realloc(sizeof(char));
			if (ArrayNum)
				memcpy(&(*this)[0], *Other, ArrayNum * sizeof(char));
		}
		return *this;
	}
	~FString()
	{
		TArray<char>::Empty();		
	}
	void Empty()
	{
		TArray<char>::Empty();
	}
	void Shrink()
	{
		TArray<char>::Shrink();
	}
	const char* operator * () const
	{
		return Num() ? &(*this)[0] : "";
	}
	operator bool() const
	{
		return Num() != 0;
	}
	TArray<char>& GetCharArray()
	{
		//warning: Operations on the TArray<char> can be unsafe, such as adding
		// non-terminating 0's or removing the terminating zero.
		return (TArray<char>&)*this;
	}
	FString& operator += (const char* Str)
	{
		if (ArrayNum)
		{
			int Index = ArrayNum - 1;
			Add(strlen(Str));
			strcpy(&(*this)[Index], Str);
		}
		else if (*Str)
		{
			Add(strlen(Str) + 1);
			strcpy(&(*this)[0], Str);
		}
		return *this;
	}
	FString& operator += (const FString& Str)
	{
		return operator += (*Str);
	}
	FString operator + (const char* Str)
	{
		return FString(*this) += Str;
	}
	FString operator + (const FString& Str)
	{
		return operator + (*Str);
	}
	FString& operator *= (const char* Str)
	{
		if (ArrayNum > 1 && (*this)[ArrayNum - 2] != '/')
			*this += "/";
		return *this += Str;
	}
	FString& operator *= (const FString& Str)
	{
		return operator *= (*Str);
	}
	FString operator * (const char* Str) const
	{
		return FString(*this) *= Str;
	}
	FString operator * (const FString& Str) const
	{
		return operator * (*Str);
	}
	bool operator == (const char* Other) const
	{
		return stricmp(**this, Other) == 0;
	}
	bool operator == (const FString& Other) const
	{
		return stricmp(**this, *Other) == 0;
	}
	bool operator != (const char* Other) const
	{
		return stricmp(**this, Other) != 0;
	}
	bool operator != (const FString& Other) const
	{
		return stricmp(**this, *Other) != 0;
	}
	int Len() const
	{
		return Num() ? Num() - 1 : 0;
	}
	FString Left(int Count) const
	{
		return FString(Clamp(Count, 0, Len()), **this);
	}
	FString LeftChop(int Count) const
	{
		return FString(Clamp(Len() - Count, 0, Len()), **this);
	}
	FString Right(int Count) const
	{
		return FString(**this + Len() - Clamp(Count, 0, Len()));
	}
	FString Mid(int Start, dword Count = MAXINT) const
	{
		dword End = Start + Count;
		Start = Clamp((dword)Start, (dword)0,     (dword)Len());
		End   = Clamp((dword)End,   (dword)Start, (dword)Len());
		return FString(End - Start, **this + Start);
	}
	int InStr(const char* SubStr, bool Right = false) const
	{
		if (!Right)
		{
			const char* Tmp = strstr(**this, SubStr);
			return Tmp ? (Tmp - **this) : -1;
		}
		else
		{
			for (int i = Len() - 1; i >= 0; i--)
			{
				int j;
				for (j = 0; SubStr[j]; j++)
					if ((*this)[i + j] != SubStr[j])
						break;
				if (!SubStr[j])
					return i;
			}
			return -1;
		}
	}
	int InStr(const FString& SubStr, bool Right = false) const
	{
		return InStr(*SubStr, Right);
	}
	bool Split(const FString& InS, FString* LeftS, FString* RightS, bool Right = false) const
	{
		int InPos = InStr(InS, Right);
		if (InPos < 0)
			return 0;
		if (LeftS)
			*LeftS = Left(InPos);
		if (RightS)
			*RightS = Mid(InPos + InS.Len());
		return 1;
	}
	FString Caps() const
	{
		FString New(**this);
		for (int i = 0; i < ArrayNum; i++)
			New[i] = toupper(New[i]);
		return New;
	}
	FString Locs() const
	{
		FString New(**this);
		for (int i = 0; i < ArrayNum; i++)
			New[i] = tolower(New[i]);
		return New;
	}
	//static FString Printf( const TCHAR* Fmt, ... );
	//static FString Chr( TCHAR Ch );
	//friend FArchive& operator<<( FArchive& Ar, FString& S );
private:
	FString(int InCount, const char* InSrc) 
		: TArray<char>(InCount ? InCount + 1 : 0)
	{
		if (ArrayNum)
		{
			strncpy(&(*this)[0], InSrc, InCount);
			(*this)[InCount] = 0;
		}
	}
};
/*inline DWORD GetTypeHash( const FString& S )
{
	return appStrihash(*S);
}*/

//**************************************************************************
//
//	$Log$
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
