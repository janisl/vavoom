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
//**    Execution of PROGS.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "progdefs.h"

// MACROS ------------------------------------------------------------------

#define MAX_PROG_STACK	10000
#define STACK_ID		0x45f6cd4b

# define CHECK_STACK_OVERFLOW
//# define CHECK_PARM_COUNT
//	Not needed, if PROGS are correct
# define CHECK_STACK_UNDERFLOW
//# define CHECK_FUNC_NUM
//# define CHECK_VALID_OPCODE
# define CHECK_FOR_EMPTY_STACK
//# define CHECK_VALID_VAR_NUM

// TYPES -------------------------------------------------------------------

struct FGlobalDef
{
	FName	Name;
	int		Ofs;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern builtin_info_t	BuiltinInfo[];

extern "C" void TestCaller(void);

// PUBLIC DATA DEFINITIONS -------------------------------------------------

FBuiltinInfo *FBuiltinInfo::Builtins;

extern "C" { 
int				*pr_stackPtr; 
FFunction		*current_func = NULL;
}

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int				pr_stack[MAX_PROG_STACK];

#define DECLARE_OPC(name, argcount)		argcount
#define OPCODE_INFO
static int OpcodeArgCount[NUM_OPCODES] =
{
#include "progdefs.h"
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	PR_Init
//
//==========================================================================

void PR_Init(void)
{
	//	Set stack ID for overflow / underflow checks
	pr_stack[0] = STACK_ID;
	pr_stack[MAX_PROG_STACK - 1] = STACK_ID;
	pr_stackPtr = pr_stack + 1;
	for (int i = 0; BuiltinInfo[i].name; i++)
	{
		new FBuiltinInfo(BuiltinInfo[i].name, BuiltinInfo[i].OuterClass,
				BuiltinInfo[i].func);
	}
}

//==========================================================================
//
//	PR_OnAbort
//
//==========================================================================

void PR_OnAbort(void)
{
	current_func = NULL;
	pr_stackPtr = pr_stack + 1;
}

//==========================================================================
//
//	PR_Profile1
//
//==========================================================================

extern "C" void PR_Profile1(void)
{
	current_func->Profile1++;
}

//==========================================================================
//
//	PR_Profile2
//
//==========================================================================

extern "C" void PR_Profile2(void)
{
	if (current_func && (!(current_func->Flags & FUNC_Native)))
	{
		current_func->Profile2++;
	}
}
extern "C" void PR_Profile2_end(void){}

//==========================================================================
//
//	PR_Traceback
//
//==========================================================================

void PR_Traceback(void)
{
	if (current_func)
	{
		Host_CoreDump("(%s.%s)", current_func->OuterClass ? 
			current_func->OuterClass->GetName() : "", *current_func->Name);
	}
}

//==========================================================================
//
//  PF_Fixme
//
//==========================================================================

static void PF_Fixme(void)
{
	Sys_Error("unimplemented bulitin");
}

//==========================================================================
//
//	TProgs::Load
//
//==========================================================================

void TProgs::Load(const char *AName)
{
	guard(TProgs::Load);
	int		i;
	int		len;
	char	progfilename[256];
	int		*Statements;
	dfunction_t		*DFunctions;
	dglobaldef_t	*DGlobalDefs;
	dclassinfo_t	*ClassInfo;
	VClass		**ClassList;
	FName	*NameRemap;
	char	*pName;

	i = M_CheckParm("-progs");
    if (i && i < myargc - 1)
    {
    	//	Load PROGS from a specified file
    	len = M_ReadFile(va("%s%s.dat", myargv[i + 1], AName), (byte**)&Progs);
    }
    else if (fl_devmode && FL_FindFile(va("progs/%s.dat", AName), progfilename))
	{
    	//	Load PROGS from a specified file
    	len = M_ReadFile(progfilename, (byte**)&Progs);
	}
    else
    {
    	//	Load PROGS from wad file
		i =	W_GetNumForName(AName);
		Progs = (dprograms_t*)W_CacheLumpNum(i, PU_STATIC);
		len = W_LumpLength(i);
	}

	//	Calcutate CRC
	crc.Init();
	for (i = 0; i < len; i++)
    {
    	crc + ((byte*)Progs)[i];
    }

	// byte swap the header
	for (i = 0; i < (int)sizeof(*Progs) / 4; i++)
    {
		((int *)Progs)[i] = LittleLong(((int *)Progs)[i]);
	}

	if (strncmp(Progs->magic, PROG_MAGIC, 4))
		Sys_Error("Progs has wrong file ID, possibly older version");
	if (Progs->version != PROG_VERSION)
		Sys_Error("Progs has wrong version number (%i should be %i)",
			Progs->version, PROG_VERSION);

	Strings = (char*)Progs + Progs->ofs_strings;
	Statements = (int*)((byte*)Progs + Progs->ofs_statements);
	Globals = (int*)((byte*)Progs + Progs->ofs_globals);
	DFunctions = (dfunction_t *)((byte *)Progs + Progs->ofs_functions);
	DGlobalDefs = (dglobaldef_t *)((byte *)Progs + Progs->ofs_globaldefs);
	ClassInfo = (dclassinfo_t *)((byte *)Progs + Progs->ofs_classinfo);

	Functions = Z_CNew<FFunction>(Progs->num_functions);
	Globaldefs = Z_CNew<FGlobalDef>(Progs->num_globaldefs);

	// Read names
	NameRemap = Z_New<FName>(Progs->num_names);
	pName = (char *)Progs + Progs->ofs_names;
	for (i = 0; i < Progs->num_names; i++)
	{
		NameRemap[i] = pName;
		pName += (strlen(pName) + 4) & ~3;
	}

	// byte swap the lumps
	for (i = 0; i < Progs->num_statements; i++)
	{
		Statements[i] = LittleLong(Statements[i]);
	}
	for (i = 0; i < Progs->num_globals; i++)
	{
		Globals[i] = LittleLong(Globals[i]);
	}
	for (i = 0; i < Progs->num_functions; i++)
	{
		DFunctions[i].name = LittleShort(DFunctions[i].name);
		DFunctions[i].outer_class = LittleShort(DFunctions[i].outer_class);
		DFunctions[i].first_statement = LittleLong(DFunctions[i].first_statement);
		DFunctions[i].num_parms = LittleShort(DFunctions[i].num_parms);
		DFunctions[i].num_locals = LittleShort(DFunctions[i].num_locals);
		DFunctions[i].type = LittleShort(DFunctions[i].type);
		DFunctions[i].flags = LittleShort(DFunctions[i].flags);
	}
	for (i = 0; i < Progs->num_globaldefs; i++)
	{
		DGlobalDefs[i].name = LittleShort(DGlobalDefs[i].name);
		DGlobalDefs[i].ofs = LittleShort(DGlobalDefs[i].ofs);
	}
	for (i = 0; i < Progs->num_classinfo; i++)
	{
		ClassInfo[i].name = LittleLong(ClassInfo[i].name);
		ClassInfo[i].vtable = LittleLong(ClassInfo[i].vtable);
		ClassInfo[i].size = LittleShort(ClassInfo[i].size);
		ClassInfo[i].num_methods = LittleShort(ClassInfo[i].num_methods);
		ClassInfo[i].parent = LittleLong(ClassInfo[i].parent);
		ClassInfo[i].num_properties = LittleLong(ClassInfo[i].num_properties);
		ClassInfo[i].ofs_properties = LittleLong(ClassInfo[i].ofs_properties);
	}

	//	Setup classes
	ClassList = Z_CNew<VClass *>(Progs->num_classinfo);
	for (i = 0; i < Progs->num_classinfo; i++)
	{
		ClassList[i] = VClass::FindClass(*NameRemap[ClassInfo[i].name]);
		if (!ClassList[i])
		{
			ClassList[i] = new(PU_STRING) VClass(NameRemap[ClassInfo[i].name],
				ClassInfo[i].size);
		}
		else if (ClassList[i]->ClassSize != ClassInfo[i].size)
		{
			Sys_Error("Bad class size, class %s, C++ %d, VavoomC %d)",
				ClassList[i]->GetName(), ClassList[i]->ClassSize,
				ClassInfo[i].size);
		}
		if (!ClassList[i]->ClassVTable)
		{
			ClassList[i]->ClassNumMethods = ClassInfo[i].num_methods;
			ClassList[i]->ClassVTable = (FFunction **)(Globals +
				ClassInfo[i].vtable);
		}
		if (!ClassList[i]->PropertyInfo)
		{
			ClassList[i]->NumPropertyInfo = ClassInfo[i].num_properties;
			ClassList[i]->PropertyInfo = (FPropertyInfo *)(Globals +
				ClassInfo[i].ofs_properties);
		}
	}
	for (i = 0; i < Progs->num_classinfo; i++)
	{
		if (!ClassList[i]->ParentClass && !(ClassList[i]->GetFlags() & OF_Native))
		{
			ClassList[i]->ParentClass = ClassList[ClassInfo[i].parent];
		}
	}

	//	Setup functions
	for (i = 0; i < Progs->num_functions; i++)
	{
		Functions[i].Name = NameRemap[DFunctions[i].name];
		Functions[i].OuterClass = DFunctions[i].outer_class != -1 ?
			ClassList[DFunctions[i].outer_class] : NULL;
		Functions[i].FirstStatement = DFunctions[i].first_statement;
		Functions[i].NumParms = DFunctions[i].num_parms;
		Functions[i].NumLocals = DFunctions[i].num_locals;
		Functions[i].Type = DFunctions[i].type;
		Functions[i].Flags = DFunctions[i].flags;
	}

	//	Setup globaldefs
	for (i = 0; i < Progs->num_globaldefs; i++)
	{
		Globaldefs[i].Ofs = DGlobalDefs[i].ofs;
		Globaldefs[i].Name = NameRemap[DGlobalDefs[i].name];
	}

	//	Setup string pointers in globals
	byte *globalinfo = (byte*)Progs + Progs->ofs_globalinfo;
	for (i = 0; i < Progs->num_globals; i++)
	{
		if (globalinfo[i] == 1)
		{
			Globals[i] += int(Strings);
		}
		if (globalinfo[i] == 2 && Globals[i])
		{
#ifdef CHECK_FUNC_NUM
			if (Globals[i] < 0 || Globals[i] >= Progs->num_functions)
			{
				Sys_Error("RunFunction: NULL function");
			}
#endif
			Globals[i] = int(Functions + Globals[i]);
		}
		if (globalinfo[i] == 3)
		{
			Globals[i] = (int)ClassList[Globals[i]];
		}
		if (globalinfo[i] == 4)
		{
			Globals[i] = NameRemap[Globals[i]].GetIndex();
		}
	}

	//	Set up builtins
	for (i = 1; i < Progs->num_functions; i++)
	{
		FBuiltinInfo *B;

		if (Functions[i].NumParms > 16)
			Sys_Error("Function has more than 16 params");
		for (B = FBuiltinInfo::Builtins; B; B = B->Next)
		{
			if (Functions[i].OuterClass == B->OuterClass &&
				!strcmp(*Functions[i].Name, B->Name))
			{
				if (Functions[i].Flags & FUNC_Native)
				{
					Functions[i].FirstStatement = (int)B->Func;
					break;
				}
				else
				{
					Sys_Error("PR_LoadProgs: Builtin %s redefined", B->Name);
				}
			}
		}
		if (!B && Functions[i].Flags & FUNC_Native)
		{
			//	Default builtin
			Functions[i].FirstStatement = (int)PF_Fixme;
#if defined CLIENT && defined SERVER
			//	Don't abort with error, because it will be done, when this
			// function will be called (if it will be called).
			GCon->Logf(NAME_Dev, "WARNING: Builtin %s.%s not found!",
				Functions[i].OuterClass ? Functions[i].OuterClass->GetName() : "",
				*Functions[i].Name);
#endif
		}
		if (!(Functions[i].Flags & FUNC_Native))
		{
			Functions[i].FirstStatement =
				(int)(Statements + Functions[i].FirstStatement);
		}
	}

	//
	//	Patch code
	//
	for (i = 0; i < Progs->num_statements; i++)
	{
		switch (Statements[i])
		{
		case OPC_PUSHSTRING:
			Statements[i + 1] += (int)Strings;
			break;
		case OPC_PUSHFUNCTION:
			Statements[i + 1] = (int)(Functions + Statements[i + 1]);
			break;
		case OPC_PUSHCLASSID:
		case OPC_DYNAMIC_CAST:
			Statements[i + 1] = (int)ClassList[Statements[i + 1]];
			break;
		case OPC_PUSHNAME:
			Statements[i + 1] = NameRemap[Statements[i + 1]].GetIndex();
			break;
		case OPC_GOTO:
		case OPC_IFGOTO:
		case OPC_IFNOTGOTO:
		case OPC_IFTOPGOTO:
		case OPC_IFNOTTOPGOTO:
			Statements[i + 1] = (int)(Statements + Statements[i + 1]);
			break;
		case OPC_CASEGOTO:
			Statements[i + 2] = (int)(Statements + Statements[i + 2]);
			break;
		case OPC_CASE_GOTO_CLASSID:
			Statements[i + 1] = (int)ClassList[Statements[i + 1]];
			Statements[i + 2] = (int)(Statements + Statements[i + 2]);
			break;
		case OPC_CASE_GOTO_NAME:
			Statements[i + 1] = NameRemap[Statements[i + 1]].GetIndex();
			Statements[i + 2] = (int)(Statements + Statements[i + 2]);
			break;
		case OPC_GLOBALADDRESS:
#ifdef CHECK_VALID_VAR_NUM
			if (Statements[i + 1] < 0 || Statements[i + 1] >= Progs->num_globaldefs)
			{
		    	Sys_Error("Bad global num %d", Statements[i + 1]);
	    	}
#endif
			Statements[i + 1] = (int)(Globals + Globaldefs[Statements[i + 1]].Ofs);
			break;
		case OPC_CALL:
		    Statements[i + 1] = (int)(Functions + Statements[i + 1]);
			break;
		}
		i += OpcodeArgCount[Statements[i]];
	}

	//	Execute initialization function
	Exec("main");
	Z_Free(ClassList);
	Z_Free(NameRemap);
	unguard;
}

//==========================================================================
//
//	TProgs::Unload
//
//==========================================================================

void TProgs::Unload(void)
{
	Z_Free(Progs);
	Z_Free(Functions);
	Z_Free(Globaldefs);
}

//==========================================================================
//
//	TProgs::CheckFuncForName
//
//==========================================================================

FFunction *TProgs::CheckFuncForName(const char* name)
{
	int		i;

	for (i = 1; i < Progs->num_functions; i++)
    {
    	if (!Functions[i].OuterClass && !strcmp(*Functions[i].Name, name))
		{
			return &Functions[i];
		}
    }
	return NULL;
}

//==========================================================================
//
//	TProgs::FuncForName
//
//==========================================================================

FFunction *TProgs::FuncForName(const char* name)
{
	FFunction *func;

	func = CheckFuncForName(name);
    if (!func)
    {
    	Sys_Error("FuncNumForName: function %s not found", name);
    }
    return func;
}

//==========================================================================
//
//	TProgs::FindFunctionChecked
//
//==========================================================================

FFunction *TProgs::FindFunctionChecked(FName InName)
{
	for (int i = 1; i < Progs->num_functions; i++)
    {
    	if (!Functions[i].OuterClass && Functions[i].Name == InName)
		{
			return &Functions[i];
		}
    }
	Sys_Error("Function %s not found", *InName);
	return NULL;
}

//==========================================================================
//
//	TProgs::CheckGlobalNumForName
//
//==========================================================================

int TProgs::CheckGlobalNumForName(const char* name)
{
	int		i;

	for (i=1; i<Progs->num_globaldefs; i++)
    {
    	if (!strcmp(*Globaldefs[i].Name, name))
		{
        	return Globaldefs[i].Ofs;
		}
    }
	return -1;
}

//==========================================================================
//
//	TProgs::GlobalNumForName
//
//==========================================================================

int TProgs::GlobalNumForName(const char* name)
{
	int		i;

	i = CheckGlobalNumForName(name);
    if (i == -1)
    {
    	Sys_Error("PR_GlobalNumForName: global %s not found", name);
    }
    return i;
}

//==========================================================================
//
//  TProgs::FuncName
//
//==========================================================================

char* TProgs::FuncName(int fnum)
{
	return const_cast<char *>(*((FFunction *)fnum)->Name);
}

//==========================================================================
//
//  RunFunction
//
//==========================================================================

//	Split out for asm
extern "C" void PR_RFInvalidOpcode(void)
{
	Sys_Error("Empty function or invalid opcode");
}

extern "C" VObject *PR_DynamicCast(VObject *object, VClass *SomeClass)
{
	if (!object || !object->IsA(SomeClass))
	{
		return NULL;
	}
	return object;
}

#ifdef USEASM

//	Use asm version
extern "C" void RunFunction(FFunction *func);

#else

static void RunFunction(FFunction *func)
{
	int			*current_statement;
	int			*sp;
	int			*local_vars;

	guardSlow(RunFunction);
    current_func = func;

	if (func->Flags & FUNC_Native)
	{
		//	Negative first statements are built in functions. Don't need to
		// check builtin number (already done when setup builtins).
		((void(*)(void))func->FirstStatement)();
		return;
	}

	//	Keep stack pointer in register
	sp = pr_stackPtr;

	//	Setup local vars
    local_vars = sp - func->NumParms;
	sp += func->NumLocals - func->NumParms;

	current_statement = (int *)func->FirstStatement;

	//
    //	The main function loop
    //
    //	I realy hate using goto in a C program, but this is the only way
    // how to force gcc to create a jump directly here. while(1) would be
    // better, but then gcc, even with optimizations, creates an dummy check,
    // which only takes time.
    //
 func_loop:

	switch (*current_statement++)
	{
     case OPC_DONE:
		PR_RFInvalidOpcode();
		break;

	 case OPC_RETURN:
		pr_stackPtr = local_vars;
        return;

	 case OPC_RETURNL:
		local_vars[0] = sp[-1];
		pr_stackPtr = local_vars + 1;
        return;

	 case OPC_RETURNV:
		local_vars[0] = sp[-3];
		local_vars[1] = sp[-2];
		local_vars[2] = sp[-1];
		pr_stackPtr = local_vars + 3;
        return;

	 case OPC_PUSHNUMBER:
	 case OPC_GLOBALADDRESS:
	 case OPC_PUSHSTRING:
	 case OPC_PUSHFUNCTION:
	 case OPC_PUSHCLASSID:
	 case OPC_PUSHNAME:
		*sp++ = *current_statement++;
		break;

	 case OPC_PUSHPOINTED:
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_LOCALADDRESS:
#ifdef CHECK_VALID_VAR_NUM
		if (*current_statement < 0 || *current_statement >= MAX_LOCALS)
	    {
	    	Sys_Error("Bad local num %d", *current_statement);
		}
#endif
		*sp++ = (int)&local_vars[*current_statement++];
		break;

	 case OPC_ADD:
        sp--;
		sp[-1] += *sp;
		break;

	 case OPC_SUBTRACT:
        sp--;
		sp[-1] -= *sp;
		break;

	 case OPC_MULTIPLY:
        sp--;
		sp[-1] *= *sp;
		break;

	 case OPC_DIVIDE:
        sp--;
		sp[-1] /= *sp;
		break;

	 case OPC_MODULUS:
        sp--;
		sp[-1] %= *sp;
		break;

	 case OPC_EQ:
        sp--;
		sp[-1] = sp[-1] == *sp;
		break;

	 case OPC_NE:
        sp--;
		sp[-1] = sp[-1] != *sp;
		break;

	 case OPC_LT:
        sp--;
		sp[-1] = sp[-1] < *sp;
		break;

	 case OPC_GT:
        sp--;
		sp[-1] = sp[-1] > *sp;
		break;

	 case OPC_LE:
        sp--;
		sp[-1] = sp[-1] <= *sp;
		break;

	 case OPC_GE:
        sp--;
		sp[-1] = sp[-1] >= *sp;
		break;

	 case OPC_ANDLOGICAL:
        sp--;
		sp[-1] = sp[-1] && *sp;
		break;

	 case OPC_ORLOGICAL:
        sp--;
		sp[-1] = sp[-1] || *sp;
		break;

	 case OPC_NEGATELOGICAL:
		sp[-1] = !sp[-1];
		break;

	 case OPC_ANDBITWISE:
        sp--;
		sp[-1] &= *sp;
		break;

	 case OPC_ORBITWISE:
        sp--;
		sp[-1] |= *sp;
		break;

	 case OPC_XORBITWISE:
        sp--;
		sp[-1] ^= *sp;
		break;

	 case OPC_LSHIFT:
        sp--;
		sp[-1] <<= *sp;
		break;

	 case OPC_RSHIFT:
        sp--;
		sp[-1] >>= *sp;
		break;

	 case OPC_UNARYMINUS:
		sp[-1] = -sp[-1];
		break;

	 case OPC_BITINVERSE:
		sp[-1] = ~sp[-1];
		break;

	 case OPC_CALL:
		pr_stackPtr = sp;
	    RunFunction((FFunction *)*current_statement++);
		current_func = func;
		sp = pr_stackPtr;
		break;

	 case OPC_GOTO:
		current_statement = (int *)*current_statement;
		break;

	 case OPC_IFGOTO:
        sp--;
		if (*sp)
		{
			current_statement = (int *)*current_statement;
		}
		else
	    {
	        current_statement++;
	    }
		break;

	 case OPC_IFNOTGOTO:
        sp--;
	    if (!*sp)
	    {
	    	current_statement = (int *)*current_statement;
		}
		else
	    {
	        current_statement++;
	    }
		break;

	 case OPC_CASEGOTO:
	 case OPC_CASE_GOTO_CLASSID:
	 case OPC_CASE_GOTO_NAME:
		if (*current_statement++ == sp[-1])
	    {
	    	current_statement = (int *)*current_statement;
			sp--;
		}
		else
	    {
	        current_statement++;
	    }
		break;

	 case OPC_DROP:
		sp--;
		break;

	 case OPC_ASSIGN:
        sp--;
		*(int*)sp[-1] = *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_ADDVAR:
        sp--;
		*(int*)sp[-1] += *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_SUBVAR:
        sp--;
		*(int*)sp[-1] -= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_MULVAR:
        sp--;
		*(int*)sp[-1] *= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_DIVVAR:
        sp--;
		*(int*)sp[-1] /= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_MODVAR:
        sp--;
		*(int*)sp[-1] %= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_ANDVAR:
        sp--;
		*(int*)sp[-1] &= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_ORVAR:
        sp--;
		*(int*)sp[-1] |= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_XORVAR:
        sp--;
		*(int*)sp[-1] ^= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_LSHIFTVAR:
        sp--;
		*(int*)sp[-1] <<= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_RSHIFTVAR:
        sp--;
		*(int*)sp[-1] >>= *sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_PREINC:
#ifdef __GNUC__
		sp[-1] = ++(*(int*)sp[-1]);
#else
		{
			int		*ptr;

			ptr = (int*)sp[-1];
			++(*ptr);
			sp[-1] = *ptr;
		}
#endif
		break;

	 case OPC_PREDEC:
#ifdef __GNUC__
		sp[-1] = --(*(int*)sp[-1]);
#else
		{
			int		*ptr;

			ptr = (int*)sp[-1];
			--(*ptr);
			sp[-1] = *ptr;
		}
#endif
		break;

	 case OPC_POSTINC:
#ifdef __GNUC__
		sp[-1] = (*(int*)sp[-1])++;
#else
		{
			int		*ptr;

			ptr = (int*)sp[-1];
			sp[-1] = *ptr;
			(*ptr)++;
		}
#endif
		break;

	 case OPC_POSTDEC:
#ifdef __GNUC__
		sp[-1] = (*(int*)sp[-1])--;
#else
		{
			int		*ptr;

			ptr = (int*)sp[-1];
			sp[-1] = *ptr;
			(*ptr)--;
		}
#endif
		break;

	 case OPC_IFTOPGOTO:
	    if (sp[-1])
	    {
	    	current_statement = (int *)*current_statement;
	    }
		else
	    {
	        current_statement++;
	    }
		break;

	 case OPC_IFNOTTOPGOTO:
	    if (!sp[-1])
	    {
	    	current_statement = (int *)*current_statement;
		}
	    else
	    {
	        current_statement++;
	    }
		break;

	 case OPC_ASSIGN_DROP:
        sp--;
		*(int*)sp[-1] = *sp;
		sp--;
		break;

	 case OPC_ADDVAR_DROP:
        sp--;
		*(int*)sp[-1] += *sp;
		sp--;
		break;

	 case OPC_SUBVAR_DROP:
        sp--;
		*(int*)sp[-1] -= *sp;
		sp--;
		break;

	 case OPC_MULVAR_DROP:
        sp--;
		*(int*)sp[-1] *= *sp;
		sp--;
		break;

	 case OPC_DIVVAR_DROP:
        sp--;
		*(int*)sp[-1] /= *sp;
		sp--;
		break;

	 case OPC_MODVAR_DROP:
        sp--;
		*(int*)sp[-1] %= *sp;
		sp--;
		break;

	 case OPC_ANDVAR_DROP:
        sp--;
		*(int*)sp[-1] &= *sp;
		sp--;
		break;

	 case OPC_ORVAR_DROP:
        sp--;
		*(int*)sp[-1] |= *sp;
		sp--;
		break;

	 case OPC_XORVAR_DROP:
        sp--;
		*(int*)sp[-1] ^= *sp;
		sp--;
		break;

	 case OPC_LSHIFTVAR_DROP:
        sp--;
		*(int*)sp[-1] <<= *sp;
		sp--;
		break;

	 case OPC_RSHIFTVAR_DROP:
        sp--;
		*(int*)sp[-1] >>= *sp;
		sp--;
		break;

	 case OPC_INC_DROP:
		(*(int*)sp[-1])++;
		sp--;
		break;

	 case OPC_DEC_DROP:
		(*(int*)sp[-1])--;
		sp--;
		break;

#define spf	((float*)sp)

//=====================================
	 case OPC_FADD:
        sp--;
		spf[-1] += *spf;
		break;

	 case OPC_FSUBTRACT:
        sp--;
		spf[-1] -= *spf;
		break;

	 case OPC_FMULTIPLY:
        sp--;
		spf[-1] *= *spf;
		break;

	 case OPC_FDIVIDE:
        sp--;
		spf[-1] /= *spf;
		break;

	 case OPC_FEQ:
        sp--;
		sp[-1] = spf[-1] == *spf;
		break;

	 case OPC_FNE:
        sp--;
		sp[-1] = spf[-1] != *spf;
		break;

	 case OPC_FLT:
        sp--;
		sp[-1] = spf[-1] < *spf;
		break;

	 case OPC_FGT:
        sp--;
		sp[-1] = spf[-1] > *spf;
		break;

	 case OPC_FLE:
        sp--;
		sp[-1] = spf[-1] <= *spf;
		break;

	 case OPC_FGE:
        sp--;
		sp[-1] = spf[-1] >= *spf;
		break;

	 case OPC_FUNARYMINUS:
		spf[-1] = -spf[-1];
		break;

	 case OPC_FADDVAR:
        sp--;
		*(float*)sp[-1] += *spf;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_FSUBVAR:
        sp--;
		*(float*)sp[-1] -= *spf;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_FMULVAR:
        sp--;
		*(float*)sp[-1] *= *spf;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_FDIVVAR:
        sp--;
		*(float*)sp[-1] /= *spf;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_FADDVAR_DROP:
        sp--;
		*(float*)sp[-1] += *spf;
		sp--;
		break;

	 case OPC_FSUBVAR_DROP:
        sp--;
		*(float*)sp[-1] -= *spf;
		sp--;
		break;

	 case OPC_FMULVAR_DROP:
        sp--;
		*(float*)sp[-1] *= *spf;
		sp--;
		break;

	 case OPC_FDIVVAR_DROP:
        sp--;
		*(float*)sp[-1] /= *spf;
		sp--;
		break;

//=====================================

	 case OPC_SWAP:
		{
			int tmp = sp[-2];
			sp[-2] = sp[-1];
			sp[-1] = tmp;
		}
		break;

	 case OPC_ICALL:
		sp--;
		pr_stackPtr = sp;
	    RunFunction((FFunction *)*sp);
		current_func = func;
		sp = pr_stackPtr;
		break;

//=====================================

#define top_vec1	(*(TVec*)(sp - 6))
#define top_vec2	(*(TVec*)(sp - 3))
#define dec_top_vec	(*(TVec*)(sp - 4))

	 case OPC_VPUSHPOINTED:
		sp += 2;
		top_vec2 = *(TVec*)sp[-3];
		break;

	 case OPC_VADD:
		top_vec1 = top_vec1 + top_vec2;
		sp -= 3;
		break;

	 case OPC_VSUBTRACT:
		top_vec1 = top_vec1 - top_vec2;
		sp -= 3;
		break;

	 case OPC_VPRESCALE:
		dec_top_vec = spf[-4] * top_vec2;
		sp--;
		break;

	 case OPC_VPOSTSCALE:
		dec_top_vec = dec_top_vec * spf[-1];
		sp--;
		break;

	 case OPC_VISCALE:
		dec_top_vec = dec_top_vec / spf[-1];
		sp--;
		break;

	 case OPC_VEQ:
		sp[-6] = top_vec1 == top_vec2;
		sp -= 5;
		break;

	 case OPC_VNE:
		sp[-6] = top_vec1 != top_vec2;
		sp -= 5;
		break;

	 case OPC_VUNARYMINUS:
		top_vec2 = -top_vec2;
		break;

	 case OPC_VDROP:
		sp -= 3;
		break;

	 case OPC_VASSIGN:
		*(TVec*)sp[-4] = top_vec2;
		dec_top_vec = *(TVec*)sp[-4];
        sp--;
		break;

	 case OPC_VADDVAR:
		*(TVec*)sp[-4] += top_vec2;
		dec_top_vec = *(TVec*)sp[-4];
        sp--;
		break;

	 case OPC_VSUBVAR:
		*(TVec*)sp[-4] -= top_vec2;
		dec_top_vec = *(TVec*)sp[-4];
        sp--;
		break;

	 case OPC_VSCALEVAR:
		sp++;
		*(TVec*)sp[-3] *= spf[-2];
		top_vec2 = *(TVec*)sp[-3];
		break;

	 case OPC_VISCALEVAR:
		sp++;
		*(TVec*)sp[-3] /= spf[-2];
		top_vec2 = *(TVec*)sp[-3];
		break;

	 case OPC_VASSIGN_DROP:
		*(TVec*)sp[-4] = top_vec2;
        sp -= 4;
		break;

	 case OPC_VADDVAR_DROP:
		*(TVec*)sp[-4] += top_vec2;
        sp -= 4;
		break;

	 case OPC_VSUBVAR_DROP:
		*(TVec*)sp[-4] -= top_vec2;
        sp -= 4;
		break;

	 case OPC_VSCALEVAR_DROP:
		*(TVec*)sp[-2] *= spf[-1];
		sp -= 2;
		break;

	 case OPC_VISCALEVAR_DROP:
		*(TVec*)sp[-2] /= spf[-1];
		sp -= 2;
		break;

//=====================================

	 case OPC_COPY:
		*sp = sp[-1];
		sp++;
		break;

	 case OPC_SWAP3:
		{
			int tmp = sp[-4];
			sp[-4] = sp[-3];
			sp[-3] = sp[-2];
			sp[-2] = sp[-1];
			sp[-1] = tmp;
		}
		break;

	 case OPC_DYNAMIC_CAST:
		sp[-1] = (int)PR_DynamicCast((VObject *)sp[-1], (VClass *)*current_statement++);
		break;

	 case OPC_PUSHBOOL:
		{
			int mask = *current_statement++;
			sp[-1] = !!(*(int*)sp[-1] & mask);
		}
		break;

	 case OPC_ASSIGNBOOL:
		{
			int mask = *current_statement++;
	        sp--;
			if (*sp)
				*(int*)sp[-1] |= mask;
			else
				*(int*)sp[-1] &= ~mask;
			sp--;
		}
		break;

	 case OPC_PUSH_VFUNC:
		*sp++ = (int)((VObject *)sp[-1])->GetVFunction(*current_statement++);
		break;

	 default:
#ifdef CHECK_VALID_OPCODE
		Sys_Error("Invalid opcode %d", current_statement[-1]);
#endif
		break;
	}

    goto func_loop;
	unguardfSlow(("(%s %d)", *func->Name, 
		current_statement - (int *)func->FirstStatement));
}

#endif

//==========================================================================
//
//  TProgs::ExecuteFunction
//
//==========================================================================

int TProgs::ExecuteFunction(FFunction *func)
{
	guard(TProgs::ExecuteFunction);
	FFunction		*prev_func;
	int				ret = 0;

	//	Run function
	prev_func = current_func;
	RunFunction(func);
	current_func = prev_func;

	//	Get return value
	if (func->Type)
    {
		ret = *(--pr_stackPtr);
	}

#ifdef CHECK_FOR_EMPTY_STACK
	//	After executing base function stack must be empty
    if (!current_func && pr_stackPtr != pr_stack + 1)
    {
    	Sys_Error("ExecuteFunction: Stack is not empty after executing function:\n%s\nstack = %p, sp = %p",
            *func->Name, pr_stack, pr_stackPtr);
    }
#endif

#ifdef CHECK_STACK_UNDERFLOW
	//	Check, if stack wasn't underflowed
	if (pr_stack[0] != STACK_ID)
   	{
   		Sys_Error("ExecuteFunction: Stack underflow in %s", *func->Name);
    }
#endif

#ifdef CHECK_STACK_OVERFLOW
	//	Check, if stack wasn't overflowed
	if (pr_stack[MAX_PROG_STACK - 1] != STACK_ID)
   	{
   		Sys_Error("ExecuteFunction: Stack overflow in %s", *func->Name);
    }
#endif

	//	All done
	return ret;
	unguardf(("(%s)", *func->Name));
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func)
{
#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 0)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 0",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 1)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 1",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 1;
	p[0] = parm1;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 2)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 2",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 2;
	p[0] = parm1;
	p[1] = parm2;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 3)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 3",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 3;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 4)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 4",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 4;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 5)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 5",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 5;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 6)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 6",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 6;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 7)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 7",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 7;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 8)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 8",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 8;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 9)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 9",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 9;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
	p[8] = parm9;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 10)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 10",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 10;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
	p[8] = parm9;
	p[9] = parm10;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 11)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 11",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 11;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
	p[8] = parm9;
	p[9] = parm10;
	p[10] = parm11;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 12)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 12",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 12;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
	p[8] = parm9;
	p[9] = parm10;
	p[10] = parm11;
	p[11] = parm12;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12, int parm13)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 13)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 13",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 13;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
	p[8] = parm9;
	p[9] = parm10;
	p[10] = parm11;
	p[11] = parm12;
	p[12] = parm13;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12, int parm13, int parm14)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 14)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 14",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 14;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
	p[8] = parm9;
	p[9] = parm10;
	p[10] = parm11;
	p[11] = parm12;
	p[12] = parm13;
	p[13] = parm14;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12, int parm13, int parm14, int parm15)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 15)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 15",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 15;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
	p[8] = parm9;
	p[9] = parm10;
	p[10] = parm11;
	p[11] = parm12;
	p[12] = parm13;
	p[13] = parm14;
	p[14] = parm15;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(FFunction *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12, int parm13, int parm14, int parm15, int parm16)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].NumParms != 16)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 16",
    		FuncName(fnum), Functions[fnum].NumParms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 16;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
	p[5] = parm6;
	p[6] = parm7;
	p[7] = parm8;
	p[8] = parm9;
	p[9] = parm10;
	p[10] = parm11;
	p[11] = parm12;
	p[12] = parm13;
	p[13] = parm14;
	p[14] = parm15;
	p[15] = parm16;
    return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::DumpProfile
//
//==========================================================================

void TProgs::DumpProfile(void)
{
	#define MAX_PROF	100
	int i;
	int profsort[MAX_PROF];
	int totalcount = 0;
	memset(profsort, 0, sizeof(profsort));
	for (i = 1; i < Progs->num_functions; i++)
	{
		if (!Functions[i].Profile1)
		{
			//	Never called
			continue;
		}
		for (int j = 0; j < MAX_PROF; j++)
		{
			totalcount += Functions[i].Profile2;
			if (Functions[profsort[j]].Profile2 <= Functions[i].Profile2)
			{
				for (int k = MAX_PROF - 1; k > j; k--)
				{
					profsort[k] = profsort[k - 1];
				}
				profsort[j] = i;
				break;
			}
		}
	}
	if (!totalcount)
	{
		return;
	}
	for (i = 0; i < MAX_PROF && profsort[i]; i++)
	{
		int fnum = profsort[i];
		GCon->Logf("%3.2f%% (%9d) %9d %s",
			(double)Functions[fnum].Profile2 * 100.0 / (double)totalcount,
			(int)Functions[fnum].Profile2, (int)Functions[fnum].Profile1,
			*Functions[fnum].Name);
	}
}

//==========================================================================
//
//	COMMAND ProgsTest
//
//==========================================================================

COMMAND(ProgsTest)
{
	TProgs tst;

	tst.Load("TSTPROGS");
	if (Argc() > 1)
	{
		tst.Exec(Argv(1));
	}
	tst.Unload();
}

//**************************************************************************
//
//	$Log$
//	Revision 1.33  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//
//	Revision 1.32  2004/12/22 07:37:21  dj_jl
//	Increased argument count limit.
//	
//	Revision 1.31  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.30  2002/11/02 17:09:55  dj_jl
//	Some debugging stuff.
//	
//	Revision 1.29  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.28  2002/05/03 17:04:35  dj_jl
//	Mangling of string pointers.
//	
//	Revision 1.27  2002/04/11 16:45:42  dj_jl
//	Ignoring natives in profiling.
//	
//	Revision 1.26  2002/03/16 17:53:12  dj_jl
//	Added opcode for pushing virtual function.
//	
//	Revision 1.25  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.24  2002/02/26 17:54:26  dj_jl
//	Importing special property info from progs and using it in saving.
//	
//	Revision 1.23  2002/02/16 16:29:26  dj_jl
//	Added support for bool variables
//	
//	Revision 1.22  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.21  2002/01/28 18:44:44  dj_jl
//	Fixed dynamic cast
//	
//	Revision 1.20  2002/01/25 18:06:53  dj_jl
//	Little changes for progs profiling
//	
//	Revision 1.19  2002/01/17 18:21:40  dj_jl
//	Fixed Hexen class bug
//	
//	Revision 1.18  2002/01/11 08:07:17  dj_jl
//	Added names to progs
//	
//	Revision 1.17  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.16  2002/01/04 18:22:13  dj_jl
//	Beautification
//	
//	Revision 1.15  2002/01/03 18:38:25  dj_jl
//	Added guard macros and core dumps
//	
//	Revision 1.14  2001/12/27 17:39:10  dj_jl
//	Added method count to VClass
//	
//	Revision 1.13  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.12  2001/12/12 19:27:46  dj_jl
//	Added dynamic cast
//	
//	Revision 1.11  2001/12/03 19:21:45  dj_jl
//	Added swaping with vector
//	
//	Revision 1.10  2001/12/01 17:43:13  dj_jl
//	Renamed ClassBase to VObject
//	
//	Revision 1.9  2001/10/27 07:48:25  dj_jl
//	Added constructors and destructors
//	
//	Revision 1.8  2001/10/04 17:24:21  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.7  2001/09/25 17:06:22  dj_jl
//	Added parent's vtable to vtable
//	
//	Revision 1.6  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.5  2001/08/31 17:28:00  dj_jl
//	Removed RANGECHECK
//	
//	Revision 1.4  2001/08/21 17:40:24  dj_jl
//	Real string pointers in progs
//	In devgame mode look for progs in <gamedir>/progs
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
