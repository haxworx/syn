/*
	(c) Copyright 2015, 2016. Al Poole <netstar@gmail.com>
	All Rights Reserved.
*/
    
#include "stdinc.h"
#include "audio.h"
#include "video.h"
#include "events.h"

int main(int argc, char **argv)
{
    (void) argc, (void) argv;

    printf("\n(c) Copyright 2016. Al Poole. All rights reserved.\n");
    printf("Close window to exit, otherwise, play the keyboard!\n");
    printf("Thanks to: Ed Poole and Steven Carle for debugging!\n\n");

    fflush(stdout);

    sound_t *sound = initialise();

    while (events_process(sound)) {
        // after each pass of events...
    }

    closedown(sound);

    return EXIT_SUCCESS;
}
