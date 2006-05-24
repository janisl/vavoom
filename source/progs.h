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

class VProgsReader;
struct dprograms_t;

typedef void (*builtin_t)();

struct builtin_info_t
{
	char*		name;
    builtin_t	func;
	VClass		*OuterClass;
};

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

class TProgs
{
public:
	void Load(const char*);
	void Unload();

	static VStruct* FindStruct(VName InName, VClass* InClass);

	static int Exec(VMethod *func);
	static int Exec(VMethod *func, int parm1);
	static int Exec(VMethod *func, int parm1, int parm2);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12, int parm13);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12, int parm13, int parm14);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12, int parm13, int parm14,
		int parm15);
	static int Exec(VMethod *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12, int parm13, int parm14,
		int parm15, int parm16);
	static int ExecuteFunction(VMethod *func);
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

extern "C" { extern int	*pr_stackPtr; }

extern VMessage*		pr_msg;

//**************************************************************************
//
//	Stack routines
//
//**************************************************************************

inline void PR_Push(int value)
{
	*(pr_stackPtr++) = value;
}

inline int PR_Pop()
{
	return *(--pr_stackPtr);
}

inline void PR_Pushf(float value)
{
	*((float*)pr_stackPtr++) = value;
}

inline float PR_Popf()
{
	return *((float*)--pr_stackPtr);
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
	*((VName*)pr_stackPtr++) = value;
}

inline VName PR_PopName()
{
	return *((VName*)--pr_stackPtr);
}

const char* PF_FormatString();
void PR_MSG_Select(int);
