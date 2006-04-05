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

//
//	A console command.
//
class VCommand
{
public:
	enum ECmdSource
	{
		SRC_Command,
		SRC_Client
	};

private:
	//	Console command alias.
	struct VAlias
	{
		VStr		Name;
		VStr		CmdLine;
		VAlias*		Next;
	};

	const char*					Name;
	VCommand*					Next;

	static bool					Initialised;
	static VStr					Original;

	static TArray<const char*>	AutoCompleteTable;

	static VCommand*			Cmds;
	static VAlias*				Alias;

	static void TokeniseString(const VStr&);

protected:
	static TArray<VStr>			Args;
	static ECmdSource			Source;

public:
	VCommand(const char*);
	virtual ~VCommand();

	virtual void Run() = 0;

	static void Init();
	static void WriteAlias(FILE*);

	static void AddToAutoComplete(const char*);
	static VStr GetAutoComplete(const VStr&, int&, bool);

	static void ExecuteString(const VStr&, ECmdSource);
	static void ForwardToServer();
	static int CheckParm(const char*);

	friend class VCmdBuf;
	friend class TCmdCmdList;
	friend class TCmdAlias;
};

//
//	Macro for declaring a console command.
//
#define COMMAND(name) \
static class TCmd ## name : public VCommand \
{ \
public: \
	TCmd ## name() : VCommand(#name) { } \
	void Run(); \
} name ## _f; \
\
void TCmd ## name::Run()

//
//	A command buffer.
//
class VCmdBuf
{
private:
	VStr		Buffer;
	bool		Wait;

public:
	void Insert(const char*);
	void Insert(const VStr&);
	void Print(const char*);
	void Print(const VStr&);
	void Exec();

	VCmdBuf& operator << (const char* data)
	{
		Print(data);
		return *this;
	}

	VCmdBuf& operator << (const VStr& data)
	{
		Print(data);
		return *this;
	}

	friend class TCmdWait;
};

//	Main command buffer.
extern VCmdBuf		GCmdBuf;

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
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
