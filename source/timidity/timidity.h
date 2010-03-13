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

#include <math.h>

namespace LibTimidity
{

#if (defined(WIN32) || defined(_WIN32)) && !defined(__WIN32__)
#define __WIN32__
#endif

/* Acoustic Grand Piano seems to be the usual default instrument. */
#define DEFAULT_PROGRAM 0

/* 9 here is MIDI channel 10, which is the standard percussion channel.
   Some files (notably C:\WINDOWS\CANYON.MID) think that 16 is one too. 
   On the other hand, some files know that 16 is not a drum channel and
   try to play music on it. This is now a runtime option, so this isn't
   a critical choice anymore. */
#define DEFAULT_DRUMCHANNELS (1<<9)
//#define DEFAULT_DRUMCHANNELS ((1<<9) | (1<<15))

/* Default sampling rate, default polyphony, and maximum polyphony.
   All but the last can be overridden from the command line. */
/* #define DEFAULT_VOICES	32 */
#define DEFAULT_VOICES	256
#define MAX_VOICES		256
#define MAXCHAN			16
#define MAXNOTE			128

/* 1000 here will give a control ratio of 22:1 with 22 kHz output.
   Higher CONTROLS_PER_SECOND values allow more accurate rendering
   of envelopes and tremolo. The cost is CPU time. */
#define CONTROLS_PER_SECOND 1000

/* Make envelopes twice as fast. Saves ~20% CPU time (notes decay
   faster) and sounds more like a GUS. There is now a command line
   option to toggle this as well. */
/* #define FAST_DECAY */

/* How many bits to use for the fractional part of sample positions.
   This affects tonal accuracy. The entire position counter must fit
   in 32 bits, so with FRACTION_BITS equal to 12, the maximum size of
   a sample is 1048576 samples (2 megabytes in memory). The GUS gets
   by with just 9 bits and a little help from its friends...
   "The GUS does not SUCK!!!" -- a happy user :) */
#define FRACTION_BITS 12

/* For some reason the sample volume is always set to maximum in all
   patch files. Define this for a crude adjustment that may help
   equalise instrument volumes. */
#define ADJUST_SAMPLE_VOLUMES

/* The number of samples to use for ramping out a dying note. Affects
   click removal. */
#define MAX_DIE_TIME 20

/**************************************************************************/
/* Anything below this shouldn't need to be changed unless you're porting
   to a new machine with other than 32-bit, big-endian words. */
/**************************************************************************/

#if MACOSX
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN
#define BIG_ENDIAN 1
#endif

}

/* Byte order, defined in <machine/endian.h> for FreeBSD and DEC OSF/1 */
#ifdef DEC
#include <machine/endian.h>
#endif

#ifdef linux
/*
 * Byte order is defined in <bytesex.h> as __BYTE_ORDER, that need to
 * be checked against __LITTLE_ENDIAN and __BIG_ENDIAN defined in <endian.h>
 * <endian.h> includes automagically <bytesex.h>
 * for Linux.
 */
#include <endian.h>

/*
 * We undef the two things to start with a clean situation
 * (oddly enough, <endian.h> defines under certain conditions
 * the two things below, as __LITTLE_ENDIAN and __BIG_ENDIAN, that
 * are useless for our plans)
 */
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN

# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define LITTLE_ENDIAN
# elif __BYTE_ORDER == __BIG_ENDIAN
#  define BIG_ENDIAN
# else
# error No byte sex defined
# endif
#endif /* linux */

namespace LibTimidity
{

/* Win32 on Intel machines */
#ifdef __WIN32__
#  define LITTLE_ENDIAN
#endif

#ifdef i386
#define LITTLE_ENDIAN
#endif

/* Instrument files are little-endian, MIDI files big-endian, so we
   need to do some conversions. */

#define XCHG_SHORT(x)	((((x)&0xFF)<<8) | (((x)>>8)&0xFF))
#define XCHG_LONG(x)	((((x)&0xFF)<<24) | \
						(((x)&0xFF00)<<8) | \
						(((x)&0xFF0000)>>8) | \
						(((x)>>24)&0xFF))

#ifdef LITTLE_ENDIAN
#define LE_SHORT(x)		x
#define LE_LONG(x)		x
#define BE_SHORT(x)		XCHG_SHORT(x)
#define BE_LONG(x)		XCHG_LONG(x)
#else
#define BE_SHORT(x)		x
#define BE_LONG(x)		x
#define LE_SHORT(x)		XCHG_SHORT(x)
#define LE_LONG(x)		XCHG_LONG(x)
#endif

/* change FRACTION_BITS above, not these */
#define INTEGER_BITS		(32 - FRACTION_BITS)
#define INTEGER_MASK		(0xFFFFFFFF << FRACTION_BITS)
#define FRACTION_MASK		(~ INTEGER_MASK)
#define MAX_SAMPLE_SIZE		(1 << INTEGER_BITS)

/* This is enforced by some computations that must fit in an int */
#define MAX_CONTROL_RATIO	255

#define MAX_AMPLIFICATION	800

#define CONFIG_FILE			"timidity.cfg"

#define FINAL_VOLUME(v) (v)

#define FSCALE(a,b)		(float)((a) * (double)(1<<(b)))
#define FSCALENEG(a,b)	(float)((a) * (1.0L / (double)(1<<(b))))

/* Vibrato and tremolo Choices of the Day */
#define SWEEP_TUNING				38
#define VIBRATO_AMPLITUDE_TUNING	1.0L
#define VIBRATO_RATE_TUNING			38
#define TREMOLO_AMPLITUDE_TUNING	1.0L
#define TREMOLO_RATE_TUNING			38

#define SWEEP_SHIFT		16
#define RATE_SHIFT		5

#define VIBRATO_SAMPLE_INCREMENTS	32

#ifndef PI
  #define PI 3.14159265358979323846
#endif

/* Noise modes for open_file */
#define OF_SILENT	0
#define OF_NORMAL	1
#define OF_VERBOSE	2

#define CMSG_INFO		0
#define CMSG_WARNING	1
#define CMSG_ERROR		2
#define CMSG_FATAL		3
#define CMSG_TEXT		4

#define VERB_NORMAL			0
#define VERB_VERBOSE		1
#define VERB_NOISY			2
#define VERB_DEBUG			3
#define VERB_DEBUG_SILLY	4

/* Bits in modes: */
#define MODES_16BIT			(1<<0)
#define MODES_UNSIGNED		(1<<1)
#define MODES_LOOPING		(1<<2)
#define MODES_PINGPONG		(1<<3)
#define MODES_REVERSE		(1<<4)
#define MODES_SUSTAIN		(1<<5)
#define MODES_ENVELOPE		(1<<6)
#define MODES_FAST_RELEASE	(1<<7)

#define INST_GUS	0
#define INST_SF2	1
#define INST_DLS	2

#define FONT_NORMAL		0
#define FONT_FFF		1
#define FONT_SBK		2
#define FONT_TONESET	3
#define FONT_DRUMSET	4
#define FONT_PRESET		5

/* A hack to delay instrument loading until after reading the
   entire MIDI file. */
#define MAGIC_LOAD_INSTRUMENT	((InstrumentLayer *)(-1))

#define MAXPROG		128
#define MAXBANK		130
#define SFXBANK		(MAXBANK-1)
#define SFXDRUM1	(MAXBANK-2)
#define SFXDRUM2	(MAXBANK-1)
#define XGDRUM		1

#define SPECIAL_PROGRAM		-1

/* Midi events */
#define ME_NONE 				0
#define ME_NOTEON				1
#define ME_NOTEOFF				2
#define ME_KEYPRESSURE			3
#define ME_MAINVOLUME			4
#define ME_PAN					5
#define ME_SUSTAIN				6
#define ME_EXPRESSION			7
#define ME_PITCHWHEEL			8
#define ME_PROGRAM				9
#define ME_TEMPO				10
#define ME_PITCH_SENS			11

#define ME_ALL_SOUNDS_OFF		12
#define ME_RESET_CONTROLLERS	13
#define ME_ALL_NOTES_OFF		14
#define ME_TONE_BANK			15

#define ME_LYRIC				16
#define ME_TONE_KIT				17
#define ME_MASTERVOLUME			18
#define ME_CHANNEL_PRESSURE		19

#define ME_HARMONICCONTENT		71
#define ME_RELEASETIME			72
#define ME_ATTACKTIME			73
#define ME_BRIGHTNESS			74

#define ME_REVERBERATION		91
#define ME_CHORUSDEPTH			93

#define ME_EOT					99

#define SFX_BANKTYPE	64

/* Causes the instrument's default panning to be used. */
#define NO_PANNING		-1
/* envelope points */
#define MAXPOINT		7

/* Voice status options: */
#define VOICE_FREE		0
#define VOICE_ON		1
#define VOICE_SUSTAINED	2
#define VOICE_OFF		3
#define VOICE_DIE		4

/* Voice panned options: */
#define PANNED_MYSTERY	0
#define PANNED_LEFT		1
#define PANNED_RIGHT	2
#define PANNED_CENTRE	3
/* Anything but PANNED_MYSTERY only uses the left volume */

/* Envelope stages: */
#define ATTACK		0
#define HOLD		1
#define DECAY		2
#define RELEASE		3
#define RELEASEB	4
#define RELEASEC	5
#define DELAY		6

#define ISDRUMCHANNEL(s, c)		(((s)->drumchannels & (1<<(c))))

#define sine(x)		(sin((2*PI/1024.0) * (x)))

#define OUTPUT_RATE		44100

/* In percent. */
/* #define DEFAULT_AMPLIFICATION 	70 */
/* #define DEFAULT_AMPLIFICATION 	50 */
#define DEFAULT_AMPLIFICATION 	30

/* You could specify a complete path, e.g. "/etc/timidity.cfg", and
   then specify the library directory in the configuration file. */
#if defined(DJGPP) || defined(__WIN32__) || defined(__OS2__)
#define DEFAULT_PATH	"\\TIMIDITY"
#else
#define DEFAULT_PATH	"/usr/local/lib/timidity"
#endif

/* These affect general volume */
#define GUARD_BITS 3
#define AMP_BITS (15-GUARD_BITS)

/* The path separator (D.M.) */
#if defined(DJGPP) || defined(__WIN32__) || defined(__OS2__)
#  define PATH_SEP '\\'
#  define PATH_STRING "\\"
#else
#  define PATH_SEP '/'
#  define PATH_STRING "/"
#endif

/* When a patch file can't be opened, one of these extensions is
   appended to the filename and the open is tried again.
 */
#define PATCH_EXT_LIST { ".pat", 0 }

#define MAX_AMP_VALUE ((1<<(AMP_BITS+1))-1)

#define SINE_CYCLE_LENGTH 1024

typedef unsigned int uint32;
typedef int int32; 
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uint8;
typedef char int8;

typedef double FLOAT_T;

typedef int16 sample_t;
typedef int32 final_volume_t;

struct ControlMode
{
	int (*cmsg)(int type, int verbosity_level, const char *fmt, ...);
};

struct PathList
{
	char*		path;
	PathList*	next;
};

struct Sample
{
	int32		loop_start, loop_end, data_length,
				sample_rate, low_vel, high_vel, low_freq, high_freq, root_freq;
	int32		envelope_rate[7], envelope_offset[7],
				modulation_offset[7];
	FLOAT_T		volume;
	sample_t*	data;
	int32 		tremolo_sweep_increment, tremolo_phase_increment, 
				vibrato_sweep_increment, vibrato_control_ratio;
	uint8		tremolo_depth, vibrato_depth,
				modes;
	uint8		freq_centre;
	int8		panning, note_to_use, exclusiveClass;
};

struct Instrument
{
	int				type;
	int				samples;
	Sample*			sample;
	int				left_samples;
	Sample*			left_sample;
	int				right_samples;
	Sample*			right_sample;
};

struct InstrumentLayer
{
	uint8				lo, hi;
	int					size;
	Instrument*			instrument;
	InstrumentLayer*	next;
};

struct ToneBankElement
{
	char*				name;
	InstrumentLayer*	layer;
	int					font_type, sf_ix, last_used, tuning;
	int					note, amp, pan, strip_loop, strip_envelope, strip_tail;
};

struct ToneBank
{
	char*				name;
	ToneBankElement		tone[MAXPROG];
};

struct Channel
{
	int
		bank, program, volume, sustain, panning, pitchbend, expression, 
		mono, /* one note only on this channel -- not implemented yet */
		/* new stuff */
		variationbank, reverberation, chorusdepth, harmoniccontent,
		releasetime, attacktime, brightness, kit, sfx,
		/* end new */
		pitchsens;
	FLOAT_T
		pitchfactor; /* precomputed pitch bend factor to save some fdiv's */
	char transpose;
	char *name;
};

struct Voice
{
	uint8
		status, channel, note, velocity, clone_type;
	Sample* sample;
	Sample* left_sample;
	Sample* right_sample;
	int32
		orig_frequency, frequency,
		sample_offset, sample_increment;
	int32
		envelope_volume, envelope_target, envelope_increment;
	int32
		tremolo_sweep, tremolo_sweep_position,
		tremolo_phase, tremolo_phase_increment,
		vibrato_sweep, vibrato_sweep_position,
		echo_delay_count;
	int32
		echo_delay;

	final_volume_t left_mix, right_mix;

	FLOAT_T
		left_amp, right_amp, volume, tremolo_volume;
	int32
		vibrato_sample_increment[VIBRATO_SAMPLE_INCREMENTS];
	int32
		envelope_rate[MAXPOINT];
	int32
		vibrato_phase, vibrato_control_ratio, vibrato_control_counter,
		envelope_stage, control_counter, panning, panned;
};

struct MidiEvent
{
	int32		time;
	uint8		channel, type, a, b;
};

struct MidiEventList
{
	MidiEvent		event;
	MidiEventList*	next;
};

struct DLS_Data;

struct MidiSong
{
	int					playing;
	uint8*				midi_image;
	uint32				image_left;
	FLOAT_T				master_volume;
	int32				amplification;
	DLS_Data*			patches;
	ToneBank*			tonebank[MAXBANK];
	ToneBank*			drumset[MAXBANK];
	/* This is a special instrument, used for all melodic programs */
	InstrumentLayer*	default_instrument;
	/* This is only used for tracks that don't specify a program */
	int					default_program;
	int					buffer_size;
	sample_t*			resample_buffer;
	int32*				common_buffer;
	/* These would both fit into 32 bits, but they are often added in
		large multiples, so it's simpler to have two roomy ints */
	/*samples per MIDI delta-t*/
	int32				sample_increment;
	int32				sample_correction;
	Channel				channel[MAXCHAN];
	Voice				voice[MAX_VOICES];
	int					voices;
	int32				drumchannels;
	int32				control_ratio;
	int32				lost_notes;
	int32				cut_notes;
	int32				samples;
	MidiEvent*			events;
	MidiEvent*			current_event;
	MidiEventList*		evlist;
	int32				current_sample;
	int32				event_count;
	int32				at;
};

extern FILE *open_file(const char *name, int decompress, int noise_mode);
extern void add_to_pathlist(const char *s);
extern void free_pathlist();
extern void close_file(FILE *fp);
extern void skip(FILE *fp, size_t len);
extern void *safe_malloc(size_t count);

extern int load_missing_instruments(MidiSong* song);
extern void free_instruments(MidiSong* song);
extern int set_default_instrument(MidiSong* song, const char *name);

extern DLS_Data *Timidity_LoadDLS(FILE *src);
extern void Timidity_FreeDLS(DLS_Data *patches);
extern InstrumentLayer* load_instrument_dls(MidiSong *song, int drum, int bank, int instrument);

extern int32 convert_vibrato_sweep(uint8 sweep, int32 vib_control_ratio);
extern int32 convert_vibrato_rate(uint8 rate);

extern int play_midi(MidiEvent *el, int32 events, int32 samples);
extern int play_midi_file(char *fn);
extern void dumb_pass_playing_list(int number_of_files, char *list_of_files[]);

MidiEvent* read_midi_mem(MidiSong* song, void* mimage, int msize, int32* count, int32* sp);

extern sample_t* resample_voice(MidiSong* song, int v, int32* countptr);
extern void pre_resample(Sample* sp);

extern void mix_voice(MidiSong* song, int32* buf, int v, int32 c);
extern int recompute_envelope(MidiSong* song, int v);
extern void apply_envelope_to_amp(MidiSong* song, int v);

extern int Timidity_Init();
extern void Timidity_SetVolume(MidiSong* song, int volume);
extern int Timidity_PlaySome(MidiSong* song, void* stream, int samples);
extern MidiSong *Timidity_LoadSongMem(void* data, int size, DLS_Data* patches);
extern void Timidity_Start(MidiSong* song);
extern int Timidity_Active(MidiSong* song);
extern void Timidity_Stop(MidiSong* song);
extern void Timidity_FreeSong(MidiSong* song);
extern void Timidity_Close();

extern char current_filename[];

extern const int32 freq_table[];
extern const double vol_table[];
extern const double bend_fine[];
extern const double bend_coarse[];

#define XMAPMAX 800
extern int xmap[XMAPMAX][5];

extern ToneBank standard_tonebank, standard_drumset;
extern ToneBank*			master_tonebank[MAXBANK];
extern ToneBank*			master_drumset[MAXBANK];

extern int fast_decay;
extern int free_instruments_afterwards;

extern int current_tune_number;
extern int max_patch_memory;
extern int current_patch_memory;

extern char def_instr_name[256];

extern signed char drumvolume[MAXCHAN][MAXNOTE];
extern signed char drumpanpot[MAXCHAN][MAXNOTE];
extern signed char drumreverberation[MAXCHAN][MAXNOTE];
extern signed char drumchorusdepth[MAXCHAN][MAXNOTE];

extern int XG_System_On;

extern int XG_System_reverb_type;
extern int XG_System_chorus_type;

extern ControlMode*		ctl;

};
