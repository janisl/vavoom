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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define CVAR_ARCHIVE		1	//	Set to cause it to be saved to vavoom.cfg
#define	CVAR_USERINFO		2	//	Added to userinfo  when changed
#define	CVAR_SERVERINFO		4	//	Added to serverinfo when changed
#define	CVAR_INIT			8	//	Don't allow change from console at all,
								// but can be set from the command line
#define	CVAR_LATCH			16	//	Save changes until server restart
#define	CVAR_ROM			32	//	Display only, cannot be set by user at all
#define CVAR_CHEAT			64	//	Can not be changed if cheats are disabled
#define CVAR_MODIFIED		128	//	Set each time the cvar is changed

// TYPES -------------------------------------------------------------------

class TCvar
{
 public:
	TCvar(const char *AName, const char *ADefault, int AFlags = 0);
	void Init(void);
	void Set(int value);
	void Set(float value);
	void Set(const char *value);

	const char	*name;				//	Variable's name
	const char	*default_string;	//	Default value
	char		*string;			//	Current value
	int			flags;				//	CVAR_ flags
	int			value;				//	atoi(string)
	float		fvalue;				//	atof(string)
	TCvar		*next;				//	For linked list if variables
	char		*latched_string;	//	For CVAR_LATCH variables

 private:
	void DoSet(const char *value);

	friend void Cvar_Unlatch(void);
	friend void Cvar_SetCheating(bool);
};

//	Cvar, that can be used as int variable
class TCvarI : public TCvar
{
 public:
	TCvarI(const char *AName, const char *ADefault, int AFlags = 0)
		: TCvar(AName, ADefault, AFlags)
	{
	}

	operator int(void) const
	{
		return value;
	}

	TCvarI &operator = (int AValue)
	{
		Set(AValue);
		return *this;
	}
};

//	Cvar, that can be used as float variable
class TCvarF : public TCvar
{
 public:
	TCvarF(const char *AName, const char *ADefault, int AFlags = 0)
		: TCvar(AName, ADefault, AFlags)
	{
	}

	operator float(void) const
	{
		return fvalue;
	}

	TCvarF &operator = (float AValue)
	{
		Set(AValue);
		return *this;
	}
};

//	Cvar, that can be used as char* variable
class TCvarS : public TCvar
{
 public:
	TCvarS(const char *AName, const char *ADefault, int AFlags = 0)
		: TCvar(AName, ADefault, AFlags)
	{
	}

	operator char*(void) const
	{
		return string;
	}

	TCvarS &operator = (const char *AValue)
	{
		Set(AValue);
		return *this;
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void Cvar_Init(void);

int Cvar_Value(const char *var_name);
float Cvar_Float(const char *var_name);
char *Cvar_String(const char *var_name);

void Cvar_Set(const char *var_name, int value);
void Cvar_Set(const char *var_name, float value);
void Cvar_Set(const char *var_name, char *value);

boolean Cvar_Command(int argc, char **argv);
void Cvar_Write(ostream &strm);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

inline ostream &operator << (ostream &strm, const TCvarI &var)
{
	return strm << (int)var;
}

inline ostream &operator << (ostream &strm, const TCvarF &var)
{
	return strm << (float)var;
}

inline ostream &operator << (ostream &strm, const TCvarS &var)
{
	return strm << (char*)var;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2001/10/04 17:18:23  dj_jl
//	Implemented the rest of cvar flags
//
//	Revision 1.5  2001/09/05 12:21:42  dj_jl
//	Release changes
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
