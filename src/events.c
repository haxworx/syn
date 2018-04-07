/*
	(c) Copyright 2015, 2016. Al Poole <netstar@gmail.com>
	All Rights Reserved.
*/

#include "events.h"
#include "audio.h"
#include "video.h"
#include "stdinc.h"


int Run(synth_t *synth)
{
    SDL_Event event;

    sound_t *sound = synth->sound;

    while (SDL_WaitEvent(&event)) {
            start_time = SDL_GetTicks();
            switch (event.type) {
            case SDL_JOYBUTTONDOWN:{
                    uint8_t button = event.jbutton.button;
                    switch (button) {
                    case 6:
                        if (sound->pitch > SOUND_PITCH_MIN) {
                            sound->pitch -= 100;
                            synth->pitch = sound->pitch;
                        }
                        break;
                    case 7:
                        if (sound->pitch < SOUND_PITCH_MAX) {
                            sound->pitch += 100;
                            synth->pitch = sound->pitch;
                        }
                        break;
                    case 3:
                        synth->bass += 1000;
                        break;
                    case 0:
                        synth->bass -= 1000;
                        break;
                    case 2:
                        synth->mid += 1000;
                        break;
                    case 1:
                        synth->mid -= 1000;
                        break;
                    case 5:
                        synth->treble += 1000;
                        break;
                    case 4:
                        synth->treble -= 1000;
                        break;
                    }
                    update_console(sound);
                    break;
                }

            case SDL_KEYUP:
                break;

            case SDL_KEYDOWN:

                sound->key = event.key.keysym.sym;
                sound->note = keyboard_to_note(synth, sound->key);

                switch (sound->key) {
                    /*
                       case SDLK_UNDECIDED:
                       synth->bass += BLAH;
                       break;
                       case SDLK_UNDECIDED:
                       synth->mid += BLAH;
                       break;
                       case SDLK_UNDECIDED:
                       synth->treble += BLAH;
                       break;
                     */
                case SDLK_F1:
                    sound->effects = SYNTH_DALEK;
                    synth->sound_range_start = -35;
                    break;
                case SDLK_F2:
                    sound->effects = SYNTH_DRUM_N_BASS;
                    synth->sound_range_start = -30;
                    break;

                case SDLK_F3:
                    sound->effects = SYNTH_TRIANGLE;
                    synth->sound_range_start = -22;
                    break;

                case SDLK_F4:
                    sound->effects = SYNTH_BASSY;
                    synth->sound_range_start = 0;
                    break;

                case SDLK_F5:
                    sound->effects = SYNTH_GIRLS_AND_BOYS;
                    synth->sound_range_start = -15;
                    break;
                case SDLK_F6:
                    sound->effects = SYNTH_VINYL_SCRATCH;
                    synth->sound_range_start = -82;
                    break;

                case SDLK_F7:
                    sound->effects = SYNTH_LAZER_QUEST;
                    synth->sound_range_start = 0;
                    break;
                case SDLK_F8:
                    sound->effects = SYNTH_CREEPY_FUZZ;
                    synth->sound_range_start = -33;
                    break;

                case SDLK_F9:
                    synth->sound_range_start--;
                    break;

                case SDLK_F10:
                    synth->sound_range_start++;
                    break;

                case SDLK_F11:
                    if (synth->volume > 0) {
                        synth->volume -= 1;
                    }
                    break;

                case SDLK_F12:
                    if (synth->volume < SDL_MIX_MAXVOLUME) {
                        synth->volume += 1;
                    }
                    break;

                case SDLK_HOME:
                    if (synth->is_recording) {
                        display_action("still recording!");
                        break;
                    }
                    recording_start(synth);
                    synth->is_recording = true;
                    display_refresh(synth);
                    break;

                case SDLK_END:
                    if (synth->is_recording) {
                        recording_stop(synth);
                        synth->is_recording = false;
                        display_refresh(synth);
                        synth->continuous = false;
                        display_action("stopped recording!");
                    }
                    break;

                case SDLK_DELETE:
                    if (!synth->is_recording) {
                        bool del = wave_file_del(synth);
                        if (del) {
                            display_action("deleted wav file!");
                        } else {
                            display_action
                                ("you can't delete what doesn't exist!");
                        }
                    } else {
                        display_action("still recording!");
                    }
                    display_refresh(synth);
                    break;

                    /* this'll be bogus, who cares! */
                case SDLK_SPACE:
                    if (!synth->continuous) {
                        display_action("continuous waveform!");
                        synth->continuous = true;
                    }
                    break;

                case SDLK_BACKSPACE:
                    reset_defaults(synth);
                    display_action("audio defaults reset!");
                    break;

                case SDLK_UP:
                    if (sound->pitch < SOUND_PITCH_MAX) {
                        sound->pitch += 1;
                        synth->pitch = sound->pitch;
                    }
                    break;

                case SDLK_DOWN:
                    if (sound->pitch > SOUND_PITCH_MIN) {
                        sound->pitch -= 1;
                        synth->pitch = sound->pitch;
                    }
                    break;

                case SDLK_0:
                    wave_file_play(synth, "00.wav");
                    break;
                case SDLK_1:
                    wave_file_play(synth, "01.wav");
                    break;
                case SDLK_2:
                    wave_file_play(synth, "02.wav");
                    break;
                case SDLK_3:
                    wave_file_play(synth, "03.wav");
                    break;
                case SDLK_4:
                    wave_file_play(synth, "04.wav");
                    break;
                case SDLK_5:
                    wave_file_play(synth, "05.wav");
                    break;
                case SDLK_6:
                    wave_file_play(synth, "06.wav");
                    break;
                case SDLK_7:
                    wave_file_play(synth, "07.wav");
                    break;
                case SDLK_8:
                    wave_file_play(synth, "08.wav");
                    break;
                case SDLK_9:
                    wave_file_play(synth, "09.wav");
                    break;

                case SDLK_ESCAPE:
                    return false;
                    break;

                default:
                    sound->A = 30000;
                    if (!synth->is_recording) {
                        display_action
                            ("Visit http://haxlab.org for more!");
                    }
                    process_sound(synth);
                    sound->A = 0;
                    break;
                }
                wave_files_count(synth);
                display_refresh(synth);
                break;

            case SDL_MOUSEWHEEL:
                if (event.wheel.y < 0) {
                    if (synth->volume < SDL_MIX_MAXVOLUME)
                        synth->volume += 1;
                } else if (synth->volume > 0)
                    synth->volume -= 1;

                display_refresh(synth);
                break;

            case SDL_QUIT:
                return false;
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_EXPOSED:
                    display_refresh(synth);
                    break;

                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    display_refresh(synth);
                    break;
                }

            }
        }

    return true;
}

