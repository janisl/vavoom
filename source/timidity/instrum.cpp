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

   instrum.c 
   
   Code to load and unload GUS-compatible instrument patches.

*/

#include <string.h>
#include <stdlib.h>

#include "timidity.h"

namespace LibTimidity
{

#include "gf1.h"

#ifdef FAST_DECAY
int fast_decay=1;
#else
int fast_decay=0;
#endif


static void free_instrument(Instrument* ip)
{
	if (!ip)
	{
		return;
	}

	for (int i = 0; i < ip->samples; i++)
	{
		Sample* sp = &(ip->sample[i]);
		if (sp->data)
		{
			free(sp->data);
			sp->data = NULL;
		}
	}
	free(ip->sample);
	ip->sample = NULL;
	free(ip);
	ip = NULL;
}

static void free_bank(MidiSong* song, int dr, int b)
{
	ToneBank* bank = ((dr) ? song->drumset[b] : song->tonebank[b]);
	for (int i = 0; i < 128; i++)
	{
		if (bank->instrument[i])
		{
			/* Not that this could ever happen, of course */
			if (bank->instrument[i] != MAGIC_LOAD_INSTRUMENT)
			{
				free_instrument(bank->instrument[i]);
				bank->instrument[i] = NULL;
			}
		}
		if (bank->tone[i].name)
		{
			free(bank->tone[i].name);
			bank->tone[i].name = NULL;
		}
	}
}


static int32 convert_envelope_rate(uint8 rate)
{
	int32 r = 3 - ((rate >> 6) & 0x3);
	r *= 3;
	r = (int32)(rate & 0x3f) << r; /* 6.9 fixed point */

	/* 15.15 fixed point. */
	return ((r * 44100) / CONTROLS_PER_SECOND) << ((fast_decay) ? 10 : 9);
}

static int32 convert_envelope_offset(uint8 offset)
{
	/* This is not too good... Can anyone tell me what these values mean?
		Are they GUS-style "exponential" volumes? And what does that mean? */

	/* 15.15 fixed point */
	return offset << (7 + 15);
}

static int32 convert_tremolo_sweep(uint8 sweep)
{
	if (!sweep)
	{
		return 0;
	}
	return (SWEEP_TUNING << SWEEP_SHIFT) / (CONTROLS_PER_SECOND * sweep);
}

static int32 convert_vibrato_sweep(uint8 sweep, int32 vib_control_ratio)
{
	if (!sweep)
	{
		return 0;
	}
	return (int32) (FSCALE((double) (vib_control_ratio) * SWEEP_TUNING, SWEEP_SHIFT) /
		(double)(OUTPUT_RATE * sweep));

	/* this was overflowing with seashore.pat

	((vib_control_ratio * SWEEP_TUNING) << SWEEP_SHIFT) /
	(play_mode->rate * sweep); */
}

static int32 convert_tremolo_rate(uint8 rate)
{
	return ((SINE_CYCLE_LENGTH * rate) << RATE_SHIFT) /
		(TREMOLO_RATE_TUNING * CONTROLS_PER_SECOND);
}

static int32 convert_vibrato_rate(uint8 rate)
{
	/* Return a suitable vibrato_control_ratio value */
	return (VIBRATO_RATE_TUNING * OUTPUT_RATE) /
		(rate * 2 * VIBRATO_SAMPLE_INCREMENTS);
}

static void reverse_data(int16* sp, int32 ls, int32 le)
{
	int16* ep = sp + le;
	sp += ls;
	le -= ls;
	le /= 2;
	while (le--)
	{
		int16 s = *sp;
		*sp++ = *ep;
		*ep-- = s;
	}
}

/* 
   If panning or note_to_use != -1, it will be used for all samples,
   instead of the sample-specific values in the instrument file. 

   For note_to_use, any value <0 or >127 will be forced to 0.
 
   For other parameters, 1 means yes, 0 means no, other values are
   undefined.

   TODO: do reverse loops right */
static Instrument* load_instrument(const char *name, int percussion,
	int panning, int amp, int note_to_use,
	int strip_loop, int strip_envelope,
	int strip_tail)
{
	Instrument *ip;
	FILE *fp;
	int i,j,noluck=0;

	if (!name)
		return 0;

	/* Open patch file */
	if ((fp=open_file(name, 1, OF_NORMAL)) == NULL)
	{
		noluck = 1;
		if (strlen(name) + strlen(".pat") < 1024)
		{
			char path[1024];
			strcpy(path, name);
			strcat(path, ".pat");
			if ((fp = open_file(path, 1, OF_NORMAL)) != NULL)
			{
				noluck = 0;
			}
		}
	}

	if (noluck)
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "Instrument `%s' can't be found.", name);
		return 0;
	}

	/*ctl->cmsg(CMSG_INFO, VERB_NOISY, "Loading instrument %s", current_filename);*/

	/* Read some headers and do cursory sanity checks. */
	GF1PatchHeader PatchHdr;
	GF1InstrumentHeader InstrHdr;
	GF1LayerHeader LayerHdr;
	if (1 != fread(&PatchHdr, sizeof(PatchHdr), 1, fp) ||
		1 != fread(&InstrHdr, sizeof(InstrHdr), 1, fp) ||
		1 != fread(&LayerHdr, sizeof(LayerHdr), 1, fp))
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: Failed to read headers", name);
		return 0;
	}

	if ((memcmp(PatchHdr.Magic, GF1_MAGIC1, 12) &&
		memcmp(PatchHdr.Magic, GF1_MAGIC2, 12)) ||
		memcmp(PatchHdr.Id, GF1_PATCH_ID, 10))
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: not an instrument", name);
		return 0;
	}

	//	instruments. To some patch makers, 0 means 1
	if (PatchHdr.NumInstruments != 1 && PatchHdr.NumInstruments != 0) 
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
			"Can't handle patches with %d instruments", PatchHdr.NumInstruments);
		return 0;
	}

	//	layers. What's a layer?
	if (InstrHdr.NumLayers != 1 && InstrHdr.NumLayers != 0)
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
			"Can't handle instruments with %d layers", InstrHdr.NumLayers);
		return 0;
	}

	if (LayerHdr.NumSamples == 0)
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "Instrument has 0 samples");
		return 0;
	}

	ip = (Instrument *)safe_malloc(sizeof(Instrument));
	ip->type = INST_GUS;
	ip->samples = LayerHdr.NumSamples;
	ip->sample = (Sample*)safe_malloc(sizeof(Sample) * ip->samples);
	memset(ip->sample, 0, sizeof(Sample) * ip->samples);
	for (i = 0; i < ip->samples; i++)
	{
		GF1SampleHeader SmplHdr;

		if (1 != fread(&SmplHdr, sizeof(GF1SampleHeader), 1, fp))
		{
fail:
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "Error reading sample %d", i);
			free_instrument(ip);
			return 0;
		}

		Sample* sp = &ip->sample[i];

		sp->data_length = LE_LONG(SmplHdr.DataLength);
		sp->loop_start = LE_LONG(SmplHdr.LoopStart);
		sp->loop_end = LE_LONG(SmplHdr.LoopEnd);
		sp->sample_rate = LE_SHORT(SmplHdr.SampleRate);
		sp->low_freq = LE_LONG(SmplHdr.LowFreq);
		sp->high_freq = LE_LONG(SmplHdr.HighFreq);
		sp->root_freq = LE_LONG(SmplHdr.RootFreq);
		sp->low_vel = 0;
		sp->high_vel = 127;

		if (panning == -1)
		{
			sp->panning = (SmplHdr.Panning * 8 + 4) & 0x7f;
		}
		else
		{
			sp->panning = (uint8)(panning & 0x7F);
		}

		if (!SmplHdr.TremoloRate || !SmplHdr.TremoloDepth)
		{
			sp->tremolo_sweep_increment =
				sp->tremolo_phase_increment = sp->tremolo_depth = 0;
			ctl->cmsg(CMSG_INFO, VERB_DEBUG, " * no tremolo");
		}
		else
		{
			sp->tremolo_sweep_increment =
				convert_tremolo_sweep(SmplHdr.TremoloSweep);
			sp->tremolo_phase_increment =
				convert_tremolo_rate(SmplHdr.TremoloRate);
			sp->tremolo_depth = SmplHdr.TremoloDepth;
			ctl->cmsg(CMSG_INFO, VERB_DEBUG,
				" * tremolo: sweep %d, phase %d, depth %d",
				sp->tremolo_sweep_increment, sp->tremolo_phase_increment,
				sp->tremolo_depth);
		}

		if (!SmplHdr.VibratoRate || !SmplHdr.VibratoDepth)
		{
			sp->vibrato_sweep_increment =
				sp->vibrato_control_ratio = sp->vibrato_depth = 0;
			ctl->cmsg(CMSG_INFO, VERB_DEBUG, " * no vibrato");
		}
		else
		{
			sp->vibrato_control_ratio =
				convert_vibrato_rate(SmplHdr.VibratoRate);
			sp->vibrato_sweep_increment =
				convert_vibrato_sweep(SmplHdr.VibratoSweep,
				sp->vibrato_control_ratio);
			sp->vibrato_depth = SmplHdr.VibratoDepth;
			ctl->cmsg(CMSG_INFO, VERB_DEBUG,
				" * vibrato: sweep %d, ctl %d, depth %d",
				sp->vibrato_sweep_increment, sp->vibrato_control_ratio,
				sp->vibrato_depth);
		}

		sp->modes = SmplHdr.Modes;

		/* Mark this as a fixed-pitch instrument if such a deed is desired. */
		if (note_to_use != -1)
			sp->note_to_use = (uint8)(note_to_use);
		else
			sp->note_to_use = 0;

		/* seashore.pat in the Midia patch set has no Sustain. I don't
			understand why, and fixing it by adding the Sustain flag to
			all looped patches probably breaks something else. We do it
			anyway. */

		if (sp->modes & MODES_LOOPING) 
			sp->modes |= MODES_SUSTAIN;

		/* Strip any loops and envelopes we're permitted to */
		if ((strip_loop==1) &&
			(sp->modes & (MODES_SUSTAIN | MODES_LOOPING |
				MODES_PINGPONG | MODES_REVERSE)))
		{
			ctl->cmsg(CMSG_INFO, VERB_DEBUG, " - Removing loop and/or sustain");
			sp->modes &=~(MODES_SUSTAIN | MODES_LOOPING | 
				MODES_PINGPONG | MODES_REVERSE);
		}

		if (strip_envelope==1)
		{
			if (sp->modes & MODES_ENVELOPE)
				ctl->cmsg(CMSG_INFO, VERB_DEBUG, " - Removing envelope");
			sp->modes &= ~MODES_ENVELOPE;
		}
		else if (strip_envelope != 0)
		{
			/* Have to make a guess. */
			if (!(sp->modes & (MODES_LOOPING | MODES_PINGPONG | MODES_REVERSE)))
			{
				/* No loop? Then what's there to sustain? No envelope needed
				either... */
				sp->modes &= ~(MODES_SUSTAIN|MODES_ENVELOPE);
				ctl->cmsg(CMSG_INFO, VERB_DEBUG, 
					" - No loop, removing sustain and envelope");
			}
			else if (!memcmp(SmplHdr.EnvelopeRate, "??????", 6) ||
				SmplHdr.EnvelopeOffset[5] >= 100) 
			{
				/* Envelope rates all maxed out? Envelope end at a high "offset"?
				That's a weird envelope. Take it out. */
				sp->modes &= ~MODES_ENVELOPE;
				ctl->cmsg(CMSG_INFO, VERB_DEBUG, 
					" - Weirdness, removing envelope");
			}
			else if (!(sp->modes & MODES_SUSTAIN))
			{
				/* No sustain? Then no envelope.  I don't know if this is
				justified, but patches without sustain usually don't need the
				envelope either... at least the Gravis ones. They're mostly
				drums.  I think. */
				sp->modes &= ~MODES_ENVELOPE;
				ctl->cmsg(CMSG_INFO, VERB_DEBUG, 
					" - No sustain, removing envelope");
			}
		}

		for (j = 0; j < 6; j++)
		{
			sp->envelope_rate[j] =
				convert_envelope_rate(SmplHdr.EnvelopeRate[j]);
			sp->envelope_offset[j] =
				convert_envelope_offset(SmplHdr.EnvelopeOffset[j]);
		}

		/* Then read the sample data */
		sp->data = (sample_t*)safe_malloc(sp->data_length);

		if (1 != fread(sp->data, sp->data_length, 1, fp))
			goto fail;

		if (!(sp->modes & MODES_16BIT)) /* convert to 16-bit data */
		{
			int32 i = sp->data_length;
			uint8 *cp = (uint8*)(sp->data);
			uint16 *tmp,*newdta;
			tmp = newdta = (uint16*)safe_malloc(sp->data_length * 2);
			while (i--)
				*tmp++ = (uint16)(*cp++) << 8;
			cp = (uint8*)(sp->data);
			sp->data = (sample_t*)newdta;
			free(cp);
			cp = NULL;
			sp->data_length *= 2;
			sp->loop_start *= 2;
			sp->loop_end *= 2;
		}
#ifndef LITTLE_ENDIAN
		else
		/* convert to machine byte order */
		{
			int32 i = sp->data_length / 2;
			int16 *tmp = (int16*)sp->data, s;
			while (i--)
			{
				s = LE_SHORT(*tmp);
				*tmp++ = s;
			}
		}
#endif

		if (sp->modes & MODES_UNSIGNED) /* convert to signed data */
		{
			int32 i = sp->data_length / 2;
			int16* tmp = (int16*)sp->data;
			while (i--)
				*tmp++ ^= 0x8000;
		}

		/* Reverse reverse loops and pass them off as normal loops */
		if (sp->modes & MODES_REVERSE)
		{
			int32 t;
			/* The GUS apparently plays reverse loops by reversing the
				whole sample. We do the same because the GUS does not SUCK. */

			ctl->cmsg(CMSG_WARNING, VERB_NORMAL, "Reverse loop in %s", name);
			reverse_data((int16*)sp->data, 0, sp->data_length / 2);

			t=sp->loop_start;
			sp->loop_start = sp->data_length - sp->loop_end;
			sp->loop_end = sp->data_length - t;

			sp->modes &= ~MODES_REVERSE;
			sp->modes |= MODES_LOOPING; /* just in case */
		}

		if (amp != -1)
			sp->volume = (float)((amp) / 100.0);
		else
		{
#ifdef ADJUST_SAMPLE_VOLUMES
			/* Try to determine a volume scaling factor for the sample.
				This is a very crude adjustment, but things sound more
				balanced with it. Still, this should be a runtime option. */
			int32 i = sp->data_length / 2;
			int16 maxamp = 0, a;
			int16* tmp = (int16*)sp->data;
			while (i--)
			{
				a = *tmp++;
				if (a < 0)
					a = -a;
				if (a > maxamp)
					maxamp = a;
			}
			sp->volume=(float)(32768.0 / maxamp);
			ctl->cmsg(CMSG_INFO, VERB_DEBUG, " * volume comp: %f", sp->volume);
#else
			sp->volume = 1.0;
#endif
		}

		sp->data_length /= 2; /* These are in bytes. Convert into samples. */

		sp->loop_start /= 2;
		sp->loop_end /= 2;

		/* Then fractional samples */
		sp->data_length <<= FRACTION_BITS;
		sp->loop_start <<= FRACTION_BITS;
		sp->loop_end <<= FRACTION_BITS;

		/* Adjust for fractional loop points. This is a guess. Does anyone
		know what "fractions" really stands for? */
		sp->loop_start |=
			(SmplHdr.Fractions & 0x0F) << (FRACTION_BITS - 4);
		sp->loop_end |=
			((SmplHdr.Fractions >> 4) & 0x0F) << (FRACTION_BITS - 4);

		/* If this instrument will always be played on the same note,
		and it's not looped, we can resample it now. */
		if (sp->note_to_use && !(sp->modes & MODES_LOOPING))
			pre_resample(sp);

		if (strip_tail == 1)
		{
			/* Let's not really, just say we did. */
			ctl->cmsg(CMSG_INFO, VERB_DEBUG, " - Stripping tail");
			sp->data_length = sp->loop_end;
		}
	}


	close_file(fp);
	return ip;
}

static int fill_bank(MidiSong* song, int dr, int b)
{
	int i, errors = 0;
	ToneBank* bank = ((dr) ? song->drumset[b] : song->tonebank[b]);
	if (!bank)
	{
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
			"Huh. Tried to load instruments in non-existent %s %d",
			(dr) ? "drumset" : "tone bank", b);
		return 0;
	}
	for (i = 0; i < 128; i++)
	{
		if (bank->instrument[i] == MAGIC_LOAD_INSTRUMENT)
		{
			bank->instrument[i] = load_instrument_sf2(song, b, i, dr ? true : false);
			if (bank->instrument[i])
			{
				continue;
			}
			bank->instrument[i] = load_instrument_dls(song, dr, b, i);
			if (bank->instrument[i])
			{
				continue;
			}
			if (!(bank->tone[i].name))
			{
				ctl->cmsg(CMSG_WARNING, (b != 0) ? VERB_VERBOSE : VERB_NORMAL,
					"No instrument mapped to %s %d, program %d%s",
					(dr)? "drum set" : "tone bank", b, i, 
					(b != 0) ? "" : " - this instrument will not be heard");
				if (b != 0)
				{
					/* Mark the corresponding instrument in the default
						bank / drumset for loading (if it isn't already) */
					if (!dr)
					{
						if (!(song->tonebank[0]->instrument[i]))
							song->tonebank[0]->instrument[i] = MAGIC_LOAD_INSTRUMENT;
					}
					else
					{
						if (!(song->drumset[0]->instrument[i]))
							song->drumset[0]->instrument[i] = MAGIC_LOAD_INSTRUMENT;
					}
				}
				bank->instrument[i] = 0;
				errors++;
			}
			else if (!(bank->instrument[i] =
				load_instrument(bank->tone[i].name, 
					(dr) ? 1 : 0,
					bank->tone[i].pan,
					bank->tone[i].amp,
					(bank->tone[i].note!=-1) ? 
					bank->tone[i].note :
					((dr) ? i : -1),
					(bank->tone[i].strip_loop!=-1) ?
					bank->tone[i].strip_loop :
					((dr) ? 1 : -1),
					(bank->tone[i].strip_envelope != -1) ? 
					bank->tone[i].strip_envelope :
					((dr) ? 1 : -1),
					bank->tone[i].strip_tail)))
			{
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL, 
					"Couldn't load instrument %s (%s %d, program %d)",
					bank->tone[i].name,
					(dr)? "drum set" : "tone bank", b, i);
				errors++;
			}
		}
	}
	return errors;
}

int load_missing_instruments(MidiSong* song)
{
	int i = 128, errors = 0;
	while (i--)
	{
		if (song->tonebank[i])
			errors += fill_bank(song, 0, i);
		if (song->drumset[i])
			errors += fill_bank(song, 1, i);
	}
	return errors;
}

void free_instruments(MidiSong* song)
{
	int i = 128;
	while(i--)
	{
		if (song->tonebank[i])
			free_bank(song, 0, i);
		if (song->drumset[i])
			free_bank(song, 1, i);
	}
}

int set_default_instrument(MidiSong* song, const char* name)
{
	Instrument* ip;
	if (!(ip = load_instrument(name, 0, -1, -1, -1, 0, 0, 0)))
		return -1;
	song->default_instrument = ip;
	song->default_program = SPECIAL_PROGRAM;
	return 0;
}

};
