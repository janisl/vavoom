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

class VNetContext;

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
	MAX_MESSAGE_HEADER_BITS		= 61,
	OUT_MESSAGE_SIZE			= MAX_MSGLEN * 8 - MAX_PACKET_HEADER_BITS -
		MAX_MESSAGE_HEADER_BITS - MAX_PACKET_TRAILER_BITS,
};

enum { MAX_CHANNELS		= 265 };

enum EChannelType
{
	CHANNEL_General		= 1,
	CHANNEL_Level,
	CHANNEL_Player,
	CHANNEL_Thinker,

	CHANNEL_MAX			= 8
};

enum EChannelIndex
{
	CHANIDX_General,
	CHANIDX_Player,
	CHANIDX_Level,
	CHANIDX_ThinkersStart
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
	virtual double SetNetTime() = 0;

	static VNetworkPublic* Create();
};

//
//	VChannel
//
//	Base class for network channels that are responsible for sending and
// receiving of the data.
//
class VChannel
{
public:
	VNetConnection*		Connection;
	vint32				Index;
	vuint8				Type;
	vuint8				OpenedLocally;
	vuint8				Closing;
	VMessageIn*			InMsg;
	VMessageOut*		OutMsg;

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

//
//	VLevelChannel
//
//	A channel for updating level data.
//
class VLevelChannel : public VChannel
{
public:
	VLevel*			Level;
	rep_side_t*		Sides;
	rep_sector_t*	Sectors;
	rep_polyobj_t*	PolyObjs;

	VLevelChannel(VNetConnection*, vint32, vuint8 = true);
	~VLevelChannel();
	void SetLevel(VLevel*);
	void Update();
	void SendNewLevel();
	void SendStaticLights();
	void ParsePacket(VMessageIn&);
};

//
//	VThinkerChannel
//
//	A channel for updating thinkers.
//
class VThinkerChannel : public VChannel
{
public:
	VThinker*		Thinker;
	VClass*			ThinkerClass;
	vuint8*			OldData;
	bool			NewObj;
	bool			UpdatedThisFrame;
	vuint8*			FieldCondValues;

	VThinkerChannel(VNetConnection*, vint32, vuint8 = true);
	~VThinkerChannel();
	void SetThinker(VThinker*);
	void EvalCondValues(VObject*, VClass*, vuint8*);
	void Update();
	void ParsePacket(VMessageIn&);
	void Close();
};

//
//	VPlayerChannel
//
//	A channel for updating player data.
//
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
	void EvalCondValues(VObject*, VClass*, vuint8*);
	void Update();
	void ParsePacket(VMessageIn&);
};

enum ENetConState
{
	NETCON_Closed,
	NETCON_Open,
};

//
//	VNetConnection
//
//	Network connection class, responsible for sending and receiving network
// packets and managing of channels.
//
class VNetConnection
{
protected:
	VSocketPublic*						NetCon;
public:
	VNetworkPublic*						Driver;
	VNetContext*						Context;
	ENetConState						State;
	VMessageOut							Message;
	double								LastSendTime;
	bool								NeedsUpdate;
	bool								AutoAck;
	VBitStreamWriter					Out;
	VChannel*							Channels[MAX_CHANNELS];
	TArray<VChannel*>					OpenChannels;
	vuint32								InSequence[MAX_CHANNELS];
	vuint32								OutSequence[MAX_CHANNELS];
	TMap<VThinker*, VThinkerChannel*>	ThinkerChannels;
	vuint32								AckSequence;
	vuint32								UnreliableSendSequence;
	vuint32								UnreliableReceiveSequence;

	VNetConnection(VSocketPublic*, VNetContext*);
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
	void Tick();
};

//
//	VNetContext
//
//	Class that provides access to client or server specific data.
//
class VNetContext
{
public:
	VField*			RoleField;
	VField*			RemoteRoleField;

	VNetContext();
	virtual ~VNetContext();

	//	VNetContext interface
	virtual VChannel* CreateGenChannel(VNetConnection*) = 0;
	virtual VLevel* GetLevel() = 0;
};

//
//	VClientNetContext
//
//	A client side network context.
//
class VClientNetContext : public VNetContext
{
public:
	//	VNetContext interface
	VChannel* CreateGenChannel(VNetConnection*);
	VLevel* GetLevel();
};

//
//	VServerNetContext
//
//	Server side network context.
//
class VServerNetContext : public VNetContext
{
public:
	//	VNetContext interface
	VChannel* CreateGenChannel(VNetConnection*);
	VLevel* GetLevel();
};

//	Global access to the low-level networking services.
extern VNetworkPublic*	GNet;

#endif
