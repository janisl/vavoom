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

#define NET_HEADERSIZE		(2 * sizeof(vuint32) + sizeof(vuint16))
#define NET_DATAGRAMSIZE	(MAX_DATAGRAM + NET_HEADERSIZE)

class VNetDriver
{
public:
	const char*	name;
	bool		initialised;

	VNetDriver(int, const char*);
	virtual ~VNetDriver();
	virtual int Init() = 0;
	virtual void Listen(bool) = 0;
	virtual void SearchForHosts(bool) = 0;
	virtual VSocket* Connect(const char*) = 0;
	virtual VSocket* CheckNewConnections() = 0;
	virtual int GetMessage(VSocket*) = 0;
	virtual int SendMessage(VSocket*, VMessage*) = 0;
	virtual int SendUnreliableMessage(VSocket*, VMessage*) = 0;
	virtual bool CanSendMessage(VSocket*) = 0;
	virtual bool CanSendUnreliableMessage(VSocket*) = 0;
	virtual void Close(VSocket*) = 0;
	virtual void Shutdown() = 0;
};

class VNetLanDriver
{
public:
	const char*	name;
	bool		initialised;
	int			controlSock;

	VNetLanDriver(int, const char*);
	virtual ~VNetLanDriver();
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
	virtual char* AddrToString(sockaddr_t*) = 0;
	virtual int StringToAddr(const char*, sockaddr_t*) = 0;
	virtual int GetSocketAddr(int, sockaddr_t*) = 0;
	virtual VStr GetNameFromAddr(sockaddr_t*) = 0;
	virtual int GetAddrFromName(const char*, sockaddr_t*) = 0;
	virtual int AddrCompare(sockaddr_t*, sockaddr_t*) = 0;
	virtual int GetSocketPort(sockaddr_t* addr) = 0;
	virtual int SetSocketPort(sockaddr_t* addr, int port) = 0;
};

#endif
