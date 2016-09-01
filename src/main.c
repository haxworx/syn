/*
	(c) Copyright 2015, 2016. Al Poole <netstar@gmail.com>
	All Rights Reserved.
*/
#include "audio.h"
#include "video.h"
#include "stdinc.h"



int main(int argc, char **argv)
{
        (void) argc, (void) argv;

        SDL_Event event;

        printf("\n(c) Copyright 2016. Al Poole. All rights reserved.\n");
        printf("Close window to exit, otherwise, play the keyboard!\n");
	printf("Thanks to: Ed Poole and Steven Carle for debugging!\n\n");

	fflush(stdout);

        int quit = 0;

        init_sdl();

        // this works...

        sound_t *sound = initialise();
	check_wav_files(working_directory);
        update_screen(sound);

        // basic code ok, needs fixing...

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

        while (!quit) {
                if (sound->is_wavefile) {
                        if (sound->wavefile_pos < sound->wavefile_len) {
                                SDL_Delay(50);
                        } else {
                                sound->is_wavefile = false;
                                free(sound->data);
                                sound->wavefile_len = 0;
                                sound->wavefile_pos = 0;
                        }
                }

                if (recording_enabled) {
                        SDL_Delay(2);
                        sound->is_recording = true;
                }

                while (SDL_WaitEvent(&event)) {
                        start_time = SDL_GetTicks();
                        switch (event.type) {
			case SDL_JOYBUTTONDOWN: {
				uint8_t button = event.jbutton.button;
				switch (button) {
				case 6:
					if (sound->pitch > SOUND_PITCH_MIN) {
						sound->pitch -= 100;
						global_pitch = sound->pitch;
					}
					break;
				case 7:
					if (sound->pitch < SOUND_PITCH_MAX) {
						sound->pitch += 100;
						global_pitch = sound->pitch;
					}
					break;
				case 3: 
					global_bass += 1000;
					break;
				case 0:
					global_bass -= 1000;	
					break;
				case 2: 
					global_mid += 1000;	
					break;
				case 1:
					global_mid -= 1000;	
					break;
				case 5:
					global_treble += 1000;
					break;
				case 4: 
					global_treble -= 1000;	
					break;
				}
				update_console(sound);
				break;
			}

			case SDL_DROPFILE: {
				char *file = event.drop.file;
				current_action("Playing music file %s", file);
				update_screen(sound);
				play_music_file(sound, file);
				free(sound); sound = initialise();
				break;
			}

                        case SDL_KEYUP:
                                break;

                        case SDL_KEYDOWN:

                                sound->key = event.key.keysym.sym;
                                sound->note = keyboard_to_note(sound->key);

                                switch (sound->key) {
				/*
				case SDLK_UNDECIDED:
					global_bass += BLAH;
					break;
				case SDLK_UNDECIDED:
					global_mid += BLAH;
					break;
				case SDLK_UNDECIDED:
					global_treble += BLAH;
					break;
				*/
                                case SDLK_F1:
                                        sound->effects = SYNTH_DALEK;
                                        sound_range_start = -35;
                                        break;
                                case SDLK_F2:
                                        sound->effects = SYNTH_DRUM_N_BASS;
                                        sound_range_start = -30;
                                        break;

                                case SDLK_F3:
                                        sound->effects = SYNTH_TRIANGLE;
                                        sound_range_start = -22;
                                        break;

                                case SDLK_F4:
                                        sound->effects = SYNTH_BASSY;
                                        sound_range_start = 0;
                                        break;

                                case SDLK_F5:
                                        sound->effects =
					SYNTH_GIRLS_AND_BOYS;
                                        sound_range_start = -15;
                                        break;
				case SDLK_F6:
					sound->effects = SYNTH_VINYL_SCRATCH;
					sound_range_start = -82;
					break;
		
				case SDLK_F7:
					sound->effects = SYNTH_LAZER_QUEST;
					sound_range_start = 0;
					break;
				case SDLK_F8:
					sound->effects = SYNTH_CREEPY_FUZZ;
					sound_range_start = -33;
					break;

                                case SDLK_F9:
                                        sound_range_start--;
                                        break;

                                case SDLK_F10:
                                        sound_range_start++;
                                        break;

                                case SDLK_F11:
                                        if (sound->volume > 0) {
                                                sound->volume -= 1;
                                        }
                                        break;

                                case SDLK_F12:
                                        if (sound->volume <
                                            SDL_MIX_MAXVOLUME) {
                                                sound->volume += 1;
                                        }
                                        break;

                                case SDLK_HOME:
					if (recording_enabled) {
						current_action("still recording!");
						break;
					}
                                        recording_start();
                                        sound->is_recording = true;
                                        update_screen(sound);
                                        break;

                                case SDLK_END:
					if (sound->is_recording) {
                                        	recording_stop();
                                      		sound->is_recording = false;
                                        	update_screen(sound);
						synth_continuous = false;
						current_action("stopped recording!");
					}
                                        break;

                                case SDLK_DELETE:
					if (!recording_enabled) {
						bool del = delete_wav_file(working_directory);
						if (del) {
							current_action("deleted wav file!");
						} else {
							current_action("you can't delete what doesn't exist!");	
						}
					} else {
						current_action("still recording!");
					}
					update_screen(sound);
                                        break;

				/* this'll be bogus, who cares! */				
				case SDLK_SPACE:
					if (!synth_continuous) {
						current_action("continuous waveform!");
						synth_continuous = true;
					} 
					break;

				case SDLK_BACKSPACE:
					reset_defaults(sound);
					current_action("audio defaults reset!");	
					break;

				case SDLK_UP:
					if (sound->pitch < SOUND_PITCH_MAX) {
						sound->pitch += 1;
						global_pitch = sound->pitch;
					}
					break;
			
				case SDLK_DOWN:
					if (sound->pitch > SOUND_PITCH_MIN) {
						sound->pitch -= 1;
						global_pitch = sound->pitch;
					}
					break;

				case SDLK_0:
					play_wave_file(sound, "00.wav");
					break;
				case SDLK_1:
					play_wave_file(sound, "01.wav");
					break;
				case SDLK_2:
					play_wave_file(sound, "02.wav");
					break;
				case SDLK_3:
					play_wave_file(sound, "03.wav");
					break;
				case SDLK_4:
					play_wave_file(sound, "04.wav");
					break;
				case SDLK_5:
					play_wave_file(sound, "05.wav");
					break;
				case SDLK_6:
					play_wave_file(sound, "06.wav");
					break;
				case SDLK_7:
					play_wave_file(sound, "07.wav");
					break;
				case SDLK_8:
					play_wave_file(sound, "08.wav");
					break;
				case SDLK_9:
					play_wave_file(sound, "09.wav");
					break;

                                case SDLK_ESCAPE:
                                        quit = 1;
                                        goto out;
                                        break;

                                default:
                                        sound->A = 30000;
					if (! recording_enabled) {
					        current_action("Visit http://haxlab.org for more!");
					}
                                        process_sound(sound);
                                        sound->A = 0;
                                        break;
                                }
				check_wav_files(working_directory);
                                update_screen(sound);
                                break;

                        case SDL_MOUSEWHEEL:
                                if (event.wheel.y < 0) {
                                        if (sound->volume <
                                            SDL_MIX_MAXVOLUME)
                                                sound->volume += 1;
                                } else if (sound->volume > 0)
                                        sound->volume -= 1;

                                update_screen(sound);
                                break;

                        case SDL_QUIT:
                                quit = 1;
                                goto out;
                                break;

                        case SDL_WINDOWEVENT:
                                switch (event.window.event) {
                                case SDL_WINDOWEVENT_EXPOSED:
                                        update_screen(sound);
                                        break;

                                case SDL_WINDOWEVENT_SIZE_CHANGED:
                                        update_screen(sound);
                                        break;
                                }

                        }
                }               // end of event loop
        }

      out:
        free(sound);
	printf("Thanks! Bye now!\n");
        closedown();

        return EXIT_SUCCESS;
}

