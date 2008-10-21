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

#ifndef _NET_LOC_H
#define _NET_LOC_H

#define check(a)
#define guard(a)
#define unguard

#include "cmdlib.h"
#include "array.h"
using namespace VavoomUtils;

enum
{
	MAX_MSGLEN			= 1024,		// max length of a message

	NET_NAMELEN			= 64,

	MAX_NET_DRIVERS		= 8,

	HOSTCACHESIZE		= 8,

	NET_DATAGRAMSIZE	= MAX_MSGLEN,

	HostPort			= 26001,
};

class VNetDriver;
class VNetLanDriver;

struct sockaddr_t
{
	vint16		sa_family;
	vint8		sa_data[14];
};

class VSocket
{
public:
	VSocket*		Next;

	VNetDriver*		Driver;

	VSocket(VNetDriver*);
	~VSocket();

	virtual int GetMessage(TArray<vuint8>&) = 0;
	virtual int SendMessage(vuint8*, vuint32) = 0;
};

struct VNetPollProcedure
{
	VNetPollProcedure*	next;
	double				nextTime;
	void				(*procedure)(void*);
	void*				arg;

	VNetPollProcedure()
	: next(NULL)
	, nextTime(0.0)
	, procedure(NULL)
	, arg(NULL)
	{}
	VNetPollProcedure(void (*aProcedure)(void*), void* aArg)
	: next(NULL)
	, nextTime(0.0)
	, procedure(aProcedure)
	, arg(aArg)
	{}
};

class VNetLanDriver
{
public:
	const char*		name;
	bool			initialised;
	int				controlSock;

	VNetLanDriver(int, const char*);
	virtual int Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Listen(bool) = 0;
	virtual int OpenSocket(int) = 0;
	virtual int CloseSocket(int) = 0;
	virtual int Connect(int, sockaddr_t*) = 0;
	virtual int CheckNewConnections() = 0;
	virtual int Read(int, vuint8*, int, sockaddr_t*) = 0;
	virtual int Write(int, vuint8*, int, sockaddr_t*) = 0;
	virtual int Broadcast(int, vuint8*, int) = 0;
	virtual const char* AddrToString(sockaddr_t*) = 0;
	virtual int StringToAddr(const char*, sockaddr_t*) = 0;
	virtual int GetSocketAddr(int, sockaddr_t*) = 0;
	virtual const char* GetNameFromAddr(sockaddr_t*) = 0;
	virtual int GetAddrFromName(const char*, sockaddr_t*) = 0;
	virtual int AddrCompare(sockaddr_t*, sockaddr_t*) = 0;
	virtual int GetSocketPort(sockaddr_t* addr) = 0;
	virtual int SetSocketPort(sockaddr_t* addr, int port) = 0;
};

#endif
