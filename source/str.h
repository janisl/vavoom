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
	{
	}
	VStr(const char* InStr)
	: Str(NULL)
	{
		if (*InStr)
		{
			Resize(strlen(InStr));
			strcpy(Str, InStr);
		}
	}
	VStr(const VStr& InStr)
	: Str(NULL)
	{
		if (InStr.Str)
		{
			Resize(InStr.Length());
			strcpy(Str, InStr.Str);
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
			strcpy(Str, "true");
		}
		else
		{
			Resize(5);
			strcpy(Str, "false");
		}
	}
	explicit VStr(int InInt)
	: Str(NULL)
	{
		char Buf[64];

		sprintf(Buf, "%d", InInt);
		Resize(strlen(Buf));
		strcpy(Str, Buf);
	}
	explicit VStr(unsigned InInt)
	: Str(NULL)
	{
		char Buf[64];

		sprintf(Buf, "%u", InInt);
		Resize(strlen(Buf));
		strcpy(Str, Buf);
	}
	explicit VStr(float InFloat)
	: Str(NULL)
	{
		char Buf[64];

		sprintf(Buf, "%f", InFloat);
		Resize(strlen(Buf));
		strcpy(Str, Buf);
	}
	explicit VStr(const VName& InName)
	: Str(NULL)
	{
		Resize(strlen(*InName));
		strcpy(Str, *InName);
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
	//	Return C string.
	const char* operator*() const
	{
		return Str ? Str : "";
	}
	//	Check if string is empty.
	operator bool() const
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
		Resize(strlen(InStr));
		if (*InStr)
			strcpy(Str, InStr);
		return *this;
	}
	VStr& operator=(const VStr& InStr)
	{
		Resize(InStr.Length());
		if (InStr.Str)
			strcpy(Str, InStr.Str);
		return *this;
	}

	//	Concatenation operators.
	VStr& operator+=(const char* InStr)
	{
		if (*InStr)
		{
			int l = Length();
			Resize(l + strlen(InStr));
			strcpy(Str + l, InStr);
		}
		return *this;
	}
	VStr& operator+=(const VStr& InStr)
	{
		if (InStr.Length())
		{
			int l = Length();
			Resize(l + InStr.Length());
			strcpy(Str + l, *InStr);
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
		return !strcmp(*S1, S2);
	}
	friend bool operator==(const VStr& S1, const VStr& S2)
	{
		return !strcmp(*S1, *S2);
	}
	friend bool operator!=(const VStr& S1, const char* S2)
	{
		return !!strcmp(*S1, S2);
	}
	friend bool operator!=(const VStr& S1, const VStr& S2)
	{
		return !!strcmp(*S1, *S2);
	}

	//	Comparison functions.
	int Cmp(const char* S2) const
	{
		return strcmp(**this, S2);
	}
	int Cmp(const VStr& S2) const
	{
		return strcmp(**this, *S2);
	}
	int ICmp(const char* S2) const
	{
		return stricmp(**this, S2);
	}
	int ICmp(const VStr& S2) const
	{
		return stricmp(**this, *S2);
	}

	//	Serialisation operator.
	friend VStream& operator<<(VStream& Strm, VStr& S)
	{
		if (Strm.IsLoading())
		{
			vint32 Len;
			Strm << STRM_INDEX(Len);
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

	VStr ExtractFilePath() const;
	VStr ExtractFileName() const;
	VStr ExtractFileBase() const;
	VStr ExtractFileExtension() const;
	VStr StripExtension() const;
	VStr DefaultPath(const VStr& basepath) const;
	VStr DefaultExtension(const VStr& extension) const;
};
