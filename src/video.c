/*
    (c) Copyright 2015, 2016. Al Poole <netstar@gmail.com>
    All Rights Reserved.
*/

#include "video.h"
#include "audio.h"
#include "stdinc.h"

uint32_t pixels[WIDTH * HEIGHT];

SDL_Window *window = NULL;
SDL_Surface *logo = NULL;
SDL_Surface *logo_recording = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
TTF_Font *font = NULL;
SDL_Joystick *Gamepad = NULL;

void init_sdl(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
        fail("SDL_Init: %s\n", SDL_GetError());


    if (SDL_NumJoysticks() < 1)
        fprintf(stderr, "no gamepads!\n");
    else {
        Gamepad = SDL_JoystickOpen(0);
    }

    window =
        SDL_CreateWindow("netstar presents: synthropy",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
                         SDL_WINDOW_RESIZABLE);
    if (window == NULL)
        fail("SDL_CreateWindow: %s\n", SDL_GetError());

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
        fail("SDL_image: %s\n", IMG_GetError());

    if (TTF_Init() == -1)
        fail("SDL_ttf: %s\n", TTF_GetError());

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL)
        fail("SDL_CreateRenderer: %s\n", SDL_GetError());

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
    if (texture == NULL)
        fail("SDL_CreateTexture: %s\n", SDL_GetError());

}

SDL_Surface *load_image(char *path)
{
    SDL_Surface *surface2 = NULL;

    SDL_Surface *surface = IMG_Load(path);
    if (surface == NULL)
        fail("IMG_Load: %s: %s\n", path, IMG_GetError());

    surface2 = SDL_ConvertSurface(surface, surface->format, 0);
    if (surface2 == NULL)
        fail("SDL_ConvertSurface: %s: %s\n", path, SDL_GetError());

    SDL_FreeSurface(surface);

    return surface2;
}

// this is a quick + dirty way to keep our action
// on display.

char current_action_text[8192] = { 0 };

void update_console(sound_t * sound)
{
#if defined(DEBUG)
    printf("VOLUME: %d INSTRUMENT %d\n", sound->volume, sound->effects);
    printf("BASS: %d MID: %d TREBLE: %d\n", global_bass, global_mid,
           global_treble);
    printf("PITCH: %dHz TONE: %d\n", global_pitch, sound->note);
    printf("DEBUG: %s\n\n", current_action_text);
#endif
}

void set_files_list(synth_t *synth, char *buf, int len)
{
    int recording_start = -1;
    int recording_end = 0;
    bool first_file = true;

    for (int i = 0; i < MAX_WAV_FILES; i++) {
        if (synth->wav_files[i]) {
            if (first_file) {
                recording_start = i;
                first_file = false;
            }
            recording_end = i;
        }
    }
    if (first_file) {
        snprintf(buf, len, "[ESC] (quit)  [DOWN/UP] (pitch -/+)");
        return;
    }

    if (recording_end == 0) {
        snprintf(buf, len,
                 "[ESC] (quit)  [DOWN/UP] (pitch -/+)  playback(\"%d\")  [DEL] (delete latest recording)",
                 recording_start);

    } else {
        snprintf(buf, len,
                 "[ESC] (quit)  [DOWN/UP] (pitch -/+)  playback(\"%d-%d\")  [DEL] (delete latest recording)",
                 recording_start, recording_end);

    }
}

void set_screen_data(synth_t * synth, char *buf, int len)
{
    sound_t *sound = synth->sound;
    char *waveform = NULL;
    switch (sound->effects) {
    case SYNTH_DALEK:
        waveform = "sick dalek";
        break;
    case SYNTH_DRUM_N_BASS:
        waveform = "drum n bass!";
        break;
    case SYNTH_TRIANGLE:
        waveform = "Thatcher's 80's";
        break;
    case SYNTH_BASSY:
        waveform = "inspector morse!";
        break;
    case SYNTH_GIRLS_AND_BOYS:
        waveform = "girls and boys!";
        break;
    case SYNTH_VINYL_SCRATCH:
        waveform = "vinyl terrorism!";
        break;
    case SYNTH_LAZER_QUEST:
        waveform = "lazer quest!";
        break;
    case SYNTH_CREEPY_FUZZ:
        waveform = "creepy fuzz!";
        break;
    }

    snprintf(buf, len,
             "Pitch: %0.2f Hz  Tone: %d  Volume: %d  Instrument: \"%s\"",
             sound->pitch, synth->sound_range_start, sound->volume, waveform);
}

void update_screen(synth_t * synth)
{
    sound_t *sound = synth->sound;
    char *filename = "images/logo.png";

    if (logo)
        update_console(sound);
    /* In case we need/want a new image for recording... */
    if (!logo_recording)
        logo_recording = load_image("images/recording.png");

    if (!logo)
        logo = load_image(filename);

    SDL_Surface *logo_copy = NULL;

    if (synth->is_recording) {
        logo_copy =
            SDL_ConvertSurface(logo_recording, logo_recording->format, 0);
    } else {
        logo_copy = SDL_ConvertSurface(logo, logo->format, 0);
    }

    if (!logo_copy)
        fail("SDL_ConvertSurface: %s\n", SDL_GetError());

#define FONT_SIZE 17
 
#define FONT_PADDING 8

    SDL_Color whiteColor = { 255, 255, 255, 0 };
    SDL_Color redColor = { 199, 0, 0, 0 };
    SDL_Color blueColor = { 88, 199, 9, 0 };

    if (!font) {
        font = TTF_OpenFont("fonts/Typewriter.ttf", FONT_SIZE);
        if (!font)
            fail("TTF_OpenFont: %s\n", TTF_GetError());
    }


    SDL_Rect aboutrect;

#define GUIDE_TEXT_TWO "[SPACE/ESC] (start/stop continuous waveform)  [BACKSPACE] (reset audio defaults)"

    SDL_Surface *guide_text_two =
        TTF_RenderText_Blended(font, GUIDE_TEXT_TWO, whiteColor);
    if (guide_text_two == NULL)
        fail("TTF_RenderTextBlended: %s\n", TTF_GetError());


#define GUIDE_TEXT_0 "[F1-F8] (instrument)  [F9/F10] (tone -/+)  [F11/F12] (volume -/+)  [HOME/END] (start/stop recording)"
    SDL_Surface *guide_text =
        TTF_RenderText_Blended(font, GUIDE_TEXT_0, whiteColor);

#define INFO_TEXT "(c) Copyright 2016. Al Poole (netstar@gmail.com). All Rights Reserved."

    char screen_data[8192] = { 0 };

    set_screen_data(synth, screen_data, sizeof(screen_data));

    SDL_Color *dataColor = &blueColor;

    if (synth->is_recording)
        dataColor = &redColor;

    SDL_Surface *text =
        TTF_RenderText_Blended(font, screen_data, redColor);

    if (!text)
        fail("TTF_RenderText_Blended: %s\n", TTF_GetError());

    SDL_Surface *action_text =
        TTF_RenderText_Blended(font, current_action_text, *dataColor);
    if (!action_text)
        fail("TTF_RenderText_Blended: %s\n", TTF_GetError());

    //TTF_SetFontStyle(font, TTF_STYLE_BOLD);

    check_wav_files(synth); // expensive???
    char files_list[8192] = { 0 };
    set_files_list(synth, files_list, sizeof(files_list));

    SDL_Surface *files_list_text =
        TTF_RenderText_Blended(font, files_list, whiteColor);
    if (!files_list_text) {
        fail("TTF_RenderText_Blended: %s\n", TTF_GetError());
    }


    SDL_Surface *version_text =
        TTF_RenderText_Blended(font, VERSION, whiteColor);
    if (!version_text)
        fail("TTF_RenderText_Blended: %s\n", TTF_GetError());

    SDL_Surface *info_text =
        TTF_RenderText_Blended(font, INFO_TEXT, blueColor);
    if (!info_text)
        fail("TTF_RenderText_Blended: %s\n", TTF_GetError());

    aboutrect.x = 2 * FONT_PADDING;
    aboutrect.y = HEIGHT - (2 * FONT_SIZE + 2 * FONT_PADDING);
    aboutrect.w = (FONT_SIZE / 2) * strlen(GUIDE_TEXT_0);
    aboutrect.h = FONT_SIZE * 2 + FONT_PADDING;

    SDL_Rect about_rect_2;
    about_rect_2.x = 2 * FONT_PADDING;
    about_rect_2.y = HEIGHT - (3 * FONT_SIZE + 3 * FONT_PADDING);
    about_rect_2.w = (FONT_SIZE / 2) * strlen(GUIDE_TEXT_TWO);
    about_rect_2.h = FONT_SIZE * 2 + FONT_PADDING;

    SDL_Rect info_rect;
    info_rect.x = (WIDTH / 2) - (strlen(INFO_TEXT) * (FONT_SIZE / 2)) / 2;
    info_rect.y = (HEIGHT / 2) + 106;
    info_rect.w = (FONT_SIZE / 2) * strlen(INFO_TEXT);
    info_rect.h = FONT_SIZE;

    SDL_Rect action_rect;
    action_rect.x =
        (WIDTH / 2) - (strlen(current_action_text) * (FONT_SIZE / 2)) / 2;
    action_rect.y = (HEIGHT / 2) + 128;
    action_rect.w = (FONT_SIZE / 2) * strlen(current_action_text);
    action_rect.h = FONT_SIZE;

    SDL_Rect files_rect;
    files_rect.x = 2 * FONT_PADDING;
    files_rect.y = 2 * FONT_PADDING + (FONT_PADDING / 2);       // (FONT_SIZE / 2);
    files_rect.w = WIDTH - (2 * FONT_PADDING);
    files_rect.h = FONT_SIZE;

    SDL_Rect data_rect;
    data_rect.x = 2 * FONT_PADDING;
    data_rect.y = HEIGHT - (FONT_PADDING + FONT_SIZE * 1);
    data_rect.w = (FONT_SIZE / 2) * strlen(screen_data);
    data_rect.h = FONT_SIZE;

    SDL_Rect version_rect;
    version_rect.x =
        WIDTH - ((2 * FONT_PADDING) + (FONT_SIZE / 2) * (strlen(VERSION)));
    version_rect.y = 2 * FONT_PADDING + (FONT_PADDING / 2);
    version_rect.w = (FONT_SIZE / 2) * strlen(VERSION);
    version_rect.h = FONT_SIZE;

    SDL_BlitSurface(text, NULL, logo_copy, &data_rect);
    SDL_BlitSurface(files_list_text, NULL, logo_copy, &files_rect);
    SDL_BlitSurface(action_text, NULL, logo_copy, &action_rect);
    SDL_BlitSurface(version_text, NULL, logo_copy, &version_rect);
    SDL_BlitSurface(guide_text_two, NULL, logo_copy, &about_rect_2);
    SDL_BlitSurface(guide_text, NULL, logo_copy, &aboutrect);
    SDL_BlitSurface(info_text, NULL, logo_copy, &info_rect);

    SDL_FreeSurface(text);
    SDL_FreeSurface(guide_text);
    SDL_FreeSurface(guide_text_two);
    SDL_FreeSurface(info_text);
    SDL_FreeSurface(version_text);
    SDL_FreeSurface(files_list_text);
    SDL_FreeSurface(action_text);

    SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, logo_copy);
    SDL_FreeSurface(logo_copy);

    SDL_RenderCopy(renderer, t, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(t);
    SDL_RenderClear(renderer);
}

uint8_t visual;

void visualization(synth_t *synth)
{
    SDL_Rect final_size;
    int x, y;
    int16_t *ptr;

    // no visualisation if we record!
    if (synth->continuous || synth->is_recording) {
        return;
    }

    if (synth->waveptr == NULL)
        return;

    visual = VISUAL_WAVE;

    switch (visual) {
    case VISUAL_WAVE:
        SDL_memset(pixels, 0x00000000, WIDTH * HEIGHT * sizeof(*pixels));

        ptr = synth->waveptr;

	for (x = 0; x < WIDTH; x++) {
	    for (y = HEIGHT; y < HEIGHT; y ++) {
                pixels[x + (y * WIDTH)] = 0xff00ffff;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * sizeof(uint32_t));
        break;

    case VISUAL_ICON:
        ptr = synth->waveptr;
        uint16_t num = (uint16_t) (*ptr >> 3);
        printf("num is %d\n", num);
        if (num > 768) {
            num = 0;
        }

        x = num;
        y = num;

        final_size.w = x;
        final_size.h = y;
        final_size.x = (WIDTH / 2) - (x / 2);
        final_size.y = (HEIGHT / 2) - (y / 2);  //    (HEIGHT / 2) + (y / 2);

        SDL_Surface *background = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                                       WIDTH, HEIGHT, 32,
                                                       0, 0, 0, 0);
        if (!background)
            fail("SDL_CreateRGBSurface: %s\n", SDL_GetError());

        SDL_Surface *icon = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                                 x, y, 32, 0, 0, 0,
                                                 0);
        if (!icon)
            fail("SDL_CreateRGBSurface: %s\n", SDL_GetError());

        SDL_Surface *avatar = load_image("avatar.png");
        if (!avatar)
            fail("load_image: %s\n", SDL_GetError());

        SDL_BlitScaled(avatar, NULL, icon, &final_size);
        SDL_BlitSurface(icon, NULL, background, &final_size);

        SDL_Texture *t =
            SDL_CreateTextureFromSurface(renderer, background);
        if (!t)
            fail("SDL_CreateTextureFromSurface: %s\n", SDL_GetError());

        SDL_FreeSurface(avatar);
        SDL_FreeSurface(icon);
        SDL_FreeSurface(background);
        SDL_RenderCopy(renderer, t, NULL, NULL);
        SDL_DestroyTexture(t);

        break;
    }

    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
}


void current_action(char *fmt, ...)
{
    char buf[8192] = { 0 };
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    snprintf(current_action_text, sizeof(current_action_text), "%s", buf);
}
