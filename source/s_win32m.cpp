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

#include "winlocal.h"
#include <mmsystem.h>
#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VMMSystemMidiDevice : public VMidiDevice
{
public:
	enum { MAX_TRACKS = 32 };
	enum { MAX_TICKS = 0xFFFFFFFF };

	//	Number and size of playback buffers to keep around
	enum { C_MIDI_BUFFERS = 4 };
	enum { CB_MIDI_BUFFERS = 1024 };

	enum
	{
		STATE_NoFile,
		STATE_Opened,
		STATE_Prerolling,
		STATE_Prerolled,
		STATE_Playing,
		STATE_Paused,
		STATE_Stopping,
		STATE_Reset,
	};

	enum EResult
	{
		RES_Success,
		RES_InvalidFile,
		RES_EndOfFile
	};

	//	Various flags.
	enum
	{
		TRACKF_Eot			= 0x00000001,

		SEQF_Eof			= 0x00000001,
		SEQF_Waiting		= 0x00000002,

		SMFF_Eof			= 0x00000001,
		SMFF_InsertSysEx	= 0x00000002
	};

	//	Midi events.
	enum
	{
		MIDI_Msg		= 0x80,
		MIDI_Meta		= 0xFF,
		MIDI_SysEx		= 0xF0,
		MIDI_SysExEnd	= 0xF7,

		MIDI_MetaEot	= 0x2F,
		MIDI_MetaTempo	= 0x51,
	};

#pragma pack(1)
	struct FChunkHdr
	{
		char			Type[4];
		dword			Length;
	};

	struct FMidiFileHdr
	{
		word			Format;
		word			Tracks;
		word			Division;
	};
#pragma pack()

	struct FEvent
	{
		dword			Delta;
		byte			Event[3];
		dword			ParmCount;
		const byte*		Parm;
	};

	struct FTrack
	{
		dword			FileOffset;
		dword			TotalLength;

		dword			Position;
		dword			BytesLeft;
		const byte*		ImagePtr;
		byte			RunningStatus;

		dword			TrackFlags;
	};

	bool			MusicPaused;
	float			MusVolume;

	const byte*		MidiImage;
	dword			MidiImageSize;

	int				State;				//	Sequencer state (SEQ_S_xxx)
	HMIDIOUT		hMidi;				//	Handle to open MIDI device
	byte			BufAlloc[(sizeof(MIDIHDR) + CB_MIDI_BUFFERS) * C_MIDI_BUFFERS];	//	Streaming buffers -- initial allocation
	LPMIDIHDR		FreeBuffers;		//	Streaming buffers -- free list
	int				BuffersInMMSYSTEM;	//	Streaming buffers -- in use
	dword			SeqFlags;			//	Various sequencer flags

	dword			MidiPosition;
	dword			Format;
	dword			NumTracks;
	dword			TimeDivision;
	dword			SmfFlags;

	dword			PendingUserEvent;
	dword			PendingUserEventCount;
	const byte*		PendingUserEvents;

	FTrack			Tracks[MAX_TRACKS];

	static const int ChanMsgLen[];

	VMMSystemMidiDevice();
	void Init();
	void Shutdown();
	void SetVolume(float);
	void Tick(float);
	void Play(void*, int, const char*, bool);
	void Pause();
	void Resume();
	void Stop();
	bool IsPlaying();

	MMRESULT Preroll();
	void Callback(UINT, DWORD);
	static void PASCAL StaticCallback(HMIDISTRM, UINT, DWORD, DWORD, DWORD);

	EResult ReadEvents(LPMIDIHDR);
	bool InsertParmData(dword, LPMIDIHDR);
	void SeekStart();
	bool BuildFileIndex();
	EResult GetNextEvent(FEvent&);
	static dword GetVDword(const byte*, dword, dword&);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_MIDI_DEVICE(VMMSystemMidiDevice, MIDIDRV_Default, "Default",
	"Windows multimedia system midi device", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

const int VMMSystemMidiDevice::ChanMsgLen[] =
{ 
	0,                      /* 0x   not a status byte   */
	0,                      /* 1x   not a status byte   */
	0,                      /* 2x   not a status byte   */
	0,                      /* 3x   not a status byte   */
	0,                      /* 4x   not a status byte   */
	0,                      /* 5x   not a status byte   */
	0,                      /* 6x   not a status byte   */
	0,                      /* 7x   not a status byte   */
	3,                      /* 8x   Note off            */
	3,                      /* 9x   Note on             */
	3,                      /* Ax   Poly pressure       */
	3,                      /* Bx   Control change      */
	2,                      /* Cx   Program change      */
	2,                      /* Dx   Chan pressure       */
	3,                      /* Ex   Pitch bend change   */
	0,                      /* Fx   SysEx (see below)   */
} ;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VMMSystemMidiDevice::VMMSystemMidiDevice
//
//==========================================================================

VMMSystemMidiDevice::VMMSystemMidiDevice()
: MidiImage(NULL)
, MusicPaused(false)
, MusVolume(-1)
{
}

//==========================================================================
//
//	VMMSystemMidiDevice::Init
//
//==========================================================================

void VMMSystemMidiDevice::Init()
{
	guard(VMMSystemMidiDevice::Init);
	State = STATE_NoFile;
	FreeBuffers = NULL;
	hMidi = NULL;
	BuffersInMMSYSTEM = 0;
	SeqFlags = 0;

	//	Initialize all MIDIHDR's and throw them into a free list
	LPBYTE Work = BufAlloc;
	for (int i = 0; i < C_MIDI_BUFFERS; i++)
	{
		((LPMIDIHDR)Work)->lpNext = FreeBuffers;
		((LPMIDIHDR)Work)->lpData = (char*)(Work + sizeof(MIDIHDR));
		((LPMIDIHDR)Work)->dwBufferLength = CB_MIDI_BUFFERS;
		((LPMIDIHDR)Work)->dwBytesRecorded = 0;
		((LPMIDIHDR)Work)->dwUser = 0;
		((LPMIDIHDR)Work)->dwFlags = 0;

		FreeBuffers = (LPMIDIHDR)Work;

		Work += sizeof(MIDIHDR) + CB_MIDI_BUFFERS;
	}

	Initialised = true;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::Shutdown
//
//==========================================================================

void VMMSystemMidiDevice::Shutdown()
{
	guard(VMMSystemMidiDevice::Shutdown);
	if (Initialised)
	{
		Stop();
		Initialised = false;
	}
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::SetVolume
//
//==========================================================================

void VMMSystemMidiDevice::SetVolume(float Volume)
{
	guard(VMMSystemMidiDevice::SetVolume);
	if (Volume != MusVolume)
	{
		MusVolume = Volume;
		//FIXME set volume.
	}
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::Tick
//
//==========================================================================

void VMMSystemMidiDevice::Tick(float)
{
}

//==========================================================================
//
//	VMMSystemMidiDevice::Play
//
//==========================================================================

void VMMSystemMidiDevice::Play(void* Data, int len, const char* song, bool loop)
{
	guard(VMMSystemMidiDevice::Play);
	//	Stop, close, etc. if we're still playing
	Stop();

	//	Open new file
	SmfFlags = 0;
	PendingUserEvent = 0;
	PendingUserEventCount = 0;
	PendingUserEvents = NULL;

	MidiImage = (byte*)Data;
	MidiImageSize = len;

	//	If the file exists, parse it just enough to pull out the header
	// and build a track index.
	if (!BuildFileIndex())
	{
		return;
	}
	State = STATE_Opened;

	//	Preroll
	if (Preroll() != MMSYSERR_NOERROR)
	{
		return;
	}

	//	Play
	State = STATE_Playing;
	midiStreamRestart((HMIDISTRM)hMidi);

	//	Pause if needed.
	if (!MusVolume || MusicPaused)
	{
		Pause();
	}
	CurrSong = FName(song, FNAME_AddLower8);
	CurrLoop = loop;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::Pause
//
//==========================================================================

void VMMSystemMidiDevice::Pause()
{
	guard(VMMSystemMidiDevice::Pause);
	if (State != STATE_Playing)
		return;

	State = STATE_Paused;
	midiStreamPause((HMIDISTRM)hMidi);
	MusicPaused = true;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::Resume
//
//==========================================================================

void VMMSystemMidiDevice::Resume()
{
	guard(VMMSystemMidiDevice::Resume);
	if (State != STATE_Paused)
		return;

	State = STATE_Playing;
	midiStreamRestart((HMIDISTRM)hMidi);
	MusicPaused = false;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::IsPlaying
//
//	Is the song playing?
//
//==========================================================================

bool VMMSystemMidiDevice::IsPlaying()
{
	guard(VMMSystemMidiDevice::IsPlaying);
	return State == STATE_Playing || State == STATE_Paused;
	unguard;
}

//==========================================================================
//
//  VMMSystemMidiDevice::Stop
//
//==========================================================================

void VMMSystemMidiDevice::Stop()
{
	guard(VMMSystemMidiDevice::Stop);
	if (MidiImage)
	{
		if (MusicPaused)
		{
			Resume();
		}
		if (State != STATE_Playing && State != STATE_Paused)
		{
			SeqFlags &= ~SEQF_Waiting;
		}
		else
		{
			State = STATE_Stopping;
			SeqFlags |= SEQF_Waiting;

			if (MMSYSERR_NOERROR != midiStreamStop((HMIDISTRM)hMidi))
			{
				SeqFlags &= ~SEQF_Waiting;
				return;
			}

			while (BuffersInMMSYSTEM)
				Sleep(0);
		}

		//	Close file.
		if (State == STATE_Opened)
		{
			//	If we were prerolled, need to clean up -- have an open MIDI
			// handle and buffers in the ready queue
			for (LPMIDIHDR lpmh = FreeBuffers; lpmh; lpmh = lpmh->lpNext)
				midiOutUnprepareHeader(hMidi, lpmh, sizeof(*lpmh));
		}

		//	Close midi stream.	
		if (hMidi)
		{
			midiStreamClose((HMIDISTRM)hMidi);
			hMidi = NULL;
		}
		State = STATE_NoFile;
		Z_Free((void*)MidiImage);
		MidiImage = NULL;
		CurrSong = NAME_None;
	}
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::Preroll
//
//	Prepares the file for playback at the given position.
//
//==========================================================================

MMRESULT VMMSystemMidiDevice::Preroll()
{
	guard(VMMSystemMidiDevice::Preroll);
	EResult				smfrc;
	MMRESULT			mmrc = MMSYSERR_NOERROR;
	MIDIPROPTIMEDIV		mptd;
	LPMIDIHDR			lpmh = NULL;
	UINT				uDeviceID;

	if (hMidi)
	{
		//	Recollect buffers from MMSYSTEM back into free queue
		State = STATE_Reset;
		midiOutReset(hMidi);

		while (BuffersInMMSYSTEM)
			Sleep(0);
	}

	BuffersInMMSYSTEM = 0;
	State = STATE_Prerolling;

	//
	// We've successfully opened the file and all of the tracks; now
	// open the MIDI device and set the time division.
	//
	if (!hMidi)
	{
		uDeviceID = 0;
		if ((mmrc = midiStreamOpen((HMIDISTRM*)&hMidi, &uDeviceID, 1,
			(DWORD)StaticCallback, (DWORD)this, CALLBACK_FUNCTION)) != MMSYSERR_NOERROR)
		{
			hMidi = NULL;
			goto seq_Preroll_Cleanup;
		}

		mptd.cbStruct  = sizeof(mptd);
		mptd.dwTimeDiv = TimeDivision;
		if ((mmrc = midiStreamProperty((HMIDISTRM)hMidi,
			(LPBYTE)&mptd, MIDIPROP_SET | MIDIPROP_TIMEDIV)) != MMSYSERR_NOERROR)
		{
			midiStreamClose((HMIDISTRM)hMidi);
			hMidi = NULL;
			mmrc = MCIERR_DEVICE_NOT_READY;
			goto seq_Preroll_Cleanup;
		}
	}

	mmrc = MMSYSERR_NOERROR;

	SeekStart();
	smfrc = RES_Success;

	SeqFlags &= ~SEQF_Eof;
	while (FreeBuffers)
	{
		lpmh = FreeBuffers;
		FreeBuffers = lpmh->lpNext;

		smfrc = ReadEvents(lpmh);
		if (RES_Success != smfrc && RES_EndOfFile != smfrc)
		{
			GCon->Log("Preroll: Invalid file");
			mmrc = MCIERR_INVALID_FILE;
			goto seq_Preroll_Cleanup;
		}

		if (MMSYSERR_NOERROR != (mmrc = midiOutPrepareHeader(hMidi, lpmh, sizeof(*lpmh))))
		{
			GCon->Log("Preroll: midiOutPrepareHeader failed");
			goto seq_Preroll_Cleanup;
		}

		if (MMSYSERR_NOERROR != (mmrc = midiStreamOut((HMIDISTRM)hMidi, lpmh, sizeof(*lpmh))))
		{
			GCon->Log("Preroll: midiStreamOut failed");
			goto seq_Preroll_Cleanup;
		}

		BuffersInMMSYSTEM++;

		if (smfrc == RES_EndOfFile)
		{
			SeqFlags |= SEQF_Eof;
			break;
		}
	}

seq_Preroll_Cleanup:
	if (MMSYSERR_NOERROR != mmrc)
	{
		State = STATE_Opened;
		SeqFlags &= ~SEQF_Waiting;
	}
	else
	{
		State = STATE_Prerolled;
	}

	return mmrc;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::Callback
//
//	Called by the system when a buffer is done.
//
//==========================================================================

void VMMSystemMidiDevice::Callback(UINT uMsg, DWORD dw1)
{
	guard(VMMSystemMidiDevice::Callback);
	LPMIDIHDR		lpmh = (LPMIDIHDR)dw1;

	if (uMsg != MOM_DONE)
		return;

	BuffersInMMSYSTEM--;
	if (State == STATE_Reset)
	{
		//	We're recollecting buffers from MMSYSTEM
		lpmh->lpNext = FreeBuffers;
		FreeBuffers = lpmh;
		return;
	}

	if ((State == STATE_Stopping) || (SeqFlags & SEQF_Eof))
	{
		//	Reached EOF, just put the buffer back on the free list
		lpmh->lpNext = FreeBuffers;
		FreeBuffers = lpmh;

		midiOutUnprepareHeader(hMidi, lpmh, sizeof(*lpmh));

		if (BuffersInMMSYSTEM == 0)
		{
			SeqFlags &= ~SEQF_Waiting;
			//	Restart stream if we are looping.
			if (CurrLoop && State != STATE_Stopping)
			{
				State = STATE_Opened;
				if (Preroll() == MMSYSERR_NOERROR)
				{
					State = STATE_Playing;
					midiStreamRestart((HMIDISTRM)hMidi);
				}
			}
			else
			{
				//	Totally done! Free device.
				midiStreamClose((HMIDISTRM)hMidi);
	
				hMidi = NULL;
				State = STATE_Opened;
			}
		}
	}
	else
	{
		//	Not EOF yet; attempt to fill another buffer
		EResult smfrc = ReadEvents(lpmh);

		switch(smfrc)
		{
		case RES_Success:
			break;

		case RES_EndOfFile:
			SeqFlags |= SEQF_Eof;
			smfrc = RES_Success;
			break;

		default:
			State = STATE_Stopping;
			break;
		}

		if (smfrc == RES_Success)
		{
			BuffersInMMSYSTEM++;
			if (midiStreamOut((HMIDISTRM)hMidi, lpmh, sizeof(*lpmh)) != MMSYSERR_NOERROR)
			{
				BuffersInMMSYSTEM--;
				State = STATE_Stopping;
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::StaticCallback
//
//==========================================================================

void PASCAL VMMSystemMidiDevice::StaticCallback(HMIDISTRM,
	UINT uMsg, DWORD inst, DWORD dw1, DWORD)
{
	((VMMSystemMidiDevice*)inst)->Callback(uMsg, dw1);
}

//==========================================================================
//
//	VMMSystemMidiDevice::ReadEvents
//
//	This function reads events from a track.
//
//==========================================================================

VMMSystemMidiDevice::EResult VMMSystemMidiDevice::ReadEvents(LPMIDIHDR lpmh)
{
	guard(VMMSystemMidiDevice::ReadEvents);
	EResult			smfrc;
	FEvent			event;
	LPDWORD			lpdw;
	DWORD			Tempo;

	//	Read events from the track and pack them into the buffer in polymsg
	// format.
	//	If a SysEx or meta would go over a buffer boundry, split it.
	lpmh->dwBytesRecorded = 0;
	if (PendingUserEvent)
	{
		if (!InsertParmData(0, lpmh))
		{
			return RES_InvalidFile;
		}
	}

	lpdw = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);

	if (SmfFlags & SMFF_Eof)
	{
		return RES_EndOfFile;
	}

	while (1)
	{
		//	If we know ahead of time we won't have room for the event, just
		// break out now. We need 2 DWORD's for the terminator event and at
		// least 2 DWORD's for any event we might store - this will allow us
		// a full short event or the delta time and stub for a long event to
		// be split.
		if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 4 * sizeof(DWORD))
		{
			break;
		}

		smfrc = GetNextEvent(event);
		if (RES_Success != smfrc)
		{
			break;
		}

		if (MIDI_SysEx > event.Event[0])
		{
			*lpdw++ = (DWORD)event.Delta;
			*lpdw++ = 0;
			*lpdw++ = (((DWORD)MEVT_SHORTMSG) << 24) |
						((DWORD)event.Event[0]) |
						(((DWORD)event.Event[1]) << 8) |
						(((DWORD)event.Event[2]) << 16);

			lpmh->dwBytesRecorded += 3 * sizeof(DWORD);
		}
		else if (MIDI_Meta == event.Event[0] &&
			MIDI_MetaEot == event.Event[1])
		{
			//	These are ignoreable since smfReadNextEvent()
			// takes care of track merging
		}
		else if (MIDI_Meta == event.Event[0] &&
			MIDI_MetaTempo == event.Event[1])
		{
			if (event.ParmCount != 3)
			{
				GCon->Log("ReadEvents: Corrupt tempo event");
				return RES_InvalidFile;
			}

			Tempo = (((DWORD)MEVT_TEMPO) << 24)|
						(((DWORD)event.Parm[0]) << 16)|
						(((DWORD)event.Parm[1]) << 8)|
						((DWORD)event.Parm[2]);

			*lpdw++ = (DWORD)event.Delta;
			*lpdw++ = 0;
			*lpdw++ = Tempo;

			lpmh->dwBytesRecorded += 3 * sizeof(DWORD);
		}
		else if (MIDI_Meta != event.Event[0])
		{
			//	Must be F0 or F7 system exclusive or FF meta
			// that we didn't recognize
			PendingUserEventCount = event.ParmCount;
			PendingUserEvents = event.Parm;
			SmfFlags &= ~SMFF_InsertSysEx;

			switch (event.Event[0])
			{
			case MIDI_SysEx:
				SmfFlags |= SMFF_InsertSysEx;
				PendingUserEventCount++;
				//	Falling through...
			case MIDI_SysExEnd:
				PendingUserEvent = ((DWORD)MEVT_LONGMSG) << 24;
				break;
			}

			if (!InsertParmData(event.Delta, lpmh))
			{
				return RES_InvalidFile;
			}

			lpdw = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);
		}
	}

	return (SmfFlags & SMFF_Eof) ? RES_EndOfFile : RES_Success;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::InsertParmData
//
//	Inserts pending long data from a track into the given buffer.
//
//	Fills as much data as will fit while leaving room for the buffer
// terminator.
//
//	If the long data is depleted, resets PendingUserEvent so that the next
// event may be read.
//
//==========================================================================

bool VMMSystemMidiDevice::InsertParmData(dword Delta, LPMIDIHDR lpmh)
{
	guard(VMMSystemMidiDevice::InsertParmData);
	//	Can't fit 4 DWORD's? (Delta + stream-id + event + some data)
	// Can't do anything.
	if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 4 * sizeof(DWORD))
	{
		if (0 == Delta)
			return true;

		//	If we got here with a real delta, that means ReadEvents screwed
		// up calculating left space and we should flag it somehow.
		GCon->Log("Can't fit initial piece of SysEx into buffer!");
		return false;
	}

	LPDWORD pData = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);

	DWORD Length = lpmh->dwBufferLength - lpmh->dwBytesRecorded - 3 * sizeof(DWORD);
	Length = min(Length, PendingUserEventCount);

	*pData++ = (DWORD)Delta;
	*pData++ = 0;
	*pData++ = (PendingUserEvent & 0xFF000000) | (Length & 0x00FFFFFF);

	DWORD Rounded = (Length + 3) & (~3);

	if (SmfFlags & SMFF_InsertSysEx)
	{
		*((LPBYTE)pData) = MIDI_SysEx;
		pData = (LPDWORD)(((LPBYTE)pData) + 1);
		SmfFlags &= ~SMFF_InsertSysEx;
		Length--;
		PendingUserEventCount--;
	}

	if (Length & 0x80000000)
	{
		GCon->Logf("Length %08lX  dwBytesRecorded %08lX  dwBufferLength %08lX",
			Length, lpmh->dwBytesRecorded, lpmh->dwBufferLength);
		GCon->Logf("PendingUserEventCount %08lX  PendingUserEvent %08lX Rounded %08lX",
			PendingUserEventCount, PendingUserEvent, Rounded);
		GCon->Logf("Offset into MIDI image %08lX", (dword)(PendingUserEvents - MidiImage));
		GCon->Logf("!memcpy is about to fault");
	}

	memcpy(pData, PendingUserEvents, Length);
	PendingUserEventCount -= Length;
	if (!PendingUserEventCount)
		PendingUserEvent = 0;

	lpmh->dwBytesRecorded += 3*sizeof(DWORD) + Rounded;

	return true;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::SeekStart
//
//==========================================================================

void VMMSystemMidiDevice::SeekStart()
{
	guard(VMMSystemMidiDevice::SeekStart);
	FTrack*					ptrk;
	dword                   idxTrack;

	MidiPosition = 0;
	SmfFlags &= ~SMFF_Eof;

	for (ptrk = Tracks, idxTrack = NumTracks; idxTrack--; ptrk++)
	{
		ptrk->Position = 0;
		ptrk->BytesLeft = ptrk->TotalLength;
		ptrk->ImagePtr = MidiImage + ptrk->FileOffset;
		ptrk->RunningStatus = 0;
		ptrk->TrackFlags = 0;
	}
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::BuildFileIndex
//
//	Preliminary parsing of a MIDI file.
//
//	This function validates the format of and existing MIDI file and builds
// the handle structure which will refer to it for the lifetime of the
// instance.
//
//	The file header information will be read and verified, and
// BuildTrackIndices will be called on every existing track
// to build keyframes and validate the track format.
//
//==========================================================================

bool VMMSystemMidiDevice::BuildFileIndex()
{
	guard(VMMSystemMidiDevice::BuildFileIndex);
	EResult					smfrc;
	const FChunkHdr*		pCh;
	const FMidiFileHdr*		pFh;
	dword					idx;
	FTrack*					pTrk;
	dword					dwLeft;
	const byte*				hpbImage;
	dword					idxTrack;
	FEvent					event;
	dword					dwLength;

	//	Validate MIDI header
	dwLeft   = MidiImageSize;
	hpbImage = MidiImage;

	if (dwLeft < sizeof(FChunkHdr))
		return false;

	pCh = (FChunkHdr*)hpbImage;

	dwLeft   -= sizeof(FChunkHdr);
	hpbImage += sizeof(FChunkHdr);

	dwLength = BigLong(pCh->Length);
	if (dwLength < sizeof(FMidiFileHdr) || dwLength > dwLeft)
		return false;

	pFh = (FMidiFileHdr*)hpbImage;

	dwLeft -= dwLength;
	hpbImage += dwLength;

	Format = (DWORD)BigShort(pFh->Format);
	NumTracks = (DWORD)BigShort(pFh->Tracks);
	TimeDivision = (DWORD)BigShort(pFh->Division);

	if (NumTracks > MAX_TRACKS)
		return false;

	//	We've successfully parsed the header. Now try to build the track index.
	//	We only check out the track header chunk here; the track will be
	// preparsed after we do a quick integretiy check.
	memset(Tracks, 0, MAX_TRACKS * sizeof(FTrack));

	pTrk = Tracks;

	for (idx = 0; idx < NumTracks; idx++)
	{
		if (dwLeft < sizeof(FChunkHdr))
			return false;

		pCh = (FChunkHdr*)hpbImage;

		dwLeft   -= sizeof(FChunkHdr);
		hpbImage += sizeof(FChunkHdr);

		if (memcmp(pCh->Type, "MTrk", 4))
			return false;

		pTrk->FileOffset = (DWORD)(hpbImage - MidiImage);
		pTrk->TotalLength = BigLong(pCh->Length);

		if (pTrk->TotalLength > dwLeft)
		{
			GCon->Log("Track longer than file!");
			return false;
		}

		dwLeft   -= pTrk->TotalLength;
		hpbImage += pTrk->TotalLength;

		pTrk++;
	}

	//	File looks OK. Now preparse, doing the following:
	// (3) Validate all events in all tracks
	MidiPosition = 0;
	SmfFlags &= ~SMFF_Eof;

	for (pTrk = Tracks, idxTrack = NumTracks; idxTrack--; pTrk++)
	{
		pTrk->Position = 0;
		pTrk->BytesLeft = pTrk->TotalLength;
		pTrk->ImagePtr = MidiImage + pTrk->FileOffset;
		pTrk->RunningStatus = 0;
		pTrk->TrackFlags = 0;
	}

	while (RES_Success == (smfrc = GetNextEvent(event)))
	{
	}

	if (RES_EndOfFile == smfrc || RES_Success == smfrc)
	{
		smfrc = RES_Success;
	}

	return smfrc == RES_Success;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::GetNextEvent
//
//	Read the next event from the given file.
//
//	This is the lowest level of parsing for a raw MIDI stream. The basic
// information about one event in the file will be returned in Event.
//
//	Merging data from all tracks into one stream is performed here.
//
//	pEvent->Delta will contain the tick delta for the event.
//
// pEvent->Event will contain a description of the event.
//  pevent->Event[0] will contain
//    F0 or F7 for a System Exclusive message.
//    FF for a MIDI file meta event.
//    The status byte of any other MIDI message. (Running status will
//    be tracked and expanded).
//
//	pEvent->ParmCount will contain the number of bytes of paramter data
//   which is still in the file behind the event header already read.
//   This data may be read with <f smfGetTrackEventData>. Any unread
//   data will be skipped on the next call to <f smfGetNextTrackEvent>.
//
// Channel messages (0x8? - 0xE?) will always be returned fully in
//   pevent->Event.
//
//  Meta events will contain the meta type in pevent->Event[1].
//
//  System exclusive events will contain only an 0xF0 or 0xF7 in
//    pevent->Event[0].
//
//  The following fields in pTrk are used to maintain state and must
//  be updated if a seek-in-track is performed:
//
//  bRunningStatus contains the last running status message or 0 if
//   there is no valid running status.
//
//  hpbImage is a pointer into the file image of the first byte of
//   the event to follow the event just read.
//
//  dwLeft contains the number of bytes from hpbImage to the end
//   of the track.
//
// Get the next due event from all (in-use?) tracks
//
// For all tracks
//   If not end-of-track
//   decode event delta time without advancing through buffer
//   event_absolute_time = track_tick_time + track_event_delta_time
//   relative_time = event_absolute_time - last_stream_time
//   if relative_time is lowest so far
//    save this track as the next to pull from, along with times
//
// If we found a track with a due event
//  Advance track pointer past event, saving ptr to parm data if needed
//  track_tick_time += track_event_delta_time
//  last_stream_time = track_tick_time
// Else
//  Mark and return end_of_file
//
//==========================================================================

VMMSystemMidiDevice::EResult VMMSystemMidiDevice::GetNextEvent(FEvent& Event)
{
	guard(VMMSystemMidiDevice::GetNextEvent);
	FTrack*		pTrk;
	FTrack*		pTrkFound;
	dword		idxTrack;
	dword		tkEventDelta;
	dword		tkRelTime;
	dword		tkMinRelTime;
	byte		bEvent;
	dword		dwGotTotal;
	dword		dwGot;
	dword		cbEvent;

	if (SmfFlags & SMFF_Eof)
	{
		return RES_EndOfFile;
	}

	pTrkFound = NULL;
	tkMinRelTime = MAX_TICKS;

	for (pTrk = Tracks, idxTrack = NumTracks; idxTrack--; pTrk++)
	{
		if (pTrk->TrackFlags & TRACKF_Eot)
			continue;

		if (!GetVDword(pTrk->ImagePtr, pTrk->BytesLeft, tkEventDelta))
		{
			GCon->Log("Hit end of track w/o end marker!");
			return RES_InvalidFile;
		}

		tkRelTime = pTrk->Position + tkEventDelta - MidiPosition;

		if (tkRelTime < tkMinRelTime)
		{
			tkMinRelTime = tkRelTime;
			pTrkFound = pTrk;
		}
	}

	if (!pTrkFound)
	{
		SmfFlags |= SMFF_Eof;
		return RES_EndOfFile;
	}

	pTrk = pTrkFound;

	pTrk->ImagePtr += (dwGot = GetVDword(pTrk->ImagePtr, pTrk->BytesLeft, tkEventDelta));
	pTrk->BytesLeft -= dwGot;

	//	We MUST have at least three bytes here (cause we haven't hit
	// the end-of-track meta yet, which is three bytes long). Checking
	// against three means we don't have to check how much is left
	// in the track again for any short event, which is most cases.
	if (pTrk->BytesLeft < 3)
	{
		return RES_InvalidFile;
	}

	pTrk->Position += tkEventDelta;
	Event.Delta = pTrk->Position - MidiPosition;
	MidiPosition = pTrk->Position;

	bEvent = *pTrk->ImagePtr++;

	if (MIDI_Msg > bEvent)
	{
		if (0 == pTrk->RunningStatus)
		{
			return RES_InvalidFile;
		}

		dwGotTotal = 1;
		Event.Event[0] = pTrk->RunningStatus;
		Event.Event[1] = bEvent;
		if (3 == ChanMsgLen[(pTrk->RunningStatus >> 4) & 0x0F])
		{
			Event.Event[2] = *pTrk->ImagePtr++;
			dwGotTotal++;
		}
	}
	else if (MIDI_SysEx > bEvent)
	{
		pTrk->RunningStatus = bEvent;

		dwGotTotal = 2;
		Event.Event[0] = bEvent;
		Event.Event[1] = *pTrk->ImagePtr++;
		if (3 == ChanMsgLen[(bEvent >> 4) & 0x0F])
		{
			Event.Event[2] = *pTrk->ImagePtr++;
			dwGotTotal++;
		}
	}
	else
	{
		pTrk->RunningStatus = 0;
		if (MIDI_Meta == bEvent)
		{
			Event.Event[0] = MIDI_Meta;
			if (MIDI_MetaEot == (Event.Event[1] = *pTrk->ImagePtr++))
			{
				pTrk->TrackFlags |= TRACKF_Eot;
			}

			dwGotTotal = 2;
		}
		else if (MIDI_SysEx == bEvent || MIDI_SysExEnd == bEvent)
		{
			Event.Event[0] = bEvent;
			dwGotTotal = 1;
		}
		else
		{
			return RES_InvalidFile;
		}

		if (0 == (dwGot = GetVDword(pTrk->ImagePtr, pTrk->BytesLeft - 2, cbEvent)))
		{
			return RES_InvalidFile;
		}

		pTrk->ImagePtr  += dwGot;
		dwGotTotal      += dwGot;

		if (dwGotTotal + cbEvent > pTrk->BytesLeft)
		{
			return RES_InvalidFile;
		}

		Event.ParmCount = cbEvent;
		Event.Parm = pTrk->ImagePtr;

		pTrk->ImagePtr += cbEvent;
		dwGotTotal += cbEvent;
	}

	pTrk->BytesLeft -= dwGotTotal;

	return RES_Success;
	unguard;
}

//==========================================================================
//
//	VMMSystemMidiDevice::GetVDword
//
//	Reads a variable length DWORD from the given file.
//
//==========================================================================

dword VMMSystemMidiDevice::GetVDword(const byte* ImagePtr, dword Left,
	dword& Out)
{
	guard(VMMSystemMidiDevice::GetVDword);
	byte		b;
	dword		NumUsed = 0;

	Out = 0;
	do
	{
		if (!Left)
		{
			return 0;
		}

		b = *ImagePtr++;
		Left--;
		NumUsed++;

		Out = (Out << 7) | (b & 0x7F);
	} while (b & 0x80);

	return NumUsed;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.15  2005/11/13 14:36:22  dj_jl
//	Moved common sound functions to main sound module.
//
//	Revision 1.14  2005/10/06 23:09:20  dj_jl
//	Some cleanup.
//	
//	Revision 1.13  2005/10/02 23:12:51  dj_jl
//	New Windows MIDI driver.
//	
//	Revision 1.12  2005/09/12 19:45:16  dj_jl
//	Created midi device class.
//	
//	Revision 1.11  2004/09/24 10:54:11  dj_jl
//	MinGW support.
//	
//	Revision 1.10  2002/11/16 17:14:22  dj_jl
//	Some changes for release.
//	
//	Revision 1.9  2002/08/24 14:49:20  dj_jl
//	Added MP3 test.
//	
//	Revision 1.8  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.7  2002/07/20 14:53:02  dj_jl
//	Got rid of warnings.
//	
//	Revision 1.6  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//	
//	Revision 1.5  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
