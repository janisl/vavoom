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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:53  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

#define MAXHISTORY			32
#define MAX_LINES			1024
#define MAX_LINE_LENGTH		40
#define NUM_NOTIFY_LINES	5

// TYPES -------------------------------------------------------------------

class TConBuf : public streambuf
{
 public:
	TConBuf(bool dev_buf)
	{
		dev_only = dev_buf;
	}

	int sync();
	int overflow(int ch);

	bool		dev_only;
};
     
// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void T_DrawString8(int x, int y, char* String);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean				graphics_started;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

static TConBuf			cbuf(false);
static TConBuf			cdbuf(true);

ostream					con(&cbuf);
ostream					cond(&cdbuf);

cons_state_t			consolestate = cons_closed;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TILine			c_iline = {"", 0, 0};

static char				clines[MAX_LINES][MAX_LINE_LENGTH];
static int				num_lines = 0;
static int				first_line = 0;
static int				last_line = 0;

static char				c_history[MAXHISTORY][MAX_ILINE_LENGTH];
static int				c_history_last;
static int				c_history_size;
static int				c_history_current;

static float			cons_h = 0;

static TCvarF			con_height("con_height", "100", CVAR_ARCHIVE);
static TCvarF			con_speed("con_speed", "200", CVAR_ARCHIVE);

//	Autocomplete
static int				c_autocompleteIndex = -1;
static char				c_autocompleteString[MAX_ILINE_LENGTH];

static const char**		c_ac_Table = NULL;
static int				c_ac_Count = 0;

static char				notify_lines[NUM_NOTIFY_LINES][MAX_LINE_LENGTH];
static double			notify_times[NUM_NOTIFY_LINES];
static int				num_notify = 0;
static int				first_notify = 0;
static TCvarF			notify_time("notify_time", "5", CVAR_ARCHIVE);

static char				center_message[256];
static float			center_time;
static TCvarF			center_msg_time("center_message_time", "7", CVAR_ARCHIVE);

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
//	C_AddToAutoComplete
//
//==========================================================================

void C_AddToAutoComplete(const char* string)
{
	int			i;
	const char*	Swap;

#ifdef PARANOID
	for (i=0; i<c_ac_Count; i++)
    {
    	if (!stricmp(string, c_ac_Table[i]))
	    	I_Error("C_AddToAutoComplete: %s is allready registered.", string);
	}
#endif

	if (!c_ac_Count)
    {
		c_ac_Table = (const char**)Z_StrMalloc(128 * sizeof(char*));
    }
    else if (c_ac_Count >= 128)
    {
		Z_Resize((void**)&c_ac_Table, (c_ac_Count + 1) * sizeof(char*));
	}
	c_ac_Table[c_ac_Count] = string;

    // Alphabetic sort
	for (i = c_ac_Count; i && (stricmp(c_ac_Table[i - 1], c_ac_Table[i]) > 0); i--)
	{
		Swap = c_ac_Table[i];
		c_ac_Table[i] = c_ac_Table[i - 1];
		c_ac_Table[i - 1] = Swap;
	}
	c_ac_Count++;
}

//==========================================================================
//
//  C_Start
//
//  Open console
//
//==========================================================================

void C_Start(void)
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
    C_ClearNotify();
}

//==========================================================================
//
//  C_StartFull
//
//==========================================================================

void C_StartFull(void)
{
	MN_DeactivateMenu();
   	c_iline.Init();
	last_line = num_lines;
	consolestate = cons_open;
	c_history_current = -1;
	c_autocompleteIndex = -1;
    C_ClearNotify();
   	cons_h = 200.0;
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

void C_Stop(void)
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
//  C_Drawer
//
//  Draws console
//
//==========================================================================

void C_Drawer(void)
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

	if (center_time)
	{
		T_SetFont(font_small);
	    T_SetAlign(hcenter, vcenter);
		T_DrawText(160, 100, center_message);
		center_time -= host_frametime;
		if (center_time < 0.0)
		{
			center_time = 0.0;
		}
	}

	if (!consolestate)
    {
    	if (cls.state == ca_connected && !cl.intermission)
        {
			T_SetFont(font_small);
		    T_SetAlign(hleft, vtop);

		    //  Notify lines
	        y = 0;
		    i = 0;
		    while (i < num_notify)
			{
				if (notify_times[(i + first_notify) % NUM_NOTIFY_LINES] < host_time)
		        {
		        	first_notify++;
		            num_notify--;
		        }
		        else
		        {
					T_DrawString8(4, y, notify_lines[(i + first_notify) % NUM_NOTIFY_LINES]);
			       	y += 9;
		            i++;
		        }
			}
		}
    	return;
    }

    // Backbround
	Drawer->DrawConsoleBackground((int)(fScaleY * cons_h));

	T_SetFont(font_small);
    T_SetAlign(hleft, vtop);

    // Input line
    y = (int)cons_h - 10;
	T_DrawString8(4, y, ">");
    i = strlen(c_iline.Data) - 37;
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

boolean C_Responder(event_t* ev)
{
    const char*	cp;
    int			i;
	bool		eat;

	//  Respond to events only when console is active
    if (!consolestate)
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
   		//	Print it
		con << ">" << c_iline.Data << "\n";

		//	Add to history
        c_history_last = (MAXHISTORY + c_history_last - 1) % MAXHISTORY;
        if (c_history_size < MAXHISTORY)
           	c_history_size++;
        strcpy(c_history[c_history_last], c_iline.Data);
        c_history_current = -1;

		//	Add to command buffer
        CmdBuf << c_iline.Data << "\n";

		//	Clear line
		c_iline.Init();
		c_autocompleteIndex = -1;
        return true;

     // Scroll lines up
     case K_PAGEUP:
        if (last_line > 1)
		{
			last_line--;
		}
       	return true;

     // Scroll lines down
     case K_PAGEDOWN:
		if (last_line < num_lines)
		{
           	last_line++;
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
			strcpy(c_autocompleteString, c_iline.Data);
           	if (shiftdown)
				i = c_ac_Count - 1;
			else
				i = 0;
		}
		else
		{
           	if (shiftdown)
				i = c_autocompleteIndex - 1;
			else
				i = c_autocompleteIndex + 1;
		}

		while ((i < c_ac_Count) && (i >= 0))
		{
            if (strlen(c_autocompleteString) <= strlen(c_ac_Table[i]) &&
            	!strnicmp(c_autocompleteString, c_ac_Table[i], strlen(c_autocompleteString)))
			{
				c_autocompleteIndex = i;
				c_iline.Init();
               	cp = c_ac_Table[i];
                while (*cp)
                	c_iline.AddChar(*cp++);
				c_iline.AddChar(' ');
				break;
			}
			if (shiftdown) i--; else i++;
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
//	C_ClearNotify
//
//==========================================================================

void C_ClearNotify(void)
{
	num_notify = 0;
	first_notify = 0;
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
    strncpy(clines[(num_lines + first_line) % MAX_LINES], Data, MAX_LINE_LENGTH);
    clines[(num_lines + first_line) % MAX_LINES][MAX_LINE_LENGTH - 1] = 0;
    num_lines++;
	last_line = num_lines;

    if (!consolestate/* && !MN_Active()*/)
    {
		if (num_notify >= NUM_NOTIFY_LINES)
	    {
	    	num_notify--;
	        first_notify++;
		}
	    strcpy(notify_lines[(num_notify + first_notify) % NUM_NOTIFY_LINES],
	    	clines[(num_lines + first_line - 1) % MAX_LINES]);
		notify_times[(num_notify + first_notify) % NUM_NOTIFY_LINES] =
			host_time + notify_time;
	    num_notify++;
	}
}

//==========================================================================
//
//	C_CenterMessage
//
//==========================================================================

void C_CenterMessage(const char *msg)
{
	strcpy(center_message, msg);
	center_time = center_msg_time;
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
                    strncpy(cpbuf, ch, MAX_LINE_LENGTH - 1);
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
//  AddChar
//
//==========================================================================

static char		con_print_buf[1024];
static int		print_buf_len = 0;

static void AddChar(char ch)
{
	con_print_buf[print_buf_len] = ch;
	print_buf_len++;

	if (ch <= ' ')
	{
        con_print_buf[print_buf_len] = 0;
	    DoPrint(con_print_buf);
        print_buf_len = 0;
	}
}

//==========================================================================
//
//  TConBuf::sync
//
//==========================================================================

int TConBuf::sync()
{
	return 0;
}
     
//==========================================================================
//
//  TConBuf::overflow
//
//==========================================================================

int TConBuf::overflow(int ch)
{
	//	All messages to debug file, even without developer being set to 1
	dprintf("%c", (char)ch);
	if (ch != EOF && (!dev_only || (int)developer))
	{
    	AddChar(ch);
	}
	return 0;
}

