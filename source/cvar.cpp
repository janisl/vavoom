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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:53  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

/*
#define	CVAR_INIT			8	//	Don't allow change from console at all,
								// but can be set from the command line
#define	CVAR_LATCH			16	//	Save changes until server restart
#define	CVAR_ROM			32	//	Display only, cannot be set by user at all
#define CVAR_CHEAT			64	//	Can not be changed if cheats are disabled
#define CVAR_MODIFIED		128	//	Set each time the cvar is changed
*/

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

static TCvar	*cvars = NULL;
static char		*cvar_null_string = "";
static bool		cvar_initialized = false;

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

	TCvar *prev = NULL;
	for (TCvar *var = cvars; var; var = var->next)
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
		next = cvars;
		cvars = this;
	}

	if (cvar_initialized)
	{
		Init();
	}
}

//==========================================================================
//
//  TCvar::Init
//
//==========================================================================

void TCvar::Init(void)
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
	DoSet(AValue);
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
//	COMMAND Vars
//
//==========================================================================

COMMAND(Vars)
{
	int count = 0;
	for (TCvar *cvar = cvars; cvar; cvar = cvar->next)
    {
		con << cvar->name << " - \"" << cvar->string << "\"\n";
		count++;
    }
	con << count << " variables.\n";
}

//==========================================================================
//
//	Cvar_Init
//
//==========================================================================

void Cvar_Init(void)
{
	for (TCvar *var = cvars; var; var = var->next)
    {
       	var->Init();
    }
	cvar_initialized = true;
}

//==========================================================================
//
//  FindVar
//
//==========================================================================

static TCvar *FindVar(const char* name)
{
	TCvar	*cvar;

	for (cvar = cvars; cvar; cvar = cvar->next)
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
//  Cvar_Value
//
//==========================================================================

int Cvar_Value(const char *var_name)
{
	TCvar	*var;
	
	var = FindVar(var_name);
	if (!var)
		return 0;
	return var->value;
}

//==========================================================================
//
//  Cvar_Float
//
//==========================================================================

float Cvar_Float(const char *var_name)
{
	TCvar	*var;
	
	var = FindVar(var_name);
	if (!var)
		return 0;
	return var->fvalue;
}

//==========================================================================
//
//  Cvar_String
//
//==========================================================================

char *Cvar_String(const char *var_name)
{
	TCvar	*var;
	
	var = FindVar(var_name);
	if (!var)
		return cvar_null_string;
	return var->string;
}

//==========================================================================
//
//  Cvar_Set
//
//==========================================================================

void Cvar_Set(const char *var_name, int value)
{
	TCvar	*var;
	
	var = FindVar(var_name);
	if (!var)
	{
		Sys_Error("Cvar_Set: variable %s not found\n", var_name);
	}
	var->Set(value);
}

//==========================================================================
//
//  Cvar_Set
//
//==========================================================================

void Cvar_Set(const char *var_name, float value)
{
	TCvar	*var;
	
	var = FindVar(var_name);
	if (!var)
	{
		Sys_Error("Cvar_Set: variable %s not found\n", var_name);
	}
	var->Set(value);
}

//==========================================================================
//
//  Cvar_Set
//
//==========================================================================

void Cvar_Set(const char *var_name, /*const*/ char *value)
{
	TCvar	*var;
	
	var = FindVar(var_name);
	if (!var)
	{
		Sys_Error("Cvar_SetString: variable %s not found\n", var_name);
	}
	var->Set(value);
}

//==========================================================================
//
//	Cvar_Command
//
//==========================================================================

boolean Cvar_Command(int argc, char **argv)
{
    TCvar			*cvar;

    cvar = FindVar(argv[0]);
    if (cvar)
    {
		// perform a variable print or set
		if (argc == 1)
		{
			con << cvar->name << " is \"" << cvar->string << "\"\n";
			if (cvar->flags & CVAR_LATCH && cvar->latched_string)
				con << "Latched \"" << cvar->latched_string << "\"\n";
		}
        else
        {
			cvar->Set(argv[1]);
		}
    	return true;
    }
    return false;
}

//==========================================================================
//
//	Cvar_Write
//
//==========================================================================

void Cvar_Write(FILE *f)
{
	TCvar*	cvar;

	for (cvar = cvars; cvar; cvar = cvar->next)
    {
    	if (cvar->flags & CVAR_ARCHIVE)
        {
        	fprintf(f, "%s\t\t\"%s\"\n", cvar->name, cvar->string);
        }
    }
}

