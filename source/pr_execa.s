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
//**	x86 assembly-language PROGS virtual machine code.
//**	
//**************************************************************************

//#define PROGS_PROFILE

#include "asm_i386.h"

#ifdef USEASM

//	External variables for progs
.extern	C(pr_stackPtr)
.extern	C(current_func)

.extern	C(PR_RFInvalidOpcode)
.extern	C(PR_DynamicCast)

.extern	C(TestFunction)

//==========================================================================
//
//  RunFunction
//
//	edi		- Current statement pointer
//	esi		- Stack pointer
//	ebp		- Local vars pointer
//
//==========================================================================

#define OFFS_FUNC		4+16

.text

	Align16
.globl C(RunFunction)
C(RunFunction):
	//	Save registers
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	//	Set current function num
	movl	OFFS_FUNC(%esp),%edi
	movl	%edi,C(current_func)
#ifdef PROGS_PROFILE
	call	C(PR_Profile1)
#endif

    //	Check for builtin
	testw	$0x0001,14(%edi)
	jz		LINTERPRET_FUNCTION

    //	Builtin function call
	call	*4(%edi)
	jmp		LEND_RUN_FUNCTION

	//---------------------------------
    //
    //	Virtual machine
    //
	//---------------------------------

	Align4
LINTERPRET_FUNCTION:
	//	Set up stack pinter
	movl	C(pr_stackPtr),%esi

	//	Get local vars
	movzwl	8(%edi),%eax
	movzwl	10(%edi),%edx
	sall	$2,%eax
	sall	$2,%edx
	subl	%eax,%esi
	movl	%esi,%ebp
	addl	%edx,%esi

    //	Set up statement pointer and jump to the first statement
	movl	4(%edi),%edi
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//
	//	Opcode table
    //
	Align4
LOPCODE_TABLE:
	.long	LOPC_Done
	.long	LOPC_Return
	.long	LOPC_PushNumber
	.long	LOPC_PushPointed
	.long	LOPC_LocalAddress
	.long	LOPC_GlobalAddress
	.long	LOPC_Add
	.long	LOPC_Subtract
	.long	LOPC_Multiply
	.long	LOPC_Divide

	.long	LOPC_Modulus
	.long	LOPC_Equals
	.long	LOPC_NotEquals
	.long	LOPC_Less
	.long	LOPC_Greater
	.long	LOPC_LessEquals
	.long	LOPC_GreaterEquals
	.long	LOPC_AndLogical
	.long	LOPC_OrLogical
	.long	LOPC_NegateLogical

	.long	LOPC_AndBitwise
	.long	LOPC_OrBitwise
	.long	LOPC_XOrBitwise
	.long	LOPC_LShift
	.long	LOPC_RShift
	.long	LOPC_UnaryMinus
	.long	LOPC_BitInverse
	.long	LOPC_Call
	.long	LOPC_Goto
	.long	LOPC_IfGoto

	.long	LOPC_IfNotGoto
	.long	LOPC_CaseGoto
	.long	LOPC_Drop
	.long	LOPC_Assign
	.long	LOPC_AddVar
	.long	LOPC_SubVar
	.long	LOPC_MulVar
	.long	LOPC_DivVar
	.long	LOPC_ModVar
	.long	LOPC_AndVar

	.long	LOPC_OrVar
	.long	LOPC_XOrVar
	.long	LOPC_LShiftVar
	.long	LOPC_RShiftVar
	.long	LOPC_PreInc
	.long	LOPC_PreDec
	.long	LOPC_PostInc
	.long	LOPC_PostDec
	.long	LOPC_IfTopGoto
	.long	LOPC_IfNotTopGoto

	.long	LOPC_AssignDrop
	.long	LOPC_AddVarDrop
	.long	LOPC_SubVarDrop
	.long	LOPC_MulVarDrop
	.long	LOPC_DivVarDrop
	.long	LOPC_ModVarDrop
	.long	LOPC_AndVarDrop
	.long	LOPC_OrVarDrop
	.long	LOPC_XOrVarDrop
	.long	LOPC_LShiftVarDrop

	.long	LOPC_RShiftVarDrop
	.long	LOPC_IncDrop
	.long	LOPC_DecDrop
	.long	LOPC_FAdd
	.long	LOPC_FSubtract
	.long	LOPC_FMultiply
	.long	LOPC_FDivide
	.long	LOPC_FEquals
	.long	LOPC_FNotEquals
	.long	LOPC_FLess

	.long	LOPC_FGreater
	.long	LOPC_FLessEquals
	.long	LOPC_FGreaterEquals
	.long	LOPC_FUnaryMinus
	.long	LOPC_FAddVar
	.long	LOPC_FSubVar
	.long	LOPC_FMulVar
	.long	LOPC_FDivVar
	.long	LOPC_FAddVarDrop
	.long	LOPC_FSubVarDrop

	.long	LOPC_FMulVarDrop
	.long	LOPC_FDivVarDrop
	.long	LOPC_Swap
	.long	LOPC_ICall
	.long	LOPC_VPushPointed
	.long	LOPC_VAdd
	.long	LOPC_VSubtract
	.long	LOPC_VPreScale
	.long	LOPC_VPostScale
	.long	LOPC_VIScale

	.long	LOPC_VEquals
	.long	LOPC_VNotEquals
	.long	LOPC_VUnaryMinus
	.long	LOPC_VDrop
	.long	LOPC_VAssign
	.long	LOPC_VAddVar
	.long	LOPC_VSubVar
	.long	LOPC_VScaleVar
	.long	LOPC_VIScaleVar
	.long	LOPC_VAssignDrop

	.long	LOPC_VAddVarDrop
	.long	LOPC_VSubVarDrop
	.long	LOPC_VScaleVarDrop
	.long	LOPC_VIScaleVarDrop
	.long	LOPC_ReturnL
	.long	LOPC_ReturnV
	.long	LOPC_PushString
	.long	LOPC_Copy
	.long	LOPC_Swap3
	.long	LOPC_PushFunction

	.long	LOPC_PushClassId
	.long	LOPC_DynamicCast
	.long	LOPC_CaseGotoClassId
	.long	LOPC_PushName
	.long	LOPC_CaseGotoName
	.long	LOPC_PushBool
	.long	LOPC_AssignBool
	.long	LOPC_PushVFunc

	Align4
LINC_STATEMENT_POINTER:
	addl	$4,%edi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Empty function or invalid opcode
	Align4
LOPC_Done:
	movl	$C(PR_RFInvalidOpcode),%eax
	call	*%eax

    //	Return from void function
	Align4
LOPC_Return:
	movl	%ebp,C(pr_stackPtr)
	jmp		LEND_RUN_FUNCTION

	//	Push number
	Align4
LOPC_PushNumber:
LOPC_GlobalAddress:		// Patched
LOPC_PushString:		// Patched
LOPC_PushFunction:
LOPC_PushClassId:
LOPC_PushName:
	movl	(%edi),%eax
	movl	%eax,(%esi)
	addl	$4,%edi
	addl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Push pointed
	Align4
LOPC_PushPointed:
	movl	-4(%esi),%eax
	movl	(%eax),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Push address of a local variable
	Align4
LOPC_LocalAddress:
	movl	(%edi),%eax
	leal	(%ebp,%eax,4),%eax
	movl	%eax,(%esi)
	addl	$4,%edi
	addl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Add
	Align4
LOPC_Add:
	subl	$4,%esi
	movl	(%esi),%eax
	addl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Subtract
	Align4
LOPC_Subtract:
	subl	$4,%esi
	movl	(%esi),%eax
	subl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Multiply
	Align4
LOPC_Multiply:
	subl	$4,%esi
	movl	-4(%esi),%eax
	imull	(%esi),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Divide
	Align4
LOPC_Divide:
	subl	$4,%esi
	movl	-4(%esi),%eax
	cltd
	idivl	(%esi)
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Modulus
	Align4
LOPC_Modulus:
	subl	$4,%esi
	movl	-4(%esi),%eax
	cltd
	idivl	(%esi)
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Equals
	Align4
LOPC_Equals:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	sete	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Not equals
	Align4
LOPC_NotEquals:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	setne	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Less
	Align4
LOPC_Less:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	setl	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Greater
	Align4
LOPC_Greater:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	setg	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Less or equals
	Align4
LOPC_LessEquals:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	setle	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Greater or equals
	Align4
LOPC_GreaterEquals:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	setge	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Logical AND
	Align4
LOPC_AndLogical:
	subl	$4,%esi
	cmpl	$0,-4(%esi)
	je		LAND_FALSE
	cmpl	$0,(%esi)
	je		LAND_FALSE
	movl	$1,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)
LAND_FALSE:
	movl	$0,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Logical OR
	Align4
LOPC_OrLogical:
	subl	$4,%esi
	cmpl	$0,-4(%esi)
	jne		LOR_TRUE
	cmpl	$0,(%esi)
	jne		LOR_TRUE
	movl	$0,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)
LOR_TRUE:
	movl	$1,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Logical negate
	Align4
LOPC_NegateLogical:
	cmpl	$0,-4(%esi)
	sete	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise AND
	Align4
LOPC_AndBitwise:
	subl	$4,%esi
	movl	(%esi),%eax
	andl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise OR
	Align4
LOPC_OrBitwise:
	subl	$4,%esi
	movl	(%esi),%eax
	orl		%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise XOR
	Align4
LOPC_XOrBitwise:
	subl	$4,%esi
	movl	(%esi),%eax
	xorl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Left shift
	Align4
LOPC_LShift:
	subl	$4,%esi
	movl	(%esi),%ecx
	sall	%cl,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Right shift
	Align4
LOPC_RShift:
	subl	$4,%esi
	movl	(%esi),%ecx
	sarl	%cl,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unary minus
	Align4
LOPC_UnaryMinus:
	negl	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bit inverse
	Align4
LOPC_BitInverse:
	notl	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Call another function
	Align4
LOPC_Call:
	movl	%esi,C(pr_stackPtr)
	pushl	(%edi)
	addl	$4,%edi
	call	C(RunFunction)
	addl	$4,%esp
	movl	OFFS_FUNC(%esp),%eax
	movl	%eax,C(current_func)
	movl	C(pr_stackPtr),%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Goto
	Align4
LOPC_Goto:
	movl	(%edi),%edi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	If goto
	Align4
LOPC_IfGoto:
	subl	$4,%esi
	cmpl	$0,(%esi)
	je		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	If not goto
	Align4
LOPC_IfNotGoto:
	subl	$4,%esi
	cmpl	$0,(%esi)
	jne		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Case goto
	Align4
LOPC_CaseGoto:
LOPC_CaseGotoClassId:
LOPC_CaseGotoName:
	movl	(%edi),%eax
	addl	$4,%edi
	cmpl	-4(%esi),%eax
	jne		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Drop from stack
	Align4
LOPC_Drop:
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Assign
	Align4
LOPC_Assign:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	movl	%eax,(%edx)
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Add to variable
	Align4
LOPC_AddVar:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	addl	%eax,(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Subtract from variable
	Align4
LOPC_SubVar:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	subl	%eax,(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Multiply variable
	Align4
LOPC_MulVar:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	imull	(%edx),%eax
	movl	%eax,(%edx)
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Divide variable
	Align4
LOPC_DivVar:
	subl	$4,%esi
	movl	-4(%esi),%ecx
	movl	(%ecx),%eax
	cltd
	idivl	(%esi)
	movl	%eax,(%ecx)
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Modulus variable
	Align4
LOPC_ModVar:
	subl	$4,%esi
	movl	-4(%esi),%ecx
	movl	(%ecx),%eax
	cltd
	idivl	(%esi)
	movl	%edx,(%ecx)
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise AND variable
	Align4
LOPC_AndVar:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	andl	%eax,(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise OR variable
	Align4
LOPC_OrVar:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	orl		%eax,(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise XOR variable
	Align4
LOPC_XOrVar:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	xorl	%eax,(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Left shift variable
	Align4
LOPC_LShiftVar:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%ecx
	sall	%cl,(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Right shift variable
	Align4
LOPC_RShiftVar:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%ecx
	sarl	%cl,(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Preincrement
	Align4
LOPC_PreInc:
	movl	-4(%esi),%edx
	incl	(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Predecrement
	Align4
LOPC_PreDec:
	movl	-4(%esi),%edx
	decl	(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Postincrement
	Align4
LOPC_PostInc:
	movl	-4(%esi),%edx
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	incl	(%edx)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Postdecrement
	Align4
LOPC_PostDec:
	movl	-4(%esi),%edx
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	decl	(%edx)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	If top goto
	Align4
LOPC_IfTopGoto:
	cmpl	$0,-4(%esi)
	je		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	If not top goto
	Align4
LOPC_IfNotTopGoto:
	cmpl	$0,-4(%esi)
	jne		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Assign, drop result
	Align4
LOPC_AssignDrop:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	movl	%eax,(%edx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Add to variable, drop result
	Align4
LOPC_AddVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	addl	%eax,(%edx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Subtract from variable, drop result
	Align4
LOPC_SubVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%eax
	subl	%eax,(%edx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Multiply variable, drop result
	Align4
LOPC_MulVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%edx),%eax
	imull	(%esi),%eax
	movl	%eax,(%edx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Divide variable, drop result
	Align4
LOPC_DivVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%ecx
	movl	(%ecx),%eax
	cltd
	idivl	(%esi)
	movl	%eax,(%ecx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Modulus variable, drop result
	Align4
LOPC_ModVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%ecx
	movl	(%ecx),%eax
	cltd
	idivl	(%esi)
	movl	%edx,(%ecx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise AND variable, drop result
	Align4
LOPC_AndVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	movl	(%esi),%ecx
	andl	%ecx,(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise OR variable, drop result
	Align4
LOPC_OrVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	movl	(%esi),%ecx
	orl	%ecx,(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise XOR variable, drop result
	Align4
LOPC_XOrVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	movl	(%esi),%ecx
	xorl	%ecx,(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Left shift variable, drop result
	Align4
LOPC_LShiftVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	movl	(%esi),%ecx
	sall	%cl,(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Right shift variable, drop result
	Align4
LOPC_RShiftVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	movl	(%esi),%ecx
	sarl	%cl,(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Increment, drop result
	Align4
LOPC_IncDrop:
	movl	-4(%esi),%edx
	incl	(%edx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Decrement, drop result
	Align4
LOPC_DecDrop:
	movl	-4(%esi),%edx
	decl	(%edx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

//**************************************************************************
	//	Add float
	Align4
LOPC_FAdd:
	subl	$4,%esi
	flds	-4(%esi)
	fadds	(%esi)
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Subtract float
	Align4
LOPC_FSubtract:
	subl	$4,%esi
	flds	-4(%esi)
	fsubs	(%esi)
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Multiply float
	Align4
LOPC_FMultiply:
	subl	$4,%esi
	flds	-4(%esi)
	fmuls	(%esi)
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Divide float
	Align4
LOPC_FDivide:
	subl	$4,%esi
	flds	-4(%esi)
	fdivs	(%esi)
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FEquals:
	subl	$4,%esi
	flds	-4(%esi)
	fcomps	(%esi)
	fnstsw	%ax
	sahf
	sete	%dl
	andl	$1,%edx
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FNotEquals:
	subl	$4,%esi
	flds	-4(%esi)
	fcomps	(%esi)
	fnstsw	%ax
	sahf
	setne	%dl
	andl	$1,%edx
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FLess:
	subl	$4,%esi
	flds	-4(%esi)
	fcomps	(%esi)
	fnstsw	%ax
	sahf
	setb	%dl
	andl	$1,%edx
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FGreater:
	subl	$4,%esi
	flds	-4(%esi)
	fcomps	(%esi)
	fnstsw	%ax
	sahf
	seta	%dl
	andl	$1,%edx
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FLessEquals:
	subl	$4,%esi
	flds	-4(%esi)
	fcomps	(%esi)
	fnstsw	%ax
	sahf
	setbe	%dl
	andl	$1,%edx
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FGreaterEquals:
	subl	$4,%esi
	flds	-4(%esi)
	fcomps	(%esi)
	fnstsw	%ax
	sahf
	setnb	%dl
	andl	$1,%edx
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FUnaryMinus:
	flds	-4(%esi)
	fchs
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FAddVar:
	subl	$4,%esi
	movl	-4(%esi),%eax
	flds	(%eax)
	fadds	(%esi)
	fstps	(%eax)
	movl	(%eax),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FSubVar:
	subl	$4,%esi
	movl	-4(%esi),%eax
	flds	(%eax)
	fsubs	(%esi)
	fstps	(%eax)
	movl	(%eax),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FMulVar:
	subl	$4,%esi
	movl	-4(%esi),%eax
	flds	(%eax)
	fmuls	(%esi)
	fstps	(%eax)
	movl	(%eax),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FDivVar:
	subl	$4,%esi
	movl	-4(%esi),%eax
	flds	(%eax)
	fdivs	(%esi)
	fstps	(%eax)
	movl	(%eax),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FAddVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	flds	(%eax)
	fadds	(%esi)
	fstps	(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FSubVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	flds	(%eax)
	fsubs	(%esi)
	fstps	(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FMulVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	flds	(%eax)
	fmuls	(%esi)
	fstps	(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FDivVarDrop:
	subl	$4,%esi
	movl	-4(%esi),%eax
	flds	(%eax)
	fdivs	(%esi)
	fstps	(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

//**************************************************************************

	//	Swap elements on top of the stack
	Align4
LOPC_Swap:
	movl	-8(%esi),%eax
	movl	-4(%esi),%edx
	movl	%edx,-8(%esi)
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Indirect call another function
	Align4
LOPC_ICall:
	subl	$4,%esi
	pushl	(%esi)
	movl	%esi,C(pr_stackPtr)
	call	C(RunFunction)
	addl	$4,%esp
	movl	OFFS_FUNC(%esp),%eax
	movl	%eax,C(current_func)
	movl	C(pr_stackPtr),%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

//**************************************************************************

	//	Push vector
	Align4
LOPC_VPushPointed:
	addl	$8,%esi
	movl	-12(%esi),%edx
	movl	(%edx),%eax
	movl	%eax,-12(%esi)
	movl	4(%edx),%eax
	movl	%eax,-8(%esi)
	movl	8(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Add vectors
	Align4
LOPC_VAdd:
	flds	-24(%esi)
	fadds	-12(%esi)
	fstps	-24(%esi)
	flds	-20(%esi)
	fadds	-8(%esi)
	fstps	-20(%esi)
	flds	-16(%esi)
	fadds	-4(%esi)
	fstps	-16(%esi)
	addl	$-12,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Subtract vectors
	Align4
LOPC_VSubtract:
	flds	-24(%esi)
	fsubs	-12(%esi)
	fstps	-24(%esi)
	flds	-20(%esi)
	fsubs	-8(%esi)
	fstps	-20(%esi)
	flds	-16(%esi)
	fsubs	-4(%esi)
	fstps	-16(%esi)
	addl	$-12,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Scale vector float*vector
	Align4
LOPC_VPreScale:
	flds	-16(%esi)
	fld		%st(0)
	fld		%st(1)
	fmuls	-12(%esi)
	fstps	-16(%esi)
	fmuls	-8(%esi)
	fstps	-12(%esi)
	fmuls	-4(%esi)
	fstps	-8(%esi)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Scale vector vector*float
	Align4
LOPC_VPostScale:
	flds	-4(%esi)
	fld		%st(0)
	fld		%st(1)
	fmuls	-16(%esi)
	fstps	-16(%esi)
	fmuls	-12(%esi)
	fstps	-12(%esi)
	fmuls	-8(%esi)
	fstps	-8(%esi)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Scale vector vector/float
	Align4
LOPC_VIScale:
	flds	-16(%esi)
	fdivs	-4(%esi)
	fstps	-16(%esi)
	flds	-12(%esi)
	fdivs	-4(%esi)
	fstps	-12(%esi)
	flds	-8(%esi)
	fdivs	-4(%esi)
	fstps	-8(%esi)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Equal vectors
	Align4
LOPC_VEquals:
	flds	-24(%esi)
	flds	-12(%esi)
	fucompp
	fnstsw	%ax
	sahf
	jne		VEQ_FALSE
	flds	-20(%esi)
	flds	-8(%esi)
	fucompp
	fnstsw	%ax
	sahf
	jne		VEQ_FALSE
	flds	-16(%esi)
	flds	-4(%esi)
	fucompp
	fnstsw	%ax
	sahf
	jne		VEQ_FALSE
	movl	$1,-24(%esi)
	addl	$-20,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)
VEQ_FALSE:
	movl	$0,-24(%esi)
	addl	$-20,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Not equal vectors
	Align4
LOPC_VNotEquals:
	flds	-24(%esi)
	flds	-12(%esi)
	fucompp
	fnstsw	%ax
	sahf
	jne		LVNE_TRUE
	flds	-20(%esi)
	flds	-8(%esi)
	fucompp
	fnstsw	%ax
	sahf
	jne		LVNE_TRUE
	flds	-16(%esi)
	flds	-4(%esi)
	fucompp
	fnstsw	%ax
	sahf
	jne		LVNE_TRUE
	movl	$0,-24(%esi)
	addl	$-20,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)
LVNE_TRUE:
	movl	$1,-24(%esi)
	addl	$-20,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Vector unary minus
	Align4
LOPC_VUnaryMinus:
	flds	-12(%esi)
	fchs
	fstps	-12(%esi)
	flds	-8(%esi)
	fchs
	fstps	-8(%esi)
	flds	-4(%esi)
	fchs
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Drop vector
	Align4
LOPC_VDrop:
	addl	$-12,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Assign vector
	Align4
LOPC_VAssign:
	movl	-16(%esi),%edx
	movl	-12(%esi),%eax
	movl	%eax,(%edx)
	movl	%eax,-16(%esi)
	movl	-8(%esi),%eax
	movl	%eax,4(%edx)
	movl	%eax,-12(%esi)
	movl	-4(%esi),%eax
	movl	%eax,8(%edx)
	movl	%eax,-8(%esi)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Add vector to vector
	Align4
LOPC_VAddVar:
	movl	-16(%esi),%edx
	flds	(%edx)
	fadds	-12(%esi)
	fsts	(%edx)
	fstps	-16(%esi)
	flds	4(%edx)
	fadds	-8(%esi)
	fsts	4(%edx)
	fstps	-12(%esi)
	flds	8(%edx)
	fadds	-4(%esi)
	fsts	8(%edx)
	fstps	-8(%esi)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Subtract vector from vector
	Align4
LOPC_VSubVar:
	movl	-16(%esi),%edx
	flds	(%edx)
	fsubs	-12(%esi)
	fsts	(%edx)
	fstps	-16(%esi)
	flds	4(%edx)
	fsubs	-8(%esi)
	fsts	4(%edx)
	fstps	-12(%esi)
	flds	8(%edx)
	fsubs	-4(%esi)
	fsts	8(%edx)
	fstps	-8(%esi)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Scale vector with float
	Align4
LOPC_VScaleVar:
	addl	$4,%esi
	movl	-12(%esi),%edx
	flds	-8(%esi)
	flds	(%edx)
	fmul	%st(1),%st
	fsts	(%edx)
	fstps	-12(%esi)
	flds	4(%edx)
	fmul	%st(1),%st
	fsts	4(%edx)
	fstps	-8(%esi)
	fmuls	8(%edx)
	fsts	8(%edx)
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Divide vector with float
	Align4
LOPC_VIScaleVar:
	addl	$4,%esi
	movl	-12(%esi),%edx
	flds	-8(%esi)
	flds	(%edx)
	fdiv	%st(1),%st
	fsts	(%edx)
	fstps	-12(%esi)
	flds	4(%edx)
	fdiv	%st(1),%st
	fsts	4(%edx)
	fstps	-8(%esi)
	fdivrs	8(%edx)
	fsts	8(%edx)
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Assign vector, drop result
	Align4
LOPC_VAssignDrop:
	movl	-16(%esi),%edx
	movl	-12(%esi),%eax
	movl	%eax,(%edx)
	movl	-8(%esi),%eax
	movl	%eax,4(%edx)
	movl	-4(%esi),%eax
	movl	%eax,8(%edx)
	addl	$-16,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Add vector to vector, drop result
	Align4
LOPC_VAddVarDrop:
	movl	-16(%esi),%edx
	flds	(%edx)
	fadds	-12(%esi)
	fstps	(%edx)
	flds	4(%edx)
	fadds	-8(%esi)
	fstps	4(%edx)
	flds	8(%edx)
	fadds	-4(%esi)
	fstps	8(%edx)
	addl	$-16,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Subtract vector from vector, drop result
	Align4
LOPC_VSubVarDrop:
	movl	-16(%esi),%edx
	flds	(%edx)
	fsubs	-12(%esi)
	fstps	(%edx)
	flds	4(%edx)
	fsubs	-8(%esi)
	fstps	4(%edx)
	flds	8(%edx)
	fsubs	-4(%esi)
	fstps	8(%edx)
	addl	$-16,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Multiply vector with float, drop result
	Align4
LOPC_VScaleVarDrop:
	movl	-8(%esi),%edx
	flds	-4(%esi)
	flds	(%edx)
	fmul	%st(1),%st
	fstps	(%edx)
	flds	4(%edx)
	fmul	%st(1),%st
	fstps	4(%edx)
	fmuls	8(%edx)
	fstps	8(%edx)
	addl	$-8,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Divide vector with float, drop result
	Align4
LOPC_VIScaleVarDrop:
	movl	-8(%esi),%edx
	flds	-4(%esi)
	flds	(%edx)
	fdiv	%st(1),%st
	fstps	(%edx)
	flds	4(%edx)
	fdiv	%st(1),%st
	fstps	4(%edx)
	fdivrs	8(%edx)
	fstps	8(%edx)
	addl	$-8,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

//**************************************************************************

    //	Return from function returning dword sized value
	Align4
LOPC_ReturnL:
	movl	-4(%esi),%eax
	movl	%eax,(%ebp)
	leal	4(%ebp),%esi
	movl	%esi,C(pr_stackPtr)
	jmp		LEND_RUN_FUNCTION

    //	Return from function returning vector
	Align4
LOPC_ReturnV:
	movl	-12(%esi),%eax
	movl	%eax,(%ebp)
	movl	-8(%esi),%eax
	movl	%eax,4(%ebp)
	movl	-4(%esi),%eax
	movl	%eax,8(%ebp)
	leal	12(%ebp),%esi
	movl	%esi,C(pr_stackPtr)
	jmp		LEND_RUN_FUNCTION

//**************************************************************************

	//	Copy top of the stack
	Align4
LOPC_Copy:
	movl	-4(%esi),%eax
	movl	%eax,(%esi)
	addl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Swap element with vector on top of the stack
	Align4
LOPC_Swap3:
	movl	-16(%esi),%eax
	movl	-12(%esi),%edx
	movl	%edx,-16(%esi)
	movl	-8(%esi),%edx
	movl	%edx,-12(%esi)
	movl	-4(%esi),%edx
	movl	%edx,-8(%esi)
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Dynamic cast of a object
	Align4
LOPC_DynamicCast:
	movl	(%edi),%eax
	movl	-4(%esi),%edx
	pushl	%eax
	pushl	%edx
	addl	$4,%edi
	movl	$C(PR_DynamicCast),%eax	// In Windows (Borland and MSVC) calling
	call	*%eax				// a C++ function directly causes segfault
	movl	%eax,-4(%esi)
	addl	$8,%esp
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_PushBool:
	movl	(%edi),%eax
	movl	-4(%esi),%edx
	addl	$4,%edi
	testl	%eax,(%edx)
	setne	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_AssignBool:
	movl	(%edi),%edx
	addl	$-4,%esi
	addl	$4,%edi
	cmpl	$0,(%esi)
	je		LABOOL_FALSE
	movl	-4(%esi),%eax
	orl		%edx,(%eax)
	addl	$-4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)
LABOOL_FALSE:
	movl	-4(%esi),%eax
	xorl	$-1,%edx
	andl	%edx,(%eax)
	addl	$-4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

LOPC_PushVFunc:
	movl	-4(%esi),%edx
	movl	(%edi),%eax
	movl	4(%edx),%edx
	addl	$4,%edi
	movl	(%edx,%eax,4),%eax
	movl	%eax,(%esi)
	addl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

LEND_RUN_FUNCTION:
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.13  2005/11/24 20:06:47  dj_jl
//	Renamed opcodes.
//
//	Revision 1.12  2002/03/16 17:53:12  dj_jl
//	Added opcode for pushing virtual function.
//	
//	Revision 1.11  2002/02/16 16:29:26  dj_jl
//	Added support for bool variables
//	
//	Revision 1.10  2002/01/11 08:07:17  dj_jl
//	Added names to progs
//	
//	Revision 1.9  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.8  2001/12/18 18:54:44  dj_jl
//	Found a workaround for calling a C++ function, progs code patching
//	
//	Revision 1.7  2001/12/12 19:27:46  dj_jl
//	Added dynamic cast
//	
//	Revision 1.6  2001/12/03 19:21:45  dj_jl
//	Added swaping with vector
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
