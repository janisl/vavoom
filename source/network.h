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
	MAX_MSGLEN			= 8000,		// max length of a reliable message
	MAX_DATAGRAM		= 1024,		// max length of unreliable message

	NET_MAXMESSAGE		= 8192
};

//
//	VSocketPublic
//
//	Public interface of a network socket.
//
class VSocketPublic : public VVirtualObjectBase
{
public:
	VStr			Address;

	virtual bool IsLocalConnection() = 0;
	virtual int GetMessage(VMessageIn*&) = 0;
	virtual int SendMessage(VMessageOut*) = 0;
	virtual int SendUnreliableMessage(VMessageOut*) = 0;
	virtual bool CanSendMessage() = 0;
	virtual void Close() = 0;
};

//
//	hostcache_t
//
//	An entry into a hosts cache table.
//
struct hostcache_t
{
	VStr		Name;
	VStr		Map;
	VStr		CName;
	VStr		WadFiles[20];
	vint32		Users;
	vint32		MaxUsers;
};

//
//	slist_t
//
//	Structure returned to progs.
//
struct slist_t
{
	enum
	{
		SF_InProgress	= 0x01,
	};
	vuint32			Flags;
	vint32			Count;
	hostcache_t*	Cache;
	VStr			ReturnReason;
};

//
//	VNetworkPublic
//
//	Public networking driver interface.
//
class VNetworkPublic : public VVirtualObjectBase
{
public:
	//	Public API
	bool			ConnectBot;

	VNetworkPublic()
	: ConnectBot(false)
	{}

	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual VSocketPublic* Connect(const char*) = 0;
	virtual VSocketPublic* CheckNewConnections() = 0;
	virtual void Poll() = 0;
	virtual void StartSearch() = 0;
	virtual slist_t* GetSlist() = 0;

	static VNetworkPublic* Create();
};

extern VNetworkPublic*	GNet;
