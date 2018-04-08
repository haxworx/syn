#ifndef __AUDIO_H__
#define __AUDIO_H__
#include "stdinc.h"
#include "limits.h"

#include <SDL_audio.h>

#define SAMPLES          2048
#define CHANNELS         2

#define MAX_SOUND_COUNT  16
#define SOUND_PITCH_MAX  20000
#define SOUND_PITCH_MIN  -4400.0

#define MAX_OUTFILE_SIZE 41943040
#define MAX_WAV_FILES    10

typedef struct sound_t sound_t;
struct sound_t
{
   int      key;
   double   A;
   double   SR;
   double   pitch;
   uint8_t  silence;
   double   duration;
   int16_t  note;
   int      effects;
   bool     is_wavefile;
   uint32_t wavefile_pos;
   uint32_t wavefile_len;
   void    *data;
};

typedef struct _synth_t
{
   uint8_t       volume;
   int16_t       bass, mid, treble;
   int32_t       pitch;

   char          working_directory[PATH_MAX];

   bool          wavefile_playing;
   bool          is_recording;
   bool          continuous;
   int16_t       sound_range_start;
   uint32_t      start_time;

   char         *recording_path;

   int16_t       buffer[SAMPLES * CHANNELS];
   int           counter;

   int16_t       output_buffer[MAX_OUTFILE_SIZE];
   int           output_buffer_index;
   int           output_buffer_len;

   char         *wav_files[MAX_WAV_FILES];
   uint16_t      wav_files_count;

   SDL_AudioSpec wav_spec;
   int16_t      *waveptr;

   sound_t      *sound;
   sound_t      *sounds[MAX_SOUND_COUNT];
   uint8_t       sounds_count;
} synth_t;


/* this is a wav_h for RIFF/WAVE file */
struct wav_file_hdr_t
{
   char      chunk_id[4];
   int       chunk_size;
   char      format[4];
   char      subchunk1_id[4];
   int       subchunk1_size;
   short int audio_format;
   short int num_channels;
   int       sample_rate;
   int       byte_rate;
   short int block_align;
   short int bits_per_sample;
   char      subchunk2_id[4];
   int       subchunk2_size;
};

synth_t *synth_new(void);
void     synth_shutdown(synth_t *synth);

sound_t *sound_new(synth_t *synth);

void     recording_start(synth_t *synth);
void     recording_stop(synth_t *synth);

void     reset_defaults(synth_t *synth);
void     process_sound(synth_t *synth);

void     play_music_file(synth_t *synth, const char *filename);
int      keyboard_to_note(synth_t *synth, int k);

void     wave_file_play(synth_t *synth, const char *filename);
bool     wave_file_del(synth_t *synth);
void     wave_files_count(synth_t *synth);

#define SYNTH_DALEK          0x0
#define SYNTH_TRIANGLE       0x1
#define SYNTH_DRUM_N_BASS    0x2
#define SYNTH_BASSY          0x3
#define SYNTH_BOWSERS_CASTLE 0x4
#define SYNTH_VINYL_SCRATCH  0x5
#define SYNTH_LAZER_QUEST    0x6
#define SYNTH_CREEPY_FUZZ    0x7
#define SYNTH_GIRLS_AND_BOYS 0x8

#endif
