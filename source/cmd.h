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

// MACROS ------------------------------------------------------------------

#define COMMAND(name) \
static class TCmd ## name : public TCommand \
{ \
 public: \
	TCmd ## name(void) : TCommand(#name) { } \
    void Run(void); \
} name ## _f; \
\
void TCmd ## name::Run(void)

// TYPES -------------------------------------------------------------------

enum cmd_source_t
{
	src_command,
	src_client
};

class TCmdBuf : public TTextBuf
{
 public:
	void Init(void);
	void Insert(const char*);
	void Exec(void);

 private:
	TTextBuf			ParsedCmd;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void Cmd_Init(void);
void Cmd_WriteAlias(ostream &strm);

void Cmd_TokenizeString(const char *str);
int Cmd_Argc(void);
char *Cmd_Argv(int parm);
char *Cmd_Args(void);
int Cmd_CheckParm(const char *check);

void Cmd_ExecuteString(const char *cmd, cmd_source_t src);
void Cmd_ForwardToServer(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TCmdBuf		CmdBuf;
extern cmd_source_t	cmd_source;

class TCommand
{
 public:
	TCommand(const char *name);

	virtual void Run(void) = 0;

	int Argc(void)
	{
		return Cmd_Argc();
	}
	char *Argv(int parm)
	{
		return Cmd_Argv(parm);
	}
	char *Args(void)
	{
		return Cmd_Args();
	}
	int CheckParm(const char *check)
	{
		return Cmd_CheckParm(check);
	}

	const char	*Name;
	TCommand	*Next;
};

//**************************************************************************
//
//	$Log$
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
