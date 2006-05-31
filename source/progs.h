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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	MSG_SV_DATAGRAM,
	MSG_SV_RELIABLE,
	MSG_SV_SIGNON,
	MSG_SV_CLIENT,
	MSG_CL_MESSAGE
};

class VProgsReader;
struct dprograms_t;

typedef void (*builtin_t)();

class FBuiltinInfo
{
	const char		*Name;
	VClass			*OuterClass;
    builtin_t		Func;
	FBuiltinInfo	*Next;

	static FBuiltinInfo *Builtins;

	friend class VMethod;

public:
	FBuiltinInfo(const char *InName, VClass *InClass, builtin_t InFunc)
		: Name(InName), OuterClass(InClass), Func(InFunc)
	{
		Next = Builtins;
		Builtins = this;
	}
};

union VStack
{
	vint32	i;
	float	f;
	void*	p;
};

class TProgs
{
public:
	void Load(const char*);
	void Unload();

	static VStruct* FindStruct(VName InName, VClass* InClass);

	static VStack ExecuteFunction(VMethod *func);
	static void DumpProfile();

	int GetStringOffs(const char *Str);
	char *StrAtOffs(int Offs);

private:
	VPackage*		Pkg;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void PR_Init();
void PR_OnAbort();
void PR_Traceback();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TProgs			svpr;

extern VStack*			pr_stackPtr;

extern VMessage*		pr_msg;

//**************************************************************************
//
//	Stack routines
//
//**************************************************************************

inline void PR_Push(int value)
{
	pr_stackPtr->i = value;
	pr_stackPtr++;
}

inline int PR_Pop()
{
	--pr_stackPtr;
	return pr_stackPtr->i;
}

inline void PR_Pushf(float value)
{
	pr_stackPtr->f = value;
	pr_stackPtr++;
}

inline float PR_Popf()
{
	--pr_stackPtr;
	return pr_stackPtr->f;
}

inline void PR_Pushv(const TVec &v)
{
	PR_Pushf(v.x);
	PR_Pushf(v.y);
	PR_Pushf(v.z);
}

inline TVec PR_Popv()
{
	TVec v;
	v.z = PR_Popf();
	v.y = PR_Popf();
	v.x = PR_Popf();
	return v;
}

inline void PR_PushName(VName value)
{
	pr_stackPtr->i = value.GetIndex();
	pr_stackPtr++;
}

inline VName PR_PopName()
{
	--pr_stackPtr;
	return *(VName*)&pr_stackPtr->i;
}

inline void PR_PushPtr(void* value)
{
	pr_stackPtr->p = value;
	pr_stackPtr++;
}

inline void* PR_PopPtr()
{
	--pr_stackPtr;
	return pr_stackPtr->p;
}

const char* PF_FormatString();
void PR_MSG_Select(int);
