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
	static VBasePlayer*			Player;	//	For SRC_Client

public:
	static bool					ParsingKeyConf;

	VCommand(const char*);
	virtual ~VCommand();

	virtual void Run() = 0;

	static void Init();
	static void WriteAlias(FILE*);
	static void Shutdown();

	static void AddToAutoComplete(const char*);
	static VStr GetAutoComplete(const VStr&, int&, bool);

	static void ExecuteString(const VStr&, ECmdSource, VBasePlayer*);
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
