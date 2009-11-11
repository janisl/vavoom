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
//	Dynamic string class.
//
//**************************************************************************

class VStr
{
private:
	char*		Str;

	void Resize(int NewLen);

public:
	//	Constructors.
	VStr()
	: Str(NULL)
	{}
	VStr(ENoInit)
	{}
	VStr(const char* InStr)
	: Str(NULL)
	{
		if (*InStr)
		{
			Resize(Length(InStr));
			Cpy(Str, InStr);
		}
	}
	VStr(const VStr& InStr)
	: Str(NULL)
	{
		if (InStr.Str)
		{
			Resize(InStr.Length());
			Cpy(Str, InStr.Str);
		}
	}
	VStr(const VStr& InStr, int Start, int Len);
	explicit VStr(char InChr)
	: Str(NULL)
	{
		Resize(1);
		Str[0] = InChr;
	}
	explicit VStr(bool InBool)
	: Str(NULL)
	{
		if (InBool)
		{
			Resize(4);
			Cpy(Str, "true");
		}
		else
		{
			Resize(5);
			Cpy(Str, "false");
		}
	}
	explicit VStr(int InInt)
	: Str(NULL)
	{
		char Buf[64];

		sprintf(Buf, "%d", InInt);
		Resize(Length(Buf));
		Cpy(Str, Buf);
	}
	explicit VStr(unsigned InInt)
	: Str(NULL)
	{
		char Buf[64];

		sprintf(Buf, "%u", InInt);
		Resize(Length(Buf));
		Cpy(Str, Buf);
	}
	explicit VStr(float InFloat)
	: Str(NULL)
	{
		char Buf[64];

		sprintf(Buf, "%f", InFloat);
		Resize(Length(Buf));
		Cpy(Str, Buf);
	}
	explicit VStr(const VName& InName)
	: Str(NULL)
	{
		Resize(Length(*InName));
		Cpy(Str, *InName);
	}

	//	Destructor.
	~VStr()
	{
		Clean();
	}

	//	Clears the string.
	void Clean()
	{
		Resize(0);
	}
	//	Return length of the string.
	size_t Length() const
	{
		return Str ? ((int*)Str)[-1] : 0;
	}
	//	Return number of characters in a UTF-8 string.
	size_t Utf8Length() const
	{
		return Str ? Utf8Length(Str) : 0;
	}
	//	Return C string.
	const char* operator*() const
	{
		return Str ? Str : "";
	}
	//	Check if string is empty.
	bool IsEmpty() const
	{
		return !Str;
	}
	bool IsNotEmpty() const
	{
		return !!Str;
	}

	//	Character ancestors.
	char operator[](int Idx) const
	{
		return Str[Idx];
	}
	char& operator[](int Idx)
	{
		return Str[Idx];
	}

	//	Assignement operators.
	VStr& operator=(const char* InStr)
	{
		Resize(Length(InStr));
		if (*InStr)
			Cpy(Str, InStr);
		return *this;
	}
	VStr& operator=(const VStr& InStr)
	{
		Resize(InStr.Length());
		if (InStr.Str)
			Cpy(Str, InStr.Str);
		return *this;
	}

	//	Concatenation operators.
	VStr& operator+=(const char* InStr)
	{
		if (*InStr)
		{
			int l = Length();
			Resize(l + Length(InStr));
			Cpy(Str + l, InStr);
		}
		return *this;
	}
	VStr& operator+=(const VStr& InStr)
	{
		if (InStr.Length())
		{
			int l = Length();
			Resize(l + InStr.Length());
			Cpy(Str + l, *InStr);
		}
		return *this;
	}
	VStr& operator+=(char InChr)
	{
		int l = Length();
		Resize(l + 1);
		Str[l] = InChr;
		return *this;
	}
	VStr& operator+=(bool InBool)
	{
		return operator+=(InBool ? "true" : "false");
	}
	VStr& operator+=(int InInt)
	{
		char Buf[64];

		sprintf(Buf, "%d", InInt);
		return operator+=(Buf);
	}
	VStr& operator+=(unsigned InInt)
	{
		char Buf[64];

		sprintf(Buf, "%u", InInt);
		return operator+=(Buf);
	}
	VStr& operator+=(float InFloat)
	{
		char Buf[64];

		sprintf(Buf, "%f", InFloat);
		return operator+=(Buf);
	}
	VStr& operator+=(const VName& InName)
	{
		return operator+=(*InName);
	}
	friend VStr operator+(const VStr& S1, const char* S2)
	{
		VStr Ret(S1);
		Ret += S2;
		return Ret;
	}
	friend VStr operator+(const VStr& S1, const VStr& S2)
	{
		VStr Ret(S1);
		Ret += S2;
		return Ret;
	}
	friend VStr operator+(const VStr& S1, char S2)
	{
		VStr Ret(S1);
		Ret += S2;
		return Ret;
	}
	friend VStr operator+(const VStr& S1, bool InBool)
	{
		VStr Ret(S1);
		Ret += InBool;
		return Ret;
	}
	friend VStr operator+(const VStr& S1, int InInt)
	{
		VStr Ret(S1);
		Ret += InInt;
		return Ret;
	}
	friend VStr operator+(const VStr& S1, unsigned InInt)
	{
		VStr Ret(S1);
		Ret += InInt;
		return Ret;
	}
	friend VStr operator+(const VStr& S1, float InFloat)
	{
		VStr Ret(S1);
		Ret += InFloat;
		return Ret;
	}
	friend VStr operator+(const VStr& S1, const VName& InName)
	{
		VStr Ret(S1);
		Ret += InName;
		return Ret;
	}

	//	Comparison operators.
	friend bool operator==(const VStr& S1, const char* S2)
	{
		return !Cmp(*S1, S2);
	}
	friend bool operator==(const VStr& S1, const VStr& S2)
	{
		return !Cmp(*S1, *S2);
	}
	friend bool operator!=(const VStr& S1, const char* S2)
	{
		return !!Cmp(*S1, S2);
	}
	friend bool operator!=(const VStr& S1, const VStr& S2)
	{
		return !!Cmp(*S1, *S2);
	}

	//	Comparison functions.
	int Cmp(const char* S2) const
	{
		return Cmp(**this, S2);
	}
	int Cmp(const VStr& S2) const
	{
		return Cmp(**this, *S2);
	}
	int ICmp(const char* S2) const
	{
		return ICmp(**this, S2);
	}
	int ICmp(const VStr& S2) const
	{
		return ICmp(**this, *S2);
	}

	bool StartsWith(const char*) const;
	bool StartsWith(const VStr&) const;
	bool EndsWith(const char*) const;
	bool EndsWith(const VStr&) const;

	VStr ToLower() const;
	VStr ToUpper() const;

	int IndexOf(char) const;
	int IndexOf(const char*) const;
	int IndexOf(const VStr&) const;
	int LastIndexOf(char) const;
	int LastIndexOf(const char*) const;
	int LastIndexOf(const VStr&) const;

	VStr Replace(const char*, const char*) const;
	VStr Replace(const VStr&, const VStr&) const;

	VStr Utf8Substring(int, int) const;

	void Split(char, TArray<VStr>&) const;
	void Split(const char*, TArray<VStr>&) const;

	bool IsValidUtf8() const;
	VStr Latin1ToUtf8() const;

	//	Serialisation operator.
	friend VStream& operator<<(VStream& Strm, VStr& S)
	{
		if (Strm.IsLoading())
		{
			vint32 Len;
			Strm << STRM_INDEX(Len);
			if (Len < 0)
				Len = 0;
			S.Resize(Len);
			if (Len)
				Strm.Serialise(S.Str, Len + 1);
		}
		else
		{
			vint32 Len = S.Length();
			Strm << STRM_INDEX(Len);
			if (Len)
				Strm.Serialise(S.Str, Len + 1);
		}
		return Strm;
	}

	VStr EvalEscapeSequences() const;

	VStr RemoveColours() const;

	VStr ExtractFilePath() const;
	VStr ExtractFileName() const;
	VStr ExtractFileBase() const;
	VStr ExtractFileExtension() const;
	VStr StripExtension() const;
	VStr DefaultPath(const VStr& basepath) const;
	VStr DefaultExtension(const VStr& extension) const;
	VStr FixFileSlashes() const;

	static size_t Length(const char*);
	static size_t Utf8Length(const char*);
	static size_t ByteLengthForUtf8(const char*, size_t);
	static int GetChar(const char*&);
	static VStr FromChar(int);
	static int Cmp(const char*, const char*);
	static int NCmp(const char*, const char*, size_t);
	static int ICmp(const char*, const char*);
	static int NICmp(const char*, const char*, size_t);
	static void Cpy(char*, const char*);
	static void NCpy(char*, const char*, size_t);
	static char ToUpper(char);
	static char ToLower(char);
};
