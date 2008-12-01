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
//	DEMO CODE
//
//	When a demo is playing back, all NET_SendMessages are skipped, and
// NET_GetMessages are read from the demo file.
//
//	Whenever cl->time gets past the last received message, another message
// is read from the demo file.
//

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "network.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

#ifdef CLIENT

//==========================================================================
//
//	VDemoPlaybackNetConnection::VDemoPlaybackNetConnection
//
//==========================================================================

VDemoPlaybackNetConnection::VDemoPlaybackNetConnection(VNetContext* AContext,
	VBasePlayer* AOwner, bool ATimeDemo)
: VNetConnection(NULL, AContext, AOwner)
, NextPacketTime(0)
, bTimeDemo(ATimeDemo)
{
	AutoAck = true;
	*cls.demofile << NextPacketTime;

	if (bTimeDemo)
	{
		//	cls.td_starttime will be grabbed at the second frame of the demo,
		// so all the loading time doesn't get counted
		td_startframe = host_framecount;
		td_lastframe = -1;		// get a new message this frame
	}
}

//==========================================================================
//
//	VDemoPlaybackNetConnection::~VDemoPlaybackNetConnection
//
//==========================================================================

VDemoPlaybackNetConnection::~VDemoPlaybackNetConnection()
{
	delete cls.demofile;
	cls.demoplayback = false;
	cls.demofile = NULL;

	if (bTimeDemo)
	{
		// the first frame didn't count
		int frames = (host_framecount - td_startframe) - 1;
		float time = realtime - td_starttime;
		if (!time)
		{
			time = 1;
		}
		GCon->Logf("%d frames %f seconds %f fps", frames, time, frames / time);
	}
}

//==========================================================================
//
//	VDemoPlaybackNetConnection::GetRawPacket
//
//	Handles recording and playback of demos, on top of NET_ code
//
//==========================================================================

int VDemoPlaybackNetConnection::GetRawPacket(TArray<vuint8>& Data)
{
	guard(VDemoPlaybackNetConnection::GetRawPacket);
	// decide if it is time to grab the next message
	if (cls.signon)	// allways grab until fully connected
	{
		if (bTimeDemo)
		{
			if (host_framecount == td_lastframe)
			{
				return 0;		// allready read this frame's message
			}
			td_lastframe = host_framecount;
			// if this is the second frame, grab the real  cls.td_starttime
			// so the bogus time on the first frame doesn't count
			if (host_framecount == td_startframe + 1)
			{
				td_starttime = realtime;
			}
		}
		else if (GClLevel->Time < NextPacketTime)
		{
			return 0;		// don't need another message yet
		}
	}

	if (cls.demofile->AtEnd())
	{
		State = NETCON_Closed;
		return 0;
	}

	// get the next message
	vint32 MsgSize;
	*cls.demofile << MsgSize;
	*cls.demofile << cl->ViewAngles;

	if (MsgSize > OUT_MESSAGE_SIZE)
		Sys_Error("Demo message > MAX_MSGLEN");
	Data.SetNum(MsgSize);
	cls.demofile->Serialise(Data.Ptr(), MsgSize);
	if (cls.demofile->IsError())
	{
		State = NETCON_Closed;
		return 0;
	}

	if (!cls.demofile->AtEnd())
	{
		*cls.demofile << NextPacketTime;
	}

	return 1;
	unguard;
}

//==========================================================================
//
//	VDemoPlaybackNetConnection::SendRawMessage
//
//==========================================================================

void VDemoPlaybackNetConnection::SendRawMessage(VMessageOut&)
{
}

//==========================================================================
//
//	VDemoRecordingNetConnection::VDemoRecordingNetConnection
//
//==========================================================================

VDemoRecordingNetConnection::VDemoRecordingNetConnection(VSocketPublic* Sock,
	VNetContext* AContext, VBasePlayer* AOwner)
: VNetConnection(Sock, AContext, AOwner)
{
}

//==========================================================================
//
//	VDemoRecordingNetConnection::GetRawPacket
//
//	Handles recording and playback of demos, on top of NET_ code
//
//==========================================================================

int VDemoRecordingNetConnection::GetRawPacket(TArray<vuint8>& Data)
{
	guard(VDemoRecordingNetConnection::GetRawPacket);
	int r = VNetConnection::GetRawPacket(Data);

	if (r == 1 && cls.demorecording)
	{
		//
		//	Dumps the current net message, prefixed by the length and view angles
		//
		float Time = GClLevel ? GClLevel->Time : 0.0;
		*cls.demofile << Time;
		vint32 MsgSize = Data.Num();
		*cls.demofile << MsgSize;
		*cls.demofile << cl->ViewAngles;
		cls.demofile->Serialise(Data.Ptr(), Data.Num());
		cls.demofile->Flush();
	}
	
	return r;
	unguard;
}

//==========================================================================
//
//	VDemoRecordingSocket::IsLocalConnection
//
//==========================================================================

bool VDemoRecordingSocket::IsLocalConnection()
{
	return false;
}

//==========================================================================
//
//	VDemoRecordingSocket::GetMessage
//
//==========================================================================

int VDemoRecordingSocket::GetMessage(TArray<vuint8>&)
{
	return 0;
}

//==========================================================================
//
//	VDemoRecordingSocket::SendMessage
//
//==========================================================================

int VDemoRecordingSocket::SendMessage(vuint8* Msg, vuint32 MsgSize)
{
	guard(VDemoRecordingSocket::SendMessage);
	if (cls.demorecording)
	{
		//
		//	Dumps the current net message, prefixed by the length and view angles
		//
		float Time = GClLevel ? GClLevel->Time : 0.0;
		*cls.demofile << Time;
		*cls.demofile << MsgSize;
		if (cl)
		{
			*cls.demofile << cl->ViewAngles;
		}
		else
		{
			TAVec A(0, 0, 0);
			*cls.demofile << A;
		}
		cls.demofile->Serialise(Msg, MsgSize);
		cls.demofile->Flush();
	}
	return 1;
	unguard;
}

#endif
