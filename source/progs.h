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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

struct dprograms_t;
struct dfunction_t;
struct globaldef_t;
struct dclassinfo_t;

typedef void (*builtin_t)(void);

struct builtin_info_t
{
	char*		name;
    builtin_t	func;
};

class TProgs
{
 public:
	TCRC		crc;

	void Load(const char*);
	void Unload(void);

	int FuncNumForName(const char* name);
	int GlobalNumForName(const char* name);

	void SetGlobal(int num, int val)
	{
		Globals[num] = val;
	}
	void SetGlobal(const char *name, int value)
	{
		Globals[GlobalNumForName(name)] = value;
	}
	int GetGlobal(int num)
	{
		return Globals[num];
	}
	int GetGlobal(const char *name)
	{
		return Globals[GlobalNumForName(name)];
	}
	int *GlobalAddr(int num)
	{
		return &Globals[num];
	}
	int *GlobalAddr(const char *name)
	{
		return &Globals[GlobalNumForName(name)];
	}

	int Exec(int fnum);
	int Exec(int fnum, int parm1);
	int Exec(int fnum, int parm1, int parm2);
	int Exec(int fnum, int parm1, int parm2, int parm3);
	int Exec(int fnum, int parm1, int parm2, int parm3, int parm4);
	int Exec(int fnum, int parm1, int parm2, int parm3, int parm4,
						int parm5);
	int Exec(int fnum, int parm1, int parm2, int parm3, int parm4,
						int parm5, int parm6);
	int Exec(int fnum, int parm1, int parm2, int parm3, int parm4,
						int parm5, int parm6, int parm7);
	int Exec(int fnum, int parm1, int parm2, int parm3, int parm4,
						int parm5, int parm6, int parm7, int parm8);
	int Exec(const char *name)
	{
		return Exec(FuncNumForName(name));
	}
	int Exec(const char *name, int parm1)
	{
		return Exec(FuncNumForName(name), parm1);
	}
	int Exec(const char *name, int parm1, int parm2)
	{
		return Exec(FuncNumForName(name), parm1, parm2);
	}
	int Exec(const char *name, int parm1, int parm2, int parm3)
	{
		return Exec(FuncNumForName(name), parm1, parm2, parm3);
	}
	int Exec(const char *name, int parm1, int parm2, int parm3, int parm4)
	{
		return Exec(FuncNumForName(name), parm1, parm2, parm3, parm4);
	}
	int Exec(const char *name, int parm1, int parm2, int parm3, int parm4,
								int parm5)
	{
		return Exec(FuncNumForName(name), parm1, parm2, parm3, parm4, parm5);
	}
	int Exec(const char *name, int parm1, int parm2, int parm3, int parm4,
								int parm5, int parm6)
	{
		return Exec(FuncNumForName(name),
			parm1, parm2, parm3, parm4, parm5, parm6);
	}
	int Exec(const char *name, int parm1, int parm2, int parm3, int parm4,
								int parm5, int parm6, int parm7)
	{
		return Exec(FuncNumForName(name),
			parm1, parm2, parm3, parm4, parm5, parm6, parm7);
	}
	int Exec(const char *name, int parm1, int parm2, int parm3, int parm4,
								int parm5, int parm6, int parm7, int parm8)
	{
		return Exec(FuncNumForName(name),
			parm1, parm2, parm3, parm4, parm5, parm6, parm7, parm8);
	}
	void DumpProfile(void);

	int GetClassID(const char *);
	VObject *Spawn(int, int);
	VObject *Spawn(const char *name, int tag)
	{
		return Spawn(GetClassID(name), tag);
	}
	void Destroy(VObject *);
	bool CanCast(VObject *, int);
	bool CanCast(int, int);

 private:
	dprograms_t	*Progs;
	char		*Strings;
	int			*Statements;
	int			*Globals;
	dfunction_t	*Functions;
	globaldef_t	*Globaldefs;
	builtin_t	*Builtins;
	dclassinfo_t	*ClassInfo;
	dword		*Profile1;
	dword		*Profile2;

	int CheckFuncNumForName(const char* name);
	int CheckGlobalNumForName(const char* name);
	char* FuncName(int fnum);
	int ExecuteFunction(int fnum);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void PR_Init(void);
void PR_OnAbort(void);
void PR_Traceback(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TProgs			svpr;

//**************************************************************************
//
//	$Log$
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
