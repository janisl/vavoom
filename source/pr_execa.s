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
//**	x86 assembly-language PROGS virtual machine code.
//**	
//**************************************************************************

//#define PROGS_PROFILE

#include "asm_i386.h"

#ifdef USEASM

//==========================================================================
//
//  RunFunction
//
//	edi		- Current statement pointer
//	esi		- Stack pointer
//	ebp		- Local vars pointer
//
//==========================================================================

#define OFFS_FUNC_NUM		4+16

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
	movl	OFFS_FUNC_NUM(%esp),%edi
	movl	%edi,C(current_func)
	sall	$4,%edi
	addl	C(pr_functions),%edi
#ifdef PROGS_PROFILE
	call	C(PR_Profile1)
#endif

    //	Check for builtin
	movl	4(%edi),%eax
	testl	%eax,%eax
	jge		LINTERPRET_FUNCTION

    //	Builtin function call
	sall	$2,%eax
	movl	C(pr_builtins),%edx
	subl	%eax,%edx
	movl	(%edx),%eax
	call	*%eax
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
	sall	$2,%edi
	addl	C(pr_statements),%edi
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//
	//	Opcode table
    //
	Align4
LOPCODE_TABLE:
	.long	LOPC_DONE
	.long	LOPC_RETURN
	.long	LOPC_PUSHNUMBER
	.long	LOPC_PUSHPOINTED
	.long	LOPC_LOCALADDRESS
	.long	LOPC_GLOBALADDRESS
	.long	LOPC_ADD
	.long	LOPC_SUBTRACT
	.long	LOPC_MULTIPLY
	.long	LOPC_DIVIDE

	.long	LOPC_MODULUS
	.long	LOPC_UDIVIDE
	.long	LOPC_UMODULUS
	.long	LOPC_EQ
	.long	LOPC_NE
	.long	LOPC_LT
	.long	LOPC_GT
	.long	LOPC_LE
	.long	LOPC_GE
	.long	LOPC_ULT

	.long	LOPC_UGT
	.long	LOPC_ULE
	.long	LOPC_UGE
	.long	LOPC_ANDLOGICAL
	.long	LOPC_ORLOGICAL
	.long	LOPC_NEGATELOGICAL
	.long	LOPC_ANDBITWISE
	.long	LOPC_ORBITWISE
	.long	LOPC_XORBITWISE
	.long	LOPC_LSHIFT

	.long	LOPC_RSHIFT
	.long	LOPC_URSHIFT
	.long	LOPC_UNARYMINUS
	.long	LOPC_BITINVERSE
	.long	LOPC_CALL
	.long	LOPC_GOTO
	.long	LOPC_IFGOTO
	.long	LOPC_IFNOTGOTO
	.long	LOPC_CASEGOTO
	.long	LOPC_DROP

	.long	LOPC_ASSIGN
	.long	LOPC_ADDVAR
	.long	LOPC_SUBVAR
	.long	LOPC_MULVAR
	.long	LOPC_DIVVAR
	.long	LOPC_MODVAR
	.long	LOPC_UDIVVAR
	.long	LOPC_UMODVAR
	.long	LOPC_ANDVAR
	.long	LOPC_ORVAR

	.long	LOPC_XORVAR
	.long	LOPC_LSHIFTVAR
	.long	LOPC_RSHIFTVAR
	.long	LOPC_URSHIFTVAR
	.long	LOPC_PREINC
	.long	LOPC_PREDEC
	.long	LOPC_POSTINC
	.long	LOPC_POSTDEC
	.long	LOPC_IFTOPGOTO
	.long	LOPC_IFNOTTOPGOTO

	.long	LOPC_ASSIGN_DROP
	.long	LOPC_ADDVAR_DROP
	.long	LOPC_SUBVAR_DROP
	.long	LOPC_MULVAR_DROP
	.long	LOPC_DIVVAR_DROP
	.long	LOPC_MODVAR_DROP
	.long	LOPC_UDIVVAR_DROP
	.long	LOPC_UMODVAR_DROP
	.long	LOPC_ANDVAR_DROP
	.long	LOPC_ORVAR_DROP

	.long	LOPC_XORVAR_DROP
	.long	LOPC_LSHIFTVAR_DROP
	.long	LOPC_RSHIFTVAR_DROP
	.long	LOPC_URSHIFTVAR_DROP
	.long	LOPC_INC_DROP
	.long	LOPC_DEC_DROP
	.long	LOPC_FADD
	.long	LOPC_FSUBTRACT
	.long	LOPC_FMULTIPLY
	.long	LOPC_FDIVIDE

	.long	LOPC_FEQ
	.long	LOPC_FNE
	.long	LOPC_FLT
	.long	LOPC_FGT
	.long	LOPC_FLE
	.long	LOPC_FGE
	.long	LOPC_FUNARYMINUS
	.long	LOPC_FADDVAR
	.long	LOPC_FSUBVAR
	.long	LOPC_FMULVAR

	.long	LOPC_FDIVVAR
	.long	LOPC_FADDVAR_DROP
	.long	LOPC_FSUBVAR_DROP
	.long	LOPC_FMULVAR_DROP
	.long	LOPC_FDIVVAR_DROP
	.long	LOPC_SWAP
	.long	LOPC_ICALL
	.long	LOPC_VPUSHPOINTED
	.long	LOPC_VADD
	.long	LOPC_VSUBTRACT

	.long	LOPC_VPRESCALE
	.long	LOPC_VPOSTSCALE
	.long	LOPC_VISCALE
	.long	LOPC_VEQ
	.long	LOPC_VNE
	.long	LOPC_VUNARYMINUS
	.long	LOPC_VDROP
	.long	LOPC_VASSIGN
	.long	LOPC_VADDVAR
	.long	LOPC_VSUBVAR

	.long	LOPC_VSCALEVAR
	.long	LOPC_VISCALEVAR
	.long	LOPC_VASSIGN_DROP
	.long	LOPC_VADDVAR_DROP
	.long	LOPC_VSUBVAR_DROP
	.long	LOPC_VSCALEVAR_DROP
	.long	LOPC_VISCALEVAR_DROP
	.long	LOPC_RETURNL
	.long	LOPC_RETURNV
	.long	LOPC_PUSHSTRING

	Align4
LINC_STATEMENT_POINTER:
	addl	$4,%edi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Empty function or invalid opcode
	Align4
LOPC_DONE:
	call	C(PR_RFInvalidOpcode)

    //	Return from void function
	Align4
LOPC_RETURN:
	movl	%ebp,C(pr_stackPtr)
	jmp		LEND_RUN_FUNCTION

	//	Push number
	Align4
LOPC_PUSHNUMBER:
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
LOPC_PUSHPOINTED:
	movl	-4(%esi),%eax
	movl	(%eax),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Push address of a local variable
	Align4
LOPC_LOCALADDRESS:
	movl	(%edi),%eax
	leal	(%ebp,%eax,4),%eax
	movl	%eax,(%esi)
	addl	$4,%edi
	addl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Push address of a global variable
	Align4
LOPC_GLOBALADDRESS:
	movl	(%edi),%eax
	movl	C(pr_globaldefs),%edx
	movzwl	2(%edx,%eax,8),%eax
	sall	$2,%eax
	addl	C(pr_globals),%eax
	movl	%eax,(%esi)
	addl	$4,%edi
	addl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Add
	Align4
LOPC_ADD:
	subl	$4,%esi
	movl	(%esi),%eax
	addl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Subtract
	Align4
LOPC_SUBTRACT:
	subl	$4,%esi
	movl	(%esi),%eax
	subl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Multiply
	Align4
LOPC_MULTIPLY:
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
LOPC_DIVIDE:
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
LOPC_MODULUS:
	subl	$4,%esi
	movl	-4(%esi),%eax
	cltd
	idivl	(%esi)
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Unsigned divide
	Align4
LOPC_UDIVIDE:
	subl	$4,%esi
	movl	-4(%esi),%eax
	xorl	%edx,%edx
	divl	(%esi)
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unsigned modulus
	Align4
LOPC_UMODULUS:
	subl	$4,%esi
	movl	-4(%esi),%eax
	xorl	%edx,%edx
	divl	(%esi)
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Equals
	Align4
LOPC_EQ:
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
LOPC_NE:
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
LOPC_LT:
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
LOPC_GT:
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
LOPC_LE:
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
LOPC_GE:
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

    //	Unsigned less
	Align4
LOPC_ULT:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	setb	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unsigned greater
	Align4
LOPC_UGT:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	seta	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unsigned less or equals
	Align4
LOPC_ULE:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	setbe	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Unsigned greater or equals
	Align4
LOPC_UGE:
	subl	$4,%esi
	movl	(%esi),%eax
	cmpl	%eax,-4(%esi)
	setae	%al
	andl	$1,%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Logical AND
	Align4
LOPC_ANDLOGICAL:
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
LOPC_ORLOGICAL:
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
LOPC_NEGATELOGICAL:
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
LOPC_ANDBITWISE:
	subl	$4,%esi
	movl	(%esi),%eax
	andl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise OR
	Align4
LOPC_ORBITWISE:
	subl	$4,%esi
	movl	(%esi),%eax
	orl		%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise XOR
	Align4
LOPC_XORBITWISE:
	subl	$4,%esi
	movl	(%esi),%eax
	xorl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Left shift
	Align4
LOPC_LSHIFT:
	subl	$4,%esi
	movl	(%esi),%ecx
	sall	%cl,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Right shift
	Align4
LOPC_RSHIFT:
	subl	$4,%esi
	movl	(%esi),%ecx
	sarl	%cl,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unsigned right shift
	Align4
LOPC_URSHIFT:
	subl	$4,%esi
	movl	(%esi),%ecx
	shrl	%cl,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unary minus
	Align4
LOPC_UNARYMINUS:
	negl	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bit inverse
	Align4
LOPC_BITINVERSE:
	notl	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Call another function
	Align4
LOPC_CALL:
	movl	%esi,C(pr_stackPtr)
	addl	$-12,%esp
	pushl	(%edi)
	addl	$4,%edi
	call	C(RunFunction)
	addl	$16,%esp
	movl	OFFS_FUNC_NUM(%esp),%eax
	movl	%eax,C(current_func)
	movl	C(pr_stackPtr),%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Goto
	Align4
LOPC_GOTO:
	movl	(%edi),%edi
	//	Set and go to the next statement
	sall	$2,%edi
	addl	C(pr_statements),%edi
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	If goto
	Align4
LOPC_IFGOTO:
	subl	$4,%esi
	cmpl	$0,(%esi)
	je		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	//	Set and go to the next statement
	sall	$2,%edi
	addl	C(pr_statements),%edi
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	If not goto
	Align4
LOPC_IFNOTGOTO:
	subl	$4,%esi
	cmpl	$0,(%esi)
	jne		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	//	Set and go to the next statement
	sall	$2,%edi
	addl	C(pr_statements),%edi
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Case goto
	Align4
LOPC_CASEGOTO:
	movl	(%edi),%eax
	addl	$4,%edi
	cmpl	-4(%esi),%eax
	jne		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	subl	$4,%esi
	//	Set and go to the next statement
	sall	$2,%edi
	addl	C(pr_statements),%edi
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Drop from stack
	Align4
LOPC_DROP:
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Assign
	Align4
LOPC_ASSIGN:
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
LOPC_ADDVAR:
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
LOPC_SUBVAR:
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
LOPC_MULVAR:
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
LOPC_DIVVAR:
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
LOPC_MODVAR:
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

    //	Unsigned divide variable
	Align4
LOPC_UDIVVAR:
	subl	$4,%esi
	movl	-4(%esi),%ecx
	movl	(%ecx),%eax
	xorl	%edx,%edx
	divl	(%esi)
	movl	%eax,(%ecx)
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unsigned modulus variable
	Align4
LOPC_UMODVAR:
	subl	$4,%esi
	movl	-4(%esi),%ecx
	movl	(%ecx),%eax
	xorl	%edx,%edx
	divl	(%esi)
	movl	%edx,(%ecx)
	movl	%edx,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise AND variable
	Align4
LOPC_ANDVAR:
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
LOPC_ORVAR:
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
LOPC_XORVAR:
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
LOPC_LSHIFTVAR:
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
LOPC_RSHIFTVAR:
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

    //	Unsigned right shift variable
	Align4
LOPC_URSHIFTVAR:
	subl	$4,%esi
	movl	-4(%esi),%edx
	movl	(%esi),%ecx
	shrl	%cl,(%edx)
	movl	(%edx),%eax
	movl	%eax,-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Preincrement
	Align4
LOPC_PREINC:
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
LOPC_PREDEC:
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
LOPC_POSTINC:
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
LOPC_POSTDEC:
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
LOPC_IFTOPGOTO:
	cmpl	$0,-4(%esi)
	je		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	//	Set and go to the next statement
	sall	$2,%edi
	addl	C(pr_statements),%edi
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	If not top goto
	Align4
LOPC_IFNOTTOPGOTO:
	cmpl	$0,-4(%esi)
	jne		LINC_STATEMENT_POINTER
	movl	(%edi),%edi
	//	Set and go to the next statement
	sall	$2,%edi
	addl	C(pr_statements),%edi
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Assign, drop result
	Align4
LOPC_ASSIGN_DROP:
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
LOPC_ADDVAR_DROP:
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
LOPC_SUBVAR_DROP:
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
LOPC_MULVAR_DROP:
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
LOPC_DIVVAR_DROP:
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
LOPC_MODVAR_DROP:
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

    //	Unsigned divide variable, drop result
	Align4
LOPC_UDIVVAR_DROP:
	subl	$4,%esi
	movl	-4(%esi),%ecx
	movl	(%ecx),%eax
	xorl	%edx,%edx
	divl	(%esi)
	movl	%eax,(%ecx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unsigned modulus variable, drop result
	Align4
LOPC_UMODVAR_DROP:
	subl	$4,%esi
	movl	-4(%esi),%ecx
	movl	(%ecx),%eax
	xorl	%edx,%edx
	divl	(%esi)
	movl	%edx,(%ecx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Bitwise AND variable, drop result
	Align4
LOPC_ANDVAR_DROP:
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
LOPC_ORVAR_DROP:
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
LOPC_XORVAR_DROP:
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
LOPC_LSHIFTVAR_DROP:
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
LOPC_RSHIFTVAR_DROP:
	subl	$4,%esi
	movl	-4(%esi),%eax
	movl	(%esi),%ecx
	sarl	%cl,(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Unsigned right shift variable, drop result
	Align4
LOPC_URSHIFTVAR_DROP:
	subl	$4,%esi
	movl	-4(%esi),%eax
	movl	(%esi),%ecx
	shrl	%cl,(%eax)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Increment, drop result
	Align4
LOPC_INC_DROP:
	movl	-4(%esi),%edx
	incl	(%edx)
	subl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

    //	Decrement, drop result
	Align4
LOPC_DEC_DROP:
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
LOPC_FADD:
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
LOPC_FSUBTRACT:
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
LOPC_FMULTIPLY:
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
LOPC_FDIVIDE:
	subl	$4,%esi
	flds	-4(%esi)
	fdivs	(%esi)
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FEQ:
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
LOPC_FNE:
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
LOPC_FLT:
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
LOPC_FGT:
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
LOPC_FLE:
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
LOPC_FGE:
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
LOPC_FUNARYMINUS:
	flds	-4(%esi)
	fchs
	fstps	-4(%esi)
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	Align4
LOPC_FADDVAR:
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
LOPC_FSUBVAR:
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
LOPC_FMULVAR:
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
LOPC_FDIVVAR:
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
LOPC_FADDVAR_DROP:
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
LOPC_FSUBVAR_DROP:
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
LOPC_FMULVAR_DROP:
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
LOPC_FDIVVAR_DROP:
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
LOPC_SWAP:
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
LOPC_ICALL:
	subl	$4,%esi
	addl	$-12,%esp
	pushl	(%esi)
	movl	%esi,C(pr_stackPtr)
	call	C(RunFunction)
	addl	$16,%esp
	movl	OFFS_FUNC_NUM(%esp),%eax
	movl	%eax,C(current_func)
	movl	C(pr_stackPtr),%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

//**************************************************************************

	//	Push vector
	Align4
LOPC_VPUSHPOINTED:
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
LOPC_VADD:
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
LOPC_VSUBTRACT:
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
LOPC_VPRESCALE:
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
LOPC_VPOSTSCALE:
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
LOPC_VISCALE:
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
LOPC_VEQ:
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
LOPC_VNE:
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
LOPC_VUNARYMINUS:
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
LOPC_VDROP:
	addl	$-12,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

	//	Assign vector
	Align4
LOPC_VASSIGN:
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
LOPC_VADDVAR:
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
LOPC_VSUBVAR:
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
LOPC_VSCALEVAR:
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
LOPC_VISCALEVAR:
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
LOPC_VASSIGN_DROP:
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
LOPC_VADDVAR_DROP:
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
LOPC_VSUBVAR_DROP:
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
LOPC_VSCALEVAR_DROP:
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
LOPC_VISCALEVAR_DROP:
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

	//	Push string
	Align4
LOPC_PUSHSTRING:
	movl	(%edi),%eax
	addl	C(pr_strings),%eax
	movl	%eax,(%esi)
	addl	$4,%edi
	addl	$4,%esi
	//	Go to the next statement
	movl	(%edi),%eax
	addl	$4,%edi
	jmp		*LOPCODE_TABLE(,%eax,4)

//**************************************************************************

    //	Return from function returning dword sized value
	Align4
LOPC_RETURNL:
	movl	-4(%esi),%eax
	movl	%eax,(%ebp)
	leal	4(%ebp),%esi
	movl	%esi,C(pr_stackPtr)
	jmp		LEND_RUN_FUNCTION

    //	Return from function returning vector
	Align4
LOPC_RETURNV:
	movl	-12(%esi),%eax
	movl	%eax,(%ebp)
	movl	-8(%esi),%eax
	movl	%eax,4(%ebp)
	movl	-4(%esi),%eax
	movl	%eax,8(%ebp)
	leal	12(%ebp),%esi
	movl	%esi,C(pr_stackPtr)
	jmp		LEND_RUN_FUNCTION

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
