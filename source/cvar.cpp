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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

#ifdef CLIENT
void C_AddToAutoComplete(const char* string);
#endif

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

TCvar*	TCvar::Variables = NULL;
bool	TCvar::Initialised = false;
bool	TCvar::Cheating;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  TCvar::TCvar
//
//==========================================================================

TCvar::TCvar(const char* AName, const char* ADefault, int AFlags)
{
	guard(TCvar::TCvar);
	Name = AName;
	DefaultString = ADefault;
	Flags = AFlags;

	TCvar *prev = NULL;
	for (TCvar *var = Variables; var; var = var->Next)
	{
		if (stricmp(var->Name, Name) < 0)
		{
			prev = var;
		}
	}

	if (prev)
	{
		Next = prev->Next;
		prev->Next = this;
	}
	else
	{
		Next = Variables;
		Variables = this;
	}

	if (Initialised)
	{
		Register();
	}
	unguard;
}

//==========================================================================
//
//  TCvar::Register
//
//==========================================================================

void TCvar::Register()
{
	guard(TCvar::Register);
#ifdef CLIENT
	C_AddToAutoComplete(Name);
#endif
	DoSet(DefaultString);
	unguard;
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(int value)
{
	guard(TCvar::Set);
	Set(VStr(value));
	unguard;
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(float value)
{
	guard(TCvar::Set);
	Set(VStr(value));
	unguard;
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(const VStr& AValue)
{
	guard(TCvar::Set);
	if (Flags & CVAR_LATCH)
	{
		LatchedString = AValue;
		return;
	}

	if (Flags & CVAR_CHEAT && !Cheating)
	{
		GCon->Log(VStr(Name) + " cannot be changed while cheating is disabled");
		return;
	}

	DoSet(AValue);

	Flags |= CVAR_MODIFIED;
	unguard;
}

//==========================================================================
//
//	TCvar::DoSet
//
//	Does the actual value assignement
//
//==========================================================================

void TCvar::DoSet(const VStr& AValue)
{
	guard(TCvar::DoSet);
	StringValue = AValue;
	IntValue = superatoi(*StringValue);
	FloatValue = atof(*StringValue);

#ifdef CLIENT
	if (Flags & CVAR_USERINFO)
	{
		Info_SetValueForKey(cls.userinfo, Name, *StringValue);
		if (cls.state >= ca_connected)
		{
			cls.message	<< (byte)clc_stringcmd
						<< (VStr("setinfo \"") + Name + "\" \"" +
							StringValue + "\"\n");
		}
	}
#endif

#ifdef SERVER
	if (Flags & CVAR_SERVERINFO)
	{
		Info_SetValueForKey(svs.serverinfo, Name, *StringValue);
		if (sv.active)
		{
			sv_reliable << (byte)svc_serverinfo << Name << StringValue;
		}
	}
#endif
	unguard;
}

//==========================================================================
//
//	TCvar::IsModified
//
//==========================================================================

bool TCvar::IsModified()
{
	guard(TCvar::IsModified);
	bool ret = !!(Flags & CVAR_MODIFIED);
	//	Clear modified flag.
	Flags &= ~CVAR_MODIFIED;
	return ret;
	unguard;
}

//==========================================================================
//
//	TCvar::Init
//
//==========================================================================

void TCvar::Init()
{
	guard(TCvar::Init);
	for (TCvar *var = Variables; var; var = var->Next)
	{
		var->Register();
	}
	Initialised = true;
	unguard;
}

//==========================================================================
//
//	TCvar::Unlatch
//
//==========================================================================

void TCvar::Unlatch()
{
	guard(TCvar::Unlatch);
	for (TCvar* cvar = Variables; cvar; cvar = cvar->Next)
	{
		if (cvar->LatchedString)
		{
			cvar->DoSet(cvar->LatchedString);
			cvar->LatchedString.Clean();
		}
	}
	unguard;
}

//==========================================================================
//
//	TCvar::SetCheating
//
//==========================================================================

void TCvar::SetCheating(bool new_state)
{
	guard(TCvar::SetCheating);
	Cheating = new_state;
	if (!Cheating)
	{
		for (TCvar *cvar = Variables; cvar; cvar = cvar->Next)
		{
			if (cvar->Flags & CVAR_CHEAT)
			{
				cvar->DoSet(cvar->DefaultString);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//  TCvar::FindVariable
//
//==========================================================================

TCvar* TCvar::FindVariable(const char* name)
{
	guard(TCvar::FindVariable);
	for (TCvar* cvar = Variables; cvar; cvar = cvar->Next)
	{
		if (!stricmp(name, cvar->Name))
		{
			return cvar;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//  TCvar::GetInt
//
//==========================================================================

int TCvar::GetInt(const char* var_name)
{
	guard(TCvar::GetInt);
	TCvar* var = FindVariable(var_name);
	if (!var)
		return 0;
	return var->IntValue;
	unguard;
}

//==========================================================================
//
//  TCvar::GetFloat
//
//==========================================================================

float TCvar::GetFloat(const char* var_name)
{
	guard(TCvar::GetFloat);
	TCvar* var = FindVariable(var_name);
	if (!var)
		return 0;
	return var->FloatValue;
	unguard;
}

//==========================================================================
//
//  GetCharp
//
//==========================================================================

const char* TCvar::GetCharp(const char* var_name)
{
	guard(TCvar::GetCharp);
	TCvar* var = FindVariable(var_name);
	if (!var)
	{
		return "";
	}
	return *var->StringValue;
	unguard;
}

//==========================================================================
//
//  TCvar::GetString
//
//==========================================================================

VStr TCvar::GetString(const char* var_name)
{
	guard(TCvar::GetString);
	TCvar* var = FindVariable(var_name);
	if (!var)
	{
		return VStr();
	}
	return var->StringValue;
	unguard;
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(const char* var_name, int value)
{
	guard(TCvar::Set);
	TCvar* var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_Set: variable %s not found\n", var_name);
	}
	var->Set(value);
	unguard;
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(const char* var_name, float value)
{
	guard(TCvar::Set);
	TCvar* var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_Set: variable %s not found\n", var_name);
	}
	var->Set(value);
	unguard;
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(const char* var_name, const VStr& value)
{
	guard(TCvar::Set);
	TCvar* var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_SetString: variable %s not found\n", var_name);
	}
	var->Set(value);
	unguard;
}

//==========================================================================
//
//	TCvar::Command
//
//==========================================================================

bool TCvar::Command(int argc, const char** argv)
{
	guard(TCvar::Command);
	TCvar* cvar = FindVariable(argv[0]);
	if (!cvar)
	{
		return false;
	}

	// perform a variable print or set
	if (argc == 1)
	{
		GCon->Log(VStr(cvar->Name) + " is \"" + cvar->StringValue + "\"");
		if (cvar->Flags & CVAR_LATCH && cvar->LatchedString)
			GCon->Log(VStr("Latched \"") + cvar->LatchedString + "\"");
	}
	else
	{
		if (cvar->Flags & CVAR_ROM)
		{
			GCon->Logf("%s is read-only", cvar->Name);
		}
		else if (cvar->Flags & CVAR_INIT && host_initialized)
		{
			GCon->Logf("%s can be set only from command-line", cvar->Name);
		}
		else
		{
			cvar->Set(argv[1]);
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	TCvar::WriteVariables
//
//==========================================================================

void TCvar::WriteVariables(FILE* f)
{
	guard(TCvar::WriteVariables);
	for (TCvar* cvar = Variables; cvar; cvar = cvar->Next)
	{
		if (cvar->Flags & CVAR_ARCHIVE)
		{
			fprintf(f, "%s\t\t\"%s\"\n", cvar->Name, *cvar->StringValue);
		}
	}
	unguard;
}

//==========================================================================
//
//	COMMAND CvarList
//
//==========================================================================

COMMAND(CvarList)
{
	guard(COMMAND CvarList);
	int count = 0;
	for (TCvar *cvar = TCvar::Variables; cvar; cvar = cvar->Next)
	{
		GCon->Log(VStr(cvar->Name) + " - \"" + cvar->StringValue + "\"");
		count++;
	}
	GCon->Logf("%d variables.", count);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2006/03/29 22:32:27  dj_jl
//	Changed console variables and command buffer to use dynamic strings.
//
//	Revision 1.9  2003/09/24 16:41:59  dj_jl
//	Fixed cvar constructor
//	
//	Revision 1.8  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.7  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/12/18 19:05:03  dj_jl
//	Made TCvar a pure C++ class
//	
//	Revision 1.5  2001/10/04 17:18:23  dj_jl
//	Implemented the rest of cvar flags
//	
//	Revision 1.4  2001/08/29 17:50:42  dj_jl
//	Implemented CVAR_LATCH
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
