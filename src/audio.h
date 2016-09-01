#ifndef __AUDIO_H__
#define __AUDIO_H__
#include "stdinc.h"

#include <SDL_audio.h>

#define SAMPLES 2048
#define CHANNELS 2

typedef struct sound_t sound_t;
struct sound_t {
    int key;
    double A;
    double SR;
    uint32_t volume;
    double pitch;
    uint8_t silence;
    double duration;
    int16_t note;
    int effects;
    bool is_recording;
    bool is_wavefile;
    uint32_t wavefile_pos;
    uint32_t wavefile_len;
    void *data;
};

/* Position counter */
int counter;

int16_t global_bass, global_mid, global_treble;
int32_t global_pitch;

bool wavefile_playing;
bool recording_enabled;
bool synth_continuous;

uint32_t start_time;

/* this is a wav_h for RIFF/WAVE file */
struct wav_file_hdr_t {
    char chunk_id[4];
    int chunk_size;
    char format[4];
    char subchunk1_id[4];
    int subchunk1_size;
    short int audio_format;
    short int num_channels;
    int sample_rate;
    int byte_rate;
    short int block_align;
    short int bits_per_sample;
    char subchunk2_id[4];
    int subchunk2_size;
};

#define MAX_SOUND_COUNT 16
uint8_t sounds_count;
int16_t sound_range_start;

#define SOUND_PITCH_MAX 20000
#define SOUND_PITCH_MIN -4400.0

sound_t *sound_create(void);
sound_t *initialise(void);
void closedown(sound_t * sound);
void process_sound(sound_t * sound);
void reset_defaults(sound_t * sound);
void recording_start(void);
void recording_stop(void);

#define MAX_WAV_FILES 10
extern char *wav_files[MAX_WAV_FILES];

SDL_AudioSpec wav_spec;
uint16_t wav_files_count;
int16_t *waveptr;

void play_wave_file(sound_t * sound, const char *filename);
void waveform_wavfile(void *userdata, uint8_t * stream, int len);
void play_music_file(sound_t * sound, const char *filename);

#define SYNTH_DALEK 0x0
#define SYNTH_TRIANGLE  0x1
#define SYNTH_DRUM_N_BASS 0x2
#define SYNTH_BASSY 0x3
#define SYNTH_BOWSERS_CASTLE  0x4
#define SYNTH_VINYL_SCRATCH 0x5
#define SYNTH_LAZER_QUEST 0x6
#define SYNTH_CREEPY_FUZZ 0x7
#define SYNTH_GIRLS_AND_BOYS 0x8

#endif
