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
//**	Copyright (C) 1999-2002 J�nis Legzdi��
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

#define COMMAND(name) \
static class TCmd ## name : public TCommand \
{ \
public: \
	TCmd ## name() : TCommand(#name) { } \
	void Run(); \
} name ## _f; \
\
void TCmd ## name::Run()

// TYPES -------------------------------------------------------------------

enum cmd_source_t
{
	src_command,
	src_client
};

//
//	A command buffer.
//
class TCmdBuf
{
private:
	VStr		Buffer;

public:
	void Init();
	void Insert(const char*);
	void Insert(const VStr&);
	void Print(const char*);
	void Print(const VStr&);
	void Exec();

	TCmdBuf& operator << (const char* data)
	{
		Print(data);
		return *this;
	}

	TCmdBuf& operator << (const VStr& data)
	{
		Print(data);
		return *this;
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void Cmd_Init();
void Cmd_WriteAlias(FILE *f);

void Cmd_TokeniseString(const char *str);
int Cmd_Argc();
char **Cmd_Argv();
char *Cmd_Argv(int parm);
char *Cmd_Args();
int Cmd_CheckParm(const char *check);

void Cmd_ExecuteString(const char *cmd, cmd_source_t src);
void Cmd_ForwardToServer();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TCmdBuf		CmdBuf;
extern cmd_source_t	cmd_source;

//
//	A console command.
//
class TCommand
{
public:
	TCommand(const char *name);
	virtual ~TCommand();

	virtual void Run() = 0;

	int Argc()
	{
		return Cmd_Argc();
	}
	char *Argv(int parm)
	{
		return Cmd_Argv(parm);
	}
	char *Args()
	{
		return Cmd_Args();
	}
	int CheckParm(const char *check)
	{
		return Cmd_CheckParm(check);
	}

	const char*	Name;
	TCommand*	Next;
};

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2006/03/29 22:32:27  dj_jl
//	Changed console variables and command buffer to use dynamic strings.
//
//	Revision 1.8  2005/04/28 07:16:11  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.7  2003/10/22 06:24:35  dj_jl
//	Access to the arguments vector
//	
//	Revision 1.6  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.5  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/10/04 17:20:25  dj_jl
//	Saving config using streams
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
