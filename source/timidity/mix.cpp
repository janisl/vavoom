/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    Suddenly, you realise that this program is free software; you get
    an overwhelming urge to redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received another copy of the GNU General Public
    License along with this program; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    I bet they'll be amazed.

    mix.c */

#include <math.h>
#include <stdlib.h>

#include "timidity.h"

namespace LibTimidity
{
/* Returns 1 if envelope runs out */
int recompute_envelope(MidiSong* song, int v)
{
	int stage = song->voice[v].envelope_stage;

	if (stage >= DELAY)
	{
		/* Envelope ran out. */
		song->voice[v].status = VOICE_FREE;
		return 1;
	}

	if (song->voice[v].sample->modes & MODES_ENVELOPE)
	{
		if (song->voice[v].status == VOICE_ON || song->voice[v].status == VOICE_SUSTAINED)
		{
			if (stage > DECAY)
			{
				/* Freeze envelope until note turns off. Trumpets want this. */
				song->voice[v].envelope_increment = 0;
				return 0;
			}
		}
	}
	song->voice[v].envelope_stage = stage + 1;

	if (song->voice[v].envelope_volume == song->voice[v].sample->envelope_offset[stage])
	{
		return recompute_envelope(song, v);
	}
	song->voice[v].envelope_target = song->voice[v].sample->envelope_offset[stage];
	song->voice[v].envelope_increment = song->voice[v].sample->envelope_rate[stage];

	if (song->voice[v].envelope_target < song->voice[v].envelope_volume)
	{
		song->voice[v].envelope_increment = -song->voice[v].envelope_increment;
	}

	return 0;
}

void apply_envelope_to_amp(MidiSong* song, int v)
{
	float lamp = song->voice[v].left_amp;
	int32 la;

	if (song->voice[v].panned == PANNED_MYSTERY)
	{
		float ramp = song->voice[v].right_amp;

		if (song->voice[v].tremolo_phase_increment)
		{
			float tv = song->voice[v].tremolo_volume;
			lamp *= tv;
			ramp *= tv;
		}

		if (song->voice[v].sample->modes & MODES_ENVELOPE)
		{
			float ev = (float)vol_table[song->voice[v].envelope_volume >> 23];
			lamp *= ev;
			ramp *= ev;
		}

		la = (int32)FSCALE(lamp,AMP_BITS);
		int32 ra = (int32)FSCALE(ramp,AMP_BITS);

		if (la > MAX_AMP_VALUE)
		{
			la = MAX_AMP_VALUE;
		}
		if (ra > MAX_AMP_VALUE)
		{
			ra = MAX_AMP_VALUE;
		}

		song->voice[v].left_mix = la;
		song->voice[v].right_mix = ra;
	}
	else
	{
		if (song->voice[v].tremolo_phase_increment)
		{
			lamp *= song->voice[v].tremolo_volume;
		}
		if (song->voice[v].sample->modes & MODES_ENVELOPE)
		{
			lamp *= (float)vol_table[song->voice[v].envelope_volume >> 23];
		}
		la = (int32)FSCALE(lamp,AMP_BITS);

		if (la > MAX_AMP_VALUE)
		{
			la = MAX_AMP_VALUE;
		}
		song->voice[v].left_mix = la;
	}
}

static int update_envelope(MidiSong* song, int v)
{
	song->voice[v].envelope_volume += song->voice[v].envelope_increment;

	/* Why is there no ^^ operator?? */
	if (((song->voice[v].envelope_increment < 0) &&
		(song->voice[v].envelope_volume <= song->voice[v].envelope_target)) ||
		((song->voice[v].envelope_increment > 0) &&
		(song->voice[v].envelope_volume >= song->voice[v].envelope_target)))
	{
		song->voice[v].envelope_volume = song->voice[v].envelope_target;

		if (recompute_envelope(song, v))
		{
			return 1;
		}
	}

	return 0;
}

static void update_tremolo(MidiSong* song, int v)
{
	int32 depth = song->voice[v].sample->tremolo_depth << 7;

	if (song->voice[v].tremolo_sweep)
	{
		/* Update sweep position */
		song->voice[v].tremolo_sweep_position += song->voice[v].tremolo_sweep;

		if (song->voice[v].tremolo_sweep_position >= (1 << SWEEP_SHIFT))
		{
			song->voice[v].tremolo_sweep = 0; /* Swept to max amplitude */
		}
		else
		{
			/* Need to adjust depth */
			depth *= song->voice[v].tremolo_sweep_position;
			depth >>= SWEEP_SHIFT;
		}
	}
	song->voice[v].tremolo_phase += song->voice[v].tremolo_phase_increment;

	song->voice[v].tremolo_volume =
		(float)(1.0 - FSCALENEG((sine(song->voice[v].tremolo_phase >> RATE_SHIFT) /*+ 1.0*/) *
		depth * TREMOLO_AMPLITUDE_TUNING, 17));

	/* I'm not sure about the +1.0 there -- it makes tremoloed voices'
		volumes on average the lower the higher the tremolo amplitude. */
}

/* Returns 1 if the note died */
static int update_signal(MidiSong* song, int v)
{
	if (song->voice[v].envelope_increment && update_envelope(song, v))
	{
		return 1;
	}

	if (song->voice[v].tremolo_phase_increment)
	{
		update_tremolo(song, v);
	}
	apply_envelope_to_amp(song, v);

	return 0;
}

static void mix_mystery_signal(MidiSong* song, sample_t* sp, int32* lp, int v, int count)
{
	Voice *vp = song->voice + v;
	final_volume_t 
		left = vp->left_mix, 
		right = vp->right_mix;
	int cc;
	sample_t s;

	if (!(cc = vp->control_counter))
	{
		cc = song->control_ratio;

		if (update_signal(song, v))
		{
			return;	/* Envelope ran out */
		}
		left = vp->left_mix;
		right = vp->right_mix;
	}

	while (count)
	{
		if (cc < count)
		{
			count -= cc;

			while (cc--)
			{
				s = *sp++;
				lp[0] += left * s;
				lp[1] += right * s;
				lp += 2;
			}
			cc = song->control_ratio;

			if (update_signal(song, v))
			{
				return;	/* Envelope ran out */
			}
			left = vp->left_mix;
			right = vp->right_mix;
		}
		else
		{
			vp->control_counter = cc - count;

			while (count--)
			{
				s = *sp++;
				lp[0] += left * s;
				lp[1] += right * s;
				lp += 2;
			}

			return;
		}
	}
}

static void mix_centre_signal(MidiSong* song, sample_t* sp, int32* lp, int v, int count)
{
	Voice *vp = song->voice + v;
	final_volume_t 
		left = vp->left_mix;
	int cc;
	sample_t s;

	if (!(cc = vp->control_counter))
	{
		cc = song->control_ratio;

		if (update_signal(song, v))
		{
			return;	/* Envelope ran out */
		}
		left = vp->left_mix;
	}

	while (count)
	{
		if (cc < count)
		{
			count -= cc;
			while (cc--)
			{
				s = *sp++;
				lp[0] += left * s;
				lp[1] += left * s;
				lp += 2;
			}
			cc = song->control_ratio;
			if (update_signal(song, v))
			{
				return;	/* Envelope ran out */
			}
			left = vp->left_mix;
		}
		else
		{
			vp->control_counter = cc - count;

			while (count--)
			{
				s = *sp++;
				lp[0] += left * s;
				lp[1] += left * s;
				lp += 2;
			}

			return;
		}
	}
}

static void mix_single_signal(MidiSong* song, sample_t* sp, int32* lp, int v, int count)
{
	Voice *vp = song->voice + v;
	final_volume_t 
		left = vp->left_mix;
	int cc;
	sample_t s;

	if (!(cc = vp->control_counter))
	{
		cc = song->control_ratio;
		if (update_signal(song, v))
		{
			return;	/* Envelope ran out */
		}
		left = vp->left_mix;
	}

	while (count)
	{
		if (cc < count)
		{
			count -= cc;

			while (cc--)
			{
				s = *sp++;
				lp[0] += left * s;
				lp += 2;
			}
			cc = song->control_ratio;

			if (update_signal(song, v))
			{
				return;	/* Envelope ran out */
			}
			left = vp->left_mix;
		}
		else
		{
			vp->control_counter = cc - count;

			while (count--)
			{
				s = *sp++;
				lp[0] += left * s;
				lp += 2;
			}

			return;
		}
	}
}

static void mix_mystery(MidiSong* song, sample_t* sp, int32* lp, int v, int count)
{
	final_volume_t 
		left = song->voice[v].left_mix, 
		right = song->voice[v].right_mix;
	sample_t s;

	while (count--)
	{
		s = *sp++;
		lp[0] += left * s;
		lp[1] += right * s;
		lp += 2;
	}
}

static void mix_centre(MidiSong* song, sample_t* sp, int32* lp, int v, int count)
{
	final_volume_t 
		left=song->voice[v].left_mix;
	sample_t s;

	while (count--)
	{
		s = *sp++;
		lp[0] += left * s;
		lp[1] += left * s;
		lp += 2;
	}
}

static void mix_single(MidiSong* song, sample_t* sp, int32* lp, int v, int count)
{
	final_volume_t left = song->voice[v].left_mix;
	sample_t s;

	while (count--)
	{
		s = *sp++;
		lp[0] += left * s;
		lp += 2;
	}
}

/* Ramp a note out in c samples */
static void ramp_out(MidiSong* song, sample_t* sp, int32* lp, int v, int32 c)
{
	/* should be final_volume_t, but uint8 gives trouble. */
	int32 left, li;

	sample_t s = 0; /* silly warning about uninitialised s */

	/* Fix by James Caldwell */
	if (c == 0)
	{
		c = 1;
	}
	left = song->voice[v].left_mix;
	li = -(left/c);

	if (!li)
	{
		li = -1;
	}

	if (song->voice[v].panned == PANNED_MYSTERY)
	{
		int32 right=song->voice[v].right_mix;
		int32 ri = -(right / c);

		while (c--)
		{
			left += li;

			if (left < 0)
			{
				left = 0;
			}
			right += ri;

			if (right < 0)
			{
				right = 0;
			}
			s = *sp++;
			lp[0] += left * s;
			lp[1] += right * s;
			lp += 2;
		}
	}
	else if (song->voice[v].panned == PANNED_CENTRE)
	{
		while (c--)
		{
			left += li;

			if (left < 0)
			{
				return;
			}
			s = *sp++;	
			lp[0] += left * s;
			lp[1] += left * s;
			lp += 2;
		}
	}
	else if (song->voice[v].panned == PANNED_LEFT)
	{
		while (c--)
		{
			left += li;

			if (left < 0)
			{
				return;
			}
			s = *sp++;
			lp[0] += left * s;
			lp += 2;
		}
	}
	else if (song->voice[v].panned == PANNED_RIGHT)
	{
		while (c--)
		{
			left += li;

			if (left < 0)
			{
				return;
			}
			s = *sp++;
			lp[1] += left * s;
			lp += 2;
		}
	}
}


/**************** interface function ******************/

void mix_voice(MidiSong* song, int32* buf, int v, int32 c)
{
	Voice* vp = song->voice + v;
	int32 count = c;
	sample_t* sp;

	if (vp->status == VOICE_DIE)
	{
		if (count >= MAX_DIE_TIME)
		{
			count = MAX_DIE_TIME;
		}
		sp = resample_voice(song, v, &count);
		ramp_out(song, sp, buf, v, count);
		vp->status = VOICE_FREE;
	}
	else
	{
		sp = resample_voice(song, v, &count);
		if (vp->panned == PANNED_MYSTERY)
		{
			if (vp->envelope_increment || vp->tremolo_phase_increment)
			{
				mix_mystery_signal(song, sp, buf, v, count);
			}
			else
			{
				mix_mystery(song, sp, buf, v, count);
			}
		}
		else if (vp->panned == PANNED_CENTRE)
		{
			if (vp->envelope_increment || vp->tremolo_phase_increment)
			{
				mix_centre_signal(song, sp, buf, v, count);
			}
			else
			{
				mix_centre(song, sp, buf, v, count);
			}
		}
		else
		{
			/* It's either full left or full right. In either case,
			every other sample is 0. Just get the offset right: */
			if (vp->envelope_increment || vp->tremolo_phase_increment)
			{
				if (vp->panned == PANNED_RIGHT)
				{
					mix_single_signal(song, sp, buf + 1, v, count);
				}
				else
				{
					mix_single_signal(song, sp, buf, v, count);
				}
			}
			else
			{
				if (vp->panned == PANNED_RIGHT)
				{
					mix_single(song, sp, buf + 1, v, count);
				}
				else
				{
					mix_single(song, sp, buf, v, count);
				}
			}
		}
	}
}

};
