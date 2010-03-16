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

static void adjust_amplification(MidiSong* song)
{
	song->master_volume = (float)(song->amplification) / (float)100.0;
}

static void reset_voices(MidiSong* song)
{
	for (int i = 0; i < MAX_VOICES; i++)
		song->voice[i].status = VOICE_FREE;
}

/* Process the Reset All Controllers event */
static void reset_controllers(MidiSong* song, int c)
{
	song->channel[c].volume = 90; /* Some standard says, although the SCC docs say 0. */
	song->channel[c].expression = 127; /* SCC-1 does this. */
	song->channel[c].sustain = 0;
	song->channel[c].pitchbend = 0x2000;
	song->channel[c].pitchfactor = 0; /* to be computed */
}

static void reset_midi(MidiSong* song)
{
	for (int i = 0; i < 16; i++)
	{
		reset_controllers(song, i);
		/* The rest of these are unaffected by the Reset All Controllers event */
		song->channel[i].program = song->default_program;
		song->channel[i].panning = NO_PANNING;
		song->channel[i].pitchsens = 2;
		song->channel[i].bank = 0; /* tone bank or drum set */
	}
	reset_voices(song);
}

static void select_sample(MidiSong* song, int v, Instrument* ip, int vel)
{
	int32 f, cdiff, diff;
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
	for (i=0; i<s; i++)
	{
		if (sp->low_vel <= vel && sp->high_vel >= vel &&
			sp->low_freq <= f && sp->high_freq >= f)
		{
			song->voice[v].sample=sp;
			return;
		}
		sp++;
	}

	/* 
		No suitable sample found! We'll select the sample whose root
		frequency is closest to the one we want. (Actually we should
		probably convert the low, high, and root frequencies to MIDI note
		values and compare those.) */

	cdiff = 0x7FFFFFFF;
	closest = sp = ip->sample;
	for (i = 0; i < s; i++)
	{
		diff = sp->root_freq - f;
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

static void recompute_freq(MidiSong* song, int v)
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
				(float)(bend_fine[(i >> 5) & 0xFF] * bend_coarse[i >> 13]);
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

static void recompute_amp(MidiSong* song, int v)
{
	int32 tempamp;
	int chan = song->voice[v].channel;
	int panning = song->voice[v].panning;
	int vol = song->channel[chan].volume;
	int expr = song->channel[chan].expression;
	int vel = vcurve[song->voice[v].velocity];
	float curved_expression, curved_volume;

	curved_expression = 127.0 * vol_table[expr];

	curved_volume = 127.0 * vol_table[vol];

	tempamp= (int32)((float)vel * curved_volume * curved_expression); /* 21 bits */

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
		float refv = (double)(tempamp) * song->voice[v].sample->volume * song->master_volume;

		song->voice[v].panned = PANNED_MYSTERY;
		song->voice[v].left_amp = FSCALENEG(refv * (128-panning), 27);
		song->voice[v].right_amp = FSCALENEG(refv * panning, 27);
	}
}

static void start_note(MidiSong* song, MidiEvent* e, int i)
{
	Instrument *ip;
	int j, banknum, ch=e->channel;
	int played_note;
	int32 rt;
	int this_note = e->a;
	int this_velocity = e->b;
	int this_prog = song->channel[ch].program;

	banknum = song->channel[ch].bank;

	song->voice[i].velocity = this_velocity;

	if (ISDRUMCHANNEL(song, ch))
	{
		if (!(ip=song->drumset[banknum]->instrument[this_note]))
		{
			if (!(ip=song->drumset[0]->instrument[this_note]))
				return; /* No instrument? Then we can't play. */
		}
		if (ip->type == INST_GUS && ip->samples != 1)
		{
			ctl->cmsg(CMSG_WARNING, VERB_VERBOSE, 
				"Strange: percussion instrument with %d samples!", ip->samples);
		}
	
		if (ip->sample->note_to_use) /* Do we have a fixed pitch? */
			song->voice[i].orig_frequency=freq_table[(int)(ip->sample->note_to_use)];
		else
			song->voice[i].orig_frequency=freq_table[this_note & 0x7F];
      
		/* drums are supposed to have only one sample */
		song->voice[i].sample = ip->sample;
	}
	else
	{
		if (song->channel[ch].program == SPECIAL_PROGRAM)
			ip = song->default_instrument;
		else if (!(ip = song->tonebank[song->channel[ch].bank]->instrument[song->channel[ch].program]))
		{
			if (!(ip=song->tonebank[0]->instrument[this_prog]))
				return; /* No instrument? Then we can't play. */
		}
		if (ip->sample->note_to_use) /* Fixed-pitch instrument? */
			song->voice[i].orig_frequency = freq_table[(int)(ip->sample->note_to_use)];
		else
			song->voice[i].orig_frequency = freq_table[this_note & 0x7F];
		select_sample(song, i, ip, e->b);
	}

	played_note = song->voice[i].sample->note_to_use;
	
	if (!played_note || !ISDRUMCHANNEL(song, ch))
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
	song->voice[i].vibrato_control_ratio = song->voice[i].sample->vibrato_control_ratio;
	song->voice[i].vibrato_control_counter = song->voice[i].vibrato_phase=0;
	for (j = 0; j < VIBRATO_SAMPLE_INCREMENTS; j++)
	{
		song->voice[i].vibrato_sample_increment[j] = 0;
	}

	if (song->channel[ch].panning != NO_PANNING)
		song->voice[i].panning = song->channel[ch].panning;
	else
		song->voice[i].panning = song->voice[i].sample->panning;

	recompute_freq(song, i);
	recompute_amp(song, i);
	if (song->voice[i].sample->modes & MODES_ENVELOPE)
	{
		/* Ramp up from 0 */
		song->voice[i].envelope_stage = ATTACK;
		song->voice[i].envelope_volume = 0;
		song->voice[i].control_counter = 0;
		recompute_envelope(song, i);
	}
	else
	{
		song->voice[i].envelope_increment = 0;
	}
	apply_envelope_to_amp(song, i);
}

static void kill_note(MidiSong* song, int i)
{
	song->voice[i].status=VOICE_DIE;
}


/* Only one instance of a note can be playing on a single channel. */
static void note_on(MidiSong* song, MidiEvent* e)
{
	int i = song->voices, lowest = -1;
	int32 lv = 0x7FFFFFFF, v;

	while (i--)
	{
		if (song->voice[i].status == VOICE_FREE)
			lowest=i; /* Can't get a lower volume than silence */
		else if (song->voice[i].channel == e->channel && 
				(song->voice[i].note == e->a || song->channel[song->voice[i].channel].mono))
			kill_note(song, i);
	}

	if (lowest != -1)
	{
		/* Found a free voice. */
		start_note(song, e,lowest);
		return;
	}

	/* Look for the decaying note with the lowest volume */
	i = song->voices;
	while (i--)
	{
		if ((song->voice[i].status != VOICE_ON) &&
			(song->voice[i].status != VOICE_DIE))
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

	if (lowest != -1)
	{
		/* This can still cause a click, but if we had a free voice to
		spare for ramping down this note, we wouldn't need to kill it
		in the first place... Still, this needs to be fixed. Perhaps
		we could use a reserve of voices to play dying notes only. */
		song->cut_notes++;
		song->voice[lowest].status = VOICE_FREE;
		start_note(song, e,lowest);
	}
	else
		song->lost_notes++;
}

static void finish_note(MidiSong* song, int i)
{
	if (song->voice[i].sample->modes & MODES_ENVELOPE)
	{
		/* We need to get the envelope out of Sustain stage */
		song->voice[i].envelope_stage = RELEASE;
		song->voice[i].status = VOICE_OFF;
		recompute_envelope(song, i);
		apply_envelope_to_amp(song, i);
	}
	else
	{
		/* Set status to OFF so resample_voice() will let this voice out
			of its loop, if any. In any case, this voice dies when it
			hits the end of its data (ofs>=data_length). */
		song->voice[i].status = VOICE_OFF;
	}
}

static void note_off(MidiSong* song, MidiEvent* e)
{
	int i = song->voices;
	while (i--)
		if (song->voice[i].status == VOICE_ON &&
			song->voice[i].channel == e->channel &&
			song->voice[i].note == e->a)
		{
			if (song->channel[e->channel].sustain)
			{
				song->voice[i].status = VOICE_SUSTAINED;
			}
			else
				finish_note(song, i);
			return;
		}
}

/* Process the All Notes Off event */
static void all_notes_off(MidiSong* song, int c)
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
				finish_note(song, i);
		}
}

/* Process the All Sounds Off event */
static void all_sounds_off(MidiSong* song, int c)
{
	int i=song->voices;
	while (i--)
		if (song->voice[i].channel == c &&
			song->voice[i].status != VOICE_FREE &&
			song->voice[i].status != VOICE_DIE)
		{
			kill_note(song, i);
		}
}

static void adjust_pressure(MidiSong* song, MidiEvent* e)
{
	int i=song->voices;
	while (i--)
		if (song->voice[i].status == VOICE_ON &&
			song->voice[i].channel == e->channel &&
			song->voice[i].note == e->a)
		{
			song->voice[i].velocity = e->b;
			recompute_amp(song, i);
			apply_envelope_to_amp(song, i);
			return;
		}
}

static void drop_sustain(MidiSong* song, int c)
{
	int i = song->voices;
	while (i--)
		if (song->voice[i].status == VOICE_SUSTAINED && song->voice[i].channel == c)
			finish_note(song, i);
}

static void adjust_pitchbend(MidiSong* song, int c)
{
	int i = song->voices;
	while (i--)
		if (song->voice[i].status != VOICE_FREE && song->voice[i].channel == c)
		{
			recompute_freq(song, i);
		}
}

static void adjust_volume(MidiSong* song, int c)
{
	int i = song->voices;
	while (i--)
		if (song->voice[i].channel == c &&
			(song->voice[i].status == VOICE_ON || song->voice[i].status == VOICE_SUSTAINED))
		{
			recompute_amp(song, i);
			apply_envelope_to_amp(song, i);
		}
}

static void seek_forward(MidiSong* song, int32 until_time)
{
	reset_voices(song);
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

		case ME_PAN:
			song->channel[song->current_event->channel].panning = song->current_event->a;
			break;

		case ME_EXPRESSION:
			song->channel[song->current_event->channel].expression = song->current_event->a;
			break;

		case ME_PROGRAM:
			if (ISDRUMCHANNEL(song, song->current_event->channel))
				/* Change drum set */
				song->channel[song->current_event->channel].bank = song->current_event->a;
			else
				song->channel[song->current_event->channel].program = song->current_event->a;
			break;

		case ME_SUSTAIN:
			song->channel[song->current_event->channel].sustain = song->current_event->a;
			break;

		case ME_RESET_CONTROLLERS:
			reset_controllers(song, song->current_event->channel);
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

static void skip_to(MidiSong* song, int32 until_time)
{
	if (song->current_sample > until_time)
		song->current_sample = 0;

	reset_midi(song);
	song->current_event = song->events;

	if (until_time)
		seek_forward(song, until_time);
}

static void do_compute_data(MidiSong* song, uint32 count)
{
	int i;
	if (!count)
		return; /* (gl) */
	memset(song->common_buffer, 0, count * 2 * 4);
	for (i = 0; i < song->voices; i++)
	{
		if (song->voice[i].status != VOICE_FREE)
		{
			mix_voice(song, song->common_buffer, i, count);
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

int Timidity_PlaySome(MidiSong* song, void *stream, int samples)
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
				if (!(song->current_event->b)) /* Velocity 0? */
					note_off(song, song->current_event);
				else
					note_on(song, song->current_event);
				break;

			case ME_NOTEOFF:
				note_off(song, song->current_event);
				break;

			case ME_KEYPRESSURE:
				adjust_pressure(song, song->current_event);
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
				adjust_pitchbend(song, song->current_event->channel);
				break;

			case ME_MAINVOLUME:
				song->channel[song->current_event->channel].volume=song->current_event->a;
				adjust_volume(song, song->current_event->channel);
				break;

			case ME_PAN:
				song->channel[song->current_event->channel].panning=song->current_event->a;
				break;

			case ME_EXPRESSION:
				song->channel[song->current_event->channel].expression=song->current_event->a;
				adjust_volume(song, song->current_event->channel);
				break;

			case ME_PROGRAM:
				if (ISDRUMCHANNEL(song, song->current_event->channel))
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
					drop_sustain(song, song->current_event->channel);
				break;

			case ME_RESET_CONTROLLERS:
				reset_controllers(song, song->current_event->channel);
				break;

			case ME_ALL_NOTES_OFF:
				all_notes_off(song, song->current_event->channel);
				break;

			case ME_ALL_SOUNDS_OFF:
				all_sounds_off(song, song->current_event->channel);
				break;

			case ME_TONE_BANK:
				song->channel[song->current_event->channel].bank=song->current_event->a;
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
			do_compute_data(song, comp_count);
			s32tos16((char*)stream + stream_start * sample_size, song->common_buffer,
				2 * comp_count);
			conv_count -= comp_count;
			stream_start += comp_count;
		}
	}
	return stream_start;
}

void Timidity_SetVolume(MidiSong* song, int volume)
{
	int i;
	if (volume > MAX_AMPLIFICATION)
		song->amplification = MAX_AMPLIFICATION;
	else if (volume < 0)
		song->amplification = 0;
	else
		song->amplification = volume;
	adjust_amplification(song);
	for (i = 0; i < song->voices; i++)
		if (song->voice[i].status != VOICE_FREE)
		{
			recompute_amp(song, i);
			apply_envelope_to_amp(song, i);
		}
}

MidiSong *Timidity_LoadSongMem(void* data, int size, DLS_Data* patches)
{
	int32 events;

	/* Allocate memory for the song */
	MidiSong* song = (MidiSong *)safe_malloc(sizeof(*song));
	memset(song, 0, sizeof(*song));
	song->patches = patches;
	song->amplification = DEFAULT_AMPLIFICATION;
	memcpy(song->tonebank, master_tonebank, sizeof(master_tonebank));
	memcpy(song->drumset, master_drumset, sizeof(master_drumset));
	song->default_program = DEFAULT_PROGRAM;
	song->buffer_size = 2 * 1024;
	song->resample_buffer = (sample_t*)safe_malloc(song->buffer_size * sizeof(sample_t) + 100);
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
		set_default_instrument(song, def_instr_name);

	/* Open the file */
	song->events = read_midi_mem(song, data, size, &events, &song->samples);

	/* Make sure everything is okay */
	if (!song->events)
	{
		free(song);
		song = NULL;
	}
	return song;
}

void Timidity_Start(MidiSong *song)
{
	load_missing_instruments(song);
	adjust_amplification(song);
	song->lost_notes = song->cut_notes = 0;

	skip_to(song, 0);
	song->playing = 1;
}

int Timidity_Active(MidiSong* song)
{
	return(song->playing);
}

void Timidity_Stop(MidiSong* song)
{
	song->playing = 0;
}

void Timidity_FreeSong(MidiSong *song)
{
	if (free_instruments_afterwards)
		free_instruments(song);

	free(song->events);
	free(song->resample_buffer);
	free(song->common_buffer);
	free(song);
}

void Timidity_Close()
{
//	free_instruments(song);
	free_pathlist();
}

};
