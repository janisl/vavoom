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
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

#define MAXHISTORY			32
#define MAX_LINES			1024
#define MAX_LINE_LENGTH		80

// TYPES -------------------------------------------------------------------

enum cons_state_t
{
 	cons_closed,
    cons_opening,
    cons_open,
    cons_closing
};

class FConsoleDevice : public FOutputDevice
{
public:
	void Serialise(const char* V, EName Event);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern bool				graphics_started;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

FConsoleDevice			Console;

FOutputDevice			*GCon = &Console;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TILine			c_iline = {"", 0};

static cons_state_t		consolestate = cons_closed;

static char				clines[MAX_LINES][MAX_LINE_LENGTH];
static int				num_lines = 0;
static int				first_line = 0;
static int				last_line = 0;

static char				c_history[MAXHISTORY][MAX_ILINE_LENGTH];
static int				c_history_last;
static int				c_history_size;
static int				c_history_current;

static float			cons_h = 0;

static VCvarF			con_height("con_height", "240", CVAR_Archive);
static VCvarF			con_speed("con_speed", "480", CVAR_Archive);

//	Autocomplete
static int				c_autocompleteIndex = -1;
static VStr				c_autocompleteString;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  C_Init
//
//  Console initialization
//
//==========================================================================

void C_Init()
{
    c_history_last = 0;
    c_history_size = 0;
}

//==========================================================================
//
//  C_Shutdown
//
//==========================================================================

void C_Shutdown()
{
	c_autocompleteString.Clean();
}

//==========================================================================
//
//  C_Start
//
//  Open console
//
//==========================================================================

void C_Start()
{
	MN_DeactivateMenu();
	if (consolestate == cons_closed)
	{
       	c_iline.Init();
        last_line = num_lines;
	}
	consolestate = cons_opening;
	c_history_current = -1;
	c_autocompleteIndex = -1;
}

//==========================================================================
//
//  C_StartFull
//
//==========================================================================

void C_StartFull()
{
	MN_DeactivateMenu();
	c_iline.Init();
	last_line = num_lines;
	consolestate = cons_open;
	c_history_current = -1;
	c_autocompleteIndex = -1;
	cons_h = 480.0;
}

//==========================================================================
//
//	ToggleConsole
//
//==========================================================================

COMMAND(ToggleConsole)
{
	C_Start();
}

//==========================================================================
//
//  C_Stop
//
//  Close console
//
//==========================================================================

void C_Stop()
{
	consolestate = cons_closing;
}

//==========================================================================
//
//  C_Stop
//
//  Close console
//
//==========================================================================

COMMAND(HideConsole)
{
	consolestate = cons_closing;
}

//==========================================================================
//
//	C_Active
//
//==========================================================================

bool C_Active()
{
	return (consolestate == cons_opening) || (consolestate == cons_open);
}

//==========================================================================
//
//  C_Drawer
//
//  Draws console
//
//==========================================================================

void C_Drawer()
{
	int 		y;
	int			i;

	// Scroll console up when closing
	if (consolestate == cons_closing)
	{
		cons_h -= con_speed * host_frametime;
		if (cons_h <= 0)
		{
			// Closed
			cons_h = 0;
			consolestate = cons_closed;
		}
	}

	// Scroll console down when opening
	if (consolestate == cons_opening)
	{
		cons_h += con_speed * host_frametime;
		if (cons_h >= con_height)
		{
			// Open
			cons_h = con_height;
			consolestate = cons_open;
		}
	}

	if (!consolestate)
	{
		return;
	}

	// Backbround
	Drawer->DrawConsoleBackground((int)(fScaleY * cons_h));

	T_SetFont(font_small);
	T_SetAlign(hleft, vtop);

	// Input line
	y = (int)cons_h - 10;
	T_DrawString8(4, y, ">");
	i = VStr::Length(c_iline.Data) - 37;
	if (i < 0)
		i = 0;
	T_DrawString8(12, y, c_iline.Data + i);
	T_DrawCursor();
	y -= 10;

	// Lines
	i = last_line;
	while ((y + 9 > 0) && i--)
	{
		T_DrawString8(4, y, clines[(i + first_line) % MAX_LINES]);
		y -= 9;
	}
}

//==========================================================================
//
//  C_Responder
//
//  Handles the events
//
//==========================================================================

bool C_Responder(event_t* ev)
{
	const char*	cp;
	VStr		str;
	int			i;
	bool		eat;

	//  Respond to events only when console is active
	if (!C_Active())
		return false;

	//	We are iterested only in key down events
	if (ev->type != ev_keydown)
		return false;

	switch (ev->data1)
	{
	// Close console
	case K_ESCAPE:
		if (consolestate != cons_open)
			return false;

	case '`':
		if (consolestate == cons_closing)
			C_Start();
		else
			C_Stop();
		return true;

	// Execute entered command
	case K_ENTER:
	case K_PADENTER:
		//	Print it
		GCon->Logf(">%s", c_iline.Data);

		//	Add to history
		c_history_last = (MAXHISTORY + c_history_last - 1) % MAXHISTORY;
		if (c_history_size < MAXHISTORY)
			c_history_size++;
		VStr::Cpy(c_history[c_history_last], c_iline.Data);
		c_history_current = -1;

		//	Add to command buffer
		GCmdBuf << c_iline.Data << "\n";

		//	Clear line
		c_iline.Init();
		c_autocompleteIndex = -1;
		return true;

	// Scroll lines up
	case K_PAGEUP:
		for (i = 0; i < (GInput->ShiftDown ? 1 : 5); i++)
		{
			if (last_line > 1)
			{
				last_line--;
			}
		}
		return true;

	// Scroll lines down
	case K_PAGEDOWN:
		for (i = 0; i < (GInput->ShiftDown ? 1 : 5); i++)
		{
			if (last_line < num_lines)
			{
				last_line++;
			}
		}
		return true;

	// Go to first line
	case K_HOME:
		last_line = 1;
		return true;

	// Go to last line
	case K_END:
		last_line = num_lines;
		return true;

	// Command history up
	case K_UPARROW:
		c_history_current++;
		c_iline.Init();
		if (c_history_current >= c_history_size)
		{
			c_history_current = c_history_size;
		}
		else
		{
			cp = c_history[(c_history_last +
				c_history_current) % MAXHISTORY];
			while (*cp) c_iline.AddChar(*cp++);
		}
		c_autocompleteIndex = -1;
		return true;

	// Command history down
	case K_DOWNARROW:
		c_history_current--;
		c_iline.Init();
		if (c_history_current < 0)
		{
			c_history_current = -1;
		}
		else
		{
			cp = c_history[(c_history_last +
				c_history_current) % MAXHISTORY];
			while (*cp) c_iline.AddChar(*cp++);
		}
		c_autocompleteIndex = -1;
		return true;

	// Auto complete
	case K_TAB:
		if (!c_iline.Data[0])
			return true;

		if (c_autocompleteIndex == -1)
		{
			c_autocompleteString = c_iline.Data;
		}
		str = VCommand::GetAutoComplete(c_autocompleteString,
			c_autocompleteIndex, GInput->ShiftDown);
		if (str)
		{
			c_iline.Init();
			for (i = 0; i < (int)str.Length(); i++)
				c_iline.AddChar(str[i]);
			c_iline.AddChar(' ');
		}
		return true;

	// Add character to input line
	default:
		eat = c_iline.Key((byte)ev->data1);
		if (eat)
			c_autocompleteIndex = -1;
		return eat;
	}
}

//==========================================================================
//
//  Cls_f
//
//==========================================================================

COMMAND(Cls)
{
	num_lines = 0;
	first_line = 0;
	last_line = 0;
}

//==========================================================================
//
//  AddLine
//
//  Ads a line to console strings
//
//==========================================================================

static void AddLine(char* Data)
{
	if (num_lines >= MAX_LINES)
	{
		num_lines--;
		first_line++;
	}
	VStr::NCpy(clines[(num_lines + first_line) % MAX_LINES], Data, MAX_LINE_LENGTH);
	clines[(num_lines + first_line) % MAX_LINES][MAX_LINE_LENGTH - 1] = 0;
	num_lines++;
	if (last_line == num_lines - 1)
	{
		last_line = num_lines;
	}
}

//==========================================================================
//
//  DoPrint
//
//==========================================================================

static char	cpbuf[MAX_LINE_LENGTH];
static int	cpbuflen = 0;

static void DoPrint(const char *buf)
{
	const char	*ch;
	const char	*p;
	int			wlen;

#ifndef _WIN32
	if (!graphics_started)
		printf("%s", buf);
#endif

	ch = buf;
	while (*ch)
	{
		if (*ch == '\n')
		{
			cpbuf[cpbuflen] = 0;
			AddLine(cpbuf);
			cpbuflen = 0;
			ch++;
		}
		else if (*ch > ' ')
		{
			//  Count word length
			p = ch;
			wlen = 0;
			while (*p > ' ')
			{
				wlen++;
				p++;
			}

			if (cpbuflen + wlen >= MAX_LINE_LENGTH)
			{
				if (cpbuflen)
				{
					//	Word too long and it is not a first word
					//	Add current buffer and try again
					cpbuf[cpbuflen] = 0;
					AddLine(cpbuf);
					cpbuflen = 0;
				}
				else
				{
					//	A very long word
					VStr::NCpy(cpbuf, ch, MAX_LINE_LENGTH - 1);
					cpbuf[MAX_LINE_LENGTH - 1] = 0;
					AddLine(cpbuf);
					ch += MAX_LINE_LENGTH - 1;
				}
			}
			else
			{
				//	Add word to buffer
				while (*ch > ' ')
				{
					cpbuf[cpbuflen++] = *ch++;
				}
			}
		}
		else
		{
			//	Whitespace symbol
			cpbuf[cpbuflen++] = *ch;
			if (cpbuflen >= MAX_LINE_LENGTH)
			{
				cpbuf[MAX_LINE_LENGTH - 1] = 0;
				AddLine(cpbuf);
				cpbuflen = 0;
			}
			ch++;
		}
	}
}

//==========================================================================
//
//	FConsoleDevice::Serialise
//
//==========================================================================

void FConsoleDevice::Serialise(const char* V, EName Event)
{
	dprintf("%s: %s\n", VName::SafeString(Event), V);
	if (Event == NAME_Dev && !developer)
	{
		return;
	}
	DoPrint(V);
	DoPrint("\n");
}

//**************************************************************************
//**************************************************************************

#define NUM_NOTIFY_LINES		5
#define MAX_NOTIFY_LINE_LENGTH	80

static char				notify_lines[NUM_NOTIFY_LINES][MAX_NOTIFY_LINE_LENGTH];
static double			notify_times[NUM_NOTIFY_LINES];
static int				num_notify = 0;
static int				first_notify = 0;
static VCvarF			notify_time("notify_time", "5", CVAR_Archive);
static VCvarI			msg_echo("msg_echo", "1", CVAR_Archive);

//==========================================================================
//
//	C_ClearNotify
//
//==========================================================================

void C_ClearNotify()
{
	num_notify = 0;
	first_notify = 0;
}

//==========================================================================
//
//	C_NotifyMessage
//
//==========================================================================

void C_NotifyMessage(const char *str)
{
	if (msg_echo)
	{
		GCon->Log(str);
	}

	if (num_notify >= NUM_NOTIFY_LINES)
	{
		num_notify--;
		first_notify++;
	}
	VStr::NCpy(notify_lines[(num_notify + first_notify) % NUM_NOTIFY_LINES],
		str, MAX_NOTIFY_LINE_LENGTH - 1);
	notify_lines[(num_notify + first_notify) % NUM_NOTIFY_LINES][
		MAX_NOTIFY_LINE_LENGTH - 1] = 0;
	notify_times[(num_notify + first_notify) % NUM_NOTIFY_LINES] =
		host_time + notify_time;
	num_notify++;
}

//==========================================================================
//
//	C_DrawNotify
//
//==========================================================================

void C_DrawNotify()
{
	T_SetFont(font_small);
	T_SetAlign(hleft, vtop);

	//  Notify lines
	int y = 0;
	int i = 0;
	while (i < num_notify)
	{
		if (notify_times[(i + first_notify) % NUM_NOTIFY_LINES] < host_time)
		{
			first_notify++;
			num_notify--;
		}
		else
		{
			int lp = T_DrawTextW(4, y, notify_lines[(i + first_notify) % NUM_NOTIFY_LINES], 640);
			y += 9 * lp;
			i++;
		}
	}
}

//**************************************************************************
//**************************************************************************

static char				centre_message[256];
static float			centre_time;
static VCvarF			centre_msg_time("centre_message_time", "7", CVAR_Archive);

//==========================================================================
//
//	C_CentreMessage
//
//==========================================================================

void C_CentreMessage(const char *msg)
{
	VStr::Cpy(centre_message, msg);
	centre_time = centre_msg_time;
}

//==========================================================================
//
//	C_DrawCentreMessage
//
//==========================================================================

void C_DrawCentreMessage()
{
	if (centre_time)
	{
		T_SetFont(font_small);
	    T_SetAlign(hcentre, vcentre);
		T_DrawTextW(320, 360, centre_message, 600);
		centre_time -= host_frametime;
		if (centre_time < 0.0)
		{
			centre_time = 0.0;
		}
	}
}
