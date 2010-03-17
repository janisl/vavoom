/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

	 This program is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timidity.h"

namespace LibTimidity
{

int free_instruments_afterwards=1;
char def_instr_name[256]="";

ControlMode*	ctl;

#define MAXWORDS 10

static int read_config_file(const char* name)
{
	FILE* fp;
	char tmp[1024], *w[MAXWORDS], *cp;
	ToneBank *bank=0;
	int i, j, k, line = 0, words;
	static int rcf_count = 0;

	if (rcf_count > 50)
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
			"Probable source loop in configuration files");
		return (-1);
	}

	if (!(fp = open_file(name, 1, OF_VERBOSE)))
		return -1;

	while (fgets(tmp, sizeof(tmp), fp))
	{
		line++;
		w[words=0] = strtok(tmp, " \t\240");
		if (!w[0])
			continue;

		/* Originally the TiMidity++ extensions were prefixed like this */
		if (strcmp(w[0], "#extension") == 0)
			words = -1;
		else if (*w[0] == '#')
			continue;

		while (w[words] && *w[words] != '#' && (words < MAXWORDS))
			w[++words]=strtok(0," \t\240");

		/*
		* TiMidity++ adds a number of extensions to the config file format.
		* Many of them are completely irrelevant to SDL_sound, but at least
		* we shouldn't choke on them.
		*
		* Unfortunately the documentation for these extensions is often quite
		* vague, gramatically strange or completely absent.
		*/
		if (
				!strcmp(w[0], "comm")      /* "comm" program second        */
			|| !strcmp(w[0], "HTTPproxy") /* "HTTPproxy" hostname:port    */
			|| !strcmp(w[0], "FTPproxy")  /* "FTPproxy" hostname:port     */
			|| !strcmp(w[0], "mailaddr")  /* "mailaddr" your-mail-address */
			|| !strcmp(w[0], "opt")       /* "opt" timidity-options       */
			)
		{
			/*
			* + "comm" sets some kind of comment -- the documentation is too
			*   vague for me to understand at this time.
			* + "HTTPproxy", "FTPproxy" and "mailaddr" are for reading data
			*   over a network, rather than from the file system.
			* + "opt" specifies default options for TiMidity++.
		*
			* These are all quite useless for our version of TiMidity, so
			* they can safely remain no-ops.
			*/
		}
		else if (!strcmp(w[0], "timeout")) /* "timeout" program second */
		{
			/*
			* Specifies a timeout value of the program. A number of seconds
			* before TiMidity kills the note. This may be useful to implement
			* later, but I don't see any urgent need for it.
			*/
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL,"FIXME: Implement \"timeout\" in TiMidity config.\n");
		}
		else if (!strcmp(w[0], "copydrumset")  /* "copydrumset" drumset */
				|| !strcmp(w[0], "copybank")) /* "copybank" bank       */
		{
			/*
			* Copies all the settings of the specified drumset or bank to
			* the current drumset or bank. May be useful later, but not a
			* high priority.
			*/
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL,"FIXME: Implement \"%s\" in TiMidity config.\n", w[0]);
		}
		else if (!strcmp(w[0], "undef")) /* "undef" progno */
		{
			/*
			* Undefines the tone "progno" of the current tone bank (or
			* drum set?). Not a high priority.
			*/
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL,"FIXME: Implement \"undef\" in TiMidity config.\n");
		}
		else if (!strcmp(w[0], "altassign")) /* "altassign" prog1 prog2 ... */
		{
			/*
			* Sets the alternate assign for drum set. Whatever that's
			* supposed to mean.
			*/
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL,"FIXME: Implement \"altassign\" in TiMidity config.\n");
		}
		else if (!strcmp(w[0], "soundfont")
			|| !strcmp(w[0], "font"))
		{
			/*
			* I can't find any documentation for these, but I guess they're
			* an alternative way of loading/unloading instruments.
			* 
			* "soundfont" sf_file "remove"
			* "soundfont" sf_file ["order=" order] ["cutoff=" cutoff]
			*                     ["reso=" reso] ["amp=" amp]
			* "font" "exclude" bank preset keynote
			* "font" "order" order bank preset keynote
			*/
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL,"FIXME: Implmement \"%s\" in TiMidity config.\n", w[0]);
		}
		else if (!strcmp(w[0], "progbase"))
		{
			/*
			* The documentation for this makes absolutely no sense to me, but
			* apparently it sets some sort of base offset for tone numbers.
			* Why anyone would want to do this is beyond me.
			*/
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL,"FIXME: Implement \"progbase\" in TiMidity config.\n");
		}
		else if (!strcmp(w[0], "map")) /* "map" name set1 elem1 set2 elem2 */
		{
			/*
			* This extension is the one we will need to implement, as it is
			* used by the "eawpats". Unfortunately I cannot find any
			* documentation whatsoever for it, but it looks like it's used
			* for remapping one instrument to another somehow.
			*/
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL,"FIXME: Implement \"map\" in TiMidity config.\n");
		}

			/* Standard TiMidity config */

		else if (!strcmp(w[0], "dir"))
		{
			if (words < 2)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
					"%s: line %d: No directory given\n", name, line);
				return -2;
			}
			for (i = 1; i < words; i++)
				add_to_pathlist(w[i]);
		}
		else if (!strcmp(w[0], "source"))
		{
			if (words < 2)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
					"%s: line %d: No file name given\n", name, line);
				return -2;
			}
			for (i = 1; i < words; i++)
			{
				rcf_count++;
				read_config_file(w[i]);
				rcf_count--;
			}
		}
		else if (!strcmp(w[0], "default"))
		{
			if (words != 2)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
					"%s: line %d: Must specify exactly one patch name\n",
					name, line);
				return -2;
			}
			strncpy(def_instr_name, w[1], 255);
			def_instr_name[255] = '\0';
		}
		else if (!strcmp(w[0], "drumset"))
		{
			if (words < 2)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
					"%s: line %d: No drum set number given\n", 
					name, line);
				return -2;
			}
			i = atoi(w[1]);
			if (i < 0 || i > 127)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
					"%s: line %d: Drum set must be between 0 and 127\n",
					name, line);
				return -2;
			}
			if (!master_drumset[i])
			{
				master_drumset[i] = (ToneBank*)safe_malloc(sizeof(ToneBank));
				memset(master_drumset[i], 0, sizeof(ToneBank));
			}
			bank = master_drumset[i];
		}
		else if (!strcmp(w[0], "bank"))
		{
			if (words < 2)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
					"%s: line %d: No bank number given\n", 
					name, line);
				return -2;
			}
			i = atoi(w[1]);
			if (i < 0 || i > 127)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
					"%s: line %d: Tone bank must be between 0 and 127\n",
					name, line);
				return -2;
			}
			if (!master_tonebank[i])
			{
				master_tonebank[i] = (ToneBank*)safe_malloc(sizeof(ToneBank));
				memset(master_tonebank[i], 0, sizeof(ToneBank));
			}
			bank = master_tonebank[i];
		}
		else
		{
			if ((words < 2) || (*w[0] < '0' || *w[0] > '9'))
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
					"%s: line %d: syntax error\n", name, line);
				return -2;
			}
			i = atoi(w[0]);
			if (i < 0 || i > 127)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
					"%s: line %d: Program must be between 0 and 127\n",
					name, line);
				return -2;
			}
			if (!bank)
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
					"%s: line %d: Must specify tone bank or drum set "
					"before assignment\n",
					name, line);
				return -2;
			}
			if (bank->tone[i].name)
				free(bank->tone[i].name);
			strcpy((bank->tone[i].name = (char*)safe_malloc(strlen(w[1]) + 1)), w[1]);
			bank->tone[i].note = bank->tone[i].amp = bank->tone[i].pan =
				bank->tone[i].strip_loop = bank->tone[i].strip_envelope =
				bank->tone[i].strip_tail = -1;

			for (j = 2; j < words; j++)
			{
				if (!(cp = strchr(w[j], '=')))
				{
					ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: line %d: bad patch option %s\n",
						name, line, w[j]);
					return -2;
				}
				*cp++ = 0;
				if (!strcmp(w[j], "amp"))
				{
					k = atoi(cp);
					if ((k < 0 || k > MAX_AMPLIFICATION) || (*cp < '0' || *cp > '9'))
					{
						ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
							"%s: line %d: amplification must be between "
							"0 and %d\n", name, line, MAX_AMPLIFICATION);
						return -2;
					}
					bank->tone[i].amp = k;
				}
				else if (!strcmp(w[j], "note"))
				{
					k = atoi(cp);
					if ((k < 0 || k > 127) || (*cp < '0' || *cp > '9'))
					{
						ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
							"%s: line %d: note must be between 0 and 127\n",
							name, line);
						return -2;
					}
					bank->tone[i].note = k;
				}
				else if (!strcmp(w[j], "pan"))
				{
					if (!strcmp(cp, "center"))
						k = 64;
					else if (!strcmp(cp, "left"))
						k = 0;
					else if (!strcmp(cp, "right"))
						k = 127;
					else
						k = ((atoi(cp) + 100) * 100) / 157;
					if ((k < 0 || k > 127) ||
						(k==0 && *cp!='-' && (*cp < '0' || *cp > '9')))
					{
						ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
							"%s: line %d: panning must be left, right, "
							"center, or between -100 and 100\n",
							name, line);
						return -2;
					}
					bank->tone[i].pan = k;
				}
				else if (!strcmp(w[j], "keep"))
				{
					if (!strcmp(cp, "env"))
						bank->tone[i].strip_envelope = 0;
					else if (!strcmp(cp, "loop"))
						bank->tone[i].strip_loop = 0;
					else
					{
						ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
							"%s: line %d: keep must be env or loop\n", name, line);
						return -2;
					}
				}
				else if (!strcmp(w[j], "strip"))
				{
					if (!strcmp(cp, "env"))
						bank->tone[i].strip_envelope = 1;
					else if (!strcmp(cp, "loop"))
						bank->tone[i].strip_loop = 1;
					else if (!strcmp(cp, "tail"))
						bank->tone[i].strip_tail = 1;
					else
					{
						ctl->cmsg(CMSG_ERROR, VERB_NORMAL,
							"%s: line %d: strip must be env, loop, or tail\n",
							name, line);
						return -2;
					}
				}
				else
				{
					ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: line %d: bad patch option %s\n",
						name, line, w[j]);
					return -2;
				}
			}
		}
	}
	if (ferror(fp))
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "Can't read from %s\n", name);
		close_file(fp);
		return -2;
	}
	close_file(fp);
	return 0;
}

int Timidity_Init()
{
	/* !!! FIXME: This may be ugly, but slightly less so than requiring the
	*            default search path to have only one element. I think.
	*
	*            We only need to include the likely locations for the config
	*            file itself since that file should contain any other directory
	*            that needs to be added to the search path.
	*/
#ifdef WIN32
	add_to_pathlist("\\TIMIDITY");
#else
	add_to_pathlist("/usr/local/lib/timidity");
	add_to_pathlist("/etc");
#endif

	return read_config_file(CONFIG_FILE);
}

};
