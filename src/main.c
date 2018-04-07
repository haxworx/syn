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

    synth_t *synth = synth_new();

    do {

    } while (Run(synth));

    synth_shutdown(synth);

    return EXIT_SUCCESS;
}
