TARGET = syn
SRC_DIR=src

PKGS=sdl2 SDL2_image SDL2_ttf

LIBS = -lm

ifeq ($(OS),Windows_NT)
	OS := WINDOWS=1
else
	OS := UNIX=1
endif

CFLAGS = -std=c99 -Wall -g -D$(OS) -D_FILE_OFFSET_BITS=64 $(shell pkg-config --cflags $(PKGS))
SDL_LIBS = $(shell pkg-config --libs $(PKGS))

OBJECTS = stdinc.o audio.o video.o main.o

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) $(SDL_LIBS) -o $@

main.o: $(SRC_DIR)/main.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/main.c -o $@

stdinc.o: $(SRC_DIR)/stdinc.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/stdinc.c -o $@

audio.o: $(SRC_DIR)/audio.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/audio.c -o $@

video.o: $(SRC_DIR)/video.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/video.c -o $@


clean:
	-rm $(OBJECTS) $(TARGET)
