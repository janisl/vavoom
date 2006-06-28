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

enum
{
	CVAR_Archive	= 0x0001,	//	Set to cause it to be saved to config.cfg
	CVAR_UserInfo	= 0x0002,	//	Added to userinfo  when changed
	CVAR_ServerInfo	= 0x0004,	//	Added to serverinfo when changed
	CVAR_Init		= 0x0008,	//	Don't allow change from console at all,
								// but can be set from the command line
	CVAR_Latch		= 0x0010,	//	Save changes until server restart
	CVAR_Rom		= 0x0020,	//	Display only, cannot be set by user at all
	CVAR_Cheat		= 0x0040,	//	Can not be changed if cheats are disabled
	CVAR_Modified	= 0x0080,	//	Set each time the cvar is changed

	CVAR_Delete		= 0x8000,	//	Delete variable on exit.
};

//
//	Console variable
//
class VCvar
{
protected:
	const char*	Name;				//	Variable's name
	const char*	DefaultString;		//	Default value
	VStr		StringValue;		//	Current value
	int			Flags;				//	CVAR_ flags
	int			IntValue;			//	atoi(string)
	float		FloatValue;			//	atof(string)
	VCvar*		Next;				//	For linked list if variables
	VStr		LatchedString;		//	For CVAR_Latch variables

public:
	VCvar(const char* AName, const char* ADefault, int AFlags = 0);
	VCvar(const char* AName, const VStr& ADefault, int AFlags = 0);
	void Register();
	void Set(int value);
	void Set(float value);
	void Set(const VStr& value);
	bool IsModified();

	static void Init();
	static void Shutdown();

	static int GetInt(const char* var_name);
	static float GetFloat(const char* var_name);
	static const char* GetCharp(const char* var_name);
	static VStr GetString(const char* var_name);

	static void Set(const char* var_name, int value);
	static void Set(const char* var_name, float value);
	static void Set(const char* var_name, const VStr& value);

	static bool Command(const TArray<VStr>& Args);
	static void WriteVariables(FILE* f);

	static void Unlatch();
	static void SetCheating(bool);

	friend class TCmdCvarList;

private:
	void DoSet(const VStr& value);

	static VCvar*	Variables;
	static bool		Initialised;
	static bool		Cheating;

	static VCvar* FindVariable(const char* name);
};

//	Cvar, that can be used as int variable
class VCvarI : public VCvar
{
public:
	VCvarI(const char* AName, const char* ADefault, int AFlags = 0)
		: VCvar(AName, ADefault, AFlags)
	{
	}

	operator int() const
	{
		return IntValue;
	}

	VCvarI &operator = (int AValue)
	{
		Set(AValue);
		return *this;
	}
};

//	Cvar, that can be used as float variable
class VCvarF : public VCvar
{
public:
	VCvarF(const char* AName, const char* ADefault, int AFlags = 0)
		: VCvar(AName, ADefault, AFlags)
	{
	}

	operator float() const
	{
		return FloatValue;
	}

	VCvarF &operator = (float AValue)
	{
		Set(AValue);
		return *this;
	}
};

//	Cvar, that can be used as char* variable
class VCvarS : public VCvar
{
public:
	VCvarS(const char* AName, const char* ADefault, int AFlags = 0)
		: VCvar(AName, ADefault, AFlags)
	{
	}

	operator const char*() const
	{
		return *StringValue;
	}

	VCvarS &operator = (const char* AValue)
	{
		Set(AValue);
		return *this;
	}
};
