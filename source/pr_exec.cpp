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
# define CHECK_PARM_COUNT
//# define CHECK_RUNAWAY
//	Not needed, if PROGS are correct
# define CHECK_STACK_UNDERFLOW
//# define CHECK_FUNC_NUM
//# define CHECK_VALID_OPCODE
# define CHECK_FOR_EMPTY_STACK
//# define CHECK_VALID_VAR_NUM

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern builtin_info_t	BuiltinInfo[];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

static int				pr_stack[MAX_PROG_STACK];
int						*pr_stackPtr;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

TProgs*					current_progs;
dprograms_t*			progs;
char*					pr_strings;
int*					pr_globals;
int*					pr_statements;
dfunction_t*			pr_functions;
globaldef_t*			pr_globaldefs;
builtin_t*				pr_builtins;
int						current_func = 0;
dword*					pr_profile1;
dword*					pr_profile2;

int						function_call_trace[256];
int						function_call_depth;

#ifdef CHECK_RUNAWAY
int						runaway;
int						caller_func;
#endif

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
}

//==========================================================================
//
//	PR_OnAbort
//
//==========================================================================

void PR_OnAbort(void)
{
	current_func = 0;
	pr_stackPtr = pr_stack + 1;
}

//==========================================================================
//
//  PR_FuncName
//
//==========================================================================

static char* PR_FuncName(int fnum)
{
	if (fnum <= 0 || fnum >= progs->num_functions)
	{
		return "NULL function";
	}

	return pr_strings + pr_functions[fnum].s_name;
}

//==========================================================================
//
//	PR_Profile1
//
//==========================================================================

extern "C" void PR_Profile1(void)
{
	pr_profile1[current_func]++;
}

//==========================================================================
//
//	PR_Profile2
//
//==========================================================================

extern "C" void PR_Profile2(void)
{
	if (current_func)
	{
		pr_profile2[current_func]++;
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
		cond << "\nCurrently running PROG function " << current_func
			<< " (" << PR_FuncName(current_func) << ")\n\n";
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
	int		i;
	int		len;
	char	progfilename[256];

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
		Sys_Error("Progs has wrong version number (%i should be %i)", Progs->version, PROG_VERSION);

	Strings = (char*)Progs + Progs->ofs_strings;
	Statements = (int*)((byte*)Progs + Progs->ofs_statements);
	Globals = (int*)((byte*)Progs + Progs->ofs_globals);
	Functions = (dfunction_t *)((byte *)Progs + Progs->ofs_functions);
	Globaldefs = (globaldef_t *)((byte *)Progs + Progs->ofs_globaldefs);
	ClassInfo = (dclassinfo_t *)((byte *)Progs + Progs->ofs_classinfo);

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
		Functions[i].s_name = LittleLong(Functions[i].s_name);
		Functions[i].first_statement = LittleLong(Functions[i].first_statement);
		Functions[i].num_parms = LittleShort(Functions[i].num_parms);
		Functions[i].num_locals = LittleShort(Functions[i].num_locals);
		Functions[i].type = LittleLong(Functions[i].type);
	}
	for (i = 0; i < Progs->num_globaldefs; i++)
	{
		Globaldefs[i].type = LittleShort(Globaldefs[i].type);
		Globaldefs[i].ofs = LittleShort(Globaldefs[i].ofs);
		Globaldefs[i].s_name = LittleLong(Globaldefs[i].s_name);
	}
	for (i = 0; i < Progs->num_classinfo; i++)
	{
		ClassInfo[i].s_name = LittleLong(ClassInfo[i].s_name);
		ClassInfo[i].vtable = LittleLong(ClassInfo[i].vtable);
		ClassInfo[i].size = LittleLong(ClassInfo[i].size);
		ClassInfo[i].parent = LittleLong(ClassInfo[i].parent);
	}

	//	Setup string pointers in globals
	byte *globalinfo = (byte*)Progs + Progs->ofs_globalinfo;
	for (i = 0; i < Progs->num_globals; i++)
	{
		if (globalinfo[i] == 1)
		{
			Globals[i] += int(Strings);
		}
	}

	//	Set up builtins
	Builtins = (builtin_t*)Z_Malloc(Progs->num_builtins * sizeof(builtin_t), PU_STATIC, 0);
	for (i = 0; i < Progs->num_builtins; i++)
    {
    	//	Default builtin
    	Builtins[i] = PF_Fixme;
    }

	for (i = 1; i < Progs->num_functions; i++)
    {
    	int		j;

		if (Functions[i].num_parms > 8)
			Sys_Error("Function haves more than 8 params");
        for (j=0; BuiltinInfo[j].name; j++)
        {
        	if (!strcmp(BuiltinInfo[j].name, FuncName(i)))
			{
            	if (Functions[i].first_statement < 0)
                {
                	int		num;

                    num = -Functions[i].first_statement;
					if (num >= Progs->num_builtins)
						Sys_Error("Bad builtin number %d", num);
                	Builtins[num] = BuiltinInfo[j].func;
					break;
                }
                else
                {
                	Sys_Error("PR_LoadProgs: Builtin %s redefined", BuiltinInfo[j].name);
                }
            }
        }
#if defined CLIENT && defined SERVER
		if (!BuiltinInfo[j].name && Functions[i].first_statement < 0)
        {
        	//	Don't abort with error, because it will be done, when this
            // function will be called (if it will be called).
        	cond << "WARNING: Builtin " << FuncName(i) << " not found!\n";
        }
#endif
    }

	//  Setup parent virtual tables
	for (i = 0; i < Progs->num_classinfo; i++)
	{
		if (ClassInfo[i].parent)
		{
			int *vtable = Globals + ClassInfo[i].vtable;
			int *pvtable = Globals + ClassInfo[ClassInfo[i].parent].vtable;
			vtable[2] = (int)pvtable;
		}
	}

	Profile1 = (dword*)Z_Calloc(Progs->num_functions * 4);
	Profile2 = (dword*)Z_Calloc(Progs->num_functions * 4);

	//	Execute initialization function
	Exec("main");
}

//==========================================================================
//
//	TProgs::Unload
//
//==========================================================================

void TProgs::Unload(void)
{
	Z_Free(Progs);
	Z_Free(Builtins);
	Z_Free(Profile1);
	Z_Free(Profile2);
}

//==========================================================================
//
//	TProgs::CheckFuncNumForName
//
//==========================================================================

int TProgs::CheckFuncNumForName(const char* name)
{
	int		i;

	for (i=1; i<Progs->num_functions; i++)
    {
    	if (!strcmp(Strings + Functions[i].s_name, name))
		{
        	return i;
		}
    }
	return 0;
}

//==========================================================================
//
//	TProgs::FuncNumForName
//
//==========================================================================

int TProgs::FuncNumForName(const char* name)
{
	int		i;

	i = CheckFuncNumForName(name);
    if (!i)
    {
    	Sys_Error("FuncNumForName: function %s not found", name);
    }
    return i;
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
    	if (!strcmp(Strings + Globaldefs[i].s_name, name))
		{
        	return Globaldefs[i].ofs;
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
	if (fnum <= 0 || fnum >= Progs->num_functions)
	{
		return "NULL function";
	}

	return Strings + Functions[fnum].s_name;
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

#ifdef USEASM

//	Use asm version
extern "C" void RunFunction(int fnum);

#else

static void RunFunction(int fnum)
{
	int			*current_statement;
	int			*sp;
	int			*local_vars;

    current_func = fnum;

#ifdef CHECK_FUNC_NUM
	if (fnum <= 0 || fnum >= progs->num_functions)
	{
		Sys_Error("RunFunction: NULL function");
	}
#endif

	if (pr_functions[fnum].first_statement < 0)
	{
		//	Negative first statements are built in functions. Don't need to
		// check builtin number (already done when setup builtins).
#ifdef CHECK_RUNAWAY
		if (!--runaway)
        {
			Sys_Error("RunFunction: runaway loop error");
		}
#endif
		pr_builtins[-pr_functions[fnum].first_statement]();
		return;
	}

	//	Keep stack pointer in register
	sp = pr_stackPtr;

	//	Setup local vars
    local_vars = sp - pr_functions[fnum].num_parms;
	sp += pr_functions[fnum].num_locals - pr_functions[fnum].num_parms;

	current_statement = pr_statements + pr_functions[fnum].first_statement;

	//
    //	The main function loop
    //
    //	I realy hate using goto in a C program, but this is the only way
    // how to force gcc to create a jump directly here. while(1) would be
    // better, but then gcc, even with optimizations, creates an dummy check,
    // which only takes time.
    //
 func_loop:

#ifdef CHECK_RUNAWAY
	if (!--runaway)
    {
		Sys_Error("RunFunction: runaway loop error");
	}
#endif
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

	 case OPC_GLOBALADDRESS:
#ifdef CHECK_VALID_VAR_NUM
		if (*current_statement < 0 || *current_statement >= progs->num_globaldefs)
		{
	    	Sys_Error("Bad global num %d", *current_statement);
	    }
#endif
		*sp++ = (int)&pr_globals[pr_globaldefs[*current_statement++].ofs];
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

	 case OPC_UDIVIDE:
        sp--;
		sp[-1] = (unsigned)sp[-1] / (unsigned)*sp;
		break;

	 case OPC_UMODULUS:
        sp--;
		sp[-1] = (unsigned)sp[-1] % (unsigned)*sp;
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

	 case OPC_ULT:
        sp--;
		sp[-1] = (unsigned)sp[-1] < (unsigned)*sp;
		break;

	 case OPC_UGT:
        sp--;
		sp[-1] = (unsigned)sp[-1] > (unsigned)*sp;
		break;

	 case OPC_ULE:
        sp--;
		sp[-1] = (unsigned)sp[-1] <= (unsigned)*sp;
		break;

	 case OPC_UGE:
        sp--;
		sp[-1] = (unsigned)sp[-1] >= (unsigned)*sp;
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

	 case OPC_URSHIFT:
        sp--;
		sp[-1] = (unsigned)sp[-1] >> (unsigned)*sp;
		break;

	 case OPC_UNARYMINUS:
		sp[-1] = -sp[-1];
		break;

	 case OPC_BITINVERSE:
		sp[-1] = ~sp[-1];
		break;

	 case OPC_CALL:
		pr_stackPtr = sp;
	    RunFunction(*current_statement++);
		current_func = fnum;
		sp = pr_stackPtr;
		break;

	 case OPC_GOTO:
		current_statement = pr_statements + *current_statement;
		break;

	 case OPC_IFGOTO:
        sp--;
		if (*sp)
		{
			current_statement = pr_statements + *current_statement;
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
	    	current_statement = pr_statements + *current_statement;
		}
		else
	    {
	        current_statement++;
	    }
		break;

	 case OPC_CASEGOTO:
		if (*current_statement++ == sp[-1])
	    {
	    	current_statement = pr_statements + *current_statement;
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

	 case OPC_UDIVVAR:
        sp--;
		*(unsigned*)sp[-1] /= (unsigned)*sp;
		sp[-1] = *(int*)sp[-1];
		break;

	 case OPC_UMODVAR:
        sp--;
		*(unsigned*)sp[-1] %= (unsigned)*sp;
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

	 case OPC_URSHIFTVAR:
        sp--;
		*(unsigned*)sp[-1] >>= (unsigned)*sp;
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
	    	current_statement = pr_statements + *current_statement;
	    }
		else
	    {
	        current_statement++;
	    }
		break;

	 case OPC_IFNOTTOPGOTO:
	    if (!sp[-1])
	    {
	    	current_statement = pr_statements + *current_statement;
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

	 case OPC_UDIVVAR_DROP:
        sp--;
		*(unsigned*)sp[-1] /= (unsigned)*sp;
		sp--;
		break;

	 case OPC_UMODVAR_DROP:
        sp--;
		*(unsigned*)sp[-1] %= (unsigned)*sp;
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

	 case OPC_URSHIFTVAR_DROP:
        sp--;
		*(unsigned*)sp[-1] >>= (unsigned)*sp;
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
	    RunFunction(*sp);
		current_func = fnum;
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

	 case OPC_PUSHSTRING:
		*sp++ = int(pr_strings) + (*current_statement++);
		break;

//=====================================

	 case OPC_COPY:
		*sp = sp[-1];
		sp++;
		break;

	 default:
#ifdef CHECK_VALID_OPCODE
		Sys_Error("Invalid opcode %d", current_statement[-1]);
#endif
		break;
	}

    goto func_loop;
}

#endif

//==========================================================================
//
//  TProgs::ExecuteFunction
//
//==========================================================================

int TProgs::ExecuteFunction(int fnum)
{
	int		prev_func;
	int		ret = 0;

#ifdef CHECK_RUNAWAY
	//	If this function is base function, then init runaway check
    if (!current_func)
    {
    	caller_func = fnum;
		runaway = 100000;
    }
#endif

#ifdef CHECK_FUNC_NUM
	if (fnum <= 0 || fnum >= Progs->num_functions)
	{
		Sys_Error("RunFunction: NULL function");
	}
#endif

	current_progs = this;
	progs = Progs;
	pr_strings = Strings;
	pr_globals = Globals;
	pr_statements =	Statements;
	pr_functions = Functions;
	pr_globaldefs = Globaldefs;
	pr_builtins = Builtins;
	pr_profile1 = Profile1;
	pr_profile2 = Profile2;

	//	Run function
	prev_func = current_func;
	RunFunction(fnum);
	current_func = prev_func;

	//	Get return value
	if (Functions[fnum].type)
    {
		ret = *(--pr_stackPtr);
	}

#ifdef CHECK_FOR_EMPTY_STACK
	//	After executing base function stack must be empty
    if (!current_func && pr_stackPtr != pr_stack + 1)
    {
    	Sys_Error("ExecuteFunction: Stack is not empty after executing function:\n%s\nstack = %p, sp = %p",
            FuncName(fnum), pr_stack, pr_stackPtr);
    }
#endif

#ifdef CHECK_STACK_UNDERFLOW
	//	Check, if stack wasn't underflowed
	if (pr_stack[0] != STACK_ID)
   	{
   		Sys_Error("ExecuteFunction: Stack underflow in %s", FuncName(fnum));
    }
#endif

#ifdef CHECK_STACK_OVERFLOW
	//	Check, if stack wasn't overflowed
	if (pr_stack[MAX_PROG_STACK - 1] != STACK_ID)
   	{
   		Sys_Error("ExecuteFunction: Stack overflow in %s", FuncName(fnum));
    }
#endif

	//	All done
	return ret;
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum)
{
#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 0)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 0",
    		FuncName(fnum), Functions[fnum].num_parms);
    }
#endif
    return ExecuteFunction(fnum);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum, int parm1)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 1)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 1",
    		FuncName(fnum), Functions[fnum].num_parms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 1;
	p[0] = parm1;
    return ExecuteFunction(fnum);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum, int parm1, int parm2)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 2)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 2",
    		FuncName(fnum), Functions[fnum].num_parms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 2;
	p[0] = parm1;
	p[1] = parm2;
    return ExecuteFunction(fnum);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum, int parm1, int parm2, int parm3)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 3)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 3",
    		FuncName(fnum), Functions[fnum].num_parms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 3;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
    return ExecuteFunction(fnum);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum, int parm1, int parm2, int parm3, int parm4)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 4)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 4",
    		FuncName(fnum), Functions[fnum].num_parms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 4;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
    return ExecuteFunction(fnum);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum, int parm1, int parm2, int parm3, int parm4,
								int parm5)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 5)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 5",
    		FuncName(fnum), Functions[fnum].num_parms);
    }
#endif
	p = pr_stackPtr;
	pr_stackPtr += 5;
	p[0] = parm1;
	p[1] = parm2;
	p[2] = parm3;
	p[3] = parm4;
	p[4] = parm5;
    return ExecuteFunction(fnum);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum, int parm1, int parm2, int parm3, int parm4,
								int parm5, int parm6)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 6)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 6",
    		FuncName(fnum), Functions[fnum].num_parms);
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
    return ExecuteFunction(fnum);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum, int parm1, int parm2, int parm3, int parm4,
								int parm5, int parm6, int parm7)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 7)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 7",
    		FuncName(fnum), Functions[fnum].num_parms);
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
    return ExecuteFunction(fnum);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(int fnum, int parm1, int parm2, int parm3, int parm4,
								int parm5, int parm6, int parm7, int parm8)
{
	int		*p;

#ifdef CHECK_PARM_COUNT
    if (Functions[fnum].num_parms != 8)
    {
    	Sys_Error("TProgs::Exec: Function %s haves %d parms, not 8",
    		FuncName(fnum), Functions[fnum].num_parms);
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
    return ExecuteFunction(fnum);
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
		if (!Profile1[i])
		{
			//	Never called
			continue;
		}
		for (int j = 0; j < MAX_PROF; j++)
		{
			totalcount += Profile2[i];
			if (Profile2[profsort[j]] <= Profile2[i])
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
		con << va("%3.2f%% (%6d) %6d %s\n",
			(double)Profile2[fnum] * 100.0 / (double)totalcount,
			(int)Profile2[fnum], (int)Profile1[fnum], FuncName(fnum));
	}
}

//==========================================================================
//
//	TProgs::GetClassID
//
//==========================================================================

int TProgs::GetClassID(const char *name)
{
	for (int i = 0; i < Progs->num_classinfo; i++)
	{
		if (!strcmp(name, Strings + ClassInfo[i].s_name))
		{
			return i;
		}
	}
	Sys_Error("Class %s not found", name);
#ifndef __GNUC__
	return 0;// Shut up compiler
#endif
}

//==========================================================================
//
//	TProgs::Spawn
//
//==========================================================================

VObject *TProgs::Spawn(int cid, int tag)
{
	try
	{
		VObject *Obj = (VObject*)Z_Calloc(ClassInfo[cid].size, tag, 0);
		Obj->vtable = Globals + ClassInfo[cid].vtable;
		Exec(Obj->vtable[4], (int)Obj);
		return Obj;
	}
	catch (...)
	{
		dprintf("- TProgs::Spawn\n");
		throw;
	}
}

//==========================================================================
//
//	TProgs::Destroy
//
//==========================================================================

void TProgs::Destroy(VObject *buf)
{
	Exec(buf->vtable[5], (int)buf);
	Z_Free(buf);
}

//==========================================================================
//
//	TProgs::CanCast
//
//==========================================================================

bool TProgs::CanCast(VObject *object, int cid)
{
	return CanCast(object->vtable[0], cid);
}

//==========================================================================
//
//	TProgs::CanCast
//
//==========================================================================

bool TProgs::CanCast(int fromid, int cid)
{
	while (fromid)
	{
		if (fromid == cid)
		{
			return true;
		}
		fromid = ClassInfo[fromid].parent;
	}
	return false;
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
