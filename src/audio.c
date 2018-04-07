/*
    (c) Copyright 2015, 2016. Al Poole <netstar@gmail.com>
    All Rights Reserved.
*/

#include "stdinc.h"
#include "video.h"
#include "audio.h"

sound_t *sound_create(synth_t *synth)
{
    sound_t *sound = calloc(1, sizeof(sound_t));
    if (!sound)
        fail("calloc: %s\n", strerror(errno));

    sound->A = 30000;
    sound->volume = 80;         // SDL_MIX_MAXVOLUME;
    sound->effects = SYNTH_DALEK;
    sound->SR = 44100;
    sound->silence = 0;
    sound->pitch = synth->pitch;
    sound->duration = 0.1;
    sound->note = 0;
    sound->is_wavefile = false;
    sound->wavefile_pos = 0;
    sound->wavefile_len = 0;

    return sound;
}

/* keyboard to sound */
int keyboard_to_note(synth_t *synth, int k)
{
    int note = k - ' ' - 24;
    char key[1024] = { 0 };
    snprintf(key, sizeof(key), "%c", k);

    node_t *found = table_search(key);
    if (found) {
        note = found->ival + synth->sound_range_start;
        return note;
    }

    return note;
}

void effect_normal(int16_t * sound, int count)
{
    int effect_count = 0;
    while (*sound > 0 && effect_count < 1264) {
        *sound -= (count * 2);
        ++effect_count;
    }
    effect_count = 0;
}

void effect_triangle(int16_t * sound, int count)
{
    int effect_count = 0;
    while (*sound > 0 && effect_count < 64) {
        *sound += (count * 3);
        ++effect_count;
    }
}

void effect_girls_n_boys(int16_t * sound, int count)
{
    if (*sound > 0)
        *sound += 3 * count;
}

void effect_drum_n_bass(int16_t * sound, int count)
{
    *sound += 3 * count;
}

void effect_creepy_fuzz(int16_t * sound, int count)
{
    if (*sound < 10)
        *sound += (3 * count);
    if (*sound > 100)
        *sound = 0xfa;
}

void effect_bassy(int16_t * sound, int count)
{
    if (!(count % 4))
        *sound *= -1;

    *sound >>= 4;
}

void effect_bowser(int16_t * sound, int count)
{
    if (!(count % 4))
        *sound *= -1;
    while (*sound > 0)
        *sound -= 0xa;
}

void effect_vinyl_scratch(int16_t * sound, int count)
{
    while (sound < 0)
        *sound += 0xf;

    while (*sound > 0)
        *sound *= 256;

}

void effect_lazer_quest(int16_t * sound, int count)
{
    *sound *= tan(4096 * 1024 * atan(count * 4));
}

void recording_start(synth_t *synth)
{
    synth->is_recording = true;

    char path[PATH_MAX] = { 0 };

    DIR *d = NULL;
    struct dirent *dirent = NULL;
    d = opendir(synth->working_directory);
    if (!d)
        fail("opendir: %s\n", strerror(errno));

    while ((dirent = readdir(d)) != NULL) {
        if (!strncmp(dirent->d_name, ".", 1)) {
            continue;
        }
        struct stat fs;
        snprintf(path, PATH_MAX, "%s%c%s", synth->working_directory, SLASH,
                 dirent->d_name);

        stat(path, &fs);

        if (!S_ISDIR(fs.st_mode)) {
            table_insert(dirent->d_name, NULL, fs.st_size);
        }
    }

    closedir(d);

    char filename[PATH_MAX] = { 0 };
    node_t *found = NULL;

    for (int i = 0; i < MAX_WAV_FILES; i++) {
        snprintf(filename, sizeof(filename), "%02d.wav", i);
        found = table_search(filename);
        if (!found) {
            break;
        }
    }

    if (synth->recording_path_current) {
        free(synth->recording_path_current);
        synth->recording_path_current = NULL;
    }

    synth->recording_path_current = strdup(filename);

    snprintf(path, sizeof(path), "%s%c%s", synth->working_directory, SLASH,
             filename);
    synth->output_file = fopen(path, "wb");
    if (!synth->output_file)
        fail("fopen: %s\n", strerror(errno));

    current_action("recording to %s", filename);
}

void recording_stop(synth_t *synth)
{
    struct wav_file_hdr_t wav_file_hdr;
    memset(&wav_file_hdr, 0, sizeof(struct wav_file_hdr_t));
    if (synth->continuous) {
        synth->continuous = false;
    }
    /* Generate a RIFF/WAVE header */
    wav_file_hdr.chunk_id[0] = 'R';
    wav_file_hdr.chunk_id[1] = 'I';
    wav_file_hdr.chunk_id[2] = 'F';
    wav_file_hdr.chunk_id[3] = 'F';

    wav_file_hdr.chunk_size =
        sizeof(struct wav_file_hdr_t) + synth->output_buffer_len - 8;

    wav_file_hdr.format[0] = 'W';
    wav_file_hdr.format[1] = 'A';
    wav_file_hdr.format[2] = 'V';
    wav_file_hdr.format[3] = 'E';

    wav_file_hdr.subchunk1_id[0] = 'f';
    wav_file_hdr.subchunk1_id[1] = 'm';
    wav_file_hdr.subchunk1_id[2] = 't';
    wav_file_hdr.subchunk1_id[3] = ' ';

    wav_file_hdr.subchunk1_size = 16;
    wav_file_hdr.audio_format = 1;
    wav_file_hdr.num_channels = CHANNELS;
    wav_file_hdr.sample_rate = 44100;
    wav_file_hdr.byte_rate =
        (wav_file_hdr.sample_rate * sizeof(int16_t) *
         wav_file_hdr.num_channels);
    wav_file_hdr.block_align = 4;
    wav_file_hdr.bits_per_sample = 16;

    wav_file_hdr.subchunk2_id[0] = 'd';
    wav_file_hdr.subchunk2_id[1] = 'a';
    wav_file_hdr.subchunk2_id[2] = 't';
    wav_file_hdr.subchunk2_id[3] = 'a';

    wav_file_hdr.subchunk2_size = synth->output_buffer_len;

    // write the wav_h to file!
    // write header to file
    fwrite(&wav_file_hdr, sizeof(struct wav_file_hdr_t), 1, synth->output_file);

    // write the audio data!
    for (int i = 0; i < synth->output_buffer_len / 2; i++) {
        fwrite(&synth->output_buffer[i], sizeof(int16_t), 1, synth->output_file);
    }

    // close the RIFF/WAV file!
    fclose(synth->output_file);

    synth->is_recording = false;
    synth->output_buffer_len = 0;
    synth->output_buffer_index = 0;

    free(synth->recording_path_current);
    synth->recording_path_current = NULL;

    current_action("finished recording!");
}

void bass_mid_treble_apply(synth_t *synth, int16_t * chunk)
{
    if (*chunk == 0)
        return;
    if (synth->bass && *chunk < 80)
        *chunk += synth->bass;

    if (synth->mid && *chunk > 80)
        *chunk += synth->mid;

    if (synth->treble)
        *chunk += synth->treble;
}

void waveform_default(void *userdata, uint8_t * stream, int len)
{
    synth_t *synth = userdata;
    sound_t *snd = synth->sound;
    int samples = len / 2;

    SDL_memset(&synth->buffer, 0, len);

    int16_t note = snd->note;

    int16_t average = 0;

    synth->sounds_count = 0;
    if (snd->is_wavefile)
        synth->sounds_count = 0;

    uint8_t *keystate = NULL;
    SDL_FlushEvents(SDL_KEYDOWN, SDL_MOUSEWHEEL);

    keystate = (uint8_t *) SDL_GetKeyboardState(NULL);
    for (int i = 0; i < 128; i++) {
        if (keystate[SDL_GetScancodeFromKey(i)]) {
            if (synth->continuous && i == SDLK_ESCAPE) {
                synth->continuous = false;
            }
            if (snd->is_wavefile) {
                continue;
            }
            sound_t *s = sound_create(synth);
            s->key = i;
            s->note = keyboard_to_note(synth, i);
            synth->sounds[synth->sounds_count++] = s;

        }

        /*
           if (! keystate[SDL_GetScancodeFromKey(i)]) {
           SDL_FlushEvents(SDL_KEYDOWN, SDL_MOUSEWHEEL);
           }

           // after initial key is unpressed KEYUP we
           // can safely flush all buffered input
         */
    }

    SDL_FlushEvents(SDL_KEYDOWN, SDL_MOUSEWHEEL);

    if ((1000 / FPS) > SDL_GetTicks() - start_time) {
        SDL_Delay((1000 / FPS) - (SDL_GetTicks() - start_time));
    }

    for (int i = 0; i < synth->sounds_count && synth->sounds[i] != NULL; i++) {
        if (synth->sounds[i]->is_wavefile) {
            continue;
        }
        average += synth->sounds[i]->note;
        free(synth->sounds[i]);
        synth->sounds[i] = NULL;
    }

    if (synth->sounds_count)
        note = average / synth->sounds_count;

    if (snd->pitch > SOUND_PITCH_MAX)   // ten times sanity
        snd->pitch = SOUND_PITCH_MAX;
    else if (snd->pitch < SOUND_PITCH_MIN)
        snd->pitch = SOUND_PITCH_MIN;

    double Hz = snd->pitch * pow(2, note / 12.0);

    double F = pi * Hz / snd->SR;

    for (int i = 0; i < samples; i += 2) {
        int16_t sound = 0;

        sound = (int16_t) snd->A * sin(F * (double) synth->counter);
        if (sound == 0)
            goto no_effects;    /* audible clicks adios! */
        switch (snd->effects) {
        case SYNTH_DALEK:
            effect_normal(&sound, synth->counter);
            break;
        case SYNTH_DRUM_N_BASS:
            effect_drum_n_bass(&sound, synth->counter);
            break;
        case SYNTH_TRIANGLE:
            effect_triangle(&sound, synth->counter);
            break;
        case SYNTH_BASSY:
            effect_bassy(&sound, synth->counter);
            break;
        case SYNTH_GIRLS_AND_BOYS:
            effect_girls_n_boys(&sound, synth->counter);
            break;
        case SYNTH_BOWSERS_CASTLE:
            effect_bowser(&sound, synth->counter);
            break;
        case SYNTH_VINYL_SCRATCH:
            effect_vinyl_scratch(&sound, synth->counter);
            break;
        case SYNTH_LAZER_QUEST:
            effect_lazer_quest(&sound, synth->counter);
            break;
        case SYNTH_CREEPY_FUZZ:
            effect_creepy_fuzz(&sound, synth->counter);
            break;
        }

      no_effects:
        bass_mid_treble_apply(synth, &sound);

        synth->buffer[i] = sound;
        synth->buffer[i + 1] = sound;
        ++synth->counter;
    }

    int16_t *wav_pointer = NULL;
    if (snd->is_wavefile) {
        wav_pointer = snd->data + snd->wavefile_pos;
        synth->waveptr = synth->buffer;
    } else {
        synth->waveptr = synth->buffer;
    }

    SDL_memset(stream, 0, len);

    if (!snd->is_wavefile) {
        SDL_MixAudio(stream, (uint8_t *) synth->waveptr, len, snd->volume);
    } else {
        // problematic
        if (snd->wavefile_pos >= snd->wavefile_len) {
            snd->wavefile_pos = 0;
            snd->is_wavefile = 0;
            free(snd->data);
            // this fixes a segfault
            return;
        }

        SDL_MixAudio(stream, (uint8_t *) wav_pointer, len, snd->volume);
        snd->wavefile_pos += len;
    }


    // this should record wav files mixed with sounds
    // this should record wav files by themselves
    // this should record wav files being recorded themselves

    if (synth->is_recording) {
        for (int i = 0; i < len / 2; i++) {
            if (synth->output_buffer_len >= MAX_OUTFILE_SIZE) {
                snd->is_recording = false;
                recording_stop(synth);
                return;
            }
            if (!snd->is_wavefile) {
                synth->output_buffer[synth->output_buffer_index++] =
                    (int16_t) * (synth->waveptr + i);
            } else {
                int16_t mixed = (int16_t) * (synth->waveptr + i);
                mixed += (int16_t) * (wav_pointer + i);
                synth->output_buffer[synth->output_buffer_index++] = mixed;
            }
            synth->output_buffer_len += sizeof(int16_t);
        }
    }
}

void reset_defaults(synth_t *synth)
{
    sound_t *sound = synth->sound;
    synth->pitch = sound->pitch = 440.0;
    sound->volume = 80;         //128;
//      sound_range_start = 0;
    synth->bass = synth->mid = synth->treble = 0;       // reset "dials"

}

sound_t *sound_system_up(synth_t *synth)
{
    sound_t *sound = sound_create(synth);
    SDL_AudioSpec wav_spec;
    SDL_memset(&wav_spec, 0, sizeof(wav_spec));
    wav_spec.freq = 44100;
    wav_spec.format = AUDIO_S16SYS;
    wav_spec.channels = CHANNELS;
    wav_spec.silence = 0;
    wav_spec.samples = SAMPLES;
    wav_spec.padding = 0;
    wav_spec.size = 0;
    wav_spec.userdata = synth;
    wav_spec.callback = waveform_default;

    sound->A = 0;

    if (SDL_OpenAudio(&wav_spec, NULL) < 0)
        fail("SDL_OpenAudio: %s\n", SDL_GetError());

    // range start with default synth range too!
    synth->sound_range_start = -35;

    // start it up! 
    SDL_PauseAudio(0);

    return sound;
}

void process_sound(synth_t *synth)
{
    sound_t *sound = synth->sound;
    synth->counter = 0;
    bool continuous = false;
    uint8_t *keystate = NULL;
    synth->sounds_count = 0;

    bool bending_pitch = false;
    bool screen_will_update = true;

    uint16_t pitch_old = sound->pitch;

    while (synth->continuous || continuous
           || (synth->counter < 44100 * sound->duration)) {
        if ((1000 / FPS) > SDL_GetTicks() - start_time) {
            SDL_Delay((1000 / FPS) - (SDL_GetTicks() - start_time));
        }

        SDL_PumpEvents();

        uint8_t scancode = SDL_GetScancodeFromKey(sound->key);
        keystate = (uint8_t *) SDL_GetKeyboardState(NULL);
        if (keystate[scancode]) {
            if (keystate[scancode] == SDLK_ESCAPE) {
                synth->continuous = false;
            }
            continuous = true;
            uint32_t mousestate = SDL_GetMouseState(NULL, NULL);
            if (mousestate & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                sound->pitch += 0.005;
                synth->pitch = sound->pitch;
                if (!bending_pitch)
                    bending_pitch = true;
            } else if (mousestate & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                sound->pitch -= 0.005;
                synth->pitch = sound->pitch;
                if (!bending_pitch)
                    bending_pitch = true;
            }
            if (bending_pitch) {
                if (screen_will_update) {
                    current_action("pitch bending...");
                    update_screen(synth);
                    screen_will_update = false;
                }
            }
            // visualization();
        } else {
            continuous = false;
        }
    }

    if (bending_pitch)
        current_action("end of the bend...");

    update_screen(synth);
    bending_pitch = false;
    sound->pitch = synth->pitch = pitch_old;
}


void synth_shutdown(synth_t *synth)
{
    sound_t * sound = synth->sound;
    free(sound);

    table_free();

    if (synth->is_recording) {
        recording_stop(synth);
    }

    SDL_CloseAudio();
    SDL_Quit();
}

#define SYNTH_DATA_DIR_NAME "Recordings"

static void
_work_directory_set(synth_t *synth)
{
    char *homedrive = getenv("HOMEDRIVE");
    if (homedrive) {
        char *homepath = getenv("HOMEPATH");
        if (!homepath)
            fail("getenv: HOME");

        snprintf(synth->working_directory, sizeof(synth->working_directory), "%s%s\\%s",
                 homedrive, homepath, SYNTH_DATA_DIR_NAME);
    } else {
        // CWD and ! Windows
        snprintf(synth->working_directory, sizeof(synth->working_directory), "%c%c%s", '.', SLASH, SYNTH_DATA_DIR_NAME);
    }

    struct stat fstats;
    if (stat(synth->working_directory, &fstats) < 0)
#ifdef WINDOWS
        mkdir(synth->working_directory);
#else
        mkdir(synth->working_directory, 0777);
#endif

}

synth_t *synth_new(void)
{
    synth_t *self = calloc(1, sizeof(synth_t));

    init_sdl();

    _work_directory_set(self);

    int16_t i = self->sound_range_start;
    /* US/UK keyboard -> sound continuity */

    table_insert("q", NULL, i++);
    table_insert("w", NULL, i++);
    table_insert("e", NULL, i++);
    table_insert("r", NULL, i++);
    table_insert("t", NULL, i++);
    table_insert("y", NULL, i++);
    table_insert("u", NULL, i++);
    table_insert("i", NULL, i++);
    table_insert("o", NULL, i++);
    table_insert("p", NULL, i++);


    table_insert("a", NULL, i++);
    table_insert("s", NULL, i++);
    table_insert("d", NULL, i++);
    table_insert("f", NULL, i++);
    table_insert("g", NULL, i++);
    table_insert("h", NULL, i++);
    table_insert("j", NULL, i++);
    table_insert("k", NULL, i++);
    table_insert("l", NULL, i++);


    table_insert("z", NULL, i++);
    table_insert("x", NULL, i++);
    table_insert("c", NULL, i++);
    table_insert("v", NULL, i++);
    table_insert("b", NULL, i++);
    table_insert("n", NULL, i++);
    table_insert("m", NULL, i++);

    self->pitch = 440.00;

    check_wav_files(self);

    self->recording_path_current = NULL;
    
    *self->output_buffer = 0;

    for (i = 0; i < MAX_WAV_FILES; i++) {
        self->wav_files[i] = NULL;
    }
    
    self->sound = sound_system_up(self);
    *self->sounds = NULL;

    return self;
}

void chomp(char *str)
{
    char *s = str;

    while (*s) {
        if (*s == '\r' || *s == '\n') {
            *s = '\0';
            return;
        }
        ++s;
    }
}

/* two functions to play WAV files from disk */

uint32_t wavefile_pos = 0;

void waveform_wavfile(void *userdata, uint8_t * stream, int len)
{
    int8_t *waveptr = NULL;

    waveptr = (int8_t *) userdata + wavefile_pos;

    SDL_memset(stream, 0, len);
    SDL_MixAudio(stream, (uint8_t *) waveptr, len, SDL_MIX_MAXVOLUME);
    wavefile_pos += len;
}

// FIXME: playing the same file as is recording...

void play_wave_file(synth_t *synth, const char *filename)
{
    sound_t * sound = synth->sound;

    if (synth->recording_path_current != NULL
        && !strcmp(filename, synth->recording_path_current)) {
        current_action
            ("you cannot play a file that is currently recording");
        return;
    }

    SDL_AudioSpec wav_file_spec;
    SDL_memset(&wav_file_spec, 0, sizeof(SDL_AudioSpec));
    struct wav_file_hdr_t wav_file_hdr;
    char path[PATH_MAX] = { 0 };

    snprintf(path, sizeof(path), "%s%c%s", synth->working_directory, SLASH,
             filename);
     printf("path: %s\n", path);
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        current_action("that audio file doesn't seem to exist!");
        return;
    }
    // read file wave header!
    size_t res =
        fread(&wav_file_hdr, sizeof(struct wav_file_hdr_t), 1, fp);
    if (!res) {
        current_action("not playing potentially corrupt file??");
        return;
    }

    int16_t *audio_buffer =
        calloc(1, sizeof(int16_t) * wav_file_hdr.subchunk2_size);
    if (!audio_buffer)
        fail("calloc: %s\n", strerror(errno));

    size_t len = fread(audio_buffer, sizeof(int16_t),
                       wav_file_hdr.subchunk2_size,
                       fp);
    if (!len) {
        fail("fread: %s\n", strerror(errno));
    }

    fclose(fp);

    sound_t *wave = sound;
    wave->is_wavefile = true;
    wave->wavefile_pos = 0;
    wave->data = audio_buffer;

    wav_file_spec.format = AUDIO_S16SYS;
    wav_file_spec.freq = wav_file_hdr.sample_rate;
    wav_file_spec.channels = wav_file_hdr.num_channels;
    wav_file_spec.samples = SAMPLES;
    wav_file_spec.silence = 0;
    wav_file_spec.padding = 0;
    wav_file_spec.size = 0;
    wav_file_spec.userdata = wave;
    wav_file_spec.callback = waveform_default;

    wave->wavefile_len = wav_file_hdr.subchunk2_size;

    current_action("we are playing %s", filename);
}

void play_music_file(synth_t * synth, const char *filename)
{
    char buf[8192] = { 0 };
    sound_t *s = sound_create(synth);
    SDL_AudioSpec wav_spec;
    SDL_memset(&wav_spec, 0, sizeof(wav_spec));
    wav_spec.freq = 44100;
    wav_spec.format = AUDIO_S16SYS;
    wav_spec.channels = 1;
    wav_spec.samples = SAMPLES;
    wav_spec.userdata = s;
    wav_spec.callback = waveform_default;

    SDL_CloseAudio();

    if (SDL_OpenAudio(&wav_spec, NULL) < 0)
        fail("SDL_OpenAudio: %s\n", SDL_GetError());

    start_time = SDL_GetTicks();

    FILE *f = fopen(filename, "r");
    if (!f) {
        return;
        fail("fopen: %s\n", strerror(errno));
    }

    printf("playing: %s\n", filename);

    while (fgets(buf, sizeof(buf), f) != NULL) {
        char *start = buf;
        // nah!  chomp(buf);
        if (*start == '#') {
            continue;
        }

        uint8_t field = 0;
        char *p = start;
        while (*p) {
            if (*p == '\t' || *p == '\n') {
                char *end = p;
                *end = '\0';
                char *str = NULL;
                // FORMAT IS "INT\tDOUBLE\tINT\n"
                switch (field) {
                case 0:
                    s->note = atoi(start);
                    s->note -= 20;
                    ++field;
                    break;

                case 1:
                    s->duration = strtod(start, &str);
                    ++field;
                    break;

                case 2:
                    s->effects = atoi(start);
                    ++field;
                    break;
                }
                start = end + 1;
            }

            ++p;

            if (field == 3) {
                //update_screen(sound);
                field = 0;
                SDL_PauseAudio(0);
                SDL_Delay(1000 * s->duration);
                SDL_PauseAudio(1);
            }
        }
    }

    fclose(f);
    free(s);
    SDL_CloseAudio();
}

bool is_wav_file(const char *path)
{
    char *dot = strrchr(path, '.');
    if (!dot) {
        return false;
    }

    dot++;

    if (!strcmp(dot, "wav")) {
        return true;
    }

    return false;
}

/* Basically checks how many recordings are on disk */
/* 00.wav to 09.wav ... crude :) */

void check_wav_files(synth_t *synth)
{
    DIR *d = NULL;
    struct dirent *dirent = NULL;
    const char *directory = synth->working_directory;

    d = opendir(directory);
    if (!d)
        fail("opendir: %s\n", strerror(errno));

    synth->wav_files_count = 0;

    while ((dirent = readdir(d)) != NULL) {
        if (!strncmp(dirent->d_name, ".", 1)) {
            continue;
        }
        char path[PATH_MAX] = { 0 };
        struct stat fs;
        snprintf(path, PATH_MAX, "%s%c%s", directory, SLASH,
                 dirent->d_name);

        stat(path, &fs);

        if (S_ISDIR(fs.st_mode)) {
            continue;
        }

        if (is_wav_file(path)) {
            if (synth->wav_files_count < MAX_WAV_FILES) {
                if (synth->wav_files[synth->wav_files_count] != NULL) {
                    //continue;
                }
                synth->wav_files[synth->wav_files_count++] = strdup(dirent->d_name);
            }
        }
    }

    closedir(d);
}

/* These are all actions on files */
bool delete_wav_file(synth_t *synth)
{
    int latest_wav_file = 0;
    bool found_file = false;
    const char *directory = synth->working_directory;

    for (int i = 0; i < MAX_WAV_FILES; i++) {
        if (synth->wav_files[i] != NULL) {
            latest_wav_file = i;
            found_file = true;
        }
    }

    if (found_file) {
        char path[PATH_MAX] = { 0 };
        snprintf(path, sizeof(path), "%s%c%s", directory,
                 SLASH, synth->wav_files[latest_wav_file]);
        unlink(path);
        current_action("deleted wav files %s", synth->wav_files[latest_wav_file]);
        table_delete(synth->wav_files[latest_wav_file]);
        free(synth->wav_files[latest_wav_file]);
        synth->wav_files[latest_wav_file] = NULL;
        return true;
    }

    return false;
}


