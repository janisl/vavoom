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
# define CHECK_STACK_UNDERFLOW
# define CHECK_FOR_EMPTY_STACK

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

#define ReadInt16(ip)		(*(vint16*)(ip))
#define ReadInt32(ip)		(*(vint32*)(ip))
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

	ip = func->Statements.Ptr();

	//
	//	The main function loop
	//
	while (1)
	{
func_loop:

#if USE_COMPUTED_GOTO
		static void* vm_labels[] = {
#define DECLARE_OPC(name, args)	&&Lbl_OPC_ ## name
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
			sp[0].p = ((VObject*)sp[-1].p)->GetVFunction(ReadInt16(ip + 1));
			ip += 3;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushVFuncB)
			sp[0].p = ((VObject*)sp[-1].p)->GetVFunction(ip[1]);
			ip += 2;
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

		PR_VM_CASE(OPC_GotoB)
			ip += ip[1];
			PR_VM_BREAK;

		PR_VM_CASE(OPC_GotoNB)
			ip -= ip[1];
			PR_VM_BREAK;

		PR_VM_CASE(OPC_GotoS)
			ip += ReadInt16(ip + 1);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Goto)
			ip += ReadInt32(ip + 1);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfGotoB)
			if (sp[-1].i)
			{
				ip += ip[1];
			}
			else
			{
				ip += 2;
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfGotoNB)
			if (sp[-1].i)
			{
				ip -= ip[1];
			}
			else
			{
				ip += 2;
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfGotoS)
			if (sp[-1].i)
			{
				ip += ReadInt16(ip + 1);
			}
			else
			{
				ip += 3;
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfGoto)
			if (sp[-1].i)
			{
				ip += ReadInt32(ip + 1);
			}
			else
			{
				ip += 5;
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotGotoB)
			if (!sp[-1].i)
			{
				ip += ip[1];
			}
			else
			{
				ip += 2;
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotGotoNB)
			if (!sp[-1].i)
			{
				ip -= ip[1];
			}
			else
			{
				ip += 2;
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotGotoS)
			if (!sp[-1].i)
			{
				ip += ReadInt16(ip + 1);
			}
			else
			{
				ip += 3;
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotGoto)
			if (!sp[-1].i)
			{
				ip += ReadInt32(ip + 1);
			}
			else
			{
				ip += 5;
			}
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfTopGotoB)
			if (sp[-1].i)
			{
				ip += ip[1];
			}
			else
			{
				ip += 2;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfTopGotoNB)
			if (sp[-1].i)
			{
				ip -= ip[1];
			}
			else
			{
				ip += 2;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfTopGotoS)
			if (sp[-1].i)
			{
				ip += ReadInt16(ip + 1);
			}
			else
			{
				ip += 3;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfTopGoto)
			if (sp[-1].i)
			{
				ip += ReadInt32(ip + 1);
			}
			else
			{
				ip += 5;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotTopGotoB)
			if (!sp[-1].i)
			{
				ip += ip[1];
			}
			else
			{
				ip += 2;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotTopGotoNB)
			if (!sp[-1].i)
			{
				ip -= ip[1];
			}
			else
			{
				ip += 2;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotTopGotoS)
			if (!sp[-1].i)
			{
				ip += ReadInt16(ip + 1);
			}
			else
			{
				ip += 3;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_IfNotTopGoto)
			if (!sp[-1].i)
			{
				ip += ReadInt32(ip + 1);
			}
			else
			{
				ip += 5;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_CaseGotoB)
			if (ip[1] == sp[-1].i)
			{
				ip += ReadInt16(ip + 2);
				sp--;
			}
			else
			{
				ip += 4;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_CaseGotoS)
			if (ReadInt16(ip + 1) == sp[-1].i)
			{
				ip += ReadInt16(ip + 3);
				sp--;
			}
			else
			{
				ip += 5;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_CaseGoto)
			if (ReadInt32(ip + 1) == sp[-1].i)
			{
				ip += ReadInt16(ip + 5);
				sp--;
			}
			else
			{
				ip += 7;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushNumber0)
			ip++;
			sp->i = 0;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushNumber1)
			ip++;
			sp->i = 1;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushNumberB)
			sp->i = ip[1];
			ip += 2;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushNumberS)
			sp->i = ReadInt16(ip + 1);
			ip += 3;
			sp++;
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

		PR_VM_CASE(OPC_PushNameS)
			sp->i = ReadInt16(ip + 1);
			ip += 3;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushNameB)
			sp->i = ip[1];
			ip += 2;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushString)
			sp->p = ReadPtr(ip + 1);
			ip += 1 + sizeof(void*);
			sp++;
			{
				const char* S = (const char*)sp[-1].p;
				sp[-1].p = NULL;
				*(VStr*)&sp[-1].p = S;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushClassId)
		PR_VM_CASE(OPC_PushState)
			sp->p = ReadPtr(ip + 1);
			ip += 1 + sizeof(void*);
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushNull)
			ip++;
			sp->p = NULL;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress0)
			ip++;
			sp->p = &local_vars[0];
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress1)
			ip++;
			sp->p = &local_vars[1];
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress2)
			ip++;
			sp->p = &local_vars[2];
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress3)
			ip++;
			sp->p = &local_vars[3];
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress4)
			ip++;
			sp->p = &local_vars[4];
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress5)
			ip++;
			sp->p = &local_vars[5];
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress6)
			ip++;
			sp->p = &local_vars[6];
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddress7)
			ip++;
			sp->p = &local_vars[7];
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddressB)
			sp->p = &local_vars[ip[1]];
			ip += 2;
			sp++;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_LocalAddressS)
			sp->p = &local_vars[ReadInt16(ip + 1)];
			ip += 3;
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

		PR_VM_CASE(OPC_OffsetS)
			sp[-1].p = (vuint8*)sp[-1].p + ReadInt16(ip + 1);
			ip += 3;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_OffsetB)
			sp[-1].p = (vuint8*)sp[-1].p + ip[1];
			ip += 2;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_ArrayElement)
			sp[-2].p = (vuint8*)sp[-2].p + sp[-1].i * ReadInt32(ip + 1);
			ip += 5;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_ArrayElementS)
			sp[-2].p = (vuint8*)sp[-2].p + sp[-1].i * ReadInt16(ip + 1);
			ip += 3;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_ArrayElementB)
			sp[-2].p = (vuint8*)sp[-2].p + sp[-1].i * ip[1];
			ip += 2;
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

		PR_VM_CASE(OPC_PushPointedPtr)
			ip++;
			sp[-1].p = *(void**)sp[-1].p;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushBool0)
			{
				vuint32 mask = ip[1];
				ip += 2;
				sp[-1].i = !!(*(vint32*)sp[-1].p & mask);
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushBool1)
			{
				vuint32 mask = ip[1] << 8;
				ip += 2;
				sp[-1].i = !!(*(vint32*)sp[-1].p & mask);
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushBool2)
			{
				vuint32 mask = ip[1] << 16;
				ip += 2;
				sp[-1].i = !!(*(vint32*)sp[-1].p & mask);
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushBool3)
			{
				vuint32 mask = ip[1] << 24;
				ip += 2;
				sp[-1].i = !!(*(vint32*)sp[-1].p & mask);
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PushPointedStr)
			{
				ip++;
				VStr* Ptr = (VStr*)sp[-1].p;
				sp[-1].p = NULL;
				*(VStr*)&sp[-1].p = *Ptr;
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

		PR_VM_CASE(OPC_VFixParam)
			{
				vint32 Idx = ip[1];
				ip += 2;
				TVec* v = (TVec*)&local_vars[Idx];
				v->y = local_vars[Idx + 1].f;
				v->z = local_vars[Idx + 2].f;
			}
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

		PR_VM_CASE(OPC_StrToBool)
			{
				ip++;
				bool Val = *(VStr*)&sp[-1].p;
				((VStr*)&sp[-1].p)->Clean();
				sp[-1].i = Val;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignStrDrop)
			ip++;
			*(VStr*)sp[-2].p = *(VStr*)&sp[-1].p;
			((VStr*)&sp[-1].p)->Clean();
			sp -= 2;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PtrEquals)
			BOOLOP(p, ==);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PtrNotEquals)
			BOOLOP(p, !=);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_PtrToBool)
			ip++;
			sp[-1].i = !!sp[-1].p;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_ClearPointedStr)
			((VStr*)sp[-1].p)->Clean();
			ip += 1;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_ClearPointedStruct)
			((VStruct*)ReadPtr(ip + 1))->DestructObject((byte*)sp[-1].p);
			ip += 1 + sizeof(void*);
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Drop)
			ip++;
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_VDrop)
			ip++;
			sp -= 3;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_DropStr)
			ip++;
			((VStr*)&sp[-1].p)->Clean();
			sp--;
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Swap)
			{
				ip++;
				VStack tmp = sp[-2];
				sp[-2] = sp[-1];
				sp[-1] = tmp;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_Swap3)
			{
				ip++;
				VStack tmp = sp[-4];
				sp[-4] = sp[-3];
				sp[-3] = sp[-2];
				sp[-2] = sp[-1];
				sp[-1] = tmp;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignPtrDrop)
			ASSIGNOP(void*, p, =);
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignBool0)
			{
				vuint32 mask = ip[1];
				if (sp[-1].i)
					*(vint32*)sp[-2].p |= mask;
				else
					*(vint32*)sp[-2].p &= ~mask;
				ip += 2;
				sp -= 2;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignBool1)
			{
				vuint32 mask = ip[1] << 8;
				if (sp[-1].i)
					*(vint32*)sp[-2].p |= mask;
				else
					*(vint32*)sp[-2].p &= ~mask;
				ip += 2;
				sp -= 2;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignBool2)
			{
				vuint32 mask = ip[1] << 16;
				if (sp[-1].i)
					*(vint32*)sp[-2].p |= mask;
				else
					*(vint32*)sp[-2].p &= ~mask;
				ip += 2;
				sp -= 2;
			}
			PR_VM_BREAK;

		PR_VM_CASE(OPC_AssignBool3)
			{
				vuint32 mask = ip[1] << 24;
				if (sp[-1].i)
					*(vint32*)sp[-2].p |= mask;
				else
					*(vint32*)sp[-2].p &= ~mask;
				ip += 2;
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
//  VObject::ExecuteFunction
//
//==========================================================================

VStack VObject::ExecuteFunction(VMethod *func)
{
	guard(VObject::ExecuteFunction);
	VMethod		*prev_func;
	VStack		ret;

	ret.i = 0;
	ret.p = NULL;
	//	Run function
	prev_func = current_func;
	RunFunction(func);
	current_func = prev_func;

	//	Get return value
	if (func->Type)
	{
		--pr_stackPtr;
		ret = *pr_stackPtr;
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
//	VObject::DumpProfile
//
//==========================================================================

void VObject::DumpProfile()
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
