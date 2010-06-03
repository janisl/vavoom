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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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

#include "vc_local.h"

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
//	VBaseInvocation::VBaseInvocation
//
//==========================================================================

VBaseInvocation::VBaseInvocation(VName AName, int ANumArgs, VExpression** AArgs,
	const TLocation& ALoc)
: VExpression(ALoc)
, Name(AName)
, NumArgs(ANumArgs)
{
	guard(VBaseInvocation::VBaseInvocation);
	for (int i = 0; i < NumArgs; i++)
	{
		Args[i] = AArgs[i];
	}
	unguard;
}

//==========================================================================
//
//	VBaseInvocation::~VBaseInvocation
//
//==========================================================================

VBaseInvocation::~VBaseInvocation()
{
	guard(VBaseInvocation::~VBaseInvocation);
	for (int i = 0; i < NumArgs; i++)
	{
		delete Args[i];
		Args[i] = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VBaseInvocation::DoResolve
//
//==========================================================================

VExpression* VBaseInvocation::DoResolve(VEmitContext& ec)
{
	guard(VBaseInvocation::DoResolve);
	if (!ec.SelfClass)
	{
		ParseError(Loc, ":: not in method");
		delete this;
		return NULL;
	}
	VMethod* Func = ec.SelfClass->ParentClass->FindMethod(Name);
	if (!Func)
	{
		ParseError(Loc, "No such method %s", *Name);
		delete this;
		return NULL;
	}

	VExpression* e = new VInvocation(NULL, Func, NULL, false,
		true, Loc, NumArgs, Args);
	NumArgs = 0;
	delete this;
	return e->Resolve(ec);
	unguard;
}

//==========================================================================
//
//	VBaseInvocation::Emit
//
//==========================================================================

void VBaseInvocation::Emit(VEmitContext&)
{
	guard(VBaseInvocation::Emit);
	ParseError(Loc, "Should not happen");
	unguard;
}

//==========================================================================
//
//	VCastOrInvocation::VCastOrInvocation
//
//==========================================================================

VCastOrInvocation::VCastOrInvocation(VName AName, const TLocation& ALoc, int ANumArgs,
	VExpression** AArgs)
: VExpression(ALoc)
, Name(AName)
, NumArgs(ANumArgs)
{
	for (int i = 0; i < NumArgs; i++)
		Args[i] = AArgs[i];
}

//==========================================================================
//
//	VCastOrInvocation::~VCastOrInvocation
//
//==========================================================================

VCastOrInvocation::~VCastOrInvocation()
{
	for (int i = 0; i < NumArgs; i++)
	{
		if (Args[i])
		{
			delete Args[i];
			Args[i] = NULL;
		}
	}
}

//==========================================================================
//
//	VCastOrInvocation::DoResolve
//
//==========================================================================

VExpression* VCastOrInvocation::DoResolve(VEmitContext& ec)
{
	VClass* Class = VMemberBase::StaticFindClass(Name);
	if (Class)
	{
		if (NumArgs != 1 || !Args[0])
		{
			ParseError(Loc, "Dynamic cast requires 1 argument");
			delete this;
			return NULL;
		}
		VExpression* e = new VDynamicCast(Class, Args[0], Loc);
		NumArgs = 0;
		delete this;
		return e->Resolve(ec);
	}

	if (ec.SelfClass)
	{
		VMethod* M = ec.SelfClass->FindMethod(Name);
		if (M)
		{
			if (M->Flags & FUNC_Iterator)
			{
				ParseError(Loc, "Iterator methods can only be used in foreach statements");
				delete this;
				return NULL;
			}
			VExpression* e = new VInvocation(NULL, M, NULL,
				false, false, Loc, NumArgs, Args);
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}
		VField* field = ec.SelfClass->FindField(Name, Loc, ec.SelfClass);
		if (field && field->Type.Type == TYPE_Delegate)
		{
			VExpression* e = new VInvocation(NULL, field->Func, field,
				false, false, Loc, NumArgs, Args);
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}
	}

	ParseError(Loc, "Unknown method %s", *Name);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VCastOrInvocation::ResolveIterator
//
//==========================================================================

VExpression* VCastOrInvocation::ResolveIterator(VEmitContext& ec)
{
	VMethod* M = ec.SelfClass->FindMethod(Name);
	if (!M)
	{
		ParseError(Loc, "Unknown method %s", *Name);
		delete this;
		return NULL;
	}
	if (!(M->Flags & FUNC_Iterator))
	{
		ParseError(Loc, "%s is not an iterator method", *Name);
		delete this;
		return NULL;
	}

	VExpression* e = new VInvocation(NULL, M, NULL, false, false, Loc,
		NumArgs, Args);
	NumArgs = 0;
	delete this;
	return e->Resolve(ec);
}

//==========================================================================
//
//	VCastOrInvocation::Emit
//
//==========================================================================

void VCastOrInvocation::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//==========================================================================
//
//	VDotInvocation::VDotInvocation
//
//==========================================================================

VDotInvocation::VDotInvocation(VExpression* ASelfExpr, VName AMethodName,
	const TLocation& ALoc, int ANumArgs, VExpression** AArgs)
: VExpression(ALoc)
, SelfExpr(ASelfExpr)
, MethodName(AMethodName)
, NumArgs(ANumArgs)
{
	for (int i = 0; i < NumArgs; i++)
		Args[i] = AArgs[i];
}

//==========================================================================
//
//	VDotInvocation::~VDotInvocation
//
//==========================================================================

VDotInvocation::~VDotInvocation()
{
	if (SelfExpr)
	{
		delete SelfExpr;
		SelfExpr = NULL;
	}
	for (int i = 0; i < NumArgs; i++)
	{
		delete Args[i];
		Args[i] = NULL;
	}
}

//==========================================================================
//
//	VDotInvocation::DoResolve
//
//==========================================================================

VExpression* VDotInvocation::DoResolve(VEmitContext& ec)
{
	if (SelfExpr)
		SelfExpr = SelfExpr->Resolve(ec);
	if (!SelfExpr)
	{
		delete this;
		return NULL;
	}

	if (SelfExpr->Type.Type == TYPE_DynamicArray)
	{
		if (MethodName == NAME_Insert)
		{
			if (NumArgs == 1)
			{
				//	Default count is 1
				Args[1] = new VIntLiteral(1, Loc);
				NumArgs = 2;
			}
			if (NumArgs != 2)
			{
				ParseError(Loc, "Insert requires 1 or 2 arguments");
				delete this;
				return NULL;
			}
			VExpression* e = new VDynArrayInsert(SelfExpr, Args[0], Args[1], Loc);
			SelfExpr = NULL;
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}

		if (MethodName == NAME_Remove)
		{
			if (NumArgs == 1)
			{
				//	Default count is 1
				Args[1] = new VIntLiteral(1, Loc);
				NumArgs = 2;
			}
			if (NumArgs != 2)
			{
				ParseError(Loc, "Insert requires 1 or 2 arguments");
				delete this;
				return NULL;
			}
			VExpression* e = new VDynArrayRemove(SelfExpr, Args[0], Args[1], Loc);
			SelfExpr = NULL;
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}

		ParseError(Loc, "Invalid operation on dynamic array");
		delete this;
		return NULL;
	}

	if (SelfExpr->Type.Type != TYPE_Reference)
	{
		ParseError(Loc, "Object reference expected left side of .");
		delete this;
		return NULL;
	}

	VMethod* M = SelfExpr->Type.Class->FindMethod(MethodName);
	if (M)
	{
		if (M->Flags & FUNC_Iterator)
		{
			ParseError(Loc, "Iterator methods can only be used in foreach statements");
			delete this;
			return NULL;
		}
		VExpression* e = new VInvocation(SelfExpr, M, NULL, true,
			false, Loc, NumArgs, Args);
		SelfExpr = NULL;
		NumArgs = 0;
		delete this;
		return e->Resolve(ec);
	}

	VField* field = SelfExpr->Type.Class->FindField(MethodName, Loc,
		ec.SelfClass);
	if (field && field->Type.Type == TYPE_Delegate)
	{
		VExpression* e = new VInvocation(SelfExpr, field->Func, field, true,
			false, Loc, NumArgs, Args);
		SelfExpr = NULL;
		NumArgs = 0;
		delete this;
		return e->Resolve(ec);
	}

	ParseError(Loc, "No such method %s", *MethodName);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VDotInvocation::ResolveIterator
//
//==========================================================================

VExpression* VDotInvocation::ResolveIterator(VEmitContext& ec)
{
	if (SelfExpr)
		SelfExpr = SelfExpr->Resolve(ec);
	if (!SelfExpr)
	{
		delete this;
		return NULL;
	}

	if (SelfExpr->Type.Type != TYPE_Reference)
	{
		ParseError(Loc, "Object reference expected left side of .");
		delete this;
		return NULL;
	}

	VMethod* M = SelfExpr->Type.Class->FindMethod(MethodName);
	if (!M)
	{
		ParseError(Loc, "No such method %s", *MethodName);
		delete this;
		return NULL;
	}
	if (!(M->Flags & FUNC_Iterator))
	{
		ParseError(Loc, "%s is not an iterator method", *MethodName);
		delete this;
		return NULL;
	}
	
	VExpression* e = new VInvocation(SelfExpr, M, NULL, true,
		false, Loc, NumArgs, Args);
	SelfExpr = NULL;
	NumArgs = 0;
	delete this;
	return e->Resolve(ec);
}

//==========================================================================
//
//	VDotInvocation::Emit
//
//==========================================================================

void VDotInvocation::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//==========================================================================
//
//	VInvocation::VInvocation
//
//==========================================================================

VInvocation::VInvocation(VExpression* ASelfExpr, VMethod* AFunc, VField* ADelegateField,
	bool AHaveSelf, bool ABaseCall, const TLocation& ALoc, int ANumArgs,
	VExpression** AArgs)
: VExpression(ALoc)
, SelfExpr(ASelfExpr)
, Func(AFunc)
, DelegateField(ADelegateField)
, HaveSelf(AHaveSelf)
, BaseCall(ABaseCall)
, NumArgs(ANumArgs)
, CallerState(NULL)
, MultiFrameState(false)
{
	guard(VInvocation::VInvocation);
	for (int i = 0; i < NumArgs; i++)
	{
		Args[i] = AArgs[i];
	}
	unguard;
}

//==========================================================================
//
//	VInvocation::~VInvocation
//
//==========================================================================

VInvocation::~VInvocation()
{
	guard(VInvocation::~VInvocation);
	if (SelfExpr)
	{
		delete SelfExpr;
		SelfExpr = NULL;
	}
	for (int i = 0; i < NumArgs; i++)
	{
		delete Args[i];
		Args[i] = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VInvocation::DoResolve
//
//==========================================================================

VExpression* VInvocation::DoResolve(VEmitContext& ec)
{
	guard(VInvocation::DoResolve);
	if (ec.Package->Name == NAME_decorate)
	{
		CheckDecorateParams(ec);
	}

	//	Resolve arguments
	bool ArgsOk = true;
	for (int i = 0; i < NumArgs; i++)
	{
		if (Args[i])
		{
			Args[i] = Args[i]->Resolve(ec);
			if (!Args[i])
			{
				ArgsOk = false;
			}
		}
	}
	if (!ArgsOk)
	{
		delete this;
		return NULL;
	}

	CheckParams(ec);

	Type  = Func->ReturnType;
	if (Type.Type == TYPE_Byte || Type.Type == TYPE_Bool)
		Type = VFieldType(TYPE_Int);
	if (Func->Flags & FUNC_Spawner)
		Type.Class = Args[0]->Type.Class;
	return this;
	unguard;
}

//==========================================================================
//
//	VInvocation::Emit
//
//==========================================================================

void VInvocation::Emit(VEmitContext& ec)
{
	guard(VInvocation::Emit);
	if (SelfExpr)
	{
		SelfExpr->Emit(ec);
	}

	bool DirectCall = BaseCall || (Func->Flags & FUNC_Final);

	if (Func->Flags & FUNC_Static)
	{
		if (HaveSelf)
		{
			ParseError(Loc, "Invalid static function call");
		}
	}
	else
	{
		if (!HaveSelf)
		{
			if (ec.CurrentFunc->Flags & FUNC_Static)
			{
				ParseError(Loc, "An object is required to call non-static methods");
			}
			ec.AddStatement(OPC_LocalValue0);
		}
	}

	vint32 SelfOffset = 1;
	for (int i = 0; i < NumArgs; i++)
	{
		if (!Args[i])
		{
			switch (Func->ParamTypes[i].Type)
			{
			case TYPE_Int:
			case TYPE_Byte:
			case TYPE_Bool:
			case TYPE_Float:
			case TYPE_Name:
				ec.EmitPushNumber(0);
				SelfOffset++;
				break;

			case TYPE_String:
			case TYPE_Pointer:
			case TYPE_Reference:
			case TYPE_Class:
			case TYPE_State:
				ec.AddStatement(OPC_PushNull);
				SelfOffset++;
				break;

			case TYPE_Vector:
				ec.EmitPushNumber(0);
				ec.EmitPushNumber(0);
				ec.EmitPushNumber(0);
				SelfOffset += 3;
				break;

			default:
				ParseError(Loc, "Bad optional parameter type");
				break;
			}
			ec.EmitPushNumber(0);
			SelfOffset++;
		}
		else
		{
			Args[i]->Emit(ec);
			if (Args[i]->Type.Type == TYPE_Vector)
				SelfOffset += 3;
			else
				SelfOffset++;
			if (Func->ParamFlags[i] & FPARM_Optional)
			{
				ec.EmitPushNumber(1);
				SelfOffset++;
			}
		}
	}

	if (DirectCall)
	{
		ec.AddStatement(OPC_Call, Func);
	}
	else if (DelegateField)
	{
		ec.AddStatement(OPC_DelegateCall, DelegateField, SelfOffset);
	}
	else
	{
		ec.AddStatement(OPC_VCall, Func, SelfOffset);
	}
	unguard;
}

//==========================================================================
//
//	VInvocation::CheckParams
//
//==========================================================================

void VInvocation::CheckParams(VEmitContext& ec)
{
	guard(VInvocation::CheckParams);
	//	Determine parameter count.
	int argsize = 0;
	int max_params;
	int num_needed_params = Func->NumParams;
	if (Func->Flags & FUNC_VarArgs)
	{
		max_params = VMethod::MAX_PARAMS - 1;
	}
	else
	{
		max_params = Func->NumParams;
	}

	for (int i = 0; i < NumArgs; i++)
	{
		if (i < num_needed_params)
		{
			if (!Args[i])
			{
				if (!(Func->ParamFlags[i] & FPARM_Optional))
				{
					ParseError(Loc, "Bad expresion");
				}
				argsize += Func->ParamTypes[i].GetStackSize();
			}
			else
			{
				if (ec.Package->Name == NAME_decorate)
				{
					switch (Func->ParamTypes[i].Type)
					{
					case TYPE_Int:
						if (Args[i]->IsFloatConst())
						{
							int Val = int(Args[i]->GetFloatConst());
							TLocation Loc = Args[i]->Loc;
							delete Args[i];
							Args[i] = NULL;
							Args[i] = new VIntLiteral(Val, Loc);
							Args[i] = Args[i]->Resolve(ec);
						}
						else if (Args[i]->Type.Type == TYPE_Float)
						{
							VExpression* TmpArgs[1];
							TmpArgs[0] = Args[i];
							Args[i] = new VInvocation(NULL,
								ec.SelfClass->FindMethodChecked("ftoi"), NULL,
								false, false, Args[i]->Loc, 1, TmpArgs);
							Args[i] = Args[i]->Resolve(ec);
						}
						break;

					case TYPE_Float:
						if (Args[i]->IsIntConst())
						{
							int Val = Args[i]->GetIntConst();
							TLocation Loc = Args[i]->Loc;
							delete Args[i];
							Args[i] = NULL;
							Args[i] = new VFloatLiteral(Val, Loc);
							Args[i] = Args[i]->Resolve(ec);
						}
						else if (Args[i]->Type.Type == TYPE_Int)
						{
							VExpression* TmpArgs[1];
							TmpArgs[0] = Args[i];
							Args[i] = new VInvocation(NULL,
								ec.SelfClass->FindMethodChecked("itof"), NULL,
								false, false, Args[i]->Loc, 1, TmpArgs);
							Args[i] = Args[i]->Resolve(ec);
						}
						break;
					}
				}
				if (Func->ParamFlags[i] & FPARM_Out)
				{
					if (!Args[i]->Type.Equals(Func->ParamTypes[i]))
					{
						//FIXME This should be error.
						Args[i]->Type.CheckMatch(Args[i]->Loc, Func->ParamTypes[i]);
						//ParseError(Args[i]->Loc, "Out parameter types must be equal");
					}
					Args[i]->RequestAddressOf();
				}
				else
				{
					Args[i]->Type.CheckMatch(Args[i]->Loc, Func->ParamTypes[i]);
				}
				argsize += Args[i]->Type.GetStackSize();
			}
		}
		else if (!Args[i])
		{
			ParseError(Loc, "Bad expresion");
		}
		else
		{
			argsize += Args[i]->Type.GetStackSize();
		}
	}
	if (NumArgs > max_params)
	{
		ParseError(Loc, "Incorrect number of arguments, need %d, got %d.", max_params, NumArgs);
	}
	while (NumArgs < num_needed_params)
	{
		if (Func->ParamFlags[NumArgs] & FPARM_Optional)
		{
			Args[NumArgs] = NULL;
			NumArgs++;
		}
		else
		{
			ParseError(Loc, "Incorrect argument count %d, should be %d",
				NumArgs, num_needed_params);
			break;
		}
	}

	if (Func->Flags & FUNC_VarArgs)
	{
		Args[NumArgs++] = new VIntLiteral(argsize / 4 - num_needed_params, Loc);
	}
	unguard;
}

//==========================================================================
//
//	VInvocation::CheckDecorateParams
//
//==========================================================================

void VInvocation::CheckDecorateParams(VEmitContext& ec)
{
	guard(VInvocation::CheckDecorateParams);
	int max_params;
	int num_needed_params = Func->NumParams;
	if (Func->Flags & FUNC_VarArgs)
	{
		max_params = VMethod::MAX_PARAMS - 1;
	}
	else
	{
		max_params = Func->NumParams;
	}

	for (int i = 0; i < NumArgs; i++)
	{
		if (i >= num_needed_params)
		{
			continue;
		}
		if (!Args[i])
		{
			continue;
		}
		switch (Func->ParamTypes[i].Type)
		{
		case TYPE_Name:
			if (Args[i]->IsDecorateSingleName())
			{
				VDecorateSingleName* E = (VDecorateSingleName*)Args[i];
				Args[i] = new VNameLiteral(*E->Name, E->Loc);
				delete E;
				E = NULL;
			}
			else if (Args[i]->IsStrConst())
			{
				VStr Val = Args[i]->GetStrConst(ec.Package);
				TLocation ALoc = Args[i]->Loc;
				delete Args[i];
				Args[i] = NULL;
				Args[i] = new VNameLiteral(*Val, ALoc);
			}
			break;

		case TYPE_String:
			if (Args[i]->IsDecorateSingleName())
			{
				VDecorateSingleName* E = (VDecorateSingleName*)Args[i];
				Args[i] = new VStringLiteral(ec.Package->FindString(*E->Name), E->Loc);
				delete E;
				E = NULL;
			}
			break;

		case TYPE_Class:
			if (Args[i]->IsDecorateSingleName())
			{
				VDecorateSingleName* E = (VDecorateSingleName*)Args[i];
				Args[i] = new VStringLiteral(ec.Package->FindString(*E->Name), E->Loc);
				delete E;
				E = NULL;
			}
			if (Args[i]->IsStrConst())
			{
				VStr CName = Args[i]->GetStrConst(ec.Package);
				TLocation ALoc = Args[i]->Loc;
				VClass* Cls = VClass::FindClassNoCase(*CName);
				if (!Cls)
				{
					ParseWarning(ALoc, "No such class %s", *CName);
					delete Args[i];
					Args[i] = NULL;
					Args[i] = new VNoneLiteral(ALoc);
				}
				else if (Func->ParamTypes[i].Class &&
					!Cls->IsChildOf(Func->ParamTypes[i].Class))
				{
					ParseWarning(ALoc, "Class %s is not a descendant of %s",
						*CName, Func->ParamTypes[i].Class->GetName());
					delete Args[i];
					Args[i] = NULL;
					Args[i] = new VNoneLiteral(ALoc);
				}
				else
				{
					delete Args[i];
					Args[i] = NULL;
					Args[i] = new VClassConstant(Cls, ALoc);
				}
			}
			break;

		case TYPE_State:
			if (Args[i]->IsIntConst())
			{
				int Offs = Args[i]->GetIntConst();
				TLocation ALoc = Args[i]->Loc;
				if (Offs < 0)
				{
					ParseError(ALoc, "Negative state jumps are not allowed");
				}
				else if (Offs == 0)
				{
					//	0 means no state
					delete Args[i];
					Args[i] = NULL;
					Args[i] = new VNoneLiteral(ALoc);
				}
				else
				{
					check(CallerState);
					VState* S = CallerState->GetPlus(Offs, true);
					if (!S)
					{
						ParseError(ALoc, "Bad state jump offset");
					}
					else
					{
						delete Args[i];
						Args[i] = NULL;
						Args[i] = new VStateConstant(S, ALoc);
					}
				}
			}
			else if (Args[i]->IsStrConst())
			{
				VStr Lbl = Args[i]->GetStrConst(ec.Package);
				TLocation ALoc = Args[i]->Loc;
				int DCol = Lbl.IndexOf("::");
				if (DCol >= 0)
				{
					//	Jump to a specific parent class state, resolve it and
					// pass value directly.
					VStr ClassName(Lbl, 0, DCol);
					VClass* CheckClass;
					if (ClassName.ICmp("Super"))
					{
						CheckClass = ec.SelfClass->ParentClass;
					}
					else
					{
						CheckClass = VClass::FindClassNoCase(*ClassName);
						if (!CheckClass)
						{
							ParseError(ALoc, "No such class %s", *ClassName);
						}
						else if (!ec.SelfClass->IsChildOf(CheckClass))
						{
							ParseError(ALoc, "%s is not a subclass of %s",
								ec.SelfClass->GetName(), CheckClass->GetName());
							CheckClass = NULL;
						}
					}
					if (CheckClass)
					{
						VStr LblName(Lbl, DCol + 2, Lbl.Length() - DCol - 2);
						TArray<VName> Names;
						VMemberBase::StaticSplitStateLabel(LblName, Names);
						VStateLabel* StLbl = CheckClass->FindStateLabel(Names, true);
						if (!StLbl)
						{
							ParseError(ALoc, "No such state %s", *Lbl);
						}
						else
						{
							delete Args[i];
							Args[i] = NULL;
							Args[i] = new VStateConstant(StLbl->State, ALoc);
						}
					}
				}
				else
				{
					//	It's a virtual state jump
					VExpression* TmpArgs[1];
					TmpArgs[0] = Args[i];
					Args[i] = new VInvocation(NULL,
						ec.SelfClass->FindMethodChecked("FindJumpState"),
						NULL, false, false, Args[i]->Loc, 1, TmpArgs);
				}
			}
			break;
		}
	}
	unguard;
}
