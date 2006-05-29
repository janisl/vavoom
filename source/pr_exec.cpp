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

#define DO_GUARD_SLOW	1
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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

FBuiltinInfo *FBuiltinInfo::Builtins;

VStack*				pr_stackPtr; 

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VMethod*		current_func = NULL;
static VStack		pr_stack[MAX_PROG_STACK];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	PR_Init
//
//==========================================================================

void PR_Init()
{
	//	Set stack ID for overflow / underflow checks
	pr_stack[0].i = STACK_ID;
	pr_stack[MAX_PROG_STACK - 1].i = STACK_ID;
	pr_stackPtr = pr_stack + 1;
}

//==========================================================================
//
//	PR_OnAbort
//
//==========================================================================

void PR_OnAbort()
{
	current_func = NULL;
	pr_stackPtr = pr_stack + 1;
}

//==========================================================================
//
//	PR_Profile1
//
//==========================================================================

extern "C" void PR_Profile1()
{
	current_func->Profile1++;
}

//==========================================================================
//
//	PR_Profile2
//
//==========================================================================

extern "C" void PR_Profile2()
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

void PR_Traceback()
{
	if (current_func)
	{
		Host_CoreDump("(%s)", *current_func->GetFullName());
	}
}

//==========================================================================
//
//	TProgs::Load
//
//==========================================================================

void TProgs::Load(const char *AName)
{
	guard(TProgs::Load);
	Pkg = VMemberBase::StaticLoadPackage(VName(AName, VName::AddLower8));
	unguard;
}

//==========================================================================
//
//	TProgs::Unload
//
//==========================================================================

void TProgs::Unload()
{
}

//==========================================================================
//
//  TProgs::FindStruct
//
//==========================================================================

VStruct* TProgs::FindStruct(VName InName, VClass* InClass)
{
	guard(TProgs::FindStruct);
	for (int i = 0; i < VMemberBase::GMembers.Num(); i++)
		if (VMemberBase::GMembers[i]->MemberType == MEMBER_Struct &&
			VMemberBase::GMembers[i]->Name == InName && 
			((VStruct*)VMemberBase::GMembers[i])->Outer == InClass)
			return (VStruct*)VMemberBase::GMembers[i];
	return NULL;
	unguard;
}

//==========================================================================
//
//  RunFunction
//
//==========================================================================

#ifdef __GNUC__
#define USE_COMPUTED_GOTO 1
#endif

#if USE_COMPUTED_GOTO
#define PR_VM_SWITCH(op)	goto *vm_labels[op];
#define PR_VM_CASE(x)		Lbl_ ## x:
#define PR_VM_BREAK			goto *vm_labels[*ip];
#define PR_VM_DEFAULT
#else
#define PR_VM_SWITCH(op)	switch(op)
#define PR_VM_CASE(x)		case x:
#define PR_VM_BREAK			break
#define PR_VM_DEFAULT		default:
#endif

#define ReadInt32(ip)		(*(int*)(ip))
#define ReadPtr(ip)			(*(void**)(ip))

static void RunFunction(VMethod *func)
{
	vuint8*		ip = NULL;
	VStack*		sp;
	VStack*		local_vars;

	guardSlow(RunFunction);
	current_func = func;

	if (func->Flags & FUNC_Native)
	{
		//	Native function, first statement is pointer to function.
		func->NativeFunc();
		return;
	}

	//	Keep stack pointer in register
	sp = pr_stackPtr;

	//	Setup local vars
	local_vars = sp - func->NumParms;
	memset(sp, 0, (func->NumLocals - func->NumParms) * sizeof(VStack));
	sp += func->NumLocals - func->NumParms;

	ip = (vuint8*)func->Statements.Ptr();

	//
	//	The main function loop
	//
	while (1)
	{
func_loop:

#if USE_COMPUTED_GOTO
		static void* vm_labels[] = {
#define DECLARE_OPC(name, args, argcount)	&&Lbl_OPC_ ## name
#define OPCODE_INFO
#include "progdefs.h"
		0 };
#endif

		PR_VM_SWITCH(*ip)
		{
		PR_VM_CASE(OPC_Done)
			Sys_Error("Empty function or invalid opcode");
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Call)
			pr_stackPtr = sp;
			RunFunction((VMethod*)ReadPtr(ip + 1));
			current_func = func;
			ip += 1 + sizeof(void*);
			sp = pr_stackPtr;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushVFunc)
			sp[0].p = ((VObject*)sp[-1].p)->GetVFunction(ReadInt32(ip + 1));
			ip += 5;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_ICall)
			sp--;
			pr_stackPtr = sp;
			RunFunction((VMethod*)sp->p);
			current_func = func;
			ip++;
			sp = pr_stackPtr;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Return)
			pr_stackPtr = local_vars;
			return;

		PR_VM_CASE(OPC_ReturnL)
			((VStack*)local_vars)[0] = sp[-1];
			pr_stackPtr = local_vars + 1;
			return;

		PR_VM_CASE(OPC_ReturnV)
			((VStack*)local_vars)[0] = sp[-3];
			((VStack*)local_vars)[1] = sp[-2];
			((VStack*)local_vars)[2] = sp[-1];
			pr_stackPtr = local_vars + 3;
			return;

		PR_VM_CASE(OPC_Goto)
			ip = (vuint8*)ReadPtr(ip + 1);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfGoto)
			if (sp[-1].i)
			{
				ip = (vuint8*)ReadPtr(ip + 1);
			}
			else
			{
				ip += 1 + sizeof(void*);
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotGoto)
			if (!sp[-1].i)
			{
				ip = (vuint8*)ReadPtr(ip + 1);
			}
			else
			{
				ip += 1 + sizeof(void*);
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfTopGoto)
			if (sp[-1].i)
			{
				ip = (vuint8*)ReadPtr(ip + 1);
			}
			else
			{
				ip += 1 + sizeof(void*);
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotTopGoto)
			if (!sp[-1].i)
			{
				ip = (vuint8*)ReadPtr(ip + 1);
			}
			else
			{
				ip += 1 + sizeof(void*);
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_CaseGoto)
			if (ReadInt32(ip + 1) == sp[-1].i)
			{
				ip = (vuint8*)ReadPtr(ip + 5);
				sp--;
			}
			else
			{
				ip += 5 + sizeof(void*);
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushNumber)
			sp->i = ReadInt32(ip + 1);
			ip += 5;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushName)
			sp->i = ReadInt32(ip + 1);
			ip += 5;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushString)
			sp->p = ReadPtr(ip + 1);
			ip += 1 + sizeof(void*);
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushClassId)
		PR_VM_CASE(OPC_PushState)
			sp->p = ReadPtr(ip + 1);
			ip += 1 + sizeof(void*);
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress)
			sp->p = &local_vars[ReadInt32(ip + 1)];
			ip += 5;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Offset)
			sp[-1].p = (vuint8*)sp[-1].p + ReadInt32(ip + 1);
			ip += 5;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_ArrayElement)
			sp[-2].p = (vuint8*)sp[-2].p + sp[-1].i * ReadInt32(ip + 1);
			ip += 5;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushPointed)
			ip++;
			sp[-1].i = *(vint32*)sp[-1].p;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VPushPointed)
			ip++;
			sp[1].f = ((TVec*)sp[-1].p)->z;
			sp[0].f = ((TVec*)sp[-1].p)->y;
			sp[-1].f = ((TVec*)sp[-1].p)->x;
			sp += 2;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushBool)
			{
				vint32 mask = ReadInt32(ip + 1);
				ip += 5;
				sp[-1].i = !!(*(vint32*)sp[-1].p & mask);
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushPointedDelegate)
			ip++;
			sp[0].p = ((void**)sp[-1].p)[1];
			sp[-1].p = ((void**)sp[-1].p)[0];
			sp++;
			PR_VM_BREAK;

#define BINOP(mem, op) \
	ip++; \
	sp[-2].mem = sp[-2].mem op sp[-1].mem; \
	sp--;
#define BINOP_Q(mem, op) \
	ip++; \
	sp[-2].mem op sp[-1].mem; \
	sp--;

		PR_VM_CASE(OPC_Add)
			BINOP_Q(i, +=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Subtract)
			BINOP_Q(i, -=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Multiply)
			BINOP_Q(i, *=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Divide)
			BINOP(i, /=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Modulus)
			BINOP_Q(i, %=);
			PR_VM_BREAK;

#define BOOLOP(mem, op) \
	ip++; \
	sp[-2].i = sp[-2].mem op sp[-1].mem; \
	sp--;

		PR_VM_CASE(OPC_Equals)
			BOOLOP(i, ==);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_NotEquals)
			BOOLOP(i, !=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Less)
			BOOLOP(i, <);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Greater)
			BOOLOP(i, >);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LessEquals)
			BOOLOP(i, <=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_GreaterEquals)
			BOOLOP(i, >=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AndLogical)
			BOOLOP(i, &&);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_OrLogical)
			BOOLOP(i, ||);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_NegateLogical)
			ip++;
			sp[-1].i = !sp[-1].i;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AndBitwise)
			BINOP_Q(i, &=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_OrBitwise)
			BINOP_Q(i, |=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_XOrBitwise)
			BINOP_Q(i, ^=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LShift)
			BINOP_Q(i, <<=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_RShift)
			BINOP_Q(i, >>=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_UnaryMinus)
			ip++;
			sp[-1].i = -sp[-1].i;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_BitInverse)
			ip++;
			sp[-1].i = ~sp[-1].i;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PreInc)
			ip++;
#ifdef __GNUC__
			sp[-1].i = ++(*(vint32*)sp[-1].p);
#else
			{
				vint32* ptr = (vint32*)sp[-1].p;
				++(*ptr);
				sp[-1].i = *ptr;
			}
#endif
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PreDec)
			ip++;
#ifdef __GNUC__
			sp[-1].i = --(*(vint32*)sp[-1].p);
#else
			{
				vint32* ptr = (vint32*)sp[-1].p;
				--(*ptr);
				sp[-1].i = *ptr;
			}
#endif
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PostInc)
			ip++;
#ifdef __GNUC__
			sp[-1].i = (*(vint32*)sp[-1].p)++;
#else
			{
				vint32* ptr = (vint32*)sp[-1].p;
				sp[-1].i = *ptr;
				(*ptr)++;
			}
#endif
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PostDec)
			ip++;
#ifdef __GNUC__
			sp[-1].i = (*(vint32*)sp[-1].p)--;
#else
			{
				vint32* ptr = (vint32*)sp[-1].p;
				sp[-1].i = *ptr;
				(*ptr)--;
			}
#endif
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IncDrop)
			ip++;
			(*(vint32*)sp[-1].p)++;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_DecDrop)
			ip++;
			(*(vint32*)sp[-1].p)--;
			sp--;
			PR_VM_BREAK;

#define ASSIGNOP(type, mem, op) \
	ip++; \
	*(type*)sp[-2].p op sp[-1].mem; \
	sp -= 2;

		PR_VM_CASE(OPC_AssignDrop)
			ASSIGNOP(vint32, i, =);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AddVarDrop)
			ASSIGNOP(vint32, i, +=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_SubVarDrop)
			ASSIGNOP(vint32, i, -=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_MulVarDrop)
			ASSIGNOP(vint32, i, *=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_DivVarDrop)
			ASSIGNOP(vint32, i, /=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_ModVarDrop)
			ASSIGNOP(vint32, i, %=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AndVarDrop)
			ASSIGNOP(vint32, i, &=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_OrVarDrop)
			ASSIGNOP(vint32, i, |=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_XOrVarDrop)
			ASSIGNOP(vint32, i, ^=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LShiftVarDrop)
			ASSIGNOP(vint32, i, <<=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_RShiftVarDrop)
			ASSIGNOP(vint32, i, >>=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FAdd)
			BINOP_Q(f, +=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FSubtract)
			BINOP_Q(f, -=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FMultiply)
			BINOP_Q(f, *=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FDivide)
			BINOP_Q(f, /=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FEquals)
			BOOLOP(f, ==);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FNotEquals)
			BOOLOP(f, !=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FLess)
			BOOLOP(f, <);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FGreater)
			BOOLOP(f, >);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FLessEquals)
			BOOLOP(f, <=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FGreaterEquals)
			BOOLOP(f, >=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FUnaryMinus)
			ip++;
			sp[-1].f = -sp[-1].f;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FAddVarDrop)
			ASSIGNOP(float, f, +=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FSubVarDrop)
			ASSIGNOP(float, f, -=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FMulVarDrop)
			ASSIGNOP(float, f, *=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_FDivVarDrop)
			ASSIGNOP(float, f, /=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VAdd)
			ip++;
			sp[-6].f += sp[-3].f;
			sp[-5].f += sp[-2].f;
			sp[-4].f += sp[-1].f;
			sp -= 3;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VSubtract)
			ip++;
			sp[-6].f -= sp[-3].f;
			sp[-5].f -= sp[-2].f;
			sp[-4].f -= sp[-1].f;
			sp -= 3;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VPreScale)
			{
				ip++;
				float scale = sp[-4].f;
				sp[-4].f = scale * sp[-3].f;
				sp[-3].f = scale * sp[-2].f;
				sp[-2].f = scale * sp[-1].f;
				sp--;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VPostScale)
			ip++;
			sp[-4].f *= sp[-1].f;
			sp[-3].f *= sp[-1].f;
			sp[-2].f *= sp[-1].f;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VIScale)
			ip++;
			sp[-4].f /= sp[-1].f;
			sp[-3].f /= sp[-1].f;
			sp[-2].f /= sp[-1].f;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VEquals)
			ip++;
			sp[-6].i = sp[-6].f == sp[-3].f && sp[-5].f == sp[-2].f &&
				sp[-4].f == sp[-1].f;
			sp -= 5;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VNotEquals)
			ip++;
			sp[-6].i = sp[-6].f != sp[-3].f || sp[-5].f != sp[-2].f ||
				sp[-4].f != sp[-1].f;
			sp -= 5;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VUnaryMinus)
			ip++;
			sp[-3].f = -sp[-3].f;
			sp[-2].f = -sp[-2].f;
			sp[-1].f = -sp[-1].f;
			PR_VM_BREAK;

#define VASSIGNOP(op) \
	{ \
		ip++; \
		TVec* ptr = (TVec*)sp[-4].p; \
		ptr->x op sp[-3].f; \
		ptr->y op sp[-2].f; \
		ptr->z op sp[-1].f; \
		sp -= 4; \
	}

		PR_VM_CASE(OPC_VAssignDrop)
			VASSIGNOP(=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VAddVarDrop)
			VASSIGNOP(+=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VSubVarDrop)
			VASSIGNOP(-=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VScaleVarDrop)
			ip++;
			*(TVec*)sp[-2].p *= sp[-1].f;
			sp -= 2;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VIScaleVarDrop)
			ip++;
			*(TVec*)sp[-2].p /= sp[-1].f;
			sp -= 2;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PtrEquals)
			BOOLOP(p, ==);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PtrNotEquals)
			BOOLOP(p, !=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Drop)
			ip++;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VDrop)
			ip++;
			sp -= 3;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Swap)
			{
				ip++;
				vint32 tmp = sp[-2].i;
				sp[-2].i = sp[-1].i;
				sp[-1].i = tmp;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Swap3)
			{
				ip++;
				vint32 tmp = sp[-4].i;
				sp[-4].i = sp[-3].i;
				sp[-3].i = sp[-2].i;
				sp[-2].i = sp[-1].i;
				sp[-1].i = tmp;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignPtrDrop)
			ASSIGNOP(void*, p, =);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignBool)
			{
				vint32 mask = ReadInt32(ip + 1);
				if (sp[-1].i)
					*(vint32*)sp[-2].p |= mask;
				else
					*(vint32*)sp[-2].p &= ~mask;
				ip += 5;
				sp -= 2;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignDelegate)
			ip++;
			((void**)sp[-3].p)[0] = sp[-2].p;
			((void**)sp[-3].p)[1] = sp[-1].p;
			sp -= 3;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_DynamicCast)
			sp[-1].p = sp[-1].p && ((VObject*)sp[-1].p)->IsA(
				(VClass*)ReadPtr(ip + 1)) ? sp[-1].p : 0;
			ip += 1 + sizeof(void*);
			PR_VM_BREAK;

		PR_VM_DEFAULT
			Sys_Error("Invalid opcode %d", *ip);
		}
	}

	goto func_loop;
	unguardfSlow(("(%s %d)", *func->GetFullName(),
		ip - func->Statements.Ptr()));
}

//==========================================================================
//
//  TProgs::ExecuteFunction
//
//==========================================================================

int TProgs::ExecuteFunction(VMethod *func)
{
	guard(TProgs::ExecuteFunction);
	VMethod		*prev_func;
	int				ret = 0;

	//	Run function
	prev_func = current_func;
	RunFunction(func);
	current_func = prev_func;

	//	Get return value
	if (func->Type)
	{
		--pr_stackPtr;
		ret = pr_stackPtr->i;
	}

#ifdef CHECK_FOR_EMPTY_STACK
	//	After executing base function stack must be empty
	if (!current_func && pr_stackPtr != pr_stack + 1)
	{
		Sys_Error("ExecuteFunction: Stack is not empty after executing function:\n%s\nstack = %p, oldsp = %p",
			*func->Name, pr_stack, pr_stackPtr);
	}
#endif

#ifdef CHECK_STACK_UNDERFLOW
	//	Check, if stack wasn't underflowed
	if (pr_stack[0].i != STACK_ID)
	{
		Sys_Error("ExecuteFunction: Stack underflow in %s", *func->Name);
	}
#endif

#ifdef CHECK_STACK_OVERFLOW
	//	Check, if stack wasn't overflowed
	if (pr_stack[MAX_PROG_STACK - 1].i != STACK_ID)
	{
		Sys_Error("ExecuteFunction: Stack overflow in %s", *func->Name);
	}
#endif

	//	All done
	return ret;
	unguardf(("(%s)", *func->GetFullName()));
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 0)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 0",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 1)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 1",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 1;
	p[0].i = parm1;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 2)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 2",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 2;
	p[0].i = parm1;
	p[1].i = parm2;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 3)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 3",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 3;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 4)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 4",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 4;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 5)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 5",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 5;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 6)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 6",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 6;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 7)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 7",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 7;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 8)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 8",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 8;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 9)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 9",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 9;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	p[8].i = parm9;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 10)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 10",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 10;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	p[8].i = parm9;
	p[9].i = parm10;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 11)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 11",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 11;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	p[8].i = parm9;
	p[9].i = parm10;
	p[10].i = parm11;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 12)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 12",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 12;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	p[8].i = parm9;
	p[9].i = parm10;
	p[10].i = parm11;
	p[11].i = parm12;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12, int parm13)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 13)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 13",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 13;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	p[8].i = parm9;
	p[9].i = parm10;
	p[10].i = parm11;
	p[11].i = parm12;
	p[12].i = parm13;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12, int parm13, int parm14)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 14)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 14",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 14;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	p[8].i = parm9;
	p[9].i = parm10;
	p[10].i = parm11;
	p[11].i = parm12;
	p[12].i = parm13;
	p[13].i = parm14;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12, int parm13, int parm14, int parm15)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 15)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 15",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 15;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	p[8].i = parm9;
	p[9].i = parm10;
	p[10].i = parm11;
	p[11].i = parm12;
	p[12].i = parm13;
	p[13].i = parm14;
	p[14].i = parm15;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::Exec
//
//==========================================================================

int TProgs::Exec(VMethod *func, int parm1, int parm2, int parm3, int parm4,
	int parm5, int parm6, int parm7, int parm8, int parm9, int parm10,
	int parm11, int parm12, int parm13, int parm14, int parm15, int parm16)
{
#ifdef CHECK_PARM_COUNT
	if (Functions[fnum]->NumParms != 16)
	{
		Sys_Error("TProgs::Exec: Function %s haves %d parms, not 16",
			FuncName(fnum), Functions[fnum]->NumParms);
	}
#endif
	VStack* p = pr_stackPtr;
	pr_stackPtr += 16;
	p[0].i = parm1;
	p[1].i = parm2;
	p[2].i = parm3;
	p[3].i = parm4;
	p[4].i = parm5;
	p[5].i = parm6;
	p[6].i = parm7;
	p[7].i = parm8;
	p[8].i = parm9;
	p[9].i = parm10;
	p[10].i = parm11;
	p[11].i = parm12;
	p[12].i = parm13;
	p[13].i = parm14;
	p[14].i = parm15;
	p[15].i = parm16;
	return ExecuteFunction(func);
}

//==========================================================================
//
//	TProgs::DumpProfile
//
//==========================================================================

void TProgs::DumpProfile()
{
	#define MAX_PROF	100
	int i;
	int profsort[MAX_PROF];
	int totalcount = 0;
	memset(profsort, 0, sizeof(profsort));
	for (i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->MemberType != MEMBER_Method)
		{
			continue;
		}
		VMethod* Func = (VMethod*)VMemberBase::GMembers[i];
		if (!Func->Profile1)
		{
			//	Never called
			continue;
		}
		for (int j = 0; j < MAX_PROF; j++)
		{
			totalcount += Func->Profile2;
			if (((VMethod*)VMemberBase::GMembers[profsort[j]])->Profile2 <= Func->Profile2)
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
		VMethod* Func = (VMethod*)VMemberBase::GMembers[profsort[i]];
		GCon->Logf("%3.2f%% (%9d) %9d %s",
			(double)Func->Profile2 * 100.0 / (double)totalcount,
			(int)Func->Profile2, (int)Func->Profile1, *Func->GetFullName());
	}
}

int TProgs::GetStringOffs(const char *Str)
{
	return Str - Pkg->Strings;
}

char *TProgs::StrAtOffs(int Offs)
{
	return Pkg->Strings + Offs;
}
//**************************************************************************
//
//	$Log$
//	Revision 1.55  2006/03/29 22:32:27  dj_jl
//	Changed console variables and command buffer to use dynamic strings.
//
//	Revision 1.54  2006/03/26 13:06:18  dj_jl
//	Implemented support for modular progs.
//	
//	Revision 1.53  2006/03/23 18:31:59  dj_jl
//	Members tree.
//	
//	Revision 1.52  2006/03/18 16:51:15  dj_jl
//	Renamed type class names, better code serialisation.
//	
//	Revision 1.51  2006/03/13 19:29:57  dj_jl
//	Clean function local variables.
//	
//	Revision 1.50  2006/03/12 20:06:02  dj_jl
//	States as objects, added state variable type.
//	
//	Revision 1.49  2006/03/12 12:54:49  dj_jl
//	Removed use of bitfields for portability reasons.
//	
//	Revision 1.48  2006/03/10 19:31:25  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.47  2006/03/06 13:02:32  dj_jl
//	Cleaning up references to destroyed objects.
//	
//	Revision 1.46  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//	
//	Revision 1.45  2006/03/02 23:24:35  dj_jl
//	Wad lump names stored as names.
//	
//	Revision 1.44  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.43  2006/02/25 17:09:35  dj_jl
//	Import all progs type info.
//	
//	Revision 1.42  2006/02/19 20:36:02  dj_jl
//	Implemented support for delegates.
//	
//	Revision 1.41  2006/02/17 19:23:47  dj_jl
//	Removed support for progs global variables.
//	
//	Revision 1.40  2006/02/15 23:27:41  dj_jl
//	Added script ID class attribute.
//	
//	Revision 1.39  2006/02/13 18:34:34  dj_jl
//	Moved all server progs global functions to classes.
//	
//	Revision 1.38  2005/12/14 18:54:53  dj_jl
//	Removed compiler limitations.
//	
//	Revision 1.37  2005/12/07 22:53:26  dj_jl
//	Moved compiler generated data out of globals.
//	
//	Revision 1.36  2005/11/24 20:06:47  dj_jl
//	Renamed opcodes.
//	
//	Revision 1.35  2005/11/20 15:52:03  dj_jl
//	Fixes for MacOS X.
//	
//	Revision 1.34  2005/11/07 23:00:24  dj_jl
//	Fixed a compiler warning.
//	
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
//	VMethod pointers used instead of the function numbers
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
