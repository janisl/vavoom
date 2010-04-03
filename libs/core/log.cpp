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
//**	Copyright (C) 1999-2010 Jānis Legzdiņš
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

#include "core.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VLog			GLog;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLog::VLog
//
//==========================================================================

VLog::VLog()
{
	memset(Listeners, 0, sizeof(Listeners));
}

//==========================================================================
//
//	VLog::AddListener
//
//==========================================================================

void VLog::AddListener(VLogListener* Listener)
{
	for (int i = 0; i < MAX_LISTENERS; i++)
	{
		if (!Listeners[i])
		{
			Listeners[i] = Listener;
			return;
		}
	}
}

//==========================================================================
//
//	VLog::RemoveListener
//
//==========================================================================

void VLog::RemoveListener(VLogListener* Listener)
{
	for (int i = 0; i < MAX_LISTENERS; i++)
	{
		if (Listeners[i] == Listener)
		{
			Listeners[i] = NULL;
		}
	}
}

//==========================================================================
//
//	VLog::Write
//
//==========================================================================

void VLog::Write(EName Type, const char* Fmt, ...)
{
	va_list		ArgPtr;
	char		String[1024];
	
	va_start(ArgPtr, Fmt);
	vsprintf(String, Fmt, ArgPtr);
	va_end(ArgPtr);

	for (int i = 0; i < MAX_LISTENERS; i++)
	{
		if (Listeners[i])
		{
			try
			{
				Listeners[i]->Serialise(String, Type);
			}
			catch (...)
			{
			}
		}
	}
}

//==========================================================================
//
//	VLog::WriteLine
//
//==========================================================================

void VLog::WriteLine(EName Type, const char* Fmt, ...)
{
	va_list		ArgPtr;
	char		String[1024];
	
	va_start(ArgPtr, Fmt);
	vsprintf(String, Fmt, ArgPtr);
	va_end(ArgPtr);
	strcat(String, "\n");

	for (int i = 0; i < MAX_LISTENERS; i++)
	{
		if (Listeners[i])
		{
			try
			{
				Listeners[i]->Serialise(String, Type);
			}
			catch (...)
			{
			}
		}
	}
}

//==========================================================================
//
//	VLog::Write
//
//==========================================================================

void VLog::Write(const char* Fmt, ...)
{
	va_list		ArgPtr;
	char		String[1024];
	
	va_start(ArgPtr, Fmt);
	vsprintf(String, Fmt, ArgPtr);
	va_end(ArgPtr);

	for (int i = 0; i < MAX_LISTENERS; i++)
	{
		if (Listeners[i])
		{
			try
			{
				Listeners[i]->Serialise(String, NAME_Log);
			}
			catch (...)
			{
			}
		}
	}
}

//==========================================================================
//
//	VLog::WriteLine
//
//==========================================================================

void VLog::WriteLine(const char* Fmt, ...)
{
	va_list		ArgPtr;
	char		String[1024];
	
	va_start(ArgPtr, Fmt);
	vsprintf(String, Fmt, ArgPtr);
	va_end(ArgPtr);
	strcat(String, "\n");

	for (int i = 0; i < MAX_LISTENERS; i++)
	{
		if (Listeners[i])
		{
			try
			{
				Listeners[i]->Serialise(String, NAME_Log);
			}
			catch (...)
			{
			}
		}
	}
}

//==========================================================================
//
//	VLog::DWrite
//
//==========================================================================

void VLog::DWrite(const char* Fmt, ...)
{
	va_list		ArgPtr;
	char		String[1024];
	
	va_start(ArgPtr, Fmt);
	vsprintf(String, Fmt, ArgPtr);
	va_end(ArgPtr);

	for (int i = 0; i < MAX_LISTENERS; i++)
	{
		if (Listeners[i])
		{
			try
			{
				Listeners[i]->Serialise(String, NAME_Dev);
			}
			catch (...)
			{
			}
		}
	}
}

//==========================================================================
//
//	VLog::DWriteLine
//
//==========================================================================

void VLog::DWriteLine(const char* Fmt, ...)
{
	va_list		ArgPtr;
	char		String[1024];
	
	va_start(ArgPtr, Fmt);
	vsprintf(String, Fmt, ArgPtr);
	va_end(ArgPtr);
	strcat(String, "\n");

	for (int i = 0; i < MAX_LISTENERS; i++)
	{
		if (Listeners[i])
		{
			try
			{
				Listeners[i]->Serialise(String, NAME_Dev);
			}
			catch (...)
			{
			}
		}
	}
}
