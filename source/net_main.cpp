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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "net_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VNetwork : public VNetworkLocal
{
public:
	//	Public API
	VNetwork();
	~VNetwork();
	void Init();
	void Shutdown();
	VSocketPublic* Connect(const char*);
	VSocketPublic* CheckNewConnections();
	void Poll();
	void StartSearch();
	slist_t* GetSlist();

	//	API only for network drivers!
	double SetNetTime();
	void SchedulePollProcedure(VNetPollProcedure*, double);

	void Slist();

private:
	VNetPollProcedure	SlistSendProcedure;
	VNetPollProcedure	SlistPollProcedure;

	bool				SlistInProgress;
	bool				SlistSilent;
	bool				SlistLocal;
	bool				SlistSorted;
	double				SlistStartTime;
	int					SlistLastShown;

	slist_t				slist;

	VNetPollProcedure*	PollProcedureList;

	static void Slist_Send(void*);
	static void Slist_Poll(void*);
	void Slist_Send();
	void Slist_Poll();
	void PrintSlistHeader();
	void PrintSlist();
	void PrintSlistTrailer();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VNetworkPublic*	GNet;

VCvarS			VNetworkLocal::HostName("hostname", "UNNAMED");
VCvarF			VNetworkPublic::MessageTimeOut("net_messagetimeout", "300");

VNetDriver*		VNetworkLocal::Drivers[MAX_NET_DRIVERS];
int				VNetworkLocal::NumDrivers = 0;

VNetLanDriver*	VNetworkLocal::LanDrivers[MAX_NET_DRIVERS];
int				VNetworkLocal::NumLanDrivers = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VNetworkPublic::Create
//
//==========================================================================

VNetworkPublic* VNetworkPublic::Create()
{
	return new VNetwork();
}

//==========================================================================
//
//	VNetworkPublic::VNetworkPublic
//
//==========================================================================

VNetworkPublic::VNetworkPublic()
: NetTime(0.0)
, MessagesSent(0)
, MessagesReceived(0)
, UnreliableMessagesSent(0)
, UnreliableMessagesReceived(0)
, packetsSent(0)
, packetsReSent(0)
, packetsReceived(0)
, receivedDuplicateCount(0)
, shortPacketCount(0)
, droppedDatagrams(0)
{
}

//==========================================================================
//
//	VNetworkLocal::VNetworkLocal
//
//==========================================================================

VNetworkLocal::VNetworkLocal()
: ActiveSockets(NULL)
, HostCacheCount(0)
, HostPort(0)
, DefaultHostPort(26000)
, IpxAvailable(false)
, IpAvailable(false)
, Listening(false)
{
	MyIpxAddress[0] = 0;
	MyIpAddress[0] = 0;
	ReturnReason[0] = 0;
}

//==========================================================================
//
//	VNetwork::VNetwork
//
//==========================================================================

VNetwork::VNetwork()
: SlistSendProcedure(Slist_Send, this)
, SlistPollProcedure(Slist_Poll, this)
, SlistInProgress(false)
, SlistSilent(false)
, SlistLocal(true)
, SlistSorted(true)
, SlistStartTime(0.0)
, SlistLastShown(0)
, PollProcedureList(NULL)
{
}

//==========================================================================
//
//	VNetwork::~VNetwork
//
//==========================================================================

VNetwork::~VNetwork()
{
	Shutdown();
}

//==========================================================================
//
//	VNetwork::Init
//
//==========================================================================

void VNetwork::Init()
{
	guard(VNetwork::Init);
	const char* p = GArgs.CheckValue("-port");
	if (p)
	{
		DefaultHostPort = atoi(p);
	}
	HostPort = DefaultHostPort;

#ifdef CLIENT
/*	if (GArgs.CheckParm("-listen") || cls.state == ca_dedicated)
		Listening = true;
*/
#else
	Listening = true;
#endif
	SetNetTime();

	// Initialise all the drivers
	for (int i = 0; i < NumDrivers; i++)
	{
		Drivers[i]->Net = this;
		if (Drivers[i]->Init() != -1)
		{
			Drivers[i]->initialised = true;
			if (Listening)
				Drivers[i]->Listen(true);
		}
	}

	if (*MyIpxAddress)
		GCon->Logf(NAME_DevNet, "IPX address %s", MyIpxAddress);
	if (*MyIpAddress)
		GCon->Logf(NAME_DevNet, "TCP/IP address %s", MyIpAddress);
	unguard;
}

//==========================================================================
//
//	VNetwork::Shutdown
//
//==========================================================================

void VNetwork::Shutdown()
{
	guard(VNetwork::Shutdown);
	SetNetTime();

	while (ActiveSockets)
	{
		delete ActiveSockets;
	}

	//
	// shutdown the drivers
	//
	for (int i = 0; i < NumDrivers; i++)
	{
		if (Drivers[i]->initialised)
		{
			Drivers[i]->Shutdown();
			Drivers[i]->initialised = false;
		}
	}
	unguard;
}

//==========================================================================
//
//	VNetwork::SetNetTime
//
//==========================================================================

double VNetwork::SetNetTime()
{
	guard(VNetwork::SetNetTime);
	NetTime = Sys_Time();
	return NetTime;
	unguard;
}

//==========================================================================
//
//	VNetwork::Poll
//
//==========================================================================

void VNetwork::Poll()
{
	guard(VNetwork::Poll);
	SetNetTime();

	for (VNetPollProcedure *pp = PollProcedureList; pp; pp = pp->next)
	{
		if (pp->nextTime > NetTime)
			break;
		PollProcedureList = pp->next;
		pp->procedure(pp->arg);
	}
	unguard;
}

//==========================================================================
//
//	VNetwork::SchedulePollProcedure
//
//==========================================================================

void VNetwork::SchedulePollProcedure(VNetPollProcedure* proc,
	double timeOffset)
{
	guard(VNetwork::SchedulePollProcedure);
	VNetPollProcedure *pp, *prev;

	proc->nextTime = Sys_Time() + timeOffset;
	for (pp = PollProcedureList, prev = NULL; pp; pp = pp->next)
	{
		if (pp->nextTime >= proc->nextTime)
			break;
		prev = pp;
	}

	if (prev == NULL)
	{
		proc->next = PollProcedureList;
		PollProcedureList = proc;
	}
	else
	{
		proc->next = pp;
		prev->next = proc;
	}
	unguard;
}

//==========================================================================
//
//	VNetwork::Slist
//
//==========================================================================

void VNetwork::Slist()
{
	guard(NET_Slist);
	if (SlistInProgress)
		return;

	if (!SlistSilent)
	{
		GCon->Log("Looking for Vavoom servers...");
		PrintSlistHeader();
	}

	SlistInProgress = true;
	SlistStartTime = Sys_Time();

	SchedulePollProcedure(&SlistSendProcedure, 0.0);
	SchedulePollProcedure(&SlistPollProcedure, 0.1);

	HostCacheCount = 0;
	unguard;
}

//==========================================================================
//
//	VNetwork::Slist_Send
//
//==========================================================================

void VNetwork::Slist_Send(void* Arg)
{
	((VNetwork*)Arg)->Slist_Send();
}

//==========================================================================
//
//	VNetwork::Slist_Poll
//
//==========================================================================

void VNetwork::Slist_Poll(void* Arg)
{
	((VNetwork*)Arg)->Slist_Poll();
}

//==========================================================================
//
//	VNetwork::Slist_Send
//
//==========================================================================

void VNetwork::Slist_Send()
{
	guard(VNetwork::Slist_Send);
	for (int i = 0; i < NumDrivers; i++)
	{
		if (!SlistLocal && i == 0)
			continue;
		if (Drivers[i]->initialised == false)
			continue;
		Drivers[i]->SearchForHosts(true);
	}

	if ((Sys_Time() - SlistStartTime) < 0.5)
		SchedulePollProcedure(&SlistSendProcedure, 0.75);
	unguard;
}

//==========================================================================
//
//	VNetwork::Slist_Poll
//
//==========================================================================

void VNetwork::Slist_Poll()
{
	guard(VNetwork::Slist_Poll);
	for (int i = 0; i < NumDrivers; i++)
	{
		if (!SlistLocal && i == 0)
			continue;
		if (Drivers[i]->initialised == false)
			continue;
		Drivers[i]->SearchForHosts(false);
	}

	if (!SlistSilent)
		PrintSlist();

	if ((Sys_Time() - SlistStartTime) < 1.5)
	{
		SchedulePollProcedure(&SlistPollProcedure, 0.1);
		return;
	}

	if (!SlistSilent)
		PrintSlistTrailer();
	SlistInProgress = false;
	SlistSilent = false;
	SlistLocal = true;
	SlistSorted = false;
	unguard;
}

//==========================================================================
//
//	VNetwork::PrintSlistHeader
//
//==========================================================================

void VNetwork::PrintSlistHeader()
{
	GCon->Log("Server          Map             Users");
	GCon->Log("--------------- --------------- -----");
	SlistLastShown = 0;
}

//==========================================================================
//
//	VNetwork::PrintSlist
//
//==========================================================================

void VNetwork::PrintSlist()
{
	int n;

	for (n = SlistLastShown; n < HostCacheCount; n++)
	{
		if (HostCache[n].MaxUsers)
			GCon->Logf("%-15s %-15s %2d/%2d", *HostCache[n].Name, 
				*HostCache[n].Map, HostCache[n].Users, HostCache[n].MaxUsers);
		else
			GCon->Logf("%-15s %-15s", *HostCache[n].Name, *HostCache[n].Map);
	}
	SlistLastShown = n;
}

//==========================================================================
//
//	VNetwork::PrintSlistTrailer
//
//==========================================================================

void VNetwork::PrintSlistTrailer()
{
	if (HostCacheCount)
		GCon->Log("== end list ==");
	else
		GCon->Log("No Vavoom servers found.");
	GCon->Log("");
}

//==========================================================================
//
//	VNetwork::StartSearch
//
//==========================================================================

void VNetwork::StartSearch()
{
	guard(VNetwork::StartSearch);
	SlistSilent = true;
	SlistLocal = false;
	Slist();
	unguard;
}

//==========================================================================
//
//	VNetwork::GetSlist
//
//==========================================================================

slist_t* VNetwork::GetSlist()
{
	guard(VNetwork::GetSlist);
	if (!SlistSorted)
	{
		if (HostCacheCount > 1)
		{
			vuint8 temp[sizeof(hostcache_t)];
			for (int i = 0; i < HostCacheCount; i++)
				for (int j = i + 1; j < HostCacheCount; j++)
					if (HostCache[j].Name.Cmp(HostCache[i].Name) < 0)
					{
						memcpy(&temp, &HostCache[j], sizeof(hostcache_t));
						memcpy(&HostCache[j], &HostCache[i], sizeof(hostcache_t));
						memcpy(&HostCache[i], &temp, sizeof(hostcache_t));
					}
		}
		SlistSorted = true;
		memset(ReturnReason, 0, sizeof(ReturnReason));
	}

	if (SlistInProgress)
		slist.Flags |= slist_t::SF_InProgress;
	else
		slist.Flags &= ~slist_t::SF_InProgress;
	slist.Count = HostCacheCount;
	slist.Cache = HostCache;
	slist.ReturnReason = ReturnReason;
	return &slist;
	unguard;
}

//==========================================================================
//
//	VNetwork::Connect
//
//==========================================================================

VSocketPublic* VNetwork::Connect(const char* InHost)
{
	guard(VNetwork::Connect);
	VStr		host = InHost;
	VSocket*	ret;
	int			numdrivers = NumDrivers;
	int			n;

	SetNetTime();

	if (host)
	{
		if (host == "local")
		{
			numdrivers = 1;
			goto JustDoIt;
		}

		if (HostCacheCount)
		{
			for (n = 0; n < HostCacheCount; n++)
			{
				if (HostCache[n].Name.ICmp(host) == 0)
				{
					host = HostCache[n].CName;
					break;
				}
			}
			if (n < HostCacheCount)
				goto JustDoIt;
		}
	}

	SlistSilent = host ? true : false;
	Slist();

	while (SlistInProgress)
		Poll();

	if (!host)
	{
		if (HostCacheCount != 1)
			return NULL;
		host = HostCache[0].CName;
		GCon->Log("Connecting to...");
		GCon->Logf("%s @ %s", *HostCache[0].Name, *host);
		GCon->Log("");
	}

	if (HostCacheCount)
	{
		for (n = 0; n < HostCacheCount; n++)
		{
			if (HostCache[n].Name.ICmp(host) == 0)
			{
				host = HostCache[n].CName;
				break;
			}
		}
	}

JustDoIt:
	for (int i = 0; i < numdrivers; i++)
	{
		if (Drivers[i]->initialised == false)
			continue;
		ret = Drivers[i]->Connect(*host);
		if (ret)
		{
			return ret;
		}
	}

	if (host)
	{
		PrintSlistHeader();
		PrintSlist();
		PrintSlistTrailer();
	}
	
	return NULL;
	unguard;
}

//==========================================================================
//
//	VNetwork::CheckNewConnections
//
//==========================================================================

VSocketPublic* VNetwork::CheckNewConnections()
{
	guard(VNetwork::CheckNewConnections);
	SetNetTime();

	for (int i = 0; i < NumDrivers; i++)
	{
		if (Drivers[i]->initialised == false)
			continue;
		if (i && Listening == false)
			continue;
		VSocket* ret = Drivers[i]->CheckNewConnections();
		if (ret)
		{
			return ret;
		}
	}
	
	return NULL;
	unguard;
}

//==========================================================================
//
//	VSocket::VSocket
//
//==========================================================================

VSocket::VSocket(VNetDriver* Drv)
: Driver(Drv)
{
	// add it to active list
	Next = Driver->Net->ActiveSockets;
	Driver->Net->ActiveSockets = this;

	ConnectTime = Driver->Net->NetTime;
	Address = "UNSET ADDRESS";
	LastMessageTime = Driver->Net->NetTime;
}

//==========================================================================
//
//	VSocket::~VSocket
//
//==========================================================================

VSocket::~VSocket()
{
	// remove it from active list
	if (this == Driver->Net->ActiveSockets)
	{
		Driver->Net->ActiveSockets = Driver->Net->ActiveSockets->Next;
	}
	else
	{
		VSocket* s;
		for (s = Driver->Net->ActiveSockets; s; s = s->Next)
		{
			if (s->Next == this)
			{
				s->Next = this->Next;
				break;
			}
		}
		if (!s)
		{
			Sys_Error("NET_FreeQSocket: not active\n");
		}
	}
}

//==========================================================================
//
//	VNetDriver::VNetDriver
//
//==========================================================================

VNetDriver::VNetDriver(int Level, const char* AName)
: name(AName)
, initialised(false)
{
	VNetwork::Drivers[Level] = this;
	if (VNetwork::NumDrivers <= Level)
		VNetwork::NumDrivers = Level + 1;
}

//==========================================================================
//
//	VNetLanDriver::VNetLanDriver
//
//==========================================================================

VNetLanDriver::VNetLanDriver(int Level, const char* AName)
: name(AName)
, initialised(false)
, controlSock(0)
{
	VNetwork::LanDrivers[Level] = this;
	if (VNetwork::NumLanDrivers <= Level)
		VNetwork::NumLanDrivers = Level + 1;
}

#if defined CLIENT && defined SERVER // I think like this

//==========================================================================
//
//	COMMAND Listen
//
//==========================================================================

COMMAND(Listen)
{
	guard(COMMAND Listen);
	VNetwork* Net = (VNetwork*)GNet;
	if (Args.Num() != 2)
	{
		GCon->Logf("\"listen\" is \"%d\"", Net->Listening ? 1 : 0);
		return;
	}

	Net->Listening = atoi(*Args[1]) ? true : false;

	for (int i = 0; i < VNetwork::NumDrivers; i++)
	{
		if (VNetwork::Drivers[i]->initialised == false)
			continue;
		VNetwork::Drivers[i]->Listen(Net->Listening);
	}
	unguard;
}

#endif

//==========================================================================
//
//	COMMAND Port
//
//==========================================================================

COMMAND(Port)
{
	guard(COMMAND Port);
	int 	n;

	VNetwork* Net = (VNetwork*)GNet;
	if (Args.Num() != 2)
	{
		GCon->Logf("\"port\" is \"%d\"", Net->HostPort);
		return;
	}

	n = atoi(*Args[1]);
	if (n < 1 || n > 65534)
	{
		GCon->Log("Bad value, must be between 1 and 65534");
		return;
	}

	Net->DefaultHostPort = n;
	Net->HostPort = n;

	if (Net->Listening)
	{
		// force a change to the new port
		GCmdBuf << "listen 0\n";
		GCmdBuf << "listen 1\n";
	}
	unguard;
}

//==========================================================================
//
//	COMMAND Slist
//
//==========================================================================

COMMAND(Slist)
{
	guard(COMMAND Slist);
	((VNetwork*)GNet)->Slist();
	unguard;
}

//==========================================================================
//
//	VNetObjectsMap::VNetObjectsMap
//
//==========================================================================

VNetObjectsMap::VNetObjectsMap()
: Connection(NULL)
{
}

//==========================================================================
//
//	VNetObjectsMap::VNetObjectsMap
//
//==========================================================================

VNetObjectsMap::VNetObjectsMap(VNetConnection* AConnection)
: Connection(AConnection)
{
}

//==========================================================================
//
//	VNetObjectsMap::CanSerialiseObject
//
//==========================================================================

bool VNetObjectsMap::CanSerialiseObject(VObject* Obj)
{
	VThinker* Thinker = Cast<VThinker>(Obj);
	if (Thinker)
	{
		//	Thinker can be serialised only if it has an open channel.
		return !!Connection->ThinkerChannels.FindPtr(Thinker);
	}
	else
	{
		//	We can always serialise NULL object.
		return !Obj;
	}
}

//==========================================================================
//
//	VNetObjectsMap::SerialiseName
//
//==========================================================================

bool VNetObjectsMap::SerialiseName(VStream& Strm, VName& Name)
{
	guard(VNetObjectsMap::SerialiseName);
	//FIXME this will work only for the local connection.
	Strm << *(vint32*)&Name;
	return true;
	unguard;
}

//==========================================================================
//
//	VNetObjectsMap::SerialiseObject
//
//==========================================================================

bool VNetObjectsMap::SerialiseObject(VStream& Strm, VObject*& Obj)
{
	guard(VNetObjectsMap::SerialiseObject);
	if (Strm.IsLoading())
	{
		Obj = NULL;
		vuint8 IsThinker = 0;
		Strm.SerialiseBits(&IsThinker, 1);
		if (IsThinker)
		{
			//	It's a thinker that has an open channel
			vuint32 Index;
			Strm.SerialiseInt(Index, MAX_CHANNELS);
			VChannel* Chan = Connection->Channels[Index];
			if (Chan && Chan->Type == CHANNEL_Thinker && !Chan->Closing)
			{
				Obj = ((VThinkerChannel*)Chan)->Thinker;
			}
		}
		return true;
	}
	else
	{
		VThinker* Thinker = Cast<VThinker>(Obj);
		vuint8 IsThinker = !!Thinker;
		Strm.SerialiseBits(&IsThinker, 1);
		if (Thinker)
		{
			//	It's a thinker. If it has an open channel we can use it's
			// channel number to identify it, otherwise we can't serialise it.
			bool Ret = false;
			vuint32 Index = 0;
			VThinkerChannel* Chan = Connection->ThinkerChannels.FindPtr(Thinker);
			if (Chan && Chan->OpenAcked)
			{
				Index = Chan->Index;
				Ret = true;
			}
			Strm.SerialiseInt(Index, MAX_CHANNELS);
			return Ret;
		}
		return !Obj;
	}
	unguard;
}

//==========================================================================
//
//	VNetObjectsMap::SerialiseClass
//
//==========================================================================

bool VNetObjectsMap::SerialiseClass(VStream& Strm, VClass*& Class)
{
	guard(VNetObjectsMap::SerialiseClass);
	if (Strm.IsLoading())
	{
		vuint32 ClassId;
		Strm.SerialiseInt(ClassId, VMemberBase::GNetClassLookup.Num());
		if (ClassId)
		{
			Class = VMemberBase::GNetClassLookup[ClassId];
		}
		else
		{
			Class = NULL;
		}
	}
	else
	{
		if (Class)
		{
			vuint32 ClassId = Class->NetId;
			Strm.SerialiseInt(ClassId, VMemberBase::GNetClassLookup.Num());
		}
		else
		{
			vuint32 NoClass = 0;
			Strm.SerialiseInt(NoClass, VMemberBase::GNetClassLookup.Num());
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VNetObjectsMap::SerialiseState
//
//==========================================================================

bool VNetObjectsMap::SerialiseState(VStream& Strm, VState*& State)
{
	guard(VNetObjectsMap::SerialiseState);
	if (Strm.IsLoading())
	{
		vuint32 ClassId;
		Strm.SerialiseInt(ClassId, VMemberBase::GNetClassLookup.Num());
		if (ClassId)
		{
			vuint32 StateId;
			Strm.SerialiseInt(StateId,
				VMemberBase::GNetClassLookup[ClassId]->StatesLookup.Num());
			State = VMemberBase::GNetClassLookup[ClassId]->StatesLookup[StateId];
		}
		else
		{
			State = NULL;
		}
	}
	else
	{
		if (State)
		{
			vuint32 ClassId = ((VClass*)State->Outer)->NetId;
			vuint32 StateId = State->NetId;
			checkSlow(ClassId);
			Strm.SerialiseInt(ClassId, VMemberBase::GNetClassLookup.Num());
			Strm.SerialiseInt(StateId,
				((VClass*)State->Outer)->StatesLookup.Num());
		}
		else
		{
			vuint32 NoClass = 0;
			Strm.SerialiseInt(NoClass, VMemberBase::GNetClassLookup.Num());
		}
	}
	return true;
	unguard;
}
