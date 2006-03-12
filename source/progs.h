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

	friend class TProgs;

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
	TCRC		crc;

	void Load(const char*);
	void Unload(void);

	VStruct* FindStruct(VName InName, VClass* InClass);

	static int Exec(FFunction *func);
	static int Exec(FFunction *func, int parm1);
	static int Exec(FFunction *func, int parm1, int parm2);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12, int parm13);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12, int parm13, int parm14);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12, int parm13, int parm14,
		int parm15);
	static int Exec(FFunction *func, int parm1, int parm2, int parm3,
		int parm4, int parm5, int parm6, int parm7, int parm8, int parm9,
		int parm10, int parm11, int parm12, int parm13, int parm14,
		int parm15, int parm16);
	static int ExecuteFunction(FFunction *func);
	void DumpProfile();

	int GetStringOffs(const char *Str)
	{
		return Str - Strings;
	}
	char *StrAtOffs(int Offs)
	{
		return Strings + Offs;
	}

private:
	VProgsReader*	Reader;
	char*			Strings;
	FFunction**		Functions;
	int				NumFunctions;
	FFunction**		VTables;
	VStruct**		Structs;
	int				NumStructs;

	char* FuncName(int fnum);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void PR_Init();
void PR_OnAbort();
void PR_Traceback();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TProgs			svpr;

extern "C" { extern int	*pr_stackPtr; }

//**************************************************************************
//
//	Stack routines
//
//**************************************************************************

inline void PR_Push(int value)
{
	*(pr_stackPtr++) = value;
}

inline int PR_Pop(void)
{
	return *(--pr_stackPtr);
}

inline void PR_Pushf(float value)
{
	*((float*)pr_stackPtr++) = value;
}

inline float PR_Popf(void)
{
	return *((float*)--pr_stackPtr);
}

inline void PR_Pushv(const TVec &v)
{
	PR_Pushf(v.x);
	PR_Pushf(v.y);
	PR_Pushf(v.z);
}

inline TVec PR_Popv(void)
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

//**************************************************************************
//
//	$Log$
//	Revision 1.21  2006/03/12 20:06:02  dj_jl
//	States as objects, added state variable type.
//
//	Revision 1.20  2006/03/10 19:31:25  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.19  2006/02/28 18:04:36  dj_jl
//	Added script execution helpers.
//	
//	Revision 1.18  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.17  2006/02/25 17:09:35  dj_jl
//	Import all progs type info.
//	
//	Revision 1.16  2006/02/17 19:23:47  dj_jl
//	Removed support for progs global variables.
//	
//	Revision 1.15  2006/02/15 23:27:41  dj_jl
//	Added script ID class attribute.
//	
//	Revision 1.14  2005/12/07 22:53:26  dj_jl
//	Moved compiler generated data out of globals.
//	
//	Revision 1.13  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.12  2002/05/03 17:06:23  dj_jl
//	Mangling of string pointers.
//	
//	Revision 1.11  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.10  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.9  2002/01/11 08:07:18  dj_jl
//	Added names to progs
//	
//	Revision 1.8  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.7  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.6  2001/12/01 17:43:13  dj_jl
//	Renamed ClassBase to VObject
//	
//	Revision 1.5  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.4  2001/08/21 17:39:22  dj_jl
//	Real string pointers in progs
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
