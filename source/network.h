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

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "protocol.h"
#include "message.h"	//	Network message class

//	Packet header IDs.
//	Since control and data communications are on different ports, there should
// never be a case when these are mixed. But I will keep it for now just in
// case.
enum
{
	NETPACKET_DATA		= 0x40,
	NETPACKET_CTL		= 0x80
};

enum
{
	MAX_MSGLEN					= 1024,		// max length of a message
	MAX_PACKET_HEADER_BITS		= 40,
	MAX_PACKET_TRAILER_BITS		= 1,
	MAX_MESSAGE_HEADER_BITS		= 59,
	OUT_MESSAGE_SIZE			= MAX_MSGLEN * 8 - MAX_PACKET_HEADER_BITS -
		MAX_MESSAGE_HEADER_BITS - MAX_PACKET_TRAILER_BITS,
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

	double			ConnectTime;
	double			LastMessageTime;

	virtual bool IsLocalConnection() = 0;
	virtual int GetMessage(TArray<vuint8>&) = 0;
	virtual int SendMessage(vuint8*, vuint32) = 0;
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

	double			NetTime;
	
	//	Statistic counters.
	int				MessagesSent;
	int				MessagesReceived;
	int				UnreliableMessagesSent;
	int				UnreliableMessagesReceived;
	int				packetsSent;
	int				packetsReSent;
	int				packetsReceived;
	int				receivedDuplicateCount;
	int				shortPacketCount;
	int				droppedDatagrams;

	static VCvarF	MessageTimeOut;

	VNetworkPublic();

	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual VSocketPublic* Connect(const char*) = 0;
	virtual VSocketPublic* CheckNewConnections() = 0;
	virtual void Poll() = 0;
	virtual void StartSearch() = 0;
	virtual slist_t* GetSlist() = 0;

	static VNetworkPublic* Create();
};

enum { MAX_CHANNELS		= 265 };

enum EChannelType
{
	CHANNEL_General		= 1,
	CHANNEL_Player,
	CHANNEL_Entity,

	CHANNEL_MAX			= 8
};

class VChannel
{
public:
	VNetConnection*		Connection;
	vint32				Index;
	vuint8				Type;
	vuint8				OpenedLocally;
	VMessageIn*			InMsg;
	VMessageOut*		OutMsg;
	vuint32				ReceiveSequence;
	vuint32				SendSequence;

	VChannel(VNetConnection*, EChannelType, vint32, vuint8);
	virtual ~VChannel();

	//	VChannel interface
	void ReceivedRawMessage(VMessageIn&);
	virtual void ParsePacket(VMessageIn&) = 0;
	void SendMessage(VMessageOut*);
	virtual void ReceivedAck();
	virtual void Close();
	virtual void Tick();
};

class VEntityChannel : public VChannel
{
public:
	VEntity*		Ent;
	vuint8*			OldData;
	bool			NewObj;
	bool			PendingClose;
	bool			UpdatedThisFrame;
	vuint8*			FieldCondValues;

	VEntityChannel(VNetConnection*, vint32, vuint8 = true);
	~VEntityChannel();
	void SetEntity(VEntity*);
	void Update();
	void ParsePacket(VMessageIn&);
};

class VPlayerChannel : public VChannel
{
public:
	VBasePlayer*	Plr;
	vuint8*			OldData;
	bool			NewObj;
	vuint8*			FieldCondValues;

	VPlayerChannel(VNetConnection*, vint32, vuint8 = true);
	~VPlayerChannel();
	void SetPlayer(VBasePlayer*);
	void Update();
	void ParsePacket(VMessageIn&);
};

enum ENetConState
{
	NETCON_Closed,
	NETCON_Open,
};

class VNetConnection
{
protected:
	VSocketPublic*					NetCon;
public:
	VNetworkPublic*					Driver;
	ENetConState					State;
	VMessageOut						Message;
	double							LastMessage;
	bool							NeedsUpdate;
	bool							AutoAck;
	VBitStreamWriter				Out;
	VChannel*						Channels[MAX_CHANNELS];
	TArray<VChannel*>				OpenChannels;
	TMap<VEntity*, VEntityChannel*>	EntityChannels;
	vuint32							AckSequence;
	vuint32							UnreliableSendSequence;
	vuint32							UnreliableReceiveSequence;

	VNetConnection(VSocketPublic*);
	virtual ~VNetConnection();

	//	VNetConnection interface
	void GetMessages();
	virtual int GetRawPacket(TArray<vuint8>&);
	void ReceivedPacket(VBitStreamReader&);
	virtual void SendRawMessage(VMessageOut&);
	virtual void SendAck(vuint32);
	void PrepareOut(int);
	void Flush();
	bool IsLocalConnection();
	VStr GetAddress() const
	{
		return NetCon->Address;
	}
	virtual VLevel* GetLevel() = 0;
	void Tick();
};

class VClientGenChannel : public VChannel
{
public:
	VClientGenChannel(VNetConnection* AConnection, vint32 AIndex, vuint8 AOpenedLocally = true)
	: VChannel(AConnection, CHANNEL_General, AIndex, AOpenedLocally)
	{}

	//	VChannel interface
	void ParsePacket(VMessageIn&);
};

class VClientPlayerNetInfo : public VNetConnection
{
public:
	VClientPlayerNetInfo(VSocketPublic* Sock)
	: VNetConnection(Sock)
	{
		new VClientGenChannel(this, 0);
	}

	//	VNetConnection interface
	int GetRawPacket(TArray<vuint8>&);
	void SendRawMessage(VMessageOut&);
	VLevel* GetLevel();
};

class VServerGenChannel : public VChannel
{
public:
	VServerGenChannel(VNetConnection* AConnection, vint32 AIndex, vuint8 AOpenedLocally = true)
	: VChannel(AConnection, CHANNEL_General, AIndex, AOpenedLocally)
	{}

	//	VChannel interface
	void ParsePacket(VMessageIn&);
};

class VServerPlayerNetInfo : public VNetConnection
{
public:
	VServerPlayerNetInfo(VSocketPublic* Sock)
	: VNetConnection(Sock)
	{
		new VServerGenChannel(this, 0);
	}

	VLevel* GetLevel();
};

extern VNetworkPublic*	GNet;

#endif
