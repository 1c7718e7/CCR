CC := clang
CFLAGS      += -std=c99 -Wall -pedantic -lm -DNDEBUG
SDL_CFLAGS  += $(shell pkg-config --cflags sdl)
SDL_LDFLAGS += $(shell pkg-config --libs sdl)
