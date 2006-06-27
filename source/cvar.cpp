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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

VCvar*	VCvar::Variables = NULL;
bool	VCvar::Initialised = false;
bool	VCvar::Cheating;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VCvar::VCvar
//
//==========================================================================

VCvar::VCvar(const char* AName, const char* ADefault, int AFlags)
{
	guard(VCvar::VCvar);
	Name = AName;
	DefaultString = ADefault;
	Flags = AFlags;

	VCvar *prev = NULL;
	for (VCvar *var = Variables; var; var = var->Next)
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
//  VCvar::Register
//
//==========================================================================

void VCvar::Register()
{
	guard(VCvar::Register);
	VCommand::AddToAutoComplete(Name);
	DoSet(DefaultString);
	unguard;
}

//==========================================================================
//
//  VCvar::Set
//
//==========================================================================

void VCvar::Set(int value)
{
	guard(VCvar::Set);
	Set(VStr(value));
	unguard;
}

//==========================================================================
//
//  VCvar::Set
//
//==========================================================================

void VCvar::Set(float value)
{
	guard(VCvar::Set);
	Set(VStr(value));
	unguard;
}

//==========================================================================
//
//  VCvar::Set
//
//==========================================================================

void VCvar::Set(const VStr& AValue)
{
	guard(VCvar::Set);
	if (Flags & CVAR_Latch)
	{
		LatchedString = AValue;
		return;
	}

	if (Flags & CVAR_Cheat && !Cheating)
	{
		GCon->Log(VStr(Name) + " cannot be changed while cheating is disabled");
		return;
	}

	DoSet(AValue);

	Flags |= CVAR_Modified;
	unguard;
}

//==========================================================================
//
//	VCvar::DoSet
//
//	Does the actual value assignement
//
//==========================================================================

void VCvar::DoSet(const VStr& AValue)
{
	guard(VCvar::DoSet);
	StringValue = AValue;
	IntValue = superatoi(*StringValue);
	FloatValue = atof(*StringValue);

#ifdef CLIENT
	if (Flags & CVAR_UserInfo)
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
	if (Flags & CVAR_ServerInfo)
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
//	VCvar::IsModified
//
//==========================================================================

bool VCvar::IsModified()
{
	guard(VCvar::IsModified);
	bool ret = !!(Flags & CVAR_Modified);
	//	Clear modified flag.
	Flags &= ~CVAR_Modified;
	return ret;
	unguard;
}

//==========================================================================
//
//	VCvar::Init
//
//==========================================================================

void VCvar::Init()
{
	guard(VCvar::Init);
	for (VCvar *var = Variables; var; var = var->Next)
	{
		var->Register();
	}
	Initialised = true;
	unguard;
}

//==========================================================================
//
//	VCvar::Shutdown
//
//==========================================================================

void VCvar::Shutdown()
{
	guard(VCvar::Shutdown);
	for (VCvar *var = Variables; var; var = var->Next)
	{
		var->StringValue.Clean();
		var->LatchedString.Clean();
	}
	Initialised = false;
	unguard;
}

//==========================================================================
//
//	VCvar::Unlatch
//
//==========================================================================

void VCvar::Unlatch()
{
	guard(VCvar::Unlatch);
	for (VCvar* cvar = Variables; cvar; cvar = cvar->Next)
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
//	VCvar::SetCheating
//
//==========================================================================

void VCvar::SetCheating(bool new_state)
{
	guard(VCvar::SetCheating);
	Cheating = new_state;
	if (!Cheating)
	{
		for (VCvar *cvar = Variables; cvar; cvar = cvar->Next)
		{
			if (cvar->Flags & CVAR_Cheat)
			{
				cvar->DoSet(cvar->DefaultString);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//  VCvar::FindVariable
//
//==========================================================================

VCvar* VCvar::FindVariable(const char* name)
{
	guard(VCvar::FindVariable);
	for (VCvar* cvar = Variables; cvar; cvar = cvar->Next)
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
//  VCvar::GetInt
//
//==========================================================================

int VCvar::GetInt(const char* var_name)
{
	guard(VCvar::GetInt);
	VCvar* var = FindVariable(var_name);
	if (!var)
		return 0;
	return var->IntValue;
	unguard;
}

//==========================================================================
//
//  VCvar::GetFloat
//
//==========================================================================

float VCvar::GetFloat(const char* var_name)
{
	guard(VCvar::GetFloat);
	VCvar* var = FindVariable(var_name);
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

const char* VCvar::GetCharp(const char* var_name)
{
	guard(VCvar::GetCharp);
	VCvar* var = FindVariable(var_name);
	if (!var)
	{
		return "";
	}
	return *var->StringValue;
	unguard;
}

//==========================================================================
//
//  VCvar::GetString
//
//==========================================================================

VStr VCvar::GetString(const char* var_name)
{
	guard(VCvar::GetString);
	VCvar* var = FindVariable(var_name);
	if (!var)
	{
		return VStr();
	}
	return var->StringValue;
	unguard;
}

//==========================================================================
//
//  VCvar::Set
//
//==========================================================================

void VCvar::Set(const char* var_name, int value)
{
	guard(VCvar::Set);
	VCvar* var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_Set: variable %s not found\n", var_name);
	}
	var->Set(value);
	unguard;
}

//==========================================================================
//
//  VCvar::Set
//
//==========================================================================

void VCvar::Set(const char* var_name, float value)
{
	guard(VCvar::Set);
	VCvar* var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_Set: variable %s not found\n", var_name);
	}
	var->Set(value);
	unguard;
}

//==========================================================================
//
//  VCvar::Set
//
//==========================================================================

void VCvar::Set(const char* var_name, const VStr& value)
{
	guard(VCvar::Set);
	VCvar* var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_SetString: variable %s not found\n", var_name);
	}
	var->Set(value);
	unguard;
}

//==========================================================================
//
//	VCvar::Command
//
//==========================================================================

bool VCvar::Command(const TArray<VStr>& Args)
{
	guard(VCvar::Command);
	VCvar* cvar = FindVariable(*Args[0]);
	if (!cvar)
	{
		return false;
	}

	// perform a variable print or set
	if (Args.Num() == 1)
	{
		GCon->Log(VStr(cvar->Name) + " is \"" + cvar->StringValue + "\"");
		if (cvar->Flags & CVAR_Latch && cvar->LatchedString)
			GCon->Log(VStr("Latched \"") + cvar->LatchedString + "\"");
	}
	else
	{
		if (cvar->Flags & CVAR_Rom)
		{
			GCon->Logf("%s is read-only", cvar->Name);
		}
		else if (cvar->Flags & CVAR_Init && host_initialized)
		{
			GCon->Logf("%s can be set only from command-line", cvar->Name);
		}
		else
		{
			cvar->Set(Args[1]);
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VCvar::WriteVariables
//
//==========================================================================

void VCvar::WriteVariables(FILE* f)
{
	guard(VCvar::WriteVariables);
	for (VCvar* cvar = Variables; cvar; cvar = cvar->Next)
	{
		if (cvar->Flags & CVAR_Archive)
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
	for (VCvar *cvar = VCvar::Variables; cvar; cvar = cvar->Next)
	{
		GCon->Log(VStr(cvar->Name) + " - \"" + cvar->StringValue + "\"");
		count++;
	}
	GCon->Logf("%d variables.", count);
	unguard;
}
