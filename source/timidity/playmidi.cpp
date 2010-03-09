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

    playmidi.c -- random stuff in need of rearrangement

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timidity.h"

namespace LibTimidity
{

MidiSong* song;

signed char drumvolume[MAXCHAN][MAXNOTE];
signed char drumpanpot[MAXCHAN][MAXNOTE];
signed char drumreverberation[MAXCHAN][MAXNOTE];
signed char drumchorusdepth[MAXCHAN][MAXNOTE];

int adjust_panning_immediately=0;

static int32 sample_count;

int GM_System_On=0;
int XG_System_On=0;
int GS_System_On=0;
int XG_System_reverb_type;
int XG_System_chorus_type;
int XG_System_variation_type;


static void adjust_amplification()
{
	song->master_volume = (FLOAT_T)(song->amplification) / (FLOAT_T)100.0;
	song->master_volume /= 2;
}


static void adjust_master_volume(int32 vol)
{
	song->master_volume = (double)(vol*song->amplification) / 1638400.0L;
	song->master_volume /= 2;
}


static void reset_voices()
{
	for (int i = 0; i < MAX_VOICES; i++)
		song->voice[i].status = VOICE_FREE;
}

/* Process the Reset All Controllers event */
static void reset_controllers(int c)
{
	song->channel[c].volume = 90; /* Some standard says, although the SCC docs say 0. */
	song->channel[c].expression = 127; /* SCC-1 does this. */
	song->channel[c].sustain = 0;
	song->channel[c].pitchbend = 0x2000;
	song->channel[c].pitchfactor = 0; /* to be computed */

	song->channel[c].reverberation = 0;
	song->channel[c].chorusdepth = 0;
}

static void reset_midi()
{
	for (int i = 0; i < MAXCHAN; i++)
	{
		reset_controllers(i);
		/* The rest of these are unaffected by the Reset All Controllers event */
		song->channel[i].program = song->default_program;
		song->channel[i].panning = NO_PANNING;
		song->channel[i].pitchsens = 2;
		song->channel[i].bank = 0; /* tone bank or drum set */
		song->channel[i].harmoniccontent = 64,
		song->channel[i].releasetime = 64,
		song->channel[i].attacktime = 64,
		song->channel[i].brightness = 64,
		song->channel[i].sfx = 0;
	}
	reset_voices();
}

static void select_sample(int v, Instrument* ip)
{
	int32 f, cdiff, diff, midfreq;
	int s,i;
	Sample *sp, *closest;

	s = ip->samples;
	sp = ip->sample;

	if (s == 1)
	{
		song->voice[v].sample = sp;
		return;
	}

	f = song->voice[v].orig_frequency;
	/* 
		No suitable sample found! We'll select the sample whose root
		frequency is closest to the one we want. (Actually we should
		probably convert the low, high, and root frequencies to MIDI note
		values and compare those.) */

	cdiff = 0x7FFFFFFF;
	closest = sp = ip->sample;
	midfreq = (sp->low_freq + sp->high_freq) / 2;
	for (i = 0; i < s; i++)
	{
		diff = sp->root_freq - f;
		/*  But the root freq. can perfectly well lie outside the keyrange
		*  frequencies, so let's try:
		*/
		/* diff=midfreq - f; */
		if (diff < 0)
			diff = -diff;
		if (diff < cdiff)
		{
			cdiff = diff;
			closest = sp;
		}
		sp++;
	}
	song->voice[v].sample = closest;
	return;
}



static void select_stereo_samples(int v, InstrumentLayer* lp)
{
	Instrument *ip;
	InstrumentLayer *nlp, *bestvel;
	int diffvel, midvel, mindiff;

	/* select closest velocity */
	bestvel = lp;
	mindiff = 500;
	for (nlp = lp; nlp; nlp = nlp->next)
	{
		midvel = (nlp->hi + nlp->lo) / 2;
		if (!midvel)
			diffvel = 127;
		else if (song->voice[v].velocity < nlp->lo || song->voice[v].velocity > nlp->hi)
			diffvel = 200;
		else
			diffvel = song->voice[v].velocity - midvel;
		if (diffvel < 0)
			diffvel = -diffvel;
		if (diffvel < mindiff)
		{
			mindiff = diffvel;
			bestvel = nlp;
		}
	}
	ip = bestvel->instrument;

	if (ip->right_sample)
	{
		ip->sample = ip->right_sample;
		ip->samples = ip->right_samples;
		select_sample(v, ip);
		song->voice[v].right_sample = song->voice[v].sample;
	}
	else
		song->voice[v].right_sample = 0;
	ip->sample = ip->left_sample;
	ip->samples = ip->left_samples;
	select_sample(v, ip);
}


static void recompute_freq(int v)
{
	int
		sign = (song->voice[v].sample_increment < 0), /* for bidirectional loops */
		pb = song->channel[song->voice[v].channel].pitchbend;
	double a;

	if (!song->voice[v].sample->sample_rate)
		return;

	if (song->voice[v].vibrato_control_ratio)
	{
		/* This instrument has vibrato. Invalidate any precomputed
			sample_increments. */
	
		int i = VIBRATO_SAMPLE_INCREMENTS;
		while (i--)
			song->voice[v].vibrato_sample_increment[i] = 0;
	}

	if (pb == 0x2000 || pb < 0 || pb > 0x3FFF)
		song->voice[v].frequency = song->voice[v].orig_frequency;
	else
	{
		pb -= 0x2000;
		if (!(song->channel[song->voice[v].channel].pitchfactor))
		{
			/* Damn. Somebody bent the pitch. */
			int32 i = pb * song->channel[song->voice[v].channel].pitchsens;
			if (pb < 0)
				i = -i;
			song->channel[song->voice[v].channel].pitchfactor =
				(FLOAT_T)(bend_fine[(i >> 5) & 0xFF] * bend_coarse[i >> 13]);
		}
		if (pb > 0)
			song->voice[v].frequency =
				(int32)(song->channel[song->voice[v].channel].pitchfactor *
				(double)(song->voice[v].orig_frequency));
		else
			song->voice[v].frequency =
				(int32)((double)(song->voice[v].orig_frequency) /
				song->channel[song->voice[v].channel].pitchfactor);
	}

	a = FSCALE(((double)(song->voice[v].sample->sample_rate) *
			(double)(song->voice[v].frequency)) /
			((double)(song->voice[v].sample->root_freq) *
			(double)(OUTPUT_RATE)),
			FRACTION_BITS);

	if (sign) 
		a = -a; /* need to preserve the loop direction */

	song->voice[v].sample_increment = (int32)(a);
}

static int vcurve[128] =
{
	0,0,18,29,36,42,47,51,55,58,
	60,63,65,67,69,71,73,74,76,77,
	79,80,81,82,83,84,85,86,87,88,
	89,90,91,92,92,93,94,95,95,96,
	97,97,98,99,99,100,100,101,101,102,
	103,103,104,104,105,105,106,106,106,107,
	107,108,108,109,109,109,110,110,111,111,
	111,112,112,112,113,113,114,114,114,115,
	115,115,116,116,116,116,117,117,117,118,
	118,118,119,119,119,119,120,120,120,120,
	121,121,121,122,122,122,122,123,123,123,
	123,123,124,124,124,124,125,125,125,125,
	126,126,126,126,126,127,127,127
};

static void recompute_amp(int v)
{
	int32 tempamp;
	int chan = song->voice[v].channel;
	int panning = song->voice[v].panning;
	int vol = song->channel[chan].volume;
	int expr = song->channel[chan].expression;
	int vel = vcurve[song->voice[v].velocity];
	int drumpan = NO_PANNING;
	FLOAT_T curved_expression, curved_volume;

	if (song->channel[chan].kit)
	{
		int note = song->voice[v].sample->note_to_use;
		if (note > 0 && drumvolume[chan][note] >= 0)
			vol = drumvolume[chan][note];
		if (note > 0 && drumpanpot[chan][note] >= 0)
			panning = drumvolume[chan][note];
	}

	curved_expression = 127.0 * vol_table[expr];

	curved_volume = 127.0 * vol_table[vol];

	tempamp= (int32)((FLOAT_T)vel * curved_volume * curved_expression); /* 21 bits */

	/* TODO: use fscale */

	if (panning > 60 && panning < 68)
	{
		song->voice[v].panned=PANNED_CENTRE;

		song->voice[v].left_amp=
			FSCALENEG((double)(tempamp) * song->voice[v].sample->volume *
			song->master_volume, 21);
	}
	else if (panning < 5)
	{
		song->voice[v].panned = PANNED_LEFT;

		song->voice[v].left_amp =
			FSCALENEG((double)(tempamp) * song->voice[v].sample->volume * song->master_volume,
			20);
	}
	else if (panning > 123)
	{
		song->voice[v].panned = PANNED_RIGHT;

		song->voice[v].left_amp = /* left_amp will be used */
			FSCALENEG((double)(tempamp) * song->voice[v].sample->volume * song->master_volume,
			20);
	}
	else
	{
		FLOAT_T refv = (double)(tempamp) * song->voice[v].sample->volume * song->master_volume;

		song->voice[v].panned = PANNED_MYSTERY;
		song->voice[v].lfe_amp = FSCALENEG(refv * 64, 27);
		song->voice[v].lr_amp = 0;
		song->voice[v].left_amp = FSCALENEG(refv * (128-panning), 27);
		song->voice[v].ce_amp = 0;
		song->voice[v].right_amp = FSCALENEG(refv * panning, 27);
		song->voice[v].rr_amp = 0;
	}
}


#define NOT_CLONE 0
#define STEREO_CLONE 1
#define REVERB_CLONE 2
#define CHORUS_CLONE 3


/* just a variant of note_on() */
static int vc_alloc(int j)
{
	int i = song->voices; 

	while (i--)
	{
		if (i == j)
			continue;
		if (song->voice[i].status & VOICE_FREE)
		{
			return i;
		}
	}
	return -1;
}

static void kill_note(int i);

static void kill_others(int i)
{
	int j = song->voices;

	if (!song->voice[i].sample->exclusiveClass)
		return;

	while (j--)
	{
		if (song->voice[j].status & (VOICE_FREE|VOICE_OFF|VOICE_DIE))
			continue;
		if (i == j)
			continue;
		if (song->voice[i].channel != song->voice[j].channel)
			continue;
		if (song->voice[j].sample->note_to_use)
		{
			if (song->voice[j].sample->exclusiveClass != song->voice[i].sample->exclusiveClass)
				continue;
			kill_note(j);
		}
	}
}


static void clone_voice(Instrument* ip, int v, MidiEvent* e, int clone_type, int variationbank)
{
	int w, played_note, chorus = 0, reverb = 0, milli;
	int chan = song->voice[v].channel;

	if (clone_type == STEREO_CLONE)
	{
		if (!song->voice[v].right_sample && variationbank != 3)
			return;
		if (variationbank == 6)
			return;
	}

	if (song->channel[chan].kit)
	{
		reverb = drumreverberation[chan][song->voice[v].note];
		chorus = drumchorusdepth[chan][song->voice[v].note];
	}
	else
	{
		reverb = song->channel[chan].reverberation;
		chorus = song->channel[chan].chorusdepth;
	}

	if (clone_type == REVERB_CLONE)
		chorus = 0;
	else if (clone_type == CHORUS_CLONE)
		reverb = 0;
	else if (clone_type == STEREO_CLONE)
		reverb = chorus = 0;

	if (reverb > 127)
		reverb = 127;
	if (chorus > 127)
		chorus = 127;

	if (clone_type == CHORUS_CLONE)
	{
		if (variationbank == 32)
			chorus = 30;
		else if (variationbank == 33)
			chorus = 60;
		else if (variationbank == 34)
			chorus = 90;
	}

	chorus /= 2;  /* This is an ad hoc adjustment. */

	if (!reverb && !chorus && clone_type != STEREO_CLONE)
		return;

	if ((w = vc_alloc(v)) < 0)
		return;

	song->voice[w] = song->voice[v];
	if (clone_type == STEREO_CLONE)
		song->voice[v].clone_voice = w;
	song->voice[w].clone_voice = v;
	song->voice[w].clone_type = clone_type;

	song->voice[w].sample = song->voice[v].right_sample;
	song->voice[w].velocity= e->b;

	milli = OUTPUT_RATE/1000;

	if (clone_type == STEREO_CLONE)
	{
		int left, right, leftpan, rightpan;
		int panrequest = song->voice[v].panning;
		if (variationbank == 3)
		{
			song->voice[v].panning = 0;
			song->voice[w].panning = 127;
		}
		else
		{
			if (song->voice[v].sample->panning > song->voice[w].sample->panning)
			{
				left = w;
				right = v;
			}
			else
			{
				left = v;
				right = w;
			}
#define INSTRUMENT_SEPARATION 12
			leftpan = panrequest - INSTRUMENT_SEPARATION / 2;
			rightpan = leftpan + INSTRUMENT_SEPARATION;
			if (leftpan < 0)
			{
				leftpan = 0;
				rightpan = leftpan + INSTRUMENT_SEPARATION;
			}
			if (rightpan > 127)
			{
				rightpan = 127;
				leftpan = rightpan - INSTRUMENT_SEPARATION;
			}
			song->voice[left].panning = leftpan;
			song->voice[right].panning = rightpan;
			song->voice[right].echo_delay = 20 * milli;
		}
	}

	song->voice[w].volume = song->voice[w].sample->volume;

	if (reverb)
	{
		if (song->voice[v].panning < 64)
			song->voice[w].panning = 64 + reverb/2;
		else
			song->voice[w].panning = 64 - reverb/2;

		/* try 98->99 for melodic instruments ? (bit much for percussion) */
		song->voice[w].volume *= vol_table[(127 - reverb) / 8 + 98];

		song->voice[w].echo_delay += reverb * milli;
		song->voice[w].envelope_rate[DECAY] *= 2;
		song->voice[w].envelope_rate[RELEASE] /= 2;

		if (XG_System_reverb_type >= 0)
		{
			int subtype = XG_System_reverb_type & 0x07;
			int rtype = XG_System_reverb_type >>3;
			switch (rtype)
			{
			case 0: /* no effect */
				break;
			case 1: /* hall */
				if (subtype) song->voice[w].echo_delay += 100 * milli;
				break;
			case 2: /* room */
				song->voice[w].echo_delay /= 2;
				break;
			case 3: /* stage */
				song->voice[w].velocity = song->voice[v].velocity;
				break;
			case 4: /* plate */
				song->voice[w].panning = song->voice[v].panning;
				break;
			case 16: /* white room */
				song->voice[w].echo_delay = 0;
				break;
			case 17: /* tunnel */
				song->voice[w].echo_delay *= 2;
				song->voice[w].velocity /= 2;
				break;
			case 18: /* canyon */
				song->voice[w].echo_delay *= 2;
				break;
			case 19: /* basement */
				song->voice[w].velocity /= 2;
				break;
			default:
				break;
			}
		}
	}
	played_note = song->voice[w].sample->note_to_use;
	if (!played_note)
	{
		played_note = e->a & 0x7f;
		if (variationbank == 35)
			played_note += 12;
		else if (variationbank == 36)
			played_note -= 12;
		else if (variationbank == 37)
			played_note += 7;
		else if (variationbank == 36)
			played_note -= 7;
	}
	song->voice[w].note = played_note;
	song->voice[w].orig_frequency = freq_table[played_note];

	if (chorus)
	{
		if (!song->voice[w].vibrato_control_ratio)
		{
			song->voice[w].vibrato_control_ratio = 100;
			song->voice[w].vibrato_depth = 6;
			song->voice[w].vibrato_sweep = 74;
		}
		song->voice[w].volume *= 0.40;
		song->voice[v].volume = song->voice[w].volume;
		recompute_amp(v);
		apply_envelope_to_amp(v);
		song->voice[w].vibrato_sweep = chorus/2;
		song->voice[w].vibrato_depth /= 2;
		if (!song->voice[w].vibrato_depth)
			song->voice[w].vibrato_depth = 2;
		song->voice[w].vibrato_control_ratio /= 2;
		song->voice[w].echo_delay += 30 * milli;

		if (XG_System_chorus_type >= 0)
		{
			int subtype = XG_System_chorus_type & 0x07;
			int chtype = 0x0f & (XG_System_chorus_type >> 3);
			switch (chtype)
			{
			case 0: /* no effect */
				break;
			case 1: /* chorus */
				chorus /= 3;
				if (song->channel[ song->voice[w].channel ].pitchbend + chorus < 0x2000)
					song->voice[w].orig_frequency =
						(uint32)( (FLOAT_T)song->voice[w].orig_frequency * bend_fine[chorus] );
				else
					song->voice[w].orig_frequency =
						(uint32)( (FLOAT_T)song->voice[w].orig_frequency / bend_fine[chorus] );
				if (subtype)
					song->voice[w].vibrato_depth *= 2;
				break;
			case 2: /* celeste */
				song->voice[w].orig_frequency += (song->voice[w].orig_frequency / 128) * chorus;
				break;
			case 3: /* flanger */
				song->voice[w].vibrato_control_ratio = 10;
				song->voice[w].vibrato_depth = 100;
				song->voice[w].vibrato_sweep = 8;
				song->voice[w].echo_delay += 200 * milli;
				break;
			case 4: /* symphonic : cf Children of the Night /128 bad, /1024 ok */
				song->voice[w].orig_frequency += (song->voice[w].orig_frequency/512) * chorus;
				song->voice[v].orig_frequency -= (song->voice[v].orig_frequency/512) * chorus;
				recompute_freq(v);
				break;
			case 8: /* phaser */
				break;
			default:
				break;
			}
		}
		else
		{
			chorus /= 3;
			if (song->channel[ song->voice[w].channel ].pitchbend + chorus < 0x2000)
				song->voice[w].orig_frequency =
					(uint32)( (FLOAT_T)song->voice[w].orig_frequency * bend_fine[chorus] );
			else
				song->voice[w].orig_frequency =
					(uint32)( (FLOAT_T)song->voice[w].orig_frequency / bend_fine[chorus] );
		}
	}
	song->voice[w].echo_delay_count = song->voice[w].echo_delay;
	if (reverb)
		song->voice[w].echo_delay *= 2;

	recompute_freq(w);
	recompute_amp(w);
	if (song->voice[w].sample->modes & MODES_ENVELOPE)
	{
		/* Ramp up from 0 */
		song->voice[w].envelope_stage = ATTACK;
		song->voice[w].modulation_stage = ATTACK;
		song->voice[w].envelope_volume = 0;
		song->voice[w].modulation_volume = 0;
		song->voice[w].control_counter = 0;
		song->voice[w].modulation_counter = 0;
		recompute_envelope(w);
		/*recompute_modulation(w);*/
	}
	else
	{
		song->voice[w].envelope_increment = 0;
		song->voice[w].modulation_increment = 0;
	}
	apply_envelope_to_amp(w);
}


static void xremap(int* banknumpt, int* this_notept, int this_kit)
{
	int i, newmap;
	int banknum = *banknumpt;
	int this_note = *this_notept;
	int newbank, newnote;

	if (!this_kit)
	{
		if (banknum == SFXBANK && song->tonebank[SFXBANK])
			return;
		if (banknum == SFXBANK && song->tonebank[120])
			*banknumpt = 120;
		return;
	}

	if (this_kit != 127 && this_kit != 126)
		return;

	for (i = 0; i < XMAPMAX; i++)
	{
		newmap = xmap[i][0];
		if (!newmap)
			return;
		if (this_kit == 127 && newmap != XGDRUM)
			continue;
		if (this_kit == 126 && newmap != SFXDRUM1)
			continue;
		if (xmap[i][1] != banknum)
			continue;
		if (xmap[i][3] != this_note)
			continue;
		newbank = xmap[i][2];
		newnote = xmap[i][4];
		if (newbank == banknum && newnote == this_note)
			return;
		if (!song->drumset[newbank])
			return;
		if (!song->drumset[newbank]->tone[newnote].layer)
			return;
		if (song->drumset[newbank]->tone[newnote].layer == MAGIC_LOAD_INSTRUMENT)
			return;
		*banknumpt = newbank;
		*this_notept = newnote;
		return;
	}
}


static void start_note(MidiEvent* e, int i)
{
	InstrumentLayer *lp;
	Instrument *ip;
	int j, banknum, ch=e->channel;
	int played_note, drumpan=NO_PANNING;
	int32 rt;
	int attacktime, releasetime, decaytime, variationbank;
	int brightness = song->channel[ch].brightness;
	int harmoniccontent = song->channel[ch].harmoniccontent;
	int this_note = e->a;
	int this_velocity = e->b;
	int drumsflag = song->channel[ch].kit;
	int this_prog = song->channel[ch].program;

	if (song->channel[ch].sfx)
		banknum = song->channel[ch].sfx;
	else
		banknum = song->channel[ch].bank;

	song->voice[i].velocity = this_velocity;

	if (XG_System_On)
		xremap(&banknum, &this_note, drumsflag);
	/*   if (current_config_pc42b) pcmap(&banknum, &this_note, &this_prog, &drumsflag); */

	if (drumsflag)
	{
		if (!(lp=song->drumset[banknum]->tone[this_note].layer))
		{
			if (!(lp=song->drumset[0]->tone[this_note].layer))
				return; /* No instrument? Then we can't play. */
		}
		ip = lp->instrument;
		if (ip->type == INST_GUS && ip->samples != 1)
		{
			ctl->cmsg(CMSG_WARNING, VERB_VERBOSE, 
				"Strange: percussion instrument with %d samples!", ip->samples);
		}
	
		if (ip->sample->note_to_use) /* Do we have a fixed pitch? */
		{
			song->voice[i].orig_frequency=freq_table[(int)(ip->sample->note_to_use)];
			drumpan = drumpanpot[ch][(int)ip->sample->note_to_use];
		}
		else
			song->voice[i].orig_frequency=freq_table[this_note & 0x7F];
	}
	else
	{
		if (song->channel[ch].program == SPECIAL_PROGRAM)
			lp = song->default_instrument;
		else if (!(lp = song->tonebank[song->channel[ch].bank]->tone[song->channel[ch].program].layer))
		{
			if (!(lp=song->tonebank[0]->tone[this_prog].layer))
				return; /* No instrument? Then we can't play. */
		}
		ip = lp->instrument;
		if (ip->sample->note_to_use) /* Fixed-pitch instrument? */
			song->voice[i].orig_frequency = freq_table[(int)(ip->sample->note_to_use)];
		else
			song->voice[i].orig_frequency = freq_table[this_note & 0x7F];
	}

	select_stereo_samples(i, lp);

	song->voice[i].starttime = e->time;
	played_note = song->voice[i].sample->note_to_use;
	
	if (!played_note || !drumsflag)
		played_note = this_note & 0x7f;
	song->voice[i].status = VOICE_ON;
	song->voice[i].channel = ch;
	song->voice[i].note = played_note;
	song->voice[i].velocity = this_velocity;
	song->voice[i].sample_offset = 0;
	song->voice[i].sample_increment = 0; /* make sure it isn't negative */

	song->voice[i].tremolo_phase = 0;
	song->voice[i].tremolo_phase_increment = song->voice[i].sample->tremolo_phase_increment;
	song->voice[i].tremolo_sweep = song->voice[i].sample->tremolo_sweep_increment;
	song->voice[i].tremolo_sweep_position = 0;

	song->voice[i].vibrato_sweep = song->voice[i].sample->vibrato_sweep_increment;
	song->voice[i].vibrato_sweep_position = 0;
	song->voice[i].vibrato_depth = song->voice[i].sample->vibrato_depth;
	song->voice[i].vibrato_control_ratio = song->voice[i].sample->vibrato_control_ratio;
	song->voice[i].vibrato_control_counter = song->voice[i].vibrato_phase=0;
	song->voice[i].vibrato_delay = song->voice[i].sample->vibrato_delay;

	kill_others(i);

	for (j = 0; j < VIBRATO_SAMPLE_INCREMENTS; j++)
		song->voice[i].vibrato_sample_increment[j] = 0;


	attacktime = song->channel[ch].attacktime;
	releasetime = song->channel[ch].releasetime;
	decaytime = 64;
	variationbank = song->channel[ch].variationbank;

	switch (variationbank)
	{
	case  8:
		attacktime = 64+32;
		break;
	case 12:
		decaytime = 64-32;
		break;
	case 16:
		brightness = 64+16;
		break;
	case 17:
		brightness = 64+32;
		break;
	case 18:
		brightness = 64-16;
		break;
	case 19:
		brightness = 64-32;
		break;
	case 20:
		harmoniccontent = 64+16;
		break;
#if 0
	case 24:
		song->voice[i].modEnvToFilterFc=2.0;
      		song->voice[i].sample->cutoff_freq = 800;
		break;
	case 25:
		song->voice[i].modEnvToFilterFc=-2.0;
      		song->voice[i].sample->cutoff_freq = 800;
		break;
	case 27:
		song->voice[i].modLfoToFilterFc=2.0;
		song->voice[i].lfo_phase_increment=109;
		song->voice[i].lfo_sweep=122;
      		song->voice[i].sample->cutoff_freq = 800;
		break;
	case 28:
		song->voice[i].modLfoToFilterFc=-2.0;
		song->voice[i].lfo_phase_increment=109;
		song->voice[i].lfo_sweep=122;
      		song->voice[i].sample->cutoff_freq = 800;
		break;
#endif
	default:
		break;
	}


	for (j = ATTACK; j < MAXPOINT; j++)
	{
		song->voice[i].envelope_rate[j] = song->voice[i].sample->envelope_rate[j];
		song->voice[i].envelope_offset[j] = song->voice[i].sample->envelope_offset[j];
	}

	song->voice[i].echo_delay = song->voice[i].envelope_rate[DELAY];
	song->voice[i].echo_delay_count = song->voice[i].echo_delay;

	if (attacktime!=64)
	{
		rt = song->voice[i].envelope_rate[ATTACK];
		rt = rt + ((64 - attacktime) * rt) / 100;
		if (rt > 1000)
			song->voice[i].envelope_rate[ATTACK] = rt;
	}
	if (releasetime!=64)
	{
		rt = song->voice[i].envelope_rate[RELEASE];
		rt = rt + ((64 - releasetime) * rt) / 100;
		if (rt > 1000)
			song->voice[i].envelope_rate[RELEASE] = rt;
	}
	if (decaytime!=64)
	{
		rt = song->voice[i].envelope_rate[DECAY];
		rt = rt + ((64 - decaytime) * rt) / 100;
		if (rt > 1000)
			song->voice[i].envelope_rate[DECAY] = rt;
	}

	if (song->channel[ch].panning != NO_PANNING)
		song->voice[i].panning = song->channel[ch].panning;
	else
		song->voice[i].panning = song->voice[i].sample->panning;
	if (drumpan != NO_PANNING)
		song->voice[i].panning = drumpan;

	if (variationbank == 1)
	{
		int pan = song->voice[i].panning;
		int disturb = 0;
		/* If they're close up (no reverb) and you are behind the pianist,
		* high notes come from the right, so we'll spread piano etc. notes
		* out horizontally according to their pitches.
		*/
		if (this_prog < 21)
		{
			int n = song->voice[i].velocity - 32;
			if (n < 0) n = 0;
			if (n > 64) n = 64;
			pan = pan / 2 + n;
		}
		/* For other types of instruments, the music sounds more alive if
		* notes come from slightly different directions.  However, instruments
		* do drift around in a sometimes disconcerting way, so the following
		* might not be such a good idea.
		*/
		else
			disturb = (song->voice[i].velocity/32 % 8) + (song->voice[i].note % 8); /* /16? */

		if (pan < 64)
			pan += disturb;
		else
			pan -= disturb;
		if (pan < 0)
			pan = 0;
		else if (pan > 127)
			pan = 127;
		song->voice[i].panning = pan;
	}

	recompute_freq(i);
	recompute_amp(i);
	if (song->voice[i].sample->modes & MODES_ENVELOPE)
	{
		/* Ramp up from 0 */
		song->voice[i].envelope_stage = ATTACK;
		song->voice[i].envelope_volume = 0;
		song->voice[i].control_counter = 0;
		recompute_envelope(i);
	}
	else
	{
		song->voice[i].envelope_increment = 0;
	}
	apply_envelope_to_amp(i);

	song->voice[i].clone_voice = -1;
	song->voice[i].clone_type = NOT_CLONE;

	clone_voice(ip, i, e, STEREO_CLONE, variationbank);
	clone_voice(ip, i, e, CHORUS_CLONE, variationbank);
	clone_voice(ip, i, e, REVERB_CLONE, variationbank);
}

static void kill_note(int i)
{
	song->voice[i].status=VOICE_DIE;
	if (song->voice[i].clone_voice >= 0)
		song->voice[ song->voice[i].clone_voice ].status = VOICE_DIE;
}


/* Only one instance of a note can be playing on a single channel. */
static void note_on(MidiEvent* e)
{
	int i = song->voices, lowest = -1;
	int32 lv = 0x7FFFFFFF, v;

	while (i--)
	{
		if (song->voice[i].status == VOICE_FREE)
			lowest=i; /* Can't get a lower volume than silence */
		else if (song->voice[i].channel == e->channel && 
				(song->voice[i].note == e->a || song->channel[song->voice[i].channel].mono))
			kill_note(i);
	}

	if (lowest != -1)
	{
		/* Found a free voice. */
		start_note(e,lowest);
		return;
	}

#if 0
  /* Look for the decaying note with the lowest volume */
  i=song->voices;
  while (i--)
    {
      if (song->voice[i].status & ~(VOICE_ON | VOICE_DIE | VOICE_FREE))
	{
	  v=song->voice[i].left_mix;
	  if ((song->voice[i].panned==PANNED_MYSTERY) && (song->voice[i].right_mix>v))
	    v=song->voice[i].right_mix;
	  if (v<lv)
	    {
	      lv=v;
	      lowest=i;
	    }
	}
    }
#endif

	/* Look for the decaying note with the lowest volume */
	if (lowest == -1)
	{
		i = song->voices;
		while (i--)
		{
			if ((song->voice[i].status & ~(VOICE_ON | VOICE_DIE | VOICE_FREE)) &&
				(!song->voice[i].clone_type))
			{
				v = song->voice[i].left_mix;
				if ((song->voice[i].panned == PANNED_MYSTERY) && (song->voice[i].right_mix > v))
					v = song->voice[i].right_mix;
				if (v < lv)
				{
					lv = v;
					lowest = i;
				}
			}
		}
	}

	if (lowest != -1)
	{
		int cl = song->voice[lowest].clone_voice;

		/* This can still cause a click, but if we had a free voice to
		spare for ramping down this note, we wouldn't need to kill it
		in the first place... Still, this needs to be fixed. Perhaps
		we could use a reserve of voices to play dying notes only. */

		if (cl >= 0)
		{
			if (song->voice[cl].clone_type == STEREO_CLONE ||
					(!song->voice[cl].clone_type && song->voice[lowest].clone_type == STEREO_CLONE))
				song->voice[cl].status = VOICE_FREE;
			else if (song->voice[cl].clone_voice==lowest)
				song->voice[cl].clone_voice = -1;
		}

		song->cut_notes++;
		song->voice[lowest].status = VOICE_FREE;
		start_note(e,lowest);
	}
	else
		song->lost_notes++;
}

static void finish_note(int i)
{
	if (song->voice[i].sample->modes & MODES_ENVELOPE)
	{
		/* We need to get the envelope out of Sustain stage */
		song->voice[i].envelope_stage = 3;
		song->voice[i].status = VOICE_OFF;
		recompute_envelope(i);
		apply_envelope_to_amp(i);
	}
	else
	{
		/* Set status to OFF so resample_voice() will let this voice out
			of its loop, if any. In any case, this voice dies when it
			hits the end of its data (ofs>=data_length). */
		song->voice[i].status = VOICE_OFF;
	}

	{
		int v;
		if ((v = song->voice[i].clone_voice) >= 0)
		{
			song->voice[i].clone_voice = -1;
			finish_note(v);
		}
	}
}

static void note_off(MidiEvent* e)
{
	int i = song->voices, v;
	while (i--)
		if (song->voice[i].status == VOICE_ON &&
			song->voice[i].channel == e->channel &&
			song->voice[i].note == e->a)
		{
			if (song->channel[e->channel].sustain)
			{
				song->voice[i].status = VOICE_SUSTAINED;

				if ((v = song->voice[i].clone_voice) >= 0)
				{
					if (song->voice[v].status == VOICE_ON)
						song->voice[v].status = VOICE_SUSTAINED;
				}
			}
			else
				finish_note(i);
			return;
		}
}

/* Process the All Notes Off event */
static void all_notes_off(int c)
{
	int i = song->voices;
	ctl->cmsg(CMSG_INFO, VERB_DEBUG, "All notes off on channel %d", c);
	while (i--)
		if (song->voice[i].status == VOICE_ON &&
			song->voice[i].channel == c)
		{
			if (song->channel[c].sustain)
			{
				song->voice[i].status = VOICE_SUSTAINED;
			}
			else
				finish_note(i);
		}
}

/* Process the All Sounds Off event */
static void all_sounds_off(int c)
{
	int i=song->voices;
	while (i--)
		if (song->voice[i].channel == c &&
			song->voice[i].status != VOICE_FREE &&
			song->voice[i].status != VOICE_DIE)
		{
			kill_note(i);
		}
}

static void adjust_pressure(MidiEvent* e)
{
	int i=song->voices;
	while (i--)
		if (song->voice[i].status == VOICE_ON &&
			song->voice[i].channel == e->channel &&
			song->voice[i].note == e->a)
		{
			song->voice[i].velocity = e->b;
			recompute_amp(i);
			apply_envelope_to_amp(i);
			return;
		}
}

static void adjust_panning(int c)
{
	int i = song->voices;
	while (i--)
		if ((song->voice[i].channel == c) &&
			(song->voice[i].status == VOICE_ON || song->voice[i].status == VOICE_SUSTAINED))
		{
			if (song->voice[i].clone_type != NOT_CLONE)
				continue;
			song->voice[i].panning = song->channel[c].panning;
			recompute_amp(i);
			apply_envelope_to_amp(i);
		}
}

static void drop_sustain(int c)
{
	int i = song->voices;
	while (i--)
		if (song->voice[i].status == VOICE_SUSTAINED && song->voice[i].channel == c)
			finish_note(i);
}

static void adjust_pitchbend(int c)
{
	int i = song->voices;
	while (i--)
		if (song->voice[i].status != VOICE_FREE && song->voice[i].channel == c)
		{
			recompute_freq(i);
		}
}

static void adjust_volume(int c)
{
	int i = song->voices;
	while (i--)
		if (song->voice[i].channel == c &&
			(song->voice[i].status == VOICE_ON || song->voice[i].status == VOICE_SUSTAINED))
		{
			recompute_amp(i);
			apply_envelope_to_amp(i);
		}
}

static void seek_forward(int32 until_time)
{
	reset_voices();
	while (song->current_event->time < until_time)
	{
		switch(song->current_event->type)
		{
		/* All notes stay off. Just handle the parameter changes. */

		case ME_PITCH_SENS:
			song->channel[song->current_event->channel].pitchsens =
				song->current_event->a;
			song->channel[song->current_event->channel].pitchfactor = 0;
			break;

		case ME_PITCHWHEEL:
			song->channel[song->current_event->channel].pitchbend =
				song->current_event->a + song->current_event->b * 128;
			song->channel[song->current_event->channel].pitchfactor = 0;
			break;

		case ME_MAINVOLUME:
			song->channel[song->current_event->channel].volume = song->current_event->a;
			break;

		case ME_MASTERVOLUME:
			adjust_master_volume(song->current_event->a + (song->current_event->b << 7));
			break;

		case ME_PAN:
			song->channel[song->current_event->channel].panning = song->current_event->a;
			break;

		case ME_EXPRESSION:
			song->channel[song->current_event->channel].expression = song->current_event->a;
			break;

		case ME_PROGRAM:
			/* if (ISDRUMCHANNEL(song->current_event->channel)) */
			if (song->channel[song->current_event->channel].kit)
				/* Change drum set */
				song->channel[song->current_event->channel].bank = song->current_event->a;
			else
				song->channel[song->current_event->channel].program = song->current_event->a;
			break;

		case ME_SUSTAIN:
			song->channel[song->current_event->channel].sustain = song->current_event->a;
			break;


		case ME_REVERBERATION:
			song->channel[song->current_event->channel].reverberation = song->current_event->a;
			break;

		case ME_CHORUSDEPTH:
			song->channel[song->current_event->channel].chorusdepth = song->current_event->a;
			break;

		case ME_HARMONICCONTENT:
			song->channel[song->current_event->channel].harmoniccontent = song->current_event->a;
			break;

		case ME_RELEASETIME:
			song->channel[song->current_event->channel].releasetime = song->current_event->a;
			break;

		case ME_ATTACKTIME:
			song->channel[song->current_event->channel].attacktime = song->current_event->a;
			break;

		case ME_BRIGHTNESS:
			song->channel[song->current_event->channel].brightness = song->current_event->a;
			break;

		case ME_TONE_KIT:
			if (song->current_event->a == SFX_BANKTYPE)
			{
				song->channel[song->current_event->channel].sfx = SFXBANK;
				song->channel[song->current_event->channel].kit = 0;
			}
			else
			{
				song->channel[song->current_event->channel].sfx = 0;
				song->channel[song->current_event->channel].kit = song->current_event->a;
			}
			break;


		case ME_RESET_CONTROLLERS:
			reset_controllers(song->current_event->channel);
			break;

		case ME_TONE_BANK:
			song->channel[song->current_event->channel].bank = song->current_event->a;
			break;

		case ME_EOT:
			song->current_sample = song->current_event->time;
			return;
		}
		song->current_event++;
	}
	/*song->current_sample=song->current_event->time;*/
	if (song->current_event != song->events)
		song->current_event--;
	song->current_sample = until_time;
}

static void skip_to(int32 until_time)
{
	if (song->current_sample > until_time)
		song->current_sample = 0;

	reset_midi();
	song->current_event = song->events;

	if (until_time)
		seek_forward(until_time);
}

static void do_compute_data(uint32 count)
{
	int i;
	if (!count)
		return; /* (gl) */
	memset(song->common_buffer, 0, count * 2 * 4);
	for (i = 0; i < song->voices; i++)
	{
		if (song->voice[i].status != VOICE_FREE)
		{
			if (!song->voice[i].sample_offset && song->voice[i].echo_delay_count)
			{
				if ((uint32)song->voice[i].echo_delay_count >= count)
					song->voice[i].echo_delay_count -= count;
				else
				{
					mix_voice(song->common_buffer + song->voice[i].echo_delay_count, i, count - song->voice[i].echo_delay_count);
					song->voice[i].echo_delay_count = 0;
				}
			}
			else
				mix_voice(song->common_buffer, i, count);
		}
	}
	song->current_sample += count;
}


static void s32tos16(void* dp, int32* lp, int32 c)
{
	int16* sp = (int16*)(dp);
	int32 l;
	while (c--)
	{
		l = (*lp++) >> (32 - 16 - GUARD_BITS);
		if (l > 32767)
			l = 32767;
		else if (l < -32768)
			l = -32768;
		*sp++ = (int16)(l);
	}
}

int Timidity_PlaySome(void *stream, int samples)
{
	int32 end_sample;
	int conv_count;
	int sample_size = 2 * 2;
	int stream_start = 0;

	if (!song->playing)
	{
		return 0;
	}
	end_sample = song->current_sample + samples;
	while (song->current_sample < end_sample)
	{
		/* Handle all events that should happen at this time */
		while (song->current_event->time <= song->current_sample)
		{
			switch (song->current_event->type)
			{

			/* Effects affecting a single note */

			case ME_NOTEON:
				song->current_event->a += song->channel[song->current_event->channel].transpose;
				if (!(song->current_event->b)) /* Velocity 0? */
					note_off(song->current_event);
				else
					note_on(song->current_event);
				break;

			case ME_NOTEOFF:
				song->current_event->a += song->channel[song->current_event->channel].transpose;
				note_off(song->current_event);
				break;

			case ME_KEYPRESSURE:
				adjust_pressure(song->current_event);
				break;

			/* Effects affecting a single channel */

			case ME_PITCH_SENS:
				song->channel[song->current_event->channel].pitchsens=song->current_event->a;
				song->channel[song->current_event->channel].pitchfactor=0;
				break;

			case ME_PITCHWHEEL:
				song->channel[song->current_event->channel].pitchbend=
					song->current_event->a + song->current_event->b * 128;
				song->channel[song->current_event->channel].pitchfactor=0;
				/* Adjust pitch for notes already playing */
				adjust_pitchbend(song->current_event->channel);
				break;

			case ME_MAINVOLUME:
				song->channel[song->current_event->channel].volume=song->current_event->a;
				adjust_volume(song->current_event->channel);
				break;

			case ME_MASTERVOLUME:
				adjust_master_volume(song->current_event->a + (song->current_event->b <<7));
				break;

			case ME_REVERBERATION:
				song->channel[song->current_event->channel].reverberation=song->current_event->a;
				break;

			case ME_CHORUSDEPTH:
				song->channel[song->current_event->channel].chorusdepth=song->current_event->a;
				break;

			case ME_PAN:
				song->channel[song->current_event->channel].panning=song->current_event->a;
				if (adjust_panning_immediately)
					adjust_panning(song->current_event->channel);
				break;

			case ME_EXPRESSION:
				song->channel[song->current_event->channel].expression=song->current_event->a;
				adjust_volume(song->current_event->channel);
				break;

			case ME_PROGRAM:
				/* if (ISDRUMCHANNEL(song->current_event->channel)) */
				if (song->channel[song->current_event->channel].kit)
				{
					/* Change drum set */
					song->channel[song->current_event->channel].bank=song->current_event->a;
				}
				else
				{
					song->channel[song->current_event->channel].program=song->current_event->a;
				}
				break;

			case ME_SUSTAIN:
				song->channel[song->current_event->channel].sustain=song->current_event->a;
				if (!song->current_event->a)
					drop_sustain(song->current_event->channel);
				break;

			case ME_RESET_CONTROLLERS:
				reset_controllers(song->current_event->channel);
				break;

			case ME_ALL_NOTES_OFF:
				all_notes_off(song->current_event->channel);
				break;

			case ME_ALL_SOUNDS_OFF:
				all_sounds_off(song->current_event->channel);
				break;

			case ME_HARMONICCONTENT:
				song->channel[song->current_event->channel].harmoniccontent=song->current_event->a;
				break;

			case ME_RELEASETIME:
				song->channel[song->current_event->channel].releasetime=song->current_event->a;
				break;

			case ME_ATTACKTIME:
				song->channel[song->current_event->channel].attacktime=song->current_event->a;
				break;

			case ME_BRIGHTNESS:
				song->channel[song->current_event->channel].brightness=song->current_event->a;
				break;

			case ME_TONE_BANK:
				song->channel[song->current_event->channel].bank=song->current_event->a;
				break;


			case ME_TONE_KIT:
				if (song->current_event->a==SFX_BANKTYPE)
				{
					song->channel[song->current_event->channel].sfx=SFXBANK;
					song->channel[song->current_event->channel].kit=0;
				}
				else
				{
					song->channel[song->current_event->channel].sfx=0;
					song->channel[song->current_event->channel].kit=song->current_event->a;
				}
				break;

			case ME_EOT:
				/* Give the last notes a couple of seconds to decay  */
				ctl->cmsg(CMSG_INFO, VERB_VERBOSE,
					"Playing time: ~%d seconds", song->current_sample/OUTPUT_RATE+2);
				ctl->cmsg(CMSG_INFO, VERB_VERBOSE,
					"Notes cut: %d", song->cut_notes);
				ctl->cmsg(CMSG_INFO, VERB_VERBOSE,
					"Notes lost totally: %d", song->lost_notes);
				song->playing = 0;
				return stream_start;
			}
			song->current_event++;
		}
		if (song->current_event->time > end_sample)
			conv_count = end_sample - song->current_sample;
		else
			conv_count = song->current_event->time - song->current_sample;

		while (conv_count)
		{
			int comp_count = conv_count;
			if (comp_count > song->buffer_size)
				comp_count = song->buffer_size;
			do_compute_data(comp_count);
			s32tos16((char*)stream + stream_start * sample_size, song->common_buffer,
				2 * comp_count);
			conv_count -= comp_count;
			stream_start += comp_count;
		}
	}
	return stream_start;
}


void Timidity_SetVolume(int volume)
{
	int i;
	if (volume > MAX_AMPLIFICATION)
		song->amplification = MAX_AMPLIFICATION;
	else if (volume < 0)
		song->amplification = 0;
	else
		song->amplification = volume;
	adjust_amplification();
	for (i = 0; i < song->voices; i++)
		if (song->voice[i].status != VOICE_FREE)
		{
			recompute_amp(i);
			apply_envelope_to_amp(i);
		}
}

MidiSong *Timidity_LoadSongMem(void* data, int size)
{
	int32 events;

	/* Allocate memory for the song */
	song = (MidiSong *)safe_malloc(sizeof(*song));
	memset(song, 0, sizeof(*song));
	song->amplification = DEFAULT_AMPLIFICATION;
	memcpy(song->tonebank, master_tonebank, sizeof(master_tonebank));
	memcpy(song->drumset, master_drumset, sizeof(master_drumset));
	song->default_program = DEFAULT_PROGRAM;
	song->buffer_size = 2 * 1024;
	song->resample_buffer = (resample_t*)safe_malloc(song->buffer_size * sizeof(resample_t) + 100);
	song->common_buffer = (int32*)safe_malloc(song->buffer_size * 2 * sizeof(int32));
	song->voices = DEFAULT_VOICES;
	song->drumchannels = DEFAULT_DRUMCHANNELS;
	if (!song->control_ratio)
	{
		song->control_ratio = OUTPUT_RATE / CONTROLS_PER_SECOND;
		if (song->control_ratio < 1)
			song->control_ratio = 1;
		else if (song->control_ratio > MAX_CONTROL_RATIO)
			song->control_ratio = MAX_CONTROL_RATIO;
	}

	if (*def_instr_name)
		set_default_instrument(def_instr_name);

	/* Open the file */
	song->events = read_midi_mem(data, size, &events, &song->samples);

	/* Make sure everything is okay */
	if (!song->events)
	{
		free(song);
		song = NULL;
	}
	return song;
}

void Timidity_Start(MidiSong *Asong)
{
	song = Asong;
	load_missing_instruments();
	adjust_amplification();
	sample_count = song->samples;
	song->lost_notes = song->cut_notes = 0;

	skip_to(0);
	song->playing = 1;
}

int Timidity_Active()
{
	return(song->playing);
}

void Timidity_Stop()
{
	song->playing = 0;
}

void Timidity_FreeSong(MidiSong *song)
{
	if (free_instruments_afterwards)
		free_instruments();

	free(song->events);
	free(song->resample_buffer);
	free(song->common_buffer);
	free(song);
}

void Timidity_Close()
{
	free_instruments();
	free_pathlist();
}

};
