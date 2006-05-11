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

#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	InitInfoTables
//
//==========================================================================

void InitInfoTables()
{
}

//==========================================================================
//
//	FindState
//
//==========================================================================

VState* CheckForState(VName StateName, VClass* InClass)
{
	VMemberBase* m = VMemberBase::StaticFindMember(StateName, InClass,
		MEMBER_State);
	if (m)
	{
		return (VState*)m;
	}
	if (InClass->ParentClass)
	{
		return CheckForState(StateName, InClass->ParentClass);
	}
	return NULL;
}

//==========================================================================
//
//	FindState
//
//==========================================================================

static VState* FindState(VName StateName, VClass* InClass)
{
	VMemberBase* m = VMemberBase::StaticFindMember(StateName, InClass,
		MEMBER_State);
	if (m)
	{
		return (VState*)m;
	}
	if (InClass->ParentClass)
	{
		return FindState(StateName, InClass->ParentClass);
	}
	ParseError("No such state %s", *StateName);
	return NULL;
}

//==========================================================================
//
//	ParseStates
//
//==========================================================================

void ParseStates(VClass* InClass)
{
	if (!InClass && TK_Check(PU_LPAREN))
	{
		InClass = CheckForClass();
		if (!InClass)
		{
			ParseError("Class name expected");
		}
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		//	State identifier
		if (tk_Token != TK_IDENTIFIER)
		{
			ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
		}
		VState* s = new VState(tk_Name, InClass, tk_Location);
		InClass->AddState(s);
		TK_NextToken();
		TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
		//	Sprite name
		if (tk_Token != TK_NAME)
		{
			ParseError(ERR_NONE, "Sprite name expected");
		}
		if (tk_Name != NAME_None && strlen(*tk_Name) != 4)
		{
			ParseError(ERR_NONE, "Invalid sprite name");
		}
		s->SpriteName = tk_Name;
		TK_NextToken();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Frame
		s->frame = EvalConstExpression(InClass, ev_int);
		TK_Expect(PU_COMMA, ERR_NONE);
		if (tk_Token == TK_NAME)
		{
			//	Model
			s->ModelName = tk_Name;
			TK_NextToken();
			TK_Expect(PU_COMMA, ERR_NONE);
			//  Frame
			s->model_frame = EvalConstExpression(InClass, ev_int);
			TK_Expect(PU_COMMA, ERR_NONE);
		}
		else
		{
			s->ModelName = NAME_None;
			s->model_frame = 0;
		}
		//  Tics
		s->time = ConstFloatExpression();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Next state
		if (tk_Token != TK_IDENTIFIER &&
			(tk_Token != TK_KEYWORD || tk_Keyword != KW_NONE))
		{
			ERR_Exit(ERR_NONE, true, NULL);
		}
		TK_NextToken();
		TK_Expect(PU_RPAREN, ERR_NONE);
		//	Code
		s->function = ParseStateCode(InClass, s);
	}
}

//==========================================================================
//
//	AddToMobjInfo
//
//==========================================================================

void AddToMobjInfo(int Index, VClass* Class)
{
	mobjinfo_t& mi = mobj_info.Alloc();
	mi.doomednum = Index;
	mi.class_id = Class;
}

//==========================================================================
//
//	AddToScriptIds
//
//==========================================================================

void AddToScriptIds(int Index, VClass* Class)
{
	mobjinfo_t& mi = script_ids.Alloc();
	mi.doomednum = Index;
	mi.class_id = Class;
}

//==========================================================================
//
//	SkipStates
//
//==========================================================================

void SkipStates(VClass* InClass)
{
	if (!InClass && TK_Check(PU_LPAREN))
	{
		InClass = CheckForClass();
		if (!InClass)
		{
			ParseError("Class name expected");
		}
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		VState* s = FindState(tk_Name, InClass);

		//	State identifier
		if (tk_Token != TK_IDENTIFIER)
		{
			ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
		}
		TK_NextToken();
		TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
		//	Sprite name
		TK_NextToken();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Frame
		EvalConstExpression(InClass, ev_int);
		TK_Expect(PU_COMMA, ERR_NONE);
		if (tk_Token == TK_NAME)
		{
			TK_NextToken();
			TK_Expect(PU_COMMA, ERR_NONE);
			//  Frame
			EvalConstExpression(InClass, ev_int);
			TK_Expect(PU_COMMA, ERR_NONE);
		}
		//  Tics
		ConstFloatExpression();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Next state
		if (tk_Token == TK_KEYWORD && tk_Keyword == KW_NONE)
		{
			s->nextstate = NULL;
		}
		else
		{
			s->nextstate = FindState(tk_Name, InClass);
		}
		TK_NextToken();
		TK_Expect(PU_RPAREN, ERR_NONE);
		//	Code
		CompileStateCode(InClass, s->function);
	}
}
