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

TCvar	*TCvar::Variables = NULL;
bool	TCvar::Initialized = false;
bool	TCvar::Cheating;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  TCvar::TCvar
//
//==========================================================================

TCvar::TCvar(const char *AName, const char *ADefault, int AFlags)
{
	name = AName;
	default_string = ADefault;
	flags = AFlags;
	string = NULL;
	latched_string = NULL;

	TCvar *prev = NULL;
	for (TCvar *var = Variables; var; var = var->next)
	{
		if (stricmp(var->name, name) < 0)
		{
			prev = var;
		}
	}

	if (prev)
	{
		next = prev->next;
		prev->next = this;
	}
	else
	{
		next = Variables;
		Variables = this;
	}

	if (Initialized)
	{
		Register();
	}
}

//==========================================================================
//
//  TCvar::Register
//
//==========================================================================

void TCvar::Register(void)
{
#ifdef CLIENT
    C_AddToAutoComplete(name);
#endif
	DoSet(default_string);
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(int value)
{
	Set(va("%d", value));
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(float value)
{
	Set(va("%f", value));
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(const char *AValue)
{
	if (flags & CVAR_LATCH)
	{
		if (latched_string)
			Z_Free(latched_string);
		latched_string = Z_StrDup(AValue);
		return;
	}

	if (flags & CVAR_CHEAT && !Cheating)
	{
		GCon->Logf("%s cannot be changed while cheating is disabled", name);
		return;
	}

	DoSet(AValue);

	flags |= CVAR_MODIFIED;
}

//==========================================================================
//
//	TCvar::DoSet
//
//	Does the actual value assignement
//
//==========================================================================

void TCvar::DoSet(const char *AValue)
{
	if (string)
	{
		Z_Resize((void**)&string, strlen(AValue) + 1);
	}
	else
	{
	    string = (char*)Z_StrMalloc(strlen(AValue) + 1);
	}
	strcpy(string, AValue);
	value = superatoi(string);
	fvalue = atof(string);

#ifdef CLIENT
	if (flags & CVAR_USERINFO)
	{
		Info_SetValueForKey(cls.userinfo, name, string);
		if (cls.state >= ca_connected)
		{
			cls.message	<< (byte)clc_stringcmd
						<< va("setinfo \"%s\" \"%s\"\n", name, string);
		}
	}
#endif

#ifdef SERVER
	if (flags & CVAR_SERVERINFO)
	{
		Info_SetValueForKey(svs.serverinfo, name, string);
		if (sv.active)
		{
			sv_reliable << (byte)svc_serverinfo << name << string;
		}
	}
#endif
}

//==========================================================================
//
//	TCvar::Init
//
//==========================================================================

void TCvar::Init(void)
{
	for (TCvar *var = Variables; var; var = var->next)
    {
       	var->Register();
    }
	Initialized = true;
}

//==========================================================================
//
//	TCvar::Unlatch
//
//==========================================================================

void TCvar::Unlatch(void)
{
	for (TCvar* cvar = Variables; cvar; cvar = cvar->next)
    {
		if (cvar->latched_string)
		{
			cvar->DoSet(cvar->latched_string);
			Z_Free(cvar->latched_string);
			cvar->latched_string = NULL;
		}
    }
}

//==========================================================================
//
//	TCvar::SetCheating
//
//==========================================================================

void TCvar::SetCheating(bool new_state)
{
	Cheating = new_state;
	if (!Cheating)
	{
		for (TCvar *cvar = Variables; cvar; cvar = cvar->next)
		{
			if (cvar->flags & CVAR_CHEAT)
			{
				cvar->DoSet(cvar->default_string);
			}
		}
	}
}

//==========================================================================
//
//  TCvar::FindVariable
//
//==========================================================================

TCvar *TCvar::FindVariable(const char* name)
{
	TCvar	*cvar;

	for (cvar = Variables; cvar; cvar = cvar->next)
    {
		if (!stricmp(name, cvar->name))
		{
        	return cvar;
		}
    }
	return NULL;
}

//==========================================================================
//
//  TCvar::Value
//
//==========================================================================

int TCvar::Value(const char *var_name)
{
	TCvar	*var;
	
	var = FindVariable(var_name);
	if (!var)
		return 0;
	return var->value;
}

//==========================================================================
//
//  TCvar::Float
//
//==========================================================================

float TCvar::Float(const char *var_name)
{
	TCvar	*var;
	
	var = FindVariable(var_name);
	if (!var)
		return 0;
	return var->fvalue;
}

//==========================================================================
//
//  TCvar::String
//
//==========================================================================

char *TCvar::String(const char *var_name)
{
	TCvar	*var;
	
	var = FindVariable(var_name);
	if (!var)
	{
		return "";
	}
	return var->string;
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(const char *var_name, int value)
{
	TCvar	*var;
	
	var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_Set: variable %s not found\n", var_name);
	}
	var->Set(value);
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(const char *var_name, float value)
{
	TCvar	*var;
	
	var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_Set: variable %s not found\n", var_name);
	}
	var->Set(value);
}

//==========================================================================
//
//  TCvar::Set
//
//==========================================================================

void TCvar::Set(const char *var_name, const char *value)
{
	TCvar	*var;
	
	var = FindVariable(var_name);
	if (!var)
	{
		Sys_Error("Cvar_SetString: variable %s not found\n", var_name);
	}
	var->Set(value);
}

//==========================================================================
//
//	TCvar::Command
//
//==========================================================================

bool TCvar::Command(int argc, const char **argv)
{
    TCvar			*cvar;

    cvar = FindVariable(argv[0]);
    if (cvar)
    {
		// perform a variable print or set
		if (argc == 1)
		{
			GCon->Logf("%s is \"%s\"", cvar->name, cvar->string);
			if (cvar->flags & CVAR_LATCH && cvar->latched_string)
				GCon->Logf("Latched \"%s\"", cvar->latched_string);
		}
        else
        {
			if (cvar->flags & CVAR_ROM)
			{
				GCon->Logf("%s is read-only", cvar->name);
			}
			else if (cvar->flags & CVAR_INIT && host_initialized)
			{
				GCon->Logf("%s can be set only from command-line", cvar->name);
			}
			else
			{
				cvar->Set(argv[1]);
			}
		}
    	return true;
    }
    return false;
}

//==========================================================================
//
//	TCvar::WriteVariables
//
//==========================================================================

void TCvar::WriteVariables(FILE *f)
{
	for (TCvar *cvar = Variables; cvar; cvar = cvar->next)
    {
    	if (cvar->flags & CVAR_ARCHIVE)
        {
        	fprintf(f, "%s\t\t\"%s\"\n", cvar->name, cvar->string);
        }
    }
}

//==========================================================================
//
//	COMMAND CvarList
//
//==========================================================================

COMMAND(CvarList)
{
	int count = 0;
	for (TCvar *cvar = TCvar::Variables; cvar; cvar = cvar->next)
    {
		GCon->Logf("%s - \"%s\"", cvar->name, cvar->string);
		count++;
    }
	GCon->Logf("%d variables.", count);
}

//**************************************************************************
//
//	$Log$
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
