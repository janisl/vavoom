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

class VParser
{
private:
	VLexer&			Lex;
	VPackage*		Package;
	bool			CheckForLocal;

	VExpression* ParseDotMethodCall(VExpression*, VName, TLocation);
	VExpression* ParseBaseMethodCall(VName, TLocation);
	VExpression* ParseMethodCallOrCast(VName, TLocation);
	VLocalDecl* ParseLocalVar(VExpression* TypeExpr);
	VExpression* ParseExpressionPriority0();
	VExpression* ParseExpressionPriority1();
	VExpression* ParseExpressionPriority2();
	VExpression* ParseExpressionPriority3();
	VExpression* ParseExpressionPriority4();
	VExpression* ParseExpressionPriority5();
	VExpression* ParseExpressionPriority6();
	VExpression* ParseExpressionPriority7();
	VExpression* ParseExpressionPriority8();
	VExpression* ParseExpressionPriority9();
	VExpression* ParseExpressionPriority10();
	VExpression* ParseExpressionPriority11();
	VExpression* ParseExpressionPriority12();
	VExpression* ParseExpressionPriority13();
	VExpression* ParseExpressionPriority14();
	VExpression* ParseExpression();
	VStatement* ParseStatement();
	VCompound* ParseCompoundStatement();
	VExpression* ParseType();
	void ParseMethodDef(VExpression*, VName, TLocation, VClass*, vint32, bool);
	void ParseDelegate(VExpression*, VField*);
	void ParseDefaultProperties(VClass*);
	void ParseStruct(VClass*, bool);
	VName ParseStateString();
	void ParseStates(VClass*);
	void ParseReplication(VClass*);
	void ParseClass();

public:
	VParser(VLexer& ALex, VPackage* APackage)
	: Lex(ALex)
	, Package(APackage)
	{}
	void Parse();
};
