#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "audio.h"

#define WIDTH 800
#define HEIGHT 600
#define FPS 60

uint32_t pixels[WIDTH * HEIGHT];

SDL_Window *window;
SDL_Surface *logo;
SDL_Surface *logo_recording;
SDL_Renderer *renderer;
SDL_Texture *texture;
TTF_Font *font;
SDL_Joystick *Gamepad;

/* visualisation types */
#define VISUAL_WAVE 0
#define VISUAL_ICON 1

void init_sdl(void);
SDL_Surface *load_image(char *path);
void update_screen(synth_t *synth);
void visualization(synth_t *synth);
void set_screen_data(synth_t *synth, char *buf, int len);
void set_files_list(synth_t *synth, char *buf, int len);
void update_console(sound_t * sound);
void current_action(char *fmt, ...);


#endif
